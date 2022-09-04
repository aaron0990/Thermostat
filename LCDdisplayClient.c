/*
 * LCDdisplayClient.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "LCDdisplayClient.h"

DisplayClient *gObj; //global DisplayClient "object" reference

void DisplayClient_turnOffLCDbacklight(TimerHandle_t xTimer)
{
    LCD_noDisplay(gObj->itsDisplayProxy);
    LCD_noBacklight(gObj->itsDisplayProxy);
}

void DisplayClient_init(DisplayClient *const me)
{
    me->itsTempTarget.temperature = 21.0;
    me->itsTempTarget.humidity = 0.0;
}

void DisplayClient_clean(DisplayClient *const me)
{

}

void DisplayClient_acceptTempSensed(DisplayClient *const me, TempData *td)
{

}

void DisplayClient_acceptTempTarget(DisplayClient *const me, TempData *td)
{

}

#pragma CODE_SECTION(DisplayClient_showInfo, ".TI.ramfunc")
void DisplayClient_showInfo(DisplayClient *const me)
{
    char output[50];
    LCD_backlight(me->itsDisplayProxy);
    LCD_display(me->itsDisplayProxy);

    LCD_setCursor(me->itsDisplayProxy, 0, 1); //Move to second line in LCD
    sprintf(output, "                "); //whitespaces to clear line
    LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
    LCD_setCursor(me->itsDisplayProxy, 0, 1);

    //Update target temperature in the screen
    if (me->itsTempTarget.temperature)
    {
        sprintf(output, "T. Obj:%.1f'C", me->itsTempTarget.temperature);
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
    else
    {
        strcpy(output, "No data");
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }

    memset(output, 0x00, 50); //Clear output buffer
    LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
    sprintf(output, "                "); //whitespaces to clear line
    LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
    LCD_setCursor(me->itsDisplayProxy, 0, 0);
    //Update sensed temperature in the screen
    if (me->itsTempSensed.temperature)
    {
        sprintf(output, "T. Act:%.1f'C", me->itsTempSensed.temperature); //DIVIDE BY 10 TO GET REAL TEMPERATURE!
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Print sensed temperature
    }
    else
    {
        strcpy(output, "No data");
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
}

DisplayClient* DisplayClient_create(void)
{
    DisplayClient *me = (DisplayClient*) malloc(sizeof(DisplayClient));
    return me;
}

void DisplayClient_destroy(DisplayClient *const me)
{
    if (me != NULL)
    {
        DisplayClient_clean(me);
    }
    free(me);
}

#pragma CODE_SECTION(displayLCDThread, ".TI.ramfunc")
void* displayLCDThread(void *arg0)
{
    Power_setConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);

    I2C_init();

    struct displayLCDThreadArgs *args = (struct displayLCDThreadArgs*) arg0;

    //Creates DisplayClient "instance"
    DisplayClient *me = DisplayClient_create();
    DisplayClient_init(me);
    gObj = me;

    TempData tSensed; //to store the dequed elem
    TempData tTarget; //to store the dequed elem

    me->itsDisplayProxy = LCD_create();
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToLCD = args->qTReadToLCDArg;
    me->qTCtrlToLCD = args->qTCtrlToLCDArg;

    TimerHandle_t lcdOFFBl_tmr = xTimerCreate(
            "LCDBacklightOFF", pdMS_TO_TICKS(LCD_ON_BACKLIGHT_T), pdFALSE,
            (void*) 0, DisplayClient_turnOffLCDbacklight);

    Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    while (1)
    {
        if (xQueueReceive(me->qTReadToLCD, &tSensed, (TickType_t) MAX_WAIT_QUEUE))
        {
            //Print value in LCD
            me->itsTempSensed = tSensed;
            //Turn on backlight for some secs and display info
            DisplayClient_showInfo(me);
            xTimerReset(lcdOFFBl_tmr, 0);
            /*
            if (!xTimerIsTimerActive(lcdOFFBl_tmr))
            {
                xTimerStart(lcdOFFBl_tmr, 0);
            }*/
        }
        if (xQueueReceive(me->qTCtrlToLCD, &tTarget, (TickType_t) MAX_WAIT_QUEUE))
        {
            //Print value in LCD
            me->itsTempTarget = tTarget;
            //Turn on backlight for some secs and display info
            DisplayClient_showInfo(me);
            xTimerReset(lcdOFFBl_tmr, 0);
            /*
            if (!xTimerIsTimerActive(lcdOFFBl_tmr))
            {
                xTimerStart(lcdOFFBl_tmr, 0);
            }*/
        }
        sched_yield();
    }
}
