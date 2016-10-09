/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Driver\I2C\I2CDevice.h
* Owner: HJ
* Date: 2014.3.10
* Time:
* Desc: Uart Device Class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
*      HJ     2014.3.10                    1.0
********************************************************************************************
*/

#ifndef __DRIVER_I2S_I2SDEVICE_H__
#define __DRIVER_I2S_I2SDEVICE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _DRIVER_I2S_I2SDEVICE_COMMON_  __attribute__((section("driver_i2s_i2sdevice_common")))
#define _DRIVER_I2S_I2SDEVICE_INIT_  __attribute__((section("driver_i2s_i2sdevice_init")))
#define _DRIVER_I2S_I2SDEVICE_SHELL_  __attribute__((section("driver_i2s_i2sdevice_shell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define _DRIVER_I2S_I2SDEVICE_COMMON_DATA_      _DRIVER_I2S_I2SDEVICE_COMMON_
#define _DRIVER_I2S_I2SDEVICE_SHELL_DATA_       _DRIVER_I2S_I2SDEVICE_SHELL_
#elif defined(__arm__) && defined(__GNUC__)
#define _DRIVER_I2S_I2SDEVICE_COMMON_DATA_  __attribute__((section("driver_i2s_i2sdevice_common_data")))
#define _DRIVER_I2S_I2SDEVICE_SHELL_DATA_  __attribute__((section("driver_i2s_i2sdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif

typedef enum _I2S_DEVICE_STATUS
{
    I2S_DEVICE_START,
    I2S_DEVICE_STOP
}I2S_DEVICE_STATUS_;

typedef enum _I2S_DEVICE_CONTROL_CMD
{
    I2S_DEVICE_INIT_CMD,
    I2S_DEVICE_RST_CMD,
    I2S_DEVICE_SET_FS,
    I2S_DEVICE_SET_DW,
    I2S_DEVICE_SET_RX_DW,
    I2S_DEVICE_NUM

}I2S_DEVICE_CONTROL_CMD;

typedef struct _I2S_DEVICE_CONFIG_REQ_ARG
{
    I2S_mode_t i2smode;
    eI2sCs_t i2sCS;
    I2sFS_en_t I2S_FS;
    eI2sFormat_t BUS_FORMAT;
    eI2sDATA_WIDTH_t Data_width ;
    I2S_BUS_MODE_t I2S_Bus_mode;

    eI2sFormat_t RX_BUS_FORMAT;
    eI2sDATA_WIDTH_t Rx_Data_width ;
    I2S_BUS_MODE_t Rx_I2S_Bus_mode;

}I2S_DEVICE_CONFIG_REQ_ARG;

typedef  struct _I2S_DEV_ARG
{
    HDC hDma;
    eI2S_SELECT Channel;
    I2S_DEVICE_CONFIG_REQ_ARG req_arg;

}I2S_DEV_ARG;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
// Interrupt status bit

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
extern void I2s0IntIsr(void);
extern void I2s1IntIsr(void);
extern HDC I2SDev_Create(uint32 DevID, void *arg);
extern rk_err_t I2SDev_Shell(HDC dev,  uint8 * pstr);
extern rk_err_t I2SDev_Delete(uint32 DevID, void * arg);
#endif


