/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellTask.h
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 14:14:20
* Version: 1.0
* Desc: shell task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     14:14:20   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_SHELL_SHELLTASK_H__
#define __SYSTEM_SHELL_SHELLTASK_H__
#include "BspConfig.h"
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

#define _SYSTEM_SHELL_SHELLTASK_COMMON_  __attribute__((section("system_shell_shelltask_common")))
#define _SYSTEM_SHELL_SHELLTASK_INIT_  __attribute__((section("system_shell_shelltask_init")))
#define _SYSTEM_SHELL_SHELLTASK_SHELL_  __attribute__((section("system_shell_shelltask_shell")))
#ifndef SHELL_CMD_NAME_LEN_MAX
#define SHELL_CMD_NAME_LEN_MAX 12
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef rk_err_t(* SHELL_PARASE_FUN)(HDC dev, uint8 *pStrBuff);
typedef  struct _SHELL_CMD
{
    char  CmdName[SHELL_CMD_NAME_LEN_MAX];
    SHELL_PARASE_FUN ShellCmdParaseFun;
    char * CmdSampleDes;
    char * CmdDes;

}SHELL_CMD;




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t ShellHelpDesDisplay(HDC dev,  uint8 * CmdDes,  uint8 * pstr);
extern rk_err_t ShellHelpSampleDesDisplay(HDC dev,  SHELL_CMD * cmd,  uint8 * pstr);
extern uint32 ShellItemExtract(uint8 * pstr, uint8 ** pItem, uint8 * Space);
extern rk_err_t ShellCheckCmd(SHELL_CMD CmdStr[],  uint8 * Cmd,  uint32 len);
extern void ShellTask(void * arg);
extern rk_err_t ShellTaskDeInit(void *pvParameters);
extern rk_err_t ShellTaskInit(void *pvParameters, void * arg);
extern rk_err_t ShellRootParsing(HDC dev, uint8 * pstr);



#endif
