/*
 * mainThread.h
 *
 *  Created on: 7 sept. 2022
 *      Author: aaron
 */

#ifndef MAINTHREAD_H_
#define MAINTHREAD_H_

/*Application includes*/
#include <LCDdisplayClient.h>
#include <TempController.h>
#include <TempSensor.h>
#include <RTC.h>
#include <Keypad.h>
#include <TempData.h>

/*Driverlib includes*/
#include <interrupt.h>
#include <rtc_c.h>

#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <ti/drivers/dpl/HwiP.h>

unsigned int notifyFxn(unsigned int eventType, unsigned int eventArg, unsigned int clientArg);

#endif /* MAINTHREAD_H_ */
