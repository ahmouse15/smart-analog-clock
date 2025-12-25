// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Bud Davis and/or Professor Jason Losh)

/*
 * wait.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Jason Losh
 *      Transcribed from pdf by Bud Davis 2/1/2025
 */
#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "util/wait.h"

void waitMicrosecond(uint32_t us)
{
    // Approx clocks per usec
    __asm("WMS_LOOP0:     MOV R1,#6");
    //  1
    __asm("WMS_LOOP1:     SUB R1,#1");
    //  6
    __asm("               CBZ R1,WMS_DONE1");
    //  5+1 *3
    __asm("               NOP");
    //  5
    __asm("               B WMS_LOOP1");
    //  5*3
    __asm("WMS_DONE1:     SUB R0, #1");
    //  1
    __asm("               CBZ R0, WMS_DONE0");
    //  1
    __asm("               B WMS_LOOP0");
    //  1*3
    __asm("WMS_DONE0:");
    //  ---
    //  40 clocks/us + error

}
