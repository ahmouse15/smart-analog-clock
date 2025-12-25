// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// Shell functions
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
#include "tm4c123gh6pm.h"
#include "io/uart0.h"
#include "sys/faults.h"
#include "util/str.h"
#include "sys/kernel.h"
#include "sys/asm.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

char regNames[][5] = { "R0", "R1", "R2", "R3", "R12", "LR", "PC", "xPSR"};

void printStackFrame(void)
{
    uint32_t *stack = getPsp();

    putsUart0("SP: ");
    putHexUart0((uint32_t) stack);
    putcUart0('\n');
    
    putsUart0("Offending instruction: ");
    putHexUart0(stack[6]);
    putcUart0('\n');
    
    int8_t i;
    for (i = 0; i < 8; i++) {

        putsUart0(regNames[i]);
        putsUart0(": ");
        putHexUart0(stack[i]);
        putcUart0('\n');
    }
}

uint8_t getMfaultFlags(void)
{
    return *((uint8_t *) 0xE000ED28);
}

uint32_t getBusFaultAddr(void)
{
    return *((uint32_t *) 0xE000ED38);
}

void printBusFaultAddr(void)
{
    uint32_t addr = getBusFaultAddr();

    putsUart0("bus fault addr: ");
    putHexUart0(addr);
    putcUart0('\n');
}

uint32_t getMemFaultAddr(void)
{
    return NVIC_MM_ADDR_R;
}

void printMemFaultAddr(void)
{
    uint32_t addr = getMemFaultAddr();

    putsUart0("memory fault addr: ");
    putHexUart0(addr);
    putcUart0('\n');
}

//Print PSP, MSP, and MFAULSTAT
void printDebugRegs(void)
{
        
    uint32_t *psp, *msp, mfault;

    //PSP
    psp = getPsp();
    putsUart0("PSP: ");
    putHexUart0((uint32_t)psp);
    putcUart0('\n');

    //MSP
    msp = getMsp();
    putsUart0("MSP: ");
    putHexUart0((uint32_t)msp);
    putcUart0('\n');

    //MFAULTSTAT
    mfault = getMfaultFlags();
    putsUart0("mfaultstat flags: ");
    putHexUart0(mfault);
    putcUart0('\n');
}

void mpuFaultIsr(void)
{
    //Clear MPU pend bit
    NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_MEMP; //Clear MPU pending bit

    putsUart0("MPU fault in process ");
    putIntUart0((uint32_t) tcb[taskCurrent].pid);
    putsUart0("\n");

    printDebugRegs();
    printMemFaultAddr();

    printStackFrame();

    //Trigger PendSV interrupt
    triggerPendSv();

    putcUart0('\n');
}

void hardFaultIsr(void)
{
    putsUart0("Hard fault in process ");
    putsUart0("\n");
    while(1);
}

void busFaultIsr(void)
{
    putsUart0("Bus fault in process ");
    putsUart0("\n\n");

    while(1);
}

void usageFaultIsr(void)
{
    putsUart0("Usage fault in process ");
    putsUart0("\n\n");

    while(1);
}
