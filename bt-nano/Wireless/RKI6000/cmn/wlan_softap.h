#ifndef _WLAN_SOFTAP_H_
#define _WLAN_SOFTAP_H_

#ifdef CONFIG_SOFTAP_SUPPORT

void wlan_start_create_bss(ADAPTER* Adapter);

/* these functions are just used for softap sta power save */
__inline static int32 wlan_any_sta_sleep(ADAPTER *Adapter, struct sta_struct  *psta_global)
{
    if (psta_global->sta_dz_bitmap & (~ BIT(0)))
        return _TRUE;
    else
        return _FALSE;
}

__inline static void wlan_set_sta_sleep(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta)
{
    //psta->state |= WIFI_SLEEP_STATE;
    psta_global->sta_dz_bitmap |= BIT(psta->aid);
}

__inline static void wlan_clr_sta_sleep(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta)
{
    //psta->state &= ~WIFI_SLEEP_STATE;
    psta_global->sta_dz_bitmap &= (~ BIT(psta->aid));
}

__inline static int32 wlan_get_sta_queued(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta)
{
    if (psta_global->tim_bitmap & BIT(psta->aid))
        return _TRUE;
    else
        return _FALSE;
}

__inline static void wlan_set_sta_queued(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta)
{
    psta_global->tim_bitmap |= BIT(psta->aid);
}

__inline static void wlan_clr_sta_queued(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta)
{
    psta_global->tim_bitmap &= (~ BIT(psta->aid));
}

__inline static int32 wlan_get_bcmc_queued(ADAPTER *Adapter, struct sta_struct  *psta_global)
{
    if (psta_global->sta_dz_bitmap & BIT(0))
        return _TRUE;
    else
        return _FALSE;
}

__inline static void wlan_set_bcmc_queued(ADAPTER *Adapter, struct sta_struct  *psta_global)
{
    psta_global->tim_bitmap |= BIT(0);
    psta_global->sta_dz_bitmap |= BIT(0);
}

__inline static void wlan_clr_bcmc_queued(ADAPTER *Adapter, struct sta_struct  *psta_global)
{
    psta_global->tim_bitmap &= (~ BIT(0));
    psta_global->sta_dz_bitmap &= (~ BIT(0));
}

void wlan_relay_other_sta_pkt(ADAPTER *Adapter, uint8 *data_ptr, uint32 data_len);
int32 wlan_get_sta_sleep(ADAPTER *Adapter, struct sta_struct  *psta_global, struct sta_info *psta);
void wlan_set_frame_more_data(ADAPTER *Adapter, struct xmit_frame *pxmitframe);
int32 wlan_check_sta_continue_tx(ADAPTER *padapter, struct sta_info *psta);
void wlan_sleep_sta_tim_map(ADAPTER *padapter, struct sta_info *psta);
void wlan_tx_bcmc_after_bcn_early(ADAPTER *padapter);
void wlan_start_wake_sta_xmit(ADAPTER *padapter, struct sta_info *psta);
void wlan_update_beacon_hdl(ADAPTER *padapter );

void wlan_rx_process_pwrbit(ADAPTER *padapter, struct recv_frame *precv_frame);

void wlan_rx_process_pspoll(ADAPTER *padapter, struct recv_frame *precv_frame);

#endif
#endif  //_WLAN_AP_H_

