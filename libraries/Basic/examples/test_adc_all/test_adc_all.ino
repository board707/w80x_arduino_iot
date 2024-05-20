#include "Arduino.h"

void setup()
{
	pinMode(PA1,ANALOG_INPUT);
	pinMode(PA2,ANALOG_INPUT);
	pinMode(PA3,ANALOG_INPUT);
	pinMode(PA4,ANALOG_INPUT);
  Serial.begin(115200);
}

void loop()
{
int ch1,ch2,ch3,ch4;
ch1 = analogRead(PA1);
ch2 = analogRead(PA2);
ch3 = analogRead(PA3);
ch4 = analogRead(PA4);
Serial.println("=================================");
String output_str = "";
output_str += String("Chan:")+String(1)+String("  ")+String(ch1)+String(" (mV) or ")+String((double)ch1/1000)+String(" (V)");
Serial.println(output_str);
output_str = "";
output_str += String("Chan:")+String(2)+String("  ")+String(ch2)+String(" (mV) or ")+String((double)ch2/1000)+String(" (V)");
Serial.println(output_str);
output_str = "";
output_str += String("Chan:")+String(3)+String("  ")+String(ch3)+String(" (mV) or ")+String((double)ch3/1000)+String(" (V)");
Serial.println(output_str);
output_str = "";
output_str += String("Chan:")+String(4)+String("  ")+String(ch4)+String(" (mV) or ")+String((double)ch4/1000)+String(" (V)");
Serial.println(output_str);
delay(1000);
}