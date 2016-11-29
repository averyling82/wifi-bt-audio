/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: WICE\wifithread.c
* Owner: linyb
* Date: 2015.7.27
* Time: 8:48:53
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    linyb     2015.7.27     8:48:53   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef _WIFI_
#define NOT_INCLUDE_OTHER

//#pragma arm section code = "ap6181wifiCode", rodata = "ap6181wifiCode", rwdata = "ap6181wifidata", zidata = "ap6181wifidata"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "RKOS.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "DeviceManagerTask.h"
#include "ShellTask.h"
#include "PowerManager.h"
#include "DriverInclude.h"
#include "DeviceInclude.h"
#include "Bsp.h"
#ifdef _WICE_
#include "ap6181wifi.h"
#include "wiced_wifi.h"
#include "wiced_management.h"
#include "wwd_events.h"
#include "tcpip.h"
#endif

#ifdef _REALTEK_
#include <autoconf.h>
#include "wifi_conf.h"
#include "wifi_io.h"
#include "wifi_constants.h"
#include "wifi_structures.h"
#include <stdint.h>
#include "lwip_netconf.h"
#include "freertos_service.h"
#endif

#include "wifithread.h"
#include "FwAnalysis.h"
#include "SysInfoSave.h"

#ifdef _WICE_
extern int  wifi_scan_doing;
wiced_bool_t wifi_network_up_flag = WICED_FALSE;
//wiced_bool_t wifi_connect = WICED_FALSE;
extern wiced_bool_t wiced_sta_link_up;
extern wiced_bool_t  wiced_wlan_initialised;
extern unsigned char easy_setup_flag;
extern unsigned char easy_setup_stop_flag;
extern pTimer  TimerHandler;
static  wiced_scan_result_t  rk_scan_result[1]= {0};
static  wiced_scan_result_t* rk_result_ptr = (wiced_scan_result_t *) &rk_scan_result;
extern const wwd_event_num_t        link_events[];
extern void wifi_application_thread(void * arg);
extern void WifiLedStatusIsr(void);
extern void link_up( void );
extern void link_down( void );
extern void* wiced_link_events_handler( const wwd_event_header_t* event_header, const uint8_t* event_data, void* handler_user_data );
extern void rk_scan_resut_handle( wiced_scan_result_handler_t results_handler, void* user_data);
rk_err_t rk_wifi_scan_5G(void *data);
#endif

#ifdef _REALTEK_
#define WIFI_USE_SDIO
#define CONFIG_INIT_NET                 1
#define CONFIG_WEP                      0
#define CONFIG_WPA                      0
#define CONFIG_WPA2                     1
#define CONFIG_PING_TEST                0
#define CONFIG_INTERACTIVE_MODE         0
#define CONFIG_POST_INIT                0
#define CONFIG_START_MP                 0
#define CONFIG_START_STA                0
#define CONFIG_START_AP                 0
#define CONFIG_START_STA_AP             0
#define CONFIG_DHCP_SERVER              0
#define CONFIG_DHCP_CLIENT              1
#define CONFIG_WEB_SERVER               0

#define STA_MODE_SSID   "cpsb-sch-test"    /* Set SSID here */
#define AP_MODE_SSID    "wlan_ap_ssid"    /* Set SSID here */
#define AP_DEFAULT_CH   6
#define WLAN0_NAME      "wlan0"
#define WLAN1_NAME      "wlan1"
#define WPA_PASSPHRASE  "123123123"        /* Max 32 characters */
#define WEP40_KEY       "12345"           /* 5 ASCII characters */

#if CONFIG_LWIP_LAYER
extern struct netif xnetif[NET_IF_NUM];
#endif

#endif

int wifi_set_scan_handle = 0;
WIFI_AP JoinAp;
uint8 wifi_join = 0;
uint8 wifi_link = 0;


int rkwifi_get_strength(char *mssid_value, int mssid_len, uint8* bssid_value, int *strength);


typedef  struct _WIFI_ASK_QUEUE
{
    uint32 cmd;
    WIFI_AP join_ap;
    #ifdef _WICE_
    wiced_scan_result_handler_t scan_hand;
    #endif

}WIFI_ASK_QUEUE;

typedef  struct _WIFI_RESP_QUEUE
{
    int cmd;
    int signal_strength;
}WIFI_RESP_QUEUE;

typedef  struct _WIFIAPP_TASK_DATA_BLOCK
{
    pQueue  WIFIAPPAskQueue;
    pQueue  WIFIAPPRespQueue;
    uint32  WifiMode;  //  1 ap, 2 sta

    //pSemaphore WIFIAPPSem;
}WIFIAPP_TASK_DATA_BLOCK;

typedef struct _WIFI_CURRENT_MESSAGE
{
      pSemaphore  scan_complete_sem;
      int abort_scan;
      uint8 octect[6];
      int strength;
      uint8 channel;

}WIFI_CURRENT_MESSAGE;

static WIFIAPP_TASK_DATA_BLOCK *gWifiAppData = NULL;
static WIFIAPP_TASK_DATA_BLOCK *gWifiBackgroundData = NULL;

#ifdef _WIFI_FOR_SYSTEM_
void rk_wifi_info_save(WIFI_AP ap)
{
    uint16 message_len = 0;
    uint16 count = 0;
    WIFI_INFO *wifi_buf;
    uint8 index = 0;
    uint8 find = 0;
    uint8 i;
    WIFI_AP mwifiap;


    message_len = sizeof(WIFI_INFO);
   // count = message_len/512;
   // printf("mesage_len = %d\n", message_len);

   // printf("apssid_len = %d, passlen = %d\n", ap.ap_ssid_len, ap.security_key_length);
    //debug_hex(ap.ap_ssid_value, 32, 16);
    //debug_hex(ap.security_key, 32, 16);

    rkos_memcpy(&mwifiap, &ap, sizeof(WIFI_AP));

   // printf("mwifiapssid_len = %d, passlen = %d\n", mwifiap.ap_ssid_len, mwifiap.security_key_length);
   // debug_hex(mwifiap.ap_ssid_value, 32, 16);
   // debug_hex(mwifiap.security_key, 32, 16);

    wifi_buf = rkos_memory_malloc(sizeof(WIFI_INFO));
    if(wifi_buf == NULL)
    {
        rk_printf("rk_wifi_info_save wifi_buf malloc fail\n");
        return;
    }

    LUNReadDB(gSysConfig.WifiSSIDStartLBA, WIFI_LEN, wifi_buf);

    rk_printf("wifiinfoflag = %x", wifi_buf->wifiinfoflag);

    if(wifi_buf->wifiinfoflag != 0x5a5a5a5a)
    {
        rkos_memset(wifi_buf, 0x00, sizeof(WIFI_INFO));
        wifi_buf->wifiinfoflag = 0x5a5a5a5a;
    }
    rk_printf("writetotal_num = %d, ssid_len = %d, passlen=%d keyindex=%d\n", wifi_buf->Totalnum, wifi_buf->wifiinfo[0].wifi_ap.ap_ssid_len,wifi_buf->wifiinfo[0].wifi_ap.security_key_length,wifi_buf->Keyindex);

    for(i=0; i<wifi_buf->Totalnum; i++)
        wifi_buf->wifiinfo[i].Connect_flag = 0;

    index = wifi_buf->Keyindex;
    //rk_printf(index = %d);
    if(index < WIFI_AP_COUNT)
    {
        for(i=0; i<wifi_buf->Totalnum; i++)
        {
            if(rkos_memcmp(mwifiap.ap_ssid_value,  wifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value, 32) == 1)
            {
                find = 1;
                break;
            }
        }
        if(find)
        {
            index = i;
        }
        else
        {
            wifi_buf->Keyindex++;
            if(wifi_buf->Keyindex >= WIFI_AP_COUNT)
            {
                wifi_buf->Keyindex = 0;
            }
            wifi_buf->Totalnum++;
            if(wifi_buf->Totalnum >= WIFI_AP_COUNT)
            {
                wifi_buf->Totalnum = WIFI_AP_COUNT;
            }
        }

        printf("index = %d\n", index);
        rkos_memcpy(&(wifi_buf->wifiinfo[index].wifi_ap), &mwifiap, sizeof(WIFI_AP));
        wifi_buf->wifiinfo[index].Connect_flag = 1;
    }

    //printf("writetotal_num = %d, ssid_len = %d, passlen=%d,connect_flag = %d\n", wifi_buf->Totalnum, wifi_buf->wifiinfo[0].wifi_ap.ap_ssid_len, wifi_buf->wifiinfo[0].wifi_ap.security_key_length,wifi_buf->wifiinfo[0].Connect_flag);
    //debug_hex(wifi_buf->wifiinfo[0].wifi_ap.ap_ssid_value, 32, 16);
    //debug_hex(wifi_buf->wifiinfo[0].wifi_ap.security_key, 32, 16);
    //printf("write gSysConfig.WifiSSIDStartLBA = 0x%4x\n,wifi_len = %d", gSysConfig.WifiSSIDStartLBA, WIFI_LEN);

    if(wifi_buf->Totalnum > WIFI_AP_COUNT || index >= WIFI_AP_COUNT)
    {
       // wifi_buf->Totalnum = WIFI_AP_COUNT;
        memset(wifi_buf, 0x00, sizeof(wifi_buf));
        wifi_buf->Totalnum = WIFI_AP_COUNT;
    }
  //  memset(wifi_buf, 0x00, sizeof(wifi_buf));
   // wifi_buf->Totalnum = WIFI_AP_COUNT;
    LUNWriteDB(gSysConfig.WifiSSIDStartLBA, 2, wifi_buf);

#if 0
    //printf("read gSysConfig.WifiSSIDStartLBA = 0x%4x\n", gSysConfig.WifiSSIDStartLBA);
    LUNReadDB(gSysConfig.WifiSSIDStartLBA, 2, wifi_buf);
   // printf("writetotal_num = %d, ssid_len = %d, passlen=%d,connect_flag = %d\n", wifi_buf->Totalnum, wifi_buf->wifiinfo[0].wifi_ap.ap_ssid_len, wifi_buf->wifiinfo[0].wifi_ap.security_key_length,wifi_buf->wifiinfo[0].Connect_flag);
   // debug_hex(wifi_buf->wifiinfo[0].wifi_ap.ap_ssid_value, 32, 16);
   // debug_hex(wifi_buf->wifiinfo[0].wifi_ap.security_key, 32, 16);
#endif

    rkos_memory_free(wifi_buf);
    return;
}

