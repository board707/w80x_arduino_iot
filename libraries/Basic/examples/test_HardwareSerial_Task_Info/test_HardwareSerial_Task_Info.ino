#include "Arduino.h"

#ifdef __cplusplus
#include <wm_mem.h>
extern "C" void vTaskList(char *);
#endif

void setup(){

	Serial.begin(9600);	
}

void loop()
{
	char *buf = NULL;

	buf = (char*)tls_mem_alloc(1024);
	if(NULL == buf) return;
	vTaskList((char *)buf);
	Serial.println("===============================================");
	Serial.println(buf);
	Serial.println("===============================================");
	tls_mem_free(buf);
	buf = NULL;
	delay(2000);
}