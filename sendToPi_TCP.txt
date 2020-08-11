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

#include "MicADC.h"
#include "SendToPi.h"

/* Bad Global Variables */
//Event_Handle Pi_Event;

/******************************************************************************
* Defines and Bad Global Vars
*******************************************************************************/
#define HOSTNAME          "www.example.com"
#define REQUEST_URI       "/"
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096

#define SW2 GPIO_PIN_1

/*
double noiseLvlAvg = 0;
double noiseLvlValues[7];
uint32_t ADCValues[7];
int lastNoiseIndex = 0;
*/
const char *RPI_IP = "192.168.0.136";
const char *PORT = "31717";
uint32_t MAXBUF = 1024;

/******************************************************************************
* Function Bodies
*******************************************************************************/

/*
 *  ======== send adc values to pi server ========
 */
void sendADCValuesToPi(void)
{
	// https://e2e.ti.com/support/legacy_forums/embedded/tirtos/f/355/t/413759

	int sockfd;
	//struct sockaddr_in piServerAddr;
	char sendBuffer[3072];
	//int round = 1;
	struct addrinfo hints;
	struct addrinfo *results = NULL;
	struct addrinfo *servaddr = NULL;
	int value;
	char* buffer = NULL;
	//int status = EXIT_SUCCESS;
	uint32_t buffSize = MAXBUF;

	//**************************
	// the following part is from tcpSendReceive.c (tirtos examples)
	//**************************
	buffer = malloc(buffSize);
	if (buffer == NULL)
	{
		System_printf("malloc failed\n");
		System_flush();
	}
	memset(buffer, 0, buffSize);

	/* initialize sockets environment */
//	socketsStartup();

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
	//node: IP addr of network host as string. service: port nr of network host as string.
	if ((value = getaddrinfo(RPI_IP, PORT, &hints, &results)) < 0)
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

	System_printf("struct addrinfo: %d, %d, %d, %d\n\n", results->ai_flags, results->ai_family, results->ai_socktype, results->ai_protocol);
	System_flush();


	/* create socket. ai_family determined for us via getaddrinfo() call */
	//if ((sockfd = socket(results->ai_family, SOCK_STREAM, 0x800)) < 0)

	for (servaddr = results; servaddr != NULL; servaddr = servaddr->ai_next)
	{
		if ((sockfd = socket(results->ai_family, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			System_printf("socket failed: 0x%x\n", fdError());
			System_flush();
			continue;
		}
		/* connect. ai_addr set to AF_INET or AF_INET6 by getaddrinfo() call */
		if (connect(sockfd, results->ai_addr, results->ai_addrlen) < 0)
		{
			close(sockfd);
			System_printf("connection failed: 0x%x\n", fdError());
			System_flush();
			continue;
		}
		break;
	}

	if (servaddr == NULL)
	{
		System_printf("client failed to connect\n");
		System_flush();
	}




	System_printf("trying to send stuff to pi now\n");
	System_flush();

	// Construct message to be sent to pi server
	uint32_t piStringLen = 0;
	char piString[100];
	//sprintf(piString, "Ambient Noise Level: %.2lf\n", noiseLvlAvg);
	sprintf(piString, "1");

	sprintf(sendBuffer, "%s", piString);

	piStringLen = strlen(piString);
	if (piStringLen < 2)
	{
		piStringLen = 3000;		//TODO why?
	}

	int bytesSent = send(sockfd, piString, buffSize, 0);
	if (bytesSent != buffSize)
	{
		System_printf("bytesSent != buffsize\n");
		System_flush();
	}

	//UDP:	bytesSent = sendto(socketFd, buffer, bufferSize, flags, (struct sockaddr*)&serverAddr, sizeof(serverAddr));


	//TODO close all the sockets and stuff.
}

/*
void create_Pi_event(void)
{
	Error_Block eb;
	Error_init(&eb);
	Pi_Event = Event_create(NULL, &eb);
	if (Pi_Event == NULL)
	{
		System_abort("Failed to create event");
	}
}


int setup_Pi_Task(void)
{
	Task_Params taskParams;
	Task_Handle piHandle;
	Error_Block eb;

	create_Pi_event();

	Error_init(&eb);
	Task_Params_init(&taskParams);
	taskParams.arg0 = NULL;
	taskParams.stackSize = 2048;
	taskParams.priority = 15;		//15: highest priority
	piHandle = Task_create((Task_FuncPtr)sendADCValuesToPi, &taskParams, &eb);
	if (piHandle == NULL)
	{
		System_abort("Error sending things to pi");
	}
	return 0;
}
*/
