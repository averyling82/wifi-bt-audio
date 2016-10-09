/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\USB\ChargeTask.h
* Owner: aaron.sun
* Date: 2015.11.16
* Time: 15:18:00
* Version: 1.0
* Desc: charge window
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.11.16     15:18:00   1.0
********************************************************************************************
*/


#ifndef __APP_USB_CHARGETASK_H__
#define __APP_USB_CHARGETASK_H__

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

typedef struct _RK_TASK_CHARGE_ARG
{
    uint32 xxx;
}RK_TASK_CHARGE_ARG;

#define _APP_USB_CHARGETASK_COMMON_  __attribute__((section("app_usb_chargetask_common")))
#define _APP_USB_CHARGETASK_INIT_  __attribute__((section("app_usb_chargetask_common")))
#define _APP_USB_CHARGETASK_SHELL_  __attribute__((section("app_usb_chargetask_shell")))
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
extern rk_err_t ChargeTask_Resume(uint32 ObjectID);
extern rk_err_t ChargeTask_Suspend(uint32 ObjectID);
extern void ChargeTask_Enter(void * arg);
extern rk_err_t ChargeTask_DeInit(void *pvParameters);
extern rk_err_t ChargeTask_Init(void *pvParameters, void *arg);



#endif
