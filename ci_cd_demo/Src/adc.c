/*
 * adc.c
 *
 *  Created on: Aug 27, 2026
 *      Author: Agha Ikechukwu Kingsley
 */


#include "stm32f4xx.h"
#include "adc.h"


#define GPIOFCLK_EN 	(1U << 5)

#define GPIOF_ANALAOGUE_RESET 	~(3U << 16)
#define GPIOF_ANALAOGUE_EN 	     (3U << 16)

#define ADC3_EN 	   			 (1U << 10)

#define ADC3_CHANNEL_SEQ_LEN	 ~(0xFU << 20)
#define ADC3_CHANNEL_SEQ_RESET	 ~(0x1FU < 0)
#define ADC3_CHANNEL_SEQ_START	  (0x6U < 0)

#define ADC_ON					  (1U << 0)
#define ADC_CONT_EN					  (1U << 1)
#define ADC_SWSTART					  (1U << 30)

#define ADC_EOC					   (1U << 1)





// ADC3_IN6: PF8, ADC3, Channel 6
//

void pf8_adc_init(void){

/***Configure the ADC GPIO pin***/
	//Enable clock access to PORT F
	RCC->AHB1ENR |= GPIOFCLK_EN;

	//Configure GPIO Pin PF8 as analog
	GPIOF->MODER &= GPIOF_ANALAOGUE_RESET;
	GPIOF->MODER |=  GPIOF_ANALAOGUE_EN;


/***Configure the ADC module***/

	//Enable clock access to ADC module
	RCC->APB2ENR |= ADC3_EN;

	//Set conversion sequence start: Tells ADC where to convert channel 6
	ADC3->SQR3 &= ADC3_CHANNEL_SEQ_RESET;
	ADC3->SQR3 |= ADC3_CHANNEL_SEQ_START;


	//Set conversion sequence length
	ADC3->SQR1 &= ADC3_CHANNEL_SEQ_LEN; // Ensures we are performing only one conversion.


	//Enable the ADC
	ADC3->CR2 |= ADC_ON;

}





void start_adc_conversion (void){

	//Enable continuous conversion
	//Enable the ADC
	ADC3->CR2 |= ADC_CONT_EN;

	//Start ADC conversion
	ADC3->CR2 |= ADC_SWSTART;
}

uint32_t read_adc(void){

	//Wait for the conversion to be completed.
	while( !(ADC3->SR & ADC_EOC) ){}

	//Read converted.
	return ADC3->DR;
}
