#ifndef __WLAN_MGMT_PROC_H_
#define __WLAN_MGMT_PROC_H_

#define SURVEY_TO           (70)        //(110) //(250)
#define REAUTH_TO           (5000)
#define REASSOC_TO          (5000)
#define WAIT_FOR_BCN_TO (2000)
#define ADDBA_TO            (2000)
#define WAIT_DHCP_RSP       (1500)
#ifdef CONFIG_SOFTAP_SUPPORT
#define AUTH_EXPIRE     (3)
#define ASSOC_EXPIRE    (8)
#endif

#define WAITE_BCN_LIMIT (3)
#define REAUTH_LIMIT    (2)
#define REASSOC_LIMIT   (2)
#define READDBA_LIMIT   (2)

#define MAX_BSS_CNT 32  //8 //64
#define   MAX_JOIN_TIMEOUT  10000
#define     SCANNING_TIMEOUT    10000

#define SWITCH_ANTENNA_TIMEOUT   100
#define SCAN_INTERVAL   (30) // unit:2sec, 30*2=60sec

#define SCANQUEUE_LIFETIME 20 // unit:sec

// Scan type including active and passive scan.
typedef enum _WLAN_SCAN_TYPE
{
    SCAN_PASSIVE,
    SCAN_ACTIVE,
    SCAN_MIX
} WLAN_SCAN_TYPE, *PWLAN_SCAN_TYPE;

enum wlan_link_state
{
    WLAN_NOLINK = 0,
    WLAN_SCANNING = 1,
    WLAN_LINKING = 2,
    WLAN_LINKED = 3,
    WLAN_LINKED_SCANNING = 4
};
enum wlan_op_mode
{
    WLAN_OPMODE_AUTO=0, /* Let the driver decides */
    WLAN_OPMODE_ADHOC,  /* Single cell network */
    WLAN_OPMODE_ADHOCMASTER,
    WLAN_OPMODE_INFRA,  /* Multi cell network, roaming, ... */
    WLAN_OPMODE_MASTER, /* Synchronisation master or Access Point*/
    WLAN_OPMODE_MONITOR,    /* Passive monitor (listen only) */
    WLAN_OPMODE_MP
};

#ifdef WDS
#define WIFI_WDS                0x00001000
#define WIFI_WDS_RX_BEACON  0x00002000      // already rx WDS AP beacon
#endif
#ifdef AUTO_CONFIG
#define WIFI_AUTOCONF           0x00004000
#define WIFI_AUTOCONF_IND   0x00008000
#endif

//#ifdef UNDER_MPTEST
#define WIFI_MP_CTX_BACKGROUND          0x00020000  // in continous tx background
#define WIFI_MP_CTX_ST                  0x00040000  // in continous tx with single-tone
#define WIFI_MP_CTX_BACKGROUND_PENDING  0x00080000  // pending in continous tx background due to out of skb
#define WIFI_MP_CTX_CCK_HW              0x00100000  // in continous tx
#define WIFI_MP_CTX_CCK_CS              0x00200000  // in continous tx with carrier suppression
#define   WIFI_MP_LPBK_STATE                0x00400000
//#endif

enum dot11AuthAlgrthmNum
{
    dot11AuthAlgrthm_Open = 0,
    dot11AuthAlgrthm_Shared,
    dot11AuthAlgrthm_8021X,
    dot11AuthAlgrthm_Auto,
    dot11AuthAlgrthm_MaxNum
};

#define     _1M_RATE_   0
#define     _2M_RATE_   1
#define     _5M_RATE_   2
#define     _11M_RATE_  3
#define     _6M_RATE_   4
#define     _9M_RATE_   5
#define     _12M_RATE_  6
#define     _18M_RATE_  7
#define     _24M_RATE_  8
#define     _36M_RATE_  9
#define     _48M_RATE_  10
#define     _54M_RATE_  11


