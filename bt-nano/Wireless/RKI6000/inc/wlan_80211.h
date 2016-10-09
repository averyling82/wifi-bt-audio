#ifndef _WLAN_80211_H_
#define _WLAN_80211_H_

#include "wlan_cfg.h"
#include "wlan_endian.h"

#define WPA_CIPHER_NONE     BIT(0)
#define WPA_CIPHER_WEP40    BIT(1)
#define WPA_CIPHER_WEP104 BIT(2)
#define WPA_CIPHER_TKIP     BIT(3)
#define WPA_CIPHER_CCMP     BIT(4)
#define WPA_SELECTOR_LEN    4
#define RSN_SELECTOR_LEN    4


#define IEEE80211_3ADDR_LEN 24
#define IEEE80211_4ADDR_LEN 30
#define IEEE80211_FCS_LEN    4

#define MIN_FRAG_THRESHOLD  256U
#define MAX_FRAG_THRESHOLD  2346U



/* QoS,QOS */
#define NORMAL_ACK          0
#define NO_ACK              1
#define NON_EXPLICIT_ACK    2
#define BLOCK_ACK           3

#define FRAME_OFFSET_FRAME_CONTROL      0
#define FRAME_OFFSET_DURATION           2
#define FRAME_OFFSET_ADDRESS1           4
#define FRAME_OFFSET_ADDRESS2           10
#define FRAME_OFFSET_ADDRESS3           16
#define FRAME_OFFSET_SEQUENCE           22
#define FRAME_OFFSET_ADDRESS4           24

#define WLAN_ETHHDR_LEN     14
#define WLAN_ETHADDR_LEN    6
#define WLAN_IEEE_OUI_LEN   3
#define WLAN_ADDR_LEN       6
#define WLAN_CRC_LEN        4
#define WLAN_BSSID_LEN      6
#define WLAN_BSS_TS_LEN     8
#define WLAN_HDR_A3_LEN     24
#define WLAN_HDR_A4_LEN     30
#define WLAN_HDR_A3_QOS_LEN 26
#define WLAN_HDR_A4_QOS_LEN 32
#define WLAN_SSID_MAXLEN    32
#define WLAN_DATA_MAXLEN    2312

#define WLAN_A3_PN_OFFSET   24
#define WLAN_A4_PN_OFFSET   30

#define WLAN_MIN_ETHFRM_LEN 60
#define WLAN_MAX_ETHFRM_LEN 1514
#define WLAN_ETHHDR_LEN     14

#define P80211CAPTURE_VERSION   0x80211001

#define MACID_INFRA_UNI 0

/* IEEE 802.11 defines */

#define P80211_OUI_LEN 3





/* Authentication algorithms */
#define WLAN_AUTH_OPEN 0
#define WLAN_AUTH_SHARED_KEY 1

#define WLAN_AUTH_CHALLENGE_LEN 128

#define WLAN_CAPABILITY_BSS (1<<0)
#define WLAN_CAPABILITY_IBSS (1<<1)
#define WLAN_CAPABILITY_CF_POLLABLE (1<<2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST (1<<3)
#define WLAN_CAPABILITY_PRIVACY (1<<4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE (1<<5)
#define WLAN_CAPABILITY_PBCC (1<<6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY (1<<7)
#define WLAN_CAPABILITY_SHORT_SLOT (1<<10)

/* Status codes */
#define WLAN_STATUS_SUCCESSFUL      0
#define WLAN_STATUS_UNSPECIFIED_FAILURE 1
#define WLAN_STATUS_CAPS_UNSUPPORTED 10
#define WLAN_STATUS_REASSOC_NO_ASSOC 11
#define WLAN_STATUS_ASSOC_DENIED_UNSPEC 12
#define WLAN_STATUS_NOT_SUPPORTED_AUTH_ALG 13
#define WLAN_STATUS_UNKNOWN_AUTH_TRANSACTION 14
#define WLAN_STATUS_CHALLENGE_FAIL 15
#define WLAN_STATUS_AUTH_TIMEOUT 16
#define WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA 17
#define WLAN_STATUS_ASSOC_DENIED_RATES 18
/* 802.11b */
#define WLAN_STATUS_ASSOC_DENIED_NOSHORT 19
#define WLAN_STATUS_ASSOC_DENIED_NOPBCC 20
#define WLAN_STATUS_ASSOC_DENIED_NOAGILITY 21

/* Reason codes */
#define WLAN_REASON_UNSPECIFIED 1
#define WLAN_REASON_PREV_AUTH_NOT_VALID 2
#define WLAN_REASON_DEAUTH_LEAVING 3
#define WLAN_REASON_DISASSOC_DUE_TO_INACTIVITY 4
#define WLAN_REASON_DISASSOC_AP_BUSY 5
#define WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA 6
#define WLAN_REASON_CLASS3_FRAME_FROM_NONASSOC_STA 7
#define WLAN_REASON_DISASSOC_STA_HAS_LEFT 8
#define WLAN_REASON_STA_REQ_ASSOC_WITHOUT_AUTH 9


/* Information Element IDs */
#define WLAN_EID_SSID 0
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_FH_PARAMS 2
#define WLAN_EID_DS_PARAMS 3
#define WLAN_EID_CF_PARAMS 4
#define WLAN_EID_TIM 5
#define WLAN_EID_IBSS_PARAMS 6
#define WLAN_EID_CHALLENGE 16
#define WLAN_EID_RSN 48
#define WLAN_EID_GENERIC 221

#define IEEE80211_MGMT_HDR_LEN 24
#define IEEE80211_DATA_HDR3_LEN 24
#define IEEE80211_DATA_HDR4_LEN 30


#define IEEE80211_STATMASK_SIGNAL (1<<0)
#define IEEE80211_STATMASK_RSSI (1<<1)
#define IEEE80211_STATMASK_NOISE (1<<2)
#define IEEE80211_STATMASK_RATE (1<<3)
#define IEEE80211_STATMASK_WEMASK 0x7


#define IEEE80211_CCK_MODULATION    (1<<0)
#define IEEE80211_OFDM_MODULATION   (1<<1)

#define IEEE80211_24GHZ_BAND     (1<<0)
#define IEEE80211_52GHZ_BAND     (1<<1)

#define IEEE80211_CCK_RATE_LEN          4
#define IEEE80211_NUM_OFDM_RATESLEN 8


#define IEEE80211_CCK_RATE_1MB              0x02
#define IEEE80211_CCK_RATE_2MB              0x04
#define IEEE80211_CCK_RATE_5MB              0x0B
#define IEEE80211_CCK_RATE_11MB             0x16
#define IEEE80211_OFDM_RATE_LEN         8
#define IEEE80211_OFDM_RATE_6MB             0x0C
#define IEEE80211_OFDM_RATE_9MB             0x12
#define IEEE80211_OFDM_RATE_12MB        0x18
#define IEEE80211_OFDM_RATE_18MB        0x24
#define IEEE80211_OFDM_RATE_24MB        0x30
#define IEEE80211_OFDM_RATE_36MB        0x48
#define IEEE80211_OFDM_RATE_48MB        0x60
#define IEEE80211_OFDM_RATE_54MB        0x6C
#define IEEE80211_BASIC_RATE_MASK       0x80

