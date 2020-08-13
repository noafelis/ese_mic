//--------------------------------------------------------------------------------------------------------
/******************************************************************************
 * https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.4.0/com.ibm.zos.v2r4.hala001/syserret.htm
 *
 *
 * connect() failed: err=6
 * 6	ENXIO	All	The device or driver was not found.	Check status of the device attempting to access.
 *
 * sendto() failed: err=22
 * 22	EINVAL	All types	An incorrect argument was specified.	Check the validity of function parameters.
 *
 *
*******************************************************************************/
//--------------------------------------------------------------------------------------------------------

/******************************************************************************
* Includes
*******************************************************************************/
#include "Board.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>

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
//#include <ti/sysbios/knl/Event.h>
//#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/hal/Hwi.h>

//#include <ti/ndk/inc/netmain.h>
//#include <ti/ndk/inc/_nettool.h>

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

const char *RPI_IP = "192.168.0.136";
const char *PORT_STR = "31717";
uint32_t PORT = 31717;
const char *SERVIP_STR = "192.168.0.136";
uint32_t MAXBUF = 1024;

/******************************************************************************
* Function Bodies
*******************************************************************************/

//TODO follow this!!! http://software-dl.ti.com/simplelink/esd/simplelink_msp432e4_sdk/2.20.00.20/docs/ndk/NDK_Users_Guide.html


/*
 *  ======== send adc values to pi server ========
 */
// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/555107?NDK-socket-creation-error
void UdpFxn(void)
{
	fdOpenSession((void *)Task_self());

	int err = NULL;
	int sockfd;
//	struct sockaddr_in servAddr;
//	socklen_t addrlen;
	char *sendBuf = "1";


/*	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd == -1)
	{
		err = fdError();
		System_printf("socket() failed: err=%d\n", err);
		System_flush();
	}
*/

	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;


	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

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
//			break;
		}

		err = NULL;
		if (connect(sockfd,rp->ai_addr, rp->ai_addrlen) < 0)
		{
			err = fdError();
			System_printf("connect() failed: err=%d\n", err);
			System_flush();
			continue;
		}
		break;
	}

/*	memset(&servAddr, 0, sizeof(servAddr));
	addrlen = sizeof(struct sockaddr_in);
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = PORT;
	servAddr.sin_addr.s_addr = inet_pton(AF_INET, SERVIP_STR, buf);		//192.168.0.136 -> c0.a8.00.88 (0xc0a80088)
*/

/*	err = NULL;
	//if (connect (sockfd, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
	if (connect(sockfd,rp->ai_addr, rp->ai_addrlen) < 0)
	{
		err = fdError();
		System_printf("connect() failed: err=%d\n", err);
		System_flush();
	}
*/
	//if ((sendto(sockfd, sendBuf, sizeof(sendBuf), 0, (struct sockaddr*)&servAddr, addrlen) < 0))
	if (sendto(sockfd, sendBuf, sizeof(sendBuf), 0, rp->ai_addr, rp->ai_addrlen) < 0)
	{
		err = fdError();
		System_printf("sendto() failed: err=%d\n", err);
		System_flush();
	}

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

	mySockThread = Task_create((Task_FuncPtr)UdpFxn, &params, NULL);

	if (!mySockThread)
	{
		System_printf("Tasc_create() failed!\n");
		System_flush();
	}
}

