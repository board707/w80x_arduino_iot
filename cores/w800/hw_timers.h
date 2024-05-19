#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C"  {
#endif
#include "wm_timer.h"
#ifdef __cplusplus
}
#endif

#define ms 	TLS_TIMER_UNIT_MS
#define us	TLS_TIMER_UNIT_US


namespace hw_timer0 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
namespace hw_timer1 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
namespace hw_timer2 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
namespace hw_timer3 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
namespace hw_timer4 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
namespace hw_timer5 {
    extern volatile unsigned char timer_id;
    void set(uint8_t unit, unsigned long count, void (*f)());
    void start();
    void stop();
}
