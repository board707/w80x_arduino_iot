#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__
#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_include.h"

#ifdef __cplusplus
}
#endif


// Определения для триггера прерывания от GPIO
#ifndef RISING
#define RISING      	WM_GPIO_IRQ_TRIG_RISING_EDGE  
#endif 
#ifndef FALLING         
#define FALLING     	WM_GPIO_IRQ_TRIG_FALLING_EDGE           
#endif
#ifndef CHANGE
#define CHANGE      	WM_GPIO_IRQ_TRIG_DOUBLE_EDGE     
#endif
#ifndef HIGH_LEVEL
#define HIGH_LEVEL  	WM_GPIO_IRQ_TRIG_HIGH_LEVEL         
#endif
#ifndef LOW_LEVEL
#define LOW_LEVEL   	WM_GPIO_IRQ_TRIG_LOW_LEVEL 
#endif
         
typedef void (*gpio_irq_callback)();
// Структура, описывающая контекст прерываний от GPIO
struct arduino_context {
	gpio_irq_callback callback;	// Указатель на обработчик прерывания 
	uint16_t pin;				// Пин, к которому привязан обработчик
};

#define IS_GPIO_IT_MODE(MODE) (((MODE) == HIGH_LEVEL )     	||\
								((MODE) == LOW_LEVEL )      ||\
								((MODE) == RISING)          ||\
								((MODE) == FALLING)         ||\
								((MODE) == CHANGE))

// Прототипы функций обслуживаня прерываний от GPIO

void attachInterrupt(uint8_t pin, gpio_irq_callback callback, uint8_t mode);
void detachInterrupt(uint8_t pin);
void arduino_gpio_isr( void *context);

#endif
