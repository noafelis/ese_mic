/*
 * SendToPi.c
 *
 *  Created on: 8 Aug 2020
 *      Author: noa
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include "Board.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
//#include <errno.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Hwi.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <SendToPi_UDP.h>

#include "MicADC.h"

#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096

const char *RPI_IP = "192.168.0.136";
//const char *PORT = "31717";
uint32_t PORT = 31717;
uint32_t MAXBUF = 1024;

/******************************************************************************
* Function Bodies
*******************************************************************************/

/*
 *  ======== send adc values to pi server ========
 */
// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/555107?NDK-socket-creation-error
void sendADCValuesToPi(void)
{
	int sockfd;
	struct sockaddr_in localAddr;
	struct sockaddr_in piServAddr;
	socklen_t addrlen;
	int status;
	char sendBuf[MAXBUF];
	int err;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1)
	{
		err = fdError();
		System_printf("socket() failed: err=%d\n", err);
		System_flush();
	}

/*	sendBuf = malloc(MAXBUF);
	{
		System_printf("malloc failed\n");
		System_flush();
	}
	*/
	memset(sendBuf, 0, sizeof(sendBuf));
	sprintf(sendBuf, "hello dis is cat");

	addrlen = sizeof(struct sockaddr_in);
	piServAddr.sin_family = AF_INET;
	piServAddr.sin_port = PORT;
	piServAddr.sin_addr.s_addr = 0xc0a80088;		//192.168.0.136 -> c0.a8.00.88 (0xc0a80088)

	sendto(sockfd, sendBuf, sizeof(sendBuf), 0, (struct sockaddr*)&piServAddr, addrlen);

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = 0;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	status = bind(sockfd, (struct sockaddr *)&localAddr, addrlen);
	if (status == -1)
	{
		err = fdError();
		System_printf("bind() failed: err=%d\n", err);
		System_flush();
	}

	addrlen = sizeof(piServAddr);
	recvfrom(sockfd, sendBuf, MAXBUF, 0, (struct sockaddr*)&piServAddr, &addrlen);
	int i = 0;
	do
	{
		System_printf("response:\n%s", sendBuf[i]);
		System_flush();
		i++;
	} while (sendBuf[i] != 0);

	close(sockfd);
}
