#include "Arduino.h"

void setup()
{
	Serial.begin(115200);
 	randomSeed(analogRead(A2));
}

void loop()
{
	Serial.print("std rnd is: ");Serial.println(random(-1024,1024));
}
