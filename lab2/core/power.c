#include "power.h"



void reset(int tick)
{
	writel(PM_RSTC_REG, PM_PASSWORD | 0x20); // full reset
	writel(PM_WDOG_REG, PM_PASSWORD | tick); // number of watchdog tick
}

void cancel_reset(void)
{
	writel(PM_RSTC_REG, PM_PASSWORD | 0); // full reset
	writel(PM_WDOG_REG, PM_PASSWORD | 0); // number of watchdog tick
}