//
// Channel Plan Type.
// Note:
//  We just add new channel plan when the new channel plan is different from any of the following
//  channel plan.
//  If you just wnat to customize the acitions(scan period or join actions) about one of the channel plan,
//  customize them in WLAN_CHANNEL_INFO in the WLAN_CHANNEL_LIST.
//
typedef enum _WLAN_CHANNEL_DOMAIN
{
    WLAN_CHANNEL_DOMAIN_FCC = 0,
    WLAN_CHANNEL_DOMAIN_IC = 1,
    WLAN_CHANNEL_DOMAIN_ETSI = 2,
    WLAN_CHANNEL_DOMAIN_SPAIN = 3,
    WLAN_CHANNEL_DOMAIN_FRANCE = 4,
    WLAN_CHANNEL_DOMAIN_MKK = 5,
    WLAN_CHANNEL_DOMAIN_MKK1 = 6,
    WLAN_CHANNEL_DOMAIN_ISRAEL = 7,
    WLAN_CHANNEL_DOMAIN_TELEC = 8,
    WLAN_CHANNEL_DOMAIN_MIC = 9,                // Be compatible with old channel plan. No good!
    WLAN_CHANNEL_DOMAIN_GLOBAL_DOAMIN = 10,     // Be compatible with old channel plan. No good!
    WLAN_CHANNEL_DOMAIN_WORLD_WIDE_13 = 11,     // Be compatible with old channel plan. No good!
    WLAN_CHANNEL_DOMAIN_TELEC_NETGEAR = 12,     // Be compatible with old channel plan. No good!
    WLAN_CHANNEL_DOMAIN_NCC = 13,
    WLAN_CHANNEL_DOMAIN_KOREA = 14,
    WLAN_CHANNEL_DOMAIN_TURKEY = 15,
    WLAN_CHANNEL_DOMAIN_JAPAN = 16,
    WLAN_CHANNEL_DOMAIN_FCC_NO_DFS = 17,
    WLAN_CHANNEL_DOMAIN_JAPAN_NO_DFS= 18,
    WLAN_CHANNEL_DOMAIN_DEFAULT = 19,
    WLAN_CHANNEL_DOMAIN_OMNIPEEK_ALL_CHANNEL = 20,
    //===== Add new channel plan above this line===============//
    WLAN_CHANNEL_DOMAIN_MAX
} WLAN_CHANNEL_DOMAIN, *PWLAN_CHANNEL_DOMAIN;

typedef struct _WLAN_CHANNEL_PLAN
{
    uint8   Channel[32];
    uint8   Len;
} WLAN_CHANNEL_PLAN, *PWLAN_CHANNEL_PLAN;


#define     traffic_threshold   10
#define traffic_scan_period 500

struct  scan_result
{
    int             state;
    int             bss_cnt;
    int             channel_idx;
    int             scan_mode;
    int             ss_ssidlen;
    int             scan_timeout;
    uint8   ss_ssid[IW_ESSID_MAX_SIZE + 1];
};

struct WLAN_LINK_DETECT_T
{
    int32   traffic_busy;
    uint8   idle_count;
    uint8   tx_idle_count;
    uint32  nolink_idle_count;
};

struct  qos_info
{
    uint32  qos_option; //bit mask option: u-apsd, s-apsd, ts, block ack...
};

#define MAX_WPA_IE 128

// The channel information about this channel including joining, scanning, and power constraints.
typedef struct _WLAN_CHANNEL_INFO
{
    uint8               ChannelNum;     // The channel number.
    WLAN_SCAN_TYPE  ScanType;       // Scan type such as passive or active scan.
    //uint16                ScanPeriod;     // Listen time in millisecond in this channel.
    //int32             MaxTxPwrDbm;    // Max allowed tx power.
    //uint32                ExInfo;         // Extended Information for this channel.
} WLAN_CHANNEL_INFO, *PWLAN_CHANNEL_INFO;

struct mlme_info
{
    uint32      state;
    uint32      wait_bcn_count;
    uint32      reauth_count;
    uint32      reassoc_count;
    uint32      auth_seq;
    uint32      auth_algo;  // 802.11 auth, could be open, shared, auto
    uint32      authModeToggle;
    uint8       key_mask;
    uint32      iv;
    uint8       chg_txt[128];
    uint16      aid;
    uint16  bcn_interval;
    uint8       assoc_AP_vendor;
    uint8       slotTime;
    uint8       ERP_enable;
    uint8       ERP_IE;
    uint8       turboMode_cts2self;
    uint8       turboMode_rtsen;
    uint8       WMM_enable;
    uint8       HT_enable;
    uint8       HT_caps_enable;
    uint8       HT_info_enable;
    uint8       HT_protection;
    uint8       agg_enable_bitmap;
    uint8       ADDBA_retry_count;
    uint8       candidate_tid_bitmap;
    struct ADDBA_request        ADDBA_req;
    struct WMM_para_element WMM_param;
    struct HT_caps_element  HT_caps;
    struct HT_info_element  HT_info;
    WLAN_BSSID_EX network;//join network or bss_network, if in ap mode, it is the same to cur_network.network
    // Accept ADDBA Request
    boolean         bAcceptAddbaReq;
    boolean         bSendAddbaReq;
    boolean         bSupport8kAmsdu;
    uint8       bwmode_updated;
    uint8       wpa_ie[MAX_WPA_IE];
    uint16 wpa_ie_len;
    uint8       rsn_ie[MAX_WPA_IE];
    uint16 rsn_ie_len;
};

