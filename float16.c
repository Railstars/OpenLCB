#include "float16.h"
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


_float16_shape_type float32_to_float16(float f32_val)
{
	_float16_shape_type f16_val;
	union { float f; uint32_t i; } v;
	v.f = f32_val;
	//printf("32to16. Input: %20.64f\n\t\t0x%x\n", v.f, v.i );

    
	uint16_t sign = (v.i >> 16) & 0x00008000;
    int16_t exponent = ((v.i >> 23) & 0x000000ff) - (127 - 15);
    uint32_t mantissa = v.i & 0x007fffff;

	if (exponent <= 0)
	{
		//printf("exponent = %d\n", exponent);
		//printf("exponent <= 0\n");
		if (exponent < -10) //too small!?
		{
			//printf("too small, exponent < -10\n");
			f16_val.bits = sign;
			//printf("returning 0x%x\n", f16_val.bits);
			return f16_val;
		}
		mantissa = (mantissa | 0x00800000) >> (1 - exponent);
		f16_val.bits = (sign | (mantissa >> 13));
		//printf("returning 0x%x\n", f16_val.bits);
		return f16_val;
	}
	else if (exponent == 0xff - (127 - 15)) //error val
	{
		f16_val.number.exponent = 0x1F; //flags an error condition
		if (mantissa == 0) // Inf
		{
			//printf("infinite result\n");
			f16_val.bits = sign | 0x7c00;
			//printf("returning 0x%x\n", f16_val.bits);
			return f16_val;
		} 
		else    // NAN
		{
			//printf("NaN\n");
			mantissa >>= 13;
			f16_val.bits = (sign | 0x7c00 | mantissa | (mantissa == 0));
			//printf("returning 0x%x\n", f16_val.bits);
			return f16_val;
		}
	}
	else
	{
		//printf("exponent > 0\n");
		if (exponent > 30) // Overflow
		{
			//printf("Overflow!\n");
			f16_val.bits = (sign | 0x7c00);
			//printf("returning 0x%x\n", f16_val.bits);
			return f16_val;
		}

		f16_val.bits = (sign | (exponent << 10) | (mantissa >> 13));
		//printf("float16:\n  f16_val.number.sign: %d\n  f16_val.number.exponent %d\n  f16_val.number.mantissa %d\n", sign, exponent, mantissa);
		//printf("returning 0x%x\n", f16_val.bits);
		return f16_val;
	}
	//should never get here!!!
	//printf("ERROR!\n");
	f16_val.bits = 0;
	return f16_val;
}

float float16_to_float32(_float16_shape_type f16_val)
{
	//printf("32to16. Input: 0x%x\n", f16_val.bits);

	int32_t sign = (f16_val.bits >> 15) & 0x00000001;
	uint32_t exponent = (f16_val.bits >> 10) & 0x0000001f;
    uint32_t mantissa =  f16_val.bits & 0x000003ff;

	//printf("float16:\n  f16_val.number.sign: %d\n  f16_val.number.exponent %d\n  f16_val.number.mantissa %d\n", sign, exponent, mantissa);

	union { float f; uint32_t i; } v;

	if(exponent == 0)
	{
		if(mantissa == 0) //zero!
		{
			//printf("zero value!\n");
			v.i = sign << 31;
			return v.f;
		}
		else	//sub-normal number, renormalize it (!?)
		{
			//printf("subnormal value!\n");
			//printf("   0x%x, 0x%x\n", mantissa, exponent);
			while (!(mantissa & 0x00000400))
			{
				mantissa <<= 1;
				exponent -=  1;
				//printf("   0x%x, 0x%x\n", mantissa, exponent);
			}
			exponent += 1;
			mantissa &= ~0x00000400;
			//printf("   0x%x, 0x%x\n", mantissa, exponent);

		}
	}

	else if(exponent == 0x1F) //NaN! treat as zero for our purposes
	{
		if (mantissa == 0) // Inf
		{
			//printf("Infinite\n");
			v.i = (sign << 31) | 0x7f800000;
			return v.f;
		}
		else // NaN
		{
			//printf("NaN\n");
			v.i = (sign << 31) | 0x7f800000 | (mantissa << 13);
			return v.f;
		}
	}

	//printf("Normal number\n");
	//else a normal number; fall through from sub-normal case.
	exponent = exponent + (127 - 15);
    mantissa = mantissa << 13;
	//printf("   0x%x, 0x%x, 0x%x\n", sign, exponent, mantissa);
    
    v.i = (sign << 31) | (exponent << 23) | mantissa;
    //printf("  0x%x\n", v.i);
    return v.f;
}

#ifdef DO_FIX16
fix16_t float16_to_fix16(_float16_shape_type f16_val)
{
// the following works, but relies on floating point multiplication == bad!
//  return fix16_from_float(float16_to_float32(f16_val));

  //first, extract the components from the float16:
  uint16_t sign = (f16_val.bits >> 15) & 0x00000001;
  int16_t exponent = (f16_val.bits >> 10) & 0x0000001f;
  uint32_t mantissa =  f16_val.bits & 0x000003ff;
  exponent -= 9;
  printf("%d, 0x%x\n", exponent, mantissa);
  if(exponent < 0)
      if(!sign)
          return (mantissa | 0x400) >> (-1*exponent);
      else
          return -1 * ( (mantissa | 0x400) >> (-1*exponent) );
  else //if(exponent > 0)
      if(!sign)
          return (mantissa | 0x400) << exponent;
      else
          return -1 * ( (mantissa | 0x400) << exponent );
}
#endif
