#include "Arduino.h"

#define COMBINATIONS	2
#define LEDS_COUNT 		7

uint8_t mode = 0;

uint8_t leds[LEDS_COUNT] = {LED_BUILTIN_1,LED_BUILTIN_2,LED_BUILTIN_3,LED_BUILTIN_4,LED_BUILTIN_5,LED_BUILTIN_6,LED_BUILTIN_7};

void isr_user_button()
{
	if (digitalRead(USER_BUTTON) == LOW)
	{
		if (mode < COMBINATIONS) mode++;
		else mode = 0;
	}
}

void setup()
{
	
	pinMode(USER_BUTTON, INPUT);
	attachInterrupt(USER_BUTTON, isr_user_button, FALLING);
	for(u8 i=0; i < LEDS_COUNT; i++) pinMode(leds[i], OUTPUT);
}

void loop()
{
	switch (mode) 
	{
		case 0:
			for(u8 i=0; i < LEDS_COUNT; i++)
			{	
				digitalToggle(leds[i]);
				delay(100);
				digitalToggle(leds[i]);
				delay(250);
			}
			break;
		case 1:
			for(u8 i=LEDS_COUNT; i > 0 ; i--)
			{	
				digitalToggle(leds[i-1]);
				delay(100);
				digitalToggle(leds[i-1]);
				delay(250);
			}
			break;
		case 2:	
			for(u8 i=LEDS_COUNT; i > 0 ; i--)
			{	
				digitalToggle(leds[i-1]);
			}
			delay(100);
			for(u8 i=LEDS_COUNT; i > 0 ; i--)
			{	
				digitalToggle(leds[i-1]);
			}
			delay(250);
			break;
		default:
			break;
	}
}
