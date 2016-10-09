#ifndef __WPAS_PSK_H_
#define __WPAS_PSK_H_

#define GMK_LEN                     32
#define GTK_LEN                     32
#define PMK_LEN                     32
#define PTK_LEN                     32
#define KEY_NONCE_LEN               32
#define NumGroupKey                 4
#define KEY_RC_LEN                  8
#define KEY_NONCE_LEN               32
#define KEY_IV_LEN                  16
#define KEY_RSC_LEN                 8
#define KEY_ID_LEN                  8
#define KEY_MIC_LEN                 16
#define KEY_MATERIAL_LEN            2
#define PTK_LEN_EAPOLMIC            16
#define PTK_LEN_EAPOLENC            16
#define PTK_LEN_TKIP                64
#define PMKID_LEN                   16

#define DescTypePos                 0
#define KeyInfoPos                  1
#define KeyLenPos                   3
#define ReplayCounterPos            5
#define KeyNoncePos                 13
#define KeyIVPos                    45
#define KeyRSCPos                   61
#define KeyIDPos                    69
#define KeyMICPos                   77
#define KeyDataLenPos               93
#define KeyDataPos                  95
#define LIB1X_EAPOL_VER             1       //00000001B
#define LIB1X_EAPOL_LOGOFF          2       //0000 0010B
#define LIB1X_EAPOL_EAPPKT          0       //0000 0000B
#define LIB1X_EAPOL_START           1       //0000 0001B
#define LIB1X_EAPOL_KEY             3       //0000 0011B
#define LIB1X_EAPOL_ENCASFALERT     4       //0000 0100B


#define PTK_LEN_CCMP                        48

#define MAX_RESEND_NUM          15

#define RESEND_TIME                 500 // in 10ms

#define A_SHA_DIGEST_LEN            20
#define ETHER_HDRLEN                14
#define LIB1X_EAPOL_HDRLEN          4
#define INFO_ELEMENT_SIZE           128
#define MAX_EAPOLMSG_LEN            512
#define MAX_EAPOLKEYMSG_LEN (MAX_EAPOLMSG_LEN-(ETHER_HDRLEN+LIB1X_EAPOL_HDRLEN))
#define EAPOLMSG_HDRLEN             95      //EAPOL-key payload length without KeyData
#define MAX_UNICAST_CIPHER          2
#define WPA_ELEMENT_ID              0xDD
#define WPA2_ELEMENT_ID             0x30

typedef enum    { desc_type_WPA2 = 2, desc_type_RSN = 254 } DescTypeRSN;
typedef enum    { type_Group = 0, type_Pairwise = 1 } KeyType;
typedef enum    { key_desc_ver1 = 1, key_desc_ver2 = 2 } KeyDescVer;
enum { PSK_WPA=1, PSK_WPA2=2};

enum
{
    PSK_STATE_IDLE,
    PSK_STATE_PTKSTART,
    PSK_STATE_PTKINITNEGOTIATING,
    PSK_STATE_PTKINITDONE
};

enum
{
    PSK_GSTATE_REKEYNEGOTIATING,
    PSK_GSTATE_REKEYESTABLISHED,
    PSK_GSTATE_KEYERROR
};

/*
 * Reason code for Disconnect
 */
typedef enum _ReasonCode
{
    unspec_reason                   = 0x01,
    auth_not_valid                  = 0x02,
    deauth_lv_ss                    = 0x03,
    inactivity                      = 0x04,
    ap_overload                     = 0x05,
    class2_err                      = 0x06,
    class3_err                      = 0x07,
    disas_lv_ss                     = 0x08,
    asoc_not_auth                   = 0x09,
    RSN_invalid_info_element        = 13,
    RSN_MIC_failure                 = 14,
    RSN_4_way_handshake_timeout = 15,
    RSN_diff_info_element           = 17,
    RSN_multicast_cipher_not_valid  = 18,
    RSN_unicast_cipher_not_valid    = 19,
    RSN_AKMP_not_valid              = 20,
    RSN_unsupported_RSNE_version    = 21,
    RSN_invalid_RSNE_capabilities   = 22,
    RSN_ieee_802dot1x_failed        = 23,

    RSN_PMK_not_avaliable           = 24,
    expire                          = 30,
    session_timeout                 = 31,
    acct_idle_timeout               = 32,
    acct_user_request               = 33
} ReasonCode;


