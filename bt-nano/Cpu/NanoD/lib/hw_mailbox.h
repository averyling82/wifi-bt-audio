/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoC\lib\hw_mailbox.h
* Owner: aaron.sun
* Date: 2014.11.18
* Time: 18:16:19
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.11.18     18:16:19   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_HW_MAILBOX_H__
#define __CPU_NANOC_LIB_HW_MAILBOX_H__

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
typedef  struct _MAIL_BOX
{

    uint32 MailBoxA2BIntEn;
    uint32 MailBoxA2BStatus;
    uint32 MailBoxA2BCmd0;
    uint32 MailBoxA2BData0;
    uint32 MailBoxA2BCmd1;
    uint32 MailBoxA2BData1;
    uint32 MailBoxA2BCmd2;
    uint32 MailBoxA2BData2;
    uint32 MailBoxA2BCmd3;
    uint32 MailBoxA2BData3;
    uint32 MailBoxB2AIntEn;
    uint32 MailBoxB2AStatus;
    uint32 MailBoxB2ACmd0;
    uint32 MailBoxB2AData0;
    uint32 MailBoxB2ACmd1;
    uint32 MailBoxB2AData1;
    uint32 MailBoxB2ACmd2;
    uint32 MailBoxB2AData2;
    uint32 MailBoxB2ACmd3;
    uint32 MailBoxB2AData3;

}MAIL_BOX;

#define MailBoxReg              ((MAIL_BOX *)MAILBOX_BASE)

#define MailBoxPort(n)          ( MailBoxReg + n )

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



#endif

