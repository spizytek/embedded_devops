/*
 * bsp.c
 *
 *  Created on: Aug 17, 2026
 *      Author: Agha Ikechukwu Kingsley
 */


#include "stm32f4xx.h"
#include "bsp.h"




void led_init(void){
	/*Enable RCC clock for GPIO access*/
	RCC->AHB1ENR |= GPIOGCLK_EN;

	/*Configure GPIOx as Output using the GPIO moder register: 2bit fields = 0 | 1 bits*/
	GPIOG->MODER &= GPIO13_RESET; //Clear existing bits to 0
	GPIOG->MODER |= GPIO13_SET;


}


void button_init(void){

	/*Enable RCC clock for GPIO A access*/
	RCC->AHB1ENR |= GPIOACLK_EN;

	/*Configure GPIO PA0 as Input using the GPIO moder register: 2bit fields = 0 | 0 bits*/
	GPIOA->MODER &= GPIO0_RESET	; //Clear existing bits to 0


}


uint8_t button_get_input(void){
	/*Read the state of the input*/
	return GPIOA->IDR & GPIO0_STATE;
}

void led_toggle(void){

	/*Toggle pin 13 in the ODR register to blink the attached LED*/
	GPIOG->ODR ^= GPIO13_LEDTOGGLE;

}


void led_on(void){
	/*Set PG13 high*/
	GPIOG->ODR |= GPIO13_LEDSET;
}


void led_off(void){
	/*Set PG13 low*/
	GPIOG->ODR |= GPIO13_LEDRESET;
}