#define IEEE80211_CCK_RATE_1MB_MASK     (1<<0)
#define IEEE80211_CCK_RATE_2MB_MASK     (1<<1)
#define IEEE80211_CCK_RATE_5MB_MASK     (1<<2)
#define IEEE80211_CCK_RATE_11MB_MASK        (1<<3)
#define IEEE80211_OFDM_RATE_6MB_MASK        (1<<4)
#define IEEE80211_OFDM_RATE_9MB_MASK        (1<<5)
#define IEEE80211_OFDM_RATE_12MB_MASK       (1<<6)
#define IEEE80211_OFDM_RATE_18MB_MASK       (1<<7)
#define IEEE80211_OFDM_RATE_24MB_MASK       (1<<8)
#define IEEE80211_OFDM_RATE_36MB_MASK       (1<<9)
#define IEEE80211_OFDM_RATE_48MB_MASK       (1<<10)
#define IEEE80211_OFDM_RATE_54MB_MASK       (1<<11)

#define IEEE80211_CCK_RATES_MASK            0x0000000F
#define IEEE80211_CCK_BASIC_RATES_MASK  (IEEE80211_CCK_RATE_1MB_MASK | \
    IEEE80211_CCK_RATE_2MB_MASK)
#define IEEE80211_CCK_DEFAULT_RATES_MASK    (IEEE80211_CCK_BASIC_RATES_MASK | \
        IEEE80211_CCK_RATE_5MB_MASK | \
        IEEE80211_CCK_RATE_11MB_MASK)

#define IEEE80211_OFDM_RATES_MASK       0x00000FF0
#define IEEE80211_OFDM_BASIC_RATES_MASK (IEEE80211_OFDM_RATE_6MB_MASK | \
    IEEE80211_OFDM_RATE_12MB_MASK | \
    IEEE80211_OFDM_RATE_24MB_MASK)
#define IEEE80211_OFDM_DEFAULT_RATES_MASK   (IEEE80211_OFDM_BASIC_RATES_MASK | \
    IEEE80211_OFDM_RATE_9MB_MASK  | \
    IEEE80211_OFDM_RATE_18MB_MASK | \
    IEEE80211_OFDM_RATE_36MB_MASK | \
    IEEE80211_OFDM_RATE_48MB_MASK | \
    IEEE80211_OFDM_RATE_54MB_MASK)
#define IEEE80211_DEFAULT_RATES_MASK (IEEE80211_OFDM_DEFAULT_RATES_MASK | \
                                IEEE80211_CCK_DEFAULT_RATES_MASK)

#define IEEE80211_NUM_OFDM_RATES        8
#define IEEE80211_NUM_CCK_RATES             4
#define IEEE80211_OFDM_SHIFT_MASK_A         4

/* IEEE 802.11 requires that STA supports concurrent reception of at least
 * three fragmented frames. This define can be increased to support more
 * concurrent frames, but it should be noted that each entry can consume about
 * 2 kB of RAM and increasing cache size will slow down frame reassembly. */
#define IEEE80211_FRAG_CACHE_LEN 4

#define WEP_KEYS 4
#define WEP_KEY_LEN 13

/*

 802.11 data frame from AP

      ,-------------------------------------------------------------------.
Bytes |  2   |  2   |    6    |    6    |    6    |  2   | 0..2312 |   4  |
      |------|------|---------|---------|---------|------|---------|------|
Desc. | ctrl | dura |  DA/RA  |   TA    |    SA   | Sequ |  frame  |  fcs |
      |      | tion | (BSSID) |         |         | ence |  data   |      |
      `-------------------------------------------------------------------'

Total: 28-2340 bytes

*/

/* MAX_RATES_LENGTH needs to be 12.  The spec says 8, and many APs
 * only use 8, and then use extended rates for the remaining supported
 * rates.  Other APs, however, stick all of their supported rates on the
 * main rates information element... */
#define MAX_RATES_LENGTH                  ((uint8)12)
#define MAX_RATES_EX_LENGTH               ((uint8)16)

#define MAX_WPA_IE_LEN 128

#define IW_ESSID_MAX_SIZE 32

#define MAXTID  16

/* Frame control field constants */
#define IEEE80211_FCTL_VERS     0x0002
#define IEEE80211_FCTL_FTYPE        0x000c
#define IEEE80211_FCTL_STYPE        0x00f0
#define IEEE80211_FCTL_TODS     0x0100
#define IEEE80211_FCTL_FROMDS       0x0200
#define IEEE80211_FCTL_MOREFRAGS    0x0400
#define IEEE80211_FCTL_RETRY        0x0800
#define IEEE80211_FCTL_PM       0x1000
#define IEEE80211_FCTL_MOREDATA 0x2000
#define IEEE80211_FCTL_WEP      0x4000
#define IEEE80211_FCTL_ORDER        0x8000

#define IEEE80211_FTYPE_MGMT        0x0000
#define IEEE80211_FTYPE_CTL     0x0004
#define IEEE80211_FTYPE_DATA        0x0008

/* management */
#define IEEE80211_STYPE_ASSOC_REQ   0x0000
#define IEEE80211_STYPE_ASSOC_RESP  0x0010
#define IEEE80211_STYPE_REASSOC_REQ 0x0020
#define IEEE80211_STYPE_REASSOC_RESP    0x0030
#define IEEE80211_STYPE_PROBE_REQ   0x0040
#define IEEE80211_STYPE_PROBE_RESP  0x0050
#define IEEE80211_STYPE_BEACON      0x0080
#define IEEE80211_STYPE_ATIM        0x0090
#define IEEE80211_STYPE_DISASSOC    0x00A0
#define IEEE80211_STYPE_AUTH        0x00B0
#define IEEE80211_STYPE_DEAUTH      0x00C0

/* control */
#define IEEE80211_STYPE_PSPOLL      0x00A0
#define IEEE80211_STYPE_RTS     0x00B0
#define IEEE80211_STYPE_CTS     0x00C0
#define IEEE80211_STYPE_ACK     0x00D0
#define IEEE80211_STYPE_CFEND       0x00E0
#define IEEE80211_STYPE_CFENDACK    0x00F0

/* data */
#define IEEE80211_STYPE_DATA        0x0000
#define IEEE80211_STYPE_DATA_CFACK  0x0010
#define IEEE80211_STYPE_DATA_CFPOLL 0x0020
#define IEEE80211_STYPE_DATA_CFACKPOLL  0x0030
#define IEEE80211_STYPE_NULLFUNC    0x0040
#define IEEE80211_STYPE_CFACK       0x0050
#define IEEE80211_STYPE_CFPOLL      0x0060
#define IEEE80211_STYPE_CFACKPOLL   0x0070
#define IEEE80211_QOS_DATAGRP       0x0080
#define IEEE80211_QoS_DATAGRP       IEEE80211_QOS_DATAGRP

#define IEEE80211_SCTL_FRAG     0x000F
#define IEEE80211_SCTL_SEQ      0xFFF0

#define WLAN_FC_GET_TYPE(fc) ((fc) & IEEE80211_FCTL_FTYPE)
#define WLAN_FC_GET_STYPE(fc) ((fc) & IEEE80211_FCTL_STYPE)
#define WLAN_QC_GET_TID(qc) ((qc) & 0x0f)
#define WLAN_GET_SEQ_FRAG(seq) ((seq) & IEEE80211_SCTL_FRAG)
#define WLAN_GET_SEQ_SEQ(seq)  ((seq) & IEEE80211_SCTL_SEQ)

//
// Queue Select Value in TxDesc
//
#define QSLT_BK                         0x2//0x01
#define QSLT_BE                         0x0
#define QSLT_VI                         0x5//0x4
#define QSLT_VO                         0x7//0x6
#define QSLT_BEACON                     0x10
#define QSLT_HIGH                       0x11
#define QSLT_MGNT                       0x12
#define QSLT_CMD                        0x13

#define TRY_AP_TIMES    3   //6

