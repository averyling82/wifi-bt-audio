/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\MailBox\MailBoxDevice.h
* Owner: cjh
* Date: 2015.5.21
* Time: 9:11:17
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.5.21     9:11:17   1.0
********************************************************************************************
*/


#ifndef __DRIVER_MAILBOX_MAILBOXDEVICE_H__
#define __DRIVER_MAILBOX_MAILBOXDEVICE_H__

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
typedef  struct _MAILBOX_DEV_ARG
{
  uint32 TypeA2BB2A;

}MAILBOX_DEV_ARG;

#define _DRIVER_MAILBOX_MAILBOXDEVICE_COMMON_  __attribute__((section("driver_mailbox_mailboxdevice_common")))
#define _DRIVER_MAILBOX_MAILBOXDEVICE_INIT_  __attribute__((section("driver_mailbox_mailboxdevice_init")))
#define _DRIVER_MAILBOX_MAILBOXDEVICE_SHELL_  __attribute__((section("driver_mailbox_mailboxdevice_shell")))
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
extern void MailBoxDevA2BIsr0(void);
extern void MailBoxDevA2BIsr1(void);
extern void MailBoxDevA2BIsr2(void);
extern void MailBoxDevA2BIsr3(void);
extern void MailBoxDevB2AIsr0(void);
extern void MailBoxDevB2AIsr1(void);
extern void MailBoxDevB2AIsr2(void);
extern void MailBoxDevB2AIsr3(void);
extern rk_err_t MailBoxDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t MailBoxDev_Write(HDC dev);
extern rk_err_t MailBoxDev_Read(HDC dev);
extern rk_err_t MailBoxDev_Delete(uint32 DevID, void * arg);
extern HDC MailBoxDev_Create(uint32 DevID, void * arg);


#endif
