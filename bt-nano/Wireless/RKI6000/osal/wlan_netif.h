#ifndef __WLAN_NETIF_H_
#define __WLAN_NETIF_H_

#define TOTAL_SKB_NUM       5//16  //5 //10
#define MAX_SKB_BUF_SIZE    1696

struct skb_priv
{
    ADAPTER *adapter;
    uint32 free_skb_cnt;
    _queue free_skb_queue;
    uint8 * allocated_skb_addr;
    uint8 * skb_addr;
};

int32 wlan_init_skb_priv(struct skb_priv *pskb_priv, ADAPTER *Adapter);
void wlan_free_skb_priv(struct skb_priv *pskb_priv);

struct pkt_buff *wifi_alloc_pktbuf(struct skb_priv *pskb_priv, uint32 size);

int32 wifi_free_pktbuf(struct skb_priv *pskb_priv, struct pkt_buff *skb);

//void skb_put(struct pkt_buff *pskb, int32 len);

uint8 *skb_pull(struct pkt_buff *skb, uint32 len);

uint8 wifi_xmit_entry(TCPIP_PACKET_INFO_T   *packet);

void wifi_recv_indicate_tcpip(ADAPTER *Adapter, struct recv_frame *precv_frame);


#endif

