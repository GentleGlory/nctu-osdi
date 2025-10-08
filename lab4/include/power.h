#ifndef _POWER_H
#define _POWER_H

#include "core.h"

#define PM_PASSWORD 0x5a000000

#define PM_RSTC_REG 0x3F10001c
#define PM_WDOG_REG 0x3F100024

void power_reset(int tick);
void power_cancel_reset(void);

#endif 