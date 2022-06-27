/*
 * InterThreadQueues.h
 *
 *  Created on: 21 jun. 2022
 *      Author: aaron
 */

#ifndef INTERTHREADQUEUES_H_
#define INTERTHREADQUEUES_H_

#include <queue.h>
#include "TempData.h"

//********************************* temperatureReading ==> temperatureController queue ****************************************//

//Queue element is TempData
extern QueueHandle_t qTReadToTCtrl;

//********************************* ALL ==> displayConsole queue ********************************************************//

#define MSG_SIZE 256

typedef struct DisplayConsoleMsg DisplayConsoleMsg;
struct DisplayConsoleMsg
{
    char *buff;
    uint16_t len;
};
extern QueueHandle_t qDispConsole;

//********************************* temperatureReading ==> displayLCD queue ********************************************************//

//Queue element is TempData
extern QueueHandle_t qTReadToLCD;

//********************************* temperatureController ==> displayLCD queue ********************************************************//

//Queue element is TempData
extern QueueHandle_t qTCtrlToLCD;


//********************************* keypad  ==> temperatureController queue ********************************************************//

#define INC_TARGET_T    0x01
#define DEC_TARGET_T    0x02

typedef struct KeypadMsg KeypadMsg;
struct KeypadMsg
{
    uint8_t cmd;
};
extern QueueHandle_t qKeypadToTCtrl;

#endif /* INTERTHREADQUEUES_H_ */
