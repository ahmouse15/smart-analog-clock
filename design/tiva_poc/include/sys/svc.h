// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)
//
#ifndef SYS_SVC_H
#define SYS_SVC_H

#include "sys/kernel.h"
#include "util/interface.h"

// svc call numbers
#define SVC_YIELD (uint8_t)0
#define SVC_SLEEP (uint8_t)1
#define SVC_LOCK (uint8_t)2
#define SVC_UNLOCK (uint8_t)3
#define SVC_WAIT (uint8_t)4
#define SVC_POST (uint8_t)5

#define SVC_READUART (uint8_t)6
#define SVC_WRITE (uint8_t)7
#define SVC_REBOOT (uint8_t)8
#define SVC_PS (uint8_t)9
#define SVC_IPCS (uint8_t)10
#define SVC_KILL (uint8_t)11
#define SVC_PKILL (uint8_t)12
#define SVC_PI (uint8_t)13
#define SVC_PREEMPT (uint8_t)14
#define SVC_SCHED (uint8_t)15
#define SVC_PIDOF (uint8_t)16
#define SVC_RUN (uint8_t)17
#define SVC_MALLOC (uint8_t)18
#define SVC_FREE (uint8_t)19
#define SVC_KILLTHREAD (uint8_t)20
#define SVC_RESTARTTHREAD (uint8_t)21
#define SVC_SETTHREADPRIORITY (uint8_t)22

union svc_param {
    uint8_t uint8;
    uint32_t uint32;
    _fn fn;
    void *voidPtr;
    char *str;
    bool boolVal;
    USER_DATA *readData;
    uartData *uart;
};

union svc_param2
{
    uint32_t size;
    uint8_t priority;
};

uint8_t dequeue(uint8_t *, uint8_t *, uint8_t);

void yield_impl(void);
void sleep_impl(uint32_t);
void lock_impl(uint8_t);
void unlock_impl(uint8_t);
void wait_impl(uint8_t);
void post_impl(uint8_t);

// Shell functions
void readUart_impl(uartData *);
void write_impl(char *);
void reboot_impl(void);
void ps_impl(void);
void ipcs_impl(void);
void kill_impl(_fn);
void pkill_impl(char *);
void pi_impl(bool);
void preempt_impl(bool);
void sched_impl(bool);
void pidof_impl(char *);
void run_impl(char *name);
void mallocHeap(uint32_t);
void freeHeap(void *, uint32_t);

#endif
