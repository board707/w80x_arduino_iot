#include "Arduino.h"

extern "C" u32 tls_mem_get_avail_heapsize(void);
extern "C" char FirmWareVer[4];

String a;

void setup() {
    Serial.begin(115200);
    Serial.println();
    serial.println("Setup...");
    printf(" ---> SDK: %c%x.%02x.%02x\n", FirmWareVer[0], FirmWareVer[1], FirmWareVer[2], FirmWareVer[3]);
    printf(" ---> GetHeap:%d\n",tls_mem_get_avail_heapsize());
}

void loop() {
	int av = Serial.available();
	if (av > 0) {
		Serial.print("Bytes received:");Serial.println(av);
		a= Serial.readString();		// read the incoming data as string
		Serial.print("Symbols received:");Serial.println(a);
	}
}
