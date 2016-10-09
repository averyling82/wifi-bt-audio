#ifndef __WLAN_EVENT_CB_H_
#define __WLAN_EVENT_CB_H_


enum WLAN_EVENT
{
    SCAN_RESULT_EVENT,
    SCAN_DONE_EVENT,
    ADD_STA_EVENT,
    DELETE_STA_EVENT,
    MAX_EVENT_NUM
};

struct wlan_event
{
    uint32 evt_code;
    uint32 param_size;
    void (*event_callback)(ADAPTER *dev, uint8 *pbuf);
};

struct scan_result_event
{
    WLAN_BSSID_EX bss;
};

struct scan_done_event
{
    uint32  bss_cnt;

};

struct stassoc_event
{
    uint8 macaddr[6];
    uint8 rsvd[2];
    uint8 Support_rate[16];
};

struct stadel_event
{
    uint8 macaddr[6];
    uint8 rsvd[2];
};

extern void wlan_scan_result_event_callback(ADAPTER *adapter, uint8 *pbuf);
extern void wlan_scan_done_event_callback(ADAPTER *adapter, uint8 *pbuf);
extern void wlan_stassoc_event_callback(ADAPTER *adapter, uint8 *pbuf);
extern void wlan_stadel_event_callback(ADAPTER *adapter, uint8 *pbuf);

#endif
