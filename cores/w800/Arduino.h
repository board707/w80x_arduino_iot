#pragma once
#include "pins_arduino.h"
#include "Common.h"
#include "Interrupts.h"
#include "pgmspace.h"

#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_include.h"
#include "wiring_time.h"

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "HardwareSerial.h"
#endif

// Прототипы для DIO
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
void digitalToggle(uint8_t pin);
uint8_t digitalRead(uint8_t pin);

// Прототипы аналоговых функций

void analogWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);

// Прототипы для скетча Ардуино

void setup(void);
void loop(void);



