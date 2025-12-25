// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef KERNEL_H_
#define KERNEL_H_

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// mutex
#define MAX_MUTEXES 1
#define MAX_MUTEX_QUEUE_SIZE 2
#define resource 0

// semaphore
#define MAX_SEMAPHORES 3
#define MAX_SEMAPHORE_QUEUE_SIZE 2
#define keyPressed 0
#define keyReleased 1
#define flashReq 2

// tasks
#define MAX_TASKS 12

// task states (moved from kernel.c)
#define STATE_INVALID           0 // no task
#define STATE_UNRUN             1 // task has never been run
#define STATE_READY             2 // has run, can resume at any time
#define STATE_DELAYED           3 // has run, but now awaiting timer
#define STATE_BLOCKED_SEMAPHORE 4 // has run, but now blocked by semaphore
#define STATE_BLOCKED_MUTEX     5 // has run, but now blocked by mutex
#define STATE_KILLED            6 // task has been killed

// tcb (copied from kernel.c)
#define NUM_PRIORITIES   8

struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    _fn pid;                     // used to uniquely identify thread (address of task fn)
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t ticks;                // ticks until sleep complete
    uint64_t srd;                  // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
    uint32_t cpu_time[2];          // CPU time, with two slots
} tcb[MAX_TASKS];

// mutex
typedef struct _mutex
{
    bool lock;
    uint8_t queueSize;
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
    uint8_t lockedBy;
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    uint8_t count;
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

typedef struct _uartData 
{
    bool isRxFull;
    bool isRxEmpty;
    bool isTxFull;
    bool isTxEmpty;
    
    uint32_t error;
    uint8_t value;
} uartData;

extern uint8_t taskCurrent;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex);
bool initSemaphore(uint8_t semaphore, uint8_t count);

void initRtos(void);
void startRtos(void);

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes);
void killThread_impl(_fn fn);
void restartThread_impl(_fn fn);
void setThreadPriority_impl(_fn fn, uint8_t priority);

void yield(void);
void sleep(uint32_t tick);
void wait(int8_t semaphore);
void post(int8_t semaphore);
void lock(int8_t mutex);
void unlock(int8_t mutex);
void reboot();
void ps();
void ipcs();
void kill(_fn fn);
void pkill(char *proc_name, uint32_t size);
void pi(bool on);
void preempt(bool on);
void sched(bool prio_on);
void pidof(char *, uint32_t);
void run(char *, uint32_t);
void killThread(_fn fn);
void restartThread(_fn fn);
void setThreadPriority(_fn fn, uint8_t priority);

void readUart(uartData *);

void triggerPendSv(void);

bool ensurePointer(void *, uint32_t);

void systickIsr(void);
void pendSvIsr(void);
void svCallIsr(void);

#endif
