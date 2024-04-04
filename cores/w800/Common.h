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

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

// Прототипы для функций задержек и UPTIME
/*
void delayMicroseconds(int us);
void delay(uint32_t ms);

u32 micros();
u32 millis();
*/
#endif