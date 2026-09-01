/*
 * bsp.h
 *
 *  Created on: Aug 17, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#ifndef BSP_H_
#define BSP_H_


#define GPIOGCLK_EN 	(1U << 6)
#define GPIOACLK_EN 	(1U << 0)

#define GPIO13_RESET	~(3U << 26)
#define GPIO13_SET		(1U << 26)

#define GPIO13_LEDTOGGLE (1U << 13)

#define GPIO13_LEDSET   (1U << 13)
#define GPIO13_LEDRESET ~(1U << 13)

#define GPIO0_RESET		~(3U << 0)

#define GPIO0_STATE		 (1U << 0)

void led_init(void);
void led_toggle(void);


void button_init(void);
uint8_t button_get_input(void);


#endif /* BSP_H_ */
