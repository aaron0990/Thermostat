/*
 * ThreadsArgStruct.h
 *
 *  Created on: 14 jun. 2022
 *      Author: aaron
 */

#ifndef THREADSARGSTRUCT_H_
#define THREADSARGSTRUCT_H_

#include <queue.h>

struct displayConsoleThreadArgs
{
    QueueHandle_t qDispConsoleArg;
};

struct displayLCDThreadArgs
{
    QueueHandle_t qDispConsoleArg;
    QueueHandle_t qTReadToLCDArg;
    QueueHandle_t qTCtrlToLCDArg;
};

struct temperatureReadingThreadArgs
{
    QueueHandle_t qDispConsoleArg;
    QueueHandle_t qTReadToLCDArg;
    QueueHandle_t qTReadToTCtrlArg;
};

struct temperatureControllerThreadArgs
{
    QueueHandle_t qDispConsoleArg;
    QueueHandle_t qTReadToTCtrlArg;
    QueueHandle_t qKeypadToTCtrlArg;
    QueueHandle_t qTCtrlToLCDArg;
};

struct keypadThreadArgs
{
    QueueHandle_t qDispConsoleArg;
    QueueHandle_t qKeypadToTCtrlArg;
};

#endif /* THREADSARGSTRUCT_H_ */
