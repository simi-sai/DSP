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
static volatile uint8_t running = 0;

// ADC Buffer Circular q15
static volatile int16_t q15_buffer[512];

/**
 * @brief Timer0 Interruption Handler
 */
void CTIMER0_IRQHandler(void) {
	CTIMER_ClearStatusFlags(CTIMER0_PERIPHERAL, kCTIMER_Match0Flag);
	LPADC_DoSoftwareTrigger(ADC0_PERIPHERAL, 1U);
}

/**
 * @brief ADC0 Interruption Handler
 */
void ADC0_IRQHandler(void) {
	static uint16_t idx = 0;

	lpadc_conv_result_t result;

	if (LPADC_GetConvResult(ADC0_PERIPHERAL, &result, 0)) {
		q15_buffer[idx] = (int16_t)result.convValue;    // [14:3] = q15
		DAC_SetData(DAC0_PERIPHERAL, (uint32_t)result.convValue >> 3);  // [14:3] -> [11:0]
		idx = (idx + 1U) % 512U;  // buffer circular
	}
}

/**
 * @brief GPIO0/0 Interruption Handler
 *
 * Switch 3 (SW3) dedicated handler, 
 * this switch STARTS or STOPS the sampling
 */
void GPIO00_IRQHandler(void) {
	if (running % 2 == 0) {
		CTIMER_StartTimer(CTIMER0_PERIPHERAL);	// START
	} else {
		CTIMER_StopTimer(CTIMER0_PERIPHERAL);	// STOP
	}

	running++;
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

    uint8_t last_rate = 6, last_run = 2; // random last numbers for first run

    while (1) {
        uint8_t run = running % 2U;
        uint8_t rate = sample_config % 5U;

        if ((run != last_run) || (rate != last_rate)) {
            last_run = run; last_rate = rate;

            if (run) {
            	setLedColors(rate);
            } else {
            	LED_RED_OFF(); LED_GREEN_OFF(); LED_BLUE_OFF();
            }
        }
    }

    return 0;
}
