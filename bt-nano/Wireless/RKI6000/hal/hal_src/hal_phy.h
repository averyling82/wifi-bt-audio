#ifndef __HAL_PHY_H
#define __HAL_PHY_H


#define FILTER_COEFFICIENTS_LEN 86

uint32  hal_read_bb_reg(PADAPTER Adapter, uint8 reg_addr);
void hal_write_bb_reg(PADAPTER Adapter, uint8 reg_addr, uint32 data);

void hal_switch_channel(PADAPTER Adapter, uint8 channel);

WLAN_STATUS hal_init_bb(IN PADAPTER Adapter);
WLAN_STATUS hal_init_rf(IN PADAPTER Adapter);

#endif

