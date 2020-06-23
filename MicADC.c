/*
 * MicADC.c
 *
 *  Created on: 22 Jun 2020
 *      Author: noa
 */


/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* TI-RTOS Header files */
#include <ti/drivers/EMAC.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SDSPI.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
// #include <ti/drivers/USBMSCHFatFs.h>
// #include <ti/drivers/Watchdog.h>
// #include <ti/drivers/WiFi.h>

/* Board Header file */
#include "Board.h"

/* All the C Utilities */
#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <unistd.h>

/* Driverlib headers */
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include "driverlib/interrupt.h"
#include <driverlib/adc.h>

#define TASKSTACKSIZE   512

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */
Void heartBeatFxn(UArg arg0, UArg arg1)
{
    while (1) {
        Task_sleep((unsigned int)arg0);
        GPIO_toggle(Board_LED0);
    }
}

static void configMicADC(void)
{
    Task_Params taskParams;
        /* Call board init functions */
        Board_initGeneral();
        // Board_initEMAC();
        Board_initGPIO();
        Board_initI2C();

        //Aktivieren der GPIO Ports, an denen das ADC Interface angeschlossen sein soll
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        //Aktivieren gewuenschter ADC Peripherals
        SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

        //Setzen von Treiberstärke, Richtung für die gewünschte ADS Pins (???)
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

        /*======= driverlib userguide 4.3 prog example =======*/
        //Config des Sequencetriggers
        ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);

        //Config ein/mehr Steps innerhalb Sequence
        ADCSequenceStepConfigure(ADC0_BASE, 0 , 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0); //??? Reihenfolge der ADC_CTL-xxx???

        //Aktivieren der Sequence
        ADCSequenceEnable(ADC0_BASE, 0);

        ADCIntClear(ADC0_BASE, 0);

        //registrieren ISR
    //  ADCIntRegister();


        //Aktivieren Interrupt
    //  ADCIntEnable();


    //  ADCSequenceDataGet(ADC0_BASE, 0, &ui32Value);
}

static void MicADC(UArg arg0)
{
    configMicADC();

    uint32_t value[7];

    int i = 0;
    int p;
    while(i < 10)
    {
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

        Task_slepp(500);
        Event_post(UART_Event, Event_Id_01);
        i++;
    }



}

static void heartbeatFxn(void)
{
    /* Construct heartBeat Task  thread */
    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)heartBeatFxn, &taskParams, NULL);

     /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */

    System_flush();
}
