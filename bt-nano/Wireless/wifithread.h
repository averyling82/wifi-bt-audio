/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: WICE\wifithread.h
* Owner: linyb
* Date: 2015.7.27
* Time: 8:53:50
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    linyb     2015.7.27     8:53:50   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifndef __WICE_WIFITHREAD_H__
#define __WICE_WIFITHREAD_H__
#include "typedef.h"
#ifdef _WICE_
#include "wwd_constants.h"
#endif
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
//#define _WICE_WIFITHREAD_COMMON_  __attribute__((section("wice_wifithread_common")))
#define _WICE_WIFITHREAD_INIT_
#define _WICE_WIFITHREAD_SHELL_  __attribute__((section("wice_wifithread_shell")))

#ifdef _WICE_
#define EASY_SETUP_ENABLE
#endif

#define WIFI_SUM_SIZE  1024
#define WIFI_LEN  WIFI_SUM_SIZE/512

typedef struct {
    uint8     security_key[64];     /* target AP's password */
    uint8     security_key_length;  /* length of password */
    uint8     ap_ssid_len;              /* target AP's name */
    uint8     ap_ssid_value[32];
    uint8     ap_bssid[6];             /* target AP's mac address */
    uint32 ap_security_type;
} WIFI_AP;

#ifdef _WIFI_FOR_SYSTEM_
typedef struct
{
    uint8 Connect_flag;
    uint8 scan_flag;
    int singal_length;
    WIFI_AP wifi_ap;
}WIFI_SAVE;

typedef struct
{
    uint32 wifiinfoflag;
    uint8 Keyindex;
    uint8 Totalnum;
    WIFI_SAVE wifiinfo[WIFI_AP_COUNT];
    uint8 pad[1024-WIFI_AP_COUNT*sizeof(WIFI_SAVE)-10];
}WIFI_INFO;
#endif

enum
{
    WIFI_INIT = 0,
    WIFI_DEINIT,
    WIFI_SCAN,
    WIFI_SMARTCONFIG,
    WIFI_SMARTCONFIG_STOP,
    WIFI_USARTCONFIG,
    LINKDOWN_SYS,
    LINK_SMARTCONFIG,
    LINKSTART_SYS,
    UPDATE_STRENGTH,
};

typedef enum
{
    WIFI_FALSE = 0,
    WIFI_TRUE  = 1,
    WIFI_WAIT = 2,
    WIFI_ERR = 3,
    WIFI_BUSY = 4

} WIFI_BOOL;


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
extern void wwdTask_Enter(void);
extern rk_err_t wwdTask_Suspend(void * hTask, uint32 Level);
extern rk_err_t wwdTask_Resume(void * hTask);
extern rk_err_t wwdTask_DeInit(void *pvParameters);
extern rk_err_t wwdTask_Init(void *pvParameters, void *arg);
extern rk_err_t wifi_applicationTask_Resume(void * hTask);
extern rk_err_t wifi_applicationTask_Suspend(void * hTask, uint32 Level);
extern void wifi_applicationTask_Enter(void);
extern rk_err_t wifi_applicationTask_DeInit(void *pvParameters);
extern rk_err_t wifi_applicationTask_Init(void *pvParameters, void *arg);
extern rk_err_t networkingTask_DeInit(void *pvParameters);
extern rk_err_t networkingTask_Init(void *pvParameters, void *arg);
extern rk_err_t networkingTask_Suspend(void * hTask, uint32 Level);
extern rk_err_t networkingTask_Resume(void * hTask);
extern void networkingTask_Enter(void);
extern rk_err_t hardware_to_workTask_DeInit(void *pvParameters);
extern rk_err_t hardware_to_workTask_Init(void *pvParameters, void *arg);
extern void hardware_to_workTask_Enter(void);
extern rk_err_t hardware_to_workTask_Suspend(void * hTask, uint32 Level);
extern rk_err_t hardware_to_workTask_Resume(void * hTask);
extern rk_err_t wifi_relink_sysconfig(void);
extern rk_err_t rk_easy_smartconfig_stop(void);
extern rk_err_t rk_wifi_scan(void* mscan_handle);
extern rk_err_t rk_wifi_smartconfig(void);

extern int wifi_init_flag(void);
extern int wifi_easy_setup_flag(void);
extern void rkwifi_get_ssid(uint8 *mssid_value, int *mssid_len);
extern int rk_update_strength(int *strength);
extern int wifi_join_flag(void);
extern int wifi_connect_flag(void);

extern void rk_wifi5_restart(void);
rk_err_t wifi_shell(void * dev, uint8 * pstr);

#endif
