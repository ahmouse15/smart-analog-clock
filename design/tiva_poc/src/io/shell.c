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

#include <stdbool.h>
#include <stdint.h>
#include "tm4c123gh6pm.h"

#include "io/shell.h"

#include "util/interface.h"
#include "io/uart0.h"
#include "util/str.h"

#include "sys/svc.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------


void shell(void)
{
    USER_DATA data = {0};
    
    while (true)
    {
        getsUart0(&data);
        parseFields(&data);
        
        //reboot
        if (isCommand(&data, "reboot", 0)) 
        {
            reboot();
        }
        //ps
        else if (isCommand(&data, "ps", 0))
        {
            ps();
        }
        //ipcs
        else if (isCommand(&data, "ipcs", 0))
        {
            ipcs();
        }
        //kill <pid>
        else if (isCommand(&data, "kill", 1))
        {
            kill( (_fn) getFieldInteger(&data, 1));
        }
        //pkill <proc_name>
        else if (isCommand(&data, "pkill", 1))
        {
            char *str = getFieldString(&data, 1);
            uint32_t size = _strlen(str);
            
            pkill(str, size);
        }
        //pi <on|off>
        else if (isCommand(&data, "pi", 1))
        {
            if (!_strcmp(getFieldString(&data, 1), "on")) pi(true);
            else if (!_strcmp(getFieldString(&data, 1), "off")) pi(false);
        }
        //preempt <on|off>
        else if (isCommand(&data, "preempt", 1))
        {
            if (!_strcmp(getFieldString(&data, 1), "on")) preempt(true);
            else if (!_strcmp(getFieldString(&data, 1), "off")) preempt(false);
        }
        //sched <prio|rr>
        else if (isCommand(&data, "sched", 1))
        {
            if (!_strcmp(getFieldString(&data, 1), "prio")) sched(true);
            else if (!_strcmp(getFieldString(&data, 1), "rr")) sched(false);
        }
        //pidof <proc_name>
        else if (isCommand(&data, "pidof", 1))
        {
            char *str = getFieldString(&data, 1);
            uint32_t size = _strlen(str);

            pidof(str, size);
        }
        else if (isCommand(&data, "run", 1))
        {
            char *str = getFieldString(&data, 1);
            uint32_t size = _strlen(str);

            run(str, size);
        }
        else putsUart0("invalid command\n");
        
    }
}