/* Action category code */
enum ieee80211_category
{
    WLAN_CATEGORY_SPECTRUM_MGMT = 0,
    WLAN_CATEGORY_QOS = 1,
    WLAN_CATEGORY_DLS = 2,
    WLAN_CATEGORY_BA = 3,
    WLAN_CATEGORY_PUBLIC = 4, //P2P public action frames
    WLAN_CATEGORY_RADIO_MEASUREMENT  = 5,
    WLAN_CATEGORY_FT = 6,
    WLAN_CATEGORY_HT = 7,
    WLAN_CATEGORY_SA_QUERY = 8,
    WLAN_CATEGORY_WMM = 17
};

enum _PUBLIC_ACTION
{
    ACT_PUBLIC_BSSCOEXIST = 0,  // 20/40 BSS Coexistence
    ACT_PUBLiC_MP = 7           // Measurement Pilot
};

/* BACK action code */
enum ieee80211_back_actioncode
{
    WLAN_ACTION_ADDBA_REQ = 0,
    WLAN_ACTION_ADDBA_RESP = 1,
    WLAN_ACTION_DELBA = 2
};

enum WIFI_FRAME_TYPE
{
    WIFI_MGT_TYPE  =    (0),
    WIFI_CTRL_TYPE =    (BIT(2)),
    WIFI_DATA_TYPE =    (BIT(3))
};


typedef enum _WLAN_MEDIA_STATUS
{
    WLAN_MEDIA_DISCONNECT   = 0,
    WLAN_MEDIA_CONNECT          = 1
} WLAN_MEDIA_STATUS;


enum WIFI_FRAME_SUBTYPE
{

    // below is for mgt frame
    WIFI_ASSOCREQ       = (0 | WIFI_MGT_TYPE),
    WIFI_ASSOCRSP       = (BIT(4) | WIFI_MGT_TYPE),
    WIFI_REASSOCREQ     = (BIT(5) | WIFI_MGT_TYPE),
    WIFI_REASSOCRSP     = (BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_PROBEREQ       = (BIT(6) | WIFI_MGT_TYPE),
    WIFI_PROBERSP       = (BIT(6) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_BEACON         = (BIT(7) | WIFI_MGT_TYPE),
    WIFI_ATIM           = (BIT(7) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DISASSOC       = (BIT(7) | BIT(5) | WIFI_MGT_TYPE),
    WIFI_AUTH           = (BIT(7) | BIT(5) | BIT(4) | WIFI_MGT_TYPE),
    WIFI_DEAUTH         = (BIT(7) | BIT(6) | WIFI_MGT_TYPE),
    WIFI_ACTION         = (BIT(7) | BIT(6) | BIT(4) | WIFI_MGT_TYPE),

    // below is for control frame
    WIFI_PSPOLL         = (BIT(7) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_RTS            = (BIT(7) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CTS            = (BIT(7) | BIT(6) | WIFI_CTRL_TYPE),
    WIFI_ACK            = (BIT(7) | BIT(6) | BIT(4) | WIFI_CTRL_TYPE),
    WIFI_CFEND          = (BIT(7) | BIT(6) | BIT(5) | WIFI_CTRL_TYPE),
    WIFI_CFEND_CFACK    = (BIT(7) | BIT(6) | BIT(5) | BIT(4) | WIFI_CTRL_TYPE),

    // below is for data frame
    WIFI_DATA           = (0 | WIFI_DATA_TYPE),
    WIFI_DATA_CFACK     = (BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_CFPOLL    = (BIT(5) | WIFI_DATA_TYPE),
    WIFI_DATA_CFACKPOLL = (BIT(5) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_DATA_NULL      = (BIT(6) | WIFI_DATA_TYPE),
    WIFI_QOS_DATA_TYPE  = (BIT(7)|WIFI_DATA_TYPE),
    WIFI_QOSDATA_NULL   = (BIT(7) | BIT(6) | WIFI_DATA_TYPE),
    WIFI_CF_ACK         = (BIT(6) | BIT(4) | WIFI_DATA_TYPE),
    WIFI_CF_POLL        = (BIT(6) | BIT(5) | WIFI_DATA_TYPE),
    WIFI_CF_ACKPOLL     = (BIT(6) | BIT(5) | BIT(4) | WIFI_DATA_TYPE)
};

enum WIFI_REASON_CODE
{
    _RSON_RESERVED_                 = 0,
    _RSON_UNSPECIFIED_              = 1,
    _RSON_AUTH_NO_LONGER_VALID_     = 2,
    _RSON_DEAUTH_STA_LEAVING_       = 3,
    _RSON_INACTIVITY_               = 4,
    _RSON_UNABLE_HANDLE_            = 5,
    _RSON_CLS2_                     = 6,
    _RSON_CLS3_                     = 7,
    _RSON_DISAOC_STA_LEAVING_       = 8,
    _RSON_ASOC_NOT_AUTH_            = 9,

    // WPA reason
    _RSON_INVALID_IE_               = 13,
    _RSON_MIC_FAILURE_              = 14,
    _RSON_4WAY_HNDSHK_TIMEOUT_      = 15,
    _RSON_GROUP_KEY_UPDATE_TIMEOUT_ = 16,
    _RSON_DIFF_IE_                  = 17,
    _RSON_MLTCST_CIPHER_NOT_VALID_  = 18,
    _RSON_UNICST_CIPHER_NOT_VALID_  = 19,
    _RSON_AKMP_NOT_VALID_           = 20,
    _RSON_UNSUPPORT_RSNE_VER_       = 21,
    _RSON_INVALID_RSNE_CAP_         = 22,
    _RSON_IEEE_802DOT1X_AUTH_FAIL_  = 23,

    _RSON_PMK_NOT_AVAILABLE_        = 24
};

enum WIFI_STATUS_CODE
{
    _STATS_SUCCESSFUL_          = 0,
    _STATS_FAILURE_             = 1,
    _STATS_CAP_FAIL_            = 10,
    _STATS_NO_ASOC_             = 11,
    _STATS_OTHER_               = 12,
    _STATS_NO_SUPP_ALG_         = 13,
    _STATS_OUT_OF_AUTH_SEQ_     = 14,
    _STATS_CHALLENGE_FAIL_      = 15,
    _STATS_AUTH_TIMEOUT_        = 16,
    _STATS_UNABLE_HANDLE_STA_   = 17,
    _STATS_RATE_FAIL_           = 18
};

enum WIFI_REG_DOMAIN
{
    DOMAIN_FCC      = 1,
    DOMAIN_IC       = 2,
    DOMAIN_ETSI     = 3,
    DOMAIN_SPAIN    = 4,
    DOMAIN_FRANCE   = 5,
    DOMAIN_MKK      = 6,
    DOMAIN_ISRAEL   = 7,
    DOMAIN_MKK1     = 8,
    DOMAIN_MKK2     = 9,
    DOMAIN_MKK3     = 10,
    DOMAIN_MAX
};

typedef enum _NETWORK_TYPE
{
    WIRELESS_INVALID    = 0,
    WIRELESS_11B = 1,
    WIRELESS_11G = 2,
    WIRELESS_11BG = (WIRELESS_11B|WIRELESS_11G),
    WIRELESS_11A = 4,
    WIRELESS_MC = 7,
    WIRELESS_11N = 8,
    WIRELESS_11GN = (WIRELESS_11G|WIRELESS_11N),
    WIRELESS_11AN = (WIRELESS_11A|WIRELESS_11N),
    WIRELESS_11BGN = (WIRELESS_11B|WIRELESS_11G|WIRELESS_11N)
} NETWORK_TYPE;


typedef enum _RATR_TABLE_MODE
{
    RATR_INX_WIRELESS_NGB = 0,
    RATR_INX_WIRELESS_NG = 1,
    RATR_INX_WIRELESS_NB = 2,
    RATR_INX_WIRELESS_N = 3,
    RATR_INX_WIRELESS_GB = 4,
    RATR_INX_WIRELESS_G = 5,
    RATR_INX_WIRELESS_B = 6,
    RATR_INX_WIRELESS_MC = 7,
    RATR_INX_WIRELESS_A = 8
} RATR_TABLE_MODE, *PRATR_TABLE_MODE;

enum eap_type
{
    EAP_PACKET = 0,
    EAPOL_START,
    EAPOL_LOGOFF,
    EAPOL_KEY,
    EAPOL_ENCAP_ASF_ALERT
};

typedef enum _RX_PACKET_TYPE
{
    NORMAL_RX,
    TX_REPORT1,
    TX_REPORT2,
    HIS_REPORT
} RX_PACKET_TYPE, *PRX_PACKET_TYPE;

__packed struct ieee80211_snap_hdr
{
    uint8    dsap;   /* always 0xAA */
    uint8    ssap;   /* always 0xAA */
    uint8    ctrl;   /* always 0x03 */
    uint8    oui[P80211_OUI_LEN];    /* organizational universal id */

};

__packed struct ieee80211_hdr
{
    uint16 frame_ctl;
    uint16 duration_id;
    uint8 addr1[ETH_ALEN];
    uint8 addr2[ETH_ALEN];
    uint8 addr3[ETH_ALEN];
    uint16 seq_ctl;
    uint8 addr4[ETH_ALEN];
};

__packed struct ieee80211_hdr_3addr
{
    uint16 frame_ctl;
    uint16 duration_id;
    uint8 addr1[ETH_ALEN];
    uint8 addr2[ETH_ALEN];
    uint8 addr3[ETH_ALEN];
    uint16 seq_ctl;
};


__packed struct ieee80211_hdr_qos
{
    struct  ieee80211_hdr wlan_hdr;
    uint16  qc;
};

__packed struct  ieee80211_hdr_3addr_qos
{
    struct  ieee80211_hdr_3addr wlan_hdr;
    uint16     qc;
};

__packed struct eapol
{
    uint8 snap[6];
    uint16 ethertype;
    uint8 version;
    uint8 type;
    uint16 length;
};

typedef struct pkt_buff
{
    _list list;
    //sk_buff_data_t transport_header;
    //sk_buff_data_t network_header;
    //sk_buff_data_t mac_header;
    uint32 len;
    //uint8 *tail;
    //uint8 *end;
    //uint8 *head;
    uint8 *end;//fixed can not change after alloc
    uint8 *head;//fixed can not change after alloc
    uint8 *data;//data start
} _pkt;

#define SNAP_SIZE sizeof(struct ieee80211_snap_hdr)

#define _TO_DS_     BIT(8)
#define _FROM_DS_   BIT(9)
#define _MORE_FRAG_ BIT(10)
#define _RETRY_     BIT(11)
#define _PWRMGT_    BIT(12)
#define _MORE_DATA_ BIT(13)
#define _PRIVACY_   BIT(14)
#define _ORDER_         BIT(15)

#if 0 // guangan 20140621

#define SetToDs(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_TO_DS_); \
    } while(0)

#define GetToDs(pbuf)   ((((*(uint16 *)(pbuf)) & be16_to_cpu(_TO_DS_)) != 0) ? 1 : 0)

#define ClearToDs(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_TO_DS_)); \
    } while(0)

#define SetFrDs(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_FROM_DS_); \
    } while(0)

#define GetFrDs(pbuf)       ((((*(uint16 *)(pbuf)) & be16_to_cpu(_FROM_DS_)) != 0) ? 1 : 0)

#define ClearFrDs(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_FROM_DS_)); \
    } while(0)

#define get_tofr_ds(pframe) ((GetToDs(pframe) << 1) | GetFrDs(pframe))


#define SetMFrag(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_MORE_FRAG_); \
    } while(0)

#define GetMFrag(pbuf)  (((*(uint16 *)(pbuf)) & be16_to_cpu(_MORE_FRAG_)) != 0)

#define ClearMFrag(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_MORE_FRAG_)); \
    } while(0)

#define SetRetry(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_RETRY_); \
    } while(0)

