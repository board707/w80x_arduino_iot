
#include "Arduino.h"
#include "pins_arduino.h"
#include "Interrupts.h"

extern "C" int _delay(int us); //Используем библиотечную функцию задержки из iperf_timer.c

extern "C" void wm_pwm0_config(enum tls_io_name); 
extern "C" void wm_pwm1_config(enum tls_io_name);
extern "C" void wm_pwm2_config(enum tls_io_name);
extern "C" void wm_pwm3_config(enum tls_io_name);
extern "C" void wm_pwm4_config(enum tls_io_name);
extern "C" void wm_adc_config(uint8_t Channel);
extern "C" int adc_get_inputVolt(uint8_t channel);

const PIN_MAP pin_Map[] =
{
    { WM_IO_PA_00,    PA0,    MUX_PA0},
    { WM_IO_PA_01,    PA1,    MUX_PA1},
    { WM_IO_PA_02,    PA2,    MUX_PA2},
    { WM_IO_PA_03,    PA3,    MUX_PA3},
    { WM_IO_PA_04,    PA4,    MUX_PA4},
    { WM_IO_PA_05,    PA5,    MUX_PA5},
    { WM_IO_PA_06,    PA6,    MUX_PA6},
    { WM_IO_PA_07,    PA7,    MUX_PA7},
    { WM_IO_PA_08,    PA8,    MUX_PA8},
    { WM_IO_PA_09,    PA9,    MUX_PA9},
    { WM_IO_PA_10,   PA10,   MUX_PA10},
    { WM_IO_PA_11,   PA11,   MUX_PA11},
    { WM_IO_PA_12,   PA12,   MUX_PA12},
    { WM_IO_PA_13,   PA13,   MUX_PA13},
    { WM_IO_PA_14,   PA14,   MUX_PA14},
    { WM_IO_PA_15,   PA15,   MUX_PA15},

    { WM_IO_PB_00,    PB0,    MUX_PB0},
    { WM_IO_PB_01,    PB1,    MUX_PB1},
    { WM_IO_PB_02,    PB2,    MUX_PB2},
    { WM_IO_PB_03,    PB3,    MUX_PB3},
    { WM_IO_PB_04,    PB4,    MUX_PB4},
    { WM_IO_PB_05,    PB5,    MUX_PB5},
    { WM_IO_PB_06,    PB6,    MUX_PB6},
    { WM_IO_PB_07,    PB7,    MUX_PB7},
    { WM_IO_PB_08,    PB8,    MUX_PB8},
    { WM_IO_PB_09,    PB9,    MUX_PB9},
    { WM_IO_PB_10,   PB10,   MUX_PB10},
    { WM_IO_PB_11,   PB11,   MUX_PB11},
    { WM_IO_PB_12,   PB12,   MUX_PB12},
    { WM_IO_PB_13,   PB13,   MUX_PB13},
    { WM_IO_PB_14,   PB14,   MUX_PB14},
    { WM_IO_PB_15,   PB15,   MUX_PB15},
    { WM_IO_PB_16,   PB16,   MUX_PB16},
    { WM_IO_PB_17,   PB17,   MUX_PB17},
    { WM_IO_PB_18,   PB18,   MUX_PB18},
    { WM_IO_PB_19,   PB19,   MUX_PB19},
    { WM_IO_PB_20,   PB20,   MUX_PB20},
    { WM_IO_PB_21,   PB21,   MUX_PB21}, 
    { WM_IO_PB_22,   PB22,   MUX_PB22},
    { WM_IO_PB_23,   PB23,   MUX_PB23},
    { WM_IO_PB_24,   PB24,   MUX_PB24},
    { WM_IO_PB_25,   PB25,   MUX_PB25}, 
    { WM_IO_PB_26,   PB26,   MUX_PB26},
    { WM_IO_PB_27,   PB27,   MUX_PB27}
};

// Инициализация пинов

