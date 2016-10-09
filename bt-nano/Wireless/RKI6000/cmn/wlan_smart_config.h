#ifndef __WLAN_SMART_CONFIG_H__
#define __WLAN_SMART_CONFIG_H__

#include "freertos_types.h"

#define MGMT_MSGTYPE_PROBE_REQ  0x40
#define IE_MSGTYPE_SSID         0

#define MSGTYPE_OFFSET          8
#define SSID_IE_OFFSET          32



#define SMARTCFG_PREAMBLE   0x5E  // '^'

#define SMARTCFG_MSGTYPE_SIGPKT_OPEN    0x61  //'a', single packet, ap is open mode
#define SMARTCFG_MSGTYPE_SIGPKT_SECU    0x42  //'B', single packet, ap is security mode
#define SMARTCFG_MSGTYPE_MLTPKT         0x67  //'g', multi packet

#define SMARTCFG_KEY_BASE   0x28    //40
#define SMARTCFG_KEY_MIN_VLAUE  0x30
#define SMARTCFG_KEY_MAX_VLAUE  0x6F

#define SMARTCFG_CHKSUM_MIN_VALUE   0x40
#define SMARTCFG_CHKSUM_MAX_VALUE   0x7E
#define SMARTCFG_CHKSUM_EXECPTION_VALUE 0x7F

#define SMARTCFG_DATA_MIN_VALUE 0x20
#define SMARTCFG_DATA_MAX_VALUE 0x7E
#define SMARTCFG_DATA_ROUND_VALUE 0x5F


#define SMARTCFG_HDR_LEN            4
#define SMARTCFG_MSGTYPE_OFFSET     1
#define SMARTCFG_CIPHKEY_OFFSET     2
#define SMARTCFG_CONTEXT_OFFSET     3
#define SMARTCFG_MAX_CONTEXT_LEN    28

#define SMARTCFG_MAX_SSID_LEN   32
#define SMARTCFG_MAX_PWD_LEN    64
#define SMARTCFG_MAX_MULTI_CONTEXT_LEN (SMARTCFG_MAX_SSID_LEN+SMARTCFG_MAX_PWD_LEN)

#define SMARTCFG_MLTPKT_HAVE_MORE   TRUE
#define SMARTCFG_MLTPKT_NO_MORE     FALSE

#define SMARTCFG_MLTPKT_MIN_SEQ     0
#define SMARTCFG_MLTPKT_MAX_SEQ     0x11

#define GET_SEQ_BIT(val)    (val & 0x03)
#define GET_MORE_BIT(val)   ((val & 0x08)>>3)
#define VALUE_TO_BITMAP(val)    (0x01<<val)

#define SMARTCFG_MLTPKT_MAX_DATA_LEN    27

typedef struct
{
    uint8 pkt_type;
    uint8 ssid_len;
    uint8 pwd_len;
    uint8 is_smart_config_info_ready;

    uint8 ssid[SMARTCFG_MAX_SSID_LEN];
    uint8 password[SMARTCFG_MAX_PWD_LEN];

    uint8 multi_pkt_nb;
    uint8 multi_pkt_total_size;
    uint8 multi_pkt_bitmap;
    uint8 multi_pkt_context[SMARTCFG_MAX_MULTI_CONTEXT_LEN];
} smart_config_context;

extern void smartconfig_reset_context();
extern boolean is_smartconfig_pkt(uint8 *msghdr, uint8 len);

extern void decode_smartconfig_pkt(uint8 *msghdr, uint8 len, smart_config_context * smart_config);

extern boolean smartconfig_notify_info(void);

extern void smartconfig_start(void);
extern boolean smartconfig_wait_get_info(int32 timeout, uint8 *pssid, int8 *pssid_len, uint8 *ppwd, int8 *ppwd_len);

#endif