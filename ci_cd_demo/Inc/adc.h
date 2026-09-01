/*
 * adc.h
 *
 *  Created on: Aug 27, 2026
 *      Author: Agha Ikechukwu Kingsley
 */

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void pf8_adc_init(void);
void start_adc_conversion (void);

uint32_t read_adc(void);


#endif /* ADC_H_ */
