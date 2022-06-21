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

/* Stack size in bytes */
#define THREADSTACKSIZE   1024

#define QUEUE_SIZE 10   //Max num of elements in the queue

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

/* EL DISPLAY HANDLE YA NO ES UNA VARIABLE GLOBAL. CADA THREAD METE EN UNA COLA DEDICADA LO QUE QUIERA PRINTAR POR CONSOLA.
void init_Display(void)
{
    Display_Params params;
    Display_Params_init(&params);
    disp_hdl = Display_open(0, NULL);
    Display_clear(disp_hdl); //Clear previous execution output of the terminal
}*/

void create_Queues(void)
{
    qTReadToTCtrl = xQueueCreate(QUEUE_SIZE, sizeof(TempData));
    qTReadToLCD = xQueueCreate(QUEUE_SIZE, sizeof(TempData));
    qDispConsole = xQueueCreate(QUEUE_SIZE, sizeof(DisplayConsoleMsg));
    qKeypadToTCtrl = xQueueCreate(QUEUE_SIZE, sizeof(KeypadMsg));
}

/* TODO:
 *
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

    init_Clock_System_module();
    init_Timer32_module();

    /* Create queues for inter-thread communication */
    create_Queues();
    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /************************** Display LCD Thread ******************************/
    struct displayLCDThreadArgs *args_dlcd = (struct displayLCDThreadArgs *)malloc(sizeof(struct displayLCDThreadArgs));
    args_dlcd->qDispConsoleArg = qDispConsole;
    args_dlcd->qTReadToLCDArg = qTReadToLCD;

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
    retc = pthread_create(&thread, &attrs, displayLCDThread, (void *)args_dlcd);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Display Console Thread ******************************/
    struct displayConsoleThreadArgs *args_dcon = (struct displayConsoleThreadArgs *)malloc(sizeof(struct displayConsoleThreadArgs));
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
    retc = pthread_create(&thread, &attrs, displayConsoleThread, (void *)args_dcon);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }


    /************************** Temperature reading Thread ******************************/
    struct temperatureReadingThreadArgs *args_tread = (struct temperatureReadingThreadArgs *)malloc(sizeof(struct temperatureReadingThreadArgs));
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
                         (void *)args_tread);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Temperature control Thread ******************************/
    struct temperatureControllerThreadArgs *args_tctrl = (struct temperatureControllerThreadArgs *)malloc(sizeof(struct temperatureControllerThreadArgs));
    args_tctrl->qDispConsoleArg = qDispConsole;
    args_tctrl->qTReadToTCtrlArg = qTReadToTCtrl;
    args_tctrl->qKeypadToTCtrl = qKeypadToTCtrl;

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
                          (void *)args_tctrl);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /************************** Keypad Thread ******************************/
    struct keypadThreadArgs *args_kpad = (struct keypadThreadArgs *)malloc(sizeof(struct keypadThreadArgs));
    args_kpad->qDispConsoleArg = qDispConsole;
    args_kpad->qKeypadToTCtrl = qKeypadToTCtrl;

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
    retc = pthread_create(&thread, &attrs, keypadThread, (void *)args_kpad);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1)
        {
        }
    }

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
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
