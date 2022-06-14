/*
 * Keypad.h
 *
 *  Created on: 4 jun. 2022
 *      Author: aaron
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#define MAX_SUBSCRIBERS 100

#include "TempData.h"
#include <ti/drivers/GPIO.h>
#include <string.h>
#include <timer_a.h>
#include <interrupt.h>
#include <gpio.h>
#include <stdlib.h>
#include "shared_vars.h"
#include <sysctl.h>
#include "ThreadsArgStruct.h"

#define TEMP_UPDATE_INT 15 //seconds

#define INC_TEMP_PIN_IDX 0
#define DEC_TEMP_PIN_IDX 1

/*Class Keypad*/
typedef struct Keypad Keypad;
struct Keypad
{
    TempData *itsTargetTemp;
    //This one is necessary
    QueueHandle_t qDispConsole;
};
extern Keypad* kp;

/*Constructors and destructors*/
void Keypad_init(Keypad *const me);
void Keypad_clean(Keypad *const me);

/*Operations*/
void Keypad_readTemp(Keypad *const me);
void Keypad_newData(Keypad *const me);
Keypad* Keypad_create(void);
void Keypad_destroy(Keypad *const me);
void Keypad_InterruptHandler(uint_least8_t idx);

//TODO: Check aPtr argument in subscribe() and unsubscribe() functions. I'm not sure if the type is tempDataAcceptorPtr or tempDataAcceptorPtr*

#endif /* KEYPAD_H_ */
