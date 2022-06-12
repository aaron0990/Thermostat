/*
 * TempData.c
 *
 *  Created on: 12 dic. 2021
 *      Author: aaron
 */
#include "TempData.h"
#include <string.h>
#include <stdlib.h>

void TempData_init(TempData* const me){
    me->temperature = 0;
    me->humidity = 0;
}

void TempData_clean(TempData* const me){

}

TempData* TempData_create(void){
    TempData* me = (TempData*) malloc(sizeof(TempData));
    if(me != NULL){
        TempData_init(me);
    }
    return me;
}

void TempData_destroy(TempData* const me){
    if(me != NULL){
        TempData_clean(me);
    }
    free(me);
}
