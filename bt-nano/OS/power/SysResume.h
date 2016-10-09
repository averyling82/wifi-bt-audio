/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: \Common\System\SysService\SysResume.h
* Owner: aaron.sun
* Date: 2014.8.29
* Time: 16:34:45
* Desc: system resume
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.8.29     16:34:45   1.0
********************************************************************************************
*/

#ifndef ___COMMON_SYSTEM_SYSSERVICE_SYSRESUME_H__
#define ___COMMON_SYSTEM_SYSSERVICE_SYSRESUME_H__

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
typedef enum _SYS_SUSPEND_LEVEL
{
    LEVEL_INIT,  //init suspend resume state
    LEVEL_0,     //device work, memery work
    LEVEL_1,     //device suspend, memery suspend
    LEVEL_2,     //device power off, memery suspend
    LEVEL_3,     //device power off, memery power off
    LEVEL_NUM

}SYS_SUSPEND_LEVEL;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern uint32 SysState;
extern uint32 SysWakeCnt;
extern uint32 PrevSysWakeCnt;
extern uint32 PrevSysTickCounter;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern rk_err_t SysResume_Shell(HDC dev, uint8 * pstr);
extern rk_err_t SysResume(uint32 Level);



#endif

