#pragma once
#include "Arduino.h"
#include "driver.h"

#define BLACK                      0x0000
#define BLUE                       0x001F
#define RED                        0xF800
#define GREEN                      0x07E0
#define CYAN                       0x07FF
#define MAGENTA                    0xF81F
#define YELLOW                     0xFFE0
#define WHITE                      0xFFFF
#define VIOLET                     0xEC1D
#define AQUA                       0x07FF
#define GREENYELLOW                0xAFE5
#define PURPLE                     0x780F
#define ILI9341_MADCTL     0x36
#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04
#define ILI9341_CASET      0x2A
#define ILI9341_PASET      0x2B
#define ILI9341_RAMWR      0x2C

#define LCD_WIDTH        320
#define LCD_HEIGHT       480


uint16_t _width, _height;  //Ширина,высота после изменения ориентации
uint8_t dc_pin,rst_pin,cs_pin;     

void Lcd_Hard_Reset(void) // Аппаратный сброс
{
  digitalWrite(rst_pin, HIGH);
  delay(5);
  digitalWrite(rst_pin, LOW);
  delay(15);
  digitalWrite(rst_pin, HIGH);
  delay(15);
}

void Lcd_Set_DC_CS(bool dc, bool cs) {
  if (dc == true)  digitalWrite(dc_pin, HIGH);
  else  digitalWrite(dc_pin, LOW);
  if (cs == true)  digitalWrite(cs_pin, HIGH);
  else  digitalWrite(cs_pin, LOW);
}

void Lcd_Write_Data(uint8_t data) {
  Lcd_Set_DC_CS(true, false);
  sdio_transfer(data);
  Lcd_Set_DC_CS(true, true);
}

void Lcd_Write_Com(uint8_t cmd) {
  Lcd_Set_DC_CS(false, false);
  sdio_transfer(cmd);
  Lcd_Set_DC_CS(true, true);
}

void displayInit() 
{
  Lcd_Hard_Reset();
  delay(10);
  Lcd_Write_Com(0x01);
  delay(10);
   Lcd_Write_Com(0x11);
   delay(100);
		
   Lcd_Write_Com(0xF0);
   Lcd_Write_Data(0xC3);
   
   Lcd_Write_Com(0xF0);
   Lcd_Write_Data(0x96);
   
   Lcd_Write_Com(0x36);
   Lcd_Write_Data(0x48);
   
   Lcd_Write_Com(0x3A);
   Lcd_Write_Data(0x55);
   
   Lcd_Write_Com(0xB4);
   Lcd_Write_Data(0x01);
   
   Lcd_Write_Com(0xB6);
   Lcd_Write_Data(0x80);
   Lcd_Write_Data(0x02);   
   Lcd_Write_Data(0x3B);   

   Lcd_Write_Com(0xE8);
   Lcd_Write_Data(0x40);
   Lcd_Write_Data(0x8A);
   Lcd_Write_Data(0x00);
   Lcd_Write_Data(0x00);
   Lcd_Write_Data(0x29);
   Lcd_Write_Data(0x19);
   Lcd_Write_Data(0xA5);
   Lcd_Write_Data(0x33);

   Lcd_Write_Com(0xC1);
   Lcd_Write_Data(0x06);
   
   Lcd_Write_Com(0xC2);
   Lcd_Write_Data(0xA7);
   
   Lcd_Write_Com(0xC5);
   Lcd_Write_Data(0x18);

   Lcd_Write_Data(100);
   
   Lcd_Write_Com(0xE0);
   Lcd_Write_Data(0xF0);
   Lcd_Write_Data(0x09);
   Lcd_Write_Data(0x0B);
   Lcd_Write_Data(0x06);
   Lcd_Write_Data(0x04);
   Lcd_Write_Data(0x15);
   Lcd_Write_Data(0x2F);
   Lcd_Write_Data(0x54);
   Lcd_Write_Data(0x42);
   Lcd_Write_Data(0x3C);
   Lcd_Write_Data(0x17);
   Lcd_Write_Data(0x14);
   Lcd_Write_Data(0x18);
   Lcd_Write_Data(0x1B);

   Lcd_Write_Com(0XE1);
   Lcd_Write_Data(0xE0);
   Lcd_Write_Data(0x09);
   Lcd_Write_Data(0x0B);
   Lcd_Write_Data(0x06);
   Lcd_Write_Data(0x04);
   Lcd_Write_Data(0x03);
   Lcd_Write_Data(0x2B);
   Lcd_Write_Data(0x43);
   Lcd_Write_Data(0x42);
   Lcd_Write_Data(0x3B);
   Lcd_Write_Data(0x16);
   Lcd_Write_Data(0x14);
   Lcd_Write_Data(0x17);
   Lcd_Write_Data(0x1B);

   delay(120);
   
   Lcd_Write_Com(0xF0);
   Lcd_Write_Data(0xC3);
   
   Lcd_Write_Com(0xF0);
   Lcd_Write_Data(0x69);
   
   delay(120);
   
   Lcd_Write_Com(0X29); // Вкл дисплея

}

void Lcd_Init(uint8_t dc, uint8_t rst,uint8_t cs) {
  pinMode(dc, OUTPUT); dc_pin = dc;
  pinMode(rst, OUTPUT); rst_pin = rst;
  pinMode(cs, OUTPUT); cs_pin = cs;
  sdio_init();
  displayInit();
}
void Lcd_Orientation(uint8_t m) 
{
  Lcd_Write_Com(ILI9341_MADCTL);
  uint8_t rotation = m & 3;
  switch (rotation) {
   case 0:
     Lcd_Write_Data((1<<3)|(1<<6));
     _width  =  LCD_WIDTH;
     _height =LCD_HEIGHT;
     break;
   case 1:
     Lcd_Write_Data((1<<3)|(1<<5));
     _width  = LCD_HEIGHT;
     _height = LCD_WIDTH;
     break;
  case 2:
    Lcd_Write_Data((1<<3)|(1<<7));
     _width  = LCD_WIDTH;
     _height = LCD_HEIGHT;
    break;
   case 3:
     Lcd_Write_Data((1<<3)|(1<<7)|(1<<6)|(1<<5));
     _width  = LCD_HEIGHT;
     _height = LCD_WIDTH;
     break;
  }
}

void Lcd_Set_Window(uint16_t XS, uint16_t YS, uint16_t XE, uint16_t YE) {
  Lcd_Write_Com(ILI9341_CASET);Lcd_Write_Data(XS >> 8);Lcd_Write_Data(XS);Lcd_Write_Data(XE >> 8);Lcd_Write_Data(XE);
  Lcd_Write_Com(ILI9341_PASET);Lcd_Write_Data(YS >> 8);Lcd_Write_Data(YS);Lcd_Write_Data(YE >> 8);Lcd_Write_Data(YE);
  Lcd_Write_Com(ILI9341_RAMWR);  Lcd_Write_Data(0x01);
}

void Lcd_Draw_Rect(unsigned short x, unsigned short y, unsigned short width, unsigned short height, uint16_t *data)
{
    Lcd_Set_Window(x, y, x + width - 1, y + height - 1);
	Lcd_Set_DC_CS(true, false);
	for(unsigned int i=0;i<width * height*2;i+=2)
	{
		uint8_t temp = *((uint8_t *)data+i);
		*((uint8_t *)data+i) = *((uint8_t *)data+i+1);
		*((uint8_t *)data+i+1) = temp;
	}

    write_sdio_spi_dma((uint32_t *)data, width * height*2);
}

