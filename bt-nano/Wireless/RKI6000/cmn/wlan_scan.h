#ifndef __WLAN_SCAN_H_
#define __WLAN_SCAN_H_


uint8 collect_bss_info(ADAPTER *Adapter, struct recv_frame *precv_frame, WLAN_BSSID_EX *bssid);

extern void wlan_scan_result_event(ADAPTER *Adapter, struct recv_frame *precv_frame);
extern void wlan_scan_done_event(ADAPTER *Adapter);
extern void scan_timer_handler (void *FunctionContext);
extern void wlan_scan_timeout_handler (void *FunctionContext);
extern uint8 wlan_start_scan(ADAPTER  *Adapter, NDIS_802_11_SSID *pssid);
extern void wlan_scan_complete(ADAPTER *Adapter);
extern uint8 wlan_scan_process(ADAPTER *Adapter, uint8 *pbuf);
extern uint8 wlan_scan_ap(ADAPTER  *Adapter, NDIS_802_11_SSID *pssid);
extern void wlan_scan_timeout_callback(void *Adapter, uint8 *data);

#endif // _CMD_H_

