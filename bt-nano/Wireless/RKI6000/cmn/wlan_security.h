#ifndef __WLAN_SECURITY_H_
#define __WLAN_SECURITY_H_


#define _WPA_IE_ID_ 0xdd
#define _WPA2_IE_ID_    0x30

#ifndef Ndis802_11AuthModeWPA2
#define Ndis802_11AuthModeWPA2 (Ndis802_11AuthModeWPANone + 1)
#endif

#ifndef Ndis802_11AuthModeWPA2PSK
#define Ndis802_11AuthModeWPA2PSK (Ndis802_11AuthModeWPANone + 2)
#endif

typedef enum
{
    ENCRYP_PROTOCOL_OPENSYS,   //open system
    ENCRYP_PROTOCOL_WEP,       //WEP
    ENCRYP_PROTOCOL_WPA,       //WPA
    ENCRYP_PROTOCOL_WPA2,      //WPA2
    ENCRYP_PROTOCOL_WAPI,      //WAPI: Not support in this version
    ENCRYP_PROTOCOL_MAX
} ENCRYP_PROTOCOL_E;

union pn48
{

    uint64  val;

#ifdef CONFIG_LITTLE_ENDIAN

    struct
    {
        uint8 TSC0;
        uint8 TSC1;
        uint8 TSC2;
        uint8 TSC3;
        uint8 TSC4;
        uint8 TSC5;
        uint8 TSC6;
        uint8 TSC7;
    } _byte_;

#elif defined(CONFIG_BIG_ENDIAN)

    struct
    {
        uint8 TSC7;
        uint8 TSC6;
        uint8 TSC5;
        uint8 TSC4;
        uint8 TSC3;
        uint8 TSC2;
        uint8 TSC1;
        uint8 TSC0;
    } _byte_;

#endif

};

struct Keytype
{
    uint8   skey[16];
};

struct Dot11EncryptKey
{
    uint32  dot11TTKeyLen;
    uint32  dot11TMicKeyLen;
    struct Keytype  dot11TTKey;
    struct Keytype  dot11TMicKey1;
    struct Keytype  dot11TMicKey2;
    union pn48      dot11TXPN48;
    union pn48      dot11RXPN48;
};

struct Dot11KeyMappingsEntry
{
    uint32  dot11Privacy;
    uint32  keyInCam;   // Is my key in CAM?
    uint32  keyid;
    struct Dot11EncryptKey  dot11EncryptKey;
};

typedef struct _WLAN_PMKID_LIST
{
    uint8                       bUsed;
    uint8                       Bssid[6];
    uint8                       PMKID[16];
    uint8                       SsidBuf[33];
    uint8*                      ssid_octet;
    uint16                      ssid_length;
} WLAN_PMKID_LIST, *PWLAN_PMKID_LIST;

typedef struct _SW_CAM_INFO
{
    uint8   mac_addr[ETH_ALEN];
    uint8   used;
} SW_CAM_INFO;
struct security_struct
{
    /* this var just used as mem for some parameter transfer use */
    NDIS_802_11_WEP ndiswep;

    uint32 dot11AuthAlgrthm;        // 802.11 auth, could be open, shared, 8021x and authswitch
    uint32  dot11PrivacyAlgrthm;    // This specify the privacy for shared auth. algorithm.
    //keeps the auth_type & enc_status from upper layer ioctl(wpa_supplicant or wzc)
    uint32 ndisauthtype;    // NDIS_802_11_AUTHENTICATION_MODE
    uint32 ndisencryptstatus;   // NDIS_802_11_ENCRYPTION_STATUS

    /* WEP */
    uint32  dot11PrivacyKeyIndex;   // this is only valid for legendary wep, 0~3 for key id.
    struct Keytype dot11DefKey[4];          // this is only valid for def. key
    uint32  dot11DefKeylen[4];

    uint32 dot118021XGrpPrivacy;    // This specify the privacy algthm. used for Grp key

    /* used for wpa set key */
    uint32  dot118021XGrpKeyid;     // key id used for Grp Key //from 1-4, we should -1 when use it
    struct Keytype dot118021XGrpKey[4]; // 802.1x Group Key, for inx0 and inx1
    struct Keytype dot118021XGrptxmickey;
    struct Keytype dot118021XGrprxmickey;
    uint8   binstallGrpkey; /* should set to true after GrpKey installed */

#ifdef TODO
#else
    /* not open WPS now */
    uint8 wps_phase;//for wps
    uint8 wps_ie[MAX_WPA_IE_LEN<<2];
    int32 wps_ie_len;

    //uint8 bcheck_grpkey;
    //uint8 bgrpkey_handshake;
#endif
    /* follwing 2 vars for SetRSC but not used now */
    union pn48      dot11Grptxpn;           // PN48 used for Grp Key xmit.
    union pn48      dot11Grprxpn;           // PN48 used for Grp Key recv.

