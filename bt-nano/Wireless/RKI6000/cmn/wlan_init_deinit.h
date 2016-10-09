#ifndef _WLAN_INIT_DEINIT_H_
#define _WLAN_INIT_DEINIT_H_

uint32 wlan_param_config( ADAPTER *Adapter);
void wlan_init_mac_addr(ADAPTER *Adapter);
uint8 wlan_init_default_value(ADAPTER *Adapter);
void wlan_cancel_all_timer(ADAPTER *Adapter);

uint8 wlan_drv_struct_init(ADAPTER *Adapter);
uint8 wlan_drv_struct_deinit(ADAPTER *Adapter);
int32 wlan_sw_init(ADAPTER *Adapter);
void wlan_sw_deinit(ADAPTER *Adapter);
uint32 wlan_hal_init(ADAPTER *Adapter);
uint32 wlan_hal_deinit(ADAPTER *Adapter);
int32 wlan_dev_open(ADAPTER *Adapter);
void wlan_dev_close(ADAPTER *Adapter);

#endif

