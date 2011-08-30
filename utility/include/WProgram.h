// Copyright (c) 2011 Per Eklund, D.E. Goodman-Wilson, Copyright (c) 2005-2006 David A. Mellis
// Arduino stand-in

#ifndef __arduino_h__
#define __arduino_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//#include "EEPROM.h"

#if defined (__cplusplus)
extern "C" {
#endif
	void RIT_IRQHandler(void);

void init(void);

uint32_t millis(void);

void delay(uint32_t);

// #define HIGH 0x1
// #define LOW  0x0

// #define INPUT 0x0
// #define OUTPUT 0x1

// #define NULL 0x0

// #define true 0x1
// #define false 0x0

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

// #define SERIAL  0x0
// #define DISPLAY 0x1

// #define LSBFIRST 0
// #define MSBFIRST 1

// #define CHANGE 1
// #define FALLING 2
// #define RISING 3

// #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
// #define INTERNAL1V1 2
// #define INTERNAL2V56 3
// #else
// #define INTERNAL 3
// #endif
// #define DEFAULT 1
// #define EXTERNAL 0

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)     ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg) ((deg)*DEG_TO_RAD)
#define degrees(rad) ((rad)*RAD_TO_DEG)
#define sq(x) ((x)*(x))

#define interrupts() __disable_irq()
#define noInterrupts() __enable_irq()

// #define clockCyclesPerMicrosecond() ( F_CPU / 1000000L )
// #define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (F_CPU / 1000L) )
// #define microsecondsToClockCycles(a) ( ((a) * (F_CPU / 1000L)) / 1000L )

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define bit(b) (1UL << (b))

long map(long, long, long, long, long);


#if defined (__cplusplus)
}
#endif

#endif //__wiring_g__
