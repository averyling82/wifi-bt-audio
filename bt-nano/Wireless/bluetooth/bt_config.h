/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: bluetooth\bt_config.h
* Owner: wangping
* Date: 2015.9.18
* Time: 16:05:48
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.9.18     16:05:48   1.0
********************************************************************************************
*/


#ifndef __WEB_BLUETOOTH_BT_CONFIG_H__
#define __WEB_BLUETOOTH_BT_CONFIG_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "BspConfig.h"
#include "RKOS.h"
#include "BSP.h"
#include "effect.h"
#include "SysInfoSave.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/* Signed.  */
typedef signed char     int8_t;
typedef short int       int16_t;
typedef int             int32_t;

/* Unsigned.  */
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;

#define _WEB_BLUETOOTH_BT_CONFIG_COMMON_  __attribute__((section("web_bluetooth_bt_config_common")))
#define _WEB_BLUETOOTH_BT_CONFIG_INIT_  __attribute__((section("web_bluetooth_bt_config_init")))
#define _WEB_BLUETOOTH_BT_CONFIG_SHELL_  __attribute__((section("web_bluetooth_bt_config_shell")))

#define BT_DEBUG(format,...)    printf("%s,L:%d:"format"\n", __MODULE__, __LINE__, ##__VA_ARGS__)

//-----------------------------------------------------------------------------
#define BT_HCI_DATA_RECV_TASK_STACK_SIZE        (256)
#define BT_HCI_DATA_PROCESS_TASK_STACK_SIZE     (256)
#define BT_HCI_H5_DATA_SEND_TASK_STACK_SIZE     (256)
#define TASK_PRIORITY_BLUETOOTH_CTRL_STACK_SIZE (1024)



//configer bluetooth function.
#define BT_CHIP_CC2564      0
#define BT_CHIP_CC2564B     1
#define BT_CHIP_RTL8761AT   2
#define BT_CHIP_RTL8761ATV  3
#define BT_CHIP_AP6212      4

#define BT_CHIP_CONFIG    BT_CHIP_RTL8761ATV
#define BT_ENABLE_SET_MAC
#define BT_UART_INTERFACE_H4   1
#define BT_UART_INTERFACE_H5   2

#if ((BT_CHIP_CONFIG == BT_CHIP_AP6212) || (BT_CHIP_CONFIG == BT_CHIP_RTL8761AT))
#define BT_UART_INTERFACE_CONFIG BT_UART_INTERFACE_H4
#endif

#if (BT_CHIP_CONFIG == BT_CHIP_RTL8761ATV)
#define BT_UART_INTERFACE_CONFIG BT_UART_INTERFACE_H5
#endif

#ifdef _BLUETOOTH_

#define _A2DP_SINK_                 //_A2DP_SINK_ and _A2DP_SOUCRE_ can't support at same project, choose one of them
#ifdef _A2DP_SINK_                  //use in src project, for build lwbt lib
#define _SPP_
#define _RFCOMM_                //for build lib
#define _HFP_                 //for build lib
#define _AVDTP_               //for build lib
#define _AVRCP_               //for build lib
//#define _OBEX_               //for build lib
//#define HAVE_BLE
#define _AVRCP_VENDORDEP_
#define _BT_TIMEOUT_RETRY_ 3
#ifdef _HFP_
#define BT_PHONE
//#define _AEC_DECODE             //for bluetooth function
#endif

#ifdef _AVDTP_
#define _SBC_DECODE             //for bluetooth function
//#define A2DP_ACC
#define SSRC                    //for bluetooth function
#define ENABLE_VAD
#endif

#endif

//#define _A2DP_SOUCRE_  //_A2DP_SINK_ and _A2DP_SOUCRE_ can't support at same project, choose one of them
#ifdef _A2DP_SOUCRE_
#define _SBC_ENCODE_
#define BT_OFF_TIME_OUT         500
#define BT_CONNECT_TIME_OUT     2000
#define SSRC                    //for bluetooth function
#define HAVE_BLE
#endif

#ifdef HAVE_BLE
#define BLE_SMALL_MEMROY
//#define BLE_ENABLE_SM
#endif

#ifdef _OBEX_
#define _OPP_               //for build lib
#endif

//#define HCI_DATA_WATCH

//#define ENABLE_BQB_RF_TEST               // when BQB RF test , define the ENABLE_BQB_RF_TEST
#ifndef ENABLE_BQB_RF_TEST          // when BQB RF test , define the ENABLE_BQB_RF_TEST
#ifndef _A2DP_SOUCRE_
//#define ENABLE_PAIR_TIMER
//#define PAIR_TIME_OUT       6000    //60s ,when time out ,bt discoverable_disable and go to sleep
//#define ENABLE_DEEP_SLEEP
//#define ENABLE_NFC
#endif
#endif

//#define BT_VOICENOTIFY
#ifdef BT_VOICENOTIFY
#define VOICE_NOTIFY_VOL        24
#endif

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



#endif
