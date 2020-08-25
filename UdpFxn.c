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
#include <ti/sysbios/knl/Event.h>
//#include <ti/sysbios/knl/Semaphore.h>

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
#define BIND
#define REC_SERVREPLY
#define TASKSTACKSIZE 4096
#define UDPPORT 31717

const char *PORT_STR = "31717";
const char *SERVIP_STR = "192.168.0.136";
uint32_t PORT = 31717;
uint32_t MAXBUF = 1024;
Event_Handle UDP_Event;

/******************************************************************************
 * Function Bodies
 *******************************************************************************/
/*
 *  ======== send adc values to pi server ========
 */
void UdpFxn(UArg arg0, UArg arg1)
{
	while (true)
	{
		System_printf("Calling Semaphore_pend()\n");
		System_flush();
		Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

		fdOpenSession((void*) Task_self());

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
#ifdef BIND
		// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/354644?NDK-UDP-Client-Issue
		// bind our socket to a particular port. Must bind else server reply drops!
		struct sockaddr_in bindAddr;
		memset(&bindAddr, 0, sizeof(bindAddr));
		bindAddr.sin_family = AF_INET;
		bindAddr.sin_addr.s_addr = INADDR_ANY;
		bindAddr.sin_port = htons(1025);	// I guess just a random port?

		err = NULL;
		if (bind(sockfd, (struct sockaddr*) &bindAddr, sizeof(bindAddr)) < 0)
		{
			err = fdError();
			System_printf("bind() failed, err = %d\n", err);
			System_flush();
		}
#endif
//<<<-------------------------------------------------------------<<<

//>>>------------------------------------------------------------->>>
		struct sockaddr_in servAddr;

		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_UNSPEC;
		servAddr.sin_port = htons(PORT);
		inet_aton("192.168.0.136", &servAddr.sin_addr);

		System_printf("servAddr.sin_addr.s_addr = %d\n",
						servAddr.sin_addr.s_addr);
		System_flush();

		char *sendBuf = "105";
		int bytesSent = NULL;
		err = NULL;
		int sendct = 1;

		do
		{
			bytesSent = sendto(sockfd, sendBuf, sizeof(sendBuf), 0,
								(struct sockaddr*) &servAddr, sizeof(servAddr));
			if (bytesSent < 0)
			{
				err = fdError();
				System_printf("sendto() of %s failed: err=%d\n", sendBuf, err);
				System_flush();
				sendct++;
				if (sendct == 7)
				{
					System_printf("sendto() failed for %d tries\n", sendct);
					System_flush();
					break;
				}
			}
			else if (bytesSent == sizeof(sendBuf))
			{
				System_printf("sendto() of \"%s\": bytesSent=%d, bufSize=%d\n",
								sendBuf, bytesSent, sizeof(sendBuf));
				System_flush();
			}
		}
		while (bytesSent < 0);
//<<<-------------------------------------------------------------<<<

//>>>------------------------------------------------------------->>>

#ifdef REC_SERVREPLY

		char *pBuf;
		HANDLE hBuffer;
		err = NULL;
		int retval = NULL;
		int recct = 1;

		struct sockaddr from;
		memset(&from, 0, sizeof(from));
		int addrlen_from;

		do
		{
			//retval = (int)recvnc(sockfd, (void **)&pBuf, MSG_DONTWAIT, &hBuffer);
			retval = (int) recvncfrom(sockfd, (void**) &pBuf, MSG_DONTWAIT,
										&from, &addrlen_from, &hBuffer);
			if (retval < 0)
			{
//				err = fdError();
//				System_printf("#%d recvnc(): err=%d [35 EWOULDBLOCK]\n", recct,	err);
//				System_flush();
				recct++;
			}
			else
			{
//			char piip[1024];
//			int piip_len = 1024;
//			inet_ntop(AF_INET, from.sa_data, piip, piip_len);
//			System_printf("#%d: recvnc() received %d bytes from %s\n", recctdn, retval, piip);
				System_printf("#%d: recvnc() received %d bytes from %d\n",
								recct, retval, from.sa_data);
				System_flush();

				recvncfree(hBuffer);
			}
//			Task_sleep(1000);
			if (recct == 7)
			{
				System_printf("recvnc() failed for %d tries\n", recct);
				System_flush();
				break;
			}

		}
		while (retval < 0);

#endif
//<<<-------------------------------------------------------------<<<

		System_printf("Calling fdClose() and fdCloseSession()\n");
		System_flush();
		fdClose(sockfd);
		fdCloseSession((void*) Task_self());
	}
}

void create_UDP_event(void)
{
	System_printf("Inside create_UDP_event\n");
	System_flush();

	Error_Block eb;
	Error_init(&eb);
	UDP_Event = Event_create(NULL, &eb);
	if (UDP_Event == NULL)
	{
		System_abort("Failed to create UDP event");
	}
}

int setup_UDP_Task(int prio)
{
	System_printf("Inside setup_UDP_Task()\n");
	System_flush();

	Task_Params taskParams;
	Task_Handle UDPHandle;
	Error_Block eb;
	int err;

	Error_init(&eb);
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.priority = prio;		//15: highest priority
	taskParams.arg0 = UDPPORT;
	UDPHandle = Task_create((Task_FuncPtr) UdpFxn, &taskParams, &eb);
	if (UDPHandle == NULL)
	{
		err = fdError();
		System_printf("Failed to setup UDP task, error: %d\n", err);
		System_flush();
	}

	create_UDP_event();

	return 0;
}

