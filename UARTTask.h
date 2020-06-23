/*
 * UARTTask.h
 *
 *  Created on: 23 Jun 2020
 *      Author: noa
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Example/Board Header files */
#include "Board.h"
#include <stdbool.h>
#include <stdint.h>

/* Drivers Header files - fall back to driverlib for gpio*/
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>
#include <ti/sysbios/knl/Event.h>

#ifndef UARTTASK_H_
#define UARTTASK_H_

extern Event_Handle UART_Event;

void UARTFxn(UArg arg0, UArg arg1);
void create_UART_event();
int setup_UART_Task(int prio);



#endif /* UARTTASK_H_ */
