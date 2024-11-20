// Tested on HLK-W801-Kit

#include <Arduino.h>
#include <W600WiFi.h>


bool flag = false;
bool connect_AP = false;


  
void get_AP_info()
{
    Serial.println("-----------------------------------------");
    Serial.println(String("STANUM: ") + String(WiFi.softAPgetStationNum()));
    Serial.println(String("AP IP: ") + String(WiFi.softAPIP()));
    Serial.println(String("AP MAC: ") + String(WiFi.softAPmacAddress()));
    Serial.println(String("AP SSID: ") + String(WiFi.softAPSSID()));
    Serial.println(String("AP PSK: ") + String(WiFi.softAPPSK()));
}
  

void isr_user_button()
{
	if (digitalRead(USER_BUTTON) == LOW)
	{
		if (!flag) flag = true;
		connect_AP = !connect_AP;
	}
}

void setup() {

	Serial.begin(115200);
	
    pinMode(USER_BUTTON, INPUT);
	attachInterrupt(USER_BUTTON, isr_user_button, FALLING);
    
	Serial.println("Hello From HLK-W801 Board ");
}

void loop() {
	if(flag)
	{
		if (connect_AP) 
		{
			Serial.println("AP mode ON");
			WiFi.softAP("W801", "1234567890");
			flag = false;
			delay(5000);
			get_AP_info();
		}
		else 
		{
            Serial.println("AP mode OFF");
            WiFi.softAPdestroy();
			flag = false;
		}
		
	}
}
