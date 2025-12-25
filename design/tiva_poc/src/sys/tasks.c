// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// Tasks
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "io/gpio.h"
#include "util/wait.h"
#include "sys/kernel.h"
#include "sys/tasks.h"

#define PB0 PORTE,3 // Push Button 0
#define PB1 PORTE,2 // Push Button 1
#define PB2 PORTE,1 // Push Button 2
#define PB3 PORTE,4 // Push Button 3
#define PB4 PORTE,5 // Push Button 4
#define PB5 PORTD,6 // Push Button 5 (errant task)

#define BLUE_LED   PORTF,2 // on-board blue LED
#define RED_LED    PORTE,0 // off-board red LED
#define ORANGE_LED PORTA,2 // off-board orange LED
#define YELLOW_LED PORTA,3 // off-board yellow LED
#define GREEN_LED  PORTA,4 // off-board green LED

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons
void initHw(void)
{
    /*** Setup LEDs and pushbuttons ***/
    //Enable GPIO ports
    enablePort(PORTB);
    
    //Pushbuttons
    selectPinPushPullOutput(PORTB, 6);
    enablePinPulldown(PORTB, 6);
    setPinAuxFunction(PORTB, 6, 4); //M0PWM0 PCTRL value
}

uint8_t readPbs(void)
{
    uint8_t read = 0;

    read |= getPinValue(PB0) << 0;
    read |= getPinValue(PB1) << 1;
    read |= getPinValue(PB2) << 2;
    read |= getPinValue(PB3) << 3;
    read |= getPinValue(PB4) << 4;
    read |= getPinValue(PB5) << 5;

    return read;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void)
{
    while(true)
    {
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        yield();
    }
}

void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(125);
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
    }
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            killThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        unlock(resource);
    }
}
