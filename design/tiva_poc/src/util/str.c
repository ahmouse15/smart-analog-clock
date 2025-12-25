// Ahmed Abdulla
// Copyright 2025 Ahmed Abdulla. All Rights Reserved.
// (Excluding work produced by Professor Jason Losh)

#include <stdbool.h>
#include <stdint.h>

#include "util/str.h"

int _ctoi(char num)
{
    switch(num)
    {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}

int _strlen(char *str)
{
    int i;

    for (i = 0; str[i] != '\0'; i++);

    return i;
}

int _strcmp(char *str1, char *str2)
{
    bool end = 0;
    int diff = 0;

    int i;
    for (i = 0; diff == 0 && !end; i++)
    {
        diff = (str1[i] > str2[i]) - (str1[i] < str2[i]);
        end = str1[i] == '\0';
    }

    return diff;
}

//String to integer
int _atoi(char *str)
{
    int i;
    int mul = 1;
    int res = 0;
    int len = _strlen(str);

    for (i = len-1; i >= 0; i--) 
    {
        res += _ctoi(str[i]) * mul;
        mul *= 10;
    }

    return res;
}

//Integer to char, defined only for num between 0 and 9.
char _itoc(uint32_t num)
{
    return '0' + num;
}

//Math power
uint32_t _pow(uint32_t base, uint32_t exp)
{
    uint32_t out = 1;

    int i;
    for (i = 0; i < exp; i++)
    {
        out *= base;
    }

    return out;
}

//Integer to string, decimal
char *_itoa(uint32_t num, char *dest)
{
    const int base = 10; //Base to convert to
    const int digits = 10; //Max number of digits that can appear. Both are related

    bool foundNonZero = false;
    int nonZeroPos = -1;
    uint32_t temp;
    
    int i;
    for (i = digits-1; i >= 0; i--)
    {
        temp = num/_pow(base, i);

        foundNonZero = foundNonZero || temp % base;

        if (foundNonZero || i == 0) {
            if (nonZeroPos == -1) nonZeroPos = i;
            dest[i] = _itoc(temp % base);
        }
        else dest[i] = '\0';
    }

    //Reverse string
    for (i = 0; i < (nonZeroPos+1)/2; i++)
    {
        temp = dest[i];
        dest[i] = dest[nonZeroPos-i];
        dest[nonZeroPos-i] = temp;
    }

    return dest;
}
//Integer to char, hex
char _itoch(uint32_t num)
{
    if (num <= 9) return '0' + num;
    else if (num >= 10) return 'A' + num - 10;
    else return '0';
}

//Integer to string, hex
char *_itoh(uint32_t num, char *dest)
{
    const int base = 16; //Base to convert to
    const int digits = 8; //Max number of digits that can appear. Both are related

    bool foundNonZero = false;
    int nonZeroPos = -1;
    uint32_t temp;
    
    int i;
    for (i = digits-1; i >= 0; i--)
    {
        temp = num/_pow(base, i);

        foundNonZero = foundNonZero || temp % base;

        if (foundNonZero || i == 0) {
            if (nonZeroPos == -1) nonZeroPos = i;
            dest[i] = _itoch(temp % base);
        }
        else dest[i] = '\0';
    }

    //Reverse string
    for (i = 0; i < (nonZeroPos+1)/2; i++)
    {
        temp = dest[i];
        dest[i] = dest[nonZeroPos-i];
        dest[nonZeroPos-i] = temp;
    }

    return dest;
}

uint32_t _strncpy(char *dest, char *src, uint32_t bytes)
{
    uint32_t i;
    for (i = 0; src[i] != '\0' && i < bytes; i++) 
    {
        dest[i] = src[i];
    }

    dest[i] = '\0';
    
    return i;
}
