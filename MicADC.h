/*
 * MicADC.h
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */
#include <ti/sysbios/knl/Semaphore.h>

#ifndef MICADC_H_
#define MICADC_H_

//*************************************************************************
/* Defines */
#define USRBUTTON GPIO_PORTJ_BASE
#define SW1 GPIO_PIN_0
#define SW2 GPIO_PIN_1

//interrupt void micISR(void);
void initializeADCnStuff(void);
void ADC_task_fxn(UArg arg0, UArg arg1);
void micADC();
void ADC_interrupt_handler();
#endif /* MICADC_H_ */
