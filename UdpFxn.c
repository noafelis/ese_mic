//--------------------------------------------------------------------------------------------------------
/******************************************************************************
 * https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.4.0/com.ibm.zos.v2r4.hala001/syserret.htm
 *
 *
 * connect() failed: err=6
 * 6	ENXIO	All	The device or driver was not found.	Check status of the device attempting to access.
 *
 * jetzt neu!! sendto() failed: err=6 (no longer 22 EINVAL)
 *
 *
 *******************************************************************************/
//--------------------------------------------------------------------------------------------------------
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
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

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

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <UdpFxn.h>

#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096

const char *PORT_STR = "31717";
uint32_t PORT = 31717;
const char *SERVIP_STR = "192.168.0.136";
uint32_t MAXBUF = 1024;

#include <ti/sysbios/knl/Semaphore.h>
Semaphore_Handle semHandle;
Semaphore_Struct sem0Struct;

/******************************************************************************
 * Function Bodies
 *******************************************************************************/

//TODO follow this!!! http://software-dl.ti.com/simplelink/esd/simplelink_msp432e4_sdk/2.20.00.20/docs/ndk/NDK_Users_Guide.html

/*
 *  ======== send adc values to pi server ========
 */
// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/555107?NDK-socket-creation-error
void UdpFxn(UArg arg0, UArg arg1)
{
//	fdOpenSession((void *)Task_self());

// don't know how semaphore work, when this is called UartFxn is preempted.

	System_printf("Inside UdpFxn().\n");
	System_flush();

	/* semaphore should be posted by ADC task.
	 *
	 *
	 if ((SemPend(semHandle, SemaphoreP_WAIT_FOREVER)) == 0)
	 {
	 System_printf("SemPend() failed, couldn't obtain semaphore.\n");
	 System_flush();
	 }
	 */

//>>>------------------------------------------------------------->>>
	int err = NULL;

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	SOCKET sockfd;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;    // IPv4; AF_UNSPEC -> IPv4 or IPv6
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
	/*	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	 if ((int)sockfd == -1)
	 {
	 err = fdError();
	 System_printf("socket() failed: err=%d\n", err);
	 System_flush();
	 }
	 */

	struct sockaddr_in servAddr;
	struct in_addr wtf;
	int addrlen;

	memset(&servAddr, 0, sizeof(servAddr));
	addrlen = sizeof(struct sockaddr_in);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = PORT;
	inet_aton("192.168.0.136", &wtf);
	servAddr.sin_addr.s_addr = wtf.s_addr;

	System_printf("servAddr.sin_addr.s_addr = %d\n", wtf.s_addr);
	System_flush();

	char *sendBuf = "1";
	int data;
	err = NULL;

	while (1)
	{
		if ((data = sendto(sockfd, sendBuf, sizeof(sendBuf), 0,
							(struct sockaddr*) &servAddr, addrlen)) < 0)
		{
			err = fdError();
			System_printf("sendto() failed: err=%d\n", err);
			System_flush();

			Task_sleep(1000);
		}
		else
		{
			System_printf(
					"at least sendto() didn't produce an error, maybe? data=%d\n",
					data);
			System_flush();
			Task_sleep(1000);
		}
	}

//<<<-------------------------------------------------------------<<<

	//close(sockfd);

//	fdCloseSession((void *)Task_self());
}

