#ifndef __HAL_CAM_H_
#define __HAL_CAM_H_

#define CAM_CONTENT_COUNT               8
#define CAM_PAIRWISE_KEY_POSITION       4

#define CFG_DEFAULT_KEY                 BIT(5)
#define CFG_VALID                       BIT(15)

#define CAM_VALID                   BIT(15)
#define CAM_WRITE                   BIT(16)
#define CAM_POLLINIG            BIT(31)

uint8 hal_set_default_or_group_key(ADAPTER *Adapter, uint8 *pbuf);
uint8 hal_set_pairwise_key(ADAPTER *Adapter, uint8 *pbuf);
int32 hal_delete_pairwise_key(ADAPTER *Adapter, uint8 *mac_addr, uint32 ul_key_id, uint8 use_dk);
void hal_reset_all_keys(ADAPTER *Adapter);
int32 get_sw_cam_entry(ADAPTER *Adapter, uint8 *addr, uint8 is_free);

#endif
