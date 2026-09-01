/*
 * uart.c
 *
 *  Created on: Aug 13, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#include "stm32f4xx.h"
#include "uart.h"
#include <stdint.h>


#define GPIOC_EN (1U << 2)
#define UART4_CLK_EN (1U << 19)

#define GPIO_PC10_ALTFUNC_CLEAR	~(3 << 20)
#define GPIO_PC10_ALTFUNC_SET	 (2 << 20) //Sets PC10 as an alt. function pin
#define GPIO_AF8    8U
#define GPIO_PC10_AF8_SET		 (GPIO_AF8 << 8) //8 is bit position.
#define UART4_TXCR1_EN		     (1U << 3)
#define UART4_EN		     	 (1U << 13)

#define DBG_UART_BAUDRATE		 115200U
#define PERIPH_CLK		 		 16000000U
#define SR_TXE_BITFIELD			 7
#define SR_TC_BITFIELD			 6



static void set_baudrate(uint32_t periph_clk, uint32_t baud_rate);
static void uart_write(uint8_t ch);

//re-targetting printf to use uart4
int __io_putchar(int ch) {
		 uart_write(ch);
          return(ch);
}



void debug_uart_init(void){
	/*UART4::GPIOC*/

	/*Enable clock access to port GPIOC*/
	RCC->AHB1ENR |= GPIOC_EN;

	/*Configure the model of pin PC10 to alternate function mode: GPIO PC10: TX, PC11: RX*/
	GPIOC->MODER &= GPIO_PC10_ALTFUNC_CLEAR;
	GPIOC->MODER |= GPIO_PC10_ALTFUNC_SET;

	/*Configure the alternate function to type AF8 which is (UART 4_TX)*/
	GPIOC->AFR[1]|= GPIO_PC10_AF8_SET; //Why AFR[1]? because Pin10 matches the AFR[1] register: pin8 - pin15



	/*Enable clock access to port UART4*/
	RCC->APB1ENR |= UART4_CLK_EN;

	/*UART BAUD: 9600*/
	set_baudrate(PERIPH_CLK, DBG_UART_BAUDRATE);

	/*Configure UART TXRX transfer Mode*/
	UART4->CR1 |= UART4_TXCR1_EN;

	/*Enable UART*/
	UART4->CR1  |= UART4_EN;
}



static uint16_t compute_baud_rate(uint32_t periph_clk, uint32_t baud_rate){
	/*On reset the 16 MHz internal RC oscillator is selected as the default CPU clock.*/

	return (  (periph_clk + (baud_rate/2)) / DBG_UART_BAUDRATE);
}



static void set_baudrate(uint32_t periph_clk, uint32_t baud_rate){

	UART4->BRR  = compute_baud_rate(periph_clk, baud_rate);
}



uint8_t uart_get_flagstatus(uint8_t flagPOS){

	if (UART4->SR & (1 << flagPOS)){
		return SET; //1
	}else{
		return RESET; //0
	}

}

static void uart_write(uint8_t ch){
	//This works for 8 bits data frame only...
	//before sending a data wait for the txe flag be set in the sr registger.
	//We have to wait till data is transferred to the shift register before sending data.
	while(! (uart_get_flagstatus(SR_TXE_BITFIELD)) );

	UART4->DR = (ch & 0xFF);

	//wait till transmission is completed.
	while(! (uart_get_flagstatus(SR_TC_BITFIELD)) );
}
