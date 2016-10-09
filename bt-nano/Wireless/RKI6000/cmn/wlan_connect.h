#ifndef __WLAN_CONNECT_H_
#define __WLAN_CONNECT_H_

#define   WIFI_FW_NULL_STATE            (uint32)0x00000000
#define WIFI_FW_AUTH_NULL           (uint32)0x00000100
#define WIFI_FW_AUTH_STATE          (uint32)0x00000200
#define WIFI_FW_AUTH_SUCCESS        (uint32)0x00000400

#define WIFI_FW_ASSOC_STATE         (uint32)0x00002000
#define WIFI_FW_ASSOC_SUCCESS       (uint32)0x00004000

#define WIFI_FW_LINKING_STATE       (WIFI_FW_AUTH_NULL | WIFI_FW_AUTH_STATE | WIFI_FW_ASSOC_STATE)

uint8 wlan_joinbss_proc(ADAPTER  *Adapter, struct wlan_network* pnetwork);
int32 wlan_select_ap_to_join(struct mlme_struct *pmlme);
uint8 wlan_do_join(ADAPTER * padapter);

void wlan_sta_assoc_start(ADAPTER *Adapter);
void wlan_sta_auth_start(ADAPTER* Adapter);
void wlan_sta_join_start(ADAPTER* Adapter);

extern void wlan_joinbss_result_proc(ADAPTER *adapter, uint8 *pbuf);
void wlan_report_join_result(ADAPTER *Adapter, int32 res);

void wlan_join_timeout_callback (void *Adapter, uint8 *data);
void wlan_join_timeout_handler (void *FunctionContext);
void wlan_link_retry_callback(void *adapter, uint8 *data);
void link_retry_timer_handler(void *FunctionContext);

#ifdef CONFIG_LAYER2_ROAMING
int32 wifi_roaming_req_entry(ADAPTER *adapter, struct wlan_network *tgt_network);
void wlan_roaming(ADAPTER *Adapter, struct wlan_network *tgt_network);
#endif

#endif
