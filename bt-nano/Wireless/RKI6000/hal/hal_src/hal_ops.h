#ifndef __HAL_OPS_H__
#define __HAL_OPS_H__


struct  hal_ops
{
    uint8 (*power_on)(ADAPTER *Adapter, uint32 IpsFlow);
    uint32 (*init)(PADAPTER Adapter, uint32 download_firmware);
    uint32 (*halt)(PADAPTER Adapter, uint32 IpsFlow);
    uint32 (*reset)(PADAPTER Adapter);

    boolean (*get_chip_id)(PADAPTER Adapter);
    void (*get_chip_info)(PADAPTER Adapter);

    int32  (*init_hal_data) (PADAPTER Adapter);
    void (*deinit_hal_data) (PADAPTER Adapter);

    void (*watchdog_init)(ADAPTER *Adapter);
    void (*watchdog_func)(PADAPTER Adapter);

    void (*init_interrupt) (PADAPTER Adapter);
    void (*enable_interrupt) (PADAPTER Adapter);
    void (*disable_interrupt) (PADAPTER Adapter);
    void (*update_interrupt_mask) (PADAPTER Adapter, uint32 add_msr, uint32 rm_msr);
    boolean (*recognize_interrupt) (PADAPTER    Adapter, uint32*    interrupt_type);

    void (*set_network_type) (PADAPTER Adapter, NDIS_802_11_NETWORK_INFRASTRUCTURE type);
    void (*set_bssid_check) (PADAPTER Adapter, boolean check_bssid);
    void (*config_beacon) (PADAPTER Adapter, NDIS_802_11_NETWORK_INFRASTRUCTURE type, uint16    BcnInterval,uint16  AtimWindow);
    void (*read_reg) (PADAPTER Adapter, uint32 address, uint32 *val);
    void (*write_reg) (PADAPTER Adapter, uint32 variable, uint32 *val);
    void (*write_reg_new) (PADAPTER Adapter, uint32 address, uint32 *val);
    uint8   (*set_rate_mask)(ADAPTER*Adapter, uint32 mask, uint8 arg);
    uint8   (*set_rate_table)(PADAPTER  Adapter, uint8  raid,   uint32  mask);
    void (*fix_tx_rate)(PADAPTER Adapter, uint8 rate);
    uint32 (*queue_mapping)(uint32 qsel);
    uint32 (*get_wlan_header_offset)(void);

    void (*set_tx_desc) (struct xmit_frame *pxmitframe, uint32 *pmem, int32 sz, uint8 frag_idx);
    void (*parse_rx_desc) (ADAPTER *Adapter, struct rx_pkt_attrib *pattrib, uint8 *pbuf);
    uint32  (*get_rx_pkt_len) (PADAPTER Adapter);

    uint8 (*set_default_or_group_key)(ADAPTER *Adapter, uint8 *pbuf);
    uint8 (*set_pairwise_key)(ADAPTER *Adapter, uint8 *pbuf);
    int32 (*delete_pairwise_key)(ADAPTER *Adapter, uint8 *pMacAddr, uint32 keyId, uint8 useDK);
    void (*reset_all_keys)(ADAPTER *Adapter);

    void (*init_bb)(ADAPTER *Adapter);
    void (*deinit_bb)(ADAPTER *Adapter);
    void (*init_rf)(ADAPTER *Adapter);
    void (*deinit_rf)(ADAPTER *Adapter);
    void (*set_bandwidth)(PADAPTER Adapter, HT_CHANNEL_WIDTH Bandwidth, uint8 Offset);
    void (*switch_channel)(PADAPTER Adapter, uint8 channel);
    uint32  (*read_bb_reg)(PADAPTER Adapter, uint8 reg_addr);
    void (*write_bb_reg)(PADAPTER Adapter, uint8 reg_addr, uint32 data);
    uint32  (*read_rf_reg)(PADAPTER Adapter, uint8 rfpath, uint32 regaddr, uint32 bitmask);
    void (*write_rf_reg)(PADAPTER Adapter, uint8 rfpath, uint32 regaddr, uint32 bitmask, uint32 data);

    void (*lps_ctrl)(ADAPTER *Adapter, uint8 en_lps);
    void (*get_ps_state)(ADAPTER *Adapter, uint32 *val);

#ifdef CONFIG_IDLE_PS_SUPPORT
    void (*enter_idle_ps)(ADAPTER *Adapter);
    void (*leave_idle_ps)(ADAPTER *Adapter);
#endif

//#ifdef CONFIG_WOWLAN_SUPPORT
    void (*wowlan_ctrl)(ADAPTER *Adapter, uint32 en_wowlan);
//#endif
    void (*wlan_info_notify)(PADAPTER   Adapter, uint8  mstatus);
    void (*keep_alive_ctrl)(PADAPTER pAdapter, boolean bFuncEn);
    void (*disconnect_try_ctrl)(PADAPTER pAdapter, uint32 need_ack);

    uint8 (*disconnect_handle)(ADAPTER *Adapter, uint8 *pbuf);
    void (*check_ap_alive)(ADAPTER * Adapter, struct sta_info   *psta);
};

void wifi_hal_ops_init(ADAPTER * pAdapter);

#endif

