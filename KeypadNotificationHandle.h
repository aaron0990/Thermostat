/*
 * KeypadNotificationHandle.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef KEYPADNOTIFICATIONHANDLE_H_
#define KEYPADNOTIFICATIONHANDLE_H_

/*  Function pointer type
 *  -first param: pointer to instance data of the class
 *  -second param: pointer to the new temp data.
 */
#include "TempData.h"
#include "shared_vars.h"
#include "TempDataAcceptor.h"

typedef struct KeypadNotificationHandle KeypadNotificationHandle;
struct KeypadNotificationHandle{
    tempDataAcceptorPtr acceptorPtr;
    void* instancePtr;
};

void KeypadNotificationHandle_init(KeypadNotificationHandle* const me);
void KeypadNotificationHandle_clean(KeypadNotificationHandle* const me);
KeypadNotificationHandle* KeypadNotificationHandle_create(void);
void KeypadNotificationHandle_destroy(KeypadNotificationHandle* const me);


#endif /* KEYPADNOTIFICATIONHANDLE_H_ */
