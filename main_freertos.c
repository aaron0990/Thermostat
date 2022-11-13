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
#include <semaphore.h>

/*
 *  ======== main_freertos.c ========
 */


#ifdef __ICCARM__
#include <DLib_Threads.h>
#endif

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include <LCDdisplayClient.h>
#include <StateMachine.h>
#include <stdint.h>
#include <unistd.h>
#include <stddef.h>
#include <TempController.h>

/* Driver configuration */
#include <ti/drivers/Board.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>

/* Board Header file */
#include "Board.h"

#include <driverlib.h>
#include <pcm.h>
#include <utils.h>
#include <string.h>
#include <shared_vars.h>
#include <queue.h>

extern void* displayConsoleThread(void *arg0);
extern void* displayLCDThread(void *arg0);
extern void* mainThread(void* arg);
extern void* keypadThread(void *arg0);
extern void* temperatureControllerThread(void *arg0);
extern void* stateMachineThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE   4096

#define QUEUE_SIZE 10   //Max num of elements in the queue

Display_Handle disp_hdl;

sem_t startReadingTemp;
sem_t initDisplayDone;
sem_t startStateMachine;

QueueHandle_t stateMachineEventQueue;
QueueHandle_t displayClientEventQueue;

/*
 * THERMOSTAT MODULES CLOCK SOURCES AND DIVIDERS
 *
 * On MCU init -> DCO=12MHz
 *
 *  - Temp. sensor      -> Capture Module   -> TIMER_A1     -> SMCLK / 1
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
    PCM_setCoreVoltageLevel(PCM_AM_LDO_VCORE0);
    FlashCtl_setWaitState(FLASH_BANK0, 0);
    FlashCtl_setWaitState(FLASH_BANK1, 0);

    /*: Internal digitally controlled oscillator (DCO) with programmable frequencies and 3-MHz
     frequency by default.*/
    //CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_1_5);
    CS_setDCOFrequency(CS_12MHZ);
    /* Internal, low-power low-frequency oscillator (REFO) with selectable 32.768-kHz or 128-
     kHz typical frequencies*/
    CS_setReferenceOscillatorFrequency(CS_REFO_32KHZ);
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


void queryClockFreqs(void){
    uint32_t SMCLKfreq, MCLKfreq, HSMCLKfreq, BCLKfreq, ACLKfreq;
    MCLKfreq = CS_getMCLK();
    HSMCLKfreq = CS_getHSMCLK();
    SMCLKfreq = CS_getSMCLK();
    ACLKfreq = CS_getACLK();
    BCLKfreq = CS_getBCLK();
    return;
}

/* TODO:
 *  - Consejos para reducir consumo
 *      - Probar a poner el GND del relé en un GPIO
 *  - Si no consigo hacer funcionar el deepSleep (LPM3), pasar el active state a PCM_AM_LDO_VCORE0 en vez de PCM_AM_DCDC_VCORE0 (baja el consumo unos 200uA)
 *  - Probar a hacer el enablePolicy() dentro de un thread y no en el main (instanciarlo despues de vTaskStartScheduler)
 *  - Si se está en LPM3, el timer interno del MSP432 se suspende tambien:
        Note: Clock ticks halt during MSP432 deep sleep states. This is a significant consequence in
        that timeouts scheduled by the Clock module are suspended during deep sleep, and
        so will not be able to wake up the device.
        For example, when Task_sleep() is called, it blocks the execution of the Task for a
        number of system ticks. But if the power policy puts the device into deep sleep, the
        Clock ticking is suspended while the device is in deep sleep. In other words, the timer
        generating Clock ticks is halted. So the timeout that would normally trigger resumption
        of the Task_sleep() call is suspended too, and the device will not be able to wake to
        service the timeout at the anticipated time. Instead, the device will need to be awoken
        by another source. When awoken, the Clock module will start ticking again, but the time
        spent in deep sleep will not be factored in to future timeouts.
        In summary, on MSP432, if timeouts via the Clock module are needed, then the default,
        lighter-weight sleep policy Power_sleepPolicy() should be used. If wakeups from deep
        sleep are triggered by other sources (such as a GPIO line changing state), then the
        PowerMSP432_deepSleepPolicy() allows much better power savings.

 *  - Voy a usar el RTC para contar tiempo cuando se está en modo deepSleep (básicamente siempre),
 *     y utilizaré un modulo RTC DS3231 externo para conseguir mayor accuracy en el reloj.
 *
 *  - Mirar de hacer que el BCLK tire de LFXTCLK, el cual será la señal que venga del oscilador del modulo RTC externo a 32kHz.
 *    Así, en la MCU contaremos los segundos de manera más precisa que con la señal de clock interna de 32kHz que genera la MCU.
 *    ESTO IMPLICA QUE EL MODULO RTC ESTÉ ENCENDIDO SIEMPRE, LO QUE AUMENTA EL CONSUMO DE BATERIA -> NO POR AHORA.
 *
 *
 */

