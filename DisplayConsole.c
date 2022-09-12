/*
 * Display_console.c
 *
 *  Created on: 14 jun. 2022
 *      Author: aaron
 */

#include <string.h>
#include <ti/display/Display.h>
#include <unistd.h>
#include "shared_vars.h"
#include "ThreadsArgStruct.h"

#pragma CODE_SECTION(displayConsoleThread, ".TI.ramfunc")
void *displayConsoleThread(void *arg0){
    uint32_t row = 0; //for printing to console in a new line;

    Display_Handle disp_hdl;
    Display_Params params;
    Display_init();
    Display_Params_init(&params);
    disp_hdl = Display_open(0, NULL);
    Display_clear(disp_hdl); //Clear previous execution output of the terminal

    Power_releaseConstraint(PowerMSP432_DISALLOW_DEEPSLEEP_0);
    while(1)
    {
       //Print value in LCD
       char text[msg.len];
       strcpy(text, msg.buff);
       Display_printf(disp_hdl, row, 0, text);
       ++row;
    }

}
