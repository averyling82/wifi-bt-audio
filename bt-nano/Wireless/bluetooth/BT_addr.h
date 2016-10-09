#ifndef __LWBT_BT_ADDR_H__
#define __LWBT_BT_ADDR_H__

__packed
struct bd_addr
{
    uint8 addr[6];
};

#define BD_ADDR_LEN 6
#endif