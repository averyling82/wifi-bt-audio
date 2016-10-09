#ifndef _WLAN_TRANSMIT_H_
#define _WLAN_TRANSMIT_H_

#define NR_XMITFRAME    15  // 4    //8 //256

#define MAX_XMITBUF_SZ  (1600)  //(2048)

#define NR_XMITBUFF     (2)     //(4)

#define XMITBUF_ALIGN_SZ 4

#define MAX_NUMBLKS     (1)

#define XMIT_QUEUE_ENTRY    (4)

#define XMIT_VO_QUEUE (0)
#define XMIT_VI_QUEUE (1)
#define XMIT_BE_QUEUE (2)
#define XMIT_BK_QUEUE (3)



#define WEP_IV(pattrib_iv, dot11txpn, keyidx)\
do{\
    pattrib_iv[0] = dot11txpn._byte_.TSC0;\
    pattrib_iv[1] = dot11txpn._byte_.TSC1;\
    pattrib_iv[2] = dot11txpn._byte_.TSC2;\
    pattrib_iv[3] = ((keyidx & 0x3)<<6);\
    dot11txpn.val = (dot11txpn.val == 0xffffff) ? 0: (dot11txpn.val+1);\
}while(0)


#define TKIP_IV(pattrib_iv, dot11txpn, keyidx)\
do{\
    pattrib_iv[0] = dot11txpn._byte_.TSC1;\
    pattrib_iv[1] = (dot11txpn._byte_.TSC1 | 0x20) & 0x7f;\
    pattrib_iv[2] = dot11txpn._byte_.TSC0;\
    pattrib_iv[3] = BIT(5) | ((keyidx & 0x3)<<6);\
    pattrib_iv[4] = dot11txpn._byte_.TSC2;\
    pattrib_iv[5] = dot11txpn._byte_.TSC3;\
    pattrib_iv[6] = dot11txpn._byte_.TSC4;\
    pattrib_iv[7] = dot11txpn._byte_.TSC5;\
    dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0: (dot11txpn.val+1);\
}while(0)

#define AES_IV(pattrib_iv, dot11txpn, keyidx)\
do{\
    pattrib_iv[0] = dot11txpn._byte_.TSC0;\
    pattrib_iv[1] = dot11txpn._byte_.TSC1;\
    pattrib_iv[2] = 0;\
    pattrib_iv[3] = BIT(5) | ((keyidx & 0x3)<<6);\
    pattrib_iv[4] = dot11txpn._byte_.TSC2;\
    pattrib_iv[5] = dot11txpn._byte_.TSC3;\
    pattrib_iv[6] = dot11txpn._byte_.TSC4;\
    pattrib_iv[7] = dot11txpn._byte_.TSC5;\
    dot11txpn.val = dot11txpn.val == 0xffffffffffffULL ? 0: (dot11txpn.val+1);\
}while(0)



struct  hw_xmit
{
    _list   pending;
    _queue *sta_queue;
    struct hw_txqueue *phwtxqueue;
    int32   txcmdcnt;
    int accnt;
};

struct pkt_attrib
{
    uint32 uplayer_data;
    uint8 type;
    uint8 subtype;
    uint8 bswenc;
    uint8 dhcp_pkt;
    uint16 ether_type;
    uint32 pktlen;      //the original 802.3 pkt raw_data len (not include ether_hdr data)
    int pkt_hdrlen; //the original 802.3 pkt header len
    uint32 hdrlen;      //the WLAN Header Len
    uint32 nr_frags;
    uint32 last_txcmdsz;
    int encrypt;    //when 0 indicate no encrypt. when non-zero, indicate the encrypt algorith
    uint8 iv[8];
    uint32 iv_len;
    uint8 icv[8];
    uint32 icv_len;
    int priority;
    int ack_policy;
    int mac_id;
    int vcs_mode;   //virtual carrier sense method

