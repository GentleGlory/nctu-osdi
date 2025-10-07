#ifndef _POWER_H
#define _POWER_H

#include "core.h"

#define PM_PASSWORD 0x5a000000

#define PM_RSTC_REG 0x3F10001c
#define PM_WDOG_REG 0x3F100024

void reset(int tick);
void cancel_reset(void);

#endif 