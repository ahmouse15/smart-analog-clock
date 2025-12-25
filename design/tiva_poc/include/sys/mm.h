// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sys/kernel.h"

#define MAX_BLOCKS 28
#define SRAM_BASE 0x20000000
#define HEAP_BASE 0x20001000
#define HEAP_TOP 0x20008000
#define BLOCK_SIZE 0x400

typedef struct heap_block
{
    bool isUsed; //Whether block is currently allocated or not
    _fn pid; //PID of owner of this region
    uint16_t len; //Length of allocation (only nonzero for initial block)
} heap_block;

void mpu_init();

void removeSramAccessWindow(uint64_t *, uint32_t *, uint32_t );
void addSramAccessWindow(uint64_t *, uint32_t *, uint32_t);
uint64_t createNoSramAccessMask(void);
void applySramAccessMask(uint64_t);
void cleanupTaskMemory(uint8_t);

void * mallocMemory(uint32_t, uint8_t);
void * mallocHeap_impl(uint32_t);
void freeHeap_impl(void *);
void initMemoryManager(void);
void initMpu(void);

uint32_t getHeapIndex(void *addr);
// Returns task id of task that owns the given memory
_fn getMemoryOwner(void *addr);

void mpuEnable();

#endif
