/*
 * MicADC.h
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */
#include <ti/sysbios/knl/Semaphore.h>

#ifndef MICADC_H_
#define MICADC_H_

extern double noiseLvlAvg;
extern double noiseLvlValues[7];
extern uint32_t ADCValues[7];
extern int lastNoiseIndex;

void initializeADCnStuff(void);
void micADC(void);
void ADC_task_fxn(UArg arg0, UArg arg1);

#endif /* MICADC_H_ */
