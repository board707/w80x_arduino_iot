#ifndef __ARDUINO_H__
#define __ARDUINO_H__
#include "pins_arduino.h"

#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_include.h"

#ifdef __cplusplus
}
#endif

#define UNUSED(X) (void)X 

#ifdef __cplusplus
#include "Stream.h"
//#include "HardwareSerial.h"
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
// Определения для DIO
#ifndef HIGH
#define HIGH 			0x1
#endif
#ifndef LOW
#define LOW  			0x0
#endif


#ifndef RISING
#define RISING      	WM_GPIO_IRQ_TRIG_RISING_EDGE  
#endif 
#ifndef FALLING         
#define FALLING     	WM_GPIO_IRQ_TRIG_FALLING_EDGE           
#endif
#ifndef CHANGE
#define CHANGE      	WM_GPIO_IRQ_TRIG_DOUBLE_EDGE     
#endif
#ifndef HIGH_LEVEL
#define HIGH_LEVEL  	WM_GPIO_IRQ_TRIG_HIGH_LEVEL         
#endif
#ifndef LOW_LEVEL
#define LOW_LEVEL   	WM_GPIO_IRQ_TRIG_LOW_LEVEL 
#endif

// Прототипы для DIO
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
void digitalToggle(uint8_t pin);
uint8_t digitalRead(uint8_t pin);

// Прототипы аналоговых функций

void analogWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);

// Прерывания GPIO

extern "C" void attachInterrupt(uint8_t pin, void(*)(), uint8_t mode);
extern "C" void detachInterrupt(uint8_t pin);
extern "C" void arduino_gpio_isr( void *context);

// Задержки

extern "C" void delayMicroseconds(uint32_t us);
extern "C" void delay(uint32_t ms);

// UpTime

extern "C" uint32_t micros();
extern "C" uint32_t millis();

// Добавлено для совместимости shiftIn/shiftOut

uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);


#endif
