#ifndef __COMMON_H__
#define __COMMON_H__
#include "pins_arduino.h"
// Определения для DIO
#ifndef HIGH
#define HIGH 			0x1
#endif
#ifndef LOW
#define LOW  			0x0
#endif

// Режимы ввода/вывода GPIO
#ifndef INPUT
#define INPUT 			1
#endif
#ifndef OUTPUT
#define OUTPUT 			2
#endif
#ifndef INPUT_PULLUP
#define INPUT_PULLUP 	3
#endif
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN 	4
#endif
#ifndef PWM_OUT
#define PWM_OUT 		6
#endif
#ifndef ANALOG_INPUT
#define ANALOG_INPUT 	5
#endif

typedef bool boolean;
typedef enum {
  LSBFIRST = 0,
  MSBFIRST = 1,
} BitOrder;

typedef uint8_t byte;
typedef bool boolean;

// Переопределение abs()
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

//Битовые операции
#define byte(w) ((uint8_t)(w))
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define maskSet(value, mask) ((value) |= (mask))
#define maskClear(value, mask) ((value) &= ~(mask))

#endif
