/*
 * TempNotificationHandle.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "TempNotificationHandle.h"
#include <string.h>
#include <stdlib.h>

void TempNotificationHandle_init(TempNotificationHandle* const me){

}

void TempNotificationHandle_clean(TempNotificationHandle* const me){

}

TempNotificationHandle* TempNotificationHandle_create(void){
    TempNotificationHandle* me = (TempNotificationHandle*) malloc(sizeof(TempNotificationHandle));
    if (me != NULL){
        TempNotificationHandle_init(me);
    }
    return me;
}

void TempNotificationHandle_destroy(TempNotificationHandle* const me){
    if (me != NULL){
        TempNotificationHandle_clean(me);
    }
    free(me);
}

