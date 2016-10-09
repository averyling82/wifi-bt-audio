#include "bt_config.h"

#ifndef __LWBT_BT_PBUF_H__
#define __LWBT_BT_PBUF_H__

#ifdef _A2DP_SINK_
#define PHY_FRAME_MAX_SIZE (1064+64) //20 + 20 + 14 + 4 + 20 + 8+8
//#define PHY_FRAME_MAX_SIZE (672+64) //20 + 20 + 14 + 4 + 20 + 8+8

#else
#define PHY_FRAME_MAX_SIZE (672+64) //20 + 20 + 14 + 4 + 20 + 8+8
#endif

#define PBUF_POOL_BUFSIZE  PHY_FRAME_MAX_SIZE

typedef enum
{
    PBUF_TRANSPORT,
    PBUF_IP,
    PBUF_ETHNET,
    PBUF_PHY,
    PBUF_RAW

} pbuf_layer;

typedef enum
{
    PBUF_POOL, /* pbuf payload refers to RAM */
    PBUF_RAM
} pbuf_type;

typedef __packed  struct pbuf
{
    /** next pbuf in singly linked pbuf chain */
    struct pbuf *next;

    /** pointer to the actual data in the buffer */
    void *payload;

    /** length of this buffer */

    uint16 tcplen;
    uint16 len;
    uint16 tot_len;
    uint16 buflen;
    /** byte stream entry in this block**/
    uint16 streamStart;

    /** byte stream size in this block**/
    uint16 streamlen;

    /** misc flags */

    uint8 ref;
    uint8 flags;
    uint8 pkt_type;
 // TCP_HDR *tcphdr;

    //uint8 buf[PHY_FRAME_MAX_SIZE];
    uint8 *buf;
} PBUF;
extern PBUF *bt_pbuf_alloc(pbuf_layer l, uint16 size, pbuf_type type);
extern uint8 bt_pbuf_free(PBUF *p);

#define  pbuf_alloc(l, s, t) bt_pbuf_alloc(l, s, t)
#define  pbuf_free(p) bt_pbuf_free(p)
#endif
