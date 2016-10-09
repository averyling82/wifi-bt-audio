#ifndef _WLAN_RECEIVE_H_
#define _WLAN_RECEIVE_H_


//These definition is used for Rx packet reordering.
#define SN_LESS(a, b)       (((a-b)&0x800)!=0)
#define SN_EQUAL(a, b)  (a == b)
//#define REORDER_WIN_SIZE  128
//#define REORDER_ENTRY_NUM 128
#define REORDER_WAIT_TIME   (30) // (ms)

#define RECVBUFF_ALIGN_SZ 512
#define RSVD_ROOM_SZ (0)
#define NR_RECVFRAME    4       //256


#define RXFRAME_ALIGN   8
#define RXFRAME_ALIGN_SZ    (1<<RXFRAME_ALIGN)

#define MAX_RXFRAME_CNT 512
#define MAX_RX_NUMBLKS      (32)
#define RECVFRAME_HDR_ALIGN 128



#define SNAP_SIZE sizeof(struct ieee80211_snap_hdr)

#define MAX_SUBFRAME_COUNT  64

//for Rx reordering buffer control
struct recv_reorder_ctrl
{
    ADAPTER *padapter;
    uint8 enable;
    uint16 indicate_seq;//=wstart_b, init_value=0xffff
    uint16 wend_b;
    uint8 wsize_b;
    _queue pending_recvframe_queue;
    _timer reordering_ctrl_timer;
};

struct  stainfo_rxcache
{
    uint16  tid_rxseq[16];
};

#define     PHY_RSSI_SLID_WIN_MAX               100
#define     PHY_LINKQUALITY_SLID_WIN_MAX        20


struct smooth_rssi_data
{
    uint32  elements[100];  //array to store values
    uint32  index;          //index to current array to store
    uint32  TotalNum;       //num of valid elements
    uint32  TotalVal;       //sum of valid elements
};

struct rx_stats
{
    uint32  IperfRx;
    uint32  NumRxOkInPeriod;
    uint64  NumRxOkByteInPeriod;
    uint64  rx_pkts;
    uint64  rx_drop;

    //For display the phy informatiom
    int32 SignalStrength; //dbm
    uint8 SignalQuality;//0-100
    uint8 noise;
    uint8 RxRSSIPercentage[4];
    int32 RxSNRdB[4];

    struct smooth_rssi_data ui_link_quality;
    struct smooth_rssi_data ui_rssi;
};


struct wlan_transfer_buffer
{
    _list list;
    uint8 *pallocated_transfer_buf;
    uint8 *transfer_buf;    // alignmented buffer
    uint32 buffer_len;
    uint32 transfer_len;
};

struct rx_pkt_attrib
{
    uint16 drvinfo_sz;
    uint32 pkt_len;

    uint32 MacIDValidEntry[2];  // 64 bits present 64 entry.
    uint8 packetReportType;
    uint8 physt;
    uint8 amsdu;
    uint8 qos;
    uint8 to_fr_ds;
    uint8 frag_num;
    uint16 seq_num;
    uint8 pw_save;
    uint8 mfrag;
    uint8 mdata;
    uint8 privacy; //in frame_ctrl field
    uint8 bdecrypted;
    uint32 hdrlen;      //the WLAN Header Len
    int encrypt;        //when 0 indicate no encrypt. when non-zero, indicate the encrypt algorith
    uint32 iv_len;
    uint32 icv_len;
    int priority;
    int ack_policy;
    uint8   crc_err;
    uint8   icverr;
    uint8   bHwError;
    uint8   dst[ETH_ALEN];
    uint8   src[ETH_ALEN];
    uint8   ta[ETH_ALEN];
    uint8   ra[ETH_ALEN];
    uint8   bssid[ETH_ALEN];
    uint8   eapol;

    uint8   mcs_rate;
    uint8   rxht;
    uint8   htc;

    uint8 SignalQuality;
    uint8 signal_strength;
    uint8 RxRSSIPercentage[4];
    uint8 RxMIMOSignalStrength[4];  // in 0~100 index
    int8 RxMIMOSignalQuality[2];
    boolean  bPacketMatchBSSID;
    boolean  bPacketToSelf;
    boolean  bPacketBeacon; //cosa add for rssi
    uint8 isCCKrate;

    int32   RxPWDBAll;
    int32   RecvSignalPower;
    int     RxSNRdB[2];
    uint8   wifi_addr[3*ETH_ALEN];
    uint16  ether_type;
    struct sta_info *psta;
};