#define GetRetry(pbuf)  (((*(uint16 *)(pbuf)) & be16_to_cpu(_RETRY_)) != 0)

#define ClearRetry(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_RETRY_)); \
    } while(0)

#define SetPwrMgt(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_PWRMGT_); \
    } while(0)

#define GetPwrMgt(pbuf) (((*(uint16 *)(pbuf)) & be16_to_cpu(_PWRMGT_)) != 0)

#define ClearPwrMgt(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_PWRMGT_)); \
    } while(0)

#define SetMData(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_MORE_DATA_); \
    } while(0)

#define GetMData(pbuf)  (((*(uint16 *)(pbuf)) & be16_to_cpu(_MORE_DATA_)) != 0)

#define ClearMData(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_MORE_DATA_)); \
    } while(0)

#define SetPrivacy(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(_PRIVACY_); \
    } while(0)

#define GetPrivacy(pbuf)    (((*(uint16 *)(pbuf)) & be16_to_cpu(_PRIVACY_)) != 0)

#define ClearPrivacy(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_be16(_PRIVACY_)); \
    } while(0)


#define GetOrder(pbuf)  (((*(uint16 *)(pbuf)) & be16_to_cpu(_ORDER_)) != 0)

#define GetFrameType(pbuf)  (be16_to_cpu(*(uint16 *)(pbuf)) & (BIT(3) | BIT(2)))

#define SetFrameType(pbuf,type) \
    do {    \
        *(uint16 *)(pbuf) &= __constant_cpu_to_be16(~(BIT(3) | BIT(2))); \
        *(uint16 *)(pbuf) |= __constant_cpu_to_be16(type); \
    } while(0)

#define GetFrameSubType(pbuf)   (cpu_to_be16(*(uint16 *)(pbuf)) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))

#define SetFrameSubType(pbuf,type) \
do {    \
    *(uint16 *)(pbuf) &= cpu_to_be16((uint16)(~(BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))); \
    *(uint16 *)(pbuf) |= cpu_to_be16((uint16)type); \
} while(0)

#define GetSequence(pbuf)   (cpu_to_be16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)) >> 4)

#define GetFragNum(pbuf)    (cpu_to_be16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & 0x0f)

#define GetTupleCache(pbuf) (cpu_to_be16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)))

#define SetFragNum(pbuf, num) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 22) = \
            ((*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & be16_to_cpu((uint16)(~0x000f))) | \
            cpu_to_be16((uint16)(0x000f) & (num));     \
    } while(0)


#define SetSeqNum(pbuf, num) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 22) = \
            ((*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & be16_to_cpu((uint16)~0xfff0)) | \
            be16_to_cpu((uint16)(0xfff0 & (num << 4))); \
    } while(0)


#define SetDuration(pbuf, dur) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 2) = cpu_to_be16(0xffff & (dur)); \
    } while(0)


#define SetPriority(pbuf, tid)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16(tid & 0xf); \
    } while(0)

#define GetPriority(pbuf)   ((be16_to_cpu(*(uint16 *)(pbuf))) & 0xf)

#define SetAckpolicy(pbuf, ack) \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16( (ack & 3) << 5); \
    } while(0)

#define GetAckpolicy(pbuf) (((be16_to_cpu(*(uint16 *)pbuf)) >> 5) & 0x3)

#define GetAMsdu(pbuf) (((be16_to_cpu(*(uint16 *)pbuf)) >> 7) & 0x1)

#define SetAMsdu(pbuf, amsdu)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_be16( (amsdu & 1) << 7); \
    } while(0)

