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
#include <MicADC.h>

/******************************************************************************
 * Defines and Bad Global Vars
 *******************************************************************************/
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define UDPTASKSTACKSIZE 4096
#define UDPPORT 31717

#define SW2 GPIO_PIN_1

Semaphore_Handle semHandle;
Semaphore_Params semParams;
Semaphore_Struct sem0Struct;

Event_Handle UDP_Event;

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

	Task_Handle taskHandle = NULL;

	/* Create a HTTP task when the IP address is added */
	if (fAdd && !taskHandle)
	{
		System_printf("netIPAddrHook() --> calling setup_UDP_Task()\n");
		System_flush();
		setup_UDP_Task(4);
	}

	System_printf("setup_ADC_Task()\n");
	System_flush();
	setup_ADC_Task(3);
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

	Error_Block eb;
	Error_init(&eb);
	Semaphore_Params_init(&semParams);
	semHandle = Semaphore_create(0, &semParams, &eb);

	if (semHandle == NULL)
	{
		System_printf("Semaphore_create() fauled!\n");
		System_flush();
	}

	/* Start BIOS */
	BIOS_start();

	return (0);
}
