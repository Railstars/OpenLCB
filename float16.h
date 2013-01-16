#ifndef __FLOAT_16__
#define __FLOAT_16__

#include <stdint.h>

#ifdef DO_FIX16
#include "fixmath.h"
#endif

/* data type for storing float16s */

typedef union
{
	struct
	{
		uint16_t sign : 1;
		uint16_t exponent : 5;
		uint16_t mantissa: 10;
	} number;

	struct
	{
		uint8_t msw;
		uint8_t lsw;
	} words;

	uint16_t bits;
} _float16_shape_type;

#ifdef __cplusplus
extern "C" {
#endif

// the following two methods are adapted from
// http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html

_float16_shape_type float32_to_float16(float f32_val);

float float16_to_float32(_float16_shape_type f16_val);

#ifdef DO_FIX16
// But I wrote this one.
fix16_t float16_to_fix16(_float16_shape_type f16_val);
#endif

#ifdef __cplusplus
}
#endif

#endif
