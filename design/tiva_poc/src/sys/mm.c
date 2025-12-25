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

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "sys/mm.h"
#include "sys/kernel.h"

#define MAX_BLOCKS 28
#define SRAM_BASE 0x20000000

//1-4 are SRAM regions
#define SRAM_BOTTOM_REGION 1
#define NUM_SRAM_REGIONS 4

//8KiB each
#define SRAM_REGION_SIZE 0x2000

#define SRAM_DEFAULT_REGION_NUM 0
#define FLASH_REGION_NUM 5
#define PERIPHERAL_REGION_NUM 6
#define SRAM_KERNEL_REGION_NUM 7

//Increasing heap_table index => decreasing memory address
//heap_alloc_table[0] = HEAP_TOP = 0x20008000
//heap_alloc_table[MAX_BLOCKS] = HEAP_BASE = 0x20001000

heap_block heap_alloc_table[MAX_BLOCKS] = {0};


void * mallocMemory(uint32_t size_in_bytes, uint8_t taskNum)
{
    //Grow from bottom to top.
    //Bottom is reserved for kernel up to 0x20001000
    const uint16_t num_blocks = (size_in_bytes-1)/BLOCK_SIZE + 1;

    void *ret = NULL;

    uint16_t count = 0, prevCount = UINT16_MAX, countTemp = 0;
    int16_t start = -1, startTemp = 0;

    bool curr = false;

    //Find best fit continuous section
    int i;
    for (i = 0; i < MAX_BLOCKS; i++)
    {
        if (heap_alloc_table[i].isUsed == false && !curr)
        {
            curr = true;

            countTemp++;
            startTemp = i;
        }
        else if (heap_alloc_table[i].isUsed == false && curr)
        {
            countTemp++;
        }

        if ((heap_alloc_table[i].isUsed == true || i == MAX_BLOCKS-1) && curr)
        {
            curr = false;

            if (countTemp < prevCount && countTemp >= num_blocks)
            {
                prevCount = count;

                count = countTemp;
                start = startTemp;
            }

            countTemp = 0;
            startTemp = 0;
        }

    }

    //Only return if we could successfully allocate; o/w defaults to NULL
    if (count >= num_blocks)
    {
        //Mark discovered blocks as allocated
        for (i = start; i < start+num_blocks; i++)
        {
            heap_alloc_table[i].isUsed = true;
            heap_alloc_table[i].pid = tcb[taskNum].pid;
        }

        //Mark length of base allocation
        heap_alloc_table[start].len = num_blocks;

        ret = (void *)(HEAP_TOP - (start*BLOCK_SIZE));

        //Add region to MPU SRD mask
        addSramAccessWindow(&(tcb[taskNum].srd), ret, size_in_bytes);
    }


    return ret;
}

void * mallocHeap_impl(uint32_t size_in_bytes)
{
    void *ret = NULL;

    ret = mallocMemory(size_in_bytes, taskCurrent);
    
    applySramAccessMask(tcb[taskCurrent].srd);

    return ret;
}

void freeMemory(uint8_t index, uint8_t taskNum)
{
    //Remove memory access
    removeSramAccessWindow(&(tcb[taskNum].srd), (uint32_t *)(HEAP_TOP-index*BLOCK_SIZE), heap_alloc_table[index].len*BLOCK_SIZE);

    //Mark all blocks as free (no need to clear PID field)
    int i;
    for (i = index; i < index+heap_alloc_table[index].len; i++)
    {
        heap_alloc_table[i].isUsed = false;
    }

    //Clear len field
    heap_alloc_table[index].len = 0;
}

void freeHeap_impl(void *address_from_malloc)
{
    uint32_t start_index = getHeapIndex(address_from_malloc);

    if (heap_alloc_table[start_index].pid == tcb[taskCurrent].pid)
    {
        freeMemory(start_index, taskCurrent);
        applySramAccessMask(tcb[taskCurrent].srd);
    }
}

// Free all memory allocated by the task
void cleanupTaskMemory(uint8_t taskNum)
{
    _fn pid = tcb[taskNum].pid;

    int i;
    // Search through all memory blocks
    for (i = 0; i < MAX_BLOCKS; i++)
    {
        // Free blocks owned by this pid
        if (heap_alloc_table[i].pid == pid && heap_alloc_table[i].isUsed
            && heap_alloc_table[i].len > 0)
        {
            freeMemory(i, taskNum);
        }
    }
}

// Returns index of the memory block that addr resides in
uint32_t getHeapIndex(void *addr)
{
    return (HEAP_TOP - (uint32_t)(addr))/BLOCK_SIZE;
}

// Returns pid of task that owns the given memory
_fn getMemoryOwner(void *addr)
{
    return heap_alloc_table[getHeapIndex(addr)].pid;
}

void initMemoryManager(void)
{

}

/*Creates a full-access MPU aperture for flash with
RWX access for both privileged and unprivileged access*/
void allowFlashAccess(void)
{
    //Select correct region number
    NVIC_MPU_NUMBER_R = FLASH_REGION_NUM;
    //Enable region
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;
    asm(" dsb\n\t"
        " isb");
}

/*
Creates a full-access MPU aperture to
peripherals and peripheral bitbanded addresses with RW access for privileged and unprivileged access.
If you choose to leave the all access rule, then this is not needed
*/
void allowPeripheralAccess(void)
{
    //Select correct region number
    NVIC_MPU_NUMBER_R = PERIPHERAL_REGION_NUM;
    //Enable region
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;
    asm(" dsb\n\t"
        " isb");
}

