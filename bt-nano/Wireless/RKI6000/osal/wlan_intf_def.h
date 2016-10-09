#ifndef __WLAN_INTF_DEF_H
#define __WLAN_INTF_DEF_H

#include <stddef.h>
#include <stdarg.h>
#include "freertos_types.h"

/*******************************************************************************/

#define MAX_PBUF_NUM_A_PACKET   6

/*******************************************************************************/

typedef  uint32     TCPIP_IP_ADDR_T;
typedef  uint32     TCPIP_NET_ID_T;
typedef uint8       boolean_T;

typedef enum
{
    TCPIP_PKTTYPE_NULL = 0,
    TCPIP_PKTTYPE_IP,       /* packet is encapsulated as IP */
    TCPIP_PKTTYPE_ETHER,    /* packet is encapsulated as Ethernet */
    TCPIP_PKTTYPE_MAX
} TCPIP_PACKET_TYPE_E;

// TCPIP net interface IP addresses
// NOTES : IP addresses should be in Big-Ending
typedef struct _netif_ipaddr_tag
{
    TCPIP_IP_ADDR_T  ipaddr;     /* host IP */
    TCPIP_IP_ADDR_T  snmask;     /* subnet mask */
    TCPIP_IP_ADDR_T  gateway;    /* gateway */
    TCPIP_IP_ADDR_T  dns1;       /* primary DNS */
    TCPIP_IP_ADDR_T  dns2;       /* secondary DNS */
} TCPIP_NETIF_IPADDR_T;

// TCPIP net interface hardware address
typedef struct _netif_haddr_tag
{
    uint8 *addr_ptr;           /* hard address pointer */
    uint32 addr_len;           /* hard address length, unit: byte */
} TCPIP_NETIF_HADDR_T;


// TCPIP packet info struct
typedef struct _packet_info_tag
{
    uint8*          data_ptr[MAX_PBUF_NUM_A_PACKET];   /* data pointer */
    uint16           data_len[MAX_PBUF_NUM_A_PACKET];   /* data length */
    uint16         pbuf_num;
    uint16         packet_len;
} TCPIP_PACKET_INFO_T;

typedef struct _rx_packet_info_tag
{
    uint8*          data_ptr;   /* data pointer */
    uint32                data_len;   /* data length - full packet encapsulation length */
} RX_PACKET_INFO_T;


/*******************************************************************************/
/**--------------------------------------------------------------------------*
 **                         MACRO DEFINITION                                 *
 **--------------------------------------------------------------------------*/


/*******************************************************************************/

/**--------------------------------------------------------------------------*
 **                         MACRO DEFINITION                                 *
 **--------------------------------------------------------------------------*/
#define WIFISUPP_BSSID_LEN              6               //the length of BSSID in bytes
#define WIFISUPP_SSID_MAX_LEN           32              //the MAX length of SSID in bytes
#define WIFISUPP_WEP_IV_LEN             3               //the length of IV in WEP
#define WIFISUPP_WEP_64BITS_KEY_LEN     8               //the length of 64bits WEP key
#define WIFISUPP_WEP_128BITS_KEY_LEN    16              //the length of 128bits WEP key
#define WIFISUPP_WPA_PSK_LEN            63              //the length of WPA PSK
#define WIFISUPP_MAX_FILE_PATH_LEN      255    //the MAX file path supported
#define WIFISUPP_WEP_KEY_ID_MAX         4               //the max WEP key ID

#define WIFISUPP_MAX_USERNAME_LEN       32
#define WIFISUPP_MAX_PSW_LEN            32
/**--------------------------------------------------------------------------*
 **                         typedef                                          *
 **--------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*                          Enum                                             */
/*---------------------------------------------------------------------------*/
typedef enum
{
    WIFISUPP_NETWORK_MODE_INFRASTRUCTURE,   //infrastructure
    WIFISUPP_NETWORK_MODE_ADHOC,            //ad-hoc
    WIFISUPP_NETWORK_MODE_MAX
} WIFISUPP_NETWORK_MODE_E;

