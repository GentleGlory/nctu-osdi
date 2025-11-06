#ifndef _MATH_H
#define _MATH_H

#include "core.h"

struct rational {
	int64_t		num;
	int64_t		den;
};

struct rational math_get_rational(int64_t n, int64_t d);

#endif