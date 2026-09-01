/*
 * timebase.c
 *
 *  Created on: Aug 14, 2026
 *      Author: Agha Ikechukwu Kingsley
 */


#include "stm32f4xx.h"
#include "timebase.h"


#define CTRL_ENABLE			(1U << 0)
#define CTRL_TICKINT		(1U << 1)
#define CTRL_CLCKSRC		(1U << 2)
#define CTRL_COUNTFLAG		(1U << 16)
#define ONE_SEC_LOADVAL		16000000U

#define TICK_FREQ			1
#define MAX_DELAY			0xFFFFFFFF

volatile uint32_t global_cur_tick;
volatile uint32_t global_cur_tick_p;


void delay(uint32_t delay){

	uint32_t start_time = get_tick();
	uint32_t wait = delay;

	if(wait < MAX_DELAY)
		wait += (uint32_t) TICK_FREQ;

	while(  (get_tick() - start_time)  < wait){}

}

uint32_t get_tick(void){

	/*Disable global Interrupt*/
	__disable_irq();

	global_cur_tick_p = global_cur_tick;

	/*Enable global Interrupt*/
	__enable_irq();

	return global_cur_tick_p;

}

void tick_increment (){

	global_cur_tick += TICK_FREQ;
}

void timebase_init(void){

	/*Disable global Interrupt*/
	__disable_irq();

	/*Load the timer with number of clock cycles per second*/
	SysTick->LOAD = (ONE_SEC_LOADVAL - 1 );

	/*Clear systick current value register*/
	SysTick->VAL = 0;

	/*Select internal clock source */
	SysTick->CTRL |= CTRL_CLCKSRC;

	/*Enable Interrupt*/
	SysTick->CTRL |= CTRL_TICKINT;

	/*Enable systick*/
	SysTick->CTRL |= CTRL_ENABLE;

	/*Enable global Interrupt*/
	__enable_irq();
}



void SysTick_Handler(void){

	tick_increment ();
}
