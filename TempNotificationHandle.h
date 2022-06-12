/*
 * TempNotificationHandle.h
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#ifndef TEMPNOTIFICATIONHANDLE_H_
#define TEMPNOTIFICATIONHANDLE_H_

/*  Function pointer type
 *  -first param: pointer to instance data of the class
 *  -second param: pointer to the new temp data.
 */
#include "shared_vars.h"
#include "TempDataAcceptor.h"

typedef struct TempNotificationHandle TempNotificationHandle;
struct TempNotificationHandle{
    tempDataAcceptorPtr acceptorPtr;
    void* instancePtr;
};

void TempNotificationHandle_init(TempNotificationHandle* const me);
void TempNotificationHandle_clean(TempNotificationHandle* const me);
TempNotificationHandle* TempNotificationHandle_create(void);
void TempNotificationHandle_destroy(TempNotificationHandle* const me);


#endif /* TEMPNOTIFICATIONHANDLE_H_ */
