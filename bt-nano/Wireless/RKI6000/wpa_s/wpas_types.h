#ifndef __WPAS_TYPES_H__
#define __WPAS_TYPES_H__

#define MAXRSNIELEN     128
#define MACADDRLEN      6

typedef void*           caddr_t;

#define OPMODE          (priv->padapter->mlme.op_mode)

#ifdef CONFIG_SOFTAP_SUPPORT
#define MAX_ASSOC_NUM       4   //8
#else
#define MAX_ASSOC_NUM       2
#endif

#define NUM_WDS                 4

typedef enum
{
    DOT11_PortStatus_Unauthorized,
    DOT11_PortStatus_Authorized,
    DOT11_PortStatus_Guest
} DOT11_POWLAN_STATUS;

typedef enum
{
    DOT11_AuthKeyType_RSN = 1,
    DOT11_AuthKeyType_PSK = 2,
    DOT11_AuthKeyType_NonRSN802dot1x = 3
} DOT11_AUTHKEY_TYPE;

typedef enum
{
    DOT11_Ioctl_Query = 0,
    DOT11_Ioctl_Set = 1
} DOT11_Ioctl_Flag;

typedef struct
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint8            MACAddr[MACADDRLEN];
    uint16   Status;
} DOT11_ASSOCIATIIN_RSP;

typedef struct _DOT11_DISCONNECT_REQ
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint16  Reason;
    uint8            MACAddr[MACADDRLEN];
} DOT11_DISCONNECT_REQ;

typedef struct _DOT11_SET_RSNIE
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint16  Flag;
    uint16  RSNIELen;
    uint8            RSNIE[MAXRSNIELEN];
    uint8            MACAddr[MACADDRLEN];
} DOT11_SET_RSNIE;

typedef struct _DOT11_SET_PORT
{
    uint8 EventId;
    uint8 PortStatus;
    uint8 PortType;
    uint8 MACAddr[MACADDRLEN];
} DOT11_SET_PORT;

typedef struct _DOT11_MIC_FAILURE
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint8            MACAddr[MACADDRLEN];
} DOT11_MIC_FAILURE;

typedef struct _DOT11_INIT_QUEUE
{
    uint8 EventId;
    uint8 IsMoreEvent;
} DOT11_INIT_QUEUE, *PDOT11_INIT_QUEUE;

typedef enum
{
    ERROR_BUFFER_TOO_SMALL = -1,
    ERROR_INVALID_PARA = -2,
    ERROR_INVALID_RSNIE = -13,
    ERROR_INVALID_MULTICASTCIPHER = -18,
    ERROR_INVALID_UNICASTCIPHER = -19,
    ERROR_INVALID_AUTHKEYMANAGE = -20,
    ERROR_UNSUPPORTED_RSNEVERSION = -21,
    ERROR_INVALID_CAPABILITIES = -22
} INFO_ERROR;

//*---------- The followings are for processing of RSN Information Element------------*/
#define RSN_ELEMENT_ID                      0xDD
#define RSN_VER1                            0x01
#define DOT11_MAX_CIPHER_ALGORITHMS     0x0a

typedef struct _DOT11_RSN_IE_HEADER
{
    uint8   ElementID;
    uint8   Length;
    uint8   OUI[4];
    uint16  Version;
} __attribute__((packed)) DOT11_RSN_IE_HEADER;


typedef struct _DOT11_RSN_IE_SUITE
{
    uint8   OUI[3];
    uint8   Type;
} __attribute__((packed)) DOT11_RSN_IE_SUITE;


typedef struct _DOT11_RSN_IE_COUNT_SUITE
{

    uint16  SuiteCount;
    DOT11_RSN_IE_SUITE  dot11RSNIESuite[DOT11_MAX_CIPHER_ALGORITHMS];
} __attribute__((packed)) DOT11_RSN_IE_COUNT_SUITE, *PDOT11_RSN_IE_COUNT_SUITE;

typedef union _DOT11_RSN_CAPABILITY
{

    uint16  shortData;
    uint8   charData[2];

    struct
    {
        volatile uint32 Reserved1:2; // B7 B6
        volatile uint32 GtksaReplayCounter:2; // B5 B4
        volatile uint32 PtksaReplayCounter:2; // B3 B2
        volatile uint32 NoPairwise:1; // B1
        volatile uint32 PreAuthentication:1; // B0
        volatile uint32 Reserved2:8;
    } __attribute__((packed)) field;

} DOT11_RSN_CAPABILITY;

struct Dot1180211AuthEntry
{
    uint32  dot11AuthAlgrthm;       // 802.11 auth, could be open, shared, auto
    uint32  dot11PrivacyAlgrthm;    // encryption algorithm, could be none, wep40, TKIP, CCMP, wep104
    uint32  dot11PrivacyKeyIndex;   // this is only valid for legendary wep, 0~3 for key id.
    uint32  dot11PrivacyKeyLen;     // this could be 40 or 104

