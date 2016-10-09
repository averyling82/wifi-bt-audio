#ifndef __WLAN_STA_INFO_H_
#define __WLAN_STA_INFO_H_

#include "wlan_80211.h"


#ifdef CONFIG_SOFTAP_SUPPORT
#define MAX_ASSOC_NUM       4   //8
#else
#define MAX_ASSOC_NUM       2
#endif

struct  stainfo_stats
{

    uint32  rx_pkts;
    uint32  rx_pkts_pre;
    uint64  rx_drops;

    uint32  tx_pkts;
    uint32  tx_pkts_pre;
    uint32  tx_drops;
    uint32  tx_drops_pre;
};

#ifdef CONFIG_SOFTAP_SUPPORT
struct  ap_info_struct
{
    uint32 assoc_sta_num;
    uint8 legecy_sta_deny;
    struct HT_caps_element ht_caps_ie;
    uint8 ht_cap_len;
    struct HT_info_element  ht_info_ie;
    uint8 ht_info_len;
#ifdef SUPPORT_ACCP_MAC_CTL
    _list wlan_acl_list;
#endif
};
#endif


struct sta_info
{
    _list list; //free_sta_queue
    _list hash_list; //sta_hash
    _list asso_list;

    struct sta_xmit_struct sta_xmit_priv;
    struct sta_recv_struct sta_recv_priv;

    uint32 sleepq_len;

    uint32 state;
    uint32 aid;
    uint32 mac_id;
    uint32 qos_option;
#if defined(WIFI_WMM) && defined(WMM_APSD)
    uint8       apsd_bitmap;    // bit 0: VO, bit 1: VI, bit 2: BK, bit 3: BE
    uint32      apsd_pkt_buffering;
    //struct apsd_pkt_queue *VO_dz_queue;
    //struct apsd_pkt_queue *VI_dz_queue;
    //struct apsd_pkt_queue *BE_dz_queue;
    //struct apsd_pkt_queue *BK_dz_queue;
#endif
    uint8   hwaddr[ETH_ALEN];

    uint8   bssrateset[16];
    uint32  bssratelen;
    int32  rssi;
    int32   SignalQuality;

    uint8       cts2self;
    uint8       rtsen;

    uint8       raid;
    uint32      init_rate;

    struct stainfo_stats sta_stats;

    //for A-MPDU TX, ADDBA timeout check
    _timer addba_retry_timer;

    //for A-MPDU Rx reordering buffer control
    struct recv_reorder_ctrl recvreorder_ctrl[16];

    //for A-MPDU Tx
    //uint8     ampdu_txen_bitmap;


    struct ht_cfg_struct    ht_cfg;


    //Notes:
    //STA_Mode:
    //curr_network(mlme_struct/security_struct/qos/ht) + sta_info: (STA & AP) CAP/INFO
    //scan_q: AP CAP/INFO

    //AP_Mode:
    //curr_network(mlme_struct/security_struct/qos/ht) : AP CAP/INFO
    //sta_info: (AP & STA) CAP/INFO

    _list asoc_list;
    _list auth_list;

#ifdef CONFIG_SOFTAP_SUPPORT
    uint32 auth_expire_to;
    uint32 auth_seq;
    uint32 authalg;
    uint8 chg_txt[128];

    uint32 asoc_expire_to;

    uint32 tx_ra_bitmap;
    uint32 useShortPreamble;
    uint32 AuthAlgrthm;
#endif
    int32 retry;
    /* before we set key we should set it to true,
     * to block hw to enc frame, but after 4-way handshake
     * we should set it to false to enc all data frames */
    uint32 ieee8021x_blocked;   //0: allowed, 1:blocked
    uint32 dot118021XPrivacy; //aes, tkip...
    struct Keytype dot11tkiptxmickey;
    struct Keytype dot11tkiprxmickey;
    struct Keytype dot118021x_UncstKey;
    union pn48 dot11txpn;           // PN48 used for Unicast xmit.
    union pn48 dot11rxpn;           // PN48 used for Unicast recv.

#ifdef INCLUDE_WPA_WPA2_PSK
    /* wpa psk ---> */
    /* this struct just used in supp_psk.c */
    WPA_STA_INFO wpa_sta_info;
    //struct Dot11KeyMappingsEntry dot11KeyMapping;
    /* wpa psk <--- */
#endif
    //rssi ralated
    int32   UndecoratedSmoothedPWDB;
};

struct  sta_struct
{
    uint8 *pallocated_stainfo_buf;
    uint8 *pstainfo_buf;
    _queue  free_sta_queue;

    _lock sta_hash_lock;
    _list   sta_hash[MAX_ASSOC_NUM];
    _list   sta_asso;/* first node is bcmc sta when open wifi, also sta_hash_lock*/
    int32 asoc_sta_count;

    ADAPTER *padapter;

    _list asoc_list;
    _list auth_list;

    /* for softap sta power save */
    uint16 sta_dz_bitmap;//only support 15 stations, staion aid bitmap for sleeping sta.
    uint16 tim_bitmap;//only support 15 stations, aid=0~15 mapping bit0~bit15
    uint16 beacon_update;
    uint16 atim_end;
    /* for softap sta power save */

#ifdef CONFIG_SOFTAP_SUPPORT
    uint32 auth_to;  //sec, time to expire in authenticating.
    uint32 assoc_to; //sec, time to expire before associating.
    uint32 asoc_expire_to; //sec , time to expire after associated.

#endif

};


__inline static uint32 wifi_mac_hash(uint8 *mac)
{
    uint32 x;

    x = mac[0];
    x = (x << 2) ^ mac[1];
    x = (x << 2) ^ mac[2];
    x = (x << 2) ^ mac[3];
    x = (x << 2) ^ mac[4];
    x = (x << 2) ^ mac[5];

    x ^= x >> 8;
    x  = x & (MAX_ASSOC_NUM - 1);

    return x;
}

extern uint32   wlan_init_sta_struct(struct sta_struct *psta_global);
extern uint32   wlan_free_sta_struct(struct sta_struct *psta_global);
extern struct sta_info *wlan_alloc_stainfo(struct   sta_struct *psta_global, uint8 *hwaddr);
extern uint32   wlan_free_stainfo(ADAPTER *Adapter , struct sta_info *psta);
extern void wlan_free_all_stainfo(ADAPTER *Adapter);
extern struct sta_info *wlan_get_stainfo(struct sta_struct *psta_global, uint8 *hwaddr);
extern uint32 wlan_init_bcmc_stainfo(ADAPTER *Adapter);
extern struct sta_info* wlan_get_bcmc_stainfo(ADAPTER *Adapter);
uint32  wlan_release_stainfo(ADAPTER *Adapter , struct sta_info *psta);
void _init_stainfo(struct sta_info *psta);

#ifdef CONFIG_SOFTAP_SUPPORT
uint32  wlan_init_ap_info_struct(struct ap_info_struct *pap_info);
#endif

#endif //_STA_INFO_H_
