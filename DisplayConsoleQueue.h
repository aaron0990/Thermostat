/*
 * DisplayConsoleQueue.h
 *
 *  Created on: 14 jun. 2022
 *      Author: aaron
 */

#ifndef DISPLAYCONSOLEQUEUE_H_
#define DISPLAYCONSOLEQUEUE_H_

#include <queue.h>
#define MSG_SIZE 256

typedef struct DisplayLCDMsg DisplayLCDMsg;
struct DisplayLCDMsg
{
    char msg[MSG_SIZE];
};

extern QueueHandle_t qDispConsole;

#endif /* DISPLAYCONSOLEQUEUE_H_ */
