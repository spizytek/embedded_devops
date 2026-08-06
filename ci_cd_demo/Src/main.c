/*
 * main.c
 *
 *  Created on: Aug 6, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#include "stm32f4xx.h"
#include "health.h"
#include <stdint.h>

#define GPIOGCLK_EN 	(1U << 6)
#define GPIO13_RESET	~(3U << 26)
#define GPIO13_SET		(1U << 26)

#define GPIO13_LEDTOGGLE (1U << 13)

int main(void)
{

	/*Enable RCC clock for GPIO access*/
	RCC->AHB1ENR |= GPIOGCLK_EN;

	/*Configure GPIOx as Output using the GPIO moder register: 2bit fields = 0 | 1 bits*/
	GPIOG->MODER &= GPIO13_RESET; //Clear existing bits to 0
	GPIOG->MODER |= GPIO13_SET;


	/* Emit the Firmware Identity Banner */
	health_emit_identity_line();

    /* Loop forever */
	while(1)
	{
		/*Toggle pin 13 in the ODR register to blink the attched LED*/
		GPIOG->ODR ^= GPIO13_LEDTOGGLE;

		/*Acts a delay*/
		for(int i =0; i < 500000; i++){}
	}
}
