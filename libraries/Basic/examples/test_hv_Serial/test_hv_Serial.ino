#include "Arduino.h"

String a;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup...");
}

void loop() {

 	while(Serial.available()) {
		a= Serial.readString();// read the incoming data as string
		Serial.println(a);
	}


}
