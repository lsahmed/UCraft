#include "log.h"
#include <stdarg.h>
#include <stdio.h>

void printl(char *type, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf("[%s]: ", type);
    vprintf(fmt, args);
    va_end(args);
}