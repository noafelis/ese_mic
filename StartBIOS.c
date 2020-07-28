/* modeled after
 * https://github.com/bveyseloglu/All-in-One-IoT-Application-for-TI-Tiva-C/blob/master/httpget_EK_TM4C1294XL_TI/httpget.c
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
#include <unistd.h>
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

#include "MicUART.h"
#include "MicADC.h"

/* Bad Global Variables */
Event_Handle UART_Event;


/******************************************************************************
* Defines
*******************************************************************************/
#define HOSTNAME          "www.example.com"
#define REQUEST_URI       "/"
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define HTTPTASKSTACKSIZE 4096

double noiseLvlAvg = 0;
double noiseLvlValues[7];
uint32_t ADCValues[7];
int lastNoiseIndex = 0;

/******************************************************************************
* Function Bodies
*******************************************************************************/

/*
 *  ======== send adc values to pi server ========
 */
void sendADCValuesToPi(double noiseLvlAvg)
{
	int sockd;
	struct sockaddr_in piServerAddr;
	char sendBuffer[3072];
	int round = 1;

	// Construct message to be sent to pi server
	uint32_t piStringLen = 0;
	char piString[100];
	sprintf(piString, "Ambient Noise Level: %.2lf\n", noiseLvlAvg);

	sprintf(sendBuffer, "%s", piString);

	// Create stream socket using TCP
	//TODO change to UDP?
	if ((sockd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("create socket failed\n");
    	exit(1);
	}


	// Construct server address
	memset(&piServerAddr, 0, sizeof(piServerAddr));
	piServerAddr.sin_family	= AF_INET;
	piServerAddr.sin_port = htons(5011);					//TODO find out correct pi server port
	inet_pton(AF_INET, "127.0.0.1", &(piServerAddr.sin_addr));	//TODO change IP to pi server IP

	// Connect to pi server
	if (connect(sockd, (struct sockaddr *) &piServerAddr, sizeof(piServerAddr)) < 0)
	{
		perror("connect to server failed\n");
    	exit(1);
	}

	piStringLen = strlen(piString);
	if (piStringLen < 2)
	{
		piStringLen = 3000;		//TODO why?
	}

	// send string to pi server

	//UDP:	bytesSent = sendto(socketFd, buffer, bufferSize, flags, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	while (round >= 0)
	{
		if (send(sockd, sendBuffer, piStringLen, 0) != piStringLen)
		{
			perror("send() error: different nr of bytes than expected");
		}
		round--;
	}

	close(sockd);
}




/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */

void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
	System_printf("inside netIPAddrHook()\n");
	System_flush();
}


/*
 *  ======== main ========
 */
int main(void)
{
	/* Call board init functions */
	Board_initGeneral();
	System_printf("Board_initGeneral()\n");
	System_flush();

	Board_initGPIO();
	System_printf("Board_initGPIO()\n");
	System_flush();

//	Board_initEMAC();
//	System_printf("Board_initEMAC()\n");
//	System_flush();

	setup_UART_Task(15);
	setup_ADC_Task();


	/* Start BIOS */
	BIOS_start();

	return (0);
}
