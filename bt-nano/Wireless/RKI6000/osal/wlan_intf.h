#ifndef __WLAN_INTF_H_
#define __WLAN_INTF_H_

//#include "ethernetif.h"
#include "wlan_intf_def.h"


#define RELEASE_VERSION "WIFI_01.15_20160129_DTIM3"
#define DEFAULT_CHANNEL 13

#define INTERRUPT_RX_REQUEST            BIT0
#define INTERRUPT_ATIM_END              BIT3
#define INTERRUPT_BCN_EARLY         BIT4
#define INTERRUPT_FW_STATUS             BIT11

struct wifi_indicate_intf
{

    void (*recv_indicatepkt)(ADAPTER *adapter, RECV_FRAME *precv_frame);
    void (*indicate_disconnect)( ADAPTER *adapter );
    void (*indicate_connect)( ADAPTER *adapter, uint32 status);
    void (*scan_complete)(ADAPTER *Adapter);
};

typedef struct _wifi_ops_entry
{
    int32 (*wifi_open_func)(void);          //wifi open func
    int32 (*wifi_close_func)(void);         //wifi close func
    int32 (*wifi_interrupt_func)(void);         //spi interrupt func
    int32 (*wifi_scan_proc_func)(WIFISUPP_SCAN_REQ_INFO_T *scan_req_info);
    int32 (*wifi_get_scan_list_func)(WIFISUPP_SCAN_AP_INFO_T *scan_list, uint8 *ap_num);
    int32 (*wifi_connect_req_func)(WIFISUPP_SSID_CONFIG_T *connect_info);
    int32 (*wifi_disconnect_req_func)(WIFISUPP_DISCONNECT_REQ_INFO_T *disconnect_info);
    int32 (*wifi_set_ipaddr_func)(uint8 *ipaddr);
    int32 (*wifi_priv_req_func)(WIFISUPP_PRIV_REQ_INFO_T *priv_info);
    int32 (*wifi_suspend_query_func)(void);
    int32 (*wifi_suspend_func)(void);
    int32 (*wifi_resume_func)(void);
    int32 (*wifi_set_idle_ps_delay)(uint32 delay_time);
    int32 (*wifi_softap_func)(uint8 *ssid, uint8 security, uint8 *password);
    int32 (*wifi_get_heap_usage_func)(WIFISUPP_USED_HEAP_INFO_T *used_heap_info);
    wifi_priv_handler *wifi_mp_handle;
} WIFI_OPS_ENTRY_T;

extern wifi_priv_handler wifi_mp_table[];

uint32 wifi_indicate_intf_init(ADAPTER *Adapter);


int32 wifi_open(void);

int32 wifi_close(void);

int32 wifi_interrupt(void);

int32 wifi_set_scan_entry(WIFISUPP_SCAN_REQ_INFO_T *scan_req_info);

int32 wifi_get_scan_list_entry(WIFISUPP_SCAN_AP_INFO_T *scan_list, uint8 *ap_num);

int32 wifi_connect_req_entry(WIFISUPP_SSID_CONFIG_T* connect_info);

int32 wifi_disconnect_req_entry(WIFISUPP_DISCONNECT_REQ_INFO_T* disconnect_info);

int32 wifi_set_ipaddr_entry(uint8 *ipaddr);

int32 wifi_get_heap_usage(WIFISUPP_USED_HEAP_INFO_T *used_heap_info);

int32 wifi_priv_req_entry(WIFISUPP_PRIV_REQ_INFO_T *priv_info);

int32 wifi_suspend_query(void);

int32 wifi_suspend(void);

int32 wifi_resume(void);

#ifdef CONFIG_SOFTAP_SUPPORT
int32 wifi_softap_entry(uint8 *ssid, uint8 security, uint8 *password);
#endif

#endif

