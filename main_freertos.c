/*
 * Copyright (c) 2016-2019, Texas Instruments Incorporated
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
 *  ======== main_freertos.c ========
 */
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>

#ifdef __ICCARM__
#include <DLib_Threads.h>
#endif

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* Driver configuration */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>

/* Board Header file */
#include "Board.h"

#include <driverlib.h>
#include <utils.h>
#include <string.h>
#include <shared_vars.h>
#include "InterThreadQueues.h"
#include "ThreadsArgStruct.h"

extern void* displayConsoleThread(void *arg0);
extern void* displayLCDThread(void *arg0);
extern void* temperatureReadingThread(void *arg0);
extern void* keypadThread(void *arg0);
extern void* temperatureControllerThread(void *arg0);

QueueHandle_t qTReadToLCD;
QueueHandle_t qTReadToTCtrl;
QueueHandle_t qDispConsole;
QueueHandle_t qKeypadToTCtrl;
QueueHandle_t qTCtrlToLCD;

/* Stack size in bytes */
#define THREADSTACKSIZE   4096

#define QUEUE_SIZE 10   //Max num of elements in the queue

Display_Handle disp_hdl;

/*
 * THERMOSTAT MODULES CLOCK SOURCES AND DIVIDERS
 *
 * On MCU init
 *
 *  - Temp. sensor      -> Capture Module   -> TIMER_A1     -> SMCLK / 8
 *  - LCD Screen        -> I2C module       -> EUSCI_B0     -> SMCLK / 1
 *  - Console display   -> UART Module      -> EUSCI_A0     -> SMCLK / 1
 *  - delay() timer     -> Timer module     -> TIMER32_0    -> MCLK / 1 (only supports MCLK src)
 *
 * On some LPM
 */

void init_Clock_System_module()
{
    /*******************************************************************************************/
    /***************************** OSCILLATORS CONFIGURATION ***********************************/
    /*******************************************************************************************/

    /* Before we start we have to change VCORE to 1 to support the 48MHz frequency
     * SI NO, SE QUEDA EL MICRO BRICKEADO Y HAY QUE HACER UN RESET DE FABRICA DEL FW EN LA FLASH
     * USANDO EL SCRIPT QUE HAY EN EL DOCUMENTO "MSP-EXP432P401R_LaunchPad_Evaluation_Kit" EN
     * LA PAGINA 28*/
    FPU_enableModule();
    PCM_setCoreVoltageLevel(PCM_AM_LDO_VCORE1);
    FlashCtl_setWaitState(FLASH_BANK0, 1);
    FlashCtl_setWaitState(FLASH_BANK1, 1);

    /*: Internal digitally controlled oscillator (DCO) with programmable frequencies and 3-MHz
     frequency by default.*/
    //CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_1_5);
    CS_setDCOFrequency(CS_48MHZ);
    /* Internal, low-power low-frequency oscillator (REFO) with selectable 32.768-kHz or 128-
     kHz typical frequencies*/
    CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    //MODCLK: Internal low-power oscillator with 25-MHz typical frequency.
    //SYSOSC: Internal oscillator with 5MHz typical frequency.
    //VLOCLK: Internal very-low-power low-frequency oscillator (VLO) with 9.4-kHz typical frequency
    FPU_disableModule(); //Module used to calculate DCO frequency
    /*******************************************************************************************/
    /***************************** CLOCK SIGNALS CONFIGURATION *********************************/
    /*******************************************************************************************/

    /*: Master clock. MCLK is software selectable as LFXTCLK, VLOCLK, REFOCLK, DCOCLK,
     MODCLK, or HFXTCLK. MCLK can be divided by 1, 2, 4, 8, 16, 32, 64, or 128. MCLK is used by the
     CPU and peripheral module interfaces, as well as, used directly by some peripheral modules.*/
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    /*Subsystem master clock. HSMCLK is software selectable as LFXTCLK, VLOCLK,
     REFOCLK, DCOCLK, MODCLK, HFXTCLK. HSMCLK can be divided by 1, 2, 4, 8, 16, 32, 64, or 128.
     HSMCLK is software selectable by individual peripheral modules.*/
    CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    /*Low-speed subsystem master clock. SMCLK uses the HSMCLK clock resource selection for
     its clock resource. SMCLK can be divided independently from HSMCLK by 1, 2, 4, 8, 16, 32, 64, or
     128. SMCLK is limited in frequency to half of the rated maximum frequency of HSMCLK. SMCLK is
     software selectable by individual peripheral modules.*/
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    /*Auxiliary clock. ACLK is software selectable as LFXTCLK, VLOCLK, or REFOCLK. ACLK can
     be divided by 1, 2, 4, 8, 16, 32, 64 or 128. ACLK is software selectable by individual peripheral
     modules. ACLK is restricted to maximum frequency of operation of 128 kHz.*/
    CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
    /*Low-speed backup domain clock. BCLK is software selectable as LFXTCLK and REFOCLK and
     it is primarily used in the backup domain. BCLK is restricted to a maximum frequency of 32.768 kHz.*/
    CS_initClockSignal(CS_BCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);
}

