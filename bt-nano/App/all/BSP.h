/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Bsp\EVK_V2.0\BSP.h
* Owner: aaron.sun
* Date: 2015.5.14
* Time: 15:25:07
* Version: 1.0
* Desc: EVK2.0 BSP init
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.5.14     15:25:07   1.0
********************************************************************************************
*/


#ifndef __BSP_EVK_V2_0_BSP_H__
#define __BSP_EVK_V2_0_BSP_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*---------------------------------------------------------------------------------------------------------------------
*/
#ifndef NOT_INCLUDE_OTHER
#include "global.h"
#include "Delay.h"
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _BSP_EVK_V20_BSP_COMMON_  __attribute__((section("bsp_evk_v20_bsp_common")))
#define _BSP_EVK_V20_BSP_INIT_  __attribute__((section("bsp_evk_v20_bsp_init")))
#define _BSP_EVK_V20_BSP_SHELL_  __attribute__((section("bsp_evk_v20_bsp_shell")))

#define min(x, y) (x < y ? x : y)
#define max(x, y) (x < y ? y : x)

//Emmc0 Config
#define EMMC_DEV0_BUS_WIDTH  BUS_WIDTH_8_BIT
#define EMMC_DEV0_SDC_ID     0
#define EMMC_DEV0_DMA_ID     0

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void * UartHDC;
extern void * FileSysHDC;
extern void * hLcd;
extern void * hKey;
extern volatile uint32 SysTickCounter;
extern uint32 SysTickCounterSave;
extern uint32 SysTickCheck;
extern uint32 MspSave;
extern void (*pIdleFun)(void);
extern void (*pIdleFunSave)(void);
extern uint8 gLun[USBMSC_LUN];



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void PnPSever(void);
extern void SystemDeInit(void);
extern void SystemInit(void);
extern rk_err_t DeleteDeviceListFm(uint32 * list, void *arg);
extern rk_err_t CreateDeviceListFm(uint32 * list, void *arg);
extern void I2cDevHwDeInit(uint32 DevID, uint32 Channel);
extern void I2sDevHwDeInit(uint32 DevID, uint32 Channel);
extern void DmaDevHwDeInit(uint32 DevID, uint32 Channel);
extern void EmmcDevDeHwInit(uint32 DevID, uint32 Channel);
extern rk_err_t DeleteDeviceListEmmcFs3(uint32 * list);
extern rk_err_t DeleteDeviceListEmmcFs2(uint32 * list);
extern rk_err_t DeleteDeviceListSpiFs3(uint32 * list);
extern rk_err_t DeleteDeviceListSpiFs2(uint32 * list);
extern rk_err_t CreateDeviceListSpiFs2(uint32 * list);
extern rk_err_t DeleteDeviceListEmmcBoot(uint32 * list);
extern rk_err_t DeleteDeviceListEmmcDataBase(uint32 * list);
extern rk_err_t DeleteDeviceListEmmcFs1(uint32 * list);
extern rk_err_t DeleteDeviceListSpiBoot(uint32 * list);
extern rk_err_t DeleteDeviceListSpiDataBase(uint32 * list);
extern rk_err_t DeleteDeviceListKey(uint32 * list);
extern rk_err_t DeleteDeviceListAudioPlay(uint32 * list);
extern rk_err_t DeleteDeviceListUsbHostMsc(uint32 * list);
extern rk_err_t CreateDeviceListUsbHostMsc(uint32 * list);
extern rk_err_t DeleteDeviceListDir(uint32 * list);
extern rk_err_t CreateDeviceListDir(uint32 * list);
extern void PwmDevHwDeInit(uint32 DevID, uint32 channel);
extern rk_err_t DeleteDeviceListDisplay(uint32 * list);
extern void GetBtAudioDeviceName(uint8 * name);
extern void GetAirplayDeviceName(uint8  * name);
extern void GetDlnaDeviceName(uint8 * name);
extern rk_err_t DeviceListLoadFs(uint32 devicelist,uint32 * list);
extern rk_err_t DeviceListRemoveFs(uint32 devicelist, uint32 * list);
extern rk_err_t DeleteDeviceListSpiFs1(uint32 * list);
extern void DmaDevHwInit(uint32 DevID, uint32 Channel);
extern void SdioDevHwDeInit(uint32 DevID, uint32 Channel);
extern rk_err_t DeleteDeviceListFIFO(uint32 * list, void *arg);
extern rk_err_t CreateDeviceListFIFO(uint32 * list, void *arg);
extern rk_err_t DeleteDeviceListUsbDeviceMsc(uint32 * list);
extern rk_err_t CreateDeviceListSdio(uint32 * list);
extern rk_err_t CreateDeviceListUsbDeviceMsc(uint32 * list);
extern rk_err_t CreateDeviceListSD(uint32 * list);
extern rk_err_t CreateDeviceListKey(uint32 * list);
extern rk_err_t CreateDeviceListDisplay(uint32 * list);
extern rk_err_t CreateDeviceListAudioPlay(uint32 * list);
extern rk_err_t CreateDeviceListSpiFs3(uint32 * list);
extern rk_err_t CreateDeviceListSpiFs1(uint32 * list);
extern rk_err_t CreateDeviceListSpiDataBase(uint32 * list);
extern rk_err_t CreateDeviceListSpiBoot(uint32 * list);
extern rk_err_t CreateDeviceListEmmcFs3(uint32 * list);
extern rk_err_t CreateDeviceListEmmcFs2(uint32 * list);
extern rk_err_t CreateDeviceListEmmcFs1(uint32 * list);
extern rk_err_t CreateDeviceListEmmcDataBase(uint32 * list);
extern rk_err_t CreateDeviceListEmmcBoot(uint32 * list);
extern uint32 Num2String(uint8 * pstr, uint32 num);
extern void DisplayDevHwInit(uint32 DevID, uint32 Channel);
extern void TimerDevHwInit(uint32 DevID, uint32 Channel);
extern void BBReqDebug(uint8 * buf);
extern void BBDebug(void);
extern void SdDevHwInit(uint32 DevID, uint32 Channel);
extern void EmmcDevHwInit(uint32 DevID, uint32 Channel);
extern void UsbOtgDevHwInit(uint32 DevID, uint32 Channel);
extern void I2cDevHwInit(uint32 DevID, uint32 Channel);
extern rk_err_t UartDevHwInit(uint32 DevID, uint32 Channel, uint32 Freq);
extern void SdioDevHwInit(uint32 DevID, uint32 Channel);
extern void SdMmcDevHwInit(uint32 DevID, uint32 Channel);
extern void SpiDevHwInit(uint32 DevID, uint32 Channel);
extern void AdcDevHwInit (uint32 DevID, uint32 Channel);
extern void PwmDevHwInit (uint32 DevID, uint32 channel);
extern void WdtDevHwInit (uint32 DevID, uint32 channel);
extern void vApplicationIdleHook(void);
extern void bsp_init(void);
extern uint32 Unicode2Ascii(uint8 * pbAscii, uint16 * pwUnicode, uint32 len);
extern uint32 Ascii2Unicode(uint8 * pbAscii, uint16 * pwUnicode, uint32 len);
extern uint32 String2Num(uint8 * pstr);
extern void MemCpy(uint8 * trg, uint8 * src, uint32 len);
extern int32 StrCmpA(uint8 * pstr1, uint8 * pstr2, uint32 len);
extern int32 StrCmpW(uint16 * pstr1, uint16 * pstr2, uint32 len);
extern uint32 StrLenA(uint8 * pstr);
extern uint32 StrLenW(uint16 * pstr);
extern uint32 rk_count_clk_end(void);
extern void rk_count_clk_start(void);
extern void rk_print_string(const uint8 * pstr);
extern void rk_printf_no_time(const char * fmt,...);
extern void debug_hex(char *buf, int BufSize, int radix);
extern void rk_printf(const char * fmt,...);
#endif
