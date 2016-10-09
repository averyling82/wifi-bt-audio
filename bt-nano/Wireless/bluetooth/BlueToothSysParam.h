#ifndef BLUETOOTH_SYS_PARAM_H
#define BLUETOOTH_SYS_PARAM_H


int  save_key_to_sysconfig(struct bd_addr *bdaddr, uint8 *key);
uint8* get_link_key(struct bd_addr *bdaddr);
void BlueToothDeletePairedDve(struct bd_addr *bdaddr);
uint8* get_last_connect_dev(void);
void updata_last_connect_dev(struct bd_addr *bdaddr);

#endif
