#include "Arduino.h"
#include "HardwareI2C.h"

extern "C" void tls_i2c_init(u32 freq);
extern "C" void wm_i2c_scl_config(enum tls_io_name io_name);
extern "C" void wm_i2c_sda_config(enum tls_io_name io_name);
extern "C" void tls_i2c_stop(void);
extern "C" void tls_i2c_write_byte(u8 data, u8 ifstart);
extern "C" int tls_i2c_wait_ack(void);
extern "C" uint8_t tls_i2c_read_byte(u8 ifack, u8 ifstop);

HardwareI2C::HardwareI2C() {}
HardwareI2C::~HardwareI2C() {}

void HardwareI2C::begin() {
	
    wm_i2c_scl_config(WM_IO_PA_01);
    wm_i2c_sda_config(WM_IO_PA_04);
	tls_i2c_init(100000);		// По дефолту 100 кГц
}
void HardwareI2C::setClock(int clockFrequency) {

	tls_i2c_init(clockFrequency);
	
}
uchar HardwareI2C::beginTransmission(uchar addr) {

  tls_i2c_write_byte(((addr << 1) & 0xFE), 1); // Предваряя START сигнал
  if (tls_i2c_wait_ack() != WM_SUCCESS) 
	{
		tls_i2c_stop();
		return GETNAK;
	} else
	return GETACK;
}

uchar HardwareI2C::endTransmission() {
	tls_i2c_stop();	
	return 0;
}

uchar HardwareI2C::write(uchar dta) {
	tls_i2c_write_byte(dta, 0);	// Без сигнала START
    if (tls_i2c_wait_ack() != WM_SUCCESS)
	{
		tls_i2c_stop(); 
		return GETNAK;
	}	else
		return GETACK;
}
uchar HardwareI2C::write(uint16_t len, uchar* dta) {
    for (uint16_t i = 0; i < len; i++) {
		tls_i2c_write_byte(dta[i], 0);	// Без сигнала START
		if (tls_i2c_wait_ack() != WM_SUCCESS) 
		{	tls_i2c_stop(); 
			return GETNAK;
		}
    }
	return GETACK;
}
uchar HardwareI2C::requestFrom(uchar addr, uint16_t len) {
	
	recv_len = len;
	tls_i2c_write_byte(((addr << 1) | 0x01), 1); //Режим чтения
    if (tls_i2c_wait_ack() != WM_SUCCESS) 
		{
			tls_i2c_stop(); 
			recv_len = 0;
			return GETNAK;
		}	else
		return GETACK;

}

uchar HardwareI2C::read() {
	uchar dta;
    if (recv_len <= 0) {
        return 0;
    }
	recv_len--;	
	if (recv_len > 0) {dta = tls_i2c_read_byte(1, 0);
	} // Отправляем ACK без сигнала STOP
	else {dta = tls_i2c_read_byte(0, 1);
	//delayMicroseconds(10);
	}				// Последний принимаемый байт - Отправляем NAK и сигнал STOP
	return dta;	
}