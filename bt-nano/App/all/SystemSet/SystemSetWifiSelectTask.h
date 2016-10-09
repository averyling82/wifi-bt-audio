/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\SystemSet\systemSetWifiSelectTask.h
* Owner: cjh
* Date: 2015.11.17
* Time: 15:30:01
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.11.17     15:30:01   1.0
********************************************************************************************
*/


#ifndef __APP_SYSTEMSET_SYSTEMSETWIFISECLECTTASK_H__
#define __APP_SYSTEMSET_SYSTEMSETWIFISECLECTTASK_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define wifiItem0               0
#define wifiItem1               1
#define wifiItem2               2
#define wifiItem3               3


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef struct _RK_TASK_SYSTEMSETWIFISELECT_ARG
{
    uint32 xxx;
}RK_TASK_SYSTEMSETWIFISELECT_ARG;

#define _APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_COMMON_  __attribute__((section("app_systemset_systemsetwifiselecttask_common")))
#define _APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_INIT_  __attribute__((section("app_systemset_systemsetwifiselecttask_common")))
#define _APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_SHELL_  __attribute__((section("app_systemset_systemsetwifiselecttask_shell")))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    SYSTEM_WIFI_NEW_SSID = 0x11,
    SYSTEM_WIFI_RECIVE_KEY,
    SYSTEM_WIFI_LIST_ENTER,

} eSYSTEM_WIFI_TYPE;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t SystemSetWifiSelectTask_Delete(void);
extern rk_err_t SystemSetWifiSelectTask_Resume(uint32 ObjectID);
extern rk_err_t SystemSetWifiSelectTask_Suspend(uint32 ObjectID);
extern void SystemSetWifiSelectTask_Enter(void * arg);
extern rk_err_t SystemSetWifiSelectTask_DeInit(void *pvParameters);
extern rk_err_t SystemSetWifiSelectTask_Init(void *pvParameters, void *arg);



#endif
