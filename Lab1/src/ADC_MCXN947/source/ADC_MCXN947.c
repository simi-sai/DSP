/**
 * @file ADC_MCXN947.c
 * @brief Application entry point.
 */
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_debug_console.h"

//          samples [kS/s]   =    8,    16,   22,   44,   48;
static uint32_t samplings[5] = {18750, 9375, 6818, 3409, 3125};
static volatile uint8_t sample_config = 0;

typedef enum {START, STOP} control_state;
control_state state;

// ADC Buffer Circular q15
static volatile q15_t buffer[512];
static volatile buffer_idx = 0;

/**
 * @brief Timer0 Interruption Handler
 */
void CTIMER0_IRQHandler(void) {
	CTIMER_ClearStatusFlags(CTIMER0_PERIPHERAL, kCTIMER_Match0Flag);

	if (state == START) {
		LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);
	} else {
		uint32_t next_value = ((int32_t)buffer[buffer_idx] + 32768) >> 4;
		DAC_SetData(DAC0_PERIPHERAL, next_value);
		buffer_idx = (buffer_idx + 1U) % 512U;
	}
}

/**
 * @brief ADC0 Interruption Handler
 */
void ADC0_IRQHandler(void) {
	lpadc_conv_result_t result;

	if (LPADC_GetConvResult(ADC0_PERIPHERAL, &result, 0)) {
		buffer[buffer_idx] = (q15_t)(result.convValue - 32768);	      	  // Transforms from unsigned binary to q15
		DAC_SetData(DAC0_PERIPHERAL, (uint32_t)(result.convValue >> 4));  // Skips transformation and reduces resolution
		buffer_idx = (buffer_idx + 1U) % 512U;  // buffer circular
	}
}

/**
 * @brief GPIO0/0 Interruption Handler
 *
 * Switch 3 (SW3) dedicated handler, 
 * this switch STARTS or STOPS the sampling
 */
void GPIO00_IRQHandler(void) {
	state = (state == START) ? STOP : START;     // toggle
	GPIO_PortClearInterruptFlags(GPIO0, 1U << 6U);
}

/**
 * @brief GPIO0/1 Interruption Handler
 *
 * Switch 2 (SW2) dedicated handler,
 * this switch changes the sampling time in a circular way.
 */
void GPIO01_IRQHandler(void) {
	sample_config = (sample_config + 1U) % 5U;
	CTIMER0_PERIPHERAL->MR[kCTIMER_Match_0] = samplings[sample_config];
	CTIMER_Reset(CTIMER0_PERIPHERAL);

	GPIO_PortClearInterruptFlags(GPIO0, 1U << 23U);
}

/**
 * @brief Sets the RGB LED color depending on the Sampling configuration
 * @param rate sampling configuration rate index
 */
void setLedColors(uint8_t rate) {
	LED_RED_OFF(); LED_GREEN_OFF(); LED_BLUE_OFF();

	switch(rate) {
		case 0: LED_GREEN_ON(); break;                //  8 kS/s - VERDE
		case 1: LED_BLUE_ON(); break;                 // 16 kS/s - AZUL
		case 2: LED_RED_ON(); LED_GREEN_ON(); break;  // 22 kS/s - AMARILLO
		case 3: LED_RED_ON(); break;                  // 44 kS/s - ROJO
		case 4: LED_RED_ON(); LED_BLUE_ON(); break;   // 48 kS/s - VIOLETA
		default: break;
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

    EnableIRQ(CTIMER0_IRQn);

    uint8_t last_rate = 6; // ignore initial value

    while (1) {
        uint8_t rate = sample_config % 5U;

        if (rate != last_rate) {
            last_rate = rate;

            setLedColors(rate);
        }
    }

    return 0;
}
