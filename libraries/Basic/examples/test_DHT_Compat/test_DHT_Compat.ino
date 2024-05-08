#include "Arduino.h"

#define TYPE_DHT11    
//#define TYPE_DHT22

#define sensorPin PB2

//Varyables
typedef struct {float Temperature; float Humidity;} DHT_DataTypedef;
DHT_DataTypedef DHT_Data;
float Temperature, Humidity;
uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
uint16_t SUM; uint8_t Presence = 0;

//Function prototypes
void DHT_GetData (DHT_DataTypedef *DHT_Data);
void DHT_Start (void);
uint8_t DHT_Check_Response (void);
uint8_t DHT_Read (void);

void setup() {
  Serial.begin(115200);
  Serial.println("DHT11/DHT22 Demo");
  pinMode(sensorPin, OUTPUT);
  digitalWrite(sensorPin, HIGH);
}

void loop() {
  DHT_GetData(&DHT_Data);
  Temperature = DHT_Data.Temperature;
  Humidity = DHT_Data.Humidity;
  Serial.print("Temperature:\t");Serial.print(Temperature);Serial.print("\t");Serial.print("Humidity:\t");Serial.println(Humidity);
  delay(2000);
}

void DHT_Start (void)
{
  pinMode(sensorPin, OUTPUT);  // set the pin as output
  digitalWrite(sensorPin, LOW);   // pull the pin low
  delay(18);   // wait for 18ms
  digitalWrite(sensorPin, HIGH);   // pull the pin high
  delayMicroseconds(30);   // wait for 30us
  pinMode(sensorPin, INPUT);    // set as input
}

uint8_t DHT_Check_Response (void)
{
  uint8_t Response = 0;
  delayMicroseconds(40);
  if (!(digitalRead(sensorPin)))
  {
    delayMicroseconds(80);
    if (digitalRead(sensorPin)) Response = 1;
    else Response = -1;
  }
  while (digitalRead(sensorPin));   // wait for the pin to go low

  return Response;
}

uint8_t DHT_Read (void)
{
  uint8_t i=0; uint8_t j=0;
  for (j=0;j<8;j++)
  {
    while (!(digitalRead(sensorPin)));   // wait for the pin to go high
    delayMicroseconds(40);   // wait for 40 us
    if (!(digitalRead(sensorPin)))   // if the pin is low
    {
      i&= ~(1<<(7-j));   // write 0
    }
    else i|= (1<<(7-j));  // if the pin is high, write 1
    while (digitalRead(sensorPin));  // wait for the pin to go low
  }
  return i;
}

void DHT_GetData (DHT_DataTypedef *DHT_Data)
{
  DHT_Start();
  Presence = DHT_Check_Response();
  if (Presence) {
  Rh_byte1 = DHT_Read ();
  Rh_byte2 = DHT_Read ();
  Temp_byte1 = DHT_Read ();
  Temp_byte2 = DHT_Read ();
  SUM = DHT_Read();

  if (SUM == (Rh_byte1+Rh_byte2+Temp_byte1+Temp_byte2))
  {
    #if defined(TYPE_DHT11)
      DHT_Data->Temperature = Temp_byte1;
      DHT_Data->Humidity = Rh_byte1;
    #endif

    #if defined(TYPE_DHT22)
      DHT_Data->Temperature = ((Temp_byte1<<8)|Temp_byte2);
      DHT_Data->Humidity = ((Rh_byte1<<8)|Rh_byte2);
    #endif
  }
} else { Serial.println("No response...");}
}
