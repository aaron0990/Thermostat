/*
 * LCDdisplayClient.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "LCDdisplayClient.h"
#include "TempData.h"
#include "TempSensor.h"

void DisplayClient_init(DisplayClient *const me, TempSensor *const ts)
{
    me->itsDisplayProxy = LCD_create();
    me->itsTempData = NULL;
    me->itsTempSensor = ts;

    //Initialize LCD
//    LCD_clear(me->itsDisplayProxy);
//    LCD_display(me->itsDisplayProxy);
//    LCD_blink(me->itsDisplayProxy);
//    LCD_home(me->itsDisplayProxy);
}

void DisplayClient_clean(DisplayClient *const me)
{
    if (me->itsTempData != NULL)
    {
        me->itsTempData = NULL;
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

void DisplayClient_accept(DisplayClient *const me, TempData *td)
{
    if (!me->itsTempData)
    {
        me->itsTempData = TempData_create();
    }
    if (me->itsTempData)
    {
        me->itsTempData->temperature = td->temperature;
        me->itsTempData->humidity = td->humidity;
    }
}

void DisplayClient_register(DisplayClient *const me)
{
    if (me->itsTempSensor)
    {
        TempSensor_subscribe(me->itsTempSensor, me, DisplayClient_accept);
    }
}

void DisplayClient_show(DisplayClient *const me)
{
    char output[50];
    LCD_clear(me->itsDisplayProxy);
    if (me->itsTempData)
    {
        sprintf(output, "T. Act:%d'C", me->itsTempData->temperature);
        LCD_write(me->itsDisplayProxy, output, strlen(output));
        LCD_setCursor(me->itsDisplayProxy, 0, 1);
        sprintf(output, "T. Obj:%d'C", 24);
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
    Display_printf(disp_hdl, 0, 0, "display_client");
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

TempData* DisplayClient_getItsTempData(DisplayClient *const me)
{
    return (TempData*) me->itsTempData;
}
void DisplayClient_setItsTempData(DisplayClient *const me, TempData *p_td)
{
    me->itsTempData = p_td;
}
TempSensor* DisplayClient_getItsTempSensor(DisplayClient *const me)
{
    return (TempSensor*) me->itsTempSensor;
}
void DisplayClient_setItsTempSensor(DisplayClient *const me, TempSensor *p_ts)
{
    me->itsTempSensor = p_ts;
}
