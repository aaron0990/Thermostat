/*
 * Display_console.c
 *
 *  Created on: 14 jun. 2022
 *      Author: aaron
 */

#include "shared_vars.h"
#include "ThreadsArgStruct.h"

void displayConsoleThread(void *arg0){
    while(1){
            sched_yield();
        }
}
