/******************************************************************************
 * Includes
 *******************************************************************************/
#include "Board.h"

#include <ti/ndk/inc/netmain.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>

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

/******************************************************************************
 * Defines and Bad Global Vars
 *******************************************************************************/
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define UDPTASKSTACKSIZE 4096
#define UDPPORT 31717

#define SW2 GPIO_PIN_1

Semaphore_Handle semHandle;
Semaphore_Struct sem0Struct;

/******************************************************************************
 * Function Bodies
 *******************************************************************************/

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */

void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{

	System_printf("Inside netIPAddrHook()\n");
	System_flush();

	Task_Handle taskHandle;
	Task_Params taskParams;
	Error_Block eb;
	int err;

	/* Create a HTTP task when the IP address is added */
	if (fAdd && !taskHandle)
	{
		Error_init(&eb);

		/*
		 *  Create the Task that handles UDP "connections."
		 *  arg0 will be the port that this task listens to.
		 */
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
	}

	void *udpTaskHandle = NULL;

	System_printf("netIPAddrHook() --> calling TaskCreate()\n");
	System_flush();

	udpTaskHandle = TaskCreate(UdpFxn, "UdpFxn\0", OS_TASKPRINORM,
								OS_TASKSTKNORM, 0, 0, 0);
	if (udpTaskHandle == NULL)
	{
		err = fdError();
		System_printf("netIPAddrHook: TaskCreate() failed, error: %d\n", err);
		System_flush();
	}

	System_printf("setup_ADC_Task(4)\n");
	System_flush();
	setup_ADC_Task(5);
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

/*
	System_printf("setup_ADC_Task(4)\n");
	System_flush();
	setup_ADC_Task(5);
*/

	System_printf("create semaphore\n");
	System_flush();
	Semaphore_Params semParams;
	Semaphore_Params_init(&semParams);
	semParams.mode = Semaphore_Mode_BINARY;
	Semaphore_construct(&sem0Struct, 1, &semParams);
	semHandle = Semaphore_handle(&sem0Struct);
	semHandle = SemCreate(0);

	/* Start BIOS */
	BIOS_start();

	return (0);
}
