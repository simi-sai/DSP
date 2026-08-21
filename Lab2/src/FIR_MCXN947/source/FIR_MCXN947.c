/**
 * @file    FIR_MCXN947.c
 * @brief   Application entry point.
 */
#include <stdio.h>
#include <dsp/filtering_functions.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

#include "coeficientes.h"

//          samples [kS/s]   =    8,    16,   22,   44,   48;
static uint32_t samplings[5] = {18750, 9375, 6818, 3409, 3125};
static volatile uint8_t sample_config = 0;

static volatile q15_t input_buffer[512], output_buffer[512];
static volatile uint16_t in_idx = 1, out_idx = 0;

typedef enum {OFF = 0, LP, HP, BP, BS} filter_type;
static volatile filter_type filter = OFF;

//            match's [s]   =       LP-1s,    HP-0.75s,  BP-0.5s,  BS-0.25s
static uint32_t led_matches[4] = {150000000, 112500000, 75000000, 37500000};

typedef enum {LEDS_OFF = 0, LEDS_ON} led_state;
static volatile led_state leds_toggle = LEDS_ON;

// Inicializacion de los Filtros FIR
arm_fir_instance_q15 fir_lp, fir_hp, fir_bp, fir_bs;
static q15_t state_lp[TAPS_LP + 1],
             state_hp[TAPS_HP + 1],
             state_bp[TAPS_BP + 1],
             state_bs[TAPS_BS + 1];

/**
 * @brief Timer0 Interruption Handler
 *
 * Starts conversion and sends next value in output_buffer to DAC
 */
void CTIMER0_IRQHandler(void) {
	CTIMER_ClearStatusFlags(CTIMER0_PERIPHERAL, CTIMER0_ADC_MATCH_CHANNEL);

	q15_t input = input_buffer[out_idx];
	q15_t output;

	switch(filter) {
		case OFF: output = input; break;	// unfiltered
		case LP: arm_fir_q15(&fir_lp, &input, &output, 1); break;
		case HP: arm_fir_q15(&fir_hp, &input, &output, 1); break;
		case BP: arm_fir_q15(&fir_bp, &input, &output, 1); break;
		case BS: arm_fir_q15(&fir_bs, &input, &output, 1); break;
		default: output = input; break;		// unfiltered
	}

	DAC_SetData(DAC0_PERIPHERAL, (uint32_t)(output + 32768) >> 4);
	output_buffer[out_idx] = output;
	out_idx = (out_idx + 1U) % 512U;

	LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);
}

/**
 * @brief Timer1 Interruption Handler
 *
 * Toggles LEDs at different rates depending on the active filter
 */
void CTIMER1_IRQHandler(void) {
	CTIMER_ClearStatusFlags(CTIMER1_PERIPHERAL, CTIMER1_LED_TOGGLE_CHANNEL);

	leds_toggle = (filter == OFF) ? LEDS_ON :
				  (leds_toggle == LEDS_ON) ? LEDS_OFF : LEDS_ON;
}

/**
 * @brief ADC0 Interruption Handler
 *
 * Saves the conversion result in q15 format in circular input_buffer
 */
void ADC0_IRQHandler(void) {
	lpadc_conv_result_t result;

	if (LPADC_GetConvResult(ADC0_PERIPHERAL, &result, 0)) {
		input_buffer[in_idx] = (q15_t)(result.convValue - 32768);	// Transforms from unsigned binary to q15
		in_idx = (in_idx + 1U) % 512U;  // buffer circular
	}
}

/**
 * @brief GPIO0/0 Interruption Handler
 *
 * Switch 3 (SW3) dedicated handler,
 * this switch changes the filter in action in a circular way
 * OFF -> LP -> HP -> BP -> BS -> ...
 */
void GPIO00_IRQHandler(void) {
	filter = (filter + 1) % 5;

	switch(filter) {
		case LP: CTIMER1_PERIPHERAL->MR[CTIMER1_LED_TOGGLE_CHANNEL] = led_matches[0]; break;
		case HP: CTIMER1_PERIPHERAL->MR[CTIMER1_LED_TOGGLE_CHANNEL] = led_matches[1]; break;
		case BP: CTIMER1_PERIPHERAL->MR[CTIMER1_LED_TOGGLE_CHANNEL] = led_matches[2]; break;
		case BS: CTIMER1_PERIPHERAL->MR[CTIMER1_LED_TOGGLE_CHANNEL] = led_matches[3]; break;
		default: break;
	}
	CTIMER_Reset(CTIMER1_PERIPHERAL);

	GPIO_PinClearInterruptFlag(GPIO0, BOARD_SW3_GPIO_PIN);
}

/**
 * @brief GPIO0/1 Interruption Handler
 *
 * Switch 2 (SW2) dedicated handler,
 * this switch changes the sampling time in a circular way
 * 8 kS/s -> 16 kS/s -> 22 kS/s -> 44 kS/s -> 48 kS/s -> ...
 */
void GPIO01_IRQHandler(void) {
	sample_config = (sample_config + 1U) % 5U;
	CTIMER0_PERIPHERAL->MR[CTIMER0_ADC_MATCH_CHANNEL] = samplings[sample_config];
	CTIMER_Reset(CTIMER0_PERIPHERAL);

	GPIO_PinClearInterruptFlag(GPIO0, BOARD_SW2_GPIO_PIN);
}

/**
 * @brief Sets the RGB LED color depending on the Sampling configuration
 * @param rate sampling configuration rate index
 */
void setLedColors(uint8_t rate) {
	LED_RED_OFF(); LED_GREEN_OFF(); LED_BLUE_OFF();

	if (leds_toggle == LEDS_ON) {
		switch(rate) {
			case 0: LED_GREEN_ON(); break;                //  8 kS/s - VERDE
			case 1: LED_BLUE_ON(); break;                 // 16 kS/s - AZUL
			case 2: LED_RED_ON(); LED_GREEN_ON(); break;  // 22 kS/s - AMARILLO
			case 3: LED_RED_ON(); break;                  // 44 kS/s - ROJO
			case 4: LED_RED_ON(); LED_BLUE_ON(); break;   // 48 kS/s - VIOLETA
			default: break;
		}
	}
}

/*
 * @brief Application entry point.
 */
int main(void) {
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
    BOARD_InitDebugConsole();
#endif

    arm_fir_init_q15(&fir_lp, TAPS_LP, coeffs_lp, state_lp, 1);
    arm_fir_init_q15(&fir_hp, TAPS_HP, coeffs_hp, state_hp, 1);
    arm_fir_init_q15(&fir_bp, TAPS_BP, coeffs_bp, state_bp, 1);
    arm_fir_init_q15(&fir_bs, TAPS_BS, coeffs_bs, state_bs, 1);

    EnableIRQ(CTIMER0_IRQn);
    EnableIRQ(CTIMER1_IRQn);

    uint8_t last_rate = 6, last_toggle = 2; // ignore initial value

        while (1) {
            uint8_t rate = sample_config;
            uint8_t toggle = leds_toggle;

            if (rate != last_rate || toggle != last_toggle) {
                last_rate = rate;
                last_toggle = toggle;

                setLedColors(rate);
            }
        }

    return 0 ;
}
