#ifndef __SPI_OPS_H__
#define __SPI_OPS_H__

struct  wifi_hci_ops
{

    struct _io_ops  io_ops;

    void (*rx_tasklet)(ADAPTER *Adapter);

    void (*send_xframe)(ADAPTER *Adapter, struct xmit_frame *pxmitframe);

    void (*xmit_complete)(ADAPTER *Adapter, struct xmit_frame *pxframe);
    void (*xmit_resource_free)(ADAPTER *Adapter, struct xmit_buf *pxmitbuf);
    int32  (*xmit_resource_alloc)(ADAPTER *Adapter, struct xmit_buf *pxmitbuf);

    void (*pkt_complete)(ADAPTER *Adapter, _pkt *pkt);

    void (*set_tx_chksum_offload)(_pkt *pkt, struct pkt_attrib *pattrib);

    int32 (*recv_resource_init)(struct recv_struct *precv_priv, ADAPTER *Adapter);
    int32 (*init_recv_struct)(struct recv_struct *precv_priv, ADAPTER *Adapter);
    void (*free_recv_struct)(struct recv_struct *precv_priv);
    void (*handle_tkip_mic_err)(ADAPTER *Adapter,uint8 bgroup);
    int32  (*recv_resource_alloc)(ADAPTER *Adapter, struct recv_frame *precvframe);
    void (*recv_resource_free)(struct recv_struct *precv_priv);
    void (*free_recv_pkt)(ADAPTER *Adapter, struct recv_frame *precvframe);
    int32 (*recvbuf_to_recvframe)(ADAPTER *Adapter, _pkt *skb, struct rx_pkt_attrib *tmp_attrib);
};

uint32 wifi_hci_ops_init(ADAPTER *Adapter);

#endif

