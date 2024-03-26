#include "Arduino.h"

void setup()
{
	pinMode(PA1,ANALOG_INPUT);
	pinMode(PA2,ANALOG_INPUT);
	pinMode(PA3,ANALOG_INPUT);
	pinMode(PA4,ANALOG_INPUT);
}

void loop()
{
int ch1,ch2,ch3,ch4;
ch1 = analogRead(PA1);
ch2 = analogRead(PA2);
ch3 = analogRead(PA3);
ch4 = analogRead(PA4);

printf("chan:%d  %d(mV) or %f(V)\r\n", 1, ch1, (double)ch1/1000);
printf("chan:%d  %d(mV) or %f(V)\r\n", 2, ch2, (double)ch2/1000);
printf("chan:%d  %d(mV) or %f(V)\r\n", 3, ch3, (double)ch3/1000);
printf("chan:%d  %d(mV) or %f(V)\r\n", 4, ch4, (double)ch4/1000);

delay(1000);

}