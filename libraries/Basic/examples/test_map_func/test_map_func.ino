#include "Arduino.h"

void setup()
{
	Serial.begin(115200);
  String Test = "Hello Arduino! ";
	Test += String("Test map. Input from 0 to 1024 --> output from -1024 to 1024");
	Serial.println(Test);
}

unsigned int count = 0;

void loop()
{
	unsigned int var = map(count, 0, 1024, -1024, 1024);
	Serial.print(count);Serial.print(" map to ");Serial.println(var);
	if (count <= 1024) count++;
	else count = 0;
}