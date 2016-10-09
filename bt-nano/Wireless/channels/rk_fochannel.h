


/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Web\channels\rk_fochannel.h
* Owner: lyb
* Date: 2015.11.4
* Time: 11:20:33
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    lyb     2015.11.4     11:20:33   1.0
********************************************************************************************
*/


#ifndef __WEB_CHANNELS_RK_FOCHANNEL_H__
#define __WEB_CHANNELS_RK_FOCHANNEL_H__

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


//#define _RK_BROADTASK_COMMON_  __attribute__((section("rk_broadtask_common")))
//#define _RK_BROADTASK_INIT_  __attribute__((section("rk_broadtask_init")))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void RK_ChannelTask_Enter(void);
extern rk_err_t RK_TcpChannelTask_Init(void *pvParameters, void *arg);
extern rk_err_t RK_TcpChannelTask_Deinit(void *pvParameters);
extern void RK_BroadTask_Enter(void);
extern rk_err_t RK_TcpBroadTask_Init(void *pvParameters, void *arg);
extern rk_err_t RK_TcpBroadTask_Deinit(void *pvParameters);
extern void RK_TcpStreamTask_Enter(void);
extern rk_err_t RK_TcpStreamTask_Init(void *pvParameters, void *arg);
extern rk_err_t RK_TcpStreamTask_Deinit(void *pvParameters);
extern void RK_TcpControlTask_Enter(void);
extern rk_err_t RK_TcpControlTask_Init(void *pvParameters, void *arg);
extern rk_err_t RK_TcpControlTask_Deinit(void *pvParameters);
extern rk_err_t RK_TcpHeartTask_Init(void *pvParameters, void *arg);
extern rk_err_t RK_TcpHeartTask_Deinit(void *pvParameters);
extern void RK_TcpHeartTask_Enter(void);


extern void GpstChannelSet_volume(void);
extern void GpstChannelSet_volume1(void);

extern void tcpcontrol_relink(void* channel_ask);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif
