/*
 * SendToPi.h
 *
 *  Created on: 8 Aug 2020
 *      Author: noa
 */

#ifndef UDPFXN_H_
#define UDPFXN_H_

extern Semaphore_Handle semHandle;
extern Semaphore_Params semParams;
extern Semaphore_Struct sem0Struct;

void UdpFxn(void);
//void create_Pi_event(void);
//int setup_Pi_Task(void);
//void createSockThread(int prio);


#endif /* UDPFXN_H_ */
