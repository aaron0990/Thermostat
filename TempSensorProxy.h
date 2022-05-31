/*
 * TempSensorProxy.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef TEMPSENSORPROXY_H_
#define TEMPSENSORPROXY_H_

#define MAX_TICK_VALUES 41

#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <gpio.h>
#include <string.h>
#include <inttypes.h>
#include <utils.h>
#include <interrupt.h>
#include <sysctl.h>
#include <pcm.h>
#include "TempData.h"
#include <ti/drivers/Capture.h>
#include <ti/drivers/GPIO.h>
#include "shared_vars.h"

typedef struct {
    TempData* itsTempData;
    Capture_Handle* captureHandle;
    Capture_Params* captureParams;
} TempSensorProxy;

extern uint32_t capturedIntervals[MAX_TICK_VALUES];
extern uint8_t capturedIntervalsPtr;
uint8_t readingData;
Capture_Handle capHandle;

TempSensorProxy* TempSensorProxy_create(void);
void TempSensorProxy_destroy(TempSensorProxy* me);
void TempSensorProxy_init (TempSensorProxy* me);
void TempSensorProxy_configure(TempSensorProxy* me);
void TempSensorProxy_access(TempSensorProxy* me);

void Capture_Callback(Capture_Handle handle, uint32_t interval);


#endif /* TEMPSENSORPROXY_H_ */
