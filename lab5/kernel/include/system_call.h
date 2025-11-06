#ifndef _SYSTEM_CALL_H
#define _SYSTEM_CALL_H

#include "core.h"
#include "exception.h"
#include "uapi/system_call.h"

void system_call_exc_handler(struct pt_regs *pt_regs);

#endif