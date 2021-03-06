/******************************************************************************
 * Includes
 *******************************************************************************/
#include <ti/ndk/inc/netmain.h>

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
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/hal/Hwi.h>

/* TI-RTOS Header files */
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/WiFi.h>
#include <ti/drivers/ports/SemaphoreP.h>

/* Board Header file */
#include "Board.h"
#include <EK_TM4C1294XL.h>

/* All the C Utilities */
#include <stdbool.h>
#include <stdint.h>

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
#include <MicADC.h>
#include <UdpFxn.h>
#include <shared_resources.h>

//*************************************************************************

MsgObj msg;
_Bool gotADCInterrupt = false;
//*************************************************************************

void initializeADCnStuff(void)
{
	System_printf("Inside initializeADCnStuff()\n");
	System_flush();

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

	/* Enable pull-up for button on pin 0 of port J damit er auf Druck reagiert und strom tut */
	GPIOPadConfigGet(USRBUTTON, SW1, &ui32Strength, &ui32PinType);
	GPIOPadConfigSet(USRBUTTON, SW1, ui32Strength, GPIO_PIN_TYPE_STD_WPU);

	//*************************************************************************
	/*=========================== ADC config ===========================*/
	//Aktivieren gewuenschter ADC Peripherals
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	//Setzen von Treiberstaerke, Richtung f die gewuenschten ADC Pins (???)
	GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_4);

	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
	{
	}

	//Bit 0 and bit 1 of RCGCADC register are used to enable the clock to ADC0 and ADC1 modules, respectively.
	//The RCGCADC is part of the System Control register and is located at base address of 0x400F.E000 with offset 0x638
	//Konfigurieren der Taktquelle, aus der sich der interne ADC Takt ableiten soll
	ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_HALF, 24); //just used any values from TivaWare Periph Driver Lib Userguide p25

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
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0,
	ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);

	//Aktivieren der Sequence
	ADCSequenceEnable(ADC0_BASE, 0);

	ADCIntClear(ADC0_BASE, 0);

	//registrieren ISR
	ADCIntRegister(ADC0_BASE, 0, ADC_interrupt_handler);

	//Aktivieren Interrupt
	ADCIntEnable(ADC0_BASE, 0);

	uint32_t buffer;
	ADCSequenceDataGet(ADC0_BASE, 0, &buffer);
}

void ADC_interrupt_handler()
{
	ADCIntClear(ADC0_BASE, 0);
	gotADCInterrupt = true;
}

void micADC(void)
{
	uint32_t value[7];

	int p;
	ADCProcessorTrigger(ADC0_BASE, 0);
	ADCSequenceDataGet(ADC0_BASE, 0, value);

	int i = 1;

	while (!gotADCInterrupt)
	{
		// wait for usrbutton1 to be pressed, creating interrupt
		if (i == 1)
		{
			System_printf("\n>>>------------------------------->>>\n");
			System_printf("Inside micADC()\n");
			System_flush();
			i--;
		}
	}

	int total_readings = 0;
	for (p = 0; p <= 8; p++)
	{
		System_printf("AIN9, reading %d = %4d\n", p, value[p]);
		System_flush();
		total_readings = total_readings + value[p];
	}
	gotADCInterrupt = false;

	int mean_readings = total_readings / 8;

	msg.id = 1;
	msg.val = mean_readings;

	System_printf("mean_readings: %d, msg.val: %d\n", mean_readings,
					msg.val);
	System_flush();

	if (mean_readings == 0)
	{
		System_printf("no valid readings!\n");
		System_flush();
	}

	if (Mailbox_post(mbxHandle, &msg, BIOS_WAIT_FOREVER))
	{
	}
	else
	{
		System_printf("Mailbox Write Failed: ID = %d and Value = '%c'.\n",
						msg.id, msg.val);
		System_flush();
	}
}

void ADC_task_fxn(UArg arg0, UArg arg1)
{
	while (1)
	{
		micADC();
		System_printf("--------------------\n");
		System_flush();

		Task_sleep(100);
	}
}
