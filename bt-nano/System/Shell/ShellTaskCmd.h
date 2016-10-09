/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellTaskCmd.h
* Owner: aaron.sun
* Date: 2015.7.24
* Time: 11:16:16
* Version: 1.0
* Desc: task shell cmd
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.7.24     11:16:16   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_SHELL_SHELLTASKCMD_H__
#define __SYSTEM_SHELL_SHELLTASKCMD_H__

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

#define _SYSTEM_SHELL_SHELLTASKCMD_COMMON_  __attribute__((section("system_shell_shelltaskcmd_common")))
#define _SYSTEM_SHELL_SHELLTASKCMD_INIT_  __attribute__((section("system_shell_shelltaskcmd_init")))
#define _SYSTEM_SHELL_SHELLTASKCMD_SHELL_  __attribute__((section("system_shell_shelltaskcmd_shell")))
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
extern rk_err_t GetRKTaskName(void * pcb, uint8 * taskname);
extern rk_err_t ShellTaskParsing(HDC dev, uint8 * pstr);


#endif