struct recv_struct
{

    _lock lock;
    ADAPTER *adapter;

    _queue  free_recv_queue;
    _queue  free_recv_buf_queue;
    _queue  rx_skb_queue;

#ifdef CONFIG_RX_QUEUE
    _queue  recvframe_queue;
#endif

    uint32  free_recvframe_cnt;
    uint32  free_recv_buf_queue_cnt;

    uint8   *pallocated_frame_buf;
    uint8   *precv_frame_buf;

    uint32      rx_icv_err;
    uint32      rx_largepacket_crcerr;
    uint32      rx_smallpacket_crcerr;
    uint32      rx_middlepacket_crcerr;

#if 1
    uint8   rx_pending_cnt;
    uint32  ff_hwaddr;
    uint8   *pallocated_recv_buf;
    uint8   *precv_buf;    // 4 alignment

#endif

};


struct sta_recv_struct
{
    _queue defrag_q;     //keeping the fragment frame until defrag

    struct  stainfo_rxcache rxcache;
};

struct recv_buf
{
#if 1
    _list list;
    _lock recvbuf_lock;
    uint32  ref_cnt;
    ADAPTER  *adapter;
    //urb *purb;
#endif

    _pkt *skb;
    uint8 reuse;

    struct wlan_transfer_buffer *rx_transfer_buf;

    uint32  len;
    uint8 *phead;
    uint8 *pdata;
    uint8 *ptail;
    uint8 *pend;

    uint8 *pbuf;
    uint8 *pallocated_buf;


};


/*
    head  ----->

        data  ----->

            payload

        tail  ----->


    end   ----->

    len = (uint32 )(tail - data);

*/
struct recv_frame_hdr
{

    _list   list;
    _pkt    *pkt;

    /* not_used */
    _pkt *pkt_newalloc;

    ADAPTER  *adapter;

    uint8 fragcnt;

    int32 frame_tag;

    struct rx_pkt_attrib attrib;

    uint32  len;
    uint8 *rx_head;
    uint8 *rx_data;
    uint8 *rx_tail;
    uint8 *rx_end;

    void *precvbuf;


    //
    struct sta_info *psta;

    //for A-MPDU Rx reordering buffer control
    struct recv_reorder_ctrl *preorder_ctrl;

};

struct recv_frame
{
    //uint32 mem[2];
    union
    {
        _list list;
        struct recv_frame_hdr hdr;
        uint32 mem[RECVFRAME_HDR_ALIGN>>2];
    } u;
};

extern struct recv_frame *wlan_alloc_recvframe (_queue *pfree_recv_queue);  //get a free recv_frame from pfree_recv_queue
extern void wlan_init_recvframe(struct recv_frame *precvframe ,struct recv_struct *precv_priv);
extern int32 wlan_free_recvframe(struct recv_frame *precvframe, _queue *pfree_recv_queue);
extern int32 wlan_enqueue_recvframe(struct recv_frame *precvframe, _queue *queue);
extern void wlan_free_recvframe_queue(_queue *pframequeue,  _queue *pfree_recv_queue);


struct wlan_transfer_buffer *wlan_alloc_transfer_buffer(uint32 sz);
void wlan_free_transfer_buffer(struct wlan_transfer_buffer *transfer_buffer);


__inline static uint8 *get_rxmem(struct recv_frame *precvframe)
{
    //always return rx_head...
    if(precvframe==NULL)
        return NULL;

    return precvframe->u.hdr.rx_head;
}

__inline static uint8 *get_rx_status(struct recv_frame *precvframe)
{

    return get_rxmem(precvframe);

}

__inline static uint8 *get_recvframe_data(struct recv_frame *precvframe)
{

    //alwasy return rx_data
    if(precvframe==NULL)
        return NULL;

    return precvframe->u.hdr.rx_data;

}

__inline static uint8 *recvframe_push(struct recv_frame *precvframe, int32 sz)
{
    // append data before rx_data

    /* add data to the start of recv_frame
    *
    *      This function extends the used data area of the recv_frame at the buffer
    *      start. rx_data must be still larger than rx_head, after pushing.
    */

    if(precvframe==NULL)
        return NULL;


    precvframe->u.hdr.rx_data -= sz ;
    if( precvframe->u.hdr.rx_data < precvframe->u.hdr.rx_head )
    {
        precvframe->u.hdr.rx_data += sz ;
        return NULL;
    }

    precvframe->u.hdr.len +=sz;

    return precvframe->u.hdr.rx_data;

}


