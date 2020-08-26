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

extern Semaphore_Handle semHandleUDP;
extern Semaphore_Params semParamsUDP;
extern Semaphore_Struct semStructUDP;

extern Event_Handle UDP_Event;

void UdpFxn(UArg arg0, UArg arg1);
void create_UDP_event(void);

#endif /* UDPFXN_H_ */