    uint8 dst[ETH_ALEN];
    uint8 src[ETH_ALEN];
    uint8 ta[ETH_ALEN];
    uint8 ra[ETH_ALEN];

    uint8 key_idx;

    uint8 qos_en;
    uint8 ht_en;
    uint8 raid;//rate adpative id
    uint8 bwmode;
    uint8 ch_offset;//PRIME_CHNL_OFFSET
    uint8 sgi;//short GI
    uint8 ampdu_en;//tx ampdu enable

    uint8  pctrl;//per packet txdesc control enable

    uint32 qsel;
    uint16 seqnum;

    struct sta_info * psta;

    /* for usb pkt size if 512x or 64x*/
    uint32 pull;
    uint32 action;
    uint8 is_fw_image;
};


#define WLANHDR_OFFSET  64

#define NULL_FRAMETAG       (0x0)
#define DATA_FRAMETAG       0x01
#define L2_FRAMETAG     0x02
#define MGNT_FRAMETAG       0x03
#define AMSDU_FRAMETAG  0x04
#define BEACON_FRAMETAG 0x05

#define EII_FRAMETAG        0x05
#define IEEE8023_FRAMETAG  0x06

#define MP_FRAMETAG     0x07


#define TXAGG_FRAMETAG  0x08


struct xmit_buf
{
    _list   list;

    ADAPTER *padapter;

    uint8 *pallocated_buf;

    uint8 *pbuf;

    void *priv_data;

    uint8 flags;

};

struct xmit_frame
{
    _list   list;

    struct pkt_attrib attrib;

    _pkt *pkt;

    int32 frame_tag;

    ADAPTER *padapter;

    uint8 *buf_addr;

    struct xmit_buf *pxmitbuf;

    uint8 pkt_offset; //after desc and before wireless header
    uint8 pkt_alligment; //after whloe pkt at tail

};

struct tx_servq
{
    _list   tx_pending;
    _queue  sta_pending;
    int32 qcnt;
};



struct sta_xmit_struct
{
    _queue sta_xmitframe_queue;

    uint16 txseq_tid[16];
};


struct  hw_txqueue
{
    volatile int32  head;
    volatile int32  tail;
    volatile int32  free_sz;    //in units of 64 bytes
    volatile int32      free_cmdsz;
    volatile int32   txsz[8];
    uint32  ff_hwaddr;
    uint32  cmd_hwaddr;
    int32   ac_tag;
};

struct tx_stats
{
    uint32  IperfTx;
    uint32  NumTxOkInPeriod;
    uint64  NumTxOkByteInPeriod;
    uint64  tx_pkts;
    uint64  last_tx_pkts;
    uint64  tx_drop;
};

struct  xmit_struct
{
    _lock   lock;

    _queue xmit_queue;
    _queue xmitframe_queue;
    /*
        _queue  be_pending;
        _queue  bk_pending;
        _queue  vi_pending;
        _queue  vo_pending;
        _queue  bm_pending;
    */
    uint8 *pallocated_frame_buf;
    uint8 *pxmit_frame_buf;
    uint32 free_xmitframe_cnt;
    _queue free_xmit_queue;

    /*
        struct  hw_txqueue  be_txqueue;
        struct  hw_txqueue  bk_txqueue;
        struct  hw_txqueue  vi_txqueue;
        struct  hw_txqueue  vo_txqueue;
        struct  hw_txqueue  bmc_txqueue;
    */

    uint32  frag_len;

    ADAPTER *adapter;

    uint8   vcs_setting;
    uint8   vcs;
    uint8   vcs_type;
    uint16  rts_thresh;

    uint16  nqos_ssn;
    struct hw_xmit *hwxmits;
    uint8   hwxmit_entry;

    _queue free_xmitbuf_queue;
    _queue pending_xmitbuf_queue;
    uint8 *pallocated_xmitbuf;
    uint8 *pxmitbuf;
    uint32 free_xmitbuf_cnt;
};

