#ifndef UART_TASK_H_
#define UART_TASK_H_

#include <stdbool.h>
#include <stdint.h>
#include <xdc/std.h>


void UARTFxn(UArg arg0, UArg arg1);

int setup_UART_Task(int prio);

#endif
