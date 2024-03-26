#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_include.h"

#ifdef __cplusplus
}
#endif

// Структура, описывающая контекст прерываний от GPIO

struct arduino_context {
	gpio_irq_callback callback;	// Указатель на обработчик прерывания 
	uint16_t pin;				// Пин, к которому привязан обработчик
};
