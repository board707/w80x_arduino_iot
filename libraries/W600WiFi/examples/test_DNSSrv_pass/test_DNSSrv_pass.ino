#include <Arduino.h>
#include <W600WiFi.h>
#include <DNSServer.h>

DNSServer dnsServer;

int start_wifi_as_ap()
{
    Serial.println("start_wifi_as_ap");
    WiFi.softAP("W801", "1234567890");
}

void w600_arduino_setup()
{
    printf("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    Serial.println("xxxxxxxxxxxx");
    start_wifi_as_ap();
    delay(1000);
    dnsServer.start("w801_dev_01");
}

int sleep_cnt(int cnt)
{
    int sleep_cnt = 60;
    int i = 0;
    for (i = 1; i <= sleep_cnt ; i++)
    {
        if ((i - 1) % 20 == 0)
        {
            Serial.println();
        }
        else if ((i-1) % 10 == 0)
        {
            Serial.print("   ");
        }
        else if ((i-1) % 5 == 0)
        {
            Serial.print(' ');
        }
        Serial.print('.');
        delay(1000);
    }
    Serial.println();
}
void w600_arduino_loop()
{
    printf("loop()\n");
    Serial.println("Hello From HLK-W801");
    sleep_cnt(60);
}

void setup() {
	Serial.begin(115200);
    printf("setup()\n");
    // put your setup code here, to run once:
    w600_arduino_setup();
}

void loop() {
    printf("[%s %s %d]\n", __FILE__, __func__, __LINE__);
    w600_arduino_loop();
}