#define GetAid(pbuf)    (cpu_to_be16(*(uint16 *)((SIZE_PTR)(pbuf) + 2)) & 0x3fff)

#define GetTid(pbuf)    (cpu_to_be16(*(uint16 *)((SIZE_PTR)(pbuf) + (((GetToDs(pbuf)<<1)|GetFrDs(pbuf))==3?30:24))) & 0x000f)

#else

#define SetToDs(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_TO_DS_); \
    } while(0)

#define GetToDs(pbuf)   ((((*(uint16 *)(pbuf)) & le16_to_cpu(_TO_DS_)) != 0) ? 1 : 0)

#define ClearToDs(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_TO_DS_)); \
    } while(0)

#define SetFrDs(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_FROM_DS_); \
    } while(0)

#define GetFrDs(pbuf)       ((((*(uint16 *)(pbuf)) & le16_to_cpu(_FROM_DS_)) != 0) ? 1 : 0)

#define ClearFrDs(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_FROM_DS_)); \
    } while(0)

#define get_tofr_ds(pframe) ((GetToDs(pframe) << 1) | GetFrDs(pframe))


#define SetMFrag(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_MORE_FRAG_); \
    } while(0)

#define GetMFrag(pbuf)  (((*(uint16 *)(pbuf)) & le16_to_cpu(_MORE_FRAG_)) != 0)

#define ClearMFrag(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_MORE_FRAG_)); \
    } while(0)

#define SetRetry(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_RETRY_); \
    } while(0)

#define GetRetry(pbuf)  (((*(uint16 *)(pbuf)) & le16_to_cpu(_RETRY_)) != 0)

#define ClearRetry(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_RETRY_)); \
    } while(0)

#define SetPwrMgt(pbuf) \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_PWRMGT_); \
    } while(0)

#define GetPwrMgt(pbuf) (((*(uint16 *)(pbuf)) & le16_to_cpu(_PWRMGT_)) != 0)

#define ClearPwrMgt(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_PWRMGT_)); \
    } while(0)

#define SetMData(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_MORE_DATA_); \
    } while(0)

#define GetMData(pbuf)  (((*(uint16 *)(pbuf)) & le16_to_cpu(_MORE_DATA_)) != 0)

#define ClearMData(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_MORE_DATA_)); \
    } while(0)

#define SetPrivacy(pbuf)    \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_PRIVACY_); \
    } while(0)

#define GetPrivacy(pbuf)    (((*(uint16 *)(pbuf)) & le16_to_cpu(_PRIVACY_)) != 0)

#define ClearPrivacy(pbuf)  \
    do  {   \
        *(uint16 *)(pbuf) &= (~cpu_to_le16(_PRIVACY_)); \
    } while(0)


#define GetOrder(pbuf)  (((*(uint16 *)(pbuf)) & le16_to_cpu(_ORDER_)) != 0)

#define GetFrameType(pbuf)  (le16_to_cpu(*(uint16 *)(pbuf)) & (BIT(3) | BIT(2)))

#define SetFrameType(pbuf,type) \
    do {    \
        *(uint16 *)(pbuf) &= __constant_cpu_to_le16(~(BIT(3) | BIT(2))); \
        *(uint16 *)(pbuf) |= __constant_cpu_to_le16(type); \
    } while(0)

#define GetFrameSubType(pbuf)   (cpu_to_le16(*(uint16 *)(pbuf)) & (BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))

#define SetFrameSubType(pbuf,type) \
do {    \
    *(uint16 *)(pbuf) &= cpu_to_le16((uint16)(~(BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3) | BIT(2)))); \
    *(uint16 *)(pbuf) |= cpu_to_le16((uint16)type); \
} while(0)

#define GetSequence(pbuf)   (cpu_to_le16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)) >> 4)

#define GetFragNum(pbuf)    (cpu_to_le16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & 0x0f)

#define GetTupleCache(pbuf) (cpu_to_le16(*(uint16 *)((SIZE_PTR)(pbuf) + 22)))

#define SetFragNum(pbuf, num) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 22) = \
            ((*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & le16_to_cpu((uint16)(~0x000f))) | \
            cpu_to_le16((uint16)(0x000f) & (num));     \
    } while(0)


#define SetSeqNum(pbuf, num) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 22) = \
            ((*(uint16 *)((SIZE_PTR)(pbuf) + 22)) & le16_to_cpu((uint16)~0xfff0)) | \
            le16_to_cpu((uint16)(0xfff0 & (num << 4))); \
    } while(0)


#define SetDuration(pbuf, dur) \
    do {    \
        *(uint16 *)((SIZE_PTR)(pbuf) + 2) = cpu_to_le16(0xffff & (dur)); \
    } while(0)


#define SetPriority(pbuf, tid)  \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(tid & 0xf); \
    } while(0)

#define GetPriority(pbuf)   ((le16_to_cpu(*(uint16 *)(pbuf))) & 0xf)

#define SetEOSP(pbuf, eosp) \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16( (eosp & 1) << 4); \
    } while(0)

#define SetAckpolicy(pbuf, ack) \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16( (ack & 3) << 5); \
    } while(0)

#define GetAckpolicy(pbuf) (((le16_to_cpu(*(uint16 *)pbuf)) >> 5) & 0x3)

#define GetAMsdu(pbuf) (((le16_to_cpu(*(uint16 *)pbuf)) >> 7) & 0x1)

#define SetAMsdu(pbuf, amsdu)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16( (amsdu & 1) << 7); \
    } while(0)

#define GetAid(pbuf)    (cpu_to_le16(*(uint16 *)((SIZE_PTR)(pbuf) + 2)) & 0x3fff)

#define GetTid(pbuf)    (cpu_to_le16(*(uint16 *)((SIZE_PTR)(pbuf) + (((GetToDs(pbuf)<<1)|GetFrDs(pbuf))==3?30:24))) & 0x000f)

#endif

#define GetAddr1Ptr(pbuf)   ((uint8 *)((SIZE_PTR)(pbuf) + 4))

#define GetAddr2Ptr(pbuf)   ((uint8 *)((SIZE_PTR)(pbuf) + 10))

#define GetAddr3Ptr(pbuf)   ((uint8 *)((SIZE_PTR)(pbuf) + 16))

#define GetAddr4Ptr(pbuf)   ((uint8 *)((SIZE_PTR)(pbuf) + 24))

#define MacAddr_isBcst(addr) \
( \
    ( (addr[0] == 0xff) && (addr[1] == 0xff) && \
        (addr[2] == 0xff) && (addr[3] == 0xff) && \
        (addr[4] == 0xff) && (addr[5] == 0xff) )  ? _TRUE : _FALSE \
)


__inline static int32 IS_MCAST(uint8 *da)
{
    if ((*da) & 0x01)
        return _TRUE;
    else
        return _FALSE;
}


__inline static uint8 * get_da(uint8 *pframe)
{
    uint8   *da;
    uint32  to_fr_ds    = (GetToDs(pframe) << 1) | GetFrDs(pframe);

    switch (to_fr_ds)
    {
        case 0x00:  // ToDs=0, FromDs=0
            da = GetAddr1Ptr(pframe);
            break;
        case 0x01:  // ToDs=0, FromDs=1
            da = GetAddr1Ptr(pframe);
            break;
        case 0x02:  // ToDs=1, FromDs=0
            da = GetAddr3Ptr(pframe);
            break;
        default:    // ToDs=1, FromDs=1
            da = GetAddr3Ptr(pframe);
            break;
    }

    return da;
}