__inline static void    wlan_init_sta_xmit_struct(struct sta_xmit_struct *psta_xmit_priv)
{
    os_memset((uint8 *)psta_xmit_priv->txseq_tid, 0, 32);
    /*
        _init_txservq(&psta_xmit_priv->be_q);
        _init_txservq(&psta_xmit_priv->bk_q);
        _init_txservq(&psta_xmit_priv->vi_q);
        _init_txservq(&psta_xmit_priv->vo_q);
        psta_xmit_priv->be_q.qcnt = 0;
        psta_xmit_priv->bk_q.qcnt = 0;
        psta_xmit_priv->vi_q.qcnt = 0;
        psta_xmit_priv->vo_q.qcnt = 0;
    */
}

void wlan_enqueue_frame(_queue *frame_queue, struct xmit_frame *frame);

struct xmit_frame *wlan_dequeue_frame(_queue *frame_queue);

uint32 wlan_equeue_frame_num(_queue *frame_queue);

extern int32 wlan_free_xmitbuf(struct xmit_struct *pxmit_priv, struct xmit_buf *pxmitbuf);
extern struct xmit_buf *wlan_alloc_xmitbuf(struct xmit_struct *pxmit_priv);

extern void wlan_update_protection(ADAPTER *Adapter, uint8 *ie, uint32 ie_len);

extern int32 wlan_make_wlanhdr(ADAPTER *Adapter, uint8 *hdr, struct pkt_attrib *pattrib);
extern int32 wlan_put_snap(uint8 *data, uint16 h_proto);

extern struct xmit_frame *wlan_alloc_xmitframe(struct xmit_struct *pxmit_priv);
extern int32 wlan_free_xmitframe(struct xmit_struct *pxmit_priv, struct xmit_frame *pxmitframe);
extern void wlan_free_xmitframe_queue(struct xmit_struct *pxmit_priv, _queue *pframequeue );
extern int32 xmitframe_enqueue(ADAPTER *Adapter, struct xmit_frame *pxmitframe);
extern struct xmit_frame* wlan_dequeue_xframe(struct xmit_struct *pxmit_priv, uint8 flags);

extern int32 wlan_xmit_classifier(ADAPTER *Adapter, struct xmit_frame *pxmitframe);

#ifdef TX_ZERO_BUF_COPY
int32 wlan_xmitframe_coalesce_zero_copy(ADAPTER *Adapter, _pkt *pkt, struct xmit_frame *pxmitframe);
#else
extern int32 wlan_xmitframe_coalesce(ADAPTER *Adapter, _pkt *pkt, struct xmit_frame *pxmitframe);
#endif

int32 _init_hw_txqueue(struct hw_txqueue* phw_txqueue, uint8 ac_tag);


int32 wlan_txframes_pending(ADAPTER *Adapter);
int32 wlan_txframes_sta_ac_pending(ADAPTER *Adapter, struct pkt_attrib *pattrib);
void wlan_init_hwxmits(struct hw_xmit *phwxmit, int32 entry);


int32 wlan_init_xmit_struct(struct xmit_struct *pxmit_priv, ADAPTER *Adapter);
void wlan_free_xmit_struct (struct xmit_struct *pxmit_priv);


void wlan_alloc_hwxmits(ADAPTER *Adapter);
void wlan_free_hwxmits(ADAPTER *Adapter);

int32 wlan_xmit(ADAPTER *Adapter, _pkt *pkt);
void _do_queue_select(ADAPTER   *Adapter, struct pkt_attrib *pattrib);
int32 wlan_xmit_frame(ADAPTER *Adapter, struct xmit_frame *pxmitframe);
void wlan_count_tx_stats(ADAPTER *Adapter, struct xmit_frame *pxmitframe, uint32 sz);


#ifdef XMIT_USE_STA_QUEUE
void wlan_xmit_frame_use_sta_queue(ADAPTER *Adapter);
#else
void wlan_xmit_frame_use_single_queue(ADAPTER *Adapter);
#endif

#endif

