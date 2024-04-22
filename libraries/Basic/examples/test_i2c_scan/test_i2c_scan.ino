#include "Arduino.h"
#include "Wire.h"

uint8_t dev_i2c[128] = {0};

void setup()
{
  Wire.begin();               // Hardware I2C  (PA4 = SDA, PA1 = SCL by default)
  //Wire.begin(PA12,PA14);    // Software I2C  (SDA, SCL) - any pins
  printf("I2C Scanner \n");
}


void loop()
{
  uint8_t error, address;
  int nDevices;

  printf("Scanning... \n");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    delayMicroseconds(10); //Timeout must be!
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      dev_i2c[nDevices] = address;
      nDevices++;
    }
    else if (error == 4)
    {
      printf("\n Unknown error at address - %d",address);
    }
  }
  if (nDevices == 0) printf("No I2C devices found. \n");
  else {
    printf("Scan complete... \n");
    for (address = 0; address < nDevices; address++) {
      printf("\n Found i2c device at address - %d",dev_i2c[address]);
    }
    printf("\n done... ");
  }
  delay(5000);	// wait 5 seconds for next scan
}