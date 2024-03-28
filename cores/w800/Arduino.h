#include "pins_arduino.h"
// Определения для DIO

#define HIGH 			0x1
#define LOW  			0x0
#define LSBFIRST		0x0
#define MSBFIRST		0x1


// Режимы ввода/вывода GPIO

#define INPUT 			1
#define OUTPUT 			2
#define INPUT_PULLUP 	3
#define INPUT_PULLDOWN 	4
#define ANALOG_INPUT 	5
#define PWM_OUT 		6

#define UNUSED(X) (void)X 

// Определения для триггера прерывания от GPIO

#define RISING      	WM_GPIO_IRQ_TRIG_RISING_EDGE             
#define FALLING     	WM_GPIO_IRQ_TRIG_FALLING_EDGE           
#define CHANGE      	WM_GPIO_IRQ_TRIG_DOUBLE_EDGE     
#define HIGH_LEVEL  	WM_GPIO_IRQ_TRIG_HIGH_LEVEL         
#define LOW_LEVEL   	WM_GPIO_IRQ_TRIG_LOW_LEVEL 
         
typedef void (*gpio_irq_callback)();

#define IS_GPIO_IT_MODE(MODE) (((MODE) == HIGH_LEVEL )     	||\
								((MODE) == LOW_LEVEL )      ||\
								((MODE) == RISING)          ||\
								((MODE) == FALLING)         ||\
								((MODE) == CHANGE))

// Прототипы функций обслуживаня прерываний от GPIO

void attachInterrupt(uint8_t pin, gpio_irq_callback callback, uint8_t mode);
void detachInterrupt(uint8_t pin);
void arduino_gpio_isr( void *context);

//typedef enum {
//  LSBFIRST = 0,
//  MSBFIRST = 1,
//} BitOrder;



#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_include.h"

#ifdef __cplusplus
}
#endif

// Прототипы для DIO
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
void digitalToggle(uint8_t pin);
uint8_t digitalRead(uint8_t pin);

// Прототипы аналоговых функций

void analogWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);

// Прототипы для скетча Ардуино

void setup(void);
void loop(void);

// Прототипы для функций задержек и UPTIME

void delayMicroseconds(int us);
void delay(uint32_t ms);
u32 micros();
u32 millis();



