#include "Arduino.h"
#include "driver.h"

#define LCD_WIDTH        480
#define LCD_HEIGHT     320

#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04
#define MADCTL  0x36

uint16_t _width, _height;  //Ширина,высота после изменения ориентации
uint8_t dc_pin,rst_pin,cs_pin;     
u8* tbuf;

const uint8_t ili9488_init_seq[] = {
    /*  len , delay, cmd, data ... */
        0x05, 0x00, 0xF7, 0xA9, 0x51, 0x2C, 0x82,
        0x03, 0x00, 0xC0, 0x11, 0x09,
        0x02, 0x00, 0xC1, 0x41,
        0x04, 0x00, 0xC5, 0x00, 0x0A, 0x80,
		0x02, 0x00, 0xB0, 0x00,
        0x02, 0x00, 0xB1, 0xA0,
        0x02, 0x00, 0xB4, 0x02,
        0x04, 0x00, 0xB6, 0x02, 0x02, 0x3B,
        0x02, 0x00, 0xB7, 0xC6,
        0x03, 0x00, 0xBE, 0x00, 0x04,
        0x02, 0x00, 0xE9, 0x00,
        0x02, 0x00, 0x36, 0x08, 
        0x02, 0x00, 0x3A, 0x66, 
        0x10, 0x00, 0xE0, 0x00, 0x07, 0x10, 0x09, 0x17, 0x0B, 0x41, 0x89, 0x4B, 0x0A, 0x0C, 0x0E, 0x18, 0x1B, 0x0F,
        0x10, 0x00, 0xE1, 0x00, 0x17, 0x1A, 0x04, 0x0E, 0x06, 0x2F, 0x45, 0x43, 0x02, 0x0A, 0x09, 0x32, 0x36, 0x0F,	
        0x01, 0x78, 0x11,
        0x01, 0x02, 0x29,
        0
};

void Lcd_Set_DC_CS(bool dc, bool cs) {
  if (dc == true)  digitalWrite(dc_pin, HIGH);
  else  digitalWrite(dc_pin, LOW);
  if (cs == true)  digitalWrite(cs_pin, HIGH);
  else  digitalWrite(cs_pin, LOW);
}

void lcd_wait_idle()
{

}

void lcd_write_cmd_data(const uint8_t *cmd, size_t count) 
{
    lcd_wait_idle();
     Lcd_Set_DC_CS(0, 0);
    sdio_transfer(*cmd++);
    if (count >= 2) {
        lcd_wait_idle();
         Lcd_Set_DC_CS(1, 0);
        for (size_t i = 0; i < count - 1; ++i)
            sdio_transfer( *cmd++);
    }
    lcd_wait_idle();
     Lcd_Set_DC_CS(1, 1);
}

void lcd_write_ram_preare()
{
    uint8_t cmd = 0x2c; // RAMWR
    lcd_write_cmd_data( &cmd, 1);
     Lcd_Set_DC_CS(1, 0);
}

void spi_lcd_init_cmd(const uint8_t *cmd)
{
    while (*cmd) 
    {
        lcd_write_cmd_data(cmd + 2, *cmd);
        u16 _delay = *(cmd + 1);
       delay(_delay);
        cmd += *cmd + 2;
    }
 
}
void lcd_write_cmd(const uint8_t cmd) 
{
    lcd_wait_idle();
    Lcd_Set_DC_CS(0, 0);

    sdio_transfer(cmd);

    lcd_wait_idle();
    Lcd_Set_DC_CS(1, 1);
}
void lcd_write_data(const uint8_t data) 
{
    lcd_wait_idle();
    Lcd_Set_DC_CS(1, 0);
    
    sdio_transfer(data);

    lcd_wait_idle();
    Lcd_Set_DC_CS(1, 1);
}

