/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name:   Msg.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _MSG_H_
#define _MSG_H_

#undef  EXT
#ifdef _IN_TASK_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*
*                           Struct define
*
*-------------------------------------------------------------------------------
*/
typedef enum
{
    //system message
    MSG_AUDIO_DECODE_FILL_BUFFER = (UINT32)0x00,
    MSG_AUDIO_ENCODE_CLEAR_BUFFER,
    //End
    MSG_ID_MAX  //max message number

}MSG_ID;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/
//_ATTR_OS_BSS_  EXT UINT32  Msg[(MSG_ID_MAX / 32) + 1];

/*
*-------------------------------------------------------------------------------
*
*                           Functon Declaration
*
*-------------------------------------------------------------------------------
*/
extern void SendMsg(MSG_ID MsgId);
extern BOOL GetMsg(MSG_ID MsgId);
extern void ClearMsg(MSG_ID MsgId);
extern BOOL CheckMsg(MSG_ID MsgId);

/*
********************************************************************************
*
*                         End of Msg.h
*
********************************************************************************
*/
#endif
