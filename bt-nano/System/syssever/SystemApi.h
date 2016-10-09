/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\syssever\SystemApi.h
* Owner: aaron.sun
* Date: 2016.6.2
* Time: 9:44:55
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.6.2     9:44:55   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_SYSSEVER_SYSTEMAPI_H__
#define __SYSTEM_SYSSEVER_SYSTEMAPI_H__

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

#define _SYSTEM_SYSSEVER_SYSTEMAPI_COMMON_  __attribute__((section("system_syssever_systemapi_common")))
#define _SYSTEM_SYSSEVER_SYSTEMAPI_INIT_  __attribute__((section("system_syssever_systemapi_init")))
#define _SYSTEM_SYSSEVER_SYSTEMAPI_SHELL_  __attribute__((section("system_syssever_systemapi_shell")))
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
extern rk_err_t rkos_get_wifi_ssid(uint8 * ssid);
extern rk_err_t rkos_get_storage_size(void);
extern rk_err_t rkos_get_bt_status(void);
extern rk_err_t rkos_get_sys_volume(void);
extern rk_err_t rkos_get_battery_level(void);
extern rk_err_t rkos_get_battery_status(void);
extern rk_err_t rkos_get_wifi_status(void);



#endif
