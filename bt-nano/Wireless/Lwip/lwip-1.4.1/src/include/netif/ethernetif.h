#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "wlan_intf_def.h"
#include  "source_macro.h"


#ifdef   __DRIVER_I6000_WIFI_C__

struct ethernetif;

typedef unsigned char (*tcpip_wifi_xmit_fn)(TCPIP_PACKET_INFO_T	*packet);
typedef void (*ethernet_if_input_fn)(struct ethernetif *eth_if,  RX_PACKET_INFO_T *rx_packet);

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
{
    struct netif *netif;
    struct eth_addr *ethaddr;

    tcpip_wifi_xmit_fn tcpip_wifi_xmit;
    ethernet_if_input_fn eth_if_input;
};


err_t ethernetif_init(struct netif *netif);
//void ethernetif_input(struct ethernetif *eth_if,  RX_PACKET_INFO_T *rx_packet);

#endif
#endif
