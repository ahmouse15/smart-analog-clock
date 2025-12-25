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

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdint.h>

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initSystemClockTo40Mhz(void);

void initTimers();
void startCurrentTaskDuration();
uint32_t getCurrentTaskDuration();
void finishCurrentTaskDuration();

uint32_t getCpuTime(uint8_t task);
void convertCpuTimeToPercent(uint32_t rawtime, uint8_t *integer, uint8_t *fraction);
void getCpuTimeAsPercent(uint8_t task, uint8_t *integer, uint8_t *fraction);

void wTimer0AIsr(void);
void wTimer0BIsr(void);

#endif