__inline static uint8 * get_sa(uint8 *pframe)
{
    uint8   *sa;
    uint32  to_fr_ds    = (GetToDs(pframe) << 1) | GetFrDs(pframe);

    switch (to_fr_ds)
    {
        case 0x00:  // ToDs=0, FromDs=0
            sa = GetAddr2Ptr(pframe);
            break;
        case 0x01:  // ToDs=0, FromDs=1
            sa = GetAddr3Ptr(pframe);
            break;
        case 0x02:  // ToDs=1, FromDs=0
            sa = GetAddr2Ptr(pframe);
            break;
        default:    // ToDs=1, FromDs=1
            sa = GetAddr4Ptr(pframe);
            break;
    }

    return sa;
}

__inline static uint8 * get_hdr_bssid(uint8 *pframe)
{
    uint8   *sa;
    uint32  to_fr_ds    = (GetToDs(pframe) << 1) | GetFrDs(pframe);

    switch (to_fr_ds)
    {
        case 0x00:  // ToDs=0, FromDs=0
            sa = GetAddr3Ptr(pframe);
            break;
        case 0x01:  // ToDs=0, FromDs=1
            sa = GetAddr2Ptr(pframe);
            break;
        case 0x02:  // ToDs=1, FromDs=0
            sa = GetAddr1Ptr(pframe);
            break;
        default:    // ToDs=1, FromDs=1
            sa = GetAddr2Ptr(pframe);
            break;
    }

    return sa;
}


__inline static int32 IsFrameTypeCtrl(uint8 *pframe)
{
    if(WIFI_CTRL_TYPE == GetFrameType(pframe))
        return _TRUE;
    else
        return _FALSE;
}

#if 0
__inline int32 ieee80211_is_empty_essid(const uint8 *essid, int32 essid_len)
{
    /* Single white space is for Linksys APs */
    if (essid_len == 1 && essid[0] == ' ')
        return 1;

    /* Otherwise, if the entire essid is 0, we assume it is hidden */
    while (essid_len)
    {
        essid_len--;
        if (essid[essid_len] != '\0')
            return 0;
    }

    return 1;
}

__inline int32 ieee80211_get_hdrlen(uint16 fc)
{
    int32 hdrlen = 24;

    switch (WLAN_FC_GET_TYPE(fc))
    {
        case IEEE80211_FTYPE_DATA:
            if (fc & IEEE80211_QOS_DATAGRP)
                hdrlen += 2;
            if ((fc & IEEE80211_FCTL_FROMDS) && (fc & IEEE80211_FCTL_TODS))
                hdrlen += 6; /* Addr4 */
            break;
        case IEEE80211_FTYPE_CTL:
            switch (WLAN_FC_GET_STYPE(fc))
            {
                case IEEE80211_STYPE_CTS:
                case IEEE80211_STYPE_ACK:
                    hdrlen = 10;
                    break;
                default:
                    hdrlen = 16;
                    break;
            }
            break;
    }

    return hdrlen;
}
#endif
/*-----------------------------------------------------------------------------
            Below is for the security related definition
------------------------------------------------------------------------------*/
#define _RESERVED_FRAME_TYPE_   0
#define _SKB_FRAME_TYPE_        2
#define _PRE_ALLOCMEM_          1
#define _PRE_ALLOCHDR_          3
#define _PRE_ALLOCLLCHDR_       4
#define _PRE_ALLOCICVHDR_       5
#define _PRE_ALLOCMICHDR_       6

#define _SIFSTIME_              ((priv->pmib->dot11BssType.net_work_type&WIRELESS_11A)?16:10)
#define _ACKCTSLNG_             14  //14 bytes long, including crclng
#define _CRCLNG_                4

#define _ASOCREQ_IE_OFFSET_     4   // excluding wlan_hdr
#define _ASOCRSP_IE_OFFSET_     6
#define _REASOCREQ_IE_OFFSET_   10
#define _REASOCRSP_IE_OFFSET_   6
#define _PROBEREQ_IE_OFFSET_    0
#define _PROBERSP_IE_OFFSET_    12
#define _AUTH_IE_OFFSET_        6
#define _DEAUTH_IE_OFFSET_      0
#define _BEACON_IE_OFFSET_      12

#define _FIXED_IE_LENGTH_           _BEACON_IE_OFFSET_

#define _SSID_IE_               0
#define _SUPPORTEDRATES_IE_ 1
#define _DSSET_IE_              3
#define _TIM_IE_                    5
#define _IBSS_PARA_IE_          6
#define _CHLGETXT_IE_           16
#define _RSN_IE_2_              48
#define _SSN_IE_1_                  221
#define _ERPINFO_IE_            42
#define _EXT_SUPPORTEDRATES_IE_ 50

#define _HT_CAPABILITY_IE_          45
#define _HT_EXTRA_INFO_IE_          61
#define _HT_ADD_INFO_IE_            61 //_HT_EXTRA_INFO_IE_

#define EID_BSSCoexistence          72 // 20/40 BSS Coexistence
#define EID_BSSIntolerantChlReport  73

#define _VENDOR_SPECIFIC_IE_        221

#define _RESERVED47_                47


/* ---------------------------------------------------------------------------
                    Below is the fixed elements...
-----------------------------------------------------------------------------*/
#define _AUTH_ALGM_NUM_         2
#define _AUTH_SEQ_NUM_          2
#define _BEACON_ITERVAL_        2
#define _CAPABILITY_            2
#define _CURRENT_APADDR_        6
#define _LISTEN_INTERVAL_       2
#define _RSON_CODE_             2
#define _ASOC_ID_               2
#define _STATUS_CODE_           2
#define _TIMESTAMP_             8

#define AUTH_ODD_TO             0
#define AUTH_EVEN_TO            1

#define WLAN_ETHCONV_ENCAP      1
#define WLAN_ETHCONV_RFC1042    2
#define WLAN_ETHCONV_8021h      3

#define cap_ESS BIT(0)
#define cap_IBSS BIT(1)
#define cap_CFPollable BIT(2)
#define cap_CFRequest BIT(3)
#define cap_Privacy BIT(4)
#define cap_ShortPremble BIT(5)

/*-----------------------------------------------------------------------------
                Below is the definition for 802.11i / 802.1x
------------------------------------------------------------------------------*/
#define _IEEE8021X_MGT_         1       // WPA
#define _IEEE8021X_PSK_         2       // WPA with pre-shared key


/*-----------------------------------------------------------------------------
                Below is the definition for WMM
------------------------------------------------------------------------------*/
#define _WMM_IE_Length_             7  // for WMM STA
#define _WMM_Para_Element_Length_       24


/*-----------------------------------------------------------------------------
                Below is the definition for 802.11n
------------------------------------------------------------------------------*/

/* block-ack parameters */
#define IEEE80211_ADDBA_PARAM_POLICY_MASK 0x0002
#define IEEE80211_ADDBA_PARAM_TID_MASK 0x003C
#define IEEE80211_ADDBA_PARAM_BUF_SIZE_MASK 0xFFA0
#define IEEE80211_DELBA_PARAM_TID_MASK 0xF000
#define IEEE80211_DELBA_PARAM_INITIATOR_MASK 0x0800

#define SetOrderBit(pbuf)   \
    do  {   \
        *(uint16 *)(pbuf) |= cpu_to_le16(_ORDER_); \
    } while(0)

#define GetOrderBit(pbuf)   (((*(uint16 *)(pbuf)) & le16_to_cpu(_ORDER_)) != 0)


/* 802.11 BAR control masks */
#define IEEE80211_BAR_CTRL_ACK_POLICY_NORMAL     0x0000
#define IEEE80211_BAR_CTRL_CBMTID_COMPRESSED_BA  0x0004

__packed struct ieee80211_ht_cap
{
    uint16  cap_info;
    uint8   ampdu_params_info;
    uint8   supp_mcs_set[16];
    uint16  extended_ht_cap_info;
    uint32      tx_BF_cap_info;
    uint8          antenna_selection_info;
};


