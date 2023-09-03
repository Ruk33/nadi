#include <stdarg.h>
#include "nadi.h"

void log_info(char *message, ...)
{
    make(struct log, new_log);
    text(new_log->severity, "%s", "info");
    
    va_list va;
    va_start(va, message);
    vstrf(new_log->message, sizeof(new_log->message), message, va);
    va_end(va);
    
    create(log, new_log);
}
