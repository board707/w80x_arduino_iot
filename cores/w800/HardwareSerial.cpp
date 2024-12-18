/**
 * @file HardwareSerial.cpp
 *
 * @brief   HardwareSerial Module
 *
 * @author Huang Leilei
 *
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 */

#include <stdio.h>

#include "pins_arduino.h"
#include "HardwareSerial.h"

extern "C" {
#include "wm_uart.h"
#include "wm_osal.h"
#include "wm_gpio_afsel.h"
}

HardwareSerial Serial(0);

unsigned char _serial_buf[TLS_UART_RX_BUF_SIZE] = {0};

int _s_buf_begin = 0;
int _s_buf_end = 0;

#define TEST_DEBUG  0

extern "C" {
extern void tls_uart_tx_callback_register(u16 uart_no,s16(*tx_callback) (struct tls_uart_port * port));
extern s16 uart_tx_sent_callback(struct tls_uart_port *port);
extern struct tls_uart *tls_uart_open(u32 uart_no, TLS_UART_MODE_T uart_mode);

}

extern "C" int sendchar(int ch);
extern "C" int tls_uart_read(u16 uart_no, u8 * buf, u16 readsize);

s16 uart_rx_cb(u16 uart_no, unsigned short len,unsigned char * pbuf, int *pend)
{
    int ret = 0;
    do
    {
        ret = tls_uart_read(uart_no, pbuf + *pend, 1);
        if (ret > 0)
            (*pend) = *pend + ret;
    } while (ret != 0);
	return 0;
}


s16 uart0_rx_cb(unsigned short len, void *p)
{
	_s_buf_begin = 0;
	_s_buf_end = 0;
    return uart_rx_cb(TLS_UART_0, len, _serial_buf, &_s_buf_end);
}

int _read_byte(unsigned char *buf, int begin, int end)
{
    int c = 0;
    if (begin < TLS_UART_RX_BUF_SIZE
        && begin < end)
    {
        c = (int)(buf[begin]);
    }
    return c;
}

/**
 * @brief       This constructor is used to init hardware serial.
 * @param[in] serial_no Specify serial_no
 *
 * @return      None 
 * 
 * @note 
 */ 
HardwareSerial::HardwareSerial(int serial_no)
{
    HardwareSerial(serial_no, false);
}

/**
 * @brief       This constructor is used to init hardware serial.
 * @param[in] serial_no Specify serial_no
 * @param[in] mul_flag Specify mul_flag
 *
 *
 * @return      None 
 * 
 * @note 
 */ 
HardwareSerial::HardwareSerial(int serial_no, bool mul_flag)
{
    _uart_no = serial_no;
#if USE_SEM
    tls_os_sem_create(&_psem, _uart_no);
#endif

    if (TLS_UART_0 == _uart_no)
    {
        _pbuf = _serial_buf;
    } 
}



void HardwareSerial::begin(unsigned long baud, int modeChoose)
{
#if USE_SEM
    if (TLS_UART_0 == _uart_no)
        tls_os_sem_create(&_psem, 1);
    
#endif
 
    tls_uart_options_t opt;
    opt.charlength = TLS_UART_CHSIZE_8BIT;
    opt.flow_ctrl = TLS_UART_FLOW_CTRL_NONE;
    opt.paritytype = TLS_UART_PMODE_DISABLED;
    opt.stopbits = TLS_UART_ONE_STOPBITS;
    opt.baudrate = baud;
    
    tls_uart_port_init(_uart_no, &opt, modeChoose);
    if (TLS_UART_0 == _uart_no)
    {
       tls_uart_rx_callback_register(TLS_UART_0, uart0_rx_cb, NULL);
    }
}

void HardwareSerial::begin()
{
    begin(115200);
}

/**
 * @brief       Sets the data rate in bits per second (baud)
 *              for serial data transmission. For communicating
 *              with the computer, use one of these rates: 300,
 *              600, 1200, 2400, 4800, 9600, 14400, 19200, 28800,
 *              38400, 57600, or 115200. You can, however, specify
 *              other rates - for example, to communicate over pins
 *              0 and 1 with a component that requires a particular
 *              baud rate.
 *
 *              An optional second argument configures the data,
 *              parity, and stop bits. The default is 8 data bits,
 *              no parity, one stop bit.
 *
 * @param[in] baud speed: in bits per second (baud) - long
 *
 * @return      nothing 
 * 
 * @note 
 */ 
void HardwareSerial::begin(unsigned long baud)
{
    begin(baud, 0);
}

/**
 * @brief       Reads incoming serial data. read() inherits
 *              from the Stream utility class.
 * @param[in] None
 *
 * @return      the first byte of incoming serial data 
 *              available (or -1 if no data is available) - int 
 * 
 * @note 
 */ 
int HardwareSerial::read(void)
{
    int c = 0;
	if (_s_buf_end==_s_buf_begin) return -1;
    c = _read_byte(_serial_buf, _s_buf_begin, _s_buf_end);
    if (0 != c)
    {
        (_s_buf_begin) = (_s_buf_begin) + 1;
    }
    return c;
}

/**
 * @brief         Returns the next byte (character) of incoming serial
 *                data without removing it from the internal serial buffer.
 *                That is, successive calls to peek() will return the same
 *                character, as will the next call to read(). peek() 
 *                inherits from the Stream utility class.
 * @param[in] None
 * @return      the first byte of incoming serial data available (or -1 if
 *              no data is available) - int
 * 
 * @note 
 */ 
int HardwareSerial::peek()
{
     int c = 0;
	if (_s_buf_end==_s_buf_begin) return -1;
    c = _read_byte(_serial_buf, _s_buf_begin, _s_buf_end);
    
    return c;
}

/**
 * @brief       Writes binary data to the serial port. 
 *              This data is sent as a byte or series 
 *              of bytes; to send the characters representing
 *              the digits of a number use the print() function instead.
 * @param[in] c Specify the byte which will be sent to the console.
 * @return      The length of sending to the console.
 * 
 * @note 
 */ 
size_t HardwareSerial::write(uint8_t c)
{
    AR_DBG();
    tls_uart_write(_uart_no, (char *)&c, 1);
    return 1;
}

/**
 * @brief       Get the number of bytes (characters) available
 *              for reading from the serial port. This is data
 *              that's already arrived and stored in the serial
 *              receive buffer (which holds 64 bytes). available()
 *              inherits from the Stream utility class.
 * @param[in] none
 *
 * @return      the number of bytes available to read 
 * 
 * @note 
 */ 
int HardwareSerial::available(void)
{
	if (_s_buf_end > _s_buf_begin) 
		return (_s_buf_end - _s_buf_begin);
	else
		return 0;
    
}

// From Stream

String HardwareSerial::readString(){
	String ret;
	ret = static_cast<Stream *> (this)->readString();
	memset(_serial_buf, 0, _s_buf_begin-_s_buf_end);
	_s_buf_begin = 0;
	_s_buf_end = 0;
	return ret;	
}
	
String HardwareSerial::readStringUntil(char terminator){
	String ret;
	ret = static_cast<Stream *> (this)->readStringUntil(terminator);
	memset(_serial_buf, 0, _s_buf_begin-_s_buf_end);
	_s_buf_begin = 0;
	_s_buf_end = 0;
	return ret;	
}

