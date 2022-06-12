/*
 * KeypadNotificationHandle.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */

#include "KeypadNotificationHandle.h"
#include <string.h>
#include <stdlib.h>

void KeypadNotificationHandle_init(KeypadNotificationHandle* const me){

}

void KeypadNotificationHandle_clean(KeypadNotificationHandle* const me){

}

KeypadNotificationHandle* KeypadNotificationHandle_create(void){
    KeypadNotificationHandle* me = (KeypadNotificationHandle*) malloc(sizeof(KeypadNotificationHandle));
    if (me != NULL){
        KeypadNotificationHandle_init(me);
    }
    return me;
}

void KeypadNotificationHandle_destroy(KeypadNotificationHandle* const me){
    if (me != NULL){
        KeypadNotificationHandle_clean(me);
    }
    free(me);
}