typedef enum
{
    WIFISUPP_RESULT_SUCC,           //success
    WIFISUPP_RESULT_FAIL,           //fail
    WIFISUPP_RESULT_NOT_FOUND,      //the AP is not found
    WIFISUPP_RESULT_TIMEOUT,        //operation time out
    WIFISUPP_RESULT_RESTRICTED,     //connection is restricted
    WIFISUPP_RESULT_MAX
} WIFISUPP_RESULT_E;

typedef enum
{
    WIFISUPP_ENCRYP_PROTOCOL_OPENSYS,   //open system
    WIFISUPP_ENCRYP_PROTOCOL_WEP,       //WEP
    WIFISUPP_ENCRYP_PROTOCOL_WPA,       //WPA
    WIFISUPP_ENCRYP_PROTOCOL_WPA2,      //WPA2
    WIFISUPP_ENCRYP_PROTOCOL_WAPI,      //WAPI: Not support in this version
    WIFISUPP_ENCRYP_PROTOCOL_EAP,      //WAPI
    WIFISUPP_ENCRYP_PROTOCOL_MAX
} WIFISUPP_ENCRYP_PROTOCOL_E;

typedef enum
{
    WIFISUPP_WEP_KEY_TYPE_64BITS,       //64bits_type WEP Key
    WIFISUPP_WEP_KEY_TYPE_128BITS,      //128bits_type WEP Key
    WIFISUPP_WEP_KEY_TYPE_MAX
} WIFISUPP_WEP_KEY_TYPE_E;

typedef enum
{
    WIFISUPP_WPA_CREDENTIAL_TYPE_PSK,   //WPA using PSK
    WIFISUPP_WPA_CREDENTIAL_TYPE_EAP,   //WPA using EAP
    WIFISUPP_WPA_CREDENTIAL_TYPE_MAX
} WIFISUPP_WPA_CREDENTIAL_TYPE_E;

typedef enum
{
    WIFISUPP_WPA_EAP_TYPE_TLS,          //EAP using TLS
    WIFISUPP_WPA_EAP_TYPE_TTLS,         //EAP using TTLS
    WIFISUPP_WPA_EAP_TYPE_PEAP,         //EAP using PEAP
    WIFISUPP_WPA_EAP_TYPE_LEAP,         //EAP using LEAP
    WIFISUPP_WPA_EAP_TYPE_SIM,          //EAP using SIM
    WIFISUPP_WPA_EAP_TYPE_AKA,          //EAP using AKA
    WIFISUPP_WPA_EAP_TYPE_FAST,         //EAP using FAST
    WIFISUPP_WPA_EAP_TYPE_MAX
} WIFISUPP_WPA_EAP_TYPE_E;

typedef enum
{
    WIFISUPP_INNER_CREDENTIAL_TYPE_CHAP,        //CHAP credential
    WIFISUPP_INNER_CREDENTIAL_TYPE_PAP,         //PAP credential
    WIFISUPP_INNER_CREDENTIAL_TYPE_MSCHAP,      //MSCHAP credential
    WIFISUPP_INNER_CREDENTIAL_TYPE_MSCHAP_V2,   //MACHAP_V2 credential
    WIFISUPP_INNER_CREDENTIAL_TYPE_GTC,         //GTC credential
    WIFISUPP_INNER_CREDENTIAL_TYPE_MAX
} WIFISUPP_INNER_CREDENTIAL_TYPE_E;

typedef enum
{
    WIFISUPP_DHCP_NOTICE_START,     //APP begin to do dhcp
    WIFISUPP_DHCP_NOTICE_END,           //APP end dhcp
    WIFISUPP_DHCP_NOTICE_MAX
} WIFISUPP_DHCP_NOTICE_E;

typedef enum
{
    WIFISUPP_CIPHER_TKIP,       //TKIP
    WIFISUPP_CIPHER_CCMP,       //CCMP
    WIFISUPP_CIPHER_WEP,
    WIFISUPP_CIPHER_SMS4,       //WAPI SMS-4
    WIFISUPP_CIPHER_MAX
} WIFISUPP_CIPHER_E;

