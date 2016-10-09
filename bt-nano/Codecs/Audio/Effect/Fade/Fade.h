/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\codecs\audio\effect\fade.h
* Owner: cjh
* Date: 2015.1.11
* Time: 15:19:31
* Desc: audio fade
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*     cjh     2015.11.11     9:19:31   1.0
********************************************************************************************
*/

#ifndef _H_AUDIO_FADE
#define _H_AUDIO_FADE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CODECS_AUDIO_FADE_COMMON_  __attribute__((section("codecs_audio_fade_common")))
#define _CODECS_AUDIO_FADE_DATA_  __attribute__((section("codecs_audio_fade_data")))


#ifdef CODEC_24BIT
typedef long    fade_short;
typedef long long   fade_long;
#define fade_max_coef   16777215
#define fade_scale   24
#else
typedef short    fade_short;
typedef long    fade_long;
#define fade_max_coef   65535
#define fade_scale   16
#endif

#define FADE_IN     0
#define FADE_OUT    1
#define FADE_NULL   -1

//initialization.
//begin:the frist specimen serial number,len:length  type: 0-fade in 1-fade out.
void FadeInit(long begin,long len,int type);
void DC_filter(short *pwBuffer, unsigned short frameLen);
long FadeDoOnce();
void FadeProcess(fade_short *pwBuffer, unsigned short frameLen);
int FadeIsFinished();

//#endif

#endif
