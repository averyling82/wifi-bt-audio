
/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Web\airplay\airplay.c
* Owner: ctf
* Date: 2015.9.16
* Time:
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*     ctf      2015.9.16                    1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "BSP.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "ModuleInfoTab.h"
#include "FwAnalysis.h"
#include "PowerManager.h"
#include "ShellTask.h"
#include "ShellDevCmd.h"
#include "ShellSysCmd.h"
#include "ShellTaskCmd.h"
#include "rtsp.h"
#include "airplay.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
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
rk_err_t airplay_shell_delete(HDC dev, uint8 * pstr);
rk_err_t airplay_shell_start(HDC dev, uint8 * pstr);
extern void airplay_main(void);
extern void airplay_end(void);
extern int airplay_init_state();


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




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AIRPLAYTask_Resume
** Input:void
** Return: rk_err_t
** Owner:ctf
** Date: 2015.9.16
** Time:
*******************************************************************************/
COMMON API rk_err_t AIRPLAYTask_Resume(void)
{

}
/*******************************************************************************
** Name: AIRPLAYTask_Suspend
** Input:void
** Return: rk_err_t
** Owner:ctf
** Date: 2015.9.16
** Time:
*******************************************************************************/
COMMON API rk_err_t AIRPLAYTask_Suspend(void)
{

}
/*******************************************************************************
** Name: AIRPLAYTask_Enter
** Input:void
** Return: void
** Owner:ctf
** Date: 2015.9.16
** Time:
*******************************************************************************/
COMMON API void AIRPLAYTask_Enter(void)
{
     airplay_main();
     while(1)
    {
      rkos_sleep(1000);
    }
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AIRPLAYTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:ctf
** Date: 2015.9.16
** Time:
*******************************************************************************/
_AIRPLAY_TASK_INIT_
INIT API rk_err_t AIRPLAYTask_DeInit(void *pvParameters)
{
    //FW_RemoveSegment(SEGMENT_ID_AIRPLAY);
    FW_RemoveSegment(SEGMENT_ID_SSL);
    FREQ_ExitModule(FREQ_MAX);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: AIRPLAYTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:ctf
** Date: 2015.9.16
** Time:
*******************************************************************************/
_AIRPLAY_TASK_INIT_
INIT API rk_err_t AIRPLAYTask_Init(void *pvParameters, void *arg)
{
    //FW_LoadSegment(SEGMENT_ID_AIRPLAY, SEGMENT_OVERLAY_CODE);
    FW_LoadSegment(SEGMENT_ID_SSL, SEGMENT_OVERLAY_CODE);
    FREQ_EnterModule(FREQ_MAX);
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_AIRPLAY_SHELL_DATA_
static SHELL_CMD ShellAirPlayName[] =
{
    "start",airplay_shell_start,"start xxx player","xxx.start",
    "delete",airplay_shell_delete,"delete xxx player","xxx.delete",
    "\b",NULL,"NULL","NULL",
};




_AIRPLAY_SHELL_
rk_err_t airplay_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellAirPlayName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellAirPlayName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellAirPlayName[i].CmdDes, pItem);
    if(ShellAirPlayName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellAirPlayName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: airplay_shell_delete
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.11.8
** Time: 15:00:49
*******************************************************************************/
_AIRPLAY_SHELL_
SHELL FUN rk_err_t airplay_shell_delete(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    airplay_end();
    RKTaskDelete(TASK_ID_AIRPLAY,0,SYNC_MODE);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: airplay_shell_start
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.11.8
** Time: 14:58:52
*******************************************************************************/
_AIRPLAY_SHELL_
SHELL FUN rk_err_t airplay_shell_start(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RKTaskCreate(TASK_ID_AIRPLAY, 0, NULL, SYNC_MODE);
    return RK_SUCCESS;
}



#endif


