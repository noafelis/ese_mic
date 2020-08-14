/*
 * convertServIP.c
 *
 *  Created on: 14 Aug 2020
 *      Author: noa
 */
#include <stdint.h>
#include <ti/ndk/inc/netmain.h>
#include <UdpFxn.h>

void convertServIP(struct sockaddr_in servAddr, const char *SERVIP_STR)
{
//	servAddr.sin_addr.s_addr = inet_aton(AF_INET, SERVIP_STR, buf);
	servAddr.sin_addr.s_addr = inet_addr(SERVIP_STR);
	//servAddr.sin_addr.s_addr = htonl(SERVIP_STR);
}