static WIFI_INFO *mwifi_buf = NULL;
rk_err_t rk_wifi_relink_sysinfo(int flag)
{
     uint8 wifi_count=0;
     WIFI_SAVE mwifi;
     int k=0;
     uint8 i=0;
     uint8 j=0;
     rk_err_t ret;
     uint8 connect_ssid[32] = {0};

     wifi_join = WICED_FALSE;

     mwifi_buf = rkos_memory_malloc(sizeof(WIFI_INFO));

     if(NULL == mwifi_buf)
     {
        rk_printf("rk_wifi_relink_sysinfo malloc fail\n");
        wifi_join = WICED_ERR;
        return RK_ERROR;
     }

     ret = LUNReadDB(gSysConfig.WifiSSIDStartLBA, WIFI_LEN, mwifi_buf);
     if((mwifi_buf->Totalnum > 5) || (mwifi_buf->Totalnum == 0))
     {
        rk_printf("read db tot num is error");
        rkos_memory_free(mwifi_buf);
        mwifi_buf = NULL;
        wifi_join = WICED_ERR;
        return RK_ERROR;
     }

     rk_printf("re wifi_buf %d totalnum = %d = %x = %d",ret ,mwifi_buf->Totalnum, mwifi_buf, sizeof(WIFI_INFO));

//扫描之前初始化内部数据，scan_flag connect_flag = 0;
     for(i=0; i<mwifi_buf->Totalnum; i++)
     {
        mwifi_buf->wifiinfo[i].scan_flag = 0;
        if(mwifi_buf->wifiinfo[i].Connect_flag == 1)
        {
            rkos_memcpy(connect_ssid, mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value, 32);
            mwifi_buf->wifiinfo[i].Connect_flag = 0;
        }

        if(easy_setup_flag == WICED_BUSY)
        {
            rkos_memory_free(mwifi_buf);
            mwifi_buf = NULL;
            wifi_join = WICED_ERR;
            return RK_ERROR;
        }
     }


       //rk_printf("re ssid = %s,len = %d password = %s,passlen = %d\n",mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value,mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_len, \
      // mwifi_buf->wifiinfo[i].wifi_ap.security_key, mwifi_buf->wifiinfo[i].wifi_ap.security_key_length);
//进行扫描获取扫描到并有保存的的ssid进行连接
      ret = rk_wifi_scan_5G((void *)mwifi_buf);
        //ret = rkwifi_get_strength(mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value, mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_len,\
             //  mwifi_buf->wifiinfo[i].wifi_ap.ap_bssid, &(mwifi_buf->wifiinfo[i].singal_length));
       // if(ret == RK_SUCCESS)
        //{
            //mwifi_buf->wifiinfo[i].scan_flag = 1;
            //wifi_count++;
        //}
    // }


     //
     if(easy_setup_flag == WICED_BUSY || ret == RK_ERROR)
     {
         rk_printf("easy_setup_flag = %d, ret = %d", easy_setup_flag,ret);
         rkos_memory_free(mwifi_buf);
         mwifi_buf = NULL;
         wifi_join = WICED_ERR;
         return RK_ERROR;
     }


#if 0
     if(wifi_count == 0)
     {
         rk_printf("there is no ssid in the round \n");
         rkos_memory_free(mwifi_buf);
         mwifi_buf =NULL;
         wifi_join = WICED_ERR;
         return RK_ERROR;
     }

     for(i=0; i<mwifi_buf->Totalnum-1; i++)
     {
        for(j=i+1; j<mwifi_buf->Totalnum; j++)
        {
            if(mwifi_buf->wifiinfo[i].singal_length < mwifi_buf->wifiinfo[j].singal_length)
            {
                rkos_memcpy(&mwifi, &(mwifi_buf->wifiinfo[i]), sizeof(WIFI_SAVE));
                rkos_memcpy(&(mwifi_buf->wifiinfo[i]), &mwifi_buf->wifiinfo[j], sizeof(WIFI_SAVE));
                rkos_memcpy(&(mwifi_buf->wifiinfo[j]), &mwifi, sizeof(WIFI_SAVE));
            }
        }
     }
#endif

     if(easy_setup_flag == WICED_BUSY)
     {
         rkos_memory_free(mwifi_buf);
         mwifi_buf = NULL;
         wifi_join = WICED_ERR;
         return RK_ERROR;
     }

     for(i=0; i<mwifi_buf->Totalnum; i++)
     {
        if(mwifi_buf->wifiinfo[i].scan_flag == 0)
            continue;
        rk_printf("wifi relink start");
        rkos_memset(&JoinAp, 0x00, sizeof(WIFI_AP));
        rkos_memcpy(&JoinAp, &(mwifi_buf->wifiinfo[i].wifi_ap), sizeof(WIFI_AP));

#if 1
        ret = rk_wifi_downrelink_sys(flag);
#endif
        //ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
        if(ret == RK_SUCCESS)
        {

            mwifi_buf->wifiinfo[i].Connect_flag = 1;
            rkos_memory_free(mwifi_buf);
            mwifi_buf = NULL;
            return RK_SUCCESS;
        }
     }

#if 1     //连接失败恢复之前连接的路由器connect， 下次以连接这个路由器为准
     for(i = 0; i < mwifi_buf->Totalnum; i++)
     {
         if(rkos_memcmp(connect_ssid, mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value, 32) == 1)
         {
            mwifi_buf->wifiinfo[i].Connect_flag = 1;
         }
     }
#endif

     LUNWriteDB(gSysConfig.WifiSSIDStartLBA, WIFI_LEN, mwifi_buf);
     rkos_memory_free(mwifi_buf);
     mwifi_buf = NULL;

     //wifi_link = WICED_FALSE;
    // wifi_join = WICED_ERR;
     return RK_ERROR;
}


rk_err_t rk_wifiinfo_start(void)
{
    WIFI_INFO *wifi_buf;
    int i;
    rk_err_t ret;

    wifi_join = WICED_FALSE;
    if(wifi_link == WICED_TRUE)
    {
        wifi_link = WICED_FALSE;
        wiced_network_down(WICED_STA_INTERFACE);
        //printf("wifi_link11 = %d\n", wifi_link);
     }

    wifi_buf = rkos_memory_malloc(sizeof(WIFI_INFO));
    if(NULL == wifi_buf)
    {
        //rk_printf("wifi_link55 = %d", wifi_link);
        wifi_join = WICED_ERR;
        return RK_ERROR;
    }

    LUNReadDB(gSysConfig.WifiSSIDStartLBA, WIFI_LEN, wifi_buf);

    if(wifi_buf->wifiinfoflag != 0x5a5a5a5a)
    {
        rkos_memory_free(wifi_buf);
        wifi_join = WICED_ERR;
        //rk_printf("wifi_link66 = %d", wifi_link);
        //printf("rk_wifiinfo_start there is no wifi info\n");
        return RK_ERROR;
    }

    rk_printf("wifi_buf totalnum = %d", wifi_buf->Totalnum);
    for(i=0; i<wifi_buf->Totalnum; i++)
    {
        if(wifi_buf->wifiinfo[i].Connect_flag == 1)
        {
            rkos_memset(&JoinAp, 0x00, sizeof(WIFI_AP));
            rkos_memcpy(&JoinAp, &(wifi_buf->wifiinfo[i].wifi_ap), sizeof(WIFI_AP));

            rk_printf("info ssid = %s,len = %d,password = %s,passlen = %d",JoinAp.ap_ssid_value,JoinAp.ap_ssid_len, JoinAp.security_key, JoinAp.security_key_length);
            rk_printf("info secruty = %x", JoinAp.ap_security_type);

           // printf("ssid = %s\n", wifi_link);
            wifi_join = WICED_WAIT;

            if(easy_setup_flag == WICED_BUSY)
            {
                rkos_memory_free(wifi_buf);
                wifi_join = WICED_ERR;
                return RK_ERROR;
            }

#if 0
            ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
#endif
            ret = wifi_relink_sysconfig_start();
            if(ret == WICED_SUCCESS)
            {
               // rk_printf("link22222 = %d", wifi_link);

                wifi_buf->wifiinfo[i].Connect_flag = 1;
                rkos_memory_free(wifi_buf);
                return RK_SUCCESS;
            }

        }
    }

    rkos_memory_free(wifi_buf);

    return RK_ERROR;
}
#endif



#ifdef EASY_SETUP_ENABLE
rk_err_t rk_wifi_connect_smartconfig(void)
{
    int ret;
    wwd_result_t result = 0;
    wifi_join = WICED_FALSE;
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

     easy_setup_enable_cooee(); /* broadcom cooee */
     easy_setup_enable_akiss();

    result = easy_setup_start();

    //rk_wifi_info_save(JoinAp);
    /* start easy setup */
    if (result != WWD_SUCCESS)
    {
        WPRINT_APP_INFO(("easy setup failed.\r\n"));

        if(result == WICED_ERROR)
        {
            //rkos_sleep(2000);
            //easy_setup_flag =  WICED_WAIT;
            rkos_sleep(2000);
            easy_setup_flag = WICED_FALSE;
            return RK_ERROR;
        }
        else
        {
            easy_setup_flag = WICED_FALSE;
            rkos_sleep(2000);
        }

    }
    else
    {
        WPRINT_APP_INFO(("easy setup done.\r\n"));
        easy_setup_flag = WICED_TRUE;
    }


    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
    wifi_ask.cmd = LINK_SMARTCONFIG;
    wifi_join = WICED_FALSE;
    rk_printf("wifi_relink_smartconfig = %d", LINKDOWN_SYS);
    rkos_queue_send(gWifiBackgroundData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);

    return RK_SUCCESS;
}

rk_err_t rk_easy_smartconfig_stop(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    wifi_ask.cmd  = WIFI_SMARTCONFIG_STOP;
    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;
}

rk_err_t rk_smartconfig_stop(void)
{
    easy_setup_stop_flag = 1;
    return RK_SUCCESS;
}


rk_err_t rk_wifi_smartconfig(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;
    //uint8 flag = 0;

    //easy_setup_flag = 0;

    if(easy_setup_flag == WICED_BUSY)
    {
        //rk_printf("smatconfig busy");
        return RK_ERROR;
    }

    easy_setup_flag = WICED_BUSY;
    easy_setup_stop_flag = 0;
    wifi_join = WICED_FALSE;

    do
    {
        //rk_printf("smartconfig = %d", wifi_network_up_flag);
        if(easy_setup_stop_flag == 1)
        {
             easy_setup_flag = WICED_FALSE;
             return  RK_ERROR;
        }

        rkos_sleep(100);
    }while(wifi_network_up_flag == 1);

   // if(wifi_link == WICED_TRUE)
   // {

        wifi_link = WICED_FALSE;
        wiced_network_down(WICED_STA_INTERFACE);
   // }
    rk_printf("link111&&&&&&&&&&&&& = %d", wifi_connect_flag());
    wifi_ask.cmd = WIFI_SMARTCONFIG;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;
}
#endif


pSemaphore  Scan_sem = NULL;

