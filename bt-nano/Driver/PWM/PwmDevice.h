/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Driver\PWM\PwmDevice.h
* Owner: chad.Ma
* Date: 2014.11.10
* Time: 17:26:56
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    chad.Ma     2014.11.10     17:26:56   1.0
********************************************************************************************
*/

#ifndef __DRIVER_PWM_PWMDEVICE_H__
#define __DRIVER_PWM_PWMDEVICE_H__

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
#define _DRIVER_PWM_PWMDEVICE_COMMON_  __attribute__((section("driver_pwm_pwmdevice_common")))
#define _DRIVER_PWM_PWMDEVICE_INIT_  __attribute__((section("driver_pwm_pwmdevice_init")))
#define _DRIVER_PWM_PWMDEVICE_SHELL_  __attribute__((section("driver_pwm_pwmdevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_PWM_PWMDEVICE_SHELL_DATA_      _DRIVER_PWM_PWMDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_PWM_PWMDEVICE_SHELL_DATA_      __attribute__((section("driver_pwm_pwmdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif


typedef  struct _PWM_DEV_ARG
{
    uint32 channel;
}PWM_DEV_ARG;

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
extern rk_err_t PwmDev_SetRate(HDC dev, UINT32 rate);
extern rk_err_t PwmDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t PwmDev_Write(HDC dev);
extern rk_err_t PwmDev_Read(HDC dev);
extern rk_err_t PwmDev_Delete(uint32 DevID, void * arg);
extern HDC PwmDev_Create(uint32 DevID, void * arg);

#endif



