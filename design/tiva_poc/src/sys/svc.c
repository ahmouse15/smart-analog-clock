// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)

#include <stdbool.h>
#include <stdint.h>

#include "tm4c123gh6pm.h"

#include "sys/clock.h"
#include "sys/svc.h"
#include "sys/kernel.h"
#include "util/interface.h"
#include "io/uart0.h"
#include "util/str.h"

extern uint32_t sleepingCount;

extern bool priorityScheduler;
extern bool priorityInheritance;
extern bool preemption;

uint8_t dequeue(uint8_t *queue, uint8_t *size, uint8_t index)
{
    int i;

    uint8_t ret = queue[index];

    // Shift queue forward
    for (i = index+1; i < *size; i++)
    {
        queue[i-1] = queue[i];
    }
    
    // Decrement queueSize
    (*size)--;

    return ret;
}

void yield_impl(void)
{
    triggerPendSv();
}

void sleep_impl(uint32_t tick)
{
    tcb[taskCurrent].state = STATE_DELAYED;
    tcb[taskCurrent].ticks = tick;
    sleepingCount++;
    
    triggerPendSv();
}

void lock_impl(uint8_t mtx_num)
{

    if (!mutexes[mtx_num].lock)
    {
        mutexes[mtx_num].lock = true;
        mutexes[mtx_num].lockedBy = taskCurrent;
        tcb[taskCurrent].mutex = mtx_num;
    }
    else
    {
        const int i = mutexes[mtx_num].queueSize;

        // Priority inheritance
        // If owner has lower priority than task trying to lock, elevate owner priority
        if (priorityInheritance && tcb[taskCurrent].priority < tcb[mutexes[mtx_num].lockedBy].priority)
        {
            tcb[mutexes[mtx_num].lockedBy].currentPriority = tcb[taskCurrent].currentPriority;
        }

        mutexes[mtx_num].processQueue[i] = taskCurrent;
        mutexes[mtx_num].queueSize++;

        tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
        tcb[taskCurrent].mutex = mtx_num;

        triggerPendSv();
    }

}

void unlock_impl(uint8_t mtx_num) 
{
    // Reset priority in case of inheritance
    tcb[mutexes[mtx_num].lockedBy].currentPriority = tcb[mutexes[mtx_num].lockedBy].priority;

    if (mutexes[mtx_num].queueSize > 0)
    {
        uint32_t procNum = dequeue(mutexes[mtx_num].processQueue, &mutexes[mtx_num].queueSize, 0);
        tcb[procNum].state = STATE_READY;
        
        mutexes[mtx_num].lockedBy = procNum;
    }
    else
    {
        mutexes[mtx_num].lock = false;
        mutexes[mtx_num].lockedBy = 0;
    }
}

void wait_impl(uint8_t sem_num) 
{
    //Semaphore available
    if (semaphores[sem_num].count > 0)
    {
        semaphores[sem_num].count--;
        tcb[taskCurrent].semaphore = sem_num;
    }
    else //No semaphore available (block until available)
    {
        const int i = semaphores[sem_num].queueSize;
        semaphores[sem_num].processQueue[i] = taskCurrent;
        semaphores[sem_num].queueSize++;

        tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;
        tcb[taskCurrent].semaphore = sem_num;

        triggerPendSv();
    }
}

void post_impl(uint8_t sem_num) 
{

    if (semaphores[sem_num].queueSize > 0)
    {
        const uint8_t procNum = dequeue(semaphores[sem_num].processQueue, &semaphores[sem_num].queueSize, 0);

        tcb[procNum].state = STATE_READY;
    }
    else 
    {
        semaphores[sem_num].count++;
    }
}

// Shell functions
// read user input from uart0
void readUart_impl(uartData *data)
{
    data->isRxFull = UART0_FR_R & UART_FR_RXFF;
    data->isRxEmpty = UART0_FR_R & UART_FR_RXFE;
    data->isTxFull = UART0_FR_R & UART_FR_TXFF;
    data->isTxEmpty = UART0_FR_R & UART_FR_TXFE;

    data->error = UART0_RSR_R;
    data->value = UART0_DR_R & 0xFF;
}

