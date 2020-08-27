/******************************************************************************
 * Includes
 *******************************************************************************/
#include "Board.h"

#include <ti/ndk/inc/netmain.h>
#include <ti/ndk/inc/serrno.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/ports/SemaphoreP.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Event.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <UdpFxn.h>

#define USER_AGENT "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096

const char *PORT_STR = "31717";
const char *SERVIP_STR = "192.168.0.136";
uint32_t PORT = 31717;
uint32_t MAXBUF = 1024;

/******************************************************************************
 * Function Bodies
 *******************************************************************************/
/*
 *  ======== send adc values to pi server ========
 */
void UdpFxn(UArg arg0, UArg arg1)
{
//	fdOpenSession((void *)Task_self());

	int ct = 0;
	while (ct < 3)
	{

	System_printf("\n>>>------------------------------->>>\n");
	System_printf("Inside UdpFxn()\n");
	System_flush();

//>>>------------------------------------------------------------->>>
	int err = NULL;

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	SOCKET sockfd;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    // IPv4; AF_UNSPEC -> IPv4 or IPv6
	hints.ai_socktype = SOCK_DGRAM; // Datagram socket = UDP
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          // Any protocol

	s = getaddrinfo(SERVIP_STR, PORT_STR, &hints, &result);
	err = NULL;
	if (s != 0)
	{
		err = fdError();
		System_printf("getaddrinfo() failed: err=%d\n", err);
		System_flush();
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		err = NULL;
		if (sockfd < 0)
		{
			err = fdError();
			System_printf("socket() failed, err=%d\n", err);
			System_flush();
			continue;
		}
		if (sockfd > 0)
		{
			System_printf("Huzzah, a socket!\n");
			System_flush();
		}
		break;
	}
//<<<-------------------------------------------------------------<<<


//>>>------------------------------------------------------------->>>
	// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/354644?NDK-UDP-Client-Issue
	struct sockaddr_in bindAddr;
	memset(&bindAddr, 0, sizeof(bindAddr));
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = INADDR_ANY;
	bindAddr.sin_port = htons(1025);		// I guess just a random port?

	err = NULL;
	if (bind(sockfd, (struct sockaddr*) &bindAddr, sizeof(bindAddr)) < 0)
	{
		err = fdError();
		System_printf("bind() failed, err = %d\n", err);
		System_flush();
	}
//<<<-------------------------------------------------------------<<<


//>>>------------------------------------------------------------->>>
	struct sockaddr_in servAddr;

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_UNSPEC;
	servAddr.sin_port = htons(PORT);
	inet_aton("192.168.0.136", &servAddr.sin_addr);

	System_printf("servAddr.sin_addr.s_addr = %d\n", servAddr.sin_addr.s_addr);
	System_flush();

	char *sendBuf = "spaghetti";
	int bytesSent = NULL;
	err = NULL;

	do
	{
		bytesSent = sendto(sockfd, sendBuf, sizeof(sendBuf), 0,
							(struct sockaddr*) &servAddr, sizeof(servAddr));
		if (bytesSent < 0)
		{
			err = fdError();
			System_printf("sendto() of %s failed: err=%d\n", sendBuf, err);
			System_flush();
		}
		else if (bytesSent == sizeof(sendBuf))
		{
			System_printf("sendto() of %s: bytesSent=%d, bufSize=%d\n", sendBuf,
							bytesSent, sizeof(sendBuf));
			System_flush();
		}
	} while (bytesSent < 0);
//<<<-------------------------------------------------------------<<<

	System_printf("Calling fdClose()\n");
	System_flush();
	fdClose(sockfd);

	ct++;

//	System_printf("Calling fdCloseSession()\n");
//	System_flush();
//	fdCloseSession((void*) Task_self());
	}
}
