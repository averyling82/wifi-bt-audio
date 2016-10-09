/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\GUI\ST7735SDriver.c
* Owner: Benjo.lei
* Date: 2015.10.28
* Time: 13:51:36
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei     2015.10.28    13:51:36      1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __GUI_ST7735SDRIVER_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "ST7735SDriver.h"
#include "vop.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define LCD_WIDTH  128
#define LCD_HEIGHT 160


typedef struct _LCD_INIT_CONFIG
{
    uint8 dat;
    uint32 mod;
}LCD_INIT_CONFIG;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name:ST7735S_Standby
** Input:
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.28
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
rk_err_t ST7735S_Standby(HDC dev)
{
    uint8 StandBye[2] = {0x28, 0x10};
    VopDev_Write(dev,StandBye, 2, 0);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name:ST7735S_WakeUp
** Input:
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.28
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
rk_err_t ST7735S_WakeUp(HDC dev)
{
    ST7735S_Init(dev);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ST7735S_Clear
** Input:HDC dev, UINT16 color
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.28
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
void ST7735S_Clear(HDC dev, void * color)
{
    int i, j;
    uint16 * pbuf= NULL, * pTemp;

    ST7735S_SetWindow(dev, 0, 0, LCD_WIDTH -1, LCD_HEIGHT -1);

    pbuf= rkos_memory_malloc(LCD_WIDTH* LCD_HEIGHT* 2);
    if( pbuf!= NULL )
    {
        pTemp= pbuf;
        for(i= 0; i< LCD_WIDTH; i++)
        {
            for(j= 0; j< LCD_HEIGHT; j++)
            {
                *pTemp++= (uint16)color;
            }
        }

        VopDev_Write(dev, (uint8 *)pbuf, LCD_WIDTH* LCD_HEIGHT* 2, 1);
        rkos_memory_free(pbuf);
    }
    else
    {
        pbuf= rkos_memory_malloc(LCD_WIDTH* 2);
        if(pbuf!= NULL)
        {
            pTemp= pbuf;
            for(i= 0; i< LCD_WIDTH; i++)
            {
                *pTemp++= (uint16)color;
            }

            for(j= 0; j< LCD_HEIGHT; j++)
            {
                if( pbuf!= NULL )
                {
                    VopDev_Write(dev, (uint8 *)pbuf, LCD_WIDTH* 2, 1);
                }
            }
            rkos_memory_free(pbuf);
        }
    }
}

/*******************************************************************************
** Name: ST7735S_SetWindow
** Input:HDC dev,UINT16 x0,UINT16 y0,UINT16 x1,INT16 y1
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.28
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
rk_err_t ST7735S_SetWindow(HDC dev, int x0, int y0, int x1, int y1)
{
    uint32 i;
    LCD_INIT_CONFIG SetWindowTab[] =
    {
        0x2a, 0, 0, 1, (uint8)x0, 1, 0, 1, (uint8)x1, 1,
        0x2b, 0, 0, 1, (uint8)y0, 1, 0, 1, (uint8)y1, 1,
        0x2c, 0,
    };

    for(i =0; i< sizeof(SetWindowTab)/ sizeof(SetWindowTab[0]); i++)
    {
        VopDev_Write(dev,&SetWindowTab[i].dat, 1, SetWindowTab[i].mod);
    }

    return RK_SUCCESS;
}

_DRIVER_LCD_LCDDEVICE_INIT_
LCD_INIT_CONFIG InitTab[] =
{
    0x11, 0,

    0xb1, 0, 0x05, 1, 0x3C, 1, 0x3C, 1,
    0xb2, 0, 0x05, 1, 0x3C, 1, 0x3C, 1,
    0xb3, 0, 0x05, 1, 0x3C, 1, 0x3C, 1,    0x05, 1, 0x3C, 1, 0x3C, 1,

    0xB4, 0,
    0x03, 1,

    0xc0, 0, 0x028, 1, 0x08, 1, 0x04, 1,

    0xc1, 0, 0xc0, 1,
    0xc2, 0, 0x0D, 1, 0x00, 1,
    0xc3, 0, 0x8D, 1, 0x2A, 1,
    0xc4, 0, 0x8D, 1, 0xEE, 1,
    0xc5, 0, 0x0A, 1, 0x36, 0,
    0xc8, 1,
    0xe0, 0, 0x04, 1, 0x22, 1, 0x07, 1, 0x0A, 1, 0x2e, 1, 0x30, 1, 0x25, 1, 0x2A, 1, 0x28, 1, 0x26, 1, 0x2E, 1, 0x3A, 1, 0x00, 1, 0x01, 1, 0x03, 1, 0x13, 1,
    0xe1, 0, 0x04, 1, 0x16, 1, 0x06, 1, 0x0D, 1, 0x2D, 1, 0x26, 1, 0x23, 1, 0x27, 1, 0x27, 1, 0x25, 1, 0x2D, 1, 0x3B, 1, 0x00, 1, 0x01, 1, 0x04, 1, 0x13, 1,
    0x3a, 0, 0x05, 1, 0x29, 0,
};

/*******************************************************************************
** Name: ST7735S_Init
** Input:HDC dev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.28
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
rk_err_t ST7735S_Init(HDC dev)
{
    uint32 i;

    for(i =0; i< sizeof(InitTab)/ sizeof(InitTab[0]); i++)
    {
        VopDev_Write(dev,&InitTab[i].dat, 1, InitTab[i].mod);
    }

    VopDevSetWidthHeight(dev, LCD_WIDTH, LCD_HEIGHT);

    return RK_SUCCESS;
}


_DRIVER_LCD_LCDDEVICE_COMMON_
RK_LCD_DRIVER ST7735SDriver =
{
    .LcdArg =
    {
        .name   = "ST7735S",
        .pallet = PALLET_RGB565,
        .BW     = 0,
        .width  = LCD_WIDTH,
        .height = LCD_HEIGHT,
    },

    .LcdInit      = ST7735S_Init,

    .LcdWakeUp    = ST7735S_WakeUp,
    .LcdStandby   = ST7735S_Standby,

    .LcdClear     = ST7735S_Clear,
    .LcdSetWindow = ST7735S_SetWindow,

    .next = NULL,
};
/*
********************************************************************************
*
*                         End of Lcd.c
*
********************************************************************************
*/
#endif
