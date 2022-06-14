/*
 * main.h
 *
 *  Created on: 28 may. 2022
 *      Author: aaron
 */

#ifndef SHARED_VARS_H_
#define SHARED_VARS_H_

#include <ti/display/Display.h>

/* POSIX Header files */
#include <pthread.h>
#include <FreeRTOS.h>
#include <queue.h>
#include <sched.h>
#include "DisplayConsoleQueue.h"

extern Display_Handle disp_hdl;

#endif /* SHARED_VARS_H_ */
