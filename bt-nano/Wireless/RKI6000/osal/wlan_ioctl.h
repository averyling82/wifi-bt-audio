#ifndef __WLAN_IOCTL_H_
#define __WLAN_IOCTL_H_

#include "wlan_intf_def.h"


void ioctl_scan_complete_notify(ADAPTER *Adapter);

void ioctl_get_scan_list(WIFISUPP_SCAN_AP_INFO_T *scanList, uint8 *ap_num);

int32 ioctl_set_mode(ADAPTER *Adapter, WIFISUPP_SSID_CONFIG_T* connectInfo);

int32 ioctl_set_essid(ADAPTER *Adapter, NDIS_802_11_SSID *pssid);

int32 ioctl_set_wap(ADAPTER *Adapter, uint8 *bssid);

int32 ioctl_set_wpa_enc(ADAPTER *Adapter, struct iw_point *data, DOT11_SET_KEY *pSetKey, uint8 *pKey);

int32 ioctl_set_wep_enc(ADAPTER *Adapter);

int32 ioctl_set_auth_algs(ADAPTER *Adapter);

void ioctl_set_wpa_auth_entry(WIFISUPP_SSID_CONFIG_T* connectInfo, struct wlan_network* pwlan);

void ioctl_set_eap_auth_entry(WIFISUPP_SSID_CONFIG_T* connectInfo, struct wlan_network* pwlan);

#ifdef CONFIG_SOFTAP_SUPPORT
int32 ioctl_process_association_rsp_event(ADAPTER *Adapter, struct iw_point *data, int32 is_reassoc);

int32 ioctl_process_disconnect_req_event(ADAPTER *Adapter, struct iw_point *data);

int32 ioctl_process_delete_key_event(ADAPTER *Adapter, struct iw_point *data);
#endif

int32 wpa_ioctl_process(ADAPTER *Adapter, struct iw_point *data);

#endif