void init_Timer32_module()
{
    Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
    TIMER32_PERIODIC_MODE);
}

void create_Queues(void)
{
    qTReadToTCtrl = xQueueCreate(QUEUE_SIZE, sizeof(TempData));
    qTReadToLCD = xQueueCreate(QUEUE_SIZE, sizeof(TempData));
    qDispConsole = xQueueCreate(QUEUE_SIZE, sizeof(DisplayConsoleMsg));
    qKeypadToTCtrl = xQueueCreate(QUEUE_SIZE, sizeof(KeypadMsg));
    qTCtrlToLCD = xQueueCreate(QUEUE_SIZE, sizeof(TempData));
}

/* TODO: Intentar que el FW funcione a 1MHz (se deberia de poder, no?)
 *  Seguramente, hay que ir añadiendo custom performance levels en MSP_EXP432P401R.c
 *  con la configuracion de los LPM a los que se quiera transicionar a lo largo de la ejecucion
 *  y debe haber alguna funcion del driver que permita cambiar de un perf level a otro
 */

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t thread;
    pthread_attr_t attrs;
    struct sched_param priParam;
    int retc;

    /* initialize the system locks */
#ifdef __ICCARM__
    __iar_Initlocks();
#endif

    /* Call driver init functions */
    Board_init();
    //Power_setPolicy((Power_PolicyFxn) PowerMSP432_sleepPolicy);
    //Power_enablePolicy();

    //init_Clock_System_module();
    init_Timer32_module();

    /* Create queues for inter-thread communication */
    create_Queues();
    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /************************** Display LCD Thread ******************************/
    struct displayLCDThreadArgs *args_dlcd =
            (struct displayLCDThreadArgs*) malloc(
                    sizeof(struct displayLCDThreadArgs));
    args_dlcd->qDispConsoleArg = qDispConsole;
    args_dlcd->qTReadToLCDArg = qTReadToLCD;
    args_dlcd->qTCtrlToLCDArg = qTCtrlToLCD;

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1)
        {
        }
    }
    retc = pthread_create(&thread, &attrs, displayLCDThread, (void*) args_dlcd);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Display Console Thread ******************************/
    struct displayConsoleThreadArgs *args_dcon =
            (struct displayConsoleThreadArgs*) malloc(
                    sizeof(struct displayConsoleThreadArgs));
    args_dcon->qDispConsoleArg = qDispConsole;

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1)
        {
        }
    }
    retc = pthread_create(&thread, &attrs, displayConsoleThread,
                          (void*) args_dcon);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Temperature reading Thread ******************************/
    struct temperatureReadingThreadArgs *args_tread =
            (struct temperatureReadingThreadArgs*) malloc(
                    sizeof(struct temperatureReadingThreadArgs));
    args_tread->qDispConsoleArg = qDispConsole;
    args_tread->qTReadToLCDArg = qTReadToLCD;
    args_tread->qTReadToTCtrlArg = qTReadToTCtrl;

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1)
        {
        }
    }
    retc = pthread_create(&thread, &attrs, temperatureReadingThread,
                          (void*) args_tread);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Temperature control Thread ******************************/
    struct temperatureControllerThreadArgs *args_tctrl =
            (struct temperatureControllerThreadArgs*) malloc(
                    sizeof(struct temperatureControllerThreadArgs));
    args_tctrl->qDispConsoleArg = qDispConsole;
    args_tctrl->qTReadToTCtrlArg = qTReadToTCtrl;
    args_tctrl->qKeypadToTCtrlArg = qKeypadToTCtrl;
    args_tctrl->qTCtrlToLCDArg = qTCtrlToLCD;

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1)
        {
        }
    }
    retc = pthread_create(&thread, &attrs, temperatureControllerThread,
                          (void*) args_tctrl);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Keypad Thread ******************************/
    struct keypadThreadArgs *args_kpad = (struct keypadThreadArgs*) malloc(
            sizeof(struct keypadThreadArgs));
    args_kpad->qDispConsoleArg = qDispConsole;
    args_kpad->qKeypadToTCtrlArg = qKeypadToTCtrl;

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0)
    {
        /* failed to set attributes */
        while (1)
        {
        }
    }
    retc = pthread_create(&thread, &attrs, keypadThread, (void*) args_kpad);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    //while (1);

    return (0);
}

void vApplicationIdleHook(void)
{
    //Power_idleFunc();
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    /* Handle Memory Allocation Errors */
    while (1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while (1)
    {
    }
}
