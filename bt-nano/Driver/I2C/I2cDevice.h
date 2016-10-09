
/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ver\I2C\I2cDevice.h
* Owner: chad.Ma
* Date: 2014.11.10
* Time: 10:37:54
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    chad.Ma     2014.11.10     10:37:54   1.0
********************************************************************************************
*/

#ifndef __DRIVER_I2C_I2CDEVICE_H__
#define __DRIVER_I2C_I2CDEVICE_H__

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
#define _DRIVER_I2C_I2CDEVICE_COMMON_  __attribute__((section("driver_i2c_i2cdevice_common")))
#define _DRIVER_I2C_I2CDEVICE_INIT_  __attribute__((section("driver_i2c_i2cdevice_init")))
#define _DRIVER_I2C_I2CDEVICE_SHELL_  __attribute__((section("driver_i2c_i2cdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_I2C_I2CDEVICE_DATA_      _DRIVER_I2C_I2CDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_I2C_I2CDEVICE_DATA_      __attribute__((section("driver_i2c_i2cdevice_data")))
#else
#error Unknown compiling tools.
#endif


typedef struct _I2C_CMD_ARG
{
    UINT16 SlaveAddress;
    UINT16 RegAddr;
    UINT16 speed ;
    uint8 addr_reg_fmt;
    uint8 RWmode;
}I2C_CMD_ARG;

typedef struct _I2C_DEVICE_ARG
{
    UINT8 Channel;
    UINT16 speed;

}I2C_DEVICE_ARG;


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
extern void I2CDevIntIsr0(void);
extern void I2CDevIntIsr1(void);
extern void I2CDevIntIsr2(void);
extern rk_err_t I2CDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t I2CDev_Delete(uint32 DevID, void * arg);
extern HDC I2CDev_Create(uint32 DevID, void * arg);
extern rk_err_t I2cDev_SendData(HDC dev, uint8* RegCmd, uint32 size, I2C_CMD_ARG *Tx_arg);
extern rk_size_t I2cDev_ReadData(HDC dev, uint8* DataBuf, uint32 size, I2C_CMD_ARG *Rx_arg);

#endif

