/*
 * Fixed.h
 *
 *  Created on: 2021年9月5日
 *      Author: ylk67
 */

#ifndef INC_FIXED_H_
#define INC_FIXED_H_

#include <stdint.h>

typedef int16_t fixed;

#define FIXED_SIZE 2
#define FIXED_UNSIGNED_TYPE uint16_t
#define FIXED_FRACTIONAL_BITS 6
#define FIXED_INTEGER_BITS 9
#define FIXED_INTEGER_MAX ((float)(512) - .015625)
#define FIXED_INTEGER_MIN (float)(-512)

fixed float_to_fixed(float x);
float fixed_to_float(fixed x);
fixed fixed_mux(fixed x, fixed y);
int FloatFixedTest(void);
fixed *AddFixedPtr(fixed *augend, unsigned int addend);

#endif /* INC_FIXED_H_ */
