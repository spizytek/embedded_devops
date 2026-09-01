/*
 * fpu.c
 *
 *  Created on: Aug 12, 2026
 *      Author: Agha Ikechukwu Kingsley
 */


#include "stm32f4xx.h"


void enable_fpu(void){

	/*Enable floating point unit: Enable CP10 and CP11 full access */
	SCB->CPACR |= (3U << 20); //Bit field 20:-> 1, Bit field 21:-> 1
	SCB->CPACR |= (3U << 22); //Bit field 22:-> 1, Bit field 23:-> 1

}
