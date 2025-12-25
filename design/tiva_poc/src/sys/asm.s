; Ahmed Abdulla
; Copyright 2025 Ahmed Abdulla. All Rights Reserved.
; (Excluding work produced by Professor Jason Losh)
;
; All assembly functions are located here
; This serves as a bridge between C and assembly
; See include/sys/asm.h for C-syntax function prototypes

    .thumb
    ;.align 2
    .global setTmpl, setAsp, setPspAddress, setPsp, startRtosHelper
    .global getSvcNum, getSvcParam, getSvcParam2
    .global getPsp, getSp, getMsp
    .global initUnrunTask, saveRegPendsv, restoreRegPendsv, dummyFn

saveRegPendsv:
    mrs r0, psp
    stmfd r0!, {r4-r11}
    msr psp, r0
    bx lr

restoreRegPendsv:
    mrs r0, psp
    ldmfd r0!, {r4-r11}
    msr psp, r0
    bx lr

; void initUnrunTask(void *fn);
; fn = r0
;
initUnrunTask:
    mrs r1, psp
    
    mov r2, #0x01000000

    ; Create caller stack frame
    stmfd r1!, {r2} ; Store xPSR
    stmfd r1!, {r0} ; Store fn addr to PC
    stmfd r1!, {r4-r9} ; Dummy values for the rest of caller stack frame

    msr psp, r1
    bx lr

; Helper function to switch to unprivileged state and run task. Never returns
; void startRtosHelper(void * fn)
; fn = r0 (function of task to run)
startRtosHelper:
    ; set asp and tmpl (bits 0 and 1)
    mrs r1, control
    orr r1, #3 
    msr control, r1
    ; Call fn
    mov pc, r0
    bx lr

; uint32_t *getPsp(void)
getPsp:
    mrs r0, psp
    bx lr

; uint32_t *getSp(void)
getSp:
    mov r0, sp
    bx lr

; uint32_t *getMsp(void)
getMsp:
    mrs r0, msp
    bx lr

; void setPsp(void *ptr)
; ptr = r0
setPsp:
    msr psp, r0
    bx lr    

; void setAsp(bool on)
; on = r0
; asp = bit 1
setAsp:
    mrs r1, control
    ;clear asp bit
    mvn r2, #2
    and r1, r2
    ;set asp bit according to "on"
    orr r1, r0, lsl #1
    msr control, r1
    bx lr

; void setTmpl(bool on)
; on = r0
; tmpl = bit 0
setTmpl:
    mrs r1, control
    ;clear tmpl bit
    mvn r2, #1
    and r1, r2
    ;set tmpl bit according to "on"
    orr r1, r0, lsl #0
    msr control, r1
    bx lr

; void *getSvcParam(void)
; SVC parameter is stored in r0 in stack frame,
; aka the first element in PSP
getSvcParam:
    mrs r1, psp
    ldr r0, [r1]
    bx lr

; void *getSvcParamSize(void)
; SVC parameter length is stored in r1 in stack frame,
; aka the second element in PSP
getSvcParam2:
    mrs r1, psp
    ldr r0, [r1, #4]
    bx lr

; uint8_t getSvcNum(void)
getSvcNum:
    mrs r1, psp
    ldr r2, [r1, #24] ; Move up by 6 words (6*4 = 24 bytes) to get stacked PC addr
    ldrb r0, [r2, #-2]  ; Load bottom byte of PC - 2 bytes (prev instruction) = svc #imm
    bx lr
