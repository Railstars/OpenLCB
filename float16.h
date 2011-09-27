#ifndef __FLOAT_16__
#define __FLOAT_16__

#include <stdint.h>
#include "fixmath.h"

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

// adapted from
// http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html


#ifdef __cplusplus
extern "C" {
#endif

_float16_shape_type float32_to_float16(float f32_val);

float float16_to_float32(_float16_shape_type f16_val);

fix16_t float16_to_fix16(_float16_shape_type f16_val);

#ifdef __cplusplus
}
#endif

#endif
