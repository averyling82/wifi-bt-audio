/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellCustomCmd.c
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 14:01:15
* Version: 1.0
* Desc: shell custom cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     14:01:15   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_SHELL_SHELLCUSTOMCMD_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "rkos.h"
#include "Bsp.h"
#include "ShellCustomCmd.h"



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
rk_err_t ShellVariableValue(HDC dev, uint8 * pstr);
rk_err_t ShellFunctionRun(HDC dev, uint8 * pstr);




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_SYSTEM_SHELL_SHELLCUSTOMCMD_COMMON_
static SHELL_CMD ShellCustomName[] =
{
    "fun",ShellFunctionRun,"call a function","custom.fun <function name>[ paramet]...",
    "var",ShellVariableValue,"get variable value","custom.var <variable name>",
    "\b",NULL,"NULL","NULL",
};



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
** Name: ShellCustomParsing
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 14:06:41
*******************************************************************************/
_SYSTEM_SHELL_SHELLCUSTOMCMD_COMMON_
COMMON API rk_err_t ShellCustomParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellCustomName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }


    ret = ShellCheckCmd(ShellCustomName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellCustomName[i].CmdDes, pItem);
    if(ShellCustomName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellCustomName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;


}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellVariableValue
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 14:03:26
*******************************************************************************/
_SYSTEM_SHELL_SHELLCUSTOMCMD_COMMON_
COMMON FUN rk_err_t ShellVariableValue(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellFunctionRun
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 14:02:26
*******************************************************************************/
_SYSTEM_SHELL_SHELLCUSTOMCMD_COMMON_
COMMON FUN rk_err_t ShellFunctionRun(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