#ifdef _WICE_
static wiced_result_t scan_handler( wiced_scan_handler_result_t* malloced_scan_result )
#endif

#ifdef _REALTEK_
static rtw_result_t scan_handler( rtw_scan_handler_result_t* malloced_scan_result )
#endif
{

   // printf("length = %d = %s\n", malloced_scan_result->ap_details.SSID.length, malloced_scan_result->ap_details.SSID.value);

    //printf("%02x:%02x:%02x:%02x:%02x:%02x", malloced_scan_result->ap_details.BSSID.octet[0], malloced_scan_result->ap_details.BSSID.octet[1],
      //  malloced_scan_result->ap_details.BSSID.octet[2], malloced_scan_result->ap_details.BSSID.octet[3],
       // malloced_scan_result->ap_details.BSSID.octet[4], malloced_scan_result->ap_details.BSSID.octet[5]);
        //printf("result     %02x:%02x:%02x:%02x:%02x:%02x", (*result_ptr)->BSSID.octet[0], (*result_ptr)->BSSID.octet[1], (*result_ptr)->BSSID.octet[2], (*result_ptr)->BSSID.octet[3],
        //(*result_ptr)->BSSID.octet[4], (*result_ptr)->BSSID.octet[5]);


    #ifdef _WICE_
    if((malloced_scan_result->status == WICED_SCAN_COMPLETED_SUCCESSFULLY) || (malloced_scan_result->status == WICED_SCAN_ABORTED))
    #endif

    #ifdef _REALTEK_
    if (malloced_scan_result->scan_complete == RTW_TRUE)
    #endif
    {
          rkos_semaphore_give(Scan_sem);

          #ifdef _WICE_
          rkos_memory_free(malloced_scan_result);
          return WICED_SUCCESS;
          #endif

          #ifdef _REALTEK_
          return RTW_SUCCESS;
          #endif
    }
    else
    {


        if(rkos_memcmp(JoinAp.ap_ssid_value, malloced_scan_result->ap_details.SSID.value, JoinAp.ap_ssid_len) == 1)
        {
            rkos_memcpy(JoinAp.ap_bssid, malloced_scan_result->ap_details.BSSID.octet, 6);
            JoinAp.ap_security_type = malloced_scan_result->ap_details.security;
            rk_printf("find %s = %x", JoinAp.ap_ssid_value, JoinAp.ap_security_type);
        }
    }

    #ifdef _WICE_
    rkos_memory_free(malloced_scan_result);
    return WICED_SUCCESS;
    #endif

    #ifdef _REALTEK_
    return RTW_SUCCESS;
    #endif

}

rk_err_t rk_wifi_connect(WIFI_AP *mwifi_ap)
{
    int ret;

    rkos_memset(&JoinAp, 0x00, sizeof(WIFI_AP));
    JoinAp.ap_ssid_len = mwifi_ap->ap_ssid_len;
    JoinAp.security_key_length = mwifi_ap->security_key_length;
    rkos_memcpy(JoinAp.ap_ssid_value, mwifi_ap->ap_ssid_value, mwifi_ap->ap_ssid_len);
    rkos_memcpy(JoinAp.security_key, mwifi_ap->security_key, mwifi_ap->security_key_length);

    wifi_join = WIFI_FALSE;

    if(wifi_link == WIFI_TRUE)
    {
        wifi_link = WIFI_FALSE;
        #ifdef _WICE_
        wiced_network_down(WICED_STA_INTERFACE);
        #endif

        #ifdef _REALTEK_

        #endif
    }

    Scan_sem = rkos_semaphore_create(1, 0);
    if(NULL == Scan_sem)
    {
        rk_printf("scan sem create fail");
        wifi_join = 0;
        return RK_ERROR;
    }

    #ifdef _WICE_
    rk_scan_resut_handle(scan_handler, NULL);
    wiced_wifi_scan_networks(scan_handler, NULL);
    #endif

    #ifdef _REALTEK_
    wifi_scan_networks(scan_handler, NULL);
    #endif

    rkos_semaphore_take(Scan_sem, MAX_DELAY);

    wifi_join = WIFI_WAIT;

    #ifdef _WICE_
    ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
    if(ret == WICED_SUCCESS)
    #endif

    #ifdef _REALTEK_
    ret = wifi_connect((char*)JoinAp.ap_ssid_value,
    						JoinAp.ap_security_type,
    						(char*)JoinAp.security_key,
    						JoinAp.ap_ssid_len,
    						JoinAp.security_key_length,
    						0,
    						NULL);

    if(ret == RTW_SUCCESS)
    #endif

    {

        #ifdef _REALTEK_
        LwIP_DHCP(0, DHCP_START);

        {
    		rtw_wifi_setting_t setting;

    		uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
    		uint8_t *ip = LwIP_GetIP(&xnetif[0]);

    		wifi_get_setting(WLAN0_NAME,&setting);
    		wifi_show_setting(WLAN0_NAME,&setting);
    		printf("\n\r  MAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
    		printf("\n\r  IP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#if  CONFIG_START_STA_AP
    	 	mac = LwIP_GetMAC(&xnetif[1]);
    		 ip = LwIP_GetIP(&xnetif[1]);
    		wifi_get_setting(WLAN1_NAME,&setting);
    		wifi_show_setting(WLAN1_NAME,&setting);
    		printf("\n\r  MAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
    		printf("\n\r  IP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#endif
    		printf("\n\r[MEM] After WLAN Init, available heap %d\n\r", rkos_GetFreeHeapSize());

            MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
        }
        #endif


        wifi_join = WIFI_TRUE;
        rk_printf("wifi_link connec");

        wifi_link = WIFI_TRUE;
#ifdef _WIFI_FOR_SYSTEM_
        rk_wifi_info_save(JoinAp);
#endif
        rkos_semaphore_delete(Scan_sem);
        return RK_SUCCESS;
    }

    rk_printf("wifi connect fail");
    wifi_join = WIFI_ERR;
    rkos_semaphore_delete(Scan_sem);
    return RK_ERROR;

}


#ifdef _WICE_
xSemaphoreHandle   Wifi5GScan_sem = NULL;
static wiced_result_t WIfi5G_scan_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    WIFI_INFO *wifi_info = (WIFI_INFO *)malloced_scan_result->user_data;
    int i=0;


    if((malloced_scan_result->status == WICED_SCAN_COMPLETED_SUCCESSFULLY) || (malloced_scan_result->status == WICED_SCAN_ABORTED))
    {
          rk_printf("wifi5G_scan_handle end");
          rkos_memory_free(malloced_scan_result);
          rkos_semaphore_give(Wifi5GScan_sem);
          return WICED_SUCCESS;
    }

    if(malloced_scan_result->ap_details.SSID.length == 0)
    {
         rkos_memory_free(malloced_scan_result);
         return WICED_SUCCESS;
    }

    if(wifi_info == NULL)
    {
        rkos_memory_free(malloced_scan_result);
        return WICED_SUCCESS;
    }

    for(i=0; i< wifi_info->Totalnum; i++)
    {
        if(rkos_memcmp(wifi_info->wifiinfo[i].wifi_ap.ap_ssid_value, malloced_scan_result->ap_details.SSID.value, 32) == 1)
        {
            rk_printf("length&&&&&&&&&&& = %d = %s = %d", malloced_scan_result->ap_details.SSID.length, malloced_scan_result->ap_details.SSID.value,malloced_scan_result->ap_details.channel);
            rk_printf("wifiinfo = %d = %s", wifi_info->wifiinfo[i].wifi_ap.ap_ssid_len, wifi_info->wifiinfo[i].wifi_ap.ap_ssid_value);
            rkos_memcpy(wifi_info->wifiinfo[i].wifi_ap.ap_bssid, malloced_scan_result->ap_details.BSSID.octet, 6);
            wifi_info->wifiinfo[i].wifi_ap.ap_security_type = malloced_scan_result->ap_details.security;
            wifi_info->wifiinfo[i].scan_flag = 1;
            rkos_memory_free(malloced_scan_result);
            return WICED_SUCCESS;
        }

    }

    rkos_memory_free(malloced_scan_result);
    return WICED_SUCCESS;
}



rk_err_t rk_wifi_scan_5G(void *data)
{
    int ret;

    Wifi5GScan_sem = rkos_semaphore_create(1, 0);
    if(NULL == Wifi5GScan_sem)
    {
        rk_printf("Wifi5GScan_sem create fail");
        return RK_ERROR;
    }

    rk_scan_resut_handle(WIfi5G_scan_handler, data);
    wiced_wifi_scan_networks(WIfi5G_scan_handler, data);
    rkos_semaphore_take(Wifi5GScan_sem, MAX_DELAY);
     rk_printf("rk_wifi_scan_5G end");
    rkos_semaphore_delete(Wifi5GScan_sem);
    return RK_SUCCESS;
}




void rkwifi_get_ssid(uint8 *mssid_value, int *mssid_len)
{
    rkos_memcpy(mssid_value, JoinAp.ap_ssid_value, JoinAp.ap_ssid_len);
    *mssid_len = JoinAp.ap_ssid_len;
    return;
}

void get_strength_easy_scan_result_callback(wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status)
{

    WIFI_CURRENT_MESSAGE *ws = (WIFI_CURRENT_MESSAGE *)user_data;
   // printf("status = %d, \n",status);
    //rkos_semaphore_give(&ws->scan_complete_sem);
    //return;
    //printf("length = %d\n", (*result_ptr)->SSID.length);

    rk_printf("ws = %02x:%02x:%02x:%02x:%02x:%02x", ws->octect[0], ws->octect[1], ws->octect[2], ws->octect[3],
        ws->octect[4], ws->octect[5]);
    rk_printf("result     %02x:%02x:%02x:%02x:%02x:%02x", (*result_ptr)->BSSID.octet[0], (*result_ptr)->BSSID.octet[1], (*result_ptr)->BSSID.octet[2], (*result_ptr)->BSSID.octet[3],
        (*result_ptr)->BSSID.octet[4], (*result_ptr)->BSSID.octet[5]);
#if 1
    if(status == WICED_SCAN_ABORTED ||status == WICED_SCAN_COMPLETED_SUCCESSFULLY)
    {
         rk_printf("get strenght ws->scan_complete = 0x%06x", ws->scan_complete_sem);
        // rkos_semaphore_give(ws->scan_complete_sem);
         //printf("get strenght ws->scan_complete end\n");
         return;
    }
#endif
    if( ws->abort_scan == WICED_TRUE)
    {
        return;
    }

    if (NULL == result_ptr)
    {
        rkos_semaphore_give(ws->scan_complete_sem);
        return;
    }
    else
    {
        /* Only look for a match on our locked BSSID */
        if (memcmp(ws->octect, (*result_ptr)->BSSID.octet, 6) == 0)
        {
            rk_printf("update strength");
            ws->abort_scan = WICED_TRUE;
            ws->channel = (*result_ptr)->channel;
            ws->strength = (*result_ptr)->signal_strength;
            //rkos_semaphore_give(ws->scan_complete_sem);
        }
    }

}

int rkwifi_get_strength(char *mssid_value, int mssid_len, uint8* bssid_value, int *strength)
{

    wiced_mac_t bogus_scan_mac;
    wiced_ssid_t ssid;
    WIFI_CURRENT_MESSAGE *ws;
    uint8 retry_times;
    wiced_scan_extended_params_t extparam = { 5, 110, 110, 50 };
    // const uint16_t chlist[] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,153, 155, 0 };
   // printf("rkwifi_get_strength bssid_value\n");

    ws = rkos_memory_malloc(sizeof(WIFI_CURRENT_MESSAGE));
    if(ws == NULL)
    {
        rk_printf("rkwifi_get_strength rkos_malloc fail");
        return RK_EMPTY;
    }

    ws->scan_complete_sem = rkos_semaphore_create(1, 0);
    if(ws->scan_complete_sem == NULL)
    {
        rk_printf("wifi_get_strengh ws->scancomplete_sem create fail");
        return RK_EMPTY;
    }

    ssid.length = mssid_len;
    rkos_memcpy(ssid.value, mssid_value, ssid.length);
    rkos_memcpy(bogus_scan_mac.octet, bssid_value, 6);
    rkos_memcpy(ws->octect, bssid_value, 6);
    ws->abort_scan = WICED_FALSE;

    rk_printf("mssid_value = %s, mac = %02x:%02x:%02x:%02x:%02x:%02x", mssid_value, bogus_scan_mac.octet[0],
        bogus_scan_mac.octet[1], bogus_scan_mac.octet[2], bogus_scan_mac.octet[3],
        bogus_scan_mac.octet[4], bogus_scan_mac.octet[5]);
   // printf("get strength wwd_scan start\n");
    rk_result_ptr = &rk_scan_result[0];
    retry_times = 3;
    do {

       // wwd_wifi_scan( WICED_SCAN_TYPE_PASSIVE, WICED_BSS_TYPE_INFRASTRUCTURE, &ssid, &bogus_scan_mac, NULL, &extparam, easy_scan_result_callback, (wiced_scan_result_t **) &result_ptr, ws, WWD_STA_INTERFACE  );
        wwd_wifi_scan(WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, &ssid, &bogus_scan_mac, NULL, &extparam, get_strength_easy_scan_result_callback, (wiced_scan_result_t **) &rk_result_ptr, ws, WWD_STA_INTERFACE  );
        rkos_semaphore_take( ws->scan_complete_sem, 2000);
        retry_times--;
    } while ((ws->channel == 0) && (retry_times > 0));

    //printf("get strength wwd_scan_end\n");
   // rkos_semaphore_take( &ws->scan_complete_sem, MAX_DELAY);
    if(ws->abort_scan == WICED_FALSE)
    {
        rk_printf("there is no this wifi ssid");
        rkos_semaphore_delete(ws->scan_complete_sem);
        rkos_memory_free( ws );
        *strength = -1000;
        return RK_ERROR;
    }

    *strength = ws->strength;
   // printf("");
    rkos_semaphore_delete(ws->scan_complete_sem);
    rkos_memory_free( ws );

    return RK_SUCCESS;
}


int rk_update_strength(int *strength)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    wifi_ask.cmd = UPDATE_STRENGTH;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
    if(wifi_resp.cmd == RK_SUCCESS)
    {
        *strength = wifi_resp.signal_strength;
        return RK_SUCCESS;
    }

    return RK_ERROR;
}


rk_err_t rk_wifi_deinit(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    wifi_ask.cmd = WIFI_DEINIT;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);

    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;
}

rk_err_t rk_wifi_scan(void* mscan_handle)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    if((wifi_scan_doing == 1) || (wifi_join == WICED_WAIT))
    {
        return RK_SUCCESS;
    }

    wifi_ask.cmd = WIFI_SCAN;
    wifi_ask.scan_hand = mscan_handle;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;
}

