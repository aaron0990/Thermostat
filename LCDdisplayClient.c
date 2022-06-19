/*
 * LCDdisplayClient.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "LCDdisplayClient.h"
#include "TempData.h"
#include "TempSensor.h"

void DisplayClient_init(DisplayClient *const me, TempSensor *const ts,
                        Keypad *const kp)
{
    Display_printf(disp_hdl, 0, 0, "DisplayClient_init()\n");
    me->itsDisplayProxy = LCD_create();
    me->itsKeypad = kp;
    me->itsTempSensed = NULL;
    me->itsTempTarget = NULL;
    me->itsTempSensor = ts;

    //Initialize LCD
//    LCD_clear(me->itsDisplayProxy);
//    LCD_display(me->itsDisplayProxy);
//    LCD_blink(me->itsDisplayProxy);
//    LCD_home(me->itsDisplayProxy);
}

void DisplayClient_clean(DisplayClient *const me)
{
    if (me->itsTempSensed != NULL)
    {
        me->itsTempSensed = NULL;
    }
    if (me->itsTempTarget != NULL)
    {
        me->itsTempTarget = NULL;
    }
    if (me->itsTempSensor != NULL)
    {
        me->itsTempSensor = NULL;
    }
    if (me->itsDisplayProxy != NULL)
    {
        LCD_destroy(me->itsDisplayProxy);
        me->itsDisplayProxy = NULL;
    }

}

void DisplayClient_acceptTempSensed(DisplayClient *const me, TempData *td)
{
    if (!me->itsTempSensed)
    {
        me->itsTempSensed = TempData_create();
    }
    if (me->itsTempSensed)
    {
        me->itsTempSensed->temperature = td->temperature;
        me->itsTempSensed->humidity = td->humidity;
    }
}

void DisplayClient_acceptTempTarget(DisplayClient *const me, TempData *td)
{
    if (!me->itsTempTarget)
    {
        me->itsTempTarget = TempData_create();
    }
    if (me->itsTempTarget)
    {
        me->itsTempTarget->temperature = td->temperature;
        me->itsTempTarget->humidity = td->humidity;
    }
    //DisplayClient_showTempTarget(me); NO MOLA esta llamada
}


void DisplayClient_showTempSensed(DisplayClient *const me)
{
    char output[50];
    if (me->itsTempSensed)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
        sprintf(output, "             "); //whitespaces to clear line
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
        LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
        sprintf(output, "T. Act:%d'C", me->itsTempSensed->temperature);
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Print sensed temperature
    }
    else
    {
        strcpy(output, "No data available!");
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
}

void DisplayClient_showTempTarget(DisplayClient *const me)
{
    char output[50];
    LCD_clear(me->itsDisplayProxy);
    if (me->itsTempSensed)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 1); //Move to first line in LCD
        sprintf(output, "             "); //whitespaces to clear line
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
        LCD_setCursor(me->itsDisplayProxy, 0, 1);
        sprintf(output, "T. Obj:%d'C", me->itsTempTarget->temperature);
        LCD_write(me->itsDisplayProxy, output, strlen(output));
    }
    else
    {
        strcpy(output, "No data available!");
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

TempData* DisplayClient_getItsTempSensed(DisplayClient *const me)
{
    return (TempData*) me->itsTempSensed;
}
void DisplayClient_setItsTempSensed(DisplayClient *const me, TempData *p_td)
{
    me->itsTempSensed = p_td;
}
TempSensor* DisplayClient_getItsTempSensor(DisplayClient *const me)
{
    return (TempSensor*) me->itsTempSensor;
}
void DisplayClient_setItsTempSensor(DisplayClient *const me, TempSensor *p_ts)
{
    me->itsTempSensor = p_ts;
}

void *displayLCDThread(void *arg0){

    I2C_init();

    struct displayLCDThreadArgs *args = (struct displayLCDThreadArgs*) arg0;

    //Creates DisplayClient "instance"
    DisplayClient *me = DisplayClient_create();

    TempData tSensed; //where to store the dequed elem
    TempData tTarget; //where to store the dequed elem

    me->itsDisplayProxy = LCD_create();
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToLCD = args->qTReadToLCDArg;
    //TODO: Initialize the rest of struct "me"

    while(1)
    {
       if (xQueueReceive(me->qTReadToLCD, &tSensed, 0)){
           //Print value in LCD
           me->itsTempSensed = &tSensed;
           DisplayClient_showTempSensed(me);
       }
       else{
           sched_yield();
       }
       sleep(1);
    }
    //vTaskDelete(NULL);
}