struct mlme_struct
{

    _lock   lock;
    uint32  link_state;
    uint8   op_mode;
    uint8   channel_plan;
    uint8   to_join;
    uint16  status_code;
    uint16  reason_code;
    ADAPTER  *padapter;

    uint8       cur_channel;
    uint8       cur_bwmode;
    uint8   cur_ch_offset;//PRIME_CHNL_OFFSET
    uint8   cur_wireless_mode;
    uint8       max_chan_nums;
    WLAN_CHANNEL_INFO   channel_set[NUM_CHANNELS];

    _list       *pscanned;
    _queue  free_bss_pool;
    _queue  scanned_queue;
    uint8   *free_bss_buf;
    uint32  num_of_scanned;

    NDIS_802_11_SSID    assoc_ssid;
    uint8   assoc_bssid[6];

    struct wlan_network cur_network;

    struct WLAN_LINK_DETECT_T   link_detect_info;

    uint32  scan_interval;
    uint8   scan_mode;
    uint8   scan_from_ui;
    NDIS_802_11_SSID    scan_ssid;
    struct scan_result      scan_result;

    _timer  scan_timer;
    _timer  scan_timeout_timer;
    _timer  assoc_timer;
    _timer  link_retry_timer;
    _timer  dynamic_chk_timer;

    uint32  assoc_by_bssid;
    uint32  assoc_by_ui;

    struct qos_info     qos_inf;

    struct ht_cfg_struct    ht_cfg;
    uint8   num_sta_no_ht;
    uint8   bsupport_shortgi;
    uint8   bw40_enable;

    uint8   basic_rates[12];
    uint8   basic_rates_len;
    uint8   basic_rates_ext[16];
    uint8   basic_rates_ext_len;
    uint32  aid_bitmap;

    uint8       basicrate[NumRates];
    uint8       datarate[NumRates];

#ifdef CONFIG_LAYER2_ROAMING
    int32 to_roaming; // roaming trying times
#endif

    _lock   event_lock;
    uint8   event_seq;
    uint16  mgnt_seq;

    uint8           disconnect_from_ui;
    uint64      tsf_value;
    uint32          ap_tsf_low;
    uint32          bcn_shift;
    uint8   bcn_fail_count;
    uint32  nolink_idle_threshold;

    struct mlme_info    mlme_info;//for sta/adhoc mode, including current scanning/connecting/connected related info.
    //for ap mode, network includes ap's cap_info
};

struct setkey_parm
{
    uint8   addr[ETH_ALEN];
    uint8   algorithm;  // encryption algorithm, could be none, wep40, TKIP, CCMP, wep104
    uint8   keyid;
    uint8   grpkey;     // 1: this is the grpkey for 802.1x. 0: this is the unicast key for 802.1x
    uint8   key[16];    // this could be 40 or 104
    uint8   mic_key[16];
};

struct set_stakey_parm
{
    uint8   addr[ETH_ALEN];
    uint8   algorithm;
    uint8   key[16];
    uint8   mic_key[16];
};

struct set_stakey_rsp
{
    uint8   addr[ETH_ALEN];
    uint8   keyid;
    uint8   rsvd;
};

struct  setbasicrate_parm
{
    uint8   basicrates[NumRates];
};

struct setdatarate_parm
{
#ifdef MP_FIRMWARE_OFFLOAD
    uint32  curr_rateidx;
#else
    uint8   mac_id;
    uint8   datarates[NumRates];
#endif
};

struct addBaReq_parm
{
    uint32 tid;
    uint8   addr[ETH_ALEN];
};

struct joinbss_event
{
    struct  wlan_network    network;
};


#define NETWORK_QUEUE_SZ    4

struct network_queue
{
    volatile int    head;
    volatile int    tail;
    WLAN_BSSID_EX networks[NETWORK_QUEUE_SZ];
};

void wlan_add_network(ADAPTER *adapter, WLAN_BSSID_EX *pnetwork);
void wlan_free_network_nolock(struct mlme_struct *pmlme, struct wlan_network *pnetwork );