__inline static uint8 *recvframe_pull(struct recv_frame *precvframe, uint32 sz)
{
    // rx_data += sz; move rx_data sz bytes  hereafter

    //used for extract sz bytes from rx_data, update rx_data and return the updated rx_data to the caller


    if(precvframe==NULL)
        return NULL;


    precvframe->u.hdr.rx_data += sz;

    if(precvframe->u.hdr.rx_data > precvframe->u.hdr.rx_tail)
    {
        precvframe->u.hdr.rx_data -= sz;
        return NULL;
    }

    precvframe->u.hdr.len -=sz;

    return precvframe->u.hdr.rx_data;

}

__inline static uint8 *recvframe_put(struct recv_frame *precvframe, int32 sz)
{
    // rx_tai += sz; move rx_tail sz bytes  hereafter

    //used for append sz bytes from ptr to rx_tail, update rx_tail and return the updated rx_tail to the caller
    //after putting, rx_tail must be still larger than rx_end.

    if(precvframe==NULL)
        return NULL;


    precvframe->u.hdr.rx_tail += sz;

    if(precvframe->u.hdr.rx_tail > precvframe->u.hdr.rx_end)
    {
        precvframe->u.hdr.rx_tail -= sz;
        return NULL;
    }

    precvframe->u.hdr.len +=sz;

    return precvframe->u.hdr.rx_tail;

}



__inline static uint8 *recvframe_pull_tail(struct recv_frame *precvframe, int32 sz)
{
    // rmv data from rx_tail (by yitsen)

    //used for extract sz bytes from rx_end, update rx_end and return the updated rx_end to the caller
    //after pulling, rx_end must be still larger than rx_data.

    if(precvframe==NULL)
        return NULL;

    precvframe->u.hdr.rx_tail -= sz;

    if(precvframe->u.hdr.rx_tail < precvframe->u.hdr.rx_data)
    {
        precvframe->u.hdr.rx_tail += sz;
        return NULL;
    }

    precvframe->u.hdr.len -=sz;

    return precvframe->u.hdr.rx_tail;

}


#if 0
__inline static struct recv_frame *rxmem_to_recvframe(uint8 *rxmem)
{
    //due to the design of 2048 bytes alignment of recv_frame, we can reference the struct recv_frame
    //from any given member of recv_frame.
    // rxmem indicates the any member/address in recv_frame

    return (struct recv_frame*)(((uint32)rxmem>>RXFRAME_ALIGN) <<RXFRAME_ALIGN) ;

}

__inline static struct recv_frame *pkt_to_recvframe(_pkt *pkt)
{

    uint8 * buf_star;
    struct recv_frame * precv_frame;
    precv_frame = rxmem_to_recvframe((uint8*)buf_star);

    return precv_frame;
}

__inline static uint8 *pkt_to_recvmem(_pkt *pkt)
{
    // return the rx_head

    struct recv_frame * precv_frame = pkt_to_recvframe(pkt);

    return  precv_frame->u.hdr.rx_head;

}

__inline static uint8 *pkt_to_recvdata(_pkt *pkt)
{
    // return the rx_data

    struct recv_frame * precv_frame =pkt_to_recvframe(pkt);

    return  precv_frame->u.hdr.rx_data;

}
#endif

__inline static int32 get_recvframe_len(struct recv_frame *precvframe)
{
    return precvframe->u.hdr.len;
}

__inline static uint8 query_rx_pwr_percentage(int32 antpower )
{
    if ((antpower <= -100) || (antpower >= 20))
    {
        return  0;
    }
    else if (antpower >= 0)
    {
        return  100;
    }
    else
    {
        return  (100+antpower);
    }
}
__inline static int32 translate_percentage_to_dbm(uint32 SignalStrengthIndex)
{
    int32   SignalPower; // in dBm.

    // Translate to dBm (x=0.5y-95).
    SignalPower = (int32)((SignalStrengthIndex + 1) >> 1);
    SignalPower -= 95;

    return SignalPower;
}


struct sta_info;

void count_rx_stats(ADAPTER *Adapter, uint32 sz, struct sta_info *psta);
extern void wlan_init_sta_recv_struct(struct sta_recv_struct *psta_recv_priv);

extern int32 wlan_init_recv_struct(struct recv_struct *precv_priv, ADAPTER *Adapter);
extern void wlan_free_recv_struct (struct recv_struct *precv_priv);
extern int32  wlan_recv_entry(struct recv_frame *precv_frame);
extern void wlan_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl);

#endif

