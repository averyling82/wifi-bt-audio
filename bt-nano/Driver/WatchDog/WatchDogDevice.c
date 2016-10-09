/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\WatchDog\WatchDogDevice.c
* Owner: wrm
* Date: 2015.5.19
* Time: 10:15:57
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm     2015.5.19     10:15:57   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_WATCHDOG_WATCHDOGDEVICE_C__

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
#include "wdt.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define WDT_DEV_NUM 2

typedef  struct _WDT_DEVICE_CLASS
{
    DEVICE_CLASS stWDTDevice;
    pSemaphore osWDTOperReqSem;
    uint32 timeout;

}WDT_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static WDT_DEVICE_CLASS * gpstWDTDevISR[WDT_DEV_NUM] = {(WDT_DEVICE_CLASS *)NULL,(WDT_DEVICE_CLASS *)NULL};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t WTDDevShellHelp(HDC dev,  uint8 * pstr);
rk_err_t WDTDevShellTest(HDC dev, uint8 * pstr);
rk_err_t WDTDevShellDel(HDC dev, uint8 * pstr);
rk_err_t WDTDevShellMc(HDC dev, uint8 * pstr);
rk_err_t WDTDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t WDTDevDeInit(WDT_DEVICE_CLASS * pstWDTDev);
rk_err_t WDTDevInit(WDT_DEVICE_CLASS * pstWDTDev);
rk_err_t WDTDevResume(HDC dev);
rk_err_t WDTDevSuspend(HDC dev);
void WDTDevIntIsr(uint32 DevID);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON API rk_err_t WDTDev_Write(HDC dev)
{
    //WDTDev Write...
    WDT_DEVICE_CLASS * pstWDTDev =  (WDT_DEVICE_CLASS *)dev;
    if(pstWDTDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: WDTDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON API rk_err_t WDTDev_Read(HDC dev)
{
    //WDTDev Read...
    WDT_DEVICE_CLASS * pstWDTDev =  (WDT_DEVICE_CLASS *)dev;
    if(pstWDTDev == NULL)
    {
        return RK_ERROR;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDevIntIsr1
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON FUN void WDTDevIntIsr1(void)
{
    //Call total int service...
    WDTDevIntIsr(1);

}
/*******************************************************************************
** Name: WDTDevIntIsr0
** Input:void
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON FUN void WDTDevIntIsr0(void)
{
    //Call total int service...
    WDTDevIntIsr(0);
}
/*******************************************************************************
** Name: WDTDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON FUN void WDTDevIntIsr(uint32 DevID)
{
    uint32 WDTDevIntType;

    //Get WDTDev Int type...
    //WDTDevIntType = GetIntType();
    if (gpstWDTDevISR[DevID] != NULL)
    {
        printf("watch dog int...");
        Grf_NOC_Remap_Sel(NOC_REMAP_BOOT_ROM);
    }

}
/*******************************************************************************
** Name: WDTDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON FUN rk_err_t WDTDevResume(HDC dev)
{
    //WDTDev Resume...
    WDT_DEVICE_CLASS * pstWDTDev =  (WDT_DEVICE_CLASS *)dev;
    if(pstWDTDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: WDTDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_COMMON_
COMMON FUN rk_err_t WDTDevSuspend(HDC dev)
{
    //WDTDev Suspend...
    WDT_DEVICE_CLASS * pstWDTDev =  (WDT_DEVICE_CLASS *)dev;
    if(pstWDTDev == NULL)
    {
        return RK_ERROR;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_INIT_
INIT API rk_err_t WDTDev_Delete(uint32 DevID, void * arg)
{
    //Check WDTDev is not exist...
    if(gpstWDTDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    //WDTDev deinit...
    WDTDevDeInit(gpstWDTDevISR[DevID]);

    //Free WDTDev memory...
    rkos_semaphore_delete(gpstWDTDevISR[DevID]->osWDTOperReqSem);
    rkos_memory_free(gpstWDTDevISR[DevID]);

    //Delete WDTDev...
    gpstWDTDevISR[DevID] = NULL;

    //Delete WDTDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    //FW_RemoveSegment(SEGMENT_ID_WDT_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: WDTDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_INIT_
INIT API HDC WDTDev_Create(uint32 DevID, void * arg)
{
    WDT_DEV_ARG * pstWDTDevArg;
    DEVICE_CLASS* pstDev;
    WDT_DEVICE_CLASS * pstWDTDev;

    if(arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    // Create handler...
    pstWDTDevArg = (WDT_DEV_ARG *)arg;
    pstWDTDev =  rkos_memory_malloc(sizeof(WDT_DEVICE_CLASS));
    memset(pstWDTDev, 0, sizeof(WDT_DEVICE_CLASS));
    if (pstWDTDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstWDTDev->osWDTOperReqSem  = rkos_semaphore_create(1,1);
    if(pstWDTDev->osWDTOperReqSem == 0)
    {
        rkos_semaphore_delete(pstWDTDev->osWDTOperReqSem);
        rkos_memory_free(pstWDTDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstWDTDev;
    pstDev->suspend = WDTDevSuspend;
    pstDev->resume  = WDTDevResume;
    pstDev->Idle1EventTime = 10 * PM_TIME;
    pstDev->SuspendMode = ENABLE_MODE;

    //init arg...
    pstWDTDev->timeout = pstWDTDevArg->timeout;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_WDOG_DEV, SEGMENT_OVERLAY_ALL);
#endif
    //device init...
    gpstWDTDevISR[DevID] = NULL;
    WdtDevHwInit (DevID,0);
    if(WDTDevInit(pstWDTDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstWDTDev->osWDTOperReqSem);
        rkos_memory_free(pstWDTDev);
        return (HDC) RK_ERROR;
    }
    gpstWDTDevISR[DevID] = pstWDTDev;
    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDevDeInit
** Input:WDT_DEVICE_CLASS * pstWDTDev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_INIT_
INIT FUN rk_err_t WDTDevDeInit(WDT_DEVICE_CLASS * pstWDTDev)
{
    //If ip device first step unregister int
    //IntDisable(INT_ID17_UART);
    //IntPendingClear(INT_ID17_UART);
    //IntUnregister(INT_ID17_UART);

    //If ip device second step close clk
    //ScuClockGateCtr(CLOCK_GATE_UART,0);
    return RK_ERROR;

}
/*******************************************************************************
** Name: WDTDevInit
** Input:WDT_DEVICE_CLASS * pstWDTDev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_INIT_
INIT FUN rk_err_t WDTDevInit(WDT_DEVICE_CLASS * pstWDTDev)
{
    WatchDogInit(RESP_MODE_INT_RESET, PCLK_CYCLES_128, pstWDTDev->timeout);
    WatchDogStart();
    return RK_SUCCESS;
}




#ifdef _WDT_DEV_SHELL_
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_DATA_
static SHELL_CMD ShellWDTName[] =
{
    "pcb",NULL,"NULL","NULL",
    "mc",NULL,"NULL","NULL",
    "del",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL API rk_err_t WDTDev_Shell(HDC dev, uint8 * pstr)
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

    ret = ShellCheckCmd(ShellWDTName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;
    switch (i)
    {
        case 0x00:
            ret = WDTDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = WDTDevShellMc(dev,pItem);
            break;

        case 0x02:
            ret = WDTDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = WDTDevShellTest(dev,pItem);
            break;

        case 0x04:
            ret = WTDDevShellHelp(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WDTDevShellHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:24:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL FUN rk_err_t WTDDevShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if( StrLenA((uint8 *) pstr) != 0)
        return RK_ERROR;

    rk_print_string("             WatchDog命令集提供了一系列的命令对Wtd进行操作\r\n");
    rk_print_string("WatchDog包含的子命令如下:           \r\n");
    rk_print_string("help      显示help信息           \r\n");
    rk_print_string("mc        打开WatchDog              \r\n");
    rk_print_string("del       删除WatchDog              \r\n");
    rk_print_string("test      测试WatchDog命令          \r\n");
    rk_print_string("pcb       显示pcb信息  \r\n");

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: WDTDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL FUN rk_err_t WDTDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hWDTDev;
    uint32 DevID;

    //Get WDTDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open WDTDev...
    hWDTDev = RKDev_Open(DEV_CLASS_WDT, 0, NOT_CARE);
    if((hWDTDev == NULL) || (hWDTDev == (HDC)RK_ERROR) || (hWDTDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("WDTDev open failure");
         return RK_SUCCESS;
    }

    //do test....
    rk_printf ("Test WatchDog Device\n");

    WatchDogStart();

    //close WDTDev...
    RKDev_Close(hWDTDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: WDTDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL FUN rk_err_t WDTDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    //Get WDTDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }
    if(RKDev_Delete(DEV_CLASS_WDT, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("WDTDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: WDTDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL FUN rk_err_t WDTDevShellMc(HDC dev, uint8 * pstr)
{
    WDT_DEV_ARG stWDTDevArg;
    rk_err_t ret;
    uint32 DevID;
    DevID = 0;

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Init WDTDev arg...
    //stWDTDevArg.usbmode = USBOTG_MODE_DEVICE;
    rk_printf ("Entry WatchDogDev Create\n");
    stWDTDevArg.timeout = PERIOD_RANGE_0X07FFFFFF;

    //Create WDTDev...
    ret = RKDev_Create(DEV_CLASS_WDT, DevID, &stWDTDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("WDTDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: WDTDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.19
** Time: 10:16:13
*******************************************************************************/
_DRIVER_WATCHDOG_WATCHDOGDEVICE_SHELL_
SHELL FUN rk_err_t WDTDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hWDTDev;
    uint32 DevID;

    //Get WDTDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if(gpstWDTDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("WDTDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("    &stWDTDevice---0x%08x", &gpstWDTDevISR[DevID]->stWDTDevice);
        //rk_printf_no_time("    usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("    usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("WDTDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}
#endif
#endif