/*
Creates multiple MPUs regions to cover the 32KiB
SRAM (each MPU region covers 8KiB with 8 subregions of 1KiB each with RW access
for unprivileged mode.
*/
void setupSramAccess(void)
{
    int i;
    for (i = 0; i < NUM_SRAM_REGIONS; i++)
    {
        //Setup SRAM regions
        NVIC_MPU_NUMBER_R = i+SRAM_BOTTOM_REGION;
        //Clear BASE first
        NVIC_MPU_BASE_R &= ~(0xFFFFFFFF << 0xD);
        NVIC_MPU_BASE_R |= SRAM_BASE + SRAM_REGION_SIZE*i;
        NVIC_MPU_ATTR_R |= 0xC << 1; //Size = 2^(12+1) = 8KiB
        NVIC_MPU_ATTR_R |= 0x3 << 24; //Full permissions, but subregions disabled
        NVIC_MPU_ATTR_R |= 0xFF << 8;//Disable all subregions
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE; //Enable region
    }

    asm(" dsb\n\t"
        " isb\n\t");
}


//Returns the values of the srdBits to allow no access to SRAM
uint64_t createNoSramAccessMask(void)
{
    return 0xFFFFFFFF;
}

//Applies the SRD bits to the MPUregions
void applySramAccessMask(uint64_t srdBitMask)
{
    int i;
    for (i = 0; i < NUM_SRAM_REGIONS; i++)
    {
        //Select region to modify
        NVIC_MPU_NUMBER_R = i+SRAM_BOTTOM_REGION;
        //Set respective SRD bits
        NVIC_MPU_ATTR_R &= ~(0xFF << 8);
        NVIC_MPU_ATTR_R |= ((srdBitMask >> i*8) & 0xFF) << 8;
    }
    asm(" dsb\n\t"
        " isb\n\t");
}

//Removes access to the requested SRAM address range.
void removeSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{
    //Each SRD bit corresponds to 1024 bytes,
    uint32_t offset = ((uint32_t)baseAdd-SRAM_BASE)/1024;
    uint32_t length = ((uint32_t)size_in_bytes-1) / 1024 + 1;

    //Set corresponding bits
    int i;
    for (i = 0; i < length; i++) *srdBitMask |= (0x1 << (offset-1 - i));
    
    asm(" dsb\n\t"
        " isb\n\t");
}

//Adds access to the requested SRAM address range.
void addSramAccessWindow(uint64_t *srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes)
{
    //Each SRD bit corresponds to 1024 bytes,
    uint32_t offset = ((uint32_t)baseAdd-SRAM_BASE)/1024;
    uint32_t length = ((uint32_t)size_in_bytes-1) / 1024 + 1;

    //Set corresponding bits
    int i;
    for (i = 0; i < length; i++) *srdBitMask &= ~(0x1 << (offset-1 - i));
    
    asm(" dsb\n\t"
        " isb\n\t");
}

void initMpu(void)
{
    asm(" dmb\n\t"
        " isb\n\t");
    
    //Enable background rule for privileged apps
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_PRIVDEFEN;

    //Setup FLASH region 
    NVIC_MPU_NUMBER_R = FLASH_REGION_NUM;
    NVIC_MPU_BASE_R |= 0x00000000; //N = 20 = log2(size)-1
    NVIC_MPU_ATTR_R |= 0x11 << 1 ; //Set SIZE field to 256KiB (entire flash)
    NVIC_MPU_ATTR_R |= 0x2 << 24; //Set permission to privileged RWX, unprivileged R-X
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); //Enable all subregions
    //Region kept disabled by default

    //Setup PERIPHERAL region
    NVIC_MPU_NUMBER_R = PERIPHERAL_REGION_NUM;
    NVIC_MPU_BASE_R |= 0x40000000; //N = 20 = log2(size)
    NVIC_MPU_ATTR_R |= 0x19 << 1 ; //Set SIZE field to 2^(25+1) = 64 MiB
    NVIC_MPU_ATTR_R |= 0x3 << 24; ///Set permission to full RW access
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); //Enable all subregions
    NVIC_MPU_ATTR_R |= 1 << 28; //Set XN (No execution)
    //Region kept disabled by default

    //Setup default SRAM region (background region)
    NVIC_MPU_NUMBER_R = SRAM_DEFAULT_REGION_NUM;
    NVIC_MPU_BASE_R |= SRAM_BASE; // N = 0xF
    NVIC_MPU_ATTR_R |= 0xE << 1; //Size = 2^(14+1) = 32KiB
    NVIC_MPU_ATTR_R |= 0x1 << 24; //Privilege RW
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); //Enables all subregions
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE; //Enable region

    //Setup kernel SRAM region (full privileged access)
    NVIC_MPU_NUMBER_R = SRAM_KERNEL_REGION_NUM;
    NVIC_MPU_BASE_R |= SRAM_BASE; // N = 0xF
    NVIC_MPU_ATTR_R |= 0xB << 1; //Size = 2^(11+1) = 4KiB
    NVIC_MPU_ATTR_R |= 0x1 << 24; //Privilege access only
    NVIC_MPU_ATTR_R &= ~(0xFF << 8); //Enable all subregions
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE; //Enable region

    setupSramAccess();

    allowFlashAccess();
    allowPeripheralAccess();
}

void mpuEnable()
{
    //Enable MPU
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;
    
    asm(" dsb\n\t"
        " isb\n\t");
}
