/*
 * SendToPi.h
 *
 *  Created on: 8 Aug 2020
 *      Author: noa
 */
#include <stdint.h>

#ifndef UDPFXN_H_
#define UDPFXN_H_

void UdpFxn(void);
//void create_Pi_event(void);
//int setup_Pi_Task(void);
void createSockThread(int prio);

void convertServIP(struct sockaddr_in servAddr, const char *SERVIP_STR);

#endif /* UDPFXN_H_ */
