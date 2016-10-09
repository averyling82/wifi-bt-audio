#ifndef _WLAN_IO_OPS_H_
#define _WLAN_IO_OPS_H_

struct  _io_ops
{
    uint8 (*_read8)(ADAPTER *Adapter, uint32 addr);
    uint16 (*_read16)(ADAPTER *Adapter, uint32 addr);
    uint32 (*_read32)(ADAPTER *Adapter, uint32 addr);

    uint32  (*_write8)(ADAPTER *Adapter, uint32 addr, uint32 buf);
    uint32  (*_write16)(ADAPTER *Adapter, uint32 addr,uint32 buf);
    uint32  (*_write32)(ADAPTER *Adapter, uint32 addr, uint32 buf);

    uint32 (*_read_port)(ADAPTER *Adapter, uint32 addr, uint32 cnt, uint8 *pmem);
    uint32 (*_write_port)(ADAPTER *Adapter, uint32 addr, uint32 cnt, uint8 *pmem);

    void (*_read_port_cancel)(ADAPTER *Adapter);
    void (*_write_port_cancel)(ADAPTER *Adapter);
};

extern uint8 wlan_read8(ADAPTER *adapter, uint32 addr);
extern uint16 wlan_read16(ADAPTER *adapter, uint32 addr);
extern uint32 wlan_read32(ADAPTER *adapter, uint32 addr);
extern void wlan_write8(ADAPTER *adapter, uint32 addr, uint8 val);
extern void wlan_write16(ADAPTER *adapter, uint32 addr, uint16 val);
extern void wlan_write32(ADAPTER *adapter, uint32 addr, uint32 val);

extern void wlan_read_port(ADAPTER *adapter, uint32 addr, uint32 cnt, uint8 *pmem);
extern void wlan_write_port(ADAPTER *adapter, uint32 addr, uint32 cnt, uint8 *pmem);

void write_port_cancel(ADAPTER *adapter);
void read_port_cancel(ADAPTER *adapter);

#endif