/* NOTES:
 *      -Do not call I2C API functions before scheduler had started (calling vTaskStartScheduler()). If this is done, you'll get a fault interrupt
 *
 *
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

    /*Comment out the notification registering to save power*/
   /*Power_registerNotify(&notifyObj,
            PowerMSP432_ENTERING_SLEEP |
            PowerMSP432_ENTERING_DEEPSLEEP |
            PowerMSP432_AWAKE_SLEEP |
            PowerMSP432_AWAKE_DEEPSLEEP |
            PowerMSP432_START_CHANGE_PERF_LEVEL |
            PowerMSP432_DONE_CHANGE_PERF_LEVEL,
            (Power_NotifyFxn) notifyFxn, 0x1);*/

   //Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_1); //Disallow LPM4 (otherwise peripherals will not work)
   //Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);

    /* Enabling SRAM Bank Retention for all banks */
    MAP_SysCtl_enableSRAMBankRetention(
            SYSCTL_SRAM_BANK1 | SYSCTL_SRAM_BANK2 | SYSCTL_SRAM_BANK3
                    | SYSCTL_SRAM_BANK4 | SYSCTL_SRAM_BANK5 | SYSCTL_SRAM_BANK6
                    | SYSCTL_SRAM_BANK7);
    /* Initialize clocks frequencies*/
    queryClockFreqs();
    init_Timer32_module();

    //Create stateMachineEventQueue

    stateMachineEventQueue = xQueueCreate(QUEUE_SIZE, sizeof(SMEvent_t));
    displayClientEventQueue = xQueueCreate(QUEUE_SIZE, sizeof(DCEvent));

    /* Semaphores initialization */
    retc = sem_init(&startReadingTemp, 0, 0);
    if (retc == -1) {
        while (1);
    }

    retc = sem_init(&initDisplayDone, 0, 0);
    if (retc == -1) {
        while (1);
    }
    retc = sem_init(&startStateMachine, 0, 0);
    if (retc == -1) {
        while (1);
    }

    /* Initialize the attributes structure with default values */
    pthread_attr_init(&attrs);

    /************************** Temperature reading Thread ******************************/

    priParam.sched_priority = 1;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    retc = pthread_create(&thread, &attrs, mainThread,
                          NULL);

    /************************** Display LCD Thread ******************************/

    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    retc = pthread_create(&thread, &attrs, displayLCDThread,
                          NULL);


    /************************** State Machine Thread ******************************/

    priParam.sched_priority = 2;
    retc = pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    retc = pthread_create(&thread, &attrs, stateMachineThread,
                          NULL);

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    return (0);
}



/* MUST NOT, UNDER ANY CIRCUMSTANCES, CALL A FUNCTION THAT MIGHT BLOCK. */
/*void vApplicationIdleHook(void)
{

}*/

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
