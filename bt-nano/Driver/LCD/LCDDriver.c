/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Gui\LCDDriver.c
* Owner: zhuzhe
* Date: 2014.4.28
* Time: 16:01:06
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* zhuzhe     2014.4.28     16:01:06   1.0
********************************************************************************************
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "BspConfig.h"
#ifdef __GUI_LCDDRIVER_C__
#include <typedef.h>
#include <DriverInclude.h>
#include <device.h>
#include <LCDDriver.h>
#include "UartDevice.h"
#include <DmaDevice.h>
#include <ST7735SDriver.h>
#include <FreeRTOS.h>
#include <RKOS.h>

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _LCD_DEVICE_CLASS
{
    DEVICE_CLASS LcdDevice;
    RK_LCD_DRIVER *pLCDriver;
    HDC hBus;
    HDC hPwm;
}LCD_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t LcdIdleCheck(HDC dev);
rk_err_t LcdCheckHandler(HDC dev);
rk_err_t LcdResume(HDC dev);
rk_err_t LcdSuspend(HDC dev, uint32 Level);
rk_err_t LcdShellBspReadSize(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspSetSize(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspReadTiming(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspSetTiming(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspReadStatus(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspReadVersion(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspReadMode(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspSetMode(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspReadData(HDC dev, uint8 * pstr);
rk_err_t LcdShellBspTestSpeed(HDC dev,  uint8 * pstr);
rk_err_t LcdShellBspSendData(HDC dev,  uint8 * pstr);
rk_err_t LcdShellBspSendCmd(HDC dev,  uint8 * pstr);
rk_err_t LcdShellBspSetSpeed(HDC dev,  uint8 * pstr);
rk_err_t LcdShellBspHelp(HDC dev,  uint8 * pstr);
rk_err_t LcdShellBsp(HDC dev,  uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static LCD_DEVICE_CLASS * gpstLcdDevISR[LCD_DEV_MAX];


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
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: Lcd_BL_Off
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.2
** Time: 16:19:08
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON API rk_err_t Lcd_BL_Off(HDC dev)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev;

    if(LcdCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    PwmDev_SetRate(pstLcdDev->hPwm, 100);
}

/*******************************************************************************
** Name: Lcd_BL_On
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.2
** Time: 16:18:25
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON API rk_err_t Lcd_BL_On(HDC dev)
{
    uint32 rate;

    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if(LcdIdleCheck(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    switch (gSysConfig.BLevel)
    {
        case BL_LEVEL_1:
            rate = BL_LEVEL_1_VALUE;break;
        case BL_LEVEL_2:
            rate = BL_LEVEL_2_VALUE;break;
        case BL_LEVEL_3:
            rate = BL_LEVEL_3_VALUE;break;
        case BL_LEVEL_4:
            rate = BL_LEVEL_4_VALUE;break;
        case BL_LEVEL_5:
            rate = BL_LEVEL_5_VALUE;break;
    }
    PwmDev_SetRate(pstLcdDev->hPwm,rate);
}

/*******************************************************************************
** Name: LcdSetWindow
** Input:HDC dev, uint32 x0, uint32 y0,uint32 x1, uint32 y1
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.08
** Time: 11:11:26
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON API rk_err_t LcdSetWindow(HDC dev, uint32 x0, uint32 y0,uint32 x1, uint32 y1)
{
    rk_err_t ret= RK_ERROR;
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if(LcdIdleCheck(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }
    ret= pstLcdDev->pLCDriver->LcdSetWindow(pstLcdDev->hBus,x0,y0,x1,y1);
    return ret;
}

/*******************************************************************************
** Name: LcdGetParameter
** Input:HDC dev
** Return: void
** Owner:benjo.lei
** Date: 2015.10.7
** Time:
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON API LCD_INFO_CLASS * LcdGetParameter(HDC dev)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;

    if(LcdIdleCheck(dev) != RK_SUCCESS)
    {
        return (LCD_INFO_CLASS *)RK_ERROR;
    }
    return &pstLcdDev->pLCDriver->LcdArg;
}


/*******************************************************************************
** Name: LcdWriteData
** Input:HDC dev, UINT32 data
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.7
** Time: 15:02:21
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON API rk_err_t Lcd_Write(HDC dev, void *data, uint32 size)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if(LcdIdleCheck(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    VopDev_Write(pstLcdDev->hBus, data, size, 1);

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LcdIdleCheck
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 19:34:56
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON FUN rk_err_t LcdIdleCheck(HDC dev)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev;

    if(LcdCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    pstLcdDev->LcdDevice.IdleTick = 0;

    if(pstLcdDev->LcdDevice.State != DEV_STATE_WORKING)
    {
        return DeviceTask_ResumeDevice(pstLcdDev->LcdDevice.DevClassID, pstLcdDev->LcdDevice.DevID, NULL, SYNC_MODE);
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LcdCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.29
** Time: 15:43:43
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_COMMON_
COMMON FUN rk_err_t LcdCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < LCD_DEV_MAX; i++)
    {
        if(gpstLcdDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LcdInit
** Input:HDC dev
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.09
** Time: 15:27:57
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
COMMON API rk_err_t LcdInit(HDC dev)
{
    rk_err_t ret= RK_ERROR;
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if (pstLcdDev == NULL)
    {
        return RK_ERROR;
    }

    if ( pstLcdDev->pLCDriver->LcdInit != NULL)
    {
        pstLcdDev->pLCDriver->LcdInit(pstLcdDev->hBus);
        pstLcdDev->pLCDriver->LcdClear(pstLcdDev->hBus, 0);
        rkos_delay(100);
        PwmDev_SetRate(pstLcdDev->hPwm,BL_LEVEL_1_VALUE);

    }
    return ret;
}

/*******************************************************************************
** Name: LcdInit
** Input:HDC dev
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.09
** Time: 15:27:57
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
COMMON API rk_err_t LcdDeInit(HDC dev)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdDev_Create
** Input:uint32 DevID, void * arg
** Return: void
** Owner:Benjo.lei
** Date: 2015.9.30
** Description:
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
INIT API HDC LcdDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS*     DevClass;
    LCD_DEVICE_CLASS* LcdDev;
    LCD_DEV_ARG * pstLcdDevArg = (LCD_DEV_ARG *)arg;

    if (arg == NULL || pstLcdDevArg->pLCDriver== NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    LcdDev = (LCD_DEVICE_CLASS *)rkos_memory_malloc(sizeof(LCD_DEVICE_CLASS));
    if (LcdDev == NULL)
    {
        return LcdDev;
    }

    //init handler...
    LcdDev->hBus = pstLcdDevArg->hBus;
    LcdDev->hPwm = pstLcdDevArg->hPwm;
    LcdDev->pLCDriver= pstLcdDevArg->pLCDriver;

    DevClass = &LcdDev->LcdDevice;
    DevClass->suspend = LcdSuspend;
    DevClass->resume  = LcdResume;
    DevClass->DevID   = DevID;
    DevClass->SuspendMode = FORCE_MODE;
    DevClass->Idle1EventTime = 10 * PM_TIME;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_LCD_DEV, SEGMENT_OVERLAY_CODE);
#endif
    gpstLcdDevISR[DevID] = LcdDev;

    LcdInit(DevClass);
    return (HDC)DevClass;
}

/*******************************************************************************
** Name: LCDDev_Delete
** Input:uint32 DevID, void * arg
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
INIT API rk_err_t LcdDev_Delete(uint32 DevID, void * arg)
{
    LCD_DEV_ARG * pstLcdDevArg = (LCD_DEV_ARG *)arg;

    if (gpstLcdDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    pstLcdDevArg->hBus = gpstLcdDevISR[DevID]->hBus;
    pstLcdDevArg->hPwm = gpstLcdDevISR[DevID]->hPwm;

    LcdDeInit(gpstLcdDevISR[DevID]);

    rkos_memory_free(gpstLcdDevISR[DevID]);
    gpstLcdDevISR[DevID] = NULL;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_LCD_DEV);
#endif
    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: LcdSuspend
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2015.9.30
** Description:
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
INIT FUN rk_err_t LcdSuspend(HDC dev, uint32 Level)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if(LcdCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    if (pstLcdDev->pLCDriver->LcdStandby != NULL )
    {
        if(pstLcdDev->pLCDriver->LcdStandby(pstLcdDev->hBus) == RK_SUCCESS)
        {
            if(Level == DEV_STATE_IDLE1)
            {
                pstLcdDev->LcdDevice.State = DEV_STATE_IDLE1;
            }
            else if(Level == DEV_SATE_IDLE2)
            {
                pstLcdDev->LcdDevice.State = DEV_SATE_IDLE2;
            }
            RKDev_Suspend(pstLcdDev->hBus);
            RKDev_Suspend(pstLcdDev->hPwm);
        }
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LcdSuspend
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2015.9.30
** Description:
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_INIT_
INIT FUN rk_err_t LcdResume(HDC dev)
{
    LCD_DEVICE_CLASS * pstLcdDev =  (LCD_DEVICE_CLASS *)dev ;
    if(LcdCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    if (pstLcdDev->pLCDriver->LcdWakeUp != NULL )
    {
        if(pstLcdDev->pLCDriver->LcdWakeUp(pstLcdDev->hBus) == RK_SUCCESS)
        {
            pstLcdDev->LcdDevice.State = DEV_STATE_WORKING;
            RKDev_Resume(pstLcdDev->hBus);
            RKDev_Resume(pstLcdDev->hPwm);
        }
    }

    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _LCD_SHELL_
static SHELL_CMD ShellLcdName[] =
{
    "pcb",NULL,"NULL","NULL",
    "mc",NULL,"NULL","NULL",
    "del",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "bsp",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",                         // the end
};

#ifdef SHELL_BSP
static SHELL_CMD ShellLcdBspName[] =
{
    "help",NULL,"NULL","NULL",
    "setspeed",NULL,"NULL","NULL",
    "sendcmd",NULL,"NULL","NULL",
    "senddata",NULL,"NULL","NULL",
    "readdata"NULL,"NULL","NULL",
    "setmode",NULL,"NULL","NULL",
    "readmode",NULL,"NULL","NULL",
    "readvsion",NULL,"NULL","NULL",
    "readstatus",NULL,"NULL","NULL",
    "settiming",NULL,"NULL","NULL",
    "readtiming",NULL,"NULL","NULL",
    "setlcdsize",NULL,"NULL","NULL",
    "getlcdsize",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef SHELL_BSP
/*******************************************************************************
** Name: LcdShellBspReadSize
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:32:13
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadSize(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readtimg : 获取LCD显示的宽高大小。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...


    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LcdShellBspSetSize
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:31:55
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSetSize(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.setsize : 设置LCD显示的宽高大小。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspReadTiming
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:31:37
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadTiming(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readtimg : 读取VOP 时序寄存器值。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: LcdShellBspSetTiming
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:31:21
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSetTiming(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.settiming : 配置VOP 时序寄存器。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspReadStatus
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:31:02
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadStatus(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readstatus : 获取VOP工作状态。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspReadVersion
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:30:20
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadVersion(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readversion : 获取VOP(video output process)版本信息。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspReadMode
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:30:04
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadMode(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readmode : 读取LCDC 工作模式。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspSetMode
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:29:38
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSetMode(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.setmode : 配置LCDC 工作模式。\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LcdShellBspReadData
** Input:HDC dev, uint8 * pstr
** Return: rk
** Owner:chad.Ma
** Date: 2014.11.11
** Time: 16:28:20
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspReadData(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.readdata : 读取数据.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspTestSpeed
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:39:21
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspTestSpeed(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.testspeed : 测试最大传输频率.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LcdShellBspSendData
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:38:42
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSendData(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.senddata : 想lcd发送数据.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellBspSendCmd
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:37:48
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSendCmd(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.sendcmd c: 发送命令，c为发送的命令参数.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: LcdShellBspSetSpeed
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:37:04
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspSetSpeed(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.bsp.setspeed : 设置传输频率.\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: LcdShellBspHelp
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_T
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:31:01
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBspHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if (StrLenA(pstr) != 0)
        return RK_ERROR;

    printf("lcd.bsp命令集提供了一系列的命令对lcd板级驱动接口进行测试\r\n");
    printf("测试命令如下:\r\n");
    printf("setspeed s       设置LCD的时钟频率 s，s为设置的时钟频率\r\n");
    printf("sendcmd c        发送命令 c ，c为命令\r\n");
    printf("senddata d       发送数据 d ，d为数据\r\n");
    printf("testspeed        设置LCD的时钟频率，在数据不出错的情况下，验证可达到多少\r\n");

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: LcdShellBsp
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.10
** Time: 15:23:55
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellBsp(HDC dev,  uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellLcdBspName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                          //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:  //bsp help
#ifdef SHELL_HELP
            ret = LcdShellBspHelp(dev,pItem);
#endif

            break;

        case 0x01:  //setspeed
            ret = LcdShellBspSetSpeed(dev,pItem);
            break;

        case 0x02:  //send command
            ret = LcdShellBspSendCmd(dev,pItem);
            break;

        case 0x03:  //send data
            ret = LcdShellBspSendData(dev,pItem);
            break;

        case 0x04:  //read data
            ret = LcdShellBspReadData(dev,pItem);
            break;

        case 0x05:  //set mode
            ret = LcdShellBspSetMode(dev,pItem);
            break;

        case 0x06:  //read mode
            ret = LcdShellBspReadMode(dev,pItem);
            break;

        case 0x07:  //read version
            ret = LcdShellBspReadVersion(dev,pItem);
            break;

        case 0x08:  //read status
            ret = LcdShellBspReadStatus(dev,pItem);
            break;

        case 0x09:  //set timing
            ret = LcdShellBspSetTiming(dev,pItem);
            break;

        case 0x0a:  //read timing
            ret = LcdShellBspReadTiming(dev,pItem);
            break;

        case 0x0b:  //set lcd size
            ret = LcdShellBspSetSize(dev,pItem);
            break;

        case 0x0c:  //read lcd size
            ret = LcdShellBspReadSize(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}
#endif

/*******************************************************************************
** Name: EmmcShellPcb
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
rk_err_t LcdShellPcb(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.pcb : lcd pcb info cmd .  \r\n");
            return RK_SUCCESS;
        }
    }
#endif

    // TODO:
    //add other code below:
    //...

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellMc
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.9
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
rk_err_t LcdShellMc(HDC dev,  uint8 * pstr)
{
    VOP_DEV_ARG stVopDevArg;
    LCD_DEV_ARG stLcdDevArg;
    uint32 ret;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.mc : lcd mc cmd   \r\n");
            return RK_SUCCESS;
        }
    }
#endif

    stLcdDevArg.hBus = RKDev_Open(DEV_CLASS_VOP, 0, NULL);
    if (stLcdDevArg.hBus == NULL)
    {
        rk_print_string("Open DEV_CLASS_VOP failure\n\n");
        return RK_ERROR;
    }

    ret = RKDev_Create(DEV_CLASS_LCD, 0, &stLcdDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("VopDev create failure");
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DisplayDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
SHELL FUN rk_err_t LcdShellDel(HDC dev, uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("lcd.del 命令用于删除 lcd device.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    if (RKDev_Delete(DEV_CLASS_LCD, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("LcdDev delete failure\n");
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
rk_err_t LcdShellTest(HDC dev,  uint8 * pstr)
{
    HDC hLCD;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("lcd.test : lcd test cmd   \r\n");
            return RK_SUCCESS;
        }
    }
#endif

    hLCD = RKDev_Open(DEV_CLASS_LCD, 0, NULL);
    if (hLCD == NULL)
    {
        printf("Open DEV_CLASS_LCD failure\n\n");
        return RK_ERROR;
    }

    //LcdClear(hLCD,(void *)0xF000);
    DelayMs(100);
//    LcdClear(hLCD,(void *)0x000F);
//    DelayMs(100);
//    LcdClear(hLCD,0x0000);
//    DelayMs(100);

    RKDev_Close(hLCD);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShellHelp
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
rk_err_t LcdShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if ( StrLenA( pstr) != 0)
        return RK_ERROR;

    printf("lcd命令集提供了一系列的命令对lcd进行操作\r\n");
    printf("lcd包含的子命令如下:           \r\n");
    printf("pcb       显示pcb信息         \r\n");
    printf("mc        打开lcd        \r\n");
    printf("del       删除lcd        \r\n");
    printf("test      测试lcd命令    \r\n");
    printf("help      显示lcd命令帮助信息  \r\n");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LcdShell
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LCD_LCDDEVICE_SHELL_
rk_err_t LcdShell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellLcdName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:  //pcb
            ret = LcdShellPcb(dev,pItem);
            break;

        case 0x01:  //mc
            ret = LcdShellMc(dev,pItem);
            break;

        case 0x02:  //del
            ret = LcdShellDel(dev,pItem);
            break;

        case 0x03:  //test
            ret = LcdShellTest(dev,pItem);
            break;

        case 0x04:  //help
            ret = LcdShellHelp(dev,pItem);
            break;

        case 0x05:  //bsp
#ifdef SHELL_BSP
            ret = LcdShellBsp(dev,pItem);
#endif
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}
#endif

#endif


