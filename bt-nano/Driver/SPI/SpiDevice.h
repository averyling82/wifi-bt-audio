/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\SPI\SpiDevice.h
* Owner: Aaron.sun
* Date: 2014.5.30
* Time: 9:07:43
* Desc: SPI DEVICE CLASS
* History:
*    <author>	 <date> 	  <time>	 <version>	   <Desc>
*    Aaron.sun     2014.5.30     9:07:43   1.0
********************************************************************************************
*/

#ifndef __DRIVER_SPI_SPIDEVICE_H__
#define __DRIVER_SPI_SPIDEVICE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _DRIVER_SPI_SPIDEVICE_COMMON_  __attribute__((section("driver_spi_spidevice_common")))
#define _DRIVER_SPI_SPIDEVICE_INIT_  __attribute__((section("driver_spi_spidevice_init")))
#define _DRIVER_SPI_SPIDEVICE_SHELL_  __attribute__((section("driver_spi_spidevice_shell")))



typedef rk_err_t (* pSpiRxTx_complete)(void* buffer);      //tx callback funciton

typedef struct _SPI_DEV_ARG
{
    uint32 Ch;
    uint32 SpiRate;
    uint32 CtrlMode;

}SPI_DEV_ARG;

#define SPI_DEVICE_MAX 2

#define SPI_CTL_MODE (SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_CK | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW\
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_BUS_CLK (12 * 1000 * 1000)

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t SpiDev_Realse_Channel(HDC dev,  uint32 Ch);
extern rk_err_t SpiDev_SetChannel(HDC dev, uint32 Ch, uint32 SpiRate, uint32 CtrlMode);
extern rk_size_t SpiDev_Read_Write(HDC dev, uint8 * ReadBuf, uint8 * WriteBuf, uint32 Size, uint32 Mode, pSpiRxTx_complete pfRxTxComplete);
extern rk_err_t SpiDev_Shell(HDC dev, uint8 * pstr);
extern rk_size_t SpiDev_Write(HDC dev, uint8 * pBuf, uint32 size, uint32 Mode, pSpiRxTx_complete pfTxComplete);
extern rk_size_t SpiDev_Read(HDC dev, uint8 * pBuf, uint32 size, uint32 DummySize, uint32 Mode, pSpiRxTx_complete pfRxComplete);
extern HDC SpiDev_Create(uint32 DevID, void * arg);



#endif

