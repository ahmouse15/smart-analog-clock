// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// RTOS Framework - Fall 2025
// J Losh

// Student Name:
// TO DO: Add your name on this line.
//        Do not include your ID number in the file.

// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// Memory Protection Unit (MPU):
//   Region to control access to flash, peripherals, and bitbanded areas
//   4 or more regions to allow SRAM access (RW or none for task)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "tm4c123gh6pm.h"
#include "sys/clock.h"
#include "io/gpio.h"
#include "io/uart0.h"
#include "util/wait.h"
#include "sys/mm.h"
#include "sys/kernel.h"
#include "sys/faults.h"
#include "sys/tasks.h"
#include "io/shell.h"

void enable_faults()
{
    NVIC_SYS_HND_CTRL_R |= 0x00070000; //Enable all faults, bits 18 17 16
}


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

#define POS_PIN 0
#define NEG_PIN 1

int main(void)
{
    // Initialize hardware
    initSystemClockTo40Mhz();
    initHw();
    initUart0();
    enable_faults();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    //Setup 32.768KHz clock on GPIO pin PB6, M0PWM0
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R0;
    _delay_cycles(10);

    /*PWM0_0_CTL_R = 0;
    PWM0_0_LOAD_R |= 0x2FAA; //0x4C3; //1220-1 = 1219 ticks
    PWM0_0_GENA_R |= 0x1; //Drive low at start
    PWM0_0_GENA_R |= 0x2 << 6; //Invert on reaching duty cycle threshold
    PWM0_0_CMPA_R |= 0x17D4;//0x261  //~50% duty cycle = 1220/2-1 = 609 ticks
    PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
    PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN; //Enable output
    */
    while(1);
    //40MHz/32768Hz = 1220.7 ~= 1220

}
