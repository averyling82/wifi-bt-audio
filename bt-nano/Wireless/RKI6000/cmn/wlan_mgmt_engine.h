#ifndef __WLAN_MGMT_ENGINE_H_
#define __WLAN_MGMT_ENGINE_H_

#ifdef CONFIG_SOFTAP_SUPPORT
#ifdef SUPPORT_ACCP_MAC_CTL
struct wlan_acl_node
{
    _list   list;
    uint8       addr[ETH_ALEN];
    uint8       mode;
};
#endif
#endif

enum Associated_AP
{
    atherosAP   = 0,
    broadcomAP  = 1,
    ciscoAP     = 2,
    marvellAP   = 3,
    ralinkAP    = 4,
    realtekAP   = 5,
    airgocapAP  = 6,
    unknownAP   = 7,
    maxAP
};


struct mlme_handler
{
    uint32 num;
    uint8 *str;
    uint32 (*func)(ADAPTER *Adapter, struct recv_frame *precv_frame);
};

struct action_handler
{
    uint32 num;
    uint8 *str;
    uint32 (*func)(ADAPTER *Adapter, struct recv_frame *precv_frame);
};

uint32 do_nothing(ADAPTER *Adapter, struct recv_frame *precv_frame);

void  wlan_mgmt_dispatcher(ADAPTER *Adapter, struct recv_frame *precv_frame);

extern struct xmit_frame *wlan_alloc_mgmtframe(struct xmit_struct *pxmit_priv);

void wlan_update_mgmtframe_attrib(ADAPTER *Adapter, struct pkt_attrib *pattrib);
struct xmit_frame *wlan_alloc_and_init_mgmtframe(ADAPTER *Adapter);
void wlan_send_mgmtframe(ADAPTER *Adapter, struct xmit_frame *pmgntframe);

void wlan_send_assoc_req(ADAPTER *Adapter);
void wlan_send_auth(ADAPTER *Adapter, struct sta_info *psta, uint16 status);
//  Added by Albert 2010/07/26
//  blnbc: 1 -> broadcast probe request
//  blnbc: 0 -> unicast probe request. The address 1 will be the BSSID.
void wlan_send_probe_req(ADAPTER *Adapter, uint8 blnbc);
void wlan_send_null_data(ADAPTER *Adapter, uint32 power_mode);
void wlan_send_deauth(ADAPTER *Adapter, uint8 *da, uint16 reason);
void wlan_send_disassoc(ADAPTER *Adapter, uint8 *da, int32 reason);

uint32 wlan_assoc_rsp_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_probe_rsp_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_beacon_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_atim_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_disassoc_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_sta_auth_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_deauth_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_action_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);

uint32 wlan_action_qos_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_action_dls_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);

uint32 wlan_action_public_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_action_wmm_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);

#ifdef CONFIG_SOFTAP_SUPPORT
uint32 wlan_probe_req_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_assoc_req_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
uint32 wlan_ap_auth_recved(ADAPTER *Adapter, struct recv_frame *precv_frame);
void wlan_send_beacon(ADAPTER *Adapter);
void wlan_send_probe_rsp(ADAPTER *Adapter, uint8 *da);
void wlan_send_assoc_rsp(ADAPTER *Adapter, uint16 status, struct sta_info *psta, int32 is_reassoc);
#endif

#endif