void rk_wifi_scan_stop(void)
{
    if(wifi_scan_doing == 1)
    {
       wifi_set_scan_handle = 1;
       while(wifi_set_scan_handle != 2)
       {
           rkos_sleep(10);
           if(wifi_scan_doing == 0)
           {
                break;
           }
       }
    }
}

int wifi_join_flag(void)
{
    return wifi_join;
}

int wifi_connect_flag(void)
{
    return wifi_link;
}

int wifi_init_flag(void)
{
    return wiced_wlan_initialised;
}

int wifi_easy_setup_flag(void)
{
    return easy_setup_flag;
}




rk_err_t rk_wifi_downrelink_sys(uint8 count_flag)
{
    unsigned int             a;
    rk_err_t ret;
    int                      retries;
    wiced_result_t  result = WICED_ERROR;
    wiced_result_t      join_result = WICED_ERROR;
    wiced_ssid_t WLAN0 = {0};
    WLAN0.length = JoinAp.ap_ssid_len;
    //uint8 flag = 0;

    wifi_network_up_flag = 1;

    if(JoinAp.ap_ssid_len == 0)
    {
        wifi_join = WICED_ERR;
        return RK_ERROR;
    }


    wifi_join = WICED_WAIT;

    rkos_memcpy(WLAN0.value, JoinAp.ap_ssid_value, WLAN0.length);
    rk_printf("relink:ssid = %s,len = %d, password = %s,passlen = %d",JoinAp.ap_ssid_value,JoinAp.ap_ssid_len, JoinAp.security_key, JoinAp.security_key_length);
    rk_printf("relink:secruty = %x", JoinAp.ap_security_type);


   // wifi_link = WICED_FALSE;
   // wiced_network_down(WICED_STA_INTERFACE);

    rkos_sleep(10);
    for(retries = 0; retries <WICED_JOIN_RETRY_ATTEMPTS; retries++)
    {
       /* If join-specific failed, try scan and join AP */
       WPRINT_NETWORK_INFO(("wwd_wifi_join start\r\n"));

       if(easy_setup_flag == WICED_BUSY)
       {

            wifi_network_up_flag = 0;
            return RK_ERROR;
       }

       if(JoinAp.ap_security_type == WICED_SECURITY_OPEN)
       {
           join_result = wwd_wifi_join((wiced_ssid_t *)&WLAN0, WICED_SECURITY_OPEN, NULL, 0, NULL );
       }
       else
       {
           join_result = wwd_wifi_join((wiced_ssid_t *)&WLAN0, JoinAp.ap_security_type, JoinAp.security_key, JoinAp.security_key_length, NULL );
       }

       if ( join_result == WICED_SUCCESS )
       {
           link_up();

           wwd_management_set_event_handler( link_events, wiced_link_events_handler, NULL, WWD_STA_INTERFACE );

#if 1
           if(easy_setup_flag == WICED_BUSY)
           {
                wifi_network_up_flag = 0;
                return RK_ERROR;
           }
#endif
           rk_printf("wiced_ip up");
           result = wiced_ip_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
           if(result == WICED_SUCCESS)
           {
                rk_printf("wiced_ip up ok");
                wifi_link = WICED_TRUE;
                wifi_join = WICED_TRUE;
                wiced_sta_link_up = WICED_TRUE;
                wifi_network_up_flag = 0;
                #ifdef _WIFI_FOR_SYSTEM_
                rk_wifi_info_save(JoinAp);
                #endif

                return RK_SUCCESS;
           }
           rk_printf("wiced_ip up err");
       }
//#ifndef __WEB_CHANNELS_RK_FOCHANNEL_C__
       if(count_flag == 1)
       {
            break;
       }
//#endif
       //
    }
   // wifi_connect = WICED_FALSE;
    wifi_link = WICED_FALSE;
    wifi_join = WICED_ERR;
    wifi_network_up_flag = 0;

    return RK_ERROR;

}
#endif

rk_err_t rk_wifi_usartconfig(uint8 *ssid_value, uint8 ssid_length, uint8 *password_value, uint8 password_length)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    wifi_ask.cmd = WIFI_USARTCONFIG;
    wifi_ask.join_ap.ap_ssid_len = ssid_length;
    wifi_ask.join_ap.security_key_length = password_length;
    rkos_memcpy(wifi_ask.join_ap.ap_ssid_value, ssid_value, ssid_length);
    rkos_memcpy(wifi_ask.join_ap.security_key, password_value, password_length);

   /* if(wifi_link != WICED_TRUE)
    {
        wifi_link = WICED_FALSE;
    }*/
    wifi_join = WIFI_FALSE;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;

}


COMMON API rk_err_t wifi_connectTask_Suspend(HTC hTask, uint32 Level)
{
    RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pShellTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pShellTask->State = TASK_STATE_IDLE2;
    }
    return RK_SUCCESS;
}

COMMON API rk_err_t wifi_connectTask_Resume(HTC hTask)
{
   RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;
   pShellTask->State = TASK_STATE_WORKING;
   return RK_SUCCESS;
}

COMMON API rk_err_t wifi_connectTask_CheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(tcpip_resume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}
extern WIFI_AP  easy_join_ap;
void wifi_connectTask_Enter(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;
    rk_err_t ret = 0;
    int flag = 0;
    HTC hSelf;

    hSelf = RKTaskGetRunHandle();

		#ifdef _WICE_
    while(wiced_wlan_initialised == WICED_FALSE|| wiced_wlan_initialised == WICED_ERR)
    {
        rkos_sleep(10);
    }

    while(1)
    {
        memset(&wifi_ask, 0, sizeof(WIFI_ASK_QUEUE));

        rkos_queue_receive(gWifiBackgroundData->WIFIAPPAskQueue, &wifi_ask, 100);

        if(wifi_connectTask_CheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

       // rk_printf("conect cmd = %d",  wifi_ask.cmd);
        if(easy_setup_flag == WICED_BUSY)
        {
            continue;
        }

        if(wifi_ask.cmd == LINKDOWN_SYS || wifi_ask.cmd == LINK_SMARTCONFIG)
        {
            if(wifi_link == WICED_TRUE)
            {
                wifi_link = WICED_FALSE;
                flag = 1;
            }

            while(wifi_scan_doing==1)  //wait scan end
            {
                rkos_sleep(10);
            }
            if(flag == 1)
            {
                wiced_network_down(WICED_STA_INTERFACE);
                flag = 0;
            }
            if(wifi_ask.cmd == LINK_SMARTCONFIG)
            {
                //rk_printf("aa");
                rkos_memcpy(&JoinAp, &easy_join_ap, sizeof(WIFI_AP));
                //rk_printf("nn");
            }

            MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 1);
            rk_wifi_downrelink_sys(0);
            MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 0);
            if(wifi_link == WICED_TRUE)
            {
                rk_printf("wifi ok");
                MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
            }

        }

        if(wifi_link == WICED_TRUE || wifi_scan_doing==1)
        {
            continue;
        }

        switch(wifi_ask.cmd)
        {
            case  LINKSTART_SYS:

                MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 1);
                ret = rk_wifi_downrelink_sys(0);
                MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 0);
                if(wifi_link == WICED_TRUE)
                {
                    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
                }
                wifi_resp.cmd = ret;
                rkos_memset(&wifi_ask, 0, sizeof(WIFI_ASK_QUEUE));
                rkos_queue_send(gWifiBackgroundData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                continue;

        }

        if(easy_setup_flag == WICED_BUSY || wifi_link == WICED_TRUE ||wifi_scan_doing == 1)
        {
            continue;
        }
        else
        {
        //#ifndef __WEB_CHANNELS_RK_FOCHANNEL_C__
            MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 1);
            rk_wifi_relink_sysinfo(1);
            MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 0);
            if(wifi_link == WICED_TRUE)
            {
                MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
            }
            else
            {
                MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
            }
        //#endif
            //rk_printf("rk_wifi_relink_sysinfo");
        }
    }
		#endif

}

