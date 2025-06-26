#pragma once

#include "Arduino.h"

#ifdef __cplusplus
extern "C"  {
#endif

#include "wm_sdio_host.h"
#include "wm_pmu.h"
#include "wm_dma.h"

#ifdef __cplusplus
}
#endif

#define LCD_SCL_PIN    	WM_IO_PB_06   // SCL - PB6
#define LCD_MOSI_PIN  	WM_IO_PB_07   // MOSI - PB7

static u8   sdio_spi_dma_channel  = 0xFF;

static int sdio_spi_dma_cfg(u32*mbuf,u32 bufsize,u8 dir)
{
    int ch;
    u32 addr_inc = 0;

    ch = tls_dma_request(0, 0);
    DMA_CHNLCTRL_REG(ch) = DMA_CHNL_CTRL_CHNL_OFF;

    if(dir)
    {
        DMA_SRCADDR_REG(ch) = (unsigned int)mbuf;
        DMA_DESTADDR_REG(ch) = (unsigned int)SDIO_HOST->DATA_BUF;
        addr_inc = DMA_CTRL_SRC_ADDR_INC;
    }
    else
    {
        DMA_SRCADDR_REG(ch) = (unsigned int)SDIO_HOST->DATA_BUF;
        DMA_DESTADDR_REG(ch) = (unsigned int)mbuf;
        addr_inc =DMA_CTRL_DEST_ADDR_INC;
    }

    DMA_CTRL_REG(ch) = addr_inc | DMA_CTRL_DATA_SIZE_WORD | (bufsize << 8);
    DMA_MODE_REG(ch) = DMA_MODE_SEL_SDIOHOST | DMA_MODE_HARD_MODE;

    DMA_CHNLCTRL_REG(ch) = DMA_CHNL_CTRL_CHNL_ON;

    return ch;
}

void sdio_init() {
	tls_io_cfg_set(LCD_SCL_PIN, WM_IO_OPTION2);
	tls_io_cfg_set(LCD_MOSI_PIN, WM_IO_OPTION2);
    tls_open_peripheral_clock(TLS_PERIPHERAL_TYPE_SDIO_MASTER);
    tls_sys_clk sysclk;
    tls_sys_clk_get(&sysclk);
    SDIO_HOST->MMC_CARDSEL = 0xC0 | (sysclk.cpuclk / 2 - 1); // enable module, enable mmcclk
    //SDIO_HOST->MMC_CTL = 0x542 | 0 << 3;// auto transfer, mmc mode. 
    SDIO_HOST->MMC_CTL = 0x542 | (0b001 << 3); // 001 1/4
    // SDIO_HOST->MMC_CTL = 0x542 | (0b010 << 3); // 010 1/6
    // SDIO_HOST->MMC_CTL = 0x542 | (0b011 << 3); // 011 1/8
    // SDIO_HOST->MMC_CTL = 0x542 | (0b100 << 3); // 100 1/10
    // SDIO_HOST->MMC_CTL = 0x542 | (0b101 << 3); // 101 1/12
    // SDIO_HOST->MMC_CTL = 0x542 | (0b110 << 3); // 110 1/14
    // SDIO_HOST->MMC_CTL = 0x542 | (0b111 << 3); // 111 1/16
    SDIO_HOST->MMC_INT_MASK = 0x100; // unmask sdio data interrupt.
    SDIO_HOST->MMC_CRCCTL = 0x00; 
    SDIO_HOST->MMC_TIMEOUTCNT = 0;
    SDIO_HOST->MMC_BYTECNTL = 0;
}

void sdio_transfer(u8 d)
{
    SDIO_HOST->BUF_CTL = 0x4820;
    SDIO_HOST->DATA_BUF[0] = d;

    SDIO_HOST->MMC_BYTECNTL = 1;
    SDIO_HOST->MMC_IO = 0x01;
    while (1) {
        if ((SDIO_HOST->MMC_IO & 0x01) == 0x00)
            break;
    }
}

void write_sdio_spi_dma(u32* data, u32 len) 
{
    while(1){
        if ((SDIO_HOST->MMC_IO & 0x01) == 0x00)
            break;
    }
    u32 offset=0;
    while(len>0){
        int datalen=len;
        if(len>0xfffc)
            datalen=0xfffc;
        len-=datalen;

        SDIO_HOST->BUF_CTL = 0x4000; //disable dma,
        sdio_spi_dma_channel = sdio_spi_dma_cfg((u32 *) data+offset, datalen, 1);
        SDIO_HOST->BUF_CTL = 0xC20; //enable dma, write sd card
        SDIO_HOST->MMC_INT_SRC |= 0x7ff; // clear all firstly
        SDIO_HOST->MMC_BYTECNTL = datalen;
        SDIO_HOST->MMC_IO = 0x01;
        offset+=datalen/4;
        while(1){
            if ((SDIO_HOST->BUF_CTL & 0x400) == 0x00)
                break;
        }
        tls_dma_free(sdio_spi_dma_channel);
    }
}

