#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include "Arduino.h"
#include <stdio.h>



extern "C" {
#include "wm_hostspi.h"
#include "wm_gpio_afsel.h"
#include "wm_spi_hal.h"
}

#define SPI_SPEED_CLOCK_DEFAULT     2000000


// SPI mode parameters for SPISettings
#define SPI_MODE0 0x00
#define SPI_MODE1 0x01
#define SPI_MODE2 0x02
#define SPI_MODE3 0x03

class SPISettings {
public:
    SPISettings(uint32_t clock, uint8_t dataOrder, uint8_t dataMode) {
      clk = clock;

      dOrder = dataOrder;

      if(SPI_MODE0 == dataMode) {
        dMode = TLS_SPI_MODE_0;
      } else if(SPI_MODE1 == dataMode) {
        dMode = TLS_SPI_MODE_1;
      } else if(SPI_MODE2 == dataMode) {
        dMode = TLS_SPI_MODE_2;
      } else if(SPI_MODE3 == dataMode) {
        dMode = TLS_SPI_MODE_3;
      }

    }
    SPISettings() {
      clk = SPI_SPEED_CLOCK_DEFAULT;
      dOrder = MSBFIRST;
      dMode = TLS_SPI_MODE_0;
    }
private:
    uint32_t clk;       //specifies the spi bus maximum clock speed
    uint8_t bOrder;     //bit order (MSBFirst or LSBFirst)
	uint8_t dOrder;		// data order (BigEndian or LittleEndian)
    uint8_t dMode;      //one of the data mode
                        //Mode          Clock Polarity (CPOL)   Clock Phase (CPHA)
                        //SPI_MODE0             0                     0
                        //SPI_MODE1             0                     1
                        //SPI_MODE2             1                     0
                        //SPI_MODE3             1                     1
    friend class SPIClass;
};

class SPIClass {

private:
	// Пины по умолчанию
    uint8_t   _miso = PIN_SPI_MISO;
    uint8_t   _mosi = PIN_SPI_MOSI;
    uint8_t   _sck  = PIN_SPI_SCK;
	uint8_t   _ss 	= PIN_SPI_SS;
	bool 	  _use_hard_cs = false;
	uint16_t  _bitOrder = MSBFIRST;  // Старший бит вперед
	uint16_t  _dataOrder = MSBFIRST; // Старший байт вперед
	
public:

    void begin(void);
    void end(void);
    
    void beginTransaction(SPISettings settings);
    void endTransaction(void);

    uint8_t transfer(uint8_t _data);
    uint16_t transfer16(uint16_t _data);
    void transfer(void *_buf, size_t _count);
    void transfer(void *_bufout, void *_bufin, size_t _count);

    void setBitOrder(uint8_t);
	void setDataOrder(uint8_t);
    void setDataMode(uint8_t _mode);
    void setFrequency(uint32_t freq);
    bool setSPIpins(uint8_t mosi, uint8_t miso, uint8_t sck);
    bool setHardCS(uint8_t cs);
	uint8_t reverseByte(uint8_t b);	
	bool isSPIpins(uint8_t mosi, uint8_t miso, uint8_t sck);
	void useSoftCS();
	
private:
    SPISettings spiSettings;
};

extern SPIClass SPI;

#endif
