// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// Clock Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// 16 MHz external crystal oscillator

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "sys/clock.h"
#include "sys/kernel.h"
#include "tm4c123gh6pm.h"

#define CLK_FREQ 40E6

bool time_slot = 0;
extern uint8_t taskCurrent;

const uint32_t intervalALoad = 10240; //1 sec = 1 frame = 10000 ticks
const uint32_t intervalBLoad = 0xFFFFFF; // just has to be larger than intervalA

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize system clock to 40 MHz using PLL and 16 MHz crystal oscillator
void initSystemClockTo40Mhz(void)
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN
            | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
}

void initTimers()
{
    const uint32_t prescale = 3999; //Pre-divide by 2

    // Setup GPTMs for tracking task execution time
    // Wide Timer 0A triggers int every 1 second, Wide Timer 0B tracks time of current task
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    NVIC_EN2_R |= 1 << 30; //Enable WTimer 0 A interrupt (interrupt 94)
    //NVIC_EN2_R |= 1 << 31; //Enable WTimer 0 B interrupt (interrupt 95)

    WTIMER0_CFG_R = 0x4;

    //---- Wide Timer 0A setup ---- (interrupt generating)
    WTIMER0_TAMR_R &= ~TIMER_TAMR_TAMR_M;
    WTIMER0_TAMR_R |= 0x2; // periodic mode
    WTIMER0_TAMR_R &= ~TIMER_TAMR_TACDIR; //count down
    WTIMER0_TAILR_R = intervalALoad;
    WTIMER0_IMR_R |= TIMER_IMR_TATOIM; // Interrupt on timer reaching intervalLoad

    // stall timer when debugger is halted
    WTIMER0_CTL_R |= TIMER_CTL_TASTALL;
    WTIMER0_CTL_R |= TIMER_CTL_TBSTALL;

    // set prescale
    WTIMER0_TAPR_R &= ~(TIMER_TAPR_TAPSR_M | TIMER_TAPR_TAPSRH_M);
    WTIMER0_TAPR_R |= prescale;



    //---- Wide Timer 0B setup ---- (non-interrupt generating)
    WTIMER0_TBMR_R &= ~TIMER_TBMR_TBMR_M;
    WTIMER0_TBMR_R |= 0x2;
    WTIMER0_TBILR_R = intervalBLoad; //effectively infinite
    WTIMER0_TBMR_R &= ~TIMER_TBMR_TBCDIR;
    // set prescale
    WTIMER0_TBPR_R &= ~(TIMER_TBPR_TBPSR_M | TIMER_TBPR_TBPSRH_M);
    WTIMER0_TBPR_R |= prescale;

    //Enable
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;

}

void clearTimer()
{
    WTIMER0_TBV_R = intervalBLoad;
}

void disableTimer()
{
    WTIMER0_CTL_R &= ~TIMER_CTL_TBEN;
}

void enableTimer()
{
    WTIMER0_CTL_R |= TIMER_CTL_TBEN;
}

void startCurrentTaskDuration()
{
    // stop timer, clear it, then start it again
    disableTimer();
    clearTimer();
    enableTimer();
}

uint32_t getCurrentTaskDuration()
{
    //return duration of timer B
    return intervalBLoad - WTIMER0_TBV_R;
}

void saveCurrentTaskDuration()
{
    //Disable timer and store task's run time
    tcb[taskCurrent].cpu_time[time_slot] += getCurrentTaskDuration();
}

void finishCurrentTaskDuration()
{
    //Disable timer and store task's run time
    saveCurrentTaskDuration();
    clearTimer();
    disableTimer();
}

uint32_t getCpuTime(uint8_t task)
{
    return tcb[task].cpu_time[!time_slot];
}

void convertCpuTimeToPercent(uint32_t rawtime, uint8_t *integer, uint8_t *fraction)
{
    // 1024 is an approximation of 1000 for performance purposes
    *integer = rawtime*10 / 1024;
    *fraction = rawtime - (*integer)*100;
    // Hacky way to ensure it always has two decimal spots
    if (*fraction >= 200) *fraction -= 200;
    else if (*fraction >= 100) *fraction -= 100;
}


void getCpuTimeAsPercent(uint8_t task, uint8_t *integer, uint8_t *fraction)
{
    // read inactive slot since its data is already finalized
    const uint32_t time = getCpuTime(task);
    convertCpuTimeToPercent(time, integer, fraction);
}

void wTimer0AIsr(void)
{   
    // Clear interrupt
    WTIMER0_ICR_R |= TIMER_ICR_TATOCINT;

    // Save mid-task duration before timeslot change
    saveCurrentTaskDuration();
    clearTimer();

    time_slot = !time_slot;
    // Clear old time entries
    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].cpu_time[time_slot] = 0;
    }
}

void wTimer0BIsr(void)
{
    // Clear interrupt
    WTIMER0_ICR_R |= TIMER_ICR_TBTOCINT;

    // INTERRUPT CURRENTLY UNUSED (DISABLED)
}
