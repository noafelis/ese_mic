/*
 * SendToPi.h
 *
 *  Created on: 8 Aug 2020
 *      Author: noa
 */
#include <stdint.h>
#include <ti/sysbios/knl/Semaphore.h>

#ifndef UDPFXN_H_
#define UDPFXN_H_

extern Semaphore_Handle semHandle;
extern Semaphore_Params semParams;
extern Semaphore_Struct sem0Struct;

void UdpFxn(UArg arg0, UArg arg1);
int setup_UDP_Task(int prio);

#endif /* UDPFXN_H_ */