    uint32  dot11EnablePSK;         // 0: disable, bit0: WPA, bit1: WPA2
    uint32  dot11WPACipher;         // bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
    uint32  dot11WPA2Cipher;            // bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
    uint32  dot11WPAGrpCipher;          // bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128
    uint32  dot11WPA2GrpCipher;         // bit0-wep64, bit1-tkip, bit2-wrap,bit3-ccmp, bit4-wep128

    uint8   dot11PassPhrase[65];    // passphrase
    uint8   dot11PassPhraseGuest[65];   // passphrase of guest

    uint32  dot11GKRekeyTime;       // group key rekey time, 0 - disable

    /* just wep here */
    uint8   key_64bits_arr[4][5];
    uint8   key_128bits_arr[4][13];

    uint8   is_eap_sim;
#ifdef CONFIG_OUTER_WPASUPP
    WIFISUPP_WPA_EAP_TYPE_E eap_type;
    uint32  simcard_num;
    uint8 peap_identity[32+1];
    uint32 peap_identity_len;
    uint8 peap_password[32+1];
    uint32 peap_password_len;
#endif
};

#ifndef MACADDRLEN
#define MACADDRLEN      6
#endif

struct wdsEntry
{
    uint8 macAddr [MACADDRLEN];
    uint32 txRate;
} __attribute__((packed));

struct wds_info
{
    int             wdsEnabled;
    int             wdsPure;        // act as WDS bridge only, no AP function
    int             wdsPriority;    // WDS packets have higer priority
    struct wdsEntry entry[NUM_WDS];
    int             wdsNum;         // number of WDS used
    int             wdsPrivacy;
    uint8   wdsWepKey[32];
    uint8   wdsMapingKey[NUM_WDS][32];
    int             wdsMappingKeyLen[NUM_WDS];
    int             wdsKeyId;
    uint8   wdsPskPassPhrase[65];
};

struct Dot11RsnIE
{
    uint8   rsnie[128];
    uint8   rsnielen;
};

struct wlan_ethhdr_t
{
    uint8 daddr[6];
    uint8 saddr[6];
    uint16 type;
} __attribute__((packed));
struct aid_obj
{
    void*   station;
    uint32      used;   // used == TRUE => has been allocated, used == FALSE => can be allocated
};
struct wpa_priv
{
    WPA_GLOBAL_INFO wpa_global_info;

    /* these were moved here from mib */
    struct Dot1180211AuthEntry  dot1180211AuthEntry;
    /* just used in AP mode, left here just
     * for build use now, we will delete it
     * later */
    struct Dot11KeyMappingsEntry dot11GroupKeysTable;
    //struct wds_info   dot11WdsInfo;
    //struct Dot11RsnIE dot11RsnIE;
    struct aid_obj   aidarray[MAX_ASSOC_NUM];
    ADAPTER *padapter;
} ;

enum
{
    DRV_STATE_INIT   = 1,   /* driver has been init */
    DRV_STATE_OPEN  = 2,    /* driver is opened */
#ifdef UNIVERSAL_REPEATER
    DRV_STATE_VXD_INIT = 4, /* vxd driver has been opened */
    DRV_STATE_VXD_AP_STARTED    = 8, /* vxd ap has been started */
#endif
    DRV_STATE_MAX
};

__inline static int32 _atoi(uint8 *s, int32 base)
{
    int32 k = 0;

    k = 0;
    if (base == 10)
    {
        while (*s != '\0' && *s >= '0' && *s <= '9')
        {
            k = 10 * k + (*s - '0');
            s++;
        }
    }
    else
    {
        while (*s != '\0')
        {
            int32 v;
            if ( *s >= '0' && *s <= '9')
                v = *s - '0';
            else if ( *s >= 'a' && *s <= 'f')
                v = *s - 'a' + 10;
            else if ( *s >= 'A' && *s <= 'F')
                v = *s - 'A' + 10;
            else
            {
                DBG_PRINT(WLAN_DBG_WPAS, _drv_err_, ("error hex format!\n"));
                return 0;
            }
            k = 16 * k + v;
            s++;
        }
    }
    return k;
}

__inline static int32 get_array_val(uint8 *dst, uint8 *src, int32 len)
{
    uint8 tmpbuf[4];
    int32 num=0;

    while (len > 0)
    {
        os_memcpy(tmpbuf, src, 2);
        tmpbuf[2]='\0';
        *dst++ = (uint8)_atoi(tmpbuf, 16);
        len-=2;
        src+=2;
        num++;
    }
    return num;
}

#define SSID(priv)              (priv->padapter->mlme.assoc_ssid.Ssid)
#define SSID_LEN(priv)          (priv->padapter->mlme.assoc_ssid.SsidLength)

#define GET_MY_HWADDR(priv)     (priv->padapter->dev_addr)

#endif
