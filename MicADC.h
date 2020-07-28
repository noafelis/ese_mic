/*
 * MicADC.h
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */

#ifndef MICADC_H_
#define MICADC_H_

void initializeADCnStuff(void);
static void micADC(void);
void create_ADC_event(void);
int setup_ADC_Task(void);

extern double noiseLvlAvg;
extern double noiseLvlValues[7];
extern uint32_t ADCValues[7];
extern int lastNoiseIndex;

#endif /* MICADC_H_ */
