/**
 * @file HardwareSerial.h 
 *
 * @brief   HardwareSerial Module
 *
 * @author Huang Leilei
 *
 * Copyright (c) 2019 Winner Microelectronics Co., Ltd.
 */
#ifndef _HARDWARESERIAL_H_
#define _HARDWARESERIAL_H_

#include "debug.h"

#include "Stream.h"

#define USE_SEM 0 // Пока не знаю для чего здесь нужен семафор

class HardwareSerial : public Stream 
{
public:
    HardwareSerial(int serial_no);
    HardwareSerial(int serial_no, bool mul_flag);
    HardwareSerial() {}
	void begin();
    void begin(unsigned long baud);
    void begin(unsigned long baud, int modeChoose);

    virtual int read(void);         // from Stream
    virtual int available(void);    // from Stream
    virtual int peek();             // from Stream
    String readString();            // from Stream
    String readStringUntil(char terminator);    // from Stream

    virtual size_t write(uint8_t c); // from Print
    
private:
    int _uart_no;
    unsigned char *_pbuf;
#if USE_SEM
    tls_os_sem_t * _psem;
#endif
};

extern HardwareSerial Serial;

#endif