__packed struct ieee80211_ht_addt_info
{
    uint8   control_chan;
    uint8       ht_param;
    uint16  operation_mode;
    uint16  stbc_param;
    uint8       basic_set[16];
};

struct HT_caps_element
{
    union
    {
        __packed struct
        {
            uint16  HT_caps_info;
            uint8   AMPDU_para;
            uint8   MCS_rate[16];
            uint16  HT_ext_caps;
            uint32  Beamforming_caps;
            uint8   ASEL_caps;
        } HT_cap_element;
        uint8 HT_cap[26];
    } u;
};

__packed struct HT_info_element
{
    uint8   primary_channel;
    uint8   infos[5];
    uint8   MCS_rate[16];
};

__packed struct AC_param
{
    uint8       ACI_AIFSN;
    uint8       CW;
    uint16  TXOP_limit;
};

__packed struct WMM_para_element
{
    uint8       QoS_info;
    uint8       reserved;
    struct AC_param ac_param[4];
};

__packed struct ADDBA_request
{
    uint8       dialog_token;
    uint16  BA_para_set;
    uint16  BA_timeout_value;
    uint16  BA_starting_seqctrl;
};

/* 802.11n HT capabilities masks */
#define IEEE80211_HT_CAP_SUP_WIDTH      0x0002
#define IEEE80211_HT_CAP_SM_PS          0x000C
#define IEEE80211_HT_CAP_GRN_FLD        0x0010
#define IEEE80211_HT_CAP_SGI_20         0x0020
#define IEEE80211_HT_CAP_SGI_40         0x0040
#define IEEE80211_HT_CAP_TX_STBC            0x0080
#define IEEE80211_HT_CAP_RX_STBC        0x0300
#define IEEE80211_HT_CAP_DELAY_BA       0x0400
#define IEEE80211_HT_CAP_MAX_AMSDU      0x0800
#define IEEE80211_HT_CAP_DSSSCCK40      0x1000
/* 802.11n HT capability AMPDU settings */
#define IEEE80211_HT_CAP_AMPDU_FACTOR       0x03
#define IEEE80211_HT_CAP_AMPDU_DENSITY      0x1C
/* 802.11n HT capability MSC set */
#define IEEE80211_SUPP_MCS_SET_UEQM     4
#define IEEE80211_HT_CAP_MAX_STREAMS        4
#define IEEE80211_SUPP_MCS_SET_LEN      10
/* maximum streams the spec allows */
#define IEEE80211_HT_CAP_MCS_TX_DEFINED     0x01
#define IEEE80211_HT_CAP_MCS_TX_RX_DIFF     0x02
#define IEEE80211_HT_CAP_MCS_TX_STREAMS     0x0C
#define IEEE80211_HT_CAP_MCS_TX_UEQM        0x10
/* 802.11n HT IE masks */
#define IEEE80211_HT_IE_CHA_SEC_OFFSET      0x03
#define IEEE80211_HT_IE_CHA_SEC_NONE        0x00
#define IEEE80211_HT_IE_CHA_SEC_ABOVE       0x01
#define IEEE80211_HT_IE_CHA_SEC_BELOW       0x03
#define IEEE80211_HT_IE_CHA_WIDTH       0x04
#define IEEE80211_HT_IE_HT_PROTECTION       0x0003
#define IEEE80211_HT_IE_NON_GF_STA_PRSNT    0x0004
#define IEEE80211_HT_IE_NON_HT_STA_PRSNT    0x0010

/* block-ack parameters */
#define IEEE80211_ADDBA_PARAM_POLICY_MASK 0x0002
#define IEEE80211_ADDBA_PARAM_TID_MASK 0x003C
#define IEEE80211_ADDBA_PARAM_BUF_SIZE_MASK 0xFFA0
#define IEEE80211_DELBA_PARAM_TID_MASK 0xF000
#define IEEE80211_DELBA_PARAM_INITIATOR_MASK 0x0800

/*
 * A-PMDU buffer sizes
 * According to IEEE802.11n spec size varies from 8K to 64K (in powers of 2)
 */
#define IEEE80211_MIN_AMPDU_BUF 0x8
#define IEEE80211_MAX_AMPDU_BUF 0x40


/* Spatial Multiplexing Power Save Modes */
#define WLAN_HT_CAP_SM_PS_STATIC        0
#define WLAN_HT_CAP_SM_PS_DYNAMIC   1
#define WLAN_HT_CAP_SM_PS_INVALID   2
#define WLAN_HT_CAP_SM_PS_DISABLED  3

#define _HT_MIMO_PS_STATIC_     BIT(0)
#define _HT_MIMO_PS_DYNAMIC_        BIT(1)

//#endif

#define RND4(x) (((x >> 2) + (((x & 3) == 0) ?  0: 1)) << 2)

#if 1
#define MAX_IE_SZ                           256     //512   //768
#define NDIS_802_11_LENGTH_RATES                8
#define NDIS_802_11_LENGTH_RATES_EX         16

#define NUM_PRE_AUTH_KEY                    16
#define NUM_PMKID_CACHE                     NUM_PRE_AUTH_KEY

typedef uint64 NDIS_802_11_KEY_RSC;

typedef enum
{
    Ndis802_11FH,
    Ndis802_11DS,
    Ndis802_11OFDM5,
    Ndis802_11OFDM24,
    Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
} NDIS_802_11_NETWORK_TYPE;

typedef enum
{
    Ndis802_11IBSS,
    Ndis802_11Infrastructure,
    Ndis802_11AutoUnknown,
    Ndis802_11InfrastructureMax,     // Not a real value, defined as upper bound
    Ndis802_11APMode
} NDIS_802_11_NETWORK_INFRASTRUCTURE;

typedef enum
{
    Ndis802_11AuthModeOpen,
    Ndis802_11AuthModeShared,
    Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeWPA,
    Ndis802_11AuthModeWPAPSK,
    Ndis802_11AuthModeWPANone,
    Ndis802_11AuthModeMax               // Not a real mode, defined as upper bound
} NDIS_802_11_AUTHENTICATION_MODE;

typedef enum
{
    Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent
} NDIS_802_11_WEP_STATUS;

enum VWLAN_CARRIER_SENSE
{
    DISABLE_VCS,
    ENABLE_VCS,
    AUTO_VCS
};

enum VCS_TYPE
{
    NONE_VCS,
    RTS_CTS,
    CTS_TO_SELF
};

typedef struct
{
    uint32  SsidLength;
    uint8  Ssid[32];
} NDIS_802_11_SSID;

typedef struct
{
    uint32           Length;             // Length of structure
    uint32           HopPattern;         // As defined by 802.11, MSB set
    uint32           HopSet;             // to one if non-802.11
    uint32           DwellTime;          // units are Kusec
} NDIS_802_11_CONFIGURATION_FH;


/* FW will only save the channel number in DSConfig.
 * ODI Handler will convert the channel number to freq. number.
 */
typedef struct
{
    uint32           Length;             // Length of structure
    uint32           BeaconPeriod;       // units are Kusec
    uint32           ATIMWindow;         // units are Kusec
    uint32           DSConfig;           // Frequency, units are kHz
    NDIS_802_11_CONFIGURATION_FH    FHConfig;
} NDIS_802_11_CONFIGURATION;

typedef struct
{
    uint8  Timestamp[8];
    uint16  BeaconInterval;
    uint16  Capabilities;
} NDIS_802_11_FIXED_IEs;

