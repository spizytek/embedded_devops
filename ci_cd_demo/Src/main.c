/*
 * main.c
 *
 *  Created on: Aug 6, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#include <stdint.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "health.h"
#include "fpu.h"
#include "uart.h"
#include "bsp.h"
#include "timebase.h"
#include "adc.h"



/*Include watch dog*/


int main(void)
{

	/*Enable FPU*/
	enable_fpu();

	/*Enable UART debug*/
	debug_uart_init();


	/*Init Delay Timer*/
	timebase_init();

	/*Init LED*/
	led_init();

	/*Init Button*/
	button_init();

	/*Init ADC*/
	pf8_adc_init();
	start_adc_conversion();

	/* Emit the Firmware Identity Banner */
	health_emit_identity_line();

	uint32_t sensor_val;
    /* Loop forever */
	while(1)
	{
		led_toggle();

		/*Acts a delay*/
		delay(1);
		printf("Hello from uart retarget...\n\r");

		if (button_get_input())
			printf("Button pressed!!! \n\r");

		sensor_val = read_adc();
		printf("ADC value:%lu ...\n\r", sensor_val);

	}
}
