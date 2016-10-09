/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Display\DisplayDevice.h
* Owner: aaron.sun
* Date: 2015.5.29
* Time: 10:20:33
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.5.29     10:20:33   1.0
********************************************************************************************
*/


#ifndef __DRIVER_DISPLAY_DISPLAYDEVICE_H__
#define __DRIVER_DISPLAY_DISPLAYDEVICE_H__

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
typedef  struct _DISPLAY_DEV_ARG
{
    HDC hLcd;
}DISPLAY_DEV_ARG;

#define BUFFER_MAX_NUM 1
#define DISPLAY_DOUBLE_LAYER
#define DISPLAY_DEV1_HEIGHT    20
//#define DISPLAY_DEV_EDIAN

#define _DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_  __attribute__((section("driver_display_displaydevice_common")))
#define _DRIVER_DISPLAY_DISPLAYDEVICE_INIT_  __attribute__((section("driver_display_displaydevice_init")))
#define _DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_  __attribute__((section("driver_display_displaydevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_DISPLAY_DISPLAYDEVICE_DATA_      _DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_DISPLAY_DISPLAYDEVICE_DATA_      __attribute__((section("driver_display_displaydevice_data")))
#else
#error Unknown compiling tools.
#endif
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
extern rk_err_t DisplayDev_ScreenUpdate(void);
extern rk_err_t DisplayDev_RotateCanvas(HDC dev ,RK_LCD_ROTATE rotate);
extern rk_err_t DisplayDev_SetWindow(HDC dev ,int x, int y, int xSize, int ySize);
extern rk_err_t DisplayDev_SendData(HDC dev, uint8 R, uint8 G, uint8 B, uint8 transparency);
extern rk_err_t DisplayDev_Write(HDC dev, void *color, uint16 size);
extern rk_err_t DisplayDev_ClrRect(HDC dev , uint8 R, uint8 G, uint8 B, uint8 transparency);
extern rk_err_t DisplayDev_Delete(uint32 DevID, void * arg);
extern HDC DisplayDev_Create(uint32 DevID, void * arg);
extern rk_err_t DisplayDev_SetOffset(HDC dev);
extern rk_err_t DisplayDev_Blur(HDC dev);

#ifdef _USE_SHELL_
extern rk_err_t DisplayDev_Shell(HDC dev, uint8 * pstr);
#endif

#endif

