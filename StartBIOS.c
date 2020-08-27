/* modeled after
 * https://github.com/bveyseloglu/All-in-One-IoT-Application-for-TI-Tiva-C/blob/master/httpget_EK_TM4C1294XL_TI/httpget.c
 */

/******************************************************************************
 * Includes
 *******************************************************************************/
#include "Board.h"

#include <ti/ndk/inc/netmain.h>

#include <stdbool.h>
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
//#include <ti/net/http/httpcli.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/hal/Hwi.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"

#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <UdpFxn.h>
#include <MicADC.h>

/* Bad Global Variables */
//Event_Handle Pi_Event;
/******************************************************************************
 * Defines and Bad Global Vars
 *******************************************************************************/
//#define HOSTNAME          "www.example.com"
//#define REQUEST_URI       "/"
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define TASKSTACKSIZE 4096
#define UDPPORT 31717

#define SW2 GPIO_PIN_1

/******************************************************************************
 * Function Bodies
 *******************************************************************************/

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */

void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
	System_printf("\n==================================\n");
	System_printf("Inside netIPAddrHook()\n");
	System_flush();

	void *udpTaskHandle = NULL;
	Error_Block eb1;
	int err;

	/* Create a HTTP task when the IP address is added */
	if (fAdd && !udpTaskHandle)
	{
		Error_init(&eb1);

		System_printf("netIPAddrHook() --> calling TaskCreate()\n");
		System_flush();

		udpTaskHandle = TaskCreate(UdpFxn, "UdpFxn\0", OS_TASKPRINORM,
		OS_TASKSTKNORM,
									0, 0, 0);
		if (udpTaskHandle == NULL)
		{
			err = fdError();
			System_printf("netIPAddrHook: TaskCreate() failed, error: %d\n",
							err);
			System_flush();
		}

		/*
		 *  Create the Task that handles UDP "connections."
		 *  arg0 will be the port that this task listens to.
		 */
		/*
		 Task_Params_init(&taskParams);
		 taskParams.stackSize = UDPTASKSTACKSIZE;
		 taskParams.priority = 1;
		 taskParams.arg0 = UDPPORT;
		 taskHandle = Task_create((Task_FuncPtr) UdpFxn, &taskParams, &eb);
		 if (taskHandle == NULL)
		 {
		 err = fdError();
		 System_printf(
		 "netIPAddrHook: Failed to create sendADCValuesToPi Task, error: %d\n",
		 err);
		 System_flush();
		 }
		 */
	}

	initializeADCnStuff();

	Task_Handle taskHandle = NULL;
	Task_Params taskParams;
	Error_Block eb2;

	Error_init(&eb2);
	Task_Params_init(&taskParams);
	taskParams.stackSize = TASKSTACKSIZE;
	taskParams.priority = 1;
	taskHandle = Task_create((Task_FuncPtr) ADC_task_fxn, &taskParams, &eb2);
	//taskHandle = Task_create((Task_FuncPtr)micADC, &taskParams, &eb2);

	if (taskHandle == NULL)
	{
		err = fdError();
		System_printf(
				"netIPAddrHook: Failed to create sendADCValuesToPi Task, error: %d\n",
				err);
		System_flush();
	}

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

	Board_initEMAC();		// is needed for receiving IP address (apparently)
	System_printf("Board_initEMAC()\n");
	System_flush();

	Semaphore_Params semParamsUDP;
	Error_Block ebs;
	Error_init(&ebs);
	Semaphore_Params_init(&semParamsUDP);
	semParamsUDP.mode = Semaphore_Mode_BINARY;
	semHandleUDP = Semaphore_create(1, &semParamsUDP, &ebs);

	if (semHandleUDP == NULL)
	{
		System_printf("Semaphore_create() fauled!\n");
		System_flush();
	}

	int sect = SemCount(semHandleUDP);
	System_printf("SemCount(semHandleUDP) = %d\n", sect);
	System_flush();


	/* Start BIOS */
	BIOS_start();

	return (0);
}
