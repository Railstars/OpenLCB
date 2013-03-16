#ifndef __FLOAT_16__
#define __FLOAT_16__

#include <stdint.h>


// the following code has been adapted from
// http://www.ogre3d.org/docs/api/html/OgreBitwise_8h_source.html
// Original copyright notice follows
/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2012 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/


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
