#include "math.h"
#include "core.h"


struct rational math_get_rational(int64_t n, int64_t d) {
	struct rational ret = {0, 0};
	if (d == 0) return ret;  // invalid

	if (d < 0) { n = -n; d = -d; }  // keep denominator positive

	ret.num = n / d;  // 整數部分
	ret.den = ((n % d) * 1000000) / d;  // 小數部分，精確到6位小數

	return ret;
}
