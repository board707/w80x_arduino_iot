#include "SPI.h"


extern "C" int32_t tls_spi_xfer(const void *data_out, void *data_in, uint32_t num_out, uint32_t num_in);

SPIClass SPI;

void SPIClass::begin(void)
{
	wm_spi_ck_config((tls_io_name)pin_Map[_sck].halPin);
	wm_spi_di_config((tls_io_name)pin_Map[_miso].halPin);
	wm_spi_do_config((tls_io_name)pin_Map[_mosi].halPin);
	if (_use_hard_cs) wm_spi_cs_config((tls_io_name)pin_Map[_ss].halPin);
}

void SPIClass::beginTransaction(SPISettings settings)
{
	this ->begin();
    tls_spi_setup(settings.dMode, TLS_SPI_CS_LOW, settings.clk);
	tls_spi_trans_type(SPI_DMA_TRANSFER);
	tls_spi_init();
}

void SPIClass::endTransaction(void)
{
	this->end();
}

void SPIClass::end(void)
{
    tls_gpio_cfg((tls_io_name)pin_Map[_sck].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
    tls_gpio_cfg((tls_io_name)pin_Map[_sck].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
    tls_gpio_cfg((tls_io_name)pin_Map[_sck].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
	if (_use_hard_cs) tls_gpio_cfg((tls_io_name)pin_Map[_ss].halPin, WM_GPIO_DIR_INPUT, WM_GPIO_ATTR_FLOATING);
}

void SPIClass::setDataOrder(uint8_t _dOrder)
{
    if (_dOrder == MSBFIRST)
    {
		_dataOrder = MSBFIRST;
    }
    else
    {
		_dataOrder = LSBFIRST;
    }
}

void SPIClass::setBitOrder(uint8_t _bOrder)
{
    if (_bOrder == MSBFIRST)
    {
		_bitOrder = MSBFIRST;
    }
    else
    {
		_bitOrder = LSBFIRST;
    }
}

void SPIClass::setDataMode(uint8_t _mode)
{
    uint8_t mode = 0;

    if (SPI_MODE0 == _mode)
    {
        mode = TLS_SPI_MODE_0;
    }
    else if (SPI_MODE1 == _mode)
    {
        mode = TLS_SPI_MODE_1;
    }
    else if (SPI_MODE2 == _mode)
    {
        mode = TLS_SPI_MODE_2;
    }
    else if (SPI_MODE3 == _mode)
    {
        mode = TLS_SPI_MODE_3;
    }
    spi_set_mode(mode);
}

void SPIClass::setFrequency(uint32_t freq)
{
    spi_set_sclk(freq);
}

uint8_t SPIClass::transfer(uint8_t data)
{
    uint8_t rxdata = 0;

	if(_bitOrder==LSBFIRST) data = reverseByte(data);
	tls_spi_xfer(&data, &rxdata, 1, 1);
	if(_bitOrder==LSBFIRST) rxdata = reverseByte(rxdata);
	
    return rxdata;
}

uint16_t SPIClass::transfer16(uint16_t data)
{
    uint16_t rxdata = 0;

  union
    {
         uint8_t _8recv[2];
         uint16_t recv;
    }in, out;

	in.recv = data;
	
    if (_bitOrder == LSBFIRST) {
        in._8recv[0] = reverseByte(in._8recv[0]);
        in._8recv[1] = reverseByte(in._8recv[1]);
    }
	
	tls_spi_xfer((uint8_t *)&in._8recv, (uint8_t *)&out._8recv, 2, 2);
	
   if (_bitOrder == LSBFIRST) {
        out._8recv[0] = reverseByte(out._8recv[0]);
        out._8recv[1] = reverseByte(out._8recv[1]);
    }
	
	rxdata = out.recv;
	
    return rxdata;
}

void SPIClass::transfer(void *_buf, size_t _count)
{  
    if(_dataOrder == MSBFIRST)
	{
		for(unsigned int i=0;i<_count;i+=2)
		{
			uint8_t temp = *((uint8_t *)_buf+i);
			*((uint8_t *)_buf+i) = *((uint8_t *)_buf+i+1);
			*((uint8_t *)_buf+i+1) = temp;
		}
	}
	if(_bitOrder == LSBFIRST) 
   {
		uint8_t buf2[_count];
		for (size_t i =0; i < _count; i++) {buf2[i] = reverseByte( *((uint8_t *)_buf + i));}
		tls_spi_write((uint8_t *)buf2, _count);
   } else 
	   tls_spi_write((uint8_t *)_buf, _count);
}

void SPIClass::transfer(void *_bufout, void *_bufin, size_t _count)
{
	if(_bufin == NULL) // Transmit only
	{
		this->transfer(_bufout, _count);
		return;
	}
	if(_bufout == NULL) //Receive only
	{
		if(_bitOrder == LSBFIRST)
		{
			tls_spi_read((uint8_t *)_bufin, _count);
			for (size_t i =0; i < _count; i++) { *((uint8_t *)_bufin + i) = reverseByte( *((uint8_t *)_bufin + i));}
		} else tls_spi_read((uint8_t *)_bufin, _count);
	}
	// Transmit-receive
	if(_bitOrder == LSBFIRST) 
	{	
        for (size_t i =0; i < _count; i++) {*((uint8_t *)_bufout + i) = reverseByte( *((uint8_t *)_bufout + i));}
        tls_spi_xfer((uint8_t *)_bufout, (uint8_t *)_bufin, _count, _count);
        for (size_t i =0; i < _count; i++) { *((uint8_t *)_bufin + i) = reverseByte( *((uint8_t *)_bufin + i));}
		
	}
	tls_spi_xfer((uint8_t *)_bufout,(uint8_t *)_bufin, _count, _count);
}

inline uint8_t SPIClass::reverseByte(uint8_t b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

bool SPIClass::setSPIpins(uint8_t mosi, uint8_t miso, uint8_t sck) {

    if (isSPIpins(mosi, miso, sck)) {
        _mosi = mosi;
        _miso = miso;
        _sck = sck;
        this-> begin();
        return true;
    } 
    return false;
}

bool SPIClass::isSPIpins(uint8_t mosi, uint8_t miso, uint8_t sck) {

    return ( ((pin_Map[mosi].ulPinAttribute & PIN_SPI_Msk) == _SPI_MOSI) && 
            ((pin_Map[miso].ulPinAttribute & PIN_SPI_Msk) == _SPI_MISO) && 
            ((pin_Map[sck].ulPinAttribute & PIN_SPI_Msk) == _SPI_SCK) ) ;
   
}

bool SPIClass::setHardCS(uint8_t cs)
{
	if(((pin_Map[cs].ulPinAttribute & PIN_SPI_Msk) == _SPI_SS))
	{
		_use_hard_cs = true;
		_ss = cs;
		spi_force_cs_out(0);
		this-> begin();
		return true;
	} else	return false;
}

void SPIClass::useSoftCS()
{
	_use_hard_cs=true;
	spi_force_cs_out(1);
	this-> begin();
}
