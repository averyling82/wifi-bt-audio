/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Key\KeyDevice.h
* Owner: aaron.sun
* Date: 2015.8.20
* Time: 14:09:22
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.8.20     14:09:22   1.0
********************************************************************************************
*/


#ifndef __DRIVER_KEY_KEYDEVICE_H__
#define __DRIVER_KEY_KEYDEVICE_H__

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
#define _DRIVER_KEY_KEYDEVICE_COMMON_  __attribute__((section("driver_key_keydevice_common")))
#define _DRIVER_KEY_KEYDEVICE_INIT_  __attribute__((section("driver_key_keydevice_init")))
#define _DRIVER_KEY_KEYDEVICE_SHELL_  __attribute__((section("driver_key_keydevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_KEY_KEYDEVICE_SHELL_DATA_      _DRIVER_KEY_KEYDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_KEY_KEYDEVICE_SHELL_DATA_      __attribute__((section("driver_key_keydevice_shell_data")))
#else
#error Unknown compiling tools.
#endif


#define KEY_STATUS_NONE             ((UINT32)0xffffffff)
#define KEY_STATUS_DOWN             ((UINT32)0x0001 << 28)      //0x1000,0000
#define KEY_STATUS_PRESS_START      ((UINT32)0x0002 << 28)      //0x2000,0000
#define KEY_STATUS_PRESS            ((UINT32)0x0003 << 28)      //0x3000,0000
#define KEY_STATUS_SHORT_UP         ((UINT32)0x0004 << 28)      //0x4000,0000
#define KEY_STATUS_LONG_UP          ((UINT32)0x0005 << 28)      //0x5000,0000

#define KEY_VAL_HOLD_ON             (((UINT32)0x0001 << 24) | (KEY_STATUS_DOWN))
#define KEY_VAL_HOLD_OFF            (((UINT32)0x0001 << 25) | (KEY_STATUS_DOWN))

#define KEY_VAL_PLAY_DOWN           ((KEY_VAL_PLAY)|(KEY_STATUS_DOWN))
#define KEY_VAL_PLAY_PRESS_START    ((KEY_VAL_PLAY)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_PLAY_PRESS          ((KEY_VAL_PLAY)|(KEY_STATUS_PRESS))
#define KEY_VAL_PLAY_SHORT_UP       ((KEY_VAL_PLAY)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_PLAY_LONG_UP        ((KEY_VAL_PLAY)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_UP_DOWN            ((KEY_VAL_UP)|(KEY_STATUS_DOWN))
#define KEY_VAL_UP_PRESS_START     ((KEY_VAL_UP)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_UP_PRESS           ((KEY_VAL_UP)|(KEY_STATUS_PRESS))
#define KEY_VAL_UP_SHORT_UP        ((KEY_VAL_UP)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_UP_LONG_UP         ((KEY_VAL_UP)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_DOWN_DOWN            ((KEY_VAL_DOWN)|(KEY_STATUS_DOWN))
#define KEY_VAL_DOWN_PRESS_START     ((KEY_VAL_DOWN)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_DOWN_PRESS           ((KEY_VAL_DOWN)|(KEY_STATUS_PRESS))
#define KEY_VAL_DOWN_SHORT_UP        ((KEY_VAL_DOWN)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_DOWN_LONG_UP         ((KEY_VAL_DOWN)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_ESC_DOWN           ((KEY_VAL_ESC)|(KEY_STATUS_DOWN))
#define KEY_VAL_ESC_PRESS_START    ((KEY_VAL_ESC)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_ESC_PRESS          ((KEY_VAL_ESC)|(KEY_STATUS_PRESS))
#define KEY_VAL_ESC_SHORT_UP       ((KEY_VAL_ESC)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_ESC_LONG_UP        ((KEY_VAL_ESC)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_MENU_DOWN           ((KEY_VAL_MENU)|(KEY_STATUS_DOWN))
#define KEY_VAL_MENU_PRESS_START    ((KEY_VAL_MENU)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_MENU_PRESS          ((KEY_VAL_MENU)|(KEY_STATUS_PRESS))
#define KEY_VAL_MENU_SHORT_UP       ((KEY_VAL_MENU)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_MENU_LONG_UP        ((KEY_VAL_MENU)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_FFW_DOWN            ((KEY_VAL_FFW)|(KEY_STATUS_DOWN))
#define KEY_VAL_FFW_PRESS_START     ((KEY_VAL_FFW)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_FFW_PRESS           ((KEY_VAL_FFW)|(KEY_STATUS_PRESS))
#define KEY_VAL_FFW_SHORT_UP        ((KEY_VAL_FFW)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_FFW_LONG_UP         ((KEY_VAL_FFW)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_FFD_DOWN            ((KEY_VAL_FFD)|(KEY_STATUS_DOWN))
#define KEY_VAL_FFD_PRESS_START     ((KEY_VAL_FFD)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_FFD_PRESS           ((KEY_VAL_FFD)|(KEY_STATUS_PRESS))
#define KEY_VAL_FFD_SHORT_UP        ((KEY_VAL_FFD)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_FFD_LONG_UP         ((KEY_VAL_FFD)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_VOL_DOWN            ((KEY_VAL_VOL)|(KEY_STATUS_DOWN))
#define KEY_VAL_VOL_PRESS_START     ((KEY_VAL_VOL)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_VOL_PRESS           ((KEY_VAL_VOL)|(KEY_STATUS_PRESS))
#define KEY_VAL_VOL_SHORT_UP        ((KEY_VAL_VOL)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_VOL_LONG_UP         ((KEY_VAL_VOL)|(KEY_STATUS_LONG_UP))

//hold 是memu 和play 的长按
#define KEY_VAL_HOLD_DOWN           ((KEY_VAL_HOLD)|(KEY_STATUS_DOWN))
#define KEY_VAL_HOLD_PRESS_START    ((KEY_VAL_HOLD)|(KEY_STATUS_PRESS_START))
#define KEY_VAL_HOLD_PRESS          ((KEY_VAL_HOLD)|(KEY_STATUS_PRESS))
#define KEY_VAL_HOLD_SHORT_UP       ((KEY_VAL_HOLD)|(KEY_STATUS_SHORT_UP))
#define KEY_VAL_HOLD_LONG_UP        ((KEY_VAL_HOLD)|(KEY_STATUS_LONG_UP))

#define KEY_VAL_POWER_DOWN          (KEY_VAL_PLAY_PRESS)

typedef rk_err_t (*pKeyDevRx_indicate)(HDC dev);

typedef struct _KEY_DEV_ARG
{
    HDC ADCHandler;
}KEY_DEV_ARG;

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
extern rk_err_t KeyDev_Delete(uint32 DevID, void * arg);
extern HDC      KeyDev_Create(uint32 DevID, void * arg);
extern rk_err_t KeyDevInit(HDC dev);
extern rk_err_t KeyDevDeInit(HDC dev);
extern rk_size_t KeyDevRead(HDC dev);
extern rk_err_t KeyDevStop(HDC dev);
extern rk_err_t KeyRxIndicate(HDC dev, uint32 size);
extern rk_err_t KeyDevSuspend(HDC dev, uint32 Level);
extern rk_err_t KeyDevResume(HDC dev);
extern rk_err_t KeyDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t KeyDev_UnRegister(HDC dev);
extern rk_err_t KeyDev_Register(HDC dev, pKeyDevRx_indicate RxCallBack);
extern void     KeySendMsgToGUI(uint32 KeyValue);
extern void     KeySendMsg(uint32 KeyValue);



#endif