typedef enum
{
    WIFISUPP_PRIV_READ_MAC_REG,
    WIFISUPP_PRIV_WRITE_MAC_REG,
    WIFISUPP_PRIV_READ_PHY_REG,
    WIFISUPP_PRIV_WRITE_PHY_REG,
    WIFISUPP_PRIV_SET_CHANNEL,
    WIFISUPP_PRIV_SET_TX_RATE,
    WIFISUPP_PRIV_ENABLE_PS,
    WIFISUPP_PRIV_DISABLE_PS,
    WIFISUPP_PRIV_GET_PS,
    WIFISUPP_PRIV_MAX
} WIFISUPP_PRIV_REQ_E;

/*---------------------------------------------------------------------------*/
/*                          Struct                                           */
/*---------------------------------------------------------------------------*/
typedef struct
{
    uint8 ssid[WIFISUPP_SSID_MAX_LEN];
    uint8 ssid_len;
} WIFISUPP_SSID_T;  //SSID

typedef struct
{
    uint8 bssid[WIFISUPP_BSSID_LEN];
} WIFISUPP_BSSID_T; //BSSID


#define WIFISUPP_WAPI_PSK_LEN           64

typedef enum
{
    WIFISUPP_WAPI_CREDENTIAL_TYPE_PSK,
    WIFISUPP_WAPI_CREDENTIAL_TYPE_CER,
    WIFISUPP_WAPI_CREDENTIAL_TYPE_MAX
} WIFISUPP_WAPI_CREDENTIAL_TYPE_E;

typedef struct WIFISUPP_WAPI_PSK_tag
{
    uint8   psk[WIFISUPP_WAPI_PSK_LEN];
} WIFISUPP_WAPI_PSK_T;   //WAPI Pre_shared Key

typedef struct
{
    uint8    *as_certificate_buf_ptr;
    uint32    as_certificate_buf_len;
    uint8    *asue_certificate_buf_ptr;
    uint32    asue_certificate_buf_len;
} WIFISUPP_WAPI_CER_T;

typedef struct WIFISUPP_WAPI_CREDENTIAL_tag
{
    WIFISUPP_WAPI_CREDENTIAL_TYPE_E  credential_type;
    union
    {
        WIFISUPP_WAPI_PSK_T  wapi_psk_info;
        WIFISUPP_WAPI_CER_T  wapi_cer_info;
    } credential_info;
} WIFISUPP_WAPI_CREDENTIAL_T;

typedef enum
{
    WIFISUPP_CREDENTIAL_TYPE_PSK,
    WIFISUPP_CREDENTIAL_TYPE_RADIUS,
    WIFISUPP_CREDENTIAL_TYPE_CER,
    WIFISUPP_CREDENTIAL_TYPE_MAX
} WIFISUPP_CREDENTIAL_TYPE_E;

typedef struct wifisupp_scan_ap_info
{
    struct wifisupp_scan_ap_info  *next_ptr;
    WIFISUPP_SSID_T            ssid;
    WIFISUPP_BSSID_T          bssid_info;
    int32                           rssi;
    uint32                          noise;              //SNR: signal noise ratio
    uint32                          beacon_interval;    //the BEACON interval
    uint32                          channel;            //the channel using
    WIFISUPP_ENCRYP_PROTOCOL_E      encryp_protocol;    //the protocol used by encryption
    uint32                          rate;               //the rate
    WIFISUPP_NETWORK_MODE_E         network_mode;       //network mode

    WIFISUPP_CIPHER_E               pairwise_cipher;
    WIFISUPP_CIPHER_E               group_cipher;
    WIFISUPP_CREDENTIAL_TYPE_E      credential_type;    //AKM type
} WIFISUPP_SCAN_AP_INFO_T;  //the AP info scanned

typedef struct
{
    uint8   psk[WIFISUPP_WPA_PSK_LEN];
} WIFISUPP_WPA_PSK_T;   //WPA Pre_shared Key

typedef struct
{
    WIFISUPP_SSID_T     ssid;
} WIFISUPP_DISCONNECT_REQ_INFO_T;   //information in the request for disconnecting a connection


