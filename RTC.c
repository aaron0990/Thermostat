/*
 * RTC.c
 *
 *  Created on: 9 sept. 2022
 *      Author: aaron
 */

#include <cs.h>
#include <gpio.h>
#include <interrupt.h>
#include <rtc_c.h>
#include <stdlib.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/power/PowerMSP432.h>
#include <ti/drivers/Power.h>
#include "RTC.h"


RTC* RTC_C_create(void)
{
    RTC* me = (RTC*) malloc(sizeof(RTC));
    return me;
}

void RTC_C_init(RTC* const me)
{
    Power_setPolicy((Power_PolicyFxn)PowerMSP432_deepSleepPolicy);
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_1);
    Power_enablePolicy();

    /* RTC, 1s period */
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_0,0x00);
    RTC_C_setPrescaleValue(RTC_C_PRESCALE_1,0x00);
    RTC_C_definePrescaleEvent(RTC_C_PRESCALE_1, RTC_C_PSEVENTDIVIDER_128);

    //this function selects the output frequency for RTCCLK
    RTC_C_setCalibrationFrequency(RTC_C_CALIBRATIONFREQ_1HZ);
    //Pin 4.3 output the RTCCLK to calibrate RTC clock
    //RTC_C_setCalibrationData(RTC_C_CALIBRATION_DOWN1PPM, 240);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P4, GPIO_PIN3, GPIO_SECONDARY_MODULE_FUNCTION);
    RTC_C_Calendar cal;
    cal.year=2022;
    cal.month=10;
    cal.dayOfmonth=02;
    cal.dayOfWeek=6;
    cal.hours=7;
    cal.minutes=14;
    cal.seconds=00;
    RTC_C_initCalendar(&cal, RTC_C_FORMAT_BINARY);


    //this function calibrates the RTC clock once we measured the actual clock freq RTCCLK
    //RTC_C_setCalibrationData(offsetDirection, offsetValue)
    //RTC_C_PRESCALE_TIMER1_INTERRUPT, con divider 256 permite generar interrupcion cada 2 segundos.
    //Es lo máximo que se puede en este modo.
    RTC_C_enableInterrupt(RTC_C_PRESCALE_TIMER1_INTERRUPT);
    Interrupt_enableInterrupt(INT_RTC_C);

    HwiP_Params hwiParams;
    HwiP_Handle myHwi;
    HwiP_Params_init(&hwiParams);
    hwiParams.arg=5;
    hwiParams.priority=0x40;

    myHwi = HwiP_create(INT_RTC_C, RTC_C_IRQHandler, &hwiParams);

    /* Start the RTC */
    RTC_C_startClock();
}

int RTC_C_isTimerExpired(RTC* const me, uint32_t timestamp)
{
    if (me != NULL)
    {
        if (me->secondsCount == timestamp)
            return 1;
        else
            return 0;
    }
}

void RTC_C_configure(RTC* const me)
{
    if (me != NULL)
    {
        me->secondsCount = 0;
    }
}
