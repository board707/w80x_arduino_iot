#include "Arduino.h"

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  printf("======================================= \n");
  tls_os_disp_task_stat_info();
  printf("======================================= \n");
  delay(1000);
}
