/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: system\usbserver\usbservice.h
* Owner: wrm
* Date: 2015.8.20
* Time: 17:29:00
* Version: 1.0
* Desc: usb server
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wrm        2015.8.20     17:29:00   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_USBSERVER_USBSERVICE_H__
#define __SYSTEM_USBSERVER_USBSERVICE_H__

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

typedef struct _USBSERVICE_ARG
{
    uint32 xxx;

}USBSERVICE_ARG;

#define _SYSTEM_USBSERVER_USBSERVICE_COMMON_  __attribute__((section("system_usbserver_usbserverice_common")))
#define _SYSTEM_USBSERVER_USBSERVICE_INIT_    __attribute__((section("system_usbserver_usbserverice_common")))
#define _SYSTEM_USBSERVER_USBSERVICE_SHELL_   __attribute__((section("system_usbserver_usbserverice_shell")))


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

extern rk_err_t UsbService_Resume(void);
extern rk_err_t UsbService_Suspend(void);
extern void     UsbService_Enter(void);
extern rk_err_t UsbService_DeInit(void *pvParameters);
extern rk_err_t UsbService_Init(void *pvParameters, void *arg);
extern rk_err_t UsbService_ButtonCallBack(uint32 evnet_type, uint32 event, void * arg, uint32 mode);
extern void     UsbService_DeleteUsb(int status);
#endif


