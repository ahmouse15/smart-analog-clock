// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#include "io/uart0.h"
#include "util/str.h"
#include "util/interface.h"

void parseFields(USER_DATA *data)
{
    data->fieldCount = 0;
    
    int i;

    char currField = 'd'; // d = delimiter, a = alphabetic, n = numeric
    for(i = 0; data->buffer[i] != '\0'; i++)
    {
        if ((data->buffer[i] >= 0x41 && data->buffer[i] <= 0x5A)
            || (data->buffer[i] >= 0x61 && data->buffer[i] <= 0x7A))
        {
            if (currField != 'a')
            {
                currField = 'a';
                data->fieldType[data->fieldCount] = 'a';

                data->fieldPosition[data->fieldCount] = i;

                data->fieldCount++;
            }
        }
        else if (data->buffer[i] >= 0x30 && data->buffer[i] <= 0x39)
        { //numeric
            if (currField != 'n')
            {
                currField = 'n';
                data->fieldType[data->fieldCount] = 'n';

                data->fieldPosition[data->fieldCount] = i;

                data->fieldCount++;
            }
        }
        else if (currField != 'd')
        {
            currField = 'd';
            data->buffer[i] = '\0';
        }

    }
}

char* getFieldString(USER_DATA *data, uint8_t fieldNumber)
{
    if (fieldNumber < data->fieldCount && data->fieldType[fieldNumber] == 'a')
    {
        return data->buffer + data->fieldPosition[fieldNumber];
    }
    else
    {
        return NULL;
    }
}

int32_t getFieldInteger(USER_DATA *data, uint8_t fieldNumber)
{
    char *str = data->buffer + data->fieldPosition[fieldNumber];

    if (fieldNumber < data->fieldCount && data->fieldType[fieldNumber] == 'n') 
    {
        return _atoi(str);
    }
    else
    {
        return 0;
    }
}

bool isCommand(USER_DATA *data, const char strCommand[], uint8_t minArgument)
{
    if (_strcmp(data->buffer + data->fieldPosition[0], (char*)strCommand) == 0 && data->fieldCount-1 >= minArgument)
    {
        return true;
    }

    return false;
}
