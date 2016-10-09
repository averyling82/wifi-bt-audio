#ifndef __WLAN_UTILITY_H_
#define __WLAN_UTILITY_H_

__inline static void update_support_rate(struct sta_info *psta, uint8* buf, int32 len)
{
    os_memset(psta->bssrateset, 0, sizeof(psta->bssrateset));
    psta->bssratelen=len;
    os_memcpy(psta->bssrateset, buf, len);
}

__inline static uint32 update_mcs_rate(struct HT_caps_element *pHT_caps)
{
    uint32 mask = 0;

    mask = ((pHT_caps->u.HT_cap_element.MCS_rate[0] << 12) | (pHT_caps->u.HT_cap_element.MCS_rate[1] << 20));

    return mask;
}

__inline static uint8 *wlan_get_capability_from_ie(uint8 *ie)
{
    return (ie + 8 + 2);
}

__inline static uint8 *wlan_get_timestampe_from_ie(uint8 *ie)
{
    return (ie + 0);
}

__inline static uint8 *wlan_get_beacon_interval_from_ie(uint8 *ie)
{
    return (ie + 8);
}

__inline static uint16 get_beacon_interval(WLAN_BSSID_EX *bss)
{
    uint16 val;
    os_memcpy((uint8 *)&val, wlan_get_beacon_interval_from_ie(bss->IEs), 2);
    return le16_to_cpu(val);

}

__inline static int32 wlan_is_same_ess(NDIS_802_11_SSID *a, NDIS_802_11_SSID *b)
{
    return ((a->SsidLength == b->SsidLength)
            &&  os_memcmp(a->Ssid, b->Ssid, a->SsidLength)==_TRUE);
}


uint8 networktype_to_raid(uint8 network_type);
int32 judge_network_type(ADAPTER *Adapter, uint8 *rate, int32 ratelen);

void set_channel_bwmode(ADAPTER *Adapter, uint8 channel, uint8 channel_offset, uint16 bwmode);
void select_channel(ADAPTER *Adapter, uint8 channel);
void set_bwmode(ADAPTER *Adapter, uint16 bwmode, uint8 channel_offset);

int32 is_client_associated_to_ap(ADAPTER *Adapter);
int32 is_IBSS_empty(ADAPTER *Adapter);

uint8 check_assoc_ap(uint8 *pframe, uint32 len);

int32 wmm_param_handler(ADAPTER *Adapter, PNDIS_802_11_VARIABLE_IEs pIE);

void erp_ie_handler(ADAPTER *Adapter, PNDIS_802_11_VARIABLE_IEs pIE);
void VCS_update(ADAPTER *Adapter, struct sta_info *psta);

int32 update_beacon_info(ADAPTER *Adapter, uint8 *pframe, uint32 len, struct sta_info *psta);

void update_iot_info(ADAPTER *Adapter);
int32 update_sta_support_rate(ADAPTER *Adapter, uint8* pvar_ie, uint32 var_ie_len, uint8 *Support_rate);

void update_sta_info(ADAPTER *Adapter, struct sta_info *psta);
void update_ra_entry(ADAPTER *Adapter, struct sta_info *sta);

void get_rate_set(ADAPTER *Adapter, uint8 *pbssrate, int32 *bssrate_len);
uint8 get_highest_rate_idx(uint32 mask);
uint32 is_ap_in_tkip(ADAPTER *Adapter);


void update_tsf(struct mlme_struct *pmlme, uint8 *pframe, uint32 len);
void update_vcs(ADAPTER *Adapter, struct sta_info *psta);

extern int32 cckrates_included(uint8 *rate, int32 ratelen);
extern int32 cckratesonly_included(uint8 *rate, int32 ratelen);

int32 check_basic_rate(ADAPTER *Adapter, uint8 *pRate, int32 pLen);
void get_matched_rate(ADAPTER *Adapter, uint8 *pRate, uint32 *pLen, int32 which);

extern int32 wlan_link_ready(ADAPTER *Adapter);


extern uint16 wlan_get_capability(WLAN_BSSID_EX *bss);

#endif

