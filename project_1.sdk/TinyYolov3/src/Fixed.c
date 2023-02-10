/*
 * Fixed.c
 *
 *  Created on: 2021年9月5日
 *      Author: ylk67
 */

#include "Fixed.h"
#include "stdio.h"
#include "math.h"

fixed float_to_fixed(float x)
{
	if (x > FIXED_INTEGER_MAX)
	{
		printf("Error!! float %f is bigger than %f\r\n", x, FIXED_INTEGER_MAX);
		x = (float)FIXED_INTEGER_MAX;
	}
	else if (x < FIXED_INTEGER_MIN)
	{
		printf("Error!! float %f is smaller than %f\r\n", x, FIXED_INTEGER_MIN);
		x = (float)FIXED_INTEGER_MIN;
	}
    fixed tmp;
    tmp = (fixed)(x * (1 << FIXED_FRACTIONAL_BITS));

    if (x > 0)
    {
        if (fmod(x, 0.015625) >= 0.0078125)
        {
            tmp += 1;
        }
    }
    else
    {
        if (fmod(x, 0.015625) < -0.0078125)
        {
            tmp -= 1;
        }
    }
    return tmp;
}

float fixed_to_float(fixed x)
{
	return ((float)x / (float)(1 << FIXED_FRACTIONAL_BITS));
}

fixed fixed_mux(fixed x, fixed y)
{
	unsigned int tmp =  ((unsigned int)x * (unsigned int)y);
    return (tmp >> FIXED_FRACTIONAL_BITS) + (((tmp >> (FIXED_FRACTIONAL_BITS>>1)) & ((1<< ((FIXED_FRACTIONAL_BITS>>1)-1)))) ? 1:0);
}

int FloatFixedTest(void)
{
	float a;
	fixed b = 0x10;

	a = 512;
	b = float_to_fixed(a);
	a = fixed_to_float(b);
	printf("o=%f\r\n", fixed_to_float(b));

	return 0;
}

fixed *AddFixedPtr(fixed *augend, unsigned int addend)
{
	return (fixed *)((unsigned int)augend + addend);
}

