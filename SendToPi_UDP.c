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
//#include <ti/ndk/inc/netmain.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>

#include "MicADC.h"
#include "SendToPi_UDP.h"

#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096
#define UDPPORT 1000

const char *RPI_IP = "192.168.0.136";
const char *PORT_INT = "31717";
uint32_t PORT = 31717;
uint32_t MAXBUF = 1024;

/******************************************************************************
* Function Bodies
*******************************************************************************/

//TODO follow this!!! http://software-dl.ti.com/simplelink/esd/simplelink_msp432e4_sdk/2.20.00.20/docs/ndk/NDK_Users_Guide.html


/*
 *  ======== send adc values to pi server ========
 */
// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/555107?NDK-socket-creation-error
void sendADCValuesToPi(void)
{
	fdOpenSession((void *)Task_self());

	int sockfd;
	struct sockaddr_in localAddr;
	struct sockaddr_in piServAddr;

	struct addrinfo hints;
//	struct addrinfo *results = NULL;
//	struct addrinfo *servaddr = NULL;
//	int value;

	socklen_t addrlen;
	int status;
	char sendBuf[MAXBUF];
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

/*
	if ((value = getaddrinfo(RPI_IP, PORT_INT, &hints, &results)) < 0)
	{
		System_printf("getaddrinfo failed: 0x%x\n", fdError());
		System_flush();
		if (value == -2 || value == 11004)
		{
			System_printf("unrecognized IP address\n");
			System_flush();
		}
		System_flush();
	}

	for (servaddr = results; servaddr != NULL; servaddr = servaddr->ai_next)
	{
		if ((sockfd = socket(results->ai_family, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		{
			err = fdError();
			System_printf("socket() failed: err=%d\n", err);
			System_flush();
			continue;
		}
	}
*/

 	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1)
	{
		err = fdError();
		System_printf("socket() failed: err=%d\n", err);
		System_flush();
	}


	memset(sendBuf, 0, sizeof(sendBuf));
	sprintf(sendBuf, "150");

	addrlen = sizeof(struct sockaddr_in);
	piServAddr.sin_family = AF_INET;
	piServAddr.sin_port = PORT;
	piServAddr.sin_addr.s_addr = INADDR_ANY;		//192.168.0.136 -> c0.a8.00.88 (0xc0a80088)

	if ((sendto(sockfd, sendBuf, sizeof(sendBuf), 0, (struct sockaddr*)&piServAddr, addrlen) < 0))
	{
		err = fdError();
		System_printf("sendto() failed: err=%d\n", err);
		System_flush();
	}

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
	if ((recvfrom(sockfd, sendBuf, MAXBUF, 0, (struct sockaddr*)&piServAddr, &addrlen) < 0))
	{
		err = fdError();
		System_printf("recvfrom() failed: err=%d\n", err);
		System_flush();
	}
	int i = 0;
	System_printf("response:\n");
	do
	{
		System_printf("%s\n", sendBuf[i]);
		System_flush();
		i++;
	} while (sendBuf[i] != 0);

	close(sockfd);

	fdCloseSession((void *)Task_self());
}


void createSockThread(int prio)
{
	//int status;
    Task_Params params;
    Task_Handle mySockThread;

    Task_Params_init(&params);
    params.instance->name = "mySockThread";
    params.priority = prio;
    params.stackSize = 2048;

	mySockThread = Task_create((Task_FuncPtr)sendADCValuesToPi, &params, NULL);

	if (!mySockThread)
	{
		System_printf("Tasc_create() failed!\n");
		System_flush();
	}
}