#ifdef _WICE_
rk_err_t wifi_relink_sysconfig(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);

    wifi_ask.cmd = LINKDOWN_SYS;
    wifi_join = WICED_FALSE;
    rk_printf("wifi_relink_sysconfig = %d", LINKDOWN_SYS);
    rkos_queue_send(gWifiBackgroundData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
   // rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return RK_SUCCESS;
}


rk_err_t wifi_relink_sysconfig_start(void)
{
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;

    wifi_ask.cmd = LINKSTART_SYS;
    wifi_join = WICED_FALSE;

    rkos_queue_send(gWifiBackgroundData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiBackgroundData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
    if(wifi_resp.cmd == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }

}
#endif

COMMON API rk_err_t wifi_applicationTask_Suspend(HTC hTask, uint32 Level)
{
    RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pShellTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pShellTask->State = TASK_STATE_IDLE2;
    }
    return RK_SUCCESS;
}

COMMON API rk_err_t wifi_applicationTask_Resume(HTC hTask)
{
   RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;
   pShellTask->State = TASK_STATE_WORKING;
   return RK_SUCCESS;
}

COMMON API rk_err_t wifi_applicationTask_CheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(tcpip_resume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: wifi_applicationTask_Resume
** Input:void
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.27
** Time: 8:54:16
*******************************************************************************/

#define WICED_IP4_ADDR(ipaddr, a,b,c,d) \
        (ipaddr)->addr = ((u32_t)((a) & 0xff) << 24) | \
                         ((u32_t)((b) & 0xff) << 16) | \
                         ((u32_t)((c) & 0xff) << 8)  | \
                          (u32_t)((d) & 0xff)


COMMON API void wifi_applicationTask_Enter(void)
{
    //FW_LoadSegment(SEGMENT_ID_AP6181_INIT, SEGMENT_OVERLAY_ALL);
#if 1
    WIFI_ASK_QUEUE wifi_ask;
    WIFI_RESP_QUEUE wifi_resp;
    rk_err_t ret = RK_ERROR;
    int ret1;
    int ssid_len;

    #ifdef _WICE_
    wiced_result_t              result;
    #endif

    HTC WIFI_CONNECTTASK_HANDLE = NULL;
    HTC hSelf;

    rk_printf("wifi_applicationTask_Enter");


    hSelf = RKTaskGetRunHandle();



#if 0
    *(uint32 *)wifi_buf = 0x55aa55aa;

    LUNWriteDB(gSysConfig.WifiSSIDStartLBA, 1, wifi_buf);
    printf("wifi_buf = %x", *(uint32 *)wifi_buf);

    *(uint32 *)wifi_buf = 0;


    LUNReadDB(gSysConfig.WifiSSIDStartLBA, 1, wifi_buf);
    printf("wifi_buf = %x", *(uint32 *)wifi_buf);
#endif

    if (DeviceTask_CreateDeviceList(DEVICE_LIST_SDIO, NULL, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("sdio  device list create fail");
         while(1);
    }

    #ifdef _WICE_
    #ifdef RK_WIFI_AP
    if(gWifiAppData->WifiMode == WLAN_MODE_AP)
    {
        FREQ_EnterModule(FREQ_MAX);

        result = wiced_init();

        wiced_ip_setting_t ip_settings;
        struct ip_addr ipaddr;
        struct ip_addr netmask;
        struct ip_addr gw;
        WICED_IP4_ADDR(&ipaddr, 192, 168, 237, 1);
        WICED_IP4_ADDR(&netmask, 255, 255, 255, 0);
        WICED_IP4_ADDR(&gw, 192, 168, 237, 1);
        memset(&ip_settings, 0 , sizeof(wiced_ip_setting_t));
        ip_settings.ip_address.ip.v4 = ipaddr.addr;
        ip_settings.gateway.ip.v4 = gw.addr;
        ip_settings.netmask.ip.v4 = netmask.addr;
        ret = wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ip_settings);

        MainTask_SetStatus(MAINTASK_WIFI_AP_OPEN_OK, 1);

        FREQ_ExitModule(FREQ_MAX);

        wiced_wlan_initialised = WICED_TRUE;

    }
    else
    #endif
    {
        WIFI_CONNECTTASK_HANDLE = RKTaskCreate2(wifi_connectTask_Enter, wifi_connectTask_Suspend, wifi_connectTask_Resume, "wificonnect", 512, 10, NULL);
        if( WIFI_CONNECTTASK_HANDLE == (HTC)RK_ERROR)
        {
            rk_printf("wifi connect task create fail");
        }

        FREQ_EnterModule(FREQ_MAX);


        result = wiced_init();

        MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 1);

        rk_printf("wiced_wlan_initialised = %d", wiced_wlan_initialised);
        if(result == WICED_SUCCESS)
        {
            wiced_wlan_initialised = WICED_TRUE;

#ifdef _WIFI_FOR_SYSTEM_
            rk_wifiinfo_start();
#endif
        }
        else
        {
             wiced_wlan_initialised = WICED_ERR;
             rk_printf("wifi init fail");
             while(1)
             {
                 rkos_sleep(100);
             }
        }

        FREQ_ExitModule(FREQ_MAX);
    }
    #endif

    #ifdef _REALTEK_
    {
#if defined (WIFI_USE_SDIO)
    	/* SDIO bus scan should start after main */
    	/* beucase some Scheduler methord used */
    	rtw_sdio_bus_ops.bus_probe();
#endif

#if CONFIG_INIT_NET
    	/* Initilaize the LwIP stack */
    	LwIP_Init();
    	wifi_manager_init();
    	/* Kill init thread after all init tasks done */
#if CONFIG_WLAN && !CONFIG_START_MP
    	{
    		char start_sta_mode = 0,start_ap_mode = 0;
#if CONFIG_START_AP
    		struct ip_addr ipaddr;
    		struct ip_addr netmask;
    		struct ip_addr gw;
    		struct netif * pnetif = &xnetif[0];
    		IP4_ADDR(&ipaddr, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    		IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
    		IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
    		netif_set_addr(pnetif, &ipaddr, &netmask,&gw);
    		start_ap_mode = 1;
#ifdef CONFIG_DONT_CARE_TP
    		pnetif->flags |= NETIF_FLAG_IPSWITCH; //add for Ping OK
#endif
#ifdef CONFIG_CONCURRENT_MODE
    		wifi_on(RTW_MODE_AP);
#endif
#elif CONFIG_START_STA
    		start_sta_mode = 1;
#ifdef CONFIG_CONCURRENT_MODE
    		wifi_on(RTW_MODE_STA);
#endif
#elif CONFIG_START_STA_AP
    		start_sta_mode = 1;
    		start_ap_mode = 1;
    		wifi_on(RTW_MODE_STA_AP);
#endif

    		if( start_ap_mode) {
#if CONFIG_WPA2
    			wifi_start_ap(AP_MODE_SSID,
    							RTW_SECURITY_WPA2_AES_PSK,
    							WPA_PASSPHRASE,
    							strlen(AP_MODE_SSID),
    							strlen(WPA_PASSPHRASE),
    							AP_DEFAULT_CH);
#else //Open
    			wifi_start_ap(AP_MODE_SSID,
    							RTW_SECURITY_OPEN,
    							NULL,
    							strlen(AP_MODE_SSID),
    							0,
    							AP_DEFAULT_CH);
#endif

#if CONFIG_DHCP_SERVER
    	#if CONFIG_START_STA_AP
    			/* Start DHCP Server */
    			dhcps_init(&xnetif[1]);
    	#else
    			/* Start DHCP Server */
    			dhcps_init(&xnetif[0]);
    	#endif //CONFIG_START_STA_AP
#endif
#if CONFIG_WEB_SERVER
    			start_web_server();
#endif
    		}

    		if((start_sta_mode == 0) && (start_ap_mode == 0))
    		{
    			wifi_on(RTW_MODE_STA);
                start_sta_mode = 1;
    		}

            MainTask_SetStatus(MAINTASK_WIFI_AP_OPEN_OK, 1);

            if(start_sta_mode)
    		{
    			rtw_network_info_t wifi = {0};
    			int ret;
#if 0//def CONFIG_AUTO_RECONNECT
    			int mode = 0;
#endif

#if CONFIG_WEP
    			wifi.security_type = RTW_SECURITY_WEP_PSK;
    			wifi.password = WEP40_KEY;
    			wifi.password_len = strlen(WEP40_KEY);
    			wifi.key_id = 0;
#elif CONFIG_WPA
    			wifi.security_type = RTW_SECURITY_WPA_AES_PSK;
    			wifi.password = WPA_PASSPHRASE;
    			wifi.password_len = strlen(WPA_PASSPHRASE);
#elif CONFIG_WPA2
    			wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
    			wifi.password = WPA_PASSPHRASE;
    			wifi.password_len = strlen(WPA_PASSPHRASE);
#else // open
    			wifi.security_type = RTW_SECURITY_OPEN;
#endif
    			strcpy((char*)wifi.ssid.value, STA_MODE_SSID);
    			wifi.ssid.len = strlen(STA_MODE_SSID);

#if 0//def CONFIG_AUTO_RECONNECT
    			//setup reconnection flag
    			mode = 1;
    			wifi_set_autoreconnect(mode);
#endif
    	 		ret = wifi_connect((char*)wifi.ssid.value,
    						wifi.security_type,
    						(char*)wifi.password,
    						wifi.ssid.len,
    						wifi.password_len,
    						wifi.key_id,
    						NULL);
#if CONFIG_DHCP_CLIENT
    			if(ret == RTW_SUCCESS) {
    				/* Start DHCPClient */
    				LwIP_DHCP(0, DHCP_START);
    			}
#endif

                {
            		rtw_wifi_setting_t setting;

            		uint8_t *mac = LwIP_GetMAC(&xnetif[0]);
            		uint8_t *ip = LwIP_GetIP(&xnetif[0]);

            		wifi_get_setting(WLAN0_NAME,&setting);
            		wifi_show_setting(WLAN0_NAME,&setting);
            		printf("\n\r  MAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
            		printf("\n\r  IP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#if  CONFIG_START_STA_AP
            	 	mac = LwIP_GetMAC(&xnetif[1]);
            		 ip = LwIP_GetIP(&xnetif[1]);
            		wifi_get_setting(WLAN1_NAME,&setting);
            		wifi_show_setting(WLAN1_NAME,&setting);
            		printf("\n\r  MAC => %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]) ;
            		printf("\n\r  IP  => %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#endif
            		printf("\n\r[MEM] After WLAN Init, available heap %d\n\r", rkos_GetFreeHeapSize());

                    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
                }

    		}
        }

#elif !CONFIG_WLAN && CONFIG_DHCP_CLIENT
    	/* Start DHCPClient */
    	LwIP_DHCP(0);
#endif	//#if CONFIG_WLAN && !CONFIG_START_MP
#endif	//#if CONFIG_INIT_NET

#if CONFIG_INTERACTIVE_MODE
    	start_interactive_mode();
    	log_service_init();
#endif

#if CONFIG_POST_INIT
#if CONFIG_START_AP
    	post_init(RTW_MODE_AP);
#elif CONFIG_START_STA_AP
    	post_init(RTW_MODE_STA_AP);
#else
    	post_init(RTW_MODE_STA);
#endif
#endif

    }
    #endif

    while(1)
    {
        //rk_printf();
        rkos_queue_receive(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);

        if(wifi_applicationTask_CheckIdle(hSelf) != RK_SUCCESS)
        {
            continue;
        }

        switch(wifi_ask.cmd)
        {
           case WIFI_DEINIT:
                #ifdef _WICE_
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = wiced_network_down(WICED_AP_INTERFACE);
                }
                else
                #endif
                {
                    if(WIFI_CONNECTTASK_HANDLE != NULL)
                    {
                        RKTaskDelete2(WIFI_CONNECTTASK_HANDLE);
                    }
                    #if 0
                    if(mwifi_buf != NULL)
                    {
                        rkos_memory_free(mwifi_buf);
                        mwifi_buf = NULL;
                    }
                    #endif
                }

                ret = wiced_deinit();
                DeviceTask_DeleteDeviceList(DEVICE_LIST_SDIO, NULL, SYNC_MODE);

                if(ret == WICED_SUCCESS)
                {
                    wifi_resp.cmd = RK_SUCCESS;
                }
                else
                {
                    wifi_resp.cmd = RK_ERROR;
                }
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                #endif
                break;

            case WIFI_SCAN:
                #ifdef _WICE_
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = WICED_SUCCESS;
                }
                else
                #endif
                {
                    rk_scan_resut_handle(wifi_ask.scan_hand, NULL);
                    ret = wiced_wifi_scan_networks(wifi_ask.scan_hand, NULL);
                }

                if(ret == WICED_SUCCESS)
                {
                    wifi_resp.cmd = RK_SUCCESS;
                }
                else
                {
                    wifi_resp.cmd = RK_ERROR;
                }
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                #endif
                break;

            case WIFI_SMARTCONFIG:
               #ifdef _WICE_
               #ifdef RK_WIFI_AP
               if(gWifiAppData->WifiMode == WLAN_MODE_AP)
               {
                   ret = WICED_SUCCESS;
               }
               else
               #endif
               {
                   ret = RK_ERROR;
                #ifdef EASY_SETUP_ENABLE
                   MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
                   ret = wiced_smartconfig_event(WICED_NETWORKING_WORKER_THREAD, rk_wifi_connect_smartconfig,  NULL);
                #endif
                }
                wifi_resp.cmd = ret;
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                #endif
                break;

            case WIFI_SMARTCONFIG_STOP:
                #ifdef _WICE_
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = WICED_SUCCESS;
                }
                else
                #endif
                {
                    ret = rk_smartconfig_stop();
                }
                wifi_resp.cmd = ret;
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                #endif
                break;

            case WIFI_USARTCONFIG:
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = WICED_SUCCESS;
                }
                else
                #endif
                {
                   // ret = wiced_smartconfig_event(WICED_NETWORKING_WORKER_THREAD, rk_wifi_connect_usart,  &(wifi_ask.join_ap));
                    MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 1);
                    ret = rk_wifi_connect(&(wifi_ask.join_ap));
                    MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 0);
                }

                if(ret == RK_SUCCESS)
                {
                    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 1);
                }
                else
                {
                    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
                }
                wifi_resp.cmd = ret;
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                break;

            case UPDATE_STRENGTH:
                #ifdef _WICE_
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = WICED_SUCCESS;
                }
                else
                #endif
                {
                    ret = rkwifi_get_strength(JoinAp.ap_ssid_value, JoinAp.ap_ssid_len, JoinAp.ap_bssid, &wifi_resp.signal_strength);
                }
                wifi_resp.cmd = ret;
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);
                #endif
                break;
             default:
                rk_printf("wifi app error cmd \n");
                break;
        }


    }
