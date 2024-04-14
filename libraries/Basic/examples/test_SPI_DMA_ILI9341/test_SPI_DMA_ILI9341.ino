
/*
  Connect your TFT display like this:
  TFT_VCC   to power
  TFT_GND   to ground
  TFT_CS    PB7
  TFT_LED   to power 
  TFT_SCL   PB15   
  TFT_MOSI  PB17   
  TFT_DC    PB8
  TFT_RST   PB9
*/


#include "ILI9341.h"
#include "Arduino.h"

static const uint16_t screenWidth  = LCD_WIDTH;
static const uint16_t screenHeight = LCD_HEIGHT;

#define TFT_DC  PB8
#define TFT_RST PB9
#define TFT_CS	PB7


void setup()
{
    
    Lcd_Init(TFT_DC, TFT_RST,TFT_CS);          
    Lcd_Orientation(0); 

}

uint8_t index = 0;
uint16_t buff[12] = { BLACK, RED, GREEN, BLUE, YELLOW, PURPLE, CYAN, MAGENTA, WHITE, GREENYELLOW, VIOLET, AQUA };

void loop()
{
	for (index = 0; index < 12; index++) 
    Lcd_Clear(buff[index]);

}
