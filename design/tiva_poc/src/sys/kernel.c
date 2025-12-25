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

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "sys/mm.h"
#include "sys/kernel.h"
#include "io/uart0.h"
#include "sys/asm.h"
#include "sys/svc.h"
#include "sys/clock.h"
#include "util/str.h"

extern bool time_slot;

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks

// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = true;          // preemption (true) or cooperative (false)

// tick count
uint32_t tickCount = 0;
uint8_t sleepingCount = 0;

// last run for each priority level
uint8_t lastRun[NUM_PRIORITIES] = {};

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define isRunnable(x) (tcb[x].state == STATE_READY || tcb[x].state == STATE_UNRUN)

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

void initRtos(void)
{
    uint8_t i;

    const uint32_t stCycles = 40E6/1E3 - 1;

    //Setup systick for 1ms
    NVIC_ST_RELOAD_R |= NVIC_ST_RELOAD_M & stCycles; //1ms tick
    NVIC_ST_CURRENT_R |= 0; //clear current
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC; //use system clock
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN; //enable systick interrupt
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE; //enable systick

    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
}

uint8_t rtosScheduler(void)
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    if (!priorityScheduler) // Round Robin
    {
        while (!ok)
        {
            task++;
            if (task >= MAX_TASKS)
                task = 0;
            ok = isRunnable(task);
        }
    }
    else // Priority Round Robin
    {
        uint8_t i;
        uint8_t lowestPrio = NUM_PRIORITIES - 1;
        
        // First pass, find lowest priority that is runnable
        for (i = 0; i < MAX_TASKS; i++)
        {
            if (tcb[i].currentPriority < lowestPrio && isRunnable(i)) 
            {
                lowestPrio = tcb[i].currentPriority;
                task = i; //Default in case second pass doesn't find a task
            }
        }

        // Second pass, finds next task at lowestPrio priority level
        // (modulo won't reduce performance since done on 2^n)
        for (i = lastRun[lowestPrio] + 1; i % NUM_PRIORITIES != lastRun[lowestPrio]; i++)
        {
            if (tcb[i].currentPriority == lowestPrio && isRunnable(i))
            {
                task = i;
                break;
            }
        }

    }

    lastRun[tcb[task].currentPriority] = task;

    return task;
}

void startRtos(void)
{
    //Choose task to run
    taskCurrent = rtosScheduler();

    applySramAccessMask(tcb[taskCurrent].srd);

    setPsp(tcb[taskCurrent].sp);

    // Start tracking task duration
    startCurrentTaskDuration();

    startRtosHelper(tcb[taskCurrent].pid);
}

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    bool found = false;
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}

            tcb[i].state = STATE_UNRUN;
            tcb[i].pid = fn;
            tcb[i].srd = createNoSramAccessMask();
            tcb[i].sp = mallocMemory(stackBytes, i); //malloc will update SRD appropiately
            tcb[i].priority = priority;
            tcb[i].currentPriority = priority;

            //Copy name
            _strncpy(tcb[i].name, (char *)name, 16);

            // increment task count
            taskCount++;
            ok = true;
        }
    }
    return ok;
}

void killThread_impl(_fn fn)
{
    int i;

    uint8_t taskNum;

    // Find task index
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (tcb[i].pid == fn) taskNum = i;
    }

    // Free malloced memory
    cleanupTaskMemory(taskNum);

    if (tcb[taskNum].state == STATE_DELAYED)
    { // clear sleep timer
        tcb[taskNum].ticks = 0;

        sleepingCount--;
    }
    else if (tcb[taskNum].state == STATE_BLOCKED_MUTEX)
    { // remove from mutex queue
        uint8_t mtx_num = tcb[taskNum].mutex;
        dequeue(mutexes[mtx_num].processQueue, &mutexes[mtx_num].queueSize, taskNum);
    }
    else if (tcb[taskNum].state == STATE_BLOCKED_SEMAPHORE)
    { // remove from semaphore queue
        uint8_t sem_num = tcb[taskNum].semaphore;
        dequeue(semaphores[sem_num].processQueue, &semaphores[sem_num].queueSize, taskNum);
    }

    tcb[taskNum].state = STATE_KILLED;
}

void restartThread_impl(_fn fn)
{
    uint8_t taskNum;

    // Find task index
    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (tcb[i].pid == fn) taskNum = i;
    }

    
    if (taskNum < MAX_TASKS)
    {
        // Start the program afresh
        tcb[taskNum].sp = mallocMemory(1024, taskNum); //new stack
        tcb[taskNum].state = STATE_UNRUN; //set ready to run
    }

}

void setThreadPriority_impl(_fn fn, uint8_t priority)
{
    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (tcb[i].pid == fn)
        {
            tcb[i].priority = priority;
        }
    }
}

void yield(void)
{
    asm(" svc #0\n\t");
}

void sleep(uint32_t tick)
{
    asm(" svc #1\n\t");
}

void lock(int8_t mutex)
{
    asm(" svc #2\n\t");
}

void unlock(int8_t mutex)
{
    asm(" svc #3\n\t");
}

void wait(int8_t semaphore)
{
    asm(" svc #4\n\t");
}

void post(int8_t semaphore)
{
    asm(" svc #5\n\t");
}

// read user input from uart0
void readUart(uartData *data)
{
    asm(" svc #6\n\t");
}

