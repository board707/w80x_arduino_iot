#include "Arduino.h"
#include "GyverOLED.h"

GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;

void setup() {
    pinMode(PA2,ANALOG_INPUT);
    oled.init(); 
    Wire.setClock(800000);    
    oled.setScale(1);
}

void loop() {
uint32_t volt=0;
volt = analogRead(PA2);
oled.clear();
oled.setCursor(0,0);
oled.print("ADC: ");oled.print(volt);oled.print(" mV");
oled.update();
delay(1000);
}
