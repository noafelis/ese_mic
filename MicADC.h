/*
 * MicADC.h
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */

#ifndef MICADC_H_
#define MICADC_H_

void initializeADCnStuff(void);
void initializeInterrupts(void);
//static void micADC(UArg arg0);
static void micADC(void);
void setupMicADCTask(void);


#endif /* MICADC_H_ */