void pinMode(uint8_t pin, uint8_t mode)
{
	uint8_t pwm_channel = 0;
	
	if(pin_Map[pin].ulPinAttribute != NONE) 
	{
		if(pin_Map[pin].ulPinAttribute & PIN_DIO_Msk) 
		{
			tls_io_cfg_set((tls_io_name)pin_Map[pin].halPin, WM_IO_OPT5_GPIO);
			
			if ((mode == INPUT) || (mode == OUTPUT) || (mode == INPUT_PULLUP) || (mode == INPUT_PULLDOWN))
			{
				switch (mode)
				{
					case INPUT:
						tls_gpio_cfg((tls_io_name)pin_Map[pin].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
						break;
					case INPUT_PULLUP:
						tls_gpio_cfg((tls_io_name)pin_Map[pin].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_PULLHIGH);
						break;
					case INPUT_PULLDOWN:
						tls_gpio_cfg((tls_io_name)pin_Map[pin].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_PULLLOW);
						break;
					case OUTPUT:
						tls_gpio_cfg((tls_io_name)pin_Map[pin].halPin, WM_GPIO_DIR_OUTPUT, WM_GPIO_ATTR_FLOATING);
						break;
					default:
						break;
				}
			}	
		}
		if(pin_Map[pin].ulPinAttribute & PIN_PWM_Msk)
		{
			if(mode == PWM_OUT) 
			{
				switch(pin_Map[pin].ulPinAttribute & PIN_PWM_Msk) {
					case PWM0:
					pwm_channel = 0;
					wm_pwm0_config((tls_io_name)pin_Map[pin].halPin);
					break;
					case PWM1:
					pwm_channel = 1;
					wm_pwm1_config((tls_io_name)pin_Map[pin].halPin);
					break;
					case PWM2:
					pwm_channel = 2;
					wm_pwm2_config((tls_io_name)pin_Map[pin].halPin);
					break;
					case PWM3:
					pwm_channel = 3;
					wm_pwm3_config((tls_io_name)pin_Map[pin].halPin);
					break;
					case PWM4:
					pwm_channel = 4;
					wm_pwm4_config((tls_io_name)pin_Map[pin].halPin);
					break;
					default:
					break;
				}
				tls_pwm_stop(pwm_channel);
				uint16_t ret = tls_pwm_init(pwm_channel, 39000, 0, 0); // Значения по умолчанию Freq=39kHz, Duty=0; Pulse=0 - непрерывный режим
				if(ret != WM_SUCCESS) printf("\n Error init pwm");
			}	
		}
		if(pin_Map[pin].ulPinAttribute & PIN_ADC_Msk) 
		{
			if(mode == ANALOG_INPUT)
			{
				switch(pin_Map[pin].ulPinAttribute & PIN_ADC_Msk) {
					case ADC1:
					wm_adc_config(0);
					break;
					case ADC2:
					wm_adc_config(1);
					break;
					case ADC3:
					wm_adc_config(2);
					break;
					case ADC4:
					wm_adc_config(3);
					break;
					default:
					break;
				}
			}	
		}	
	}	
}

// Функции цифрового ввода-вывода

void digitalWrite(uint8_t pin, uint8_t val)
{
	tls_gpio_write((tls_io_name)pin_Map[pin].halPin, val);
}

uint8_t digitalRead(uint8_t pin) 
{
    return tls_gpio_read((tls_io_name)pin_Map[pin].halPin);
}

void digitalToggle(uint8_t pin)
{
	digitalWrite(pin, !digitalRead(pin));
}

// Функции задержки

void delayMicroseconds(int us)
{
	_delay(us);
}

void delay(uint32_t ms) 
{
	_delay(ms*1000);
}

// Функции UPTIME

u32 millis()
{
	return (micros()/1000);
}

u32 micros()
{
	return (tls_os_get_time()*1000/HZ) *1000;
}

// Функции обслуживания GPIO прерываний

static struct arduino_context ardu_gpio_context[PINS_COUNT] = {{NULL,0xFFFF}}; // Глобальный массив активных прерываний - Ардуино контекст

void attachInterrupt(uint8_t pin, gpio_irq_callback callback, uint8_t mode)
{
	
	if (pin_Map[pin].ulPinAttribute & PIN_DIO_Msk)
	{
		if (IS_GPIO_IT_MODE(mode))
		{
			ardu_gpio_context[pin_Map[pin].halPin].callback = callback;
			ardu_gpio_context[pin_Map[pin].halPin].pin = pin_Map[pin].halPin;
			tls_gpio_isr_register((tls_io_name)pin_Map[pin].halPin, arduino_gpio_isr, &ardu_gpio_context[pin_Map[pin].halPin]);
			tls_gpio_irq_enable((tls_io_name)pin_Map[pin].halPin, (tls_gpio_irq_trig)mode);
		}
	}	
}

// Главный маршрутизатор прерываний

void arduino_gpio_isr(void *context)
{
	struct arduino_context *p = (arduino_context*)context;
	
	if(tls_get_gpio_irq_status((tls_io_name)p->pin))
	{
		tls_clr_gpio_irq_status((tls_io_name)p->pin);	// Сброс флага прерывания
		if (p->callback != NULL) (* p->callback)();		// Запуск обработчика прерывания в скетче
	}
}	

void detachInterrupt(uint8_t pin)
{
	if (pin_Map[pin].ulPinAttribute & PIN_DIO_Msk)
	{
		ardu_gpio_context[pin_Map[pin].halPin].callback = NULL;
		ardu_gpio_context[pin_Map[pin].halPin].pin = 0xFFFF;
		tls_gpio_irq_disable((tls_io_name)pin_Map[pin].halPin);
	}
}

// Аналоговые функции

void analogWrite(uint8_t pin, uint8_t val)
{
	switch(pin_Map[pin].ulPinAttribute & PIN_PWM_Msk) {
	case PWM0:
		tls_pwm_duty_set(0, val);
		break;
	case PWM1:
		tls_pwm_duty_set(1, val);
		break;
	case PWM2:
		tls_pwm_duty_set(2, val);
		break;
	case PWM3:
		tls_pwm_duty_set(3, val);
		break;
	case PWM4:
		tls_pwm_duty_set(4, val);
		break;
	default:
		break;
	}
}

int analogRead(uint8_t pin) // Возвращаемая величина в милливольтах!
{
	int result = 0;
	
	switch(pin_Map[pin].ulPinAttribute & PIN_ADC_Msk) {
	case ADC1:
		result = adc_get_inputVolt(0);
		break;
	case ADC2:
		result = adc_get_inputVolt(1);
		break;
	case ADC3:
		result = adc_get_inputVolt(2);
		break;
	case ADC4:
		result = adc_get_inputVolt(3);
		break;
	default:
		break;
	}
	return result;
}