#include "Arduino.h"
#include "hw_timers.h"

// Использование аппаратных таймеров

void lamp1() {digitalToggle(LED_BUILTIN_1); hw_timer0::start();}
void lamp2() {digitalToggle(LED_BUILTIN_2); hw_timer1::start();}
void lamp3() {digitalToggle(LED_BUILTIN_3); hw_timer2::start();}
void lamp4() {digitalToggle(LED_BUILTIN_4); hw_timer3::start();}
void lamp5() {digitalToggle(LED_BUILTIN_5); hw_timer4::start();}
void lamp6() {digitalToggle(LED_BUILTIN_6); hw_timer5::start();}

void setup() 
{
	pinMode(LED_BUILTIN_1, OUTPUT);
	pinMode(LED_BUILTIN_2, OUTPUT);
	pinMode(LED_BUILTIN_3, OUTPUT);
	pinMode(LED_BUILTIN_4, OUTPUT);
	pinMode(LED_BUILTIN_5, OUTPUT);
	pinMode(LED_BUILTIN_6, OUTPUT);

	hw_timer0::set(ms, 100, lamp1); hw_timer0::start();
	hw_timer1::set(ms, 200, lamp2); hw_timer1::start();
	hw_timer2::set(ms, 400, lamp3); hw_timer2::start();
	hw_timer3::set(ms, 800, lamp4); hw_timer3::start();
	hw_timer4::set(ms, 1600, lamp5); hw_timer4::start();
	hw_timer5::set(ms, 3200, lamp6); hw_timer5::start();
}

void loop()
{
	
}