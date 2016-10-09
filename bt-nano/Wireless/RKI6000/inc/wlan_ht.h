#ifndef _WLAN_HT_H_
#define _WLAN_HT_H_

#include "wlan_80211.h"

struct ht_cfg_struct
{
    uint32  ht_option;
    uint32  ampdu_enable;//for enable Tx A-MPDU
    //uint8 baddbareq_issued[16];
    uint32  tx_amsdu_enable;//for enable Tx A-MSDU
    uint32  tx_amdsu_maxlen; // 1: 8k, 0:4k ; default:8k, for tx
    uint32  rx_ampdu_maxlen; //for rx reordering ctrl win_sz, updated when join_callback.

    uint8   bwmode;//
    uint8 ch_offset;//PRIME_CHNL_OFFSET
    uint8 sgi;//short GI

    //for processing Tx A-MPDU
    uint8       agg_enable_bitmap;
    //uint8     ADDBA_retry_count;
    uint8       candidate_tid_bitmap;
    uint8       MIMO_ps;
    struct ieee80211_ht_cap ht_cap;
    uint32  ht_cap_len;
};


#endif

