/*
********************************************************************************************
*
*       Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\AD\AdcDevice.h
* Owner: zhuzhe
* Date: 2014.5.26
* Time: 10:43:06
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    zhuzhe     2014.5.26     10:43:06   1.0
********************************************************************************************
*/

#ifndef __DRIVER_AD_ADCDEVICE_H__
#define __DRIVER_AD_ADCDEVICE_H__


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

#define _DRIVER_AD_ADCDEVICE_COMMON_  __attribute__((section("driver_ad_adcdevice_common")))
//#define _DRIVER_AD_ADCDEVICE_COMMON_ __attribute__((section("driver_ad_adcdevice_write")))
#define _DRIVER_AD_ADCDEVICE_INIT_  __attribute__((section("driver_ad_adcdevice_init")))
#define _DRIVER_AD_ADCDEVICE_SHELL_  __attribute__((section("driver_ad_adcdevice_shell")))

typedef rk_err_t (*pADCDevRx_indicate)(HDC dev);

typedef struct _ADC_DEV_ARG
{
    HDC hTimmer;
    uint32 channel;
    uint32 size;
}ADC_DEV_ARG;

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
extern rk_err_t ADCDev_GetAdcBufData(HDC dev, uint16* buf, uint16 size, uint16 channel);
extern rk_err_t ADCDev_GetMainClk(HDC dev);
extern rk_err_t ADCDev_ClearMainClk(HDC dev);
extern rk_err_t ADCDev_SetMainClk(HDC dev,uint16 clk);
extern rk_err_t ADCDev_Delete(uint32 DevID, void * arg);
extern rk_err_t ADCDev_Read(HDC dev,uint16 channel,uint16 size, uint16 clk);
extern HDC ADCDev_Create(uint32 DevID, void * arg);
extern void ADCDevISR(void);
extern rk_err_t ADCDev_Shell(HDC dev, uint8 * pstr);

#endif

