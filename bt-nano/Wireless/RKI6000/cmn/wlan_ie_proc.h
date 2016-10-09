#ifndef __WLAN_IE_PROC_H
#define __WLAN_IE_PROC_H

uint8 *wlan_set_fixed_ie(uint8 *pbuf, uint32 len, uint8 *source, uint32 *frlen);
uint8 *wlan_set_ie(uint8 *pbuf, int32 index, uint32 len, uint8 *source, uint32 *frlen);
uint8 *wlan_get_ie(uint8 *pbuf, int32 index, uint32 *len, uint32 limit);
void wlan_set_supported_rate(uint8* SupportedRates, uint32 mode) ;

uint8 *wlan_get_wpa_ie(uint8 *pie, uint32 *wpa_ie_len, uint32 limit);
uint8 *wlan_get_wpa2_ie(uint8 *pie, uint32 *rsn_ie_len, uint32 limit);
int32 wlan_get_wpa_cipher_suite(uint8 *s);
int32 wlan_get_wpa2_cipher_suite(uint8 *s);
int32 wlan_parse_wpa_ie(uint8* wpa_ie, int32 wpa_ie_len, int32 *group_cipher, int32 *pairwise_cipher, int32 *is_8021x);
int32 wlan_parse_wpa2_ie(uint8* wpa_ie, int32 wpa_ie_len, int32 *group_cipher, int32 *pairwise_cipher, int32 *is_8021x);

int32 wlan_get_sec_ie(uint8 *in_ie,uint32 in_len,uint8 *rsn_ie,uint16 *rsn_len,uint8 *wpa_ie,uint16 *wpa_len);
int32 wlan_get_wps_ie(uint8 *in_ie, uint32 in_len, uint8 *wps_ie, uint32 *wps_ielen);

uint32  wlan_get_rateset_len(uint8  *rateset);

struct config_struct;
int32 wlan_generate_ie(ADAPTER* Adapter, struct config_struct *pconfig);

uint32  wlan_is_cckrates_included(uint8 *rate);

uint32  wlan_is_cckratesonly_included(uint8 *rate);

int32 wlan_check_network_type(uint8 *rate, int32 ratelen, int32 channel);

int32 wlan_get_cipher_info(struct wlan_network *pnetwork);
void wlan_get_encrypt_info(struct wlan_network *pnetwork);

extern int32 wlan_restruct_wmm_ie(ADAPTER *Adapter, uint8 *in_ie, uint8 *out_ie, uint32 in_len, uint32 initial_out_len);

#endif /* IEEE80211_H */

