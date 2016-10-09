/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
//#include "bt_config.h"
//#include "BlueToothWin.h"
//#include "BlueToothControl.h"
#ifndef __LWBT_PBUF_H__
#define __LWBT_PBUF_H__

#include "rk_bt_pbuf.h"

#define PBUF_TRANSPORT_HLEN 20
#define PBUF_IP_HLEN        20
#define PBUF_PHY_HLEN       20
#define PBUF_ETHNET_HLEN    14

#if 0
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
#endif

/** indicates this packet's data should be immediately passed to the application */
#define PBUF_FLAG_PUSH 0x01U

#if 0

/* add __packed, jan@2009-11-24 */
extern void *FrameBuf();
extern void  pbuf_init(void);
extern PBUF *pbuf_alloc(pbuf_layer l, uint16 size, pbuf_type type);
extern uint8 pbuf_free(PBUF *p);
extern void  pbuf_cat(PBUF *h, PBUF *t);
extern void pbuf_chain(struct pbuf *h, struct pbuf *t);
extern struct pbuf * pbuf_dechain(struct pbuf *p);
extern void pbuf_ref(struct pbuf *h);

#endif
extern void  bt_pbuf_init(void);
extern void bt_pbuf_deinit(void);

extern void  bt_pbuf_cat(PBUF *h, PBUF *t);
extern void  bt_pbuf_chain(struct pbuf *h, struct pbuf *t);
extern struct pbuf * bt_pbuf_dechain(struct pbuf *p);
extern void bt_pbuf_ref(struct pbuf *h);
extern uint8 bt_pbuf_header(PBUF *p, int16 header_size_increment);
extern void bt_pbuf_copy(struct pbuf *p_to, struct pbuf *p_from);
extern void bt_pbuf_realloc(struct pbuf *p, uint16 new_len);
extern int bt_pbuf_queue_len(struct pbuf **head);
extern int bt_pbuf_queue_empty(struct pbuf **head);
extern int bt_pbuf_queue_head(struct pbuf **head,struct pbuf *p);
extern int bt_pbuf_queue_tail(struct pbuf **head,struct pbuf *p);
extern PBUF* bt_pbuf_dequeue_head(struct pbuf **head);
extern PBUF* bt_pbuf_dequeue_tail(struct pbuf **head);

#define  pbuf_init() bt_pbuf_init()
#define  pbuf_deinit() bt_pbuf_deinit()

#define  pbuf_cat(h, t) bt_pbuf_cat(h,t)
#define  pbuf_chain(h, t) bt_pbuf_chain(h,t)
#define  pbuf_dechain(p) bt_pbuf_dechain(p)
#define  pbuf_ref(h) bt_pbuf_ref(h)
#define  pbuf_header(p,h) bt_pbuf_header(p,h)
#define  pbuf_copy(p,q) bt_pbuf_copy(p,q)
#define  pbuf_realloc(p,l) bt_pbuf_realloc(p,l)

#define  pbuf_queue_len(h) bt_pbuf_queue_len(h)
#define  pbuf_queue_empty(h) bt_pbuf_queue_empty(h)
#define  pbuf_queue_head(h,p) bt_pbuf_queue_head(h,p)
#define  pbuf_queue_tail(h,p) bt_pbuf_queue_tail(h,p)
#define  pbuf_dequeue_head(h) bt_pbuf_dequeue_head(h)
#define  pbuf_dequeue_tail(h) bt_pbuf_dequeue_tail(h)
#define  pbuf_dequeue(h) bt_pbuf_dequeue_head(h)
#define  pbuf_queue_purge(p) bt_pbuf_free(p)

#endif /* __LWIP_PBUF_H__ */
