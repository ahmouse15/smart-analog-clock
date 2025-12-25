// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)

#ifndef SYS_ASM_H_
#define SYS_ASM_H_

#include <stdbool.h>

extern void startRtosHelper(void *fn);
extern void setPsp(void *ptr);
extern void setAsp(bool on);
extern void setTmpl(bool on);

extern uint8_t getSvcNum(void);
extern union svc_param getSvcParam(void);
extern union svc_param2 getSvcParam2(void);

extern uint32_t *getPsp(void);
extern uint32_t *getSp(void);
extern uint32_t *getMsp(void);

extern void initUnrunTask(void *fn);
extern void saveRegPendsv(void);
extern void restoreRegPendsv(void);

#endif
