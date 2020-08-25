/*
 * MicADC.h
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */
#include <ti/sysbios/knl/Semaphore.h>

#ifndef MICADC_H_
#define MICADC_H_

void initializeADCnStuff(void);
void micADC(void);
void create_ADC_event(void);
int setup_ADC_Task(int prio);

extern double noiseLvlAvg;
extern double noiseLvlValues[7];
extern uint32_t ADCValues[7];
extern int lastNoiseIndex;

extern Event_Handle udpEvent;
extern Semaphore_Handle semHandle;

#endif /* MICADC_H_ */
