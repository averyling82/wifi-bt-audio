/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\SystemSet\SystemSetTask.h
* Owner: cjh
* Date: 2015.11.16
* Time: 10:13:43
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.11.16     10:13:43   1.0
********************************************************************************************
*/


#ifndef __APP_SYSTEMSET_SYSTEMSETTASK_H__
#define __APP_SYSTEMSET_SYSTEMSETTASK_H__

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

typedef struct _RK_TASK_SYSTEMSET_ARG
{
    uint32 xxx;
}RK_TASK_SYSTEMSET_ARG;

#define _APP_SYSTEMSET_SYSTEMSETTASK_COMMON_  __attribute__((section("app_systemset_systemsettask_common")))
#define _APP_SYSTEMSET_SYSTEMSETTASK_INIT_  __attribute__((section("app_systemset_systemsettask_common")))
#define _APP_SYSTEMSET_SYSTEMSETTASK_SHELL_  __attribute__((section("app_systemset_systemsettask_shell")))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    SYSTEM_SET_UPDATA = 0x11,
    SYSTEM_SET_ENTER,
    SYSTEM_SET_DLETE_MSGBOX,

} eSYSTEM_TYPE;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t SystemTask_Delete(void);
extern rk_err_t SystemSetTask_UpData(uint32 event_type, uint32 event, void * arg, uint32 mode);
extern rk_err_t SystemSetTask_Resume(uint32 ObjectID);
extern rk_err_t SystemSetTask_Suspend(uint32 ObjectID);
extern void SystemSetTask_Enter(void * arg);
extern rk_err_t SystemSetTask_DeInit(void *pvParameters);
extern rk_err_t SystemSetTask_Init(void *pvParameters, void *arg);



#endif
