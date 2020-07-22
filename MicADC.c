/*
 * MicADC.c
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */

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
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
//#include <ti/drivers/SDSPI.h>
//#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
#include <ti/drivers/WiFi.h>

/* Board Header file */
#include "Board.h"
#include <EK_TM4C1294XL.h>

/* All the C Utilities */
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <inc/hw_memmap.h>
#include <inc/tm4c1294ncpdt.h>
#include <inc/hw_types.h>

/* Driverlib headers */
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/adc.h>

/* Application headers */
#include "MicADC.h"

//*************************************************************************
/* Defines */
#define TASKSTACKSIZE   512

#define USRBUTTON GPIO_PORTJ_BASE
#define SW1 GPIO_PIN_0
#define SW2 GPIO_PIN_1

//*************************************************************************
/* Global vars */
Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];
Event_Handle ADC_Event;

//*************************************************************************
/* Fctn declarations */
interrupt void micISR(void);
static void micADC(void);

//*************************************************************************

void initializeADCnStuff(void)
{
	/* Call board init functions */
//	Board_initGeneral();
	// Board_initEMAC();
//	Board_initGPIO();
	Board_initI2C();

	//Aktivieren der GPIO Ports, an denen das ADC Interface angeschlossen sein soll
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	//*************************************************************************
	/*=========================== USRSW config ===========================*/
	uint32_t ui32Strength;
	uint32_t ui32PinType;

	/* Activate GPIO ports */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);    // USRSWITCH Port

	/* Set pin 0 of GPIO port J to digital input */
	GPIOPinTypeGPIOInput(USRBUTTON, SW1);
	/* Set pin 1 of GPIO port J to digital input */
	GPIOPinTypeGPIOInput(USRBUTTON, SW2);

	/* Enable pull-up for button on pin 0 of port J damit er auf Druck reagiert und strom tut */
	GPIOPadConfigGet(USRBUTTON, SW1, &ui32Strength, &ui32PinType);
	GPIOPadConfigSet(USRBUTTON, SW1, ui32Strength, GPIO_PIN_TYPE_STD_WPU);

	/* Enable pull-up for button on pin 1 of port J damit er auf Druck reagiert und strom tut */
	GPIOPadConfigGet(USRBUTTON, SW2, &ui32Strength, &ui32PinType);
	GPIOPadConfigSet(USRBUTTON, SW2, ui32Strength, GPIO_PIN_TYPE_STD_WPU);

	//*************************************************************************
	/*=========================== ADC config ===========================*/
	//Aktivieren gewuenschter ADC Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	//Setzen von Treiberstaerke, Richtung f die gewuenschten ADC Pins (???)
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);

	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
	{
	}

	//Bit 0 and bit 1 of RCGCADC register are used to enable the clock to ADC0 and ADC1 modules, respectively.
	//The RCGCADC is part of the System Control register and is located at base address of 0x400F.E000 with offset 0x638
	//Konfigurieren der Taktquelle, aus der sich der interne ADC Takt ableiten soll
	ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_HALF, 24);  //just used any values from TivaWare Periph Driver Lib Userguide p25

	//jedes der 2 ADC Module verfügt über mehrere Sequencer, die über mehrere Steps programmiert werden können
	//Deaktiveren eines Sequencers
	ADCSequenceDisable(ADC0_BASE, 0);

	// ADCACTSS (ADC Active Sample Sequencer) to enable the SS0. When bit 0 (ASEN0) is set to 1 the SS0 is enabled.

	//Enables a GPIO pin as a trigger to start an ADC capture.
	GPIOADCTriggerEnable(USRBUTTON, SW1);

	/*======= driverlib userguide 4.3 prog example =======*/
	//Config des Sequencetriggers
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_EXTERNAL, 0);

	//Config ein/mehr Steps innerhalb Sequence
	ADCSequenceStepConfigure(ADC0_BASE, 0 , 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0); //??? Reihenfolge der ADC_CTL-xxx???

	//Aktivieren der Sequence
	ADCSequenceEnable(ADC0_BASE, 0);

	ADCIntClear(ADC0_BASE, 0);

	//registrieren ISR
	ADCIntRegister(ADC0_BASE, 0, micADC);

	//Aktivieren Interrupt
	ADCIntEnable(ADC0_BASE, 0);

	uint32_t buffer;
	ADCSequenceDataGet(ADC0_BASE, 0, &buffer);


}



static void micADC(void)
{
	uint32_t value[7];

	int p;
	ADCProcessorTrigger(ADC0_BASE, 0);
	ADCIntClear(ADC0_BASE, 0);

	ADCSequenceDataGet(ADC0_BASE, 0, value);

	while(!ADCIntStatus(ADC0_BASE, 0, false))
	{
	}



	for (p = 0; p <= 8; p++)
	{
		System_printf("AIN9, reading %d = %4d\n", p, value[p]);
		System_flush();
	}

//	Task_sleep(500);
//	Event_post(UART_Event, Event_Id_01);
}


void create_ADC_event(void)
{
	Error_Block eb;
	Error_init(&eb);
	ADC_Event = Event_create(NULL, &eb);
	if (ADC_Event == NULL)
	{
		System_abort("Failed to create event");
	}
}


int setup_ADC_Task(void)
{
	Task_Params taskParams;
	Task_Handle ADCHandle;
	Error_Block eb;

	create_ADC_event();
	initializeADCnStuff();

	Error_init(&eb);
	Task_Params_init(&taskParams);
	taskParams.arg0 = NULL;
	taskParams.stackSize = 2048;
	taskParams.priority = 15;		//15: highest priority
	ADCHandle = Task_create((Task_FuncPtr)micADC, &taskParams, &eb);
	if (ADCHandle == NULL)
	{
		System_abort("Error creating ADC task");
	}
	return 0;

}
