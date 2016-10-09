/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Line_In\LineIntask.h
* Owner: cjh
* Date: 2016.5.11
* Time: 9:02:10
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.11     9:02:10   1.0
********************************************************************************************
*/


#ifndef __APP_LINE_IN_LINEINTASK_H__
#define __APP_LINE_IN_LINEINTASK_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _LINEIN_AUDIO_INFO
{
    uint32 InputType;
    uint32 Samplerate;
    uint32 channels;
    uint32 bitpersample;
    uint32 PlayVolume;
    uint32 playerr;
    uint32 EqMode;

}LINEIN_AUDIO_INFO;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _APP_LINE_IN_LINEINTASK_COMMON_  __attribute__((section("app_line_in_lineintask_common")))
#define _APP_LINE_IN_LINEINTASK_INIT_  __attribute__((section("app_line_in_lineintask_common")))
#define _APP_LINE_IN_LINEINTASK_SHELL_  __attribute__((section("app_line_in_lineintask_shell")))
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
extern rk_err_t LineInTask_Resume(uint32 ObjectID);
extern rk_err_t LineInTask_Suspend(uint32 ObjectID);
extern void LineInTask_Enter(void * arg);
extern rk_err_t LineInTask_DeInit(void *pvParameters);
extern rk_err_t LineInTask_Init(void *pvParameters, void *arg);
extern rk_err_t cjhTask_Resume(uint32 ObjectID);
extern rk_err_t cjhTask_Suspend(uint32 ObjectID);
extern void cjhTask_Enter(void * arg);
extern rk_err_t cjhTask_DeInit(void *pvParameters);
extern rk_err_t cjhTask_Init(void *pvParameters, void *arg);



#endif