// read user input from uart0
void write(char *str)
{
    asm(" svc #7\n\t");
}
void reboot()
{
    asm(" svc #8\n\t");
}
void ps()
{
    asm(" svc #9\n\t");
}
void ipcs()
{
    asm(" svc #10\n\t");
}
void kill(_fn fn)
{
    asm(" svc #11\n\t");
}
void pkill(char *proc_name, uint32_t size)
{
    asm(" svc #12\n\t");
}
void pi(bool on)
{
    asm(" svc #13\n\t");
}
void preempt(bool on)
{
    asm(" svc #14\n\t");
}
void sched(bool prio_on)
{
    asm(" svc #15\n\t");
}
void pidof(char *proc_name, uint32_t size)
{
    asm(" svc #16\n\t");
}
void run(char *name, uint32_t size)
{
    asm(" svc #17\n\t");
}
void mallocHeap(uint32_t size_in_bytes)
{
    asm(" svc #18\n\t");
}
void freeHeap(void *ptr, uint32_t size)
{
    asm(" svc #19\n\t");
}
void killThread(_fn fn)
{
    asm(" svc #20\n\t");
}
void restartThread(_fn fn)
{
    asm(" svc #21\n\t");
}
void setThreadPriority(_fn fn, uint8_t priority)
{
    asm(" svc #22\n\t");
}

void systickIsr(void)
{
    tickCount++;

    //Keep track of sleeping programs
    if (sleepingCount > 0)
    {
        int i;
        for (i = 0; i < MAX_TASKS; i++)
        {
            if (tcb[i].state == STATE_DELAYED && --(tcb[i].ticks) == 0)
            {
                tcb[i].state = STATE_READY;
                sleepingCount--;
            }
        }
    }

    //Preempt processes if needed
    if (preemption) pendSvIsr();
}

//TODO: (optional) Add short-circuit if scheduler chooses
//      same process to avoid unnecessary context switch
void pendSvIsr()
{
    // called from MPU
    if (NVIC_FAULT_STAT_R & 0x03)
    {
        NVIC_FAULT_STAT_R |= 0x03; //Clear MPU status bits

        killThread_impl(tcb[taskCurrent].pid);

        putsUart0("Killed offending task with PID ");
        putIntUart0((uint32_t) tcb[taskCurrent].pid);
        putsUart0("\n");

    }

    // context switch

    // Save task time duration
    finishCurrentTaskDuration();

    /*** Store context ***/
    // Save callee regs
    saveRegPendsv();

    // Save tcb state
    tcb[taskCurrent].sp = getPsp();

    // Select next task
    taskCurrent = rtosScheduler();

    /*** Restore context ***/
    // Restore context for new task
    setPsp(tcb[taskCurrent].sp);

    // Restore memory permissions
    applySramAccessMask(tcb[taskCurrent].srd);

    //Perform first-run setup as needed
    if (tcb[taskCurrent].state == STATE_UNRUN)
    {
        tcb[taskCurrent].state = STATE_READY;
        initUnrunTask(tcb[taskCurrent].pid);
    }
    else
    {
        restoreRegPendsv();
    }

    // Start tracking task duration
    startCurrentTaskDuration();
}

void triggerPendSv(void)
{
    //Trigger pendSV interrupt
    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV; //Set PENDSV pending
}

// Ensures memory is owned by task who passed addr, and kills it if not
// Returns whether the memory is owned by the task.
bool ensurePointer(void *addr, uint32_t size)
{
    bool valid = true;
    void *endAddr = (void *) ((uint32_t)addr + size - 1);

    // Check owner of beginning and end
    valid &= getMemoryOwner(addr) == tcb[taskCurrent].pid;
    valid &= getMemoryOwner(endAddr) == tcb[taskCurrent].pid;

    // Prevent overflow attack
    valid &= endAddr >= addr;

    // Kill offending task
    if (!valid) killThread_impl(tcb[taskCurrent].pid);

    return valid;
}

void svCallIsr(void)
{
    uint8_t svcNum = getSvcNum();
    union svc_param param = getSvcParam();
    union svc_param2 param2 = getSvcParam2();

    switch (svcNum)
    {
        case SVC_YIELD: yield_impl(); break;
        case SVC_SLEEP: sleep_impl(param.uint32); break;
        case SVC_LOCK: lock_impl(param.uint8); break;
        case SVC_UNLOCK: unlock_impl(param.uint8); break;
        case SVC_WAIT: wait_impl(param.uint8); break;
        case SVC_POST: post_impl(param.uint8); break;

        case SVC_READUART: readUart_impl(param.uart); break;
        case SVC_WRITE:
            if (ensurePointer(param.str, param2.size)) write_impl(param.str);
            break;
        case SVC_REBOOT: reboot_impl(); break;
        case SVC_PS: ps_impl(); break;
        case SVC_IPCS: ipcs_impl(); break;
        case SVC_KILL: kill_impl(param.fn); break;
        case SVC_PKILL: 
            if (ensurePointer(param.str, param2.size)) pkill_impl(param.str);
            break;
        case SVC_PI: pi_impl(param.boolVal); break;
        case SVC_PREEMPT: preempt_impl(param.boolVal); break;
        case SVC_SCHED: sched_impl(param.boolVal); break;
        case SVC_PIDOF:
            if (ensurePointer(param.str, param2.size)) pidof_impl(param.str);
            break;
        case SVC_RUN: 
            run_impl(param.str);
            break;
        case SVC_MALLOC: mallocHeap_impl(param.uint32); break;
        case SVC_FREE:
            if (ensurePointer(param.voidPtr, param2.size)) freeHeap_impl(param.voidPtr);
            break;
        case SVC_KILLTHREAD:
            killThread_impl(param.fn);
            break;
        case SVC_RESTARTTHREAD:
            restartThread_impl(param.fn);
            break;
        case SVC_SETTHREADPRIORITY:
            setThreadPriority_impl(param.fn, param2.priority);
            break;
    }
}
