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
#ifdef _DRIVER_WIFI__
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
#include "ap6181wifi.h"
#include "wifithread.h"
#include "wiced_wifi.h"
#include "wiced_management.h"
#include "FwAnalysis.h"
#include "SysInfoSave.h"
#include "wwd_events.h"
//#ifdef __WICE_WIFITHREAD_C__


extern int  wifi_scan_doing;
wiced_bool_t wifi_join = WICED_FALSE;
wiced_bool_t wifi_network_up_flag = WICED_FALSE;
//wiced_bool_t wifi_connect = WICED_FALSE;
extern wiced_bool_t wifi_link;
extern wiced_bool_t wiced_sta_link_up;
extern wiced_bool_t  wiced_wlan_initialised;
extern unsigned char easy_setup_flag;
extern unsigned char easy_setup_stop_flag;
extern pTimer  TimerHandler;
extern WIFI_AP JoinAp;
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

int wifi_set_scan_handle = 0;


int rkwifi_get_strength(char *mssid_value, int mssid_len, uint8* bssid_value, int *strength);


typedef  struct _WIFI_ASK_QUEUE
{
    uint32 cmd;
    WIFI_AP join_ap;
    wiced_scan_result_handler_t scan_hand;

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
     if(mwifi_buf->Totalnum == 0xff)//JJJHHH 20161014-----Flash空白位置是ff
     {
        rk_printf("mwifi_buf->Totalnum == 0xff\n");
        mwifi_buf->Totalnum = 0;
        return RK_SUCCESS;
     }

     if(mwifi_buf->Totalnum > WIFI_AP_COUNT)//JJJHHH 20161103
     {
        rk_printf("read db tot num is error mwifi_buf->Totalnum=%d",mwifi_buf->Totalnum);
        rkos_memory_free(mwifi_buf);
        mwifi_buf = NULL;
        wifi_join = WICED_ERR;
        return RK_ERROR;
     }

     rk_printf("re wifi_buf %d totalnum = %d = 0x%x = %d",ret ,mwifi_buf->Totalnum, mwifi_buf, sizeof(WIFI_INFO));

//扫描之前初始化内部数据，scan_flag connect_flag = 0;
     for(i=0; i<mwifi_buf->Totalnum; i++)
     {
        mwifi_buf->wifiinfo[i].scan_flag = 0;
        if(mwifi_buf->wifiinfo[i].Connect_flag == 1)
        {
            rkos_memcpy(connect_ssid, mwifi_buf->wifiinfo[i].wifi_ap.ap_ssid_value, 32);
            mwifi_buf->wifiinfo[i].Connect_flag = 0;rk_printf("connect_ssid=%s\n",connect_ssid);
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
    rk_printf("wifi_relink_smartconfig = %d", LINK_SMARTCONFIG);
    rkos_queue_send(gWifiBackgroundData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
	MainTask_SetStatus(MAINTASK_WIFICONFIG,0);//jjjhhh 20161110
    return RK_SUCCESS;
}

#endif


xSemaphoreHandle  UsartScan_sem = NULL;
static wiced_result_t usart_scan_handler( wiced_scan_handler_result_t* malloced_scan_result )
{

   // printf("length = %d = %s\n", malloced_scan_result->ap_details.SSID.length, malloced_scan_result->ap_details.SSID.value);

    //printf("%02x:%02x:%02x:%02x:%02x:%02x", malloced_scan_result->ap_details.BSSID.octet[0], malloced_scan_result->ap_details.BSSID.octet[1],
      //  malloced_scan_result->ap_details.BSSID.octet[2], malloced_scan_result->ap_details.BSSID.octet[3],
       // malloced_scan_result->ap_details.BSSID.octet[4], malloced_scan_result->ap_details.BSSID.octet[5]);
        //printf("result     %02x:%02x:%02x:%02x:%02x:%02x", (*result_ptr)->BSSID.octet[0], (*result_ptr)->BSSID.octet[1], (*result_ptr)->BSSID.octet[2], (*result_ptr)->BSSID.octet[3],
        //(*result_ptr)->BSSID.octet[4], (*result_ptr)->BSSID.octet[5]);



    if((malloced_scan_result->status == WICED_SCAN_COMPLETED_SUCCESSFULLY) || (malloced_scan_result->status == WICED_SCAN_ABORTED))
    {
          rkos_memory_free(malloced_scan_result);
          rkos_semaphore_give(UsartScan_sem);
          return WICED_SUCCESS;
    }
    else
    {
        if(rkos_memcmp(JoinAp.ap_ssid_value, malloced_scan_result->ap_details.SSID.value, 32) == 1)
        {
            rkos_memcpy(JoinAp.ap_bssid, malloced_scan_result->ap_details.BSSID.octet, 6);
            JoinAp.ap_security_type = malloced_scan_result->ap_details.security;
        }
    }

    rkos_memory_free(malloced_scan_result);
    return WICED_SUCCESS;

}


rk_err_t rk_wifi_connect_usart(WIFI_AP *mwifi_ap)
{
    int ret;

    rkos_memset(&JoinAp, 0x00, sizeof(WIFI_AP));
    JoinAp.ap_ssid_len = mwifi_ap->ap_ssid_len;
    JoinAp.security_key_length = mwifi_ap->security_key_length;
    rkos_memcpy(JoinAp.ap_ssid_value, mwifi_ap->ap_ssid_value, mwifi_ap->ap_ssid_len);
    rkos_memcpy(JoinAp.security_key, mwifi_ap->security_key, mwifi_ap->security_key_length);
    //rkos_memcpy(&JoinAp, mwifi_ap, sizeof(WIFI_AP));

    //wifi_connect = WICED_BUSY;
    wifi_join = WICED_FALSE;

    if(wifi_link == WICED_TRUE)
    {
        wifi_link = WICED_FALSE;
        wiced_network_down(WICED_STA_INTERFACE);
    }

    UsartScan_sem = rkos_semaphore_create(1, 0);
    if(NULL == UsartScan_sem)
    {
        printf("usartscan_scan create fail\n");
        wifi_join = WICED_ERR;
        //wifi_connect = WICED_FALSE;
        return RK_ERROR;
    }


    rk_scan_resut_handle(usart_scan_handler, NULL);
    wiced_wifi_scan_networks(usart_scan_handler, NULL);
    rkos_semaphore_take(UsartScan_sem, MAX_DELAY);

    wifi_join = WICED_WAIT;
    ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
    if(ret == WICED_SUCCESS)
    {
        wifi_join = WICED_TRUE;
        printf(" wifi_link usart connec\n");
        wifi_link = WICED_TRUE;
#ifdef _WIFI_FOR_SYSTEM_
        rk_wifi_info_save(JoinAp);
#endif

        rkos_semaphore_delete(UsartScan_sem);
        //wifi_connect = WICED_FALSE;
        return RK_SUCCESS;
    }
    printf("wifi connect fail\n");
    wifi_join = WICED_ERR;
    rkos_semaphore_delete(UsartScan_sem);
    //wifi_connect = WICED_FALSE;
    return RK_ERROR;

}



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
       
	    rk_printf("jjjhhh 1111 wwd_wifi_scan ssid=%s\n",ssid.value);
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
        rk_printf("smatconfig busy");
        return RK_ERROR;
    }

    easy_setup_flag = WICED_BUSY;
    easy_setup_stop_flag = 0;
    wifi_join = WICED_FALSE;

    do
    {
        rk_printf("smartconfig = %d", wifi_network_up_flag);//jjjhhh 20161110
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
    wifi_join = WICED_FALSE;

    rkos_queue_send(gWifiAppData->WIFIAPPAskQueue, &wifi_ask, MAX_DELAY);
    rkos_queue_receive(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);

    return wifi_resp.cmd;

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
       WPRINT_NETWORK_INFO(("2wwd_wifi_join start\r\n"));

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
            if(wifi_ask.cmd == LINK_SMARTCONFIG)//update wifi info
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

}

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
    wiced_result_t              result;
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

    #ifdef RK_WIFI_AP
    if(gWifiAppData->WifiMode == WLAN_MODE_AP)
    {
        FREQ_EnterModule(FREQ_MAX);

        result = wiced_init();

        wiced_ip_setting_t ip_settings;
        struct ip_addr ipaddr;
        struct ip_addr netmask;
        struct ip_addr gw;
        WICED_IP4_ADDR(&ipaddr, 192, 168, 1, 1);//JJJHHH
        WICED_IP4_ADDR(&netmask, 255, 255, 255, 0);
        WICED_IP4_ADDR(&gw, 192, 168, 1, 1);
        memset(&ip_settings, 0 , sizeof(wiced_ip_setting_t));
        ip_settings.ip_address.ip.v4 = ipaddr.addr;
    ip_settings.gateway.ip.v4 = gw.addr;
    ip_settings.netmask.ip.v4 = netmask.addr;
        ret = wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ip_settings);

        MainTask_SetStatus(MAINTASK_WIFI_AP_OPEN_OK, 1);

        FREQ_ExitModule(FREQ_MAX);

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
            if(RK_SUCCESS != rk_wifiinfo_start())//wifi link failed   jjjhhh 20161110
            {
				#ifdef EASY_SETUP_ENABLE
					rk_printf ("wificonfig----wifi_applicationTask_Enter---on");
					MainTask_SetStatus(MAINTASK_WIFICONFIG,1);
					MainTask_SetStatus(MAINTASK_WIFI_CONNECT_OK, 0);
					ret = wiced_smartconfig_event(WICED_NETWORKING_WORKER_THREAD, rk_wifi_connect_smartconfig,  NULL);
                #endif		
			}
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
                #ifdef RK_WIFI_AP
                if(gWifiAppData->WifiMode == WLAN_MODE_AP)
                {
                    ret = wiced_network_down(WICED_AP_INTERFACE);
                }
                else
                #endif
                {//rk_printf("WIFI_DEINIT 11111111\n");
                    if(WIFI_CONNECTTASK_HANDLE != NULL)
                    {	
                    	//rk_printf("WIFI_DEINIT 22222222\n");
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
				//rk_printf("WIFI_DEINIT 333333\n");
                ret = wiced_deinit();//rk_printf("WIFI_DEINIT 444444\n");
                DeviceTask_DeleteDeviceList(DEVICE_LIST_SDIO, NULL, SYNC_MODE);
				//rk_printf("WIFI_DEINIT 5555555\n");
                if(ret == WICED_SUCCESS)
                {
                    wifi_resp.cmd = RK_SUCCESS;
                }
                else
                {
                    wifi_resp.cmd = RK_ERROR;
                }//rk_printf("WIFI_DEINIT 66666\n");
                rkos_queue_send(gWifiAppData->WIFIAPPRespQueue, &wifi_resp, MAX_DELAY);//rk_printf("WIFI_DEINIT 7777\n");
                break;

            case WIFI_SCAN:
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
                break;

            case WIFI_SMARTCONFIG:
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
                break;

            case WIFI_SMARTCONFIG_STOP:
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
                    ret = rk_wifi_connect_usart(&(wifi_ask.join_ap));
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
    // FW_RemoveSegment(SEGMENT_ID_AP6181_INIT);
    FW_RemoveSegment(SEGMENT_ID_AP6181_WIFI);
    //FW_RemoveSegment(SEGMENT_ID_AP6181_INIT);
    FW_RemoveSegment(SEGMENT_ID_LWIP);
    //FW_RemoveSegment(SEGMENT_ID_LWIP_API);
    //FW_RemoveSegment(SEGMENT_ID_WIFI_APPTASK);
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

    FW_LoadSegment(SEGMENT_ID_AP6181_WIFI, SEGMENT_OVERLAY_ALL);
    FW_LoadSegment(SEGMENT_ID_LWIP, SEGMENT_OVERLAY_ALL);
    //FW_LoadSegment(SEGMENT_ID_LWIP_API, SEGMENT_OVERLAY_ALL);
    //FW_LoadSegment(SEGMENT_ID_WIFI_APPTASK, SEGMENT_OVERLAY_ALL);
#ifdef __SSL_MbedTLS__
    FW_LoadSegment(SEGMENT_ID_MBEDTLS, SEGMENT_OVERLAY_ALL);
#endif

    gWifiAppData = mWifiAppData;
    gWifiBackgroundData = mWifiBackgroundData;

    return RK_SUCCESS;
}

void rk_wifi5_restart(void)
{
     wifi_link= WICED_FALSE;
     wiced_network_down(WICED_STA_INTERFACE);
     wifi_relink_sysconfig();
}




_ap6181WICE_WIFI_SHELL_
static SHELL_CMD ShellWifiName[] =
{
    "start",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "configst",NULL,"NULL","NULL",
    "configsp",NULL,"NULL","NULL",
    "recv",NULL,"NULL","NULL",
    "send",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};


_ap6181WICE_WIFI_SHELL_
rk_err_t wifi_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        printf("\n StrCnt = 0 \n");
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellWifiName, pItem, StrCnt);
    if (ret < 0)
    {
        printf("\n ret < 0 \n");
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item
    switch (i)
    {
        case 0x00:
            RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_AP, SYNC_MODE);
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
            break;

        case 0x01:
            rk_wifi_deinit();
            RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
            break;
        case 0x02:
            rk_wifi_smartconfig();
            break;

        case 0x03:
            rk_easy_smartconfig_stop();
            break;
        case 0x04:
            ap6181tcp_test_init();
            break;
        case 0x05:
            ap6181tcp_test_send_init();
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    ret = RK_SUCCESS;

    return ret;
}


#endif

//#pragma arm section code


