#ifndef __WLAN_IOCTL_PROC_H
#define __WLAN_IOCTL_PROC_H


typedef uint8 NDIS_802_11_PMKID_VALUE[16];

typedef struct _BSSIDInfo
{
    uint8 BSSID[6];
    NDIS_802_11_PMKID_VALUE  PMKID;
} BSSIDInfo, *PBSSIDInfo;

uint8 wlan_set_802_11_add_key(ADAPTER * padapter, NDIS_802_11_KEY * key);
uint8 wlan_set_802_11_authentication_mode(ADAPTER *pdapter, NDIS_802_11_AUTHENTICATION_MODE authmode);
uint8 wlan_set_802_11_bssid(ADAPTER *Adapter, uint8 *bssid);
uint8 wlan_set_802_11_add_wep(ADAPTER * padapter, NDIS_802_11_WEP * wep);
uint8 wlan_set_802_11_disassociate(ADAPTER * padapter);
uint8 wlan_set_802_11_bssid_list_scan(ADAPTER *Adapter);
uint8 wlan_set_802_11_infrastructure_mode(ADAPTER * padapter, NDIS_802_11_NETWORK_INFRASTRUCTURE networktype);
uint8 wlan_set_802_11_remove_wep(ADAPTER * padapter, uint32 keyindex);
uint8 wlan_set_802_11_ssid(ADAPTER * padapter, NDIS_802_11_SSID * ssid);
uint8 wlan_set_802_11_remove_key(ADAPTER * padapter, NDIS_802_11_REMOVE_KEY * key);
uint8 wlan_set_802_11_disassociate(ADAPTER *Adapter);

uint8 wlan_validate_ssid(NDIS_802_11_SSID *ssid);

#endif