void Lcd_Hard_Reset(void) // Аппаратный сброс
{
  digitalWrite(rst_pin, HIGH);
  delay(5);
  digitalWrite(rst_pin, LOW);
  delay(15);
  digitalWrite(rst_pin, HIGH);
  delay(15);
}
void lcd_set_windows(u16 x_start, u16 y_start, u16 x_end, u16 y_end)
{	
    // printf("---> x:%d y:%d x: %d y: %d\n", x_start, y_start, x_end, y_end);
	lcd_write_cmd(0x2A);	
	lcd_write_data(x_start>>8);
	lcd_write_data(0x00FF&x_start);		
	lcd_write_data(x_end>>8);
	lcd_write_data(0x00FF&x_end);

	lcd_write_cmd(0x2B);	
	lcd_write_data(y_start>>8);
	lcd_write_data(0x00FF&y_start);		
	lcd_write_data(y_end>>8);
	lcd_write_data(0x00FF&y_end);

	lcd_write_ram_preare();	//开始写入GRAM	
   
    //wait_sdio_spi_dma_ready();
    tls_mem_free(tbuf);
}

void Lcd_Orientation(uint8_t m) 
{
  lcd_write_cmd(MADCTL);
  uint8_t rotation = m & 3;
  switch (rotation) {
   case 0:
     lcd_write_data(MADCTL_MX | MADCTL_RGB);
     _width  = LCD_HEIGHT;
     _height = LCD_WIDTH;
     break;
   case 1:
     lcd_write_data(MADCTL_MV | MADCTL_RGB);
     _width  = LCD_WIDTH;
     _height = LCD_HEIGHT;
     break;
  case 2:
    lcd_write_data(MADCTL_MY | MADCTL_RGB);
     _width  = LCD_HEIGHT;
     _height = LCD_WIDTH;
    break;
   case 3:
     lcd_write_data(MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_RGB);
     _width  = LCD_WIDTH;
     _height = LCD_HEIGHT;
     break;
  }
}
void Lcd_Init(uint8_t dc, uint8_t rst,uint8_t cs) {
  pinMode(dc, OUTPUT); dc_pin = dc;
  pinMode(rst, OUTPUT); rst_pin = rst;
  pinMode(cs, OUTPUT); cs_pin = cs;
  sdio_init();
  Lcd_Hard_Reset();
  spi_lcd_init_cmd(ili9488_init_seq);
}

#define swap(a, b) { uint8_t t = a; a = b; b = t; }

void Lcd_Draw_Rect(unsigned short x, unsigned short y, unsigned short width, unsigned short height, uint16 *data)
{
    lcd_set_windows(x, y, x + width - 1, y + height - 1);
    u16 *p = (u16 *)data;
    tbuf = (u8 *)tls_mem_alloc((width * height * 3));
	
	union {
		uint16_t val;
		struct {
			uint8_t msb;
			uint8_t lsb;
		};
	} pixel;

	/*
	 * Правило преобразования rgb555 ->rgb666
	 * R6[5:0] = (R5[4:0]<<1)+(R5[4]>>5)
	 * G6[5:0] = (R6[5:0])
	 * B6[5:0] = (B5[4:0]<<1)+(B5[4]>>5)
	 */

    for(u32 i = 0; i < width * height * 3; i++)
    {
		pixel.val = (u16)*p;
		swap(pixel.msb,pixel.lsb); // Swap нужен для того, чтобы не менять параметры в lv_conf.h и сделать работу с любыми чипами LCD единообразно
		u16 pix = pixel.val;
		u16 b = pix & 0x1f;
		u16 g = (pix & (0x3f << 5)) >> 5;
		u16 r = (pix & (0x1f << 11)) >> 11;

		u8 r8 = (r & 0x1F) << 3;
		u8 g8 = (g & 0x3F) << 2;
		u8 b8 = (b & 0x1F) << 3;

       if(i%3==0) 
            tbuf[i] =b8;
        if(i%3==1) 
           tbuf[i] = g8;
        if(i%3==2){
            tbuf[i] =r8;
            p++;
        }
    }

    write_sdio_spi_dma((u32 *)tbuf, width * height * 3);
    tls_mem_free(tbuf);

}
