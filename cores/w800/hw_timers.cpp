#include "hw_timers.h"

volatile unsigned char hw_timer0::timer_id;
volatile unsigned char hw_timer1::timer_id;
volatile unsigned char hw_timer2::timer_id;
volatile unsigned char hw_timer3::timer_id;
volatile unsigned char hw_timer4::timer_id;
volatile unsigned char hw_timer5::timer_id;

void hw_timer0::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer0::start() {

    tls_timer_start(timer_id);
}
void hw_timer0::stop() {

    tls_timer_stop(timer_id);
}
void hw_timer1::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer1::start() {

    tls_timer_start(timer_id);
}
void hw_timer1::stop() {

    tls_timer_stop(timer_id);
}
void hw_timer2::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer2::start() {

    tls_timer_start(timer_id);
}
void hw_timer2::stop() {

    tls_timer_stop(timer_id);
}
void hw_timer3::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer3::start() {

    tls_timer_start(timer_id);
}
void hw_timer3::stop() {

    tls_timer_stop(timer_id);
}
void hw_timer4::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer4::start() {

    tls_timer_start(timer_id);
}
void hw_timer4::stop() {

    tls_timer_stop(timer_id);
}
void hw_timer5::set(uint8_t unit, unsigned long count, void (*f)()) {

    struct tls_timer_cfg timer_cfg;
    
    timer_cfg.unit = (tls_timer_unit)unit;
    timer_cfg.timeout = count;
    timer_cfg.is_repeat = 1;
    timer_cfg.callback = (tls_timer_irq_callback)f;
    timer_cfg.arg = NULL;
    timer_id = tls_timer_create(&timer_cfg);
}
void hw_timer5::start() {

    tls_timer_start(timer_id);
}
void hw_timer5::stop() {

    tls_timer_stop(timer_id);
}

