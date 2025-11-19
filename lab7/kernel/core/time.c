#include "core.h"
#include "time.h"
#include "string.h"


struct rational time_get_time_tick()
{
	struct rational ret = {0,0};
	uint64_t freq = read_sys_reg(CNTFRQ_EL0);
	uint64_t tick = read_sys_reg(CNTPCT_EL0);

	ret = math_get_rational(tick, freq);
	return ret;
}

