#pragma once
#include <wm_include.h>
#include <csi_core.h>
#include <wm_cpu.h>

// Прототипы для функций задержек и UPTIME

void delayMicroseconds(uint32_t us);
void delay(uint32_t ms);
uint32_t micros();
uint32_t millis();

