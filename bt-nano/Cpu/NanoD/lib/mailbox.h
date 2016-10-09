/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\mailbox.h
* Owner: aaron.sun
* Date: 2015.8.26
* Time: 15:13:49
* Version: 1.0
* Desc: mailbox bsp driver h file
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.8.26     15:13:49   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_MAILBOX_H__
#define __CPU_NANOD_LIB_MAILBOX_H__

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

#define _CPU_NANOD_LIB_MAILBOX_COMMON_  __attribute__((section("cpu_nanod_lib_mailbox_common")))
#define _CPU_NANOD_LIB_MAILBOX_INIT_  __attribute__((section("cpu_nanod_lib_mailbox_init")))
#define _CPU_NANOD_LIB_MAILBOX_SHELL_  __attribute__((section("cpu_nanod_lib_mailbox_shell")))

#define MAILBOX_INT_0 (uint32)(1 << 0)
#define MAILBOX_INT_1 (uint32)(1 << 1)
#define MAILBOX_INT_2 (uint32)(1 << 2)
#define MAILBOX_INT_3 (uint32)(1 << 3)

#define MAILBOX_CHANNEL_0 (0)
#define MAILBOX_CHANNEL_1 (1)
#define MAILBOX_CHANNEL_2 (2)
#define MAILBOX_CHANNEL_3 (3)

typedef enum _MAILBOX_INT_TYPE_A2B_B2A
{
    MAILBOX_A2B_TYPE = 1,
    MAILBOX_B2A_TYPE = 2,

    MAILBOX_TYPE_MAX
}MAILBOX_INT_TYPE_A2B_B2A, * P_MAILBOX_INT_TYPE_A2B_B2A;

#define MAILBOX_ID_0 (0)
#define MAILBOX_ID_1 (1)
#define MAILBOX_ID_2 (2)
#define MAILBOX_ID_3 (3)
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
extern uint32 MailBoxGetIntType(uint32 MailBoxID, uint32 TypeA2BB2A);
extern rk_err_t MailBoxClearA2BInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxClearB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxDisableA2BInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxDisableB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxEnableB2AInt(uint32 MailBoxID, uint32 IntType);
extern rk_err_t MailBoxEnableA2BInt(uint32 MailBoxID, uint32 IntType);
extern uint32 MailBoxReadB2ACmd(uint32 MailBoxID, uint32 CmdPort);
extern uint32 MailBoxReadB2AData(uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteB2ACmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort);
extern rk_err_t MailBoxWriteB2AData(uint32 Data, uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteA2BData(uint32 Data, uint32 MailBoxID, uint32 DataPort);
extern uint32 MailBoxReadA2BData(uint32 MailBoxID, uint32 DataPort);
extern rk_err_t MailBoxWriteA2BCmd(uint32 Cmd, uint32 MailBoxID, uint32 CmdPort);
extern uint32 MailBoxReadA2BCmd(uint32 MailBoxID, uint32 CmdPort);


#endif
