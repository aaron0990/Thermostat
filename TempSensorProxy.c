/*
 * TempSensorProxy.c
 *
 *  Created on: 28 dic. 2021
 *      Author: aaron
 */

#include "TempSensorProxy.h"

uint32_t capturedIntervals[MAX_TICK_VALUES];
uint8_t capturedIntervalsPtr;

void TempSensorProxy_init(TempSensorProxy *me)
{
    me->itsTempData = TempData_create();
}

void TempSensorProxy_configure(TempSensorProxy *me)
{
    Capture_init();
    me->captureHandle = (Capture_Handle*) malloc(sizeof(Capture_Handle));
    me->captureParams = (Capture_Params*) malloc(sizeof(Capture_Params));
    Capture_Params_init(me->captureParams);
    me->captureParams->mode = Capture_FALLING_EDGE;
    me->captureParams->periodUnit = Capture_PERIOD_US;
    me->captureParams->callbackFxn = Capture_Callback;
    *(me->captureHandle) = Capture_open(0, me->captureParams);

    if (*(me->captureHandle) == NULL)
    {
        exit(-1);
    }
}

void TempSensorProxy_access(TempSensorProxy *me)
{

    capturedIntervalsPtr = 0;
    readingData = 0;
    memset(capturedIntervals, 0, sizeof(uint32_t) * MAX_TICK_VALUES);

    //Disable task switching for temp reading
    vTaskSuspendAll();

    //Use pin P3.0 for debugging
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);

    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN6);
    delay_us(1000);
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);

    //MCU start signal ~18ms
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN6);
    delay_us(20000);

    //MCU waits for DHT response (~20-40us)
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3, GPIO_PIN6);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
    delay_us(80); //not using usleep() because it doesn't work well with so small values.
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
    //Set timer for capture
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN6,
                                               GPIO_PRIMARY_MODULE_FUNCTION);
    readingData = 1;

    int32_t ret = Capture_start(*(me->captureHandle));

    if (ret != Capture_STATUS_SUCCESS)
    {
        Display_printf(disp_hdl,0,0,"TempSensorProxy.access()::Capture_start() failed!\n");
        readingData = 0;
    }

    // Wait until sensor data are read
    while (readingData);

    Capture_stop(*(me->captureHandle));

    //Reenable task switching once finished
    xTaskResumeAll();

    uint64_t sensorData = 0;
    int i;
    for (i = 0; i < MAX_TICK_VALUES; ++i)
    {
        // time less than 100us -> logic 0. Otherwise logic 1
        uint32_t period_us = capturedIntervals[i] / 4;
        sensorData |= (((uint64_t) (period_us < 100 ? 0 : 1))
                << MAX_TICK_VALUES - i - 1);
    }

    me->itsTempData->humidity = (uint8_t) ((sensorData >> 32) & 0xFF);
    me->itsTempData->temperature = (uint8_t) ((sensorData >> 16) & 0xFF);

    //Checksum
    /*if (me->humidityInt + me->humidityDec + me->temperatureInt + me->temperatureDec == (sensorData & 0xFF)){
     return 1;
     }
     else{
     return 0;
     }*/
}

/*
 * Timer_A0 interrupt handler
 */

void Capture_Callback(Capture_Handle handle, uint32_t interval)
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
    capturedIntervals[capturedIntervalsPtr] = interval;
    ++capturedIntervalsPtr;
    if (capturedIntervalsPtr == MAX_TICK_VALUES)
    {
        readingData = 0;
    }
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
}

TempSensorProxy* TempSensorProxy_create(void)
{
    TempSensorProxy *me = (TempSensorProxy*) malloc(sizeof(TempSensorProxy));
    if (me != NULL)
    {
        TempSensorProxy_configure(me);
        TempSensorProxy_init(me);
    }
    return me;
}

void TempSensorProxy_destroy(TempSensorProxy *me)
{
    if (me != NULL)
    {
        free(me);
    }
}
