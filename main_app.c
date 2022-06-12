/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== empty.c ========
 */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>

#include <LCDdisplayClient.h>
#include <TempController.h>
#include <TempData.h>
#include <utils.h>
#include <string.h>
#include <driverlib.h>
#include <shared_vars.h>


/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
// #include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>
#include <TempSensor.h>

/* Board Header file */
#include "Board.h"

Display_Handle disp_hdl;

void init_Clock_System_module()
{
    //Configures CS_MCLK clock signal with DCO clock source (3MHz)
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    //Configures CS_SMCLK clock signal with DCO clock source (3MHz)
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

void init_Timer32_module()
{
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
    TIMER32_PERIODIC_MODE);
}

void init_Display(void)
{
    Display_Params params;
    Display_Params_init(&params);
    disp_hdl = Display_open(0, NULL);
    Display_clear(disp_hdl); //Clear previous execution output of the terminal
}


/*TODO: en Keypad.h, definir interrupciones de los botones + y -. La ISR no tiene parametros, entonces no le puedo pasar el estado del keypad.
 * Alternativa 1: Hacer una variable global Keypad* en el fichero Keypad con el estado del keypad. Así la ISR puede acceder a la info.
 *                Desde la ISR hacer lo que hace newData().
 *
 *                Añadir en DisplayClient_acceptTargetTemp() un show() de la temperatura target
 * Alternativa 2: hacer un thread a parte que gestione el Keypad y que tenga un puntero a TempController y DisplayClient para enviarles
 *                la nueva temperatura target.
 *
 */

/*TODO: En las funciones XXX_acccept(), no le mola que hagamos un show() porque estamos dentro de la ISR y por tema de interrupciones o yo que se
 *      no va bien el tema de I2C.
 *      Hacer que la ISR ponga active algun flag para notificar  que se ha de actualizar el valor de la temperatura target.
 *
 * */

/*
 *  ======== mainThread ========
 */
void* mainThread(void *arg0)
{
    WDT_A_holdTimer();
    /* Initialize clock system module */
    init_Clock_System_module();

    /* Initialize Timer32 module */
    init_Timer32_module();
    init_Display();
    uint32_t smclk_freq_ = CS_getSMCLK();

    /* Call driver init functions */
    GPIO_init();
    I2C_init();
    // SPI_init();
    //UART_init();
    //Watchdog_init();

    Display_printf(disp_hdl, 0, 0, "main_app\n");

    /*Creates target temp variable*/
    //TempData* tgtTemp;
    //tgtTemp = TempData_create();

    /*Creates the keypad*/
    Keypad* kp;
    kp = Keypad_create();
    Keypad_init(kp);

    /*Create a TempSensor (publisher of temp and humidity) */
    TempSensor *ts;
    ts = TempSensor_create();

    /*Create DisplayClient and TempController (subscribers of TempSensor) */
    DisplayClient *dc;
    dc = DisplayClient_create();
    DisplayClient_init(dc, ts, kp);
    DisplayClient_register(dc); //subscribe to TempSensor & Keypad

    TempController *tc;
    tc = TempController_create();
    TempController_init(tc, ts, kp);
    TempController_register(tc); //subscribe to TempSensor & Keypad

    //DisplayClient_show(dc);

    while (1)
    {
        TempSensor_readTemp(ts);
        DisplayClient_showTempSensed(dc);
        sleep(15); //Read temperature every 2 minutes
    }
}

/* ALTERNATIVA PARA PRINTAR TEXTO POR CONSOLA
 const eUSCI_UART_ConfigV1 uartConfig = {
 EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
 78, // BRDIV = 78 ==> 9600 BAUDS
 2, // UCxBRF = 2
 0, // UCxBRS = 0
 EUSCI_A_UART_NO_PARITY, // No Parity
 EUSCI_A_UART_LSB_FIRST, // MSB First
 EUSCI_A_UART_ONE_STOP_BIT, // One stop bit
 EUSCI_A_UART_MODE, // UART mode
 EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION // Oversampling
 };

 UART_initModule(EUSCI_A0_BASE, &uartConfig);
 // Enable UART module
 UART_enableModule(EUSCI_A0_BASE);
 // Enabling interrupts
 UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
 Interrupt_enableInterrupt(INT_EUSCIA0);
 Interrupt_enableSleepOnIsrExit();
 Interrupt_enableMaster();

 const char *text = "hello world!";
 int i;
 for (i=0; i<strlen(text); ++i){
 UART_transmitData(EUSCI_A0_BASE, text[i]);
 }*/