    int32   sw_encrypt;//from config_struct
    int32   sw_decrypt;//from config_struct
    int32 hw_decrypted;//if the rx packets is hw_decrypted==_FALSE, it means the hw has not been ready.

    //for tkip countermeasure, because it needs app support, so we close it temp.
    //uint32 last_mic_err_time;
    //uint8  btkip_countermeasure;
    //uint32 btkip_countermeasure_time;

    //---------------------------------------------------------------------------
    // For WPA2 Pre-Authentication.
    //---------------------------------------------------------------------------
    //uint8 RegEnablePreAuth;               // Default value: Pre-Authentication enabled or not, from registry "EnablePreAuth". Added by Annie, 2005-11-01.
    //uint8 EnablePreAuthentication;            // Current Value: Pre-Authentication enabled or not.
    //uint8 PMKIDIndex;
    //uint32 PMKIDCount;                        // Added by Annie, 2006-10-13.
    //uint8 szCapability[256];              // For WPA2-PSK using zero-config, by Annie, 2005-09-20.
    SW_CAM_INFO swsec_cam_info[32];
};

#define GET_ENCRY_ALGO(psecurity, psta, encry_algo, bmcst)\
do{\
    switch(psecurity->dot11AuthAlgrthm)\
    {\
        case dot11AuthAlgrthm_Open:\
        case dot11AuthAlgrthm_Shared:\
        case dot11AuthAlgrthm_Auto:\
            encry_algo = (uint8)psecurity->dot11PrivacyAlgrthm;\
            break;\
        case dot11AuthAlgrthm_8021X:\
            if(bmcst)\
                encry_algo = (uint8)psecurity->dot118021XGrpPrivacy;\
            else\
                encry_algo =(uint8) psta->dot118021XPrivacy;\
            break;\
        default:\
            break;\
    }\
}while(0)


#define SET_ICE_IV_LEN( iv_len, icv_len, encrypt)\
do{\
    switch(encrypt)\
    {\
        case DOT11_ENC_WEP40:\
        case DOT11_ENC_WEP104:\
            iv_len = 4;\
            icv_len = 4;\
            break;\
        case DOT11_ENC_TKIP:\
            iv_len = 8;\
            icv_len = 4;\
            break;\
        case DOT11_ENC_CCMP:\
            iv_len = 8;\
            icv_len = 8;\
            break;\
        default:\
            iv_len = 0;\
            icv_len = 0;\
            break;\
    }\
}while(0)


#define GET_TKIP_PN(iv,dot11txpn)\
do{\
    dot11txpn._byte_.TSC0=iv[2];\
    dot11txpn._byte_.TSC1=iv[0];\
    dot11txpn._byte_.TSC2=iv[4];\
    dot11txpn._byte_.TSC3=iv[5];\
    dot11txpn._byte_.TSC4=iv[6];\
    dot11txpn._byte_.TSC5=iv[7];\
}while(0)


#define ROL32( A, n )   ( ((A) << (n)) | ( ((A)>>(32-(n)))  & ( (1UL << (n)) - 1 ) ) )
#define ROR32( A, n )   ROL32( (A), 32-(n) )

struct mic_data
{
    uint32  K0, K1;         // Key
    uint32  L, R;           // Current state
    uint32  M;              // Message accumulator (single word)
    uint32  nBytesInM;      // # bytes in M
};

void wlan_secmicsetkey(struct mic_data *pmicdata, uint8 * key );
void wlan_secmicappendbyte(struct mic_data *pmicdata, uint8 b );
void wlan_secmicappend(struct mic_data *pmicdata, uint8 * src, uint32 nBytes );
void wlan_secgetmic(struct mic_data *pmicdata, uint8 * dst );

void wlan_seccalctkipmic(
    uint8 * key,
    uint8 *header,
    uint8 *data,
    uint32 data_len,
    uint8 *Miccode,
    uint8   priority);

void wlan_wep_encrypt(ADAPTER *Adapter, uint8 *pxmitframe);
uint32 wlan_wep_decrypt(ADAPTER *Adapter, uint8  *precvframe);

#ifdef SW_ENCRYPT_SUPPORT
uint32 wlan_aes_encrypt(ADAPTER *Adapter, uint8 *pxmitframe);
uint32 wlan_tkip_encrypt(ADAPTER *Adapter, uint8 *pxmitframe);
#endif

#ifdef SW_DECRYPT_SUPPORT
uint32 wlan_aes_decrypt(ADAPTER *Adapter, uint8  *precvframe);
uint32 wlan_tkip_decrypt(ADAPTER *Adapter, uint8  *precvframe);
#endif

#endif

