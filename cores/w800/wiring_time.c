#include "wiring_time.h"

extern int _delay(int us); //Используем библиотечную функцию задержки из iperf_timer.c
uint32_t csi_coret_get_load (void);
uint32_t csi_coret_get_value (void);

// Функции задержки

void delayMicroseconds(uint32_t us)
{
	
	if (us > 1000)
	{
		delay(us / 1000);
		us = us % 1000;
	}
	if (us == 0) return;

  uint32_t cnt,cur,start,load;
  tls_sys_clk sysclk;
  tls_sys_clk_get (&sysclk);
  cnt = sysclk.cpuclk * us;
  load = csi_coret_get_load ();
  start = csi_coret_get_value ();

  while (1)
    {
      cur = csi_coret_get_value ();

      if (start > cur)
        {
          if (start - cur >= cnt)
            {
              return;
            }
        }
      else
        {
          if (load - cur + start > cnt)
            {
              return;
            }
        }
    }
}

void delay(uint32_t ms) 
{
	_delay(ms*1000);
}

// Функции UPTIME

uint32_t millis()
{
	return (micros()/1000);
}

uint32_t micros()
{
	return (tls_os_get_time()*1000/HZ) *1000;
}