extern void wlan_free_network_queue(ADAPTER *adapter,uint8 isfreeall);
extern int32 wlan_init_mlme_struct(ADAPTER *adapter);// (struct mlme_struct *pmlme);

extern void wlan_free_mlme_struct (struct mlme_struct *pmlme);


extern uint32 wlan_set_key(ADAPTER *adapter,struct security_struct *psecurity, uint8 * hw_addr, uint32 keyid);
extern int32 wlan_set_auth(ADAPTER *adapter,struct security_struct *psecurity);

__inline static uint8 *get_bssid(struct mlme_struct *pmlme)
{
    //if sta_mode:pmlme->cur_network.network.MacAddress=> bssid
    // if adhoc_mode:pmlme->cur_network.network.MacAddress=> ibss mac address
    return pmlme->cur_network.network.MacAddress;
}

/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 *
 * ### NOTE:#### (!!!!)
 * MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED pmlme->lock
 */
/*
 * No Limit on the calling context,
 * therefore set it to be the critical section...
 */
__inline static void up_scanned_network(struct mlme_struct *pmlme)
{
    _irqL irqL;

    os_spin_lock_irqsave(&pmlme->lock, &irqL);
    pmlme->num_of_scanned++;
    os_spin_unlock_irqsave(&pmlme->lock, &irqL);
}

__inline static void down_scanned_network(struct mlme_struct *pmlme)
{
    _irqL irqL;

    os_spin_lock_irqsave(&pmlme->lock, &irqL);
    pmlme->num_of_scanned--;
    os_spin_unlock_irqsave(&pmlme->lock, &irqL);
}

__inline static void set_scanned_network_val(struct mlme_struct *pmlme, int32 val)
{
    _irqL irqL;

    os_spin_lock_irqsave(&pmlme->lock, &irqL);
    pmlme->num_of_scanned = val;
    os_spin_unlock_irqsave(&pmlme->lock, &irqL);
}

__inline static void update_network(WLAN_BSSID_EX *dst, WLAN_BSSID_EX *src,ADAPTER *Adapter)
{
    os_memcpy((uint8 *)dst, (uint8 *)src, get_WLAN_BSSID_EX_sz(src));
}


void wlan_update_current_network(ADAPTER *adapter, WLAN_BSSID_EX *pnetwork);
void wlan_update_scanned_network(ADAPTER *Adapter, WLAN_BSSID_EX *target);

extern struct wlan_network* wlan_find_network(_queue *scanned_queue, uint8 *addr);
extern struct wlan_network* wlan_find_network_by_ssid(_queue *scanned_queue, uint8 *ssid);
extern struct wlan_network* wlan_get_oldest_wlan_network(_queue *scanned_queue);

extern void wlan_free_assoc_resources(ADAPTER* Adapter);

uint32 wlan_receive_disconnect(ADAPTER *Adapter, uint8 *MacAddr);
extern void wlan_disconnect_indicate(ADAPTER* Adapter);
extern void wlan_connect_indicate(ADAPTER* Adapter);

void report_del_sta_event(ADAPTER *Adapter, uint8* MacAddr);
void report_add_sta_event(ADAPTER *Adapter, uint8* MacAddr, uint8 *Support_rate);

extern void wlan_init_config_dev_network(ADAPTER *Adapter);

extern void wlan_update_config_dev_network(ADAPTER *Adapter);

uint8 wlan_init_channel_set(IN PADAPTER Adapter);

extern int32 wlan_enqueue_network(_queue *queue, struct wlan_network *pnetwork);

extern struct wlan_network* wlan_dequeue_network(_queue *queue);

extern struct wlan_network* wlan_alloc_network(struct mlme_struct *pmlme);


extern void wlan_free_network(struct mlme_struct *pmlme, struct wlan_network *pnetwork,uint8 isfreeall);

extern struct wlan_network* wlan_find_network(_queue *scanned_queue, uint8 *addr);

extern void wlan_free_network_queue(ADAPTER* Adapter,uint8 isfreeall);

extern uint8 wlan_disassoc_proc(ADAPTER  *Adapter);

uint8 wlan_set_sta_key(ADAPTER *Adapter, uint8 *psta, uint8 unicast_key);

void wlan_reset_security(ADAPTER *adapter);

#ifdef CONFIG_LAYER2_ROAMING
void wlan_roaming(ADAPTER *Adapter, struct wlan_network *tgt_network);
#endif

#endif

