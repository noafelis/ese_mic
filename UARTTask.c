/*
 * UARTTask.c
 *
 *  Created on: 23 Jun 2020
 *      Author: noa
 */
/*
 *  ======== UART_Task.c ========
 *  Author: Michael Kramer / Matthias Wenzl
 */
#define TARGET_IS_TM4C129_RA2

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inc/hw_memmap.h>
#include <ctype.h>
#include <string.h>

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/hal/Hwi.h>

/* TI-RTOS Header files */
#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>

/* Driverlib headers */
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include "driverlib/interrupt.h"

/*Board Header files */
#include <Board.h>
#include <EK_TM4C1294XL.h>

/* Application headers */
#include "Uarttask.h"

Event_Handle UART_Event;

#define registerdebug


void UARTFxn(UArg arg0, UArg arg1)
{
	UART_Handle uart;
	UART_Params uartParams;
	uint8_t events;

	/* Create a UART with data processing off. */
	UART_Params_init(&uartParams);
	uartParams.writeDataMode = UART_DATA_BINARY;
	uartParams.readDataMode = UART_DATA_BINARY;
	uartParams.readReturnMode = UART_RETURN_FULL;
	uartParams.readEcho = UART_ECHO_OFF;
	uartParams.baudRate = 9600;
	uart = UART_open(Board_UART0, &uartParams);

	if (uart == NULL)
	{
		System_abort("Error opening the UART");
	}

	/* Loop forever echoing */
	while (1)
	{
		events = Event_pend(UART_Event, Event_Id_NONE, Event_Id_01, BIOS_WAIT_FOREVER);

		GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 1);

		if((events && Event_Id_01) == Event_Id_01)
		{
			char uartTest[] = "\n\nThis is a test!\n\n";
			UART_write(uart, uartTest, strlen(uartTest));
		}
	}
}


void create_UART_event()
{
	Error_Block eb;
	Error_init(&eb);
	UART_Event = Event_create(NULL, &eb);
	if (UART_Event == NULL)
	{
		System_abort("Failed to create event");
	}
}


/**
 * \brief Setup UART task
 */
int setup_UART_Task(int prio)
{
	Task_Params taskUARTParams;
	Task_Handle UARThandle;
	Error_Block eb;

	create_UART_event();

	// Enable and configure the peripherals used by the UART0
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	GPIOPinConfigure(GPIO_PA0_U0RX);
	GPIOPinConfigure(GPIO_PA1_U0TX);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UART_init();

	// Setup PortN LED1 activity signaling
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);

	Error_init(&eb);
	Task_Params_init(&taskUARTParams);
	taskUARTParams.stackSize = 2048;    //stack in bytes; reducing stacksize leads to overflow
	taskUARTParams.priority = prio;     // 0-15 (15 is highest priority by default -> see RTOS Task configuration)
	UARThandle = Task_create((Task_FuncPtr)UARTFxn, &taskUARTParams, &eb);
	if (UARThandle == NULL)
	{
		System_abort("TaskUART create failed");
	}

	return 0;
}