typedef struct _OCTET_STRING
{
    uint8   *Octet;
    int32   Length;
} OCTET_STRING;

typedef union _LARGE_INTEGER
{
    uint8   charData[8];
    struct
    {
        uint32  HighPart;
        uint32  LowPart;
    } field;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef union _OCTET16_INTEGER
{
    uint8   charData[16];
    struct
    {
        LARGE_INTEGER   HighPart;
        LARGE_INTEGER   LowPart;
    } field;
} OCTET16_INTEGER;

typedef union  _OCTET32_INTEGER
{
    uint8   charData[32];
    struct
    {
        OCTET16_INTEGER HighPart;
        OCTET16_INTEGER LowPart;
    } field;
} OCTET32_INTEGER;

typedef struct _DOT11_WPA2_IE_HEADER
{
    uint8 ElementID;
    uint8 Length;
    uint16 Version;
} DOT11_WPA2_IE_HEADER;


// group key info
typedef struct _wpa_global_info
{
    OCTET32_INTEGER     Counter;
    uint8       PSK[A_SHA_DIGEST_LEN*2];
    uint8       PSKGuest[A_SHA_DIGEST_LEN*2];
    int         GTKAuthenticator;
    int         GKeyDoneStations;
    int         GkeyReady;
    OCTET_STRING        AuthInfoElement;
    uint8       AuthInfoBuf[INFO_ELEMENT_SIZE];
    uint8       MulticastCipher;
    int         NumOfUnicastCipher;
    uint8       UnicastCipher[MAX_UNICAST_CIPHER];
    int         NumOfUnicastCipherWPA2;
    uint8       UnicastCipherWPA2[MAX_UNICAST_CIPHER];
    OCTET_STRING        GNonce;
    uint8       GNonceBuf[KEY_NONCE_LEN];
    uint8       GTK[NumGroupKey][GTK_LEN];
    uint8       GMK[GMK_LEN];
    int         GN;
    int         GM;
    int         GRekeyCounts;
    int         GResetCounter;

    int         IntegrityFailed;
    int         GTKRekey;
    int         GKeyFailure;
    struct timer_list   GKRekeyTimer;
} WPA_GLOBAL_INFO;

// wpa sta info
typedef struct _wpa_sta_info
{
    int         state;
    int         gstate;
    int         RSNEnabled;     // bit0-WPA, bit1-WPA2
    int         PMKCached;
    int         PInitAKeys;
    uint8       UnicastCipher;
    uint8       NumOfRxTSC;
    uint8       AuthKeyMethod;
    int         isSuppSupportPreAuthentication;
    int         isSuppSupportPairwiseAsDefaultKey;
    LARGE_INTEGER       CurrentReplayCounter;
    LARGE_INTEGER       ReplayCounterStarted;
    OCTET_STRING        ANonce;
    OCTET_STRING        SNonce;
    uint8       AnonceBuf[KEY_NONCE_LEN];
    uint8       SnonceBuf[KEY_NONCE_LEN];
    uint8       PMK[PMK_LEN];
    uint8       PTK[PTK_LEN_TKIP];
    OCTET_STRING        EAPOLMsgRecvd;
    OCTET_STRING        EAPOLMsgSend;
    OCTET_STRING        EapolKeyMsgRecvd;
    OCTET_STRING        EapolKeyMsgSend;
    //uint8     eapSendBuf[MAX_EAPOLMSG_LEN];
    //uint8     eapRecvdBuf[MAX_EAPOLMSG_LEN];
    int         resendCnt;
    int         isGuest;
    int         clientHndshkProcessing;
    int         clientHndshkDone;
    struct timer_list   resendTimer;
    struct wpa_priv     *priv;
    uint8       wpa_ie[INFO_ELEMENT_SIZE];
    uint32      wpa_ie_len;
    int         GUpdateStationKeys;
} WPA_STA_INFO;

typedef struct _LIB1X_EAPOL_KEY
{
    uint8       key_desc_ver;
    uint8       key_info[2];
    uint8       key_len[2];
    uint8       key_replay_counter[KEY_RC_LEN];
    uint8       key_nounce[KEY_NONCE_LEN];
    uint8       key_iv[KEY_IV_LEN];
    uint8       key_rsc[KEY_RSC_LEN];
    uint8       key_id[KEY_ID_LEN];
    uint8       key_mic[KEY_MIC_LEN];
    uint8       key_data_len[KEY_MATERIAL_LEN];
    uint8       *key_data;
} __attribute__((packed)) lib1x_eapol_key;


struct lib1x_eapol
{
    uint8       protocol_version;
    uint8       packet_type;            // This makes it odd in number !
    uint16      packet_body_length;
} __attribute__((packed)) ;

#define SetSubStr(f,a,l)                os_memcpy(f->Octet+l,a.Octet,a.Length)
#define GetKeyInfo0(f, mask)            ((f->Octet[KeyInfoPos + 1] & mask) ? 1 :0)
#define SetKeyInfo0(f,mask,b)           (f->Octet[KeyInfoPos + 1] = (f->Octet[KeyInfoPos + 1] & ~mask) | ( b?mask:0x0) )
#define GetKeyInfo1(f, mask)            ((f->Octet[KeyInfoPos] & mask) ? 1 :0)
#define SetKeyInfo1(f,mask,b)           (f->Octet[KeyInfoPos] = (f->Octet[KeyInfoPos] & ~mask) | ( b?mask:0x0) )

// EAPOLKey
#define Message_DescType(f)             (f->Octet[DescTypePos])
#define Message_setDescType(f, type)    (f->Octet[DescTypePos] = type)
// Key Information Filed
#define Message_KeyDescVer(f)           (f->Octet[KeyInfoPos+1] & 0x07)//(f->Octet[KeyInfoPos+1] & 0x01) | (f->Octet[KeyInfoPos+1] & 0x02) <<1 | (f->Octet[KeyInfoPos+1] & 0x04) <<2
#define Message_setKeyDescVer(f, v)     (f->Octet[KeyInfoPos+1] &= 0xf8) , f->Octet[KeyInfoPos+1] |= (v & 0x07)//(f->Octet[KeyInfoPos+1] |= ((v&0x01)<<7 | (v&0x02)<<6 | (v&0x04)<<5) )
#define Message_KeyType(f)              GetKeyInfo0(f,0x08)
#define Message_setKeyType(f, b)        SetKeyInfo0(f,0x08,b)
#define Message_KeyIndex(f)             (((f->Octet[KeyInfoPos+1] & 0x30) >> 4) & 0x03) //(f->Octet[KeyInfoPos+1] & 0x20) | (f->Octet[KeyInfoPos+1] & 0x10) <<1
#define Message_setKeyIndex(f, v)       (f->Octet[KeyInfoPos+1] &= 0xcf), f->Octet[KeyInfoPos+1] |= ((v<<4) & 0x07)//(f->Octet[KeyInfoPos+1] |= ( (v&0x01)<<5 | (v&0x02)<<4)  )
#define Message_Install(f)              GetKeyInfo0(f,0x40)
#define Message_setInstall(f, b)        SetKeyInfo0(f,0x40,b)
#define Message_KeyAck(f)               GetKeyInfo0(f,0x80)
#define Message_setKeyAck(f, b)         SetKeyInfo0(f,0x80,b)

#define Message_KeyMIC(f)               GetKeyInfo1(f,0x01)
#define Message_setKeyMIC(f, b)         SetKeyInfo1(f,0x01,b)
#define Message_Secure(f)               GetKeyInfo1(f,0x02)
#define Message_setSecure(f, b)         SetKeyInfo1(f,0x02,b)
#define Message_Error(f)                GetKeyInfo1(f,0x04)
#define Message_setError(f, b)          SetKeyInfo1(f,0x04,b)
#define Message_Request(f)              GetKeyInfo1(f,0x08)
#define Message_setRequest(f, b)        SetKeyInfo1(f,0x08,b)
#define Message_Reserved(f)             (f->Octet[KeyInfoPos] & 0xf0)
#define Message_setReserved(f, v)       (f->Octet[KeyInfoPos] |= (v<<4&0xff))
#define Message_KeyLength(f)            ((uint16)(f->Octet[KeyLenPos] <<8) + (uint16)(f->Octet[KeyLenPos+1]))
#define Message_setKeyLength(f, v)      (f->Octet[KeyLenPos] = (v&0xff00) >>8 ,  f->Octet[KeyLenPos+1] = (v&0x00ff))

#define Message_KeyNonce(f)                 sub_str(f,KeyNoncePos,KEY_NONCE_LEN)
#define Message_setKeyNonce(f, v)           SetSubStr(f, v, KeyNoncePos)
#define Message_EqualKeyNonce(f1, f2)       !os_memcmp(f1->Octet + KeyNoncePos, f2->Octet, KEY_NONCE_LEN)? 0:1
#define Message_KeyIV(f)                    sub_str(f, KeyIVPos, KEY_IV_LEN)
#define Message_setKeyIV(f, v)              SetSubStr(f, v, KeyIVPos)
#define Message_KeyRSC(f)                   sub_str(f, KeyRSCPos, KEY_RSC_LEN)
#define Message_setKeyRSC(f, v)             SetSubStr(f, v, KeyRSCPos)
#define Message_KeyID(f)                    sub_str(f, KeyIDPos, KEY_ID_LEN)
#define Message_setKeyID(f, v)              SetSubStr(f, v, KeyIDPos)
#define Message_MIC(f)                      sub_str(f, KeyMICPos, KEY_MIC_LEN)
#define Message_setMIC(f, v)                SetSubStr(f, v, KeyMICPos)
#define Message_clearMIC(f)                 os_memset(f->Octet+KeyMICPos, 0, KEY_MIC_LEN)
#define Message_KeyDataLength(f)            ((uint16)(f->Octet[KeyDataLenPos] <<8) + (uint16)(f->Octet[KeyDataLenPos+1]))
#define Message_setKeyDataLength(f, v)      (f->Octet[KeyDataLenPos] = (v&0xff00) >>8 ,  f->Octet[KeyDataLenPos+1] = (v&0x00ff))
#define Message_KeyData(f, l)               sub_str(f, KeyDataPos, l)
#define Message_setKeyData(f, v)            SetSubStr(f, v, KeyDataPos);
#define Message_EqualRSNIE(f1 , f2, l)      !os_memcmp(f1->Octet, f2->Octet, l) ? 0:1
#define Message_ReturnKeyDataLength(f)      f->Length - (ETHER_HDRLEN + LIB1X_EAPOL_HDRLEN + EAPOLMSG_HDRLEN)

#define Message_CopyReplayCounter(f1, f2)   os_memcpy(f1->Octet + ReplayCounterPos, f2->Octet + ReplayCounterPos, KEY_RC_LEN)
#define Message_DefaultReplayCounter(li)    ((li.field.HighPart == 0xffffffff) && (li.field.LowPart == 0xffffffff) ) ?1:0

#ifdef CONFIG_SOFTAP_SUPPORT
int32 wpas_insert_aid(struct wpa_priv *priv, struct sta_info *pstat);
int32 wpas_delete_aid(struct wpa_priv *priv, struct sta_info *pstat);
#endif

void wpas_derive_psk_callback(void* pcontext, uint8 *data);

void wpas_send_timeout_callback(void *pcontext, uint8 *data);

uint8 wpas_priv_alloc(ADAPTER *Adapter);

void wpas_priv_free(ADAPTER *Adapter);

void wpas_psk_event_handler(struct wpa_priv *priv, int32 event_id, uint8 *mac, uint8 *msg, int32 msg_len);

void wpas_psk_init(struct wpa_priv *priv);

#endif

