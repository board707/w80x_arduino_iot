/*LVGL 8.4 
Чтобы использовать графику LVGL необходимо сделать несколько шагов
1. Через менеджер библиотек загрузить LVGL версии 8.4
2. Скопировать /libraries/lvgl/lv_conf_template.h в /libraries/lv_conf.h (т.е переименованием)
3. В новом файле lv_conf.h включить контент. Это первый дефайн #if 0 Set it to "1" to enable content
4. Установить опцию #define LV_COLOR_16_SWAP 1 
*/
/*
  Connect your TFT display like this:
  TFT_VCC   to power
  TFT_GND   to ground
  TFT_CS    PB10
  TFT_LED   to power 
  TFT_SCL   PB6   
  TFT_MOSI  PB7   
  TFT_DC    PB8
  TFT_RST   PB9
*/

#include "Arduino.h"
#include "ILI9341_SDIO.h"
#include "lvgl.h"

static const uint16_t screenWidth  = LCD_WIDTH;
static const uint16_t screenHeight = LCD_HEIGHT;

extern "C" void vApplicationTickHook(void);		// Перехват системных тиков FreeRTOS

static lv_disp_draw_buf_t draw_buf ;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

#define TFT_DC  PB8
#define TFT_RST PB9
#define TFT_CS	PB10

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p )
{
  Lcd_Draw_Rect(area->x1,area->y1,area->x2 - area->x1 + 1,area->y2 - area->y1 + 1,(uint16_t*)color_p);
  lv_disp_flush_ready( disp_drv );
}
// Для работы с LVGL это обязательно, потому что мы не используем опцию LV_TICK_CUSTOM и не дергаем millis() из Arduino_Core 
void vApplicationTickHook(void)
{
	lv_tick_inc(2);
}

void setup()
{
    //Serial.begin( 115200 ); /* prepare for possible serial debug */
    
    //String LVGL_Arduino = "Hello Arduino! ";
    //LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

    //Serial.println( LVGL_Arduino );
    //Serial.println( "I am LVGL_Arduino" );
    
    Lcd_Init(TFT_DC, TFT_RST,TFT_CS);          /* TFT init. */
    Lcd_Orientation(1); /* Landscape orientation, flipped */
 
	lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight/10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    //Serial.println( "Setup done" );
    
    lv_obj_t * label1 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
                      "and wrap long text automatically.");
    lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);

    lv_obj_t * label2 = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label2, 240);
    lv_label_set_text(label2, "Hello World! This is LVGL 8.4 with w80x_arduino_iot project from RUSSIA. Based on W800-SDK ver 1.00.10");
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);

}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