#endif


}

/*******************************************************************************
** Name: wifi_applicationTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.27
** Time: 8:54:16
*******************************************************************************/

_WICE_WIFITHREAD_INIT_
INIT API rk_err_t wifi_applicationTask_DeInit(void *pvParameters)
{
    MainTask_SetStatus(MAINTASK_WIFI_OPEN_OK, 0);
    MainTask_SetStatus(MAINTASK_WIFI_CONNECTING, 0);
    MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
    MainTask_SetStatus(MAINTASK_WIFI_AP_OPEN_OK, 0);
    MainTask_SetStatus(MAINTASK_WIFI_AP_CONNECT_OK, 0);
    rkos_queue_delete(gWifiAppData->WIFIAPPAskQueue);
    rkos_queue_delete(gWifiAppData->WIFIAPPRespQueue);
    rkos_queue_delete(gWifiBackgroundData->WIFIAPPAskQueue);
    rkos_queue_delete(gWifiBackgroundData->WIFIAPPRespQueue);

    rkos_memory_free(gWifiBackgroundData);
    rkos_memory_free(gWifiAppData);
    #ifdef _WICE_
    FW_RemoveSegment(SEGMENT_ID_AP6181_WIFI);
    #endif

    #ifdef _REALTEK_
    FW_RemoveSegment(SEGMENT_ID_REALTEK_WIFI);
    FW_RemoveSegment(SEGMENT_ID_REALTEK_WIFI2);
    FW_RemoveSegment(SEGMENT_ID_REALTEK_WIFI3);
    #endif

    FW_RemoveSegment(SEGMENT_ID_LWIP);
#ifdef __SSL_MbedTLS__
    FW_RemoveSegment(SEGMENT_ID_MBEDTLS);
#endif


    FREQ_ExitModule(FREQ_WIFI);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: wifi_applicationTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.27
** Time: 8:54:16
*******************************************************************************/
_WICE_WIFITHREAD_INIT_
INIT API rk_err_t wifi_applicationTask_Init(void *pvParameters, void *arg)
{

    WIFIAPP_TASK_DATA_BLOCK *mWifiAppData = NULL;
    WIFIAPP_TASK_DATA_BLOCK *mWifiBackgroundData = NULL;

    mWifiAppData = rkos_memory_malloc(sizeof(WIFIAPP_TASK_DATA_BLOCK));
    if(NULL == mWifiAppData)
    {
        rk_printf("mwifiappdata malloc fail\n");
        return RK_ERROR;
    }

    mWifiBackgroundData = rkos_memory_malloc(sizeof(WIFIAPP_TASK_DATA_BLOCK));
    if(NULL == mWifiBackgroundData)
    {
        rk_printf("mwifibackgrouddta malloc fail");
        rkos_memory_free(mWifiAppData);
    }

    FREQ_EnterModule(FREQ_WIFI);
    mWifiAppData->WifiMode = (uint32)arg;
    mWifiAppData->WIFIAPPAskQueue = rkos_queue_create(1, sizeof(WIFI_ASK_QUEUE));
    mWifiAppData->WIFIAPPRespQueue = rkos_queue_create(1, sizeof(WIFI_RESP_QUEUE));
    mWifiBackgroundData->WIFIAPPAskQueue = rkos_queue_create(5, sizeof(WIFI_ASK_QUEUE));
    mWifiBackgroundData->WIFIAPPRespQueue = rkos_queue_create(1, sizeof(WIFI_RESP_QUEUE));

    #ifdef _WICE_
    FW_LoadSegment(SEGMENT_ID_AP6181_WIFI, SEGMENT_OVERLAY_ALL);
    #endif

    #ifdef _REALTEK_
    FW_LoadSegment(SEGMENT_ID_REALTEK_WIFI, SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_REALTEK_WIFI2, SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_REALTEK_WIFI3, SEGMENT_OVERLAY_ALL);
    #endif

    FW_LoadSegment(SEGMENT_ID_LWIP, SEGMENT_OVERLAY_ALL);

#ifdef __SSL_MbedTLS__
    FW_LoadSegment(SEGMENT_ID_MBEDTLS, SEGMENT_OVERLAY_ALL);
#endif

    gWifiAppData = mWifiAppData;
    gWifiBackgroundData = mWifiBackgroundData;

    return RK_SUCCESS;
}

#ifdef _WICE_
void rk_wifi5_restart(void)
{
     wifi_link= WICED_FALSE;
     wiced_network_down(WICED_STA_INTERFACE);
     wifi_relink_sysconfig();
}
#endif


rk_err_t wifi_shell_start(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_delete(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_configst(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_configsp(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_recv(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_send(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_scan(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_connect(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_tsf(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_mp(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_tx(HDC dev, uint8 * pstr);
rk_err_t wifi_shell_rx(HDC dev, uint8 * pstr);

_WICE_WIFITHREAD_SHELL_
static SHELL_CMD ShellWifiName[] =
{
    "start",wifi_shell_start,"start wifi controller","wifi.start <ap | sta>",
    "delete",wifi_shell_delete,"delete wifi controller","wifi.delete",
    "configst",wifi_shell_configst,"recive ssid from phone","wifi.configst",
    "configsp",wifi_shell_configsp,"stop revice ssid","wifi.configsp",
    "recv",wifi_shell_recv,"create or stop a recv connect on the lwip use udp or tcp","wifi.recv <udp | tcp | stop>",
    "send",wifi_shell_send,"create or stop a send connect on the lwip use udp or tcp","wifi.send <udp | tcp | stop> <ipaddr>",
    "scan",wifi_shell_scan,"scan ssid","wifi.scan",
    "connect",wifi_shell_connect,"connect ssid","wifi.connect <ssid> <password>",
    "tsf",wifi_shell_tsf,"get wlan tsf value","wifi.tsf <number> --- get number tsf value",
    "mp",wifi_shell_mp,"mp cmd package","test wifi module rx and tx",
    "\b",NULL,"NULL","NULL",
};


_WICE_WIFITHREAD_SHELL_
static SHELL_CMD ShellMpName[] =
{
    "tx",wifi_shell_tx,"test wifi module tx funciton","wifi.mp.tx </cs | /csp | /ctp | /st> <time> <rate> [count]\r\n\t/cs ---continuous tx testing.\r\n\t/csp ---continuous packet tx testing.\r\n\t/ctp --- count packet tx test.if first parameter is /ctp, the count is valib.\r\n\t/st --- single tone tx test.",
    "rx",wifi_shell_rx,"test wifi module rx","wifi.mp3.rx --- air rx test",
    "\b",NULL,"NULL","NULL",
};



_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_mp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellMpName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellMpName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellMpName[i].CmdDes, pItem);
    if(ShellMpName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellMpName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;


}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_tx(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;
    uint32 time;

    if(ShellHelpDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != ' '))
    {
        return RK_ERROR;
    }

    pItem2 = pItem1 + StrCnt1 + 1;

    StrCnt2 = ShellItemExtract(pItem2, &pItem2, &Space);
    if ((StrCnt2 == 0) || (Space == ' '))
    {
        return RK_ERROR;
    }

    time = String2Num(pItem2);
    if(time == 0)
    {
        return RK_SUCCESS;
    }

    if((StrCmpA(pItem1, "/cs", 3) == 0) && (StrCnt1 == 3))
    {
        rk_printf("continuous tx testing time = %d ms", time);
        #ifdef _REALTEK_
        wext_private_command(WLAN0_NAME, "mp_start", 1);
        wext_private_command(WLAN0_NAME, "mp_channel 1", 1);
        wext_private_command(WLAN0_NAME, "mp_bandwidth 40M=0,shortGI=0", 1);
        wext_private_command(WLAN0_NAME, "mp_txpower patha=44,pathb=44", 1);
        wext_private_command(WLAN0_NAME, "mp_rate 108", 1);
        wext_private_command(WLAN0_NAME, "mp_ctx background", 1);
        rkos_sleep(time);
        wext_private_command(WLAN0_NAME, "mp_ctx stop", 1);
        wext_private_command(WLAN0_NAME, "mp_stop", 1);
        #endif
    }
    else if((StrCmpA(pItem1, "/csp", 4) == 0) && (StrCnt1 == 4))
    {
        rk_printf("continuous tx packet testing time = %d ms", time);
        #ifdef _REALTEK_
        wext_private_command(WLAN0_NAME, "mp_start", 1);
        wext_private_command(WLAN0_NAME, "mp_channel 1", 1);
        wext_private_command(WLAN0_NAME, "mp_bandwidth 40M=0,shortGI=0", 1);
        wext_private_command(WLAN0_NAME, "mp_txpower patha=44,pathb=44", 1);
        wext_private_command(WLAN0_NAME, "mp_rate 108", 1);
        wext_private_command(WLAN0_NAME, "mp_ctx background,pkt", 1);
        rkos_sleep(time);
        wext_private_command(WLAN0_NAME, "mp_ctx stop", 1);
        wext_private_command(WLAN0_NAME, "mp_stop", 1);
        #endif
    }
    else if((StrCmpA(pItem1, "/ctp", 4) == 0)  && (StrCnt1 == 4))
    {
        rk_printf("count tx packet testing time = %d ms", time);
        #ifdef _REALTEK_
        wext_private_command(WLAN0_NAME, "mp_start", 1);
        wext_private_command(WLAN0_NAME, "mp_channel 1", 1);
        wext_private_command(WLAN0_NAME, "mp_bandwidth 40M=0,shortGI=0", 1);
        wext_private_command(WLAN0_NAME, "mp_txpower patha=44,pathb=44", 1);
        wext_private_command(WLAN0_NAME, "mp_rate 108", 1);
        wext_private_command(WLAN0_NAME, "mp_ctx count=100,pkt", 1);
        rkos_sleep(time);
        wext_private_command(WLAN0_NAME, "mp_ctx stop", 1);
        wext_private_command(WLAN0_NAME, "mp_stop", 1);
        #endif
    }
    else if((StrCmpA(pItem1, "/st", 3) == 0)  && (StrCnt1 == 3))
    {
        rk_printf(" Single Tone Tx testing time = %d ms", time);
        #ifdef _REALTEK_
        wext_private_command(WLAN0_NAME, "mp_start", 1);
        wext_private_command(WLAN0_NAME, "mp_channel 1", 1);
        wext_private_command(WLAN0_NAME, "mp_bandwidth 40M=0,shortGI=0", 1);
        wext_private_command(WLAN0_NAME, "mp_txpower patha=44,pathb=44", 1);
        wext_private_command(WLAN0_NAME, "mp_rate 108", 1);
        wext_private_command(WLAN0_NAME, "mp_ctx background,stone", 1);
        rkos_sleep(time);
        wext_private_command(WLAN0_NAME, "mp_ctx stop", 1);
        wext_private_command(WLAN0_NAME, "mp_stop", 1);
        #endif
    }
    return RK_SUCCESS;

}


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_rx(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;
    uint32 time;

    if(ShellHelpDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }

    time = String2Num(pstr);

    if(time == 0)
    {
        return RK_SUCCESS;
    }

    rk_printf("Air Rx testing testing time = %d ms", time);
    #ifdef _REALTEK_
    wext_private_command(WLAN0_NAME, "mp_start", 1);
    wext_private_command(WLAN0_NAME, "mp_bandwidth 40M=0,shortGI=0", 1);
    wext_private_command(WLAN0_NAME, "mp_channel 6", 1);
    wext_private_command(WLAN0_NAME, "mp_arx start", 1);
    rkos_sleep(time);
    wext_private_command(WLAN0_NAME, "mp_arx stop", 1);
    wext_private_command(WLAN0_NAME, "mp_stop", 1);
    #endif

    return RK_SUCCESS;

}

typedef  struct _WIFI_SEND_ARG
{
    uint32 ipaddr;
    uint32 tcp;
    uint32 recv_run;
    uint32 send_run;

}WIFI_SEND_ARG;


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_tsf(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;
    uint32 num;
    long long tfs;

    if(ShellHelpDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }

    num = String2Num(pstr);

    rk_printf("num = %d", num);

    while(num--)
    {
        #ifdef _REALTEAK_
        wifi_update_tsf(&tfs);
        #endif
        rk_printf("tsf = 0x%08x%08x", (uint32)(tfs>>32), (uint32)tfs);
        //rkos_sleep(100);
    }

}

_WICE_WIFITHREAD_SHELL_
WIFI_SEND_ARG stWifiArg;

#if 1
void tcp_test_task(void *arg)
{

    struct tcp_pcb *pcb;
    struct netconn *conn;
    struct netconn *conn_new;
    struct netbuf * buf;
    ip_addr_t lipaddr;
    err_t err;

    uint32 tcpflag = (uint32)arg;

    uint32 totalen;
    uint32 SystickSave;
    uint32 time;

    rk_printf("wifi test tcp = %d", (uint32)arg);

    if(tcpflag)
    {
        conn = netconn_new(NETCONN_TCP);
    }
    else
    {
        conn = netconn_new(NETCONN_UDP);
    }

    if (conn == NULL)
    {
        rk_printf("netconn_new alloc fail");
        RKTaskDelete2(NULL);
        while (1)
        {
            rkos_delay(100);
        }
    }

    netconn_bind(conn , IP_ADDR_ANY, 90);


    stWifiArg.recv_run = 1;

    if(tcpflag)
    {
        netconn_listen(conn);
        netconn_accept(conn, &conn_new);
    }
    else
    {
        conn_new = conn;
    }

    SystickSave = SysTickCounter;
    totalen = 0;

    while (1)
    {
        if(stWifiArg.recv_run == 0)
        {
            netconn_close(conn_new);
            netconn_delete(conn_new);

            if(tcpflag)
            {
                rk_printf("netconn_close");
                netconn_close(conn);
                netconn_delete(conn);
            }

            RKTaskDelete2(NULL);
            while (1)
            {
                rkos_delay(100);
            }
        }

        if (netconn_recv(conn_new, &buf) == 0)
        {
            if(SysTickCounter - SystickSave >= 100)
            {
                time = (SysTickCounter - SystickSave) / 100  + ((SysTickCounter - SystickSave) % 100 > 50)? 1:0;

                rk_printf("recive %d KByte, %d Kbits/sec", totalen /1000/time, (totalen /1000 * 8) / time);
                SystickSave = SysTickCounter;
                totalen = 0;
            }
            totalen += buf->p->len;
            netbuf_delete(buf);
        }
        else
        {
            rk_printf("netconn_close");
            netconn_close(conn_new);
            netconn_delete(conn_new);
            netconn_accept(conn, &conn_new);
            rk_printf("netconn_accept");
        }
    }
}

_WICE_WIFITHREAD_SHELL_
void tcp_test_init(uint32 tcp)
{
    RKTaskCreate2(tcp_test_task, NULL, NULL, "tcp_test_task",512, 18 ,(void *)tcp);
}

void tcp_test_send_task(void *arg)
{
    struct netconn *conn;
    ip_addr_t lipaddr;
    struct netbuf * buf;

    uint8 *send_buf = NULL;
    int i=0;
    uint32 tcp;

    uint32 totalen;
    uint32 SystickSave;
    uint32 time;

    send_buf = rkos_memory_malloc(2048);

    for(i=0; i<2048; i++)
    {
        *(send_buf+i) = i;
    }

    rk_printf("ipaddr = %x, tcp = %d", ((WIFI_SEND_ARG *)arg)->ipaddr, ((WIFI_SEND_ARG *)arg)->tcp);

    lipaddr.addr = ((WIFI_SEND_ARG *)arg)->ipaddr;

    tcp = ((WIFI_SEND_ARG *)arg)->tcp;

    if(tcp)
    {
        conn = netconn_new(NETCONN_TCP);
    }
    else
    {
        conn = netconn_new(NETCONN_UDP);
    }

    if (conn == NULL)
    {
        rk_printf("netconn_new alloc fail");

        rkos_memory_free(send_buf);
        RKTaskDelete2(NULL);
        while(1)
        {
            rkos_sleep(10);
        }
    }

    netconn_connect(conn, &lipaddr, 100);

    if(tcp == 0)
    {
        buf = netbuf_new();
    }

    rk_printf("wifi send ok ipaddr = 0x%08x", lipaddr.addr);

    SystickSave = SysTickCounter;
    totalen = 0;

    stWifiArg.send_run = 1;

    while (1)
    {
         if(stWifiArg.send_run == 0)
         {
            netconn_disconnect(conn);
            rkos_memory_free(send_buf);
            RKTaskDelete2(NULL);
            while(1)
            {
                rkos_sleep(10);
            }
         }

         if(tcp)
         {
             if(netconn_write(conn, send_buf, 2048, NETCONN_COPY) != ERR_OK)
             {
                netconn_disconnect(conn);
                rkos_memory_free(send_buf);
                stWifiArg.send_run = 0;
                RKTaskDelete2(NULL);
                while(1)
                {
                    rkos_sleep(10);
                }
             }

             totalen += 2048;
         }
         else
         {
            netbuf_ref(buf,send_buf,1400);
            if(netconn_send(conn,buf)!= ERR_OK)
            {

            }
            rkos_sleep(1);
            totalen += 1400;
         }

        if(SysTickCounter - SystickSave >= 100)
        {
            time = (SysTickCounter - SystickSave) / 100  + ((SysTickCounter - SystickSave) % 100 > 50)? 1:0;
            rk_printf("send %d KByte, %d Kbits/sec",totalen /1000/time, (totalen /1000 * 8)/time);
            SystickSave = SysTickCounter;
            totalen = 0;
        }

    }
}
void tcp_test_send_init(WIFI_SEND_ARG * arg)
{
    RKTaskCreate2(tcp_test_send_task, NULL, NULL, "tcp_send", 512, 18, (void *)arg);
}
#endif


_WICE_WIFITHREAD_SHELL_
#ifdef _REALTEK_
void print_scan_result( rtw_scan_result_t* record )
{
    RTW_API_INFO( ( "\r\n" ) );

    RTW_API_INFO( ( "%5s ", ( record->bss_type == RTW_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra" ) );
    RTW_API_INFO( ( MAC_FMT, MAC_ARG(record->BSSID.octet) ) );
    RTW_API_INFO( ( " %d ", record->signal_strength ) );
    RTW_API_INFO( ( " %2d  ", record->channel ) );
    RTW_API_INFO( ( " %2d  ", record->wps_type ) );
    RTW_API_INFO( ( "%-14s ", ( record->security == RTW_SECURITY_OPEN ) ? "Open" :
                                 ( record->security == RTW_SECURITY_WEP_PSK ) ? "WEP" :
                                 ( record->security == RTW_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
                                 ( record->security == RTW_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
                                 ( record->security == RTW_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
                                 ( record->security == RTW_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
                                 ( record->security == RTW_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                 ( record->security == RTW_SECURITY_WPA_WPA2_MIXED ) ? "WPA/WPA2 AES" :
                                 "Unknown" ) );

    RTW_API_INFO( ( " %s ", record->SSID.value) );
    RTW_API_INFO( ( "\r\n" ) );
}
#endif


#ifdef _WICE_
#define MAC_ARG(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
static void printf_scan_result( wiced_scan_handler_result_t* malloced_scan_result )
{

    wiced_scan_result_t * record;
    record = &malloced_scan_result->ap_details;

    printf(  "\r\n"  );

    printf(  "%5s ", ( record->bss_type == WICED_BSS_TYPE_ADHOC ) ? "Adhoc" : "Infra"  );
    printf(  MAC_FMT, MAC_ARG(record->BSSID.octet)  );
    printf(  " %d ", record->signal_strength  );
    printf(  " %2d  ", record->channel  );
    printf(  "%-14s ", ( record->security == WICED_SECURITY_OPEN ) ? "Open" :
                                 ( record->security == WICED_SECURITY_WEP_PSK ) ? "WEP" :
                                 ( record->security == WICED_SECURITY_WPA_TKIP_PSK ) ? "WPA TKIP" :
                                 ( record->security == WICED_SECURITY_WPA_AES_PSK ) ? "WPA AES" :
                                 ( record->security == WICED_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" :
                                 ( record->security == WICED_SECURITY_WPA2_TKIP_PSK ) ? "WPA2 TKIP" :
                                 ( record->security == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                 ( record->security == WICED_SECURITY_WPA2_MIXED_ENT ) ? "WPA/WPA2 AES" :
                                 "Unknown"  );

    printf(  " %s ", record->SSID.value );
    printf(  "\r\n"  );

    rkos_memory_free(malloced_scan_result);
}
#endif


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_start(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;
    uint32 mode;

    if(ShellHelpDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }

    if(StrCmpA(pstr, "ap", 2) == 0)
    {
        mode = WLAN_MODE_AP;
    }
    else if(StrCmpA(pstr, "sta", 3) == 0)
    {
        mode = WLAN_MODE_STA;
    }
    else
    {
        return RK_ERROR;
    }


    RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)mode, SYNC_MODE);
    //rk_wifi_usartconfig("long",4,"123123123", 9);
    //rk_wifi_usartconfig("cpsb-lyb-test",13, "123456789", 9);
    // rk_wifi_usartconfig("socket_test", 11, "12345678", 8);
    //rk_wifi_usartconfig("cpsb-ctf-test",13, "cp3b_ctf", 8);
   // rk_wifi_usartconfig("cpsb-sch-test",13, "123123123", 9);
   //rk_wifi_usartconfig("cpsb-wp-test",12, "123456789", 9);

    #if 0
    while (wifi_link == WICED_FALSE)
    {
        host_rtos_delay_milliseconds(1000);
    }
    #endif

    rk_printf("wifi start ok\n");

    return RK_SUCCESS;

}


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_delete(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    #ifdef _WICE_
    rk_wifi_deinit();
    RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
    #endif

    return RK_SUCCESS;
}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_configst(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    #ifdef _WICE_
    rk_wifi_smartconfig();
    #endif

    return RK_SUCCESS;
}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_configsp(HDC dev, uint8 * pstr)
{
   if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    #ifdef _WICE_
    rk_easy_smartconfig_stop();
    #endif

    return RK_SUCCESS;
}


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_recv(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    uint32 tcp;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }

    if(StrCmpA(pstr, "udp", 3) == 0)
    {
        tcp = 0;
    }
    else if(StrCmpA(pstr, "tcp", 3) == 0)
    {
        tcp = 1;
    }
    else if(StrCmpA(pstr, "stop", 4) == 0)
    {
        stWifiArg.recv_run = 0;
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }


    if(stWifiArg.recv_run)
    {
       rk_printf("exist recv thread");
       return RK_SUCCESS;
    }

    tcp_test_init(tcp);

    return RK_SUCCESS;
}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_send(HDC dev, uint8 * pstr)
{
    uint8  *pItem0, *pItem1, *pItem2, *pItem3, *pItem4;
    uint16 StrCnt0, StrCnt1, StrCnt2, StrCnt3, StrCnt4;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;


    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }


    StrCnt0 = ShellItemExtract(pstr, &pItem0, &Space);
    if ((StrCnt0 == 0) || (Space == '.'))
    {
        return RK_ERROR;
    }

    if(StrCmpA(pstr, "udp", 3) == 0)
    {
        stWifiArg.tcp = 0;
    }
    else if(StrCmpA(pstr, "tcp", 3) == 0)
    {
        stWifiArg.tcp = 1;
    }
    else if(StrCmpA(pstr, "stop", 4) == 0)
    {
        stWifiArg.send_run = 0;
        return RK_SUCCESS;
    }

    rk_printf("tcp = %d", stWifiArg.tcp);

    pItem1 = pItem0 + StrCnt0 + 1;

    StrCnt1 = ShellItemExtract(pItem1, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != '.'))
    {
        return RK_ERROR;
    }

    pItem2 = pItem1 + StrCnt1 + 1;

    StrCnt2 = ShellItemExtract(pItem2, &pItem2, &Space);
    if ((StrCnt2 == 0) || (Space != '.'))
    {
        return RK_ERROR;
    }

    pItem3 = pItem2 + StrCnt2 + 1;

    StrCnt3 = ShellItemExtract(pItem3, &pItem3, &Space);
    if ((StrCnt3 == 0) || (Space != '.'))
    {
        return RK_ERROR;
    }

    pItem4 = pItem3 + StrCnt3 + 1;

    StrCnt4 = ShellItemExtract(pItem4, &pItem4, &Space);
    if ((StrCnt4 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }

    stWifiArg.ipaddr = 0;

    pItem1[StrCnt1] = 0;
    stWifiArg.ipaddr |= String2Num(pItem1);
    pItem1[StrCnt1] = '.';

    pItem2[StrCnt2] = 0;
    stWifiArg.ipaddr |= String2Num(pItem2) << 8;
    pItem2[StrCnt2] = '.';

    pItem3[StrCnt3] = 0;
    stWifiArg.ipaddr |= String2Num(pItem3) << 16;
    pItem3[StrCnt3] = '.';

    pItem4[StrCnt4] = 0;
    stWifiArg.ipaddr |= String2Num(pItem4) << 24;

    rk_printf("ipaddr = %x", stWifiArg.ipaddr);


    if(stWifiArg.send_run)
    {
        rk_printf("exist send thread");
        return RK_SUCCESS;
    }

    tcp_test_send_init(&stWifiArg);

    return RK_SUCCESS;
}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_scan(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    rk_printf("scan....");

    #ifdef _REALTEK_
    wifi_scan_networks(print_scan_result, NULL);
    #endif

    #ifdef _WICE_
    rk_scan_resut_handle(printf_scan_result, NULL);
    wiced_wifi_scan_networks(printf_scan_result, NULL);
    #endif

    return RK_SUCCESS;
}

_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell_connect(HDC dev, uint8 * pstr)
{
    uint8  *pItem1, *pItem2;
    uint16 StrCnt1, StrCnt2;
    rk_err_t   ret;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    StrCnt1 = ShellItemExtract(pstr, &pItem1, &Space);
    if ((StrCnt1 == 0) || (Space != ' '))
    {
        return RK_ERROR;
    }

    pItem2 = pItem1 + StrCnt1 + 1;


    StrCnt2 = ShellItemExtract(pItem2, &pItem2, &Space);
    if ((StrCnt2 == 0) || (Space != '\0'))
    {
        return RK_ERROR;
    }



    rk_wifi_usartconfig(pItem1,StrCnt1,pItem2, StrCnt2);

    return RK_SUCCESS;
}


_WICE_WIFITHREAD_SHELL_
rk_err_t wifi_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellWifiName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellWifiName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellWifiName[i].CmdDes, pItem);
    if(ShellWifiName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellWifiName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;


}


#endif

//#pragma arm section code


