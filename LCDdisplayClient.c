/*
 * LCDdisplayClient.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "LCDdisplayClient.h"
#include "TempData.h"
#include "TempSensor.h"

void DisplayClient_init(DisplayClient *const me)
{
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


void DisplayClient_showTempSensed(DisplayClient *const me)
{
    char output[50];
    if (me->itsTempSensed.temperature)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
        sprintf(output, "             "); //whitespaces to clear line
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
        LCD_setCursor(me->itsDisplayProxy, 0, 0); //Move to first line in LCD
        sprintf(output, "T. Act:%d'C", me->itsTempSensed.temperature);
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
    if (me->itsTempSensed.temperature)
    {
        LCD_setCursor(me->itsDisplayProxy, 0, 1); //Move to first line in LCD
        sprintf(output, "             "); //whitespaces to clear line
        LCD_write(me->itsDisplayProxy, output, strlen(output)); //Clear only this line (cannot call to LCD_clear(). Else, all screen gets cleared)
        LCD_setCursor(me->itsDisplayProxy, 0, 1);
        sprintf(output, "T. Obj:%d'C", me->itsTempTarget.temperature);
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

void *displayLCDThread(void *arg0){

    I2C_init();

    struct displayLCDThreadArgs *args = (struct displayLCDThreadArgs*) arg0;

    //Creates DisplayClient "instance"
    DisplayClient *me = DisplayClient_create();

    TempData tSensed; //to store the dequed elem
    TempData tTarget; //to store the dequed elem

    me->itsDisplayProxy = LCD_create();
    me->qDispConsole = args->qDispConsoleArg;
    me->qTReadToLCD = args->qTReadToLCDArg;
    me->qTCtrlToLCD = args->qTCtrlToLCDArg;

    while(1)
    {
       if (xQueueReceive(me->qTReadToLCD, &tSensed, 0)){
           //Print value in LCD
           me->itsTempSensed = tSensed;
           DisplayClient_showTempSensed(me);
       }
       if (xQueueReceive(me->qTCtrlToLCD, &tTarget, 0)){
           //Print value in LCD
           me->itsTempTarget = tTarget;
           DisplayClient_showTempTarget(me);
       }
       sched_yield();
       sleep(1);
    }
    //vTaskDelete(NULL);
}