void write_impl(char *str)
{
    //TODO: Memory bounds/ownership check
    putsUart0(str);
}
void reboot_impl(void)
{
    putsUart0("REBOOTING\n");
    NVIC_APINT_R = NVIC_APINT_SYSRESETREQ | NVIC_APINT_VECTKEY;
}
void ps_impl(void)
{
    int i;
    const int fieldSize = 18;

    uint32_t kernel_rawtime = 0;

    uint8_t integer, fraction;

    // Field names
    putFieldUart0("pid", fieldSize);
    putFieldUart0("name", fieldSize);
    putFieldUart0("state", fieldSize);
    putFieldUart0("sleep time", fieldSize);
    putFieldUart0("blocked on", fieldSize);
    putFieldUart0("%CPU", fieldSize);
    putsUart0("\n");

    for (i = 0; i < MAX_TASKS; i++)
    {
        //Longest state name is 17 chars
        char stateStr[18] = "INVALID";
        int stateStrSize = 18;

        if (tcb[i].pid)
        {
            putIntFieldUart0((uint32_t) tcb[i].pid, fieldSize);
            putFieldUart0(tcb[i].name, fieldSize);

            // Display state as string
            switch(tcb[i].state)
            {
                case STATE_INVALID:             
                    _strncpy(stateStr,"INVALID", stateStrSize-1); break;
                case STATE_UNRUN:               
                    _strncpy(stateStr,"UNRUN", stateStrSize-1); break;
                case STATE_READY:               
                    _strncpy(stateStr,"READY", stateStrSize-1); break; 
                case STATE_DELAYED:             
                    _strncpy(stateStr,"DELAYED", stateStrSize-1); break;
                case STATE_BLOCKED_SEMAPHORE:   
                    _strncpy(stateStr,"BLOCKED_SEMAPHORE", stateStrSize-1); break;
                case STATE_BLOCKED_MUTEX:       
                    _strncpy(stateStr,"BLOCKED_MUTEX", stateStrSize-1); break;
                case STATE_KILLED:             
                     _strncpy(stateStr,"KILLED", stateStrSize-1); break;
            }
            putFieldUart0(stateStr, fieldSize);

            if (tcb[i].state == STATE_DELAYED) putIntFieldUart0(tcb[i].ticks, fieldSize);
            else putFieldUart0("", fieldSize);

            // Display blocking resource
            if (tcb[i].state == STATE_BLOCKED_MUTEX)
            {
                char temp[9] = "mutex[";
                temp[6] = _itoc(tcb[i].mutex);
                temp[7] = ']';

                putFieldUart0(temp, fieldSize);
            }
            else if (tcb[i].state == STATE_BLOCKED_SEMAPHORE)
            {
                char temp[13] = "semaphore[";
                temp[10] = _itoc(tcb[i].semaphore);
                temp[11] = ']';

                putFieldUart0(temp, fieldSize);
            }
            else putFieldUart0("", fieldSize);

            // Display CPU time
            getCpuTimeAsPercent(i, &integer, &fraction);

            kernel_rawtime += getCpuTime(i);

            putIntUart0(integer);
            putsUart0(".");
            putIntUart0(fraction);

            putsUart0("\n");
        }
        
    }

    // Kernel time = remainder of time after subtracting all other programs
    convertCpuTimeToPercent(10000-kernel_rawtime, &integer, &fraction);

    putFieldUart0("", fieldSize);
    putFieldUart0("kernel", fieldSize);
    putFieldUart0("", 3*fieldSize);

    // Display kernel CPU time
    putIntUart0(integer);
    putsUart0(".");
    putIntUart0(fraction);

    putsUart0("\n");
    
}
void ipcs_impl(void)
{
    int i;
    const int fieldSize = 16;
    
    // Field names
    putsUart0("------ Mutexes ------\n");
    putFieldUart0("index", fieldSize);
    putFieldUart0("locked", fieldSize);
    putFieldUart0("held by", fieldSize);
    putFieldUart0("queue size", fieldSize);
    putFieldUart0("waiting", fieldSize);
    putsUart0("\n");

    // Field values
    for (i = 0; i < MAX_MUTEXES; i++)
    {
        // Index
        putIntFieldUart0(i, fieldSize);

        // Whether locked or not
        putFieldUart0(mutexes[i].lock ? "true" : "false", fieldSize);

        // Who's locking this mutex
        if (mutexes[i].lockedBy < MAX_TASKS)
        {
            putFieldUart0(tcb[mutexes[i].lockedBy].name, fieldSize);
        }
        else
        {
            putFieldUart0("N/A", fieldSize);
        }

        // Queue size
        putIntFieldUart0(mutexes[i].queueSize, fieldSize);

        // Tasks in queue
        int j;
        if (mutexes[i].queueSize > 0)
        {
            putsUart0(tcb[mutexes[i].processQueue[0]].name);
            for (j = 1; j < mutexes[i].queueSize; j++)
            {
                putsUart0(", ");
                putsUart0(tcb[mutexes[i].processQueue[0]].name);
            }
        }
        
        putsUart0("\n");
    }

    // Separator
    putsUart0("\n");

    // Semaphores
    putsUart0("------ Semaphores ------\n");
    putFieldUart0("index", fieldSize);
    putFieldUart0("count", fieldSize);
    putFieldUart0("queue size", fieldSize);
    putFieldUart0("waiting", fieldSize);
    putsUart0("\n");

    // Field values
    for (i = 0; i < MAX_SEMAPHORES; i++)
    {
        // Index
        putIntFieldUart0(i, fieldSize);

        // Sema count
        putIntFieldUart0(semaphores[i].count, fieldSize);

        // Queue size
        putIntFieldUart0(semaphores[i].queueSize, fieldSize);

        // Tasks in queue
        int j;
        if (semaphores[i].queueSize > 0)
        {
            putsUart0(tcb[semaphores[i].processQueue[0]].name);
            for (j = 1; j < semaphores[i].queueSize; j++)
            {
                putsUart0(", ");
                putsUart0(tcb[semaphores[i].processQueue[0]].name);
            }
        }

        putsUart0("\n");   
    }

}
void kill_impl(_fn pid)
{
    uint8_t taskNum = UINT8_MAX;

    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (tcb[i].pid == pid)
        {
            taskNum = i;
        }
    }

    putsUart0("Task with PID ");
    putIntUart0((uint32_t)pid);
    putsUart0(" ");

    if (taskNum < MAX_TASKS)
    {
        if (tcb[taskNum].state != STATE_KILLED && tcb[taskNum].state != STATE_UNRUN)
        { // OK
            killThread_impl(tcb[taskNum].pid);

            putsUart0("was killed successfully");
        }
        else // task is not running
        {
            putsUart0("is not running");
        }
    }
    else // no such task exists
    {
        putsUart0("does not exist");
    }

    putsUart0("\n");
}
void pkill_impl(char *proc_name)
{
    uint8_t taskNum = UINT8_MAX;

    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (_strcmp(tcb[i].name, proc_name) == 0)
        {
            taskNum = i;
        }
    }

    putsUart0("Task with name \"");
    putsUart0(proc_name);
    putsUart0("\" ");

    if (taskNum < MAX_TASKS)
    {
        if (tcb[taskNum].state != STATE_KILLED && tcb[taskNum].state != STATE_INVALID)
        { // OK
            killThread_impl(tcb[taskNum].pid);

            putsUart0("was killed successfully");
        }
        else // task is not running
        {
            putsUart0("is not running");
        }
    }
    else // no such task exists
    {
        putsUart0("does not exist");
    }

    putsUart0("\n");
}
void pi_impl(bool on)
{
    if (on)
    {
        putsUart0("pi on\n");
        priorityInheritance = true;
    }
    else
    {
        putsUart0("pi off\n");
        priorityInheritance = false;
    }
}
void preempt_impl(bool on)
{
    if (on)
    {
        putsUart0("preempt on\n");
        preemption = true;
    }
    else
    {
        putsUart0("preempt off\n");
        preemption = false;
    }
}
void sched_impl(bool prio_on)
{
    if (prio_on)
    {
        putsUart0("sched prio\n");
        priorityScheduler = true;
    }
    else
    {
        putsUart0("sched rr\n");
        priorityScheduler = false;
    }
}
void pidof_impl(char *proc_name)
{
    uint32_t pid = 0;

    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (!_strcmp(proc_name, tcb[i].name))
        {
            pid = (uint32_t) tcb[i].pid;
            break;
        }
    }

    if (!pid)
    {
        putsUart0("No process named ");
        putsUart0(proc_name);
    }
    else putIntUart0(pid);
    
    putsUart0("\n");
}

void run_impl(char *name)
{
    int i;
    bool found = false;
    for (i = 0; i < MAX_TASKS; i++)
    {
        if (_strcmp(name, tcb[i].name) == 0)
        {
            found = true;
            if (tcb[i].state != STATE_KILLED)
            {
                //task already running
                putsUart0("Task \"");
                putsUart0(name);
                putsUart0("\" is already running.\n");
            }
            else {
                // restart task
                putsUart0("Restarted thread named \"");
                putsUart0(name);
                putsUart0("\"\n");

                restartThread_impl(tcb[i].pid);

            }

            break;
        }
    }

    if (!found)
    {
        putsUart0("No task named \"");
        putsUart0(name);
        putsUart0("\"\n");
    }

}
