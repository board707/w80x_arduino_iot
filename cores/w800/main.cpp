
extern  void setup();
extern  void loop();

#ifdef __cplusplus
extern "C"  {
#endif

#include <wm_include.h>
#include <wm_cpu.h>

void __cxa_pure_virtual() { while (1); }


void UserMain(void)
{
	#if (F_CPU ==   240000000)
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_240M;
	#elif (F_CPU == 160000000)
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_160M;
	#elif (F_CPU == 80000000)
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_80M;
	#elif (F_CPU == 40000000)
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_40M;
	#elif (F_CPU == 2000000)
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_2M;
	#else
		uint32_t f_cpu_dir = (uint32_t)CPU_CLK_80M;
	#endif

	tls_sys_clk_set(f_cpu_dir);
	setup();
    while(1) 
	{
        loop();
    }

}
#ifdef __cplusplus
}
#endif

