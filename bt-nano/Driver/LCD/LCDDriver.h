/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Gui\LCDDriver.h
* Owner: zhuzhe
* Date: 2014.4.28
* Time: 16:01:13
* Desc:
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* zhuzhe     2014.4.28     16:01:13   1.0
********************************************************************************************
*/

#ifndef __GUI_LCDDRIVER_H__
#define __GUI_LCDDRIVER_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _DRIVER_LCD_LCDDEVICE_SHELL_      __attribute__((section("driver_lcd_lcddevice_shell")))
#define _DRIVER_LCD_LCDDEVICE_INIT_      __attribute__((section("driver_lcd_lcddevice_init")))
#define _DRIVER_LCD_LCDDEVICE_COMMON_      __attribute__((section("driver_lcd_lcddevice_common")))

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum LCD_DEV
{
    LCD_DEV0 = 0,   //0
    LCD_DEV_MAX
}eLCD_DEV;

typedef enum
{
    ROTATE_NOMAL = 0,
    ROTATE_FLIP_LR ,
    ROTATE_FLIP_UD ,
    ROTATE_ROTATE
}RK_LCD_ROTATE;

typedef struct _LCD_INFO_CLASS
{
    char * name;
    uint16 width;
    uint16 height;
    uint32 pallet;   // BCD : transparency¡¢R¡¢G¡¢B  Such RGB565: 0x0565
    uint16 BW;       // 0 : Color     1: BW
    uint32 LittleEnd;
}LCD_INFO_CLASS;

//every lcd driver must be packed to these struct
/*Because the number of bytes occupied pixels undecided, so the incoming pointer of type void * */
typedef struct _RK_LCD_DRIVER
{
    LCD_INFO_CLASS LcdArg;

    void (*LcdInit)(HDC dev);

    rk_err_t (*LcdWakeUp)(HDC dev);
    rk_err_t (*LcdStandby)(HDC dev);

    rk_err_t (*LcdClear)(HDC dev, void * color);
    rk_size_t (*Lcd_Write)(HDC dev,void * color, uint32 size);
    rk_err_t (*LcdSetWindow)(HDC dev, int x0, int y0, int x1, int y1);

    /* point the next driver*/
    void *next;
}RK_LCD_DRIVER;

typedef  struct _LCD_DEV_ARG
{
    HDC hPwm;
    HDC hBus;
    RK_LCD_DRIVER *pLCDriver;
}LCD_DEV_ARG;

typedef enum
{
    DIRECTION_UP = (uint16)0,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_MAX
} BUFFER_SHIFT_DIRECTION;

#define PALLET_RGB565    0x0565
//#define PALLET_RGB555    0x1555
//#define PALLET_RGB888    0x0888
//#define PALLET_RGB8888   0x8888
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
extern rk_err_t Lcd_BL_Off(HDC dev);
extern rk_err_t Lcd_BL_On(HDC dev);
extern rk_err_t LcdSetWindow(HDC dev, uint32 x0, uint32 y0,uint32 x1, uint32 y1);
extern LCD_INFO_CLASS * LcdGetParameter(HDC dev);
extern rk_err_t LcdClear(HDC dev, void * color);
extern rk_err_t Lcd_Write(HDC dev, void *data, uint32 size);
extern HDC LcdDev_Create(uint32 DevID, void * arg);
extern rk_err_t LcdDev_Delete(uint32 DevID, void * arg);
#ifdef _USE_SHELL_
extern rk_err_t LcdShellPcb(HDC dev,  uint8 * pstr);
#ifdef __DRIVER_DISPLAY_DISPLAYDEVICE_C__
extern rk_err_t LcdShellOpen(HDC dev,  uint8 * pstr);
#endif
extern rk_err_t LcdShellTest(HDC dev,  uint8 * pstr);
extern rk_err_t LcdShell(HDC dev, uint8 * pstr);
#endif

#endif


