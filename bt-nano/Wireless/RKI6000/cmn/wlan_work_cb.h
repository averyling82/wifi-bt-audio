#ifndef __WLAN_WORK_CB_H_
#define __WLAN_WORK_CB_H_


struct mlme_event_header
{
    uint16 len;
    uint8 ID;
    uint8 seq;

    void    *event_work;
};

void wlan_xmitframe_callback(void *context, uint8 *data);

void wlan_recv_callback(void *context, uint8 *data);

void wlan_dyn_chk_callback(void *Adapter, uint8 *data);

void wlan_scan_callback(void *adapter, uint8 *data);

void wlan_dsr_callback(void *context, uint8 *data);

void wlan_fw_msg_callback(void *context, uint8 *data);

void wlan_join_callback(void *context, uint8 *pbuf);

void wlan_disconnect_callback(void *context, uint8 *pbuf);

void wlan_event_callback(void *context, uint8 *pbuf);

#endif
