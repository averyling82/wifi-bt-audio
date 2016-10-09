/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Bcore\BcoreDevice.h
* Owner: aaron.sun
* Date: 2015.6.26
* Time: 10:19:13
* Version: 1.0
* Desc: BB system driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.26     10:19:13   1.0
********************************************************************************************
*/


#ifndef __DRIVER_BCORE_BCOREDEVICE_H__
#define __DRIVER_BCORE_BCOREDEVICE_H__

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
typedef enum _MSGBOX_SYSTEM_CMD
{
    MSGBOX_CMD_SYSTEM_NULL,
    MSGBOX_CMD_SYSTEM_PRINT_LOG,
    MSGBOX_CMD_SYSTEM_PRINT_LOG_OK,
    MSGBOX_CMD_SYSTEM_START_OK,
    MSGBOX_CMD_SYSTEM_SET_FREQ,
    MSGBOX_CMD_SYSTEM_GET_STATUS,
    MSGBOX_CMD_SYSTEM_PAUSE,
    MSGBOX_CMD_SYSTEM_RUN,
    MSGBOX_CMD_SYSTEM_RESET,
    MSGBOX_CMD_SYSTEM_NUM

}MSGBOX_SYSTEM_CMD;


typedef  struct _BCORE_DEV_ARG
{
    uint32 Channel;

}BCORE_DEV_ARG;

#define _DRIVER_BCORE_BCOREDEVICE_COMMON_  __attribute__((section("driver_bcore_bcoredevice_common")))
#define _DRIVER_BCORE_BCOREDEVICE_INIT_  __attribute__((section("driver_bcore_bcoredevice_init")))
#define _DRIVER_BCORE_BCOREDEVICE_SHELL_  __attribute__((section("driver_bcore_bcoredevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_BCORE_BCOREDEVICE_DATA_      _DRIVER_BCORE_BCOREDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_BCORE_BCOREDEVICE_DATA_      __attribute__((section("driver_bcore_bcoredevice_data")))
#else
#error Unknown compiling tools.
#endif
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
extern rk_err_t BcoreDev_IsRuning(HDC dev);
extern rk_err_t BcoreDev_ShutOff(HDC dev);
extern rk_err_t BcoreDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t BcoreDev_Task_DeInit(void *pvParameters);
extern rk_err_t BcoreDev_Task_Init(void *pvParameters);
extern void BcoreDev_Task_Enter(void);
extern rk_err_t BcoreDev_Write(HDC dev, uint32 Cmd, uint32 Data, uint32 port);
extern rk_err_t BcoreDev_Start(HDC dev, uint32 ModuleId);
extern rk_err_t BcoreDev_Delete(uint32 DevID, void * arg);
extern HDC BcoreDev_Create(uint32 DevID, void * arg);
extern void BcoreDevIntIsr0(void);



#endif