typedef struct
{
    WIFISUPP_WPA_CREDENTIAL_TYPE_E  credential_type;
    union
    {
        WIFISUPP_WPA_PSK_T  wpa_psk_info;
        //WIFISUPP_WPA_EAP_T  wpa_eap_info;
    } credential_info;
} WIFISUPP_WPA_CREDENTIAL_T;    //WPA credential information

typedef struct
{
    WIFISUPP_WEP_KEY_TYPE_E     key_type;
    uint8                    is_open_mode;
    uint32                      key_in_use;
    union
    {
        uint8   key_64bits_arr[WIFISUPP_WEP_KEY_ID_MAX][WIFISUPP_WEP_64BITS_KEY_LEN - WIFISUPP_WEP_IV_LEN];
        uint8   key_128bits_arr[WIFISUPP_WEP_KEY_ID_MAX][WIFISUPP_WEP_128BITS_KEY_LEN - WIFISUPP_WEP_IV_LEN];
    } key;
} WIFISUPP_WEP_CREDENTIAL_T;    //WEP credential information


typedef struct
{
    WIFISUPP_SSID_T             ssid;
    WIFISUPP_BSSID_T            bssid_info;
    WIFISUPP_NETWORK_MODE_E     network_mode;
    WIFISUPP_ENCRYP_PROTOCOL_E  encryp_protocol;
    uint32                      channel;
    union
    {
        WIFISUPP_WEP_CREDENTIAL_T   wep_credential;
        WIFISUPP_WPA_CREDENTIAL_T   wpa_credential;
        WIFISUPP_WAPI_CREDENTIAL_T  wapi_credential;
    } credential;

    WIFISUPP_CIPHER_E           pairwise_cipher;
    WIFISUPP_CIPHER_E           group_cipher;
} WIFISUPP_SSID_CONFIG_T;   //the config of a SSID

typedef struct
{
    WIFISUPP_RESULT_E   result;
    WIFISUPP_SSID_T     ssid;
    WIFISUPP_BSSID_T    bssid_info;
    uint32             channel;
    long               rssi;
} WIFISUPP_CONNECT_RESULT_T;    //connect result


typedef struct
{
    WIFISUPP_RESULT_E   result;
    WIFISUPP_SSID_T     ssid;
} WIFISUPP_DISCONNECT_RESULT_T; //disconnect result


typedef struct
{
    WIFISUPP_SSID_T     ssid;
} WIFISUPP_DISCONNECT_INFO_T;   //disconnect information


typedef struct
{
    WIFISUPP_SSID_T     ssid;
    WIFISUPP_BSSID_T    new_bssid_info;
} WIFISUPP_BSSID_CHANGED_INFO_T;    //new bssid information


typedef struct
{
    WIFISUPP_SSID_T     ssid;
    WIFISUPP_BSSID_T    bssid_info;
    uint32              new_signal_qua;
} WIFISUPP_RSSI_CHANGED_INFO_T; //RSSI information

typedef struct
{
    WIFISUPP_SSID_T     ssid;
} WIFISUPP_SCAN_REQ_INFO_T;

typedef struct
{
    WIFISUPP_PRIV_REQ_E     priv_req;
    uint32  priv_param1;
    uint32  priv_param2;
    uint32  priv_param3;
    uint32  priv_param4;
    uint32  *result_data;
} WIFISUPP_PRIV_REQ_INFO_T;

typedef struct
{
    int32   used_heap_size;
    int32   max_used_heap_size;
    int32   free_heap_size;
    int32   min_free_heap_size;
} WIFISUPP_USED_HEAP_INFO_T;

enum
{
    MP_OPEN = 0,
    MP_CLOSE,
    MP_SET_MACADDR,
    MP_SET_CHANNEL,
    MP_SET_RATE,
    MP_TX_POWER,
    MP_TX_PKT,
    MP_SET_RX,
    MP_QUERY_TRX,
    MP_CUSTOM,
    MP_MAX
};

typedef int32 (*wifi_priv_handler)(uint8 *extra);

__inline void wifi_supp_notifyScanCnf(WIFISUPP_RESULT_E * result)
{
    return;
}

#endif

