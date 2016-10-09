/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellCustomCmd.h
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 14:00:58
* Version: 1.0
* Desc: shell custom cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     14:00:58   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_SHELL_SHELLCUSTOMCMD_H__
#define __SYSTEM_SHELL_SHELLCUSTOMCMD_H__

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

#define _SYSTEM_SHELL_SHELLCUSTOMCMD_COMMON_  __attribute__((section("system_shell_shellcustomcmd_common")))
#define _SYSTEM_SHELL_SHELLCUSTOMCMD_INIT_  __attribute__((section("system_shell_shellcustomcmd_init")))
#define _SYSTEM_SHELL_SHELLCUSTOMCMD_SHELL_  __attribute__((section("system_shell_shellcustomcmd_shell")))
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
extern rk_err_t ShellCustomParsing(HDC dev, uint8 * pstr);



#endif