typedef struct
{
    uint8  ElementID;
    uint8  Length;
    uint8  data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

// Key mapping keys require a BSSID
typedef struct
{
    uint32           Length;             // Length of this structure
    uint32           KeyIndex;
    uint32           KeyLength;          // length of key in bytes
    uint8 BSSID[6];
    NDIS_802_11_KEY_RSC KeyRSC;
    uint8           KeyMaterial[32];     // variable length depending on above field
} NDIS_802_11_KEY;

typedef struct
{
    uint32                   Length;        // Length of this structure
    uint32                   KeyIndex;
    uint8 BSSID[6];
} NDIS_802_11_REMOVE_KEY;

typedef struct
{
    //uint32     Length;        // Length of this structure
    uint32     KeyIndex;      // 0 is the per-client key, 1-N are the global keys
    uint32     KeyLength;     // length of key in bytes
    uint8     KeyMaterial[16];// variable length depending on above field
} NDIS_802_11_WEP;

typedef struct
{
    uint8   SignalStrength;//(in percentage)
    uint8   SignalQuality;//(in percentage)
    uint8   Optimum_antenna;  //for Antenna diversity
    uint8   Reserved_0;
} WLAN_PHY_INFO;

typedef struct
{
    uint32 Length;
    uint8  MacAddress[6];
    uint8 Reserved[2];//[0]: IS beacon frame
    NDIS_802_11_SSID Ssid;
    uint32  Privacy;
    long  Rssi;//(in dBM,raw data ,get from PHY)
    NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
    NDIS_802_11_CONFIGURATION  Configuration;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode;
    uint8 SupportedRates[NDIS_802_11_LENGTH_RATES_EX];  // Set of 16 data rates
    WLAN_PHY_INFO   PhyInfo;
    uint32 BcnLength;
    uint32  IELength;
    uint8  IEs[MAX_IE_SZ];  //(timestamp, beacon interval, and capability information)
} WLAN_BSSID_EX;
__inline  static uint32 get_WLAN_BSSID_EX_sz(WLAN_BSSID_EX *bss)
{
    return (sizeof(WLAN_BSSID_EX) -MAX_IE_SZ + bss->IELength);
}

struct  wlan_network
{
    _list   list;
    int network_type;   //refer to ieee80211.h for WIRELESS_11A/B/G
    int fixed;          // set to fixed when not to be removed as site-surveying
    uint32  last_scanned; //timestamp for the network
    int aid;            //will only be valid when a BSS is joinned.
    int join_res;
    WLAN_BSSID_EX   network; //must be the last item

    /* these infor get from wlan_get_encrypt_info when
     * translate scan to UI */
    uint8 encryp_protocol;//ENCRYP_PROTOCOL_E: OPEN/WEP/WPA/WPA2/WAPI
    int32 group_cipher; //WPA/WPA2 group cipher
    int32 pairwise_cipher;////WPA/WPA2 pairwise cipher
    int32 PrivacyAlgrthm;// encryption algorithm, could be none, wep40, TKIP, CCMP, wep104
    int32 is_8021x;
};
#endif

/****************************** eth ***********************************/
/* IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 * and FCS/CRC (frame check sequence).  */
#define ETH_HLEN            14      /* Total octets in header.   */
#define LLC_HEADER_SIZE     6       //!< LLC Header Length

/* These are the defined Ethernet Protocol ID's. */
#define ETH_P_IP                0x0800      /* Internet Protocol packet */
#define ETH_P_ARP           0x0806      /* Address Resolution packet    */
#define ETH_P_RARP          0x8035      /* Reverse Addr Res packet  */
#define ETH_P_AARP          0x80F3      /* Appletalk AARP       */
#define ETH_P_IPX           0x8137      /* IPX over DIX         */
#define ETH_P_PAE           0x888E      /* Port Access Entity (IEEE 802.1X) */


/* This is an Ethernet frame header.  */
struct ethhdr
{
    uint8   h_dest[ETH_ALEN];   /* destination eth addr */
    uint8   h_source[ETH_ALEN]; /* source ether addr    */
    uint16 h_proto;         /* packet type ID field */
};

struct _vlan
{
    uint16 h_vlan_TCI;                // Encapsulates priority and VLAN ID
    uint16 h_vlan_encapsulated_proto;
};

struct iphdr
{
#if defined(__LITTLE_ENDIAN_BITFIELD)
    uint32  ihl:4;
    uint32  version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
    uint32  version:4;
    uint32  ihl:4;
#else
#error  "Please define BIG or LITTLE endian"
#endif
    __uint8 tos;
    __u16   tot_len;
    __u16   id;
    __u16   frag_off;
    __uint8 ttl;
    __uint8 protocol;
    __u16   check;
    __u32   saddr;
    __u32   daddr;
    /*The options start here. */
};
/****************************** eth ***********************************/

__inline static uint32 _RND4(uint32 sz)
{

    uint32  val;

    val = ((sz >> 2) + ((sz & 3) ? 1: 0)) << 2;

    return val;

}

__inline static uint32 _RND8(uint32 sz)
{

    uint32  val;

    val = ((sz >> 3) + ((sz & 7) ? 1: 0)) << 3;

    return val;

}

__inline static uint32 _RND128(uint32 sz)
{

    uint32  val;

    val = ((sz >> 7) + ((sz & 127) ? 1: 0)) << 7;

    return val;

}

__inline static uint32 _RND256(uint32 sz)
{

    uint32  val;

    val = ((sz >> 8) + ((sz & 255) ? 1: 0)) << 8;

    return val;

}

__inline static uint32 _RND512(uint32 sz)
{

    uint32  val;

    val = ((sz >> 9) + ((sz & 511) ? 1: 0)) << 9;

    return val;

}

__inline static uint32 bitshift(uint32 bitmask)
{
    uint32 i;

    for (i = 0; i <= 31; i++)
        if (((bitmask>>i) &  0x1) == 1) break;

    return i;
}

__inline static uint8 key_char2num(uint8 ch)
{
    if((ch>='0')&&(ch<='9'))
        return ch - '0';
    else if ((ch>='a')&&(ch<='f'))
        return ch - 'a' + 10;
    else if ((ch>='A')&&(ch<='F'))
        return ch - 'A' + 10;
    else
        return 0xff;
}

__inline static uint8 key_2char2num(uint8 hch, uint8 lch)
{
    return ((key_char2num(hch) << 4) | key_char2num(lch));
}

#define MAC_ARG(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

__inline static uint32 buf_endian_reverse(uint32 src)
{

    return (    ((((src))&0x000000ff)<<24)  |       \
                ((((src))&0x0000ff00)<<8)   |       \
                ((((src))&0x00ff0000)>>8)   |       \
                ((((src))&0xff000000)>>24)  );

}

__inline static void wlan_enqueue_pkt(_queue *pskb_queue, _pkt* skb)
{
    _irqL irqL;

    os_spin_lock_irqsave(&pskb_queue->lock, &irqL);

    os_list_insert_tail(&skb->list, os_get_queue_head(pskb_queue));

    os_spin_unlock_irqsave(&pskb_queue->lock, &irqL);

}

__inline static _pkt *wlan_dequeue_pkt(_queue *pskb_queue)
{
    _irqL irqL;
    _list   *plist, *phead;
    _pkt *rx_skb = NULL;

    os_spin_lock_irqsave(&pskb_queue->lock, &irqL);

    if(os_queue_empty(pskb_queue) == _TRUE)
    {
        rx_skb = NULL;
    }
    else
    {
        phead = os_get_queue_head(pskb_queue);

        plist = os_list_get_next(phead);

        rx_skb = LIST_CONTAINOR(plist, _pkt, list);

        os_list_delete(&rx_skb->list);
    }

    os_spin_unlock_irqsave(&pskb_queue->lock, &irqL);

    return rx_skb;
}
#endif // _WIFI_H_

