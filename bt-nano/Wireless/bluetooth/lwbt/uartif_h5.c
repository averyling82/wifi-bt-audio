/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
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
 * This file is part of the lwBT Bluetooth stack.
 *
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */

/*-----------------------------------------------------------------------------------*/
/* uartif.c
 *
 * Implementation of the HCI UART transport layer for Linux
 */
/*-----------------------------------------------------------------------------------*/

#include "lwbt.h"

#include "bt_config.h"
#include "lwbtopts.h"
#include "phybusif.h"
#include "hci.h"
#include "lwbterr.h"
#include "uart.h"
#include "bt_pbuf.h"
#include "delay.h"
#ifdef _BLUETOOTH_
#if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#ifdef _A2DP_SINK_
_ATTR_LWBT_DATA_ uint8 scoTxBuf[520];
#endif

//struct phybusif_cb *g_cbP;
_ATTR_LWBT_BSS_ struct phybusif_cb g_cb;

void HciServeIsrRequest(void);
void  phybusif_data_recv_task(void *arg);
void  phybusif_data_process_task(void *arg);
void  phybusif_h5_data_send_task(void *arg);
static int h5_recv(struct phybusif_cb *hu, void *data, int count);
static PBUF *h5_dequeue(struct phybusif_cb *hu);
static void h5_complete_rx_pkt(struct phybusif_cb *hu,PBUF *skb);
/*-----------------------------------------------------------------------------------*/
/* Initializes the physical bus interface
 */
/*-----------------------------------------------------------------------------------*/
struct h5_struct
{
//    PBUF unack;  /* Unack'ed packets queue */
//    PBUF rel;    /* Reliable packets queue */
//    PBUF unrel;  /* Unreliable packets queue */

    PBUF *unack;
    PBUF *rel;
    PBUF *unrel;
    unsigned long rx_count;
    PBUF *rx_skb;
    uint8  rxseq_txack;        /* rxseq == txack. */
    uint8  rxack;          /* Last packet sent by us that the peer ack'ed */
    //struct  timer_list th5;
    pTimer  th5;
    pSemaphore unack_lock;
    uint8 is_checking;

    enum
    {
        H5_W4_PKT_DELIMITER,
        H5_W4_PKT_START,
        H5_W4_HDR,
        H5_W4_DATA,
        H5_W4_CRC
    } rx_state;

    enum
    {
        H5_ESCSTATE_NOESC,
        H5_ESCSTATE_ESC
    } rx_esc_state;

    uint8  use_crc;
    uint16 message_crc;
    uint8  txack_req;      /* Do we need to send ack's to the peer? */

    /* Reliable packet sequence number - used to assign seq to each rel pkt. */
    uint8  msgq_txseq;
    uint16 buf_used_cnt;

    uint8 sync_resp_recved;
    uint8 conf_resp_recved;
};


_ATTR_LWBT_INIT_CODE_
err_t phybusif_init(bt_hw_control_t * hw_control)
{
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
    pbuf_init();
    g_cb.hw_conrtol = hw_control;
    g_cb.hci_data_proc_sem = rkos_semaphore_create(10, 0);
    if(g_cb.hci_data_proc_sem == NULL)
    {
        return ERR_MEM;
    }
    //g_cb.h5_data_send_sem = rkos_semaphore_create(5, 0);
    g_cb.h5_data_send_sem = rkos_semaphore_create(30, 0);
    if(g_cb.h5_data_send_sem == NULL)
    {
        rkos_semaphore_delete(g_cb.hci_data_proc_sem);
        pbuf_deinit();
        return ERR_MEM;
    }
    if(h5_open(&g_cb))
    {
        rkos_semaphore_delete(g_cb.hci_data_proc_sem);
        rkos_semaphore_delete(g_cb.h5_data_send_sem);
        pbuf_deinit();
        return ERR_TIMEOUT;
    }

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
err_t phybusif_deinit(bt_hw_control_t * hw_control)
{
    pbuf_deinit();
    h5_close(&g_cb);
    rkos_semaphore_delete(g_cb.hci_data_proc_sem);
    rkos_semaphore_delete(g_cb.h5_data_send_sem);
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
}

_ATTR_LWBT_UARTIF_CODE_
void phybusif_waitsend_pbuf_input(struct pbuf *p)
{
    struct pbuf *q;

    if(g_cb.waitsend== NULL)
    {
        g_cb.waitsend = p;
    }
    else
    {
        q = g_cb.waitsend;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
    }
}


/*-----------------------------------------------------------------------------------*/
/* HciServeIsrRequest():
 *
 * Called by main loop
 *
 */
/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
void HciServeIsrRequest(void)
{
    rkos_semaphore_give(g_cb.hci_data_proc_sem);
}

_ATTR_LWBT_UARTIF_CODE_
void HciServeIsrDisable(void)
{

}

_ATTR_LWBT_UARTIF_CODE_
void HciServeIsrEnable(void)
{

}

_ATTR_LWBT_UARTIF_CODE_
void _phybusif_output(struct pbuf *p, uint16 len, void (*func)(void))
{
    struct pbuf *q;
    unsigned char *ptr;
    /* Send pbuf on UART */
    if(g_cb.hw_conrtol!= NULL && g_cb.hw_conrtol->get_power_status)
    {
        if(g_cb.hw_conrtol->get_power_status() == POWER_STATUS_SLEEP)
        {

            if(g_cb.hw_conrtol->wake)
            {
                pbuf_ref(p);
                phybusif_waitsend_pbuf_input(p);
                g_cb.hw_conrtol->wake(NULL);
            }

            return;
        }
    }
#if 1

    q = p;
    {
        pbuf_ref(q);
        q->pkt_type = ((char *)q->payload)[0];
        pbuf_header(q, -1); // remove h4 header
        h5_enqueue(&g_cb, q);

    }
#else
    ptr = p->payload;
    g_cb.hw_conrtol->write(ptr,p->len);
#endif

}

_ATTR_LWBT_UARTIF_CODE_
void phybusif_send_waitlist()
{
    struct pbuf *q;
    struct pbuf *p;
    unsigned char *ptr;
    if(g_cb.waitsend == NULL)
        return;

    q = g_cb.waitsend;

    while(q != NULL)
    {
        ptr = q->payload;
        g_cb.hw_conrtol->write(ptr,q->len);
        p= q;
        q = q->next;
        p->next = NULL;
        pbuf_free(p);

    }

    g_cb.waitsend = NULL;
}


#ifdef _A2DP_SINK_
_ATTR_LWBT_UARTIF_CODE_
void phybusif_sco_output(uint8 *scohdr, uint8 *data, uint16 datalen, void (*func)(void))
{

//    memcpy(scoTxBuf, scohdr, 4);
//    memcpy(scoTxBuf+4, data, datalen);
//    g_cb.hw_conrtol->write(scoTxBuf,datalen+4);
    PBUF *q;
    q = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
    if (NULL == q)
    {
        rk_printf("\n SCO data can't malloc pbuf \n");
        return;
    }
    memcpy(q->payload, scohdr, 4);
    memcpy((char *)q->payload + 4, data, datalen);
    q->len = q->tot_len = datalen + 4;
    q->pkt_type = ((char *)q->payload)[0];
    pbuf_header(q, -1); // remove h4 header
    h5_enqueue(&g_cb, q);
}

#endif

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_data_recv_task(void *arg)
{
    unsigned char c[32];
    struct phybusif_cb *cb;
    cb = &g_cb;
    int needlen = 0;
    int realsize = 0;
    while(1)
    {
        realsize = cb->hw_conrtol->read(c, 16);
        //printf("c0 = %x\n",c[0]);
        //debug_hex(c, 1, 16);
        if(realsize)
        {
            //printf("\n realsize = %d\n",realsize);
            //debug_hex(c, realsize, 16);
        }
        h5_recv(cb, c, realsize);
    }
}

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_data_process_task(void *arg)
{
    BT_DEBUG("phybusif_data_process_task");
    struct h5_struct *h5 = g_cb.priv;
    PBUF *skb;
    while(1)
    {
        rkos_semaphore_take(g_cb.hci_data_proc_sem, MAX_DELAY);
        //BT_DEBUG("phybusif_data_process_task");
        //hci_serve();
        rkos_semaphore_take(h5->unack_lock, MAX_DELAY);
        skb = pbuf_dequeue((PBUF**)&g_cb.h5_unread);
        if(skb)
        {
            //BT_DEBUG("data_process get a packet");
            h5_complete_rx_pkt(&g_cb,skb);
        }
        else
        {
            BT_DEBUG("data_process skb = NULL");
        }
        rkos_semaphore_give(h5->unack_lock);
    }
}

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_h5_data_send_task(void *arg)
{
    BT_DEBUG("phybusif_h5_data_send_task");
    PBUF *skb;
    while(1)
    {
        rkos_semaphore_take(g_cb.h5_data_send_sem, MAX_DELAY);
        //BT_DEBUG("phybusif_h5_data_send_task\n");
        //while((skb = h5_dequeue(&g_cb)) != NULL)
        if((skb = h5_dequeue(&g_cb)) != NULL)
        {
            //BT_DEBUG("phybusif write skb->len = %d\n",skb->buflen);
//            printf("out >>:");
//            if(skb->buflen > 16)
//            {
//                //debug_hex(skb->payload, skb->buflen, 16);
//                debug_hex(skb->payload, 16, 16);
//            }
//            else
//            {
//                debug_hex(skb->payload, skb->buflen, 16);
//            }
            g_cb.hw_conrtol->write(skb->payload,skb->buflen);
            pbuf_free(skb) ;
        }
        //rkos_delay(10);
    }
}


#define VERSION "1.0"

_ATTR_LWBT_DATA_ static int txcrc = 0;
#ifdef CONFIG_RTL8723BS_MODULE
_ATTR_LWBT_DATA_ static int need_check_bt_state = 0;
#else
_ATTR_LWBT_DATA_ static int need_check_bt_state = 1;
#endif

//static int hciextn = 1;
#define H5_BTSTATE_CHECK_CONFIG_PATH      "/system/etc/firmware/rtl8723as/need_check_bt_state"

#define H5_TXWINSIZE    4
#define HCI_COMMAND_PKT 0x01
#define HCI_ACLDATA_PKT 0x02
#define HCI_SCODATA_PKT 0x03
#define HCI_EVENT_PKT   0x04
#define H5_ACK_PKT  0x00
#define H5_LE_PKT       0x0F
#define H5_VDRSPEC_PKT  0x0E

//sleep check begin
struct h5_sleep_info
{
    int  wake_sleep; //1: can sleep, value 0 need wake up
    void *uport;
    pSemaphore wake_lock;
};

/*
 *  Global variable
 */
#define TX_TIMER_INTERVAL      2

#define STACK_TXDATA       0x01
#define STACK_SLEEP        0x02
/** Global state flags */
_ATTR_LWBT_BSS_ static unsigned long flags;

_ATTR_LWBT_BSS_ static struct h5_sleep_info *hsi;
//static struct hci_dev *h5_hci_hdev;
/** Lock for state transitions */
_ATTR_LWBT_BSS_ pSemaphore rw_lock;

//static void h5_sleep_tx_timer_expire(unsigned long data);
//static DEFINE_TIMER(h5_tx_timer, h5_sleep_tx_timer_expire, 0, 0);

_ATTR_LWBT_BSS_ pTimer h5_tx_timer;

static int h5_parse_hci_event(struct notifier_block *this, unsigned long event, void *data);
void hci_uart_tx_wakeup(struct phybusif_cb *hu);
/** Notifier block for HCI events */
//struct notifier_block hci_event_nblock =
//{
//    .notifier_call = h5_parse_hci_event,
//};


static void h5_sleep_work(struct work_struct *work);
//DECLARE_DELAYED_WORK(sleep_workqueue, h5_sleep_work);
//sleep check end



_ATTR_LWBT_BSS_ struct phybusif_cb* hci_uart_info;
static void h5_bt_state_err_worker(struct work_struct *private_);
static void h5_bt_state_check_worker(struct work_struct *private_);
//static DECLARE_DELAYED_WORK(bt_state_err_work, h5_bt_state_err_worker);
//static DECLARE_DELAYED_WORK(bt_state_check_work, h5_bt_state_check_worker);

//static struct mutex sem_exit;

_ATTR_LWBT_BSS_ pSemaphore sem_exit;

/* ---- H5 CRC calculation ---- */

/* Table for calculating CRC for polynomial 0x1021, LSB processed first,
initial value 0xffff, bits shifted in reverse order. */
_ATTR_LWBT_UARTIF_DATA_
static uint16 crc_table[] =
{
    0x0000, 0x1081, 0x2102, 0x3183,
    0x4204, 0x5285, 0x6306, 0x7387,
    0x8408, 0x9489, 0xa50a, 0xb58b,
    0xc60c, 0xd68d, 0xe70e, 0xf78f
};

/* Initialise the crc calculator */
#define H5_CRC_INIT(x) x = 0xffff
_ATTR_LWBT_UARTIF_DATA_
uint8 byte_rev_table[256] =
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};
_ATTR_LWBT_UARTIF_CODE_
uint16 bitrev16(uint16 x)
{
    return (byte_rev_table[x & 0xff] << 8) | byte_rev_table[x >> 8];
}

/*
   Update crc with next data byte

   Implementation note
        The data byte is treated as two nibbles.  The crc is generated
        in reverse, i.e., bits are fed into the register from the top.
*/
_ATTR_LWBT_UARTIF_CODE_
static void h5_crc_update(uint16 *crc, uint8 d)
{
    uint16 reg = *crc;

    reg = (reg >> 4) ^ crc_table[(reg ^ d) & 0x000f];
    reg = (reg >> 4) ^ crc_table[(reg ^ (d >> 4)) & 0x000f];

    *crc = reg;
}

_ATTR_LWBT_UARTIF_CODE_
char *skb_put(PBUF * skb, int len)
{
    skb->buflen += len;
    return (char*)skb->payload+(skb->buflen-len);
}
_ATTR_LWBT_UARTIF_CODE_
void skb_trim(PBUF * skb, int len)
{
    skb->buflen = len;
}
_ATTR_LWBT_UARTIF_CODE_
void skb_to_pbuf(PBUF * skb)
{
    skb->len = skb->buflen;
    skb->tot_len = skb->buflen;
}

_ATTR_LWBT_UARTIF_CODE_
void pbuf_to_skb(PBUF * skb)
{
    skb->buflen = skb->len;
}
/* ---- H5 core ---- */
_ATTR_LWBT_UARTIF_CODE_
static void h5_slip_msgdelim(PBUF *skb)
{
    const char pkt_delim = 0xc0;

    memcpy(skb_put(skb, 1), &pkt_delim, 1);
}

//static void h5_slip_one_byte(struct sk_buff *skb, uint8 c)
_ATTR_LWBT_UARTIF_CODE_
static void h5_slip_one_byte(PBUF *skb, uint8 c)
{
    const char esc_c0[2] = { 0xdb, 0xdc };
    const char esc_db[2] = { 0xdb, 0xdd };
    const char esc_11[2] = { 0xdb, 0xde };
    const char esc_13[2] = { 0xdb, 0xdf };

    switch (c)
    {
        case 0xc0:
            memcpy(skb_put(skb, 2), &esc_c0, 2);
            break;
        case 0xdb:
            memcpy(skb_put(skb, 2), &esc_db, 2);
            break;
        case 0x11:
            memcpy(skb_put(skb, 2), &esc_11, 2);
            break;
        case 0x13:
            memcpy(skb_put(skb, 2), &esc_13, 2);
            break;
        default:
            memcpy(skb_put(skb, 1), &c, 1);
            break;
    }
}
_ATTR_LWBT_UARTIF_CODE_
static int h5_enqueue(struct phybusif_cb *hu, PBUF *skb)
{
    struct h5_struct *h5 = hu->priv;

//    if (skb->buflen > 0xFFF)   //Pkt length must be less than 4095 bytes
//    {
//        BT_DEBUG("Packet too long");
//        pbuf_free(skb);
//        return 0;
//    }
    pbuf_to_skb(skb);
    switch (skb->pkt_type)
    {
        case HCI_ACLDATA_PKT:
        case HCI_COMMAND_PKT:
            pbuf_queue_tail(&h5->rel, skb);
            break;

        case HCI_SCODATA_PKT:
            pbuf_queue_tail(&h5->unrel, skb);
            break;
        case H5_LE_PKT:
        case H5_ACK_PKT:
        case H5_VDRSPEC_PKT:
            pbuf_queue_tail(&h5->unrel, skb);    /* 3-wire LinkEstablishment*/
            break;

        default:
            BT_DEBUG("Unknown packet type");
            pbuf_free(skb);
            break;
    }
    hci_uart_tx_wakeup(hu);
    return 0;
}
_ATTR_LWBT_UARTIF_CODE_
static PBUF *h5_prepare_pkt(struct h5_struct *h5, uint8 *data,
                            int len, int pkt_type)
{
    //struct sk_buff *nskb;
    PBUF *nskb;
    uint8 hdr[4], chan;
    uint16 H5_CRC_INIT(h5_txmsg_crc);
    int rel, i;
    //BT_DEBUG("h5_prepare_pkt\n");
    switch (pkt_type)
    {
        case HCI_ACLDATA_PKT:
            chan = 2;   /* 3-wire ACL channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_COMMAND_PKT:
            chan = 1;   /* 3-wire cmd channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_EVENT_PKT:
            chan = 4;   /* 3-wire cmd channel */
            rel = 1;    /* reliable channel */
            break;
        case HCI_SCODATA_PKT:
            chan = 3;   /* 3-wire SCO channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_LE_PKT:
            chan = 15;  /* 3-wire LinkEstablishment channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_ACK_PKT:
            chan = 0;   /* 3-wire ACK channel */
            rel = 0;    /* unreliable channel */
            break;
        case H5_VDRSPEC_PKT:
            chan = 14;  /* 3-wire Vendor Specific channel */
            rel = 0;    /* unreliable channel */
            break;
        default:
            BT_DEBUG("Unknown packet type");
            return NULL;
    }


    /* Max len of packet: (original len +4(h5 hdr) +2(crc))*2
       (because bytes 0xc0 and 0xdb are escaped, worst case is
       when the packet is all made of 0xc0 and 0xdb :) )
       + 2 (0xc0 delimiters at start and end). */

    //nskb = alloc_skb((len + 6) * 2 + 2, GFP_ATOMIC);
    nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
    if (!nskb)
        return NULL;

    //bt_cb(nskb)->pkt_type = pkt_type;
    nskb->pkt_type = pkt_type;

    h5_slip_msgdelim(nskb);

    hdr[0] = h5->rxseq_txack << 3;
    h5->txack_req = 0;
    //BT_DEBUG("We request packet no %u to card", h5->rxseq_txack);

    if (rel)
    {
        hdr[0] |= 0x80 + h5->msgq_txseq;
        //BT_DEBUG("Sending packet with seqno %u", h5->msgq_txseq);
        h5->msgq_txseq = (h5->msgq_txseq + 1) & 0x07;
    }

    if (h5->use_crc)
        hdr[0] |= 0x40;

    hdr[1] = ((len << 4) & 0xff) | chan;
    hdr[2] = len >> 4;
    hdr[3] = ~(hdr[0] + hdr[1] + hdr[2]);

    /* Put H5 header */
    for (i = 0; i < 4; i++)
    {
        h5_slip_one_byte(nskb, hdr[i]);

        if (h5->use_crc)
            h5_crc_update(&h5_txmsg_crc, hdr[i]);
    }

    /* Put payload */
    for (i = 0; i < len; i++)
    {
        h5_slip_one_byte(nskb, data[i]);

        if (h5->use_crc)
            h5_crc_update(&h5_txmsg_crc, data[i]);
    }

    /* Put CRC */
    if (h5->use_crc)
    {
        h5_txmsg_crc = bitrev16(h5_txmsg_crc);
        h5_slip_one_byte(nskb, (uint8) ((h5_txmsg_crc >> 8) & 0x00ff));
        h5_slip_one_byte(nskb, (uint8) (h5_txmsg_crc & 0x00ff));
    }

    h5_slip_msgdelim(nskb);

    return nskb;
}

/* This is a rewrite of pkt_avail in AH5 */
_ATTR_LWBT_UARTIF_CODE_
static PBUF *h5_dequeue(struct phybusif_cb *hu)
{
    struct h5_struct *h5 = hu->priv;
    unsigned long flags;
    PBUF *skb;
    //BT_DEBUG("h5_dequeue\n");
    /* First of all, check for unreliable messages in the queue,
       since they have priority */

    if ((skb = pbuf_dequeue(&h5->unrel)) != NULL)
    {
        //BT_DEBUG("skb = %04x",(uint32)skb);
        PBUF *nskb = h5_prepare_pkt(h5, (uint8 *)skb->payload, (int)skb->buflen, (int)skb->pkt_type);
        if (nskb)
        {

            pbuf_free(skb);

            return nskb;
        }
        else
        {
            pbuf_queue_head(&h5->unrel, skb);
            BT_DEBUG("Could not dequeue pkt because alloc_skb failed");
        }
    }

    /* Now, try to send a reliable pkt. We can only send a
       reliable packet if the number of packets sent but not yet ack'ed
       is < than the winsize */

    //spin_lock_irqsave_nested(&h5->unack.lock, flags, SINGLE_DEPTH_NESTING);//wp
    rkos_semaphore_take(h5->unack_lock, MAX_DELAY);
    if (pbuf_queue_len(&h5->unack) < H5_TXWINSIZE && (skb = pbuf_dequeue(&h5->rel)) != NULL)
    {
        PBUF * nskb = h5_prepare_pkt(h5, (uint8 *)skb->payload, (int)skb->buflen, skb->pkt_type);
        if (nskb)
        {
            pbuf_queue_tail(&h5->unack, skb);
            rkos_mod_timer(h5->th5, configTICK_RATE_HZ/4, MAX_DELAY);
            //rkos_mod_timer(h5->th5, 1, MAX_DELAY);
            //spin_unlock_irqrestore(&h5->unack.lock, flags);
            rkos_semaphore_give(h5->unack_lock);

            return nskb;
        }
        else
        {
            pbuf_queue_head(&h5->rel, skb);
            BT_DEBUG("Could not dequeue pkt because alloc_skb failed");
        }
    }
    rkos_semaphore_give(h5->unack_lock);
    //spin_unlock_irqrestore(&h5->unack.lock, flags); //wp

    /* We could not send a reliable packet, either because there are
       none or because there are too many unack'ed pkts. Did we receive
       any packets we have not acknowledged yet ? */

    if (h5->txack_req)
    {
        /* if so, craft an empty ACK pkt and send it on H5 unreliable
           channel 0 */
        PBUF*nskb = h5_prepare_pkt(h5, NULL, 0, H5_ACK_PKT);
        return nskb;
    }
    /*
        if (need_check_bt_state) {
            schedule_delayed_work(&bt_state_check_work, HZ * 15);
        }
    */
    /* We have nothing to send */
    return NULL;
}
_ATTR_LWBT_UARTIF_CODE_
static int h5_flush(struct phybusif_cb * hu)
{
    BT_DEBUG("hu %p", hu);
    return 0;
}

/* Remove ack'ed packets */
_ATTR_LWBT_UARTIF_CODE_
static void h5_pkt_cull(struct h5_struct *h5)
{
    struct sk_buff *skb, *tmp;
    unsigned long flags;
    int i, pkts_to_be_removed;
    uint8 seqno;

    //spin_lock_irqsave(&h5->unack.lock, flags);//wp

    pkts_to_be_removed = pbuf_queue_len(&h5->unack);
    seqno = h5->msgq_txseq;

    while (pkts_to_be_removed)
    {
        if (h5->rxack == seqno)
            break;
        pkts_to_be_removed--;
        seqno = (seqno - 1) & 0x07;
    }

    if (h5->rxack != seqno)
        BT_DEBUG("Peer acked invalid packet,h5->msgq_txseq = %d,h5->rxack = %d, unack = %d\n",h5->msgq_txseq,h5->rxack,pbuf_queue_len(&h5->unack));

//    BT_DEBUG("Removing %u pkts out of %u, up to seqno %u",
//             pkts_to_be_removed, pbuf_queue_len(&h5->unack),
//             (seqno - 1) & 0x07);
    i = 0;
#if 0

    skb_queue_walk_safe(&h5->unack, skb, tmp)
    {
        if (i >= pkts_to_be_removed)
            break;
        i++;

        __skb_unlink(skb, &h5->unack);
        pbuf_free(skb);
    }
#endif

    int j;
    j = pkts_to_be_removed;
    while(j)
    {
        j--;
        PBUF * temp;
        temp = pbuf_dequeue_head(&h5->unack);
        if(temp)
        {
            pbuf_free(temp);
        }
        else
        {
            break;
        }
    }


    if (pbuf_queue_empty(&h5->unack))
    {
        rkos_stop_timer(h5->th5);
    }
    //spin_unlock_irqrestore(&h5->unack.lock, flags);//wp

    //if (j != pkts_to_be_removed)
    //    BT_DEBUG("Removed only %u out of %u pkts", i, pkts_to_be_removed);
}

/* Handle H5 link-establishment packets. When we
   detect a "sync" packet, symptom that the BT module has reset,
   we do nothing :) (yet) */
#define H5_RECV_BUF  ((char*)h5->rx_skb->payload)
#define H5_PROC_BUF  ((char*)skb->payload)
_ATTR_LWBT_UARTIF_CODE_
static void h5_handle_le_pkt(struct phybusif_cb *hu, PBUF *skb)
{
    struct h5_struct *h5 = hu->priv;
    uint8 conf_pkt[2]     = { 0x03, 0xfc};
    uint8 conf_rsp_pkt[3] = { 0x04, 0x7b, 0x00};
    uint8 sync_pkt[2]     = { 0x01, 0x7e};
    uint8 sync_rsp_pkt[2] = { 0x02, 0x7d};

    uint8 wakeup_pkt[2]   = { 0x05, 0xfa};
    uint8 woken_pkt[2]    = { 0x06, 0xf9};
    uint8 sleep_pkt[2]    = { 0x07, 0x78};
    uint8 h5_sync_resp_pkt[8] = {0xc0, 0x00, 0x2F, 0x00, 0xD0, 0x02, 0x7D, 0xc0};
    uint8 h5_conf_resp_pkt_to_Ctrl[8] = {0xc0, 0x00, 0x2F, 0x00, 0xD0, 0x04, 0x7B, 0xc0};
    //PBUF *skb = h5->rx_skb;
    /* spot "conf" pkts and reply with a "conf rsp" pkt */
    if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
        !memcmp(&H5_PROC_BUF[4], conf_pkt, 2))
    {
        /*
        PBUF *nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);

        BT_DEBUG("Found a LE conf pkt");
        if (!nskb)
            return;

        conf_rsp_pkt[2] |= txcrc << 0x4; //crc check enable, version no = 0. needed to be as avariable.
        memcpy(skb_put(nskb, 3), conf_rsp_pkt, 3);
        nskb->pkt_type = H5_LE_PKT;

        pbuf_queue_head(&h5->unrel, nskb);
        hci_uart_tx_wakeup(hu);//wp 2015/10/16
        */
        BT_DEBUG("Found a LE conf pkt");
        g_cb.hw_conrtol->write(h5_conf_resp_pkt_to_Ctrl, 8);
    }
    /* spot "conf resp" pkts*/
    else if (/*H5_RECV_BUF[1] >> 4 == 2 && */H5_PROC_BUF[2] == 0 &&
            !memcmp(&H5_PROC_BUF[4], conf_rsp_pkt, 2))
    {
        BT_DEBUG("Found a LE conf resp pkt, device go into active state");
        txcrc = (H5_PROC_BUF[6] >> 0x4) & 0x1;
        h5->conf_resp_recved = 1;
    }

    /* Spot "sync" pkts. If we find one...disaster! */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sync_pkt, 2))
    {
        BT_DEBUG("Found a LE sync pkt, card has reset");
        //DO Something here
        g_cb.hw_conrtol->write(h5_sync_resp_pkt, 8);
    }
    /* Spot "sync resp" pkts. If we find one...disaster! */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sync_rsp_pkt, 2))
    {
        BT_DEBUG("Found a LE sync resp pkt, device go into initialized state");
        //      DO Something here
        h5->sync_resp_recved = 1;
    }
    /* Spot "wakeup" pkts. reply woken message when in active mode */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], wakeup_pkt, 2))
    {
        PBUF *nskb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);

        BT_DEBUG("Found a LE Wakeup pkt, and reply woken message");
        //      DO Something here

        memcpy(skb_put(nskb, 2), woken_pkt, 2);
        nskb->pkt_type = H5_LE_PKT;

        pbuf_queue_head(&h5->unrel, nskb);
        hci_uart_tx_wakeup(hu);
    }
    /* Spot "woken" pkts. receive woken message from device */
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], woken_pkt, 2))
    {
        BT_DEBUG("Found a LE woken pkt from device");
        //      DO Something here
    }
    /* Spot "Sleep" pkts*/
    else if (H5_PROC_BUF[1] >> 4 == 2 && H5_PROC_BUF[2] == 0 &&
             !memcmp(&H5_PROC_BUF[4], sleep_pkt, 2))
    {
        BT_DEBUG("Found a LE Sleep pkt");
        //      DO Something here
    }

}
_ATTR_LWBT_UARTIF_CODE_
static inline void h5_unslip_one_byte(struct h5_struct *h5, unsigned char byte)
{
    const uint8 c0   = 0xc0, db   = 0xdb;
    const uint8 oof1 = 0x11, oof2 = 0x13;

    switch (h5->rx_esc_state)
    {
        case H5_ESCSTATE_NOESC:
            switch (byte)
            {
                case 0xdb:
                    h5->rx_esc_state = H5_ESCSTATE_ESC;
                    break;
                default:
                    memcpy(skb_put(h5->rx_skb, 1), &byte, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                        h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5->message_crc, byte);
                    h5->rx_count--;
            }
            break;

        case H5_ESCSTATE_ESC:
            switch (byte)
            {
                case 0xdc:
                    memcpy(skb_put(h5->rx_skb, 1), &c0, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                        h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, 0xc0);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xdd:
                    memcpy(skb_put(h5->rx_skb, 1), &db, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 &&
                        h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, 0xdb);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xde:
                    memcpy(skb_put(h5->rx_skb, 1), &oof1, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 && h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, oof1);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                case 0xdf:
                    memcpy(skb_put(h5->rx_skb, 1), &oof2, 1);
                    if ((H5_RECV_BUF[0] & 0x40) != 0 && h5->rx_state != H5_W4_CRC)
                        h5_crc_update(&h5-> message_crc, oof2);
                    h5->rx_esc_state = H5_ESCSTATE_NOESC;
                    h5->rx_count--;
                    break;

                default:
                    BT_DEBUG ("Invalid byte %02x after esc byte", byte);
                    pbuf_free(h5->rx_skb);
                    h5->rx_skb = NULL;
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
            }
    }
}
_ATTR_LWBT_UARTIF_CODE_
void hci_recv_frame(PBUF *skb)
{
    switch (skb->pkt_type)
    {
        case HCI_ACLDATA_PKT:
            pbuf_header(skb, -HCI_ACL_HDR_LEN);
            hci_acl_input(skb);

            break;
        case HCI_COMMAND_PKT:

            break;
        case HCI_EVENT_PKT:
            pbuf_header(skb, -HCI_EVENT_HDR_LEN);
            hci_event_input(skb);
            pbuf_free(skb);
            break;
        case HCI_SCODATA_PKT:
            #ifdef _A2DP_SINK_
            pbuf_header(skb, -HCI_SCO_HDR_LEN);
            hci_sco_input(skb);
            #endif
            break;
        case H5_LE_PKT:
            pbuf_free(skb);
            break;
        case H5_ACK_PKT:
            pbuf_free(skb);
            break;
        case H5_VDRSPEC_PKT:
            pbuf_free(skb);
            break;
        default:
            BT_DEBUG("Unknown packet type");
            pbuf_free(skb);
            break ;
    }

    //HciServeIsrRequest();
}
_ATTR_LWBT_UARTIF_CODE_
static void h5_complete_rx_pkt(struct phybusif_cb *hu,PBUF *skb)
{
    struct h5_struct *h5 = hu->priv;
    int pass_up;
    //PBUF * skb = h5->rx_skb;
#if 1
    if (H5_PROC_BUF[0] & 0x80)     /* reliable pkt */
    {
        //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
//        h5->rxseq_txack++;
//        h5->rxseq_txack %= 0x8;
//        h5->txack_req    = 1;

        /* If needed, transmit an ack pkt */
        hci_uart_tx_wakeup(hu); //wp 20151016
    }
#endif
    h5->rxack = (H5_PROC_BUF[0] >> 3) & 0x07;
    //BT_DEBUG("Request for pkt %u from card", h5->rxack);
//    printf("in <<:");
//    debug_hex(skb->payload, skb->buflen, 16);
    h5_pkt_cull(h5);

    if ((H5_PROC_BUF[1] & 0x0f) == 2 &&
        H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_ACLDATA_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 4 &&
             H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_EVENT_PKT;
        pass_up = 1;
        //BT_DEBUG("pkt_type = HCI_EVENT_PKT");
        if (need_check_bt_state)
        {
            /*
             * after having received packets from controller, del hungup timer
            */
            if (h5->is_checking)
            {
                h5->is_checking = FALSE;
                BT_DEBUG("cancle state err work\n");
                //cancel_delayed_work(&bt_state_check_work);
                //cancel_delayed_work(&bt_state_err_work); //wp 20151016
                //mutex_unlock(&sem_exit);
                rkos_semaphore_give(sem_exit);
            }
        }

    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 3)
    {
        skb->pkt_type = HCI_SCODATA_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 15 &&
             !(H5_PROC_BUF[0] & 0x80))
    {
        h5_handle_le_pkt(hu,skb);//Link Establishment Pkt
        pass_up = 0;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 1 &&
             H5_PROC_BUF[0] & 0x80)
    {
        skb->pkt_type = HCI_COMMAND_PKT;
        pass_up = 1;
    }
    else if ((H5_PROC_BUF[1] & 0x0f) == 14)
    {
        skb->pkt_type = H5_VDRSPEC_PKT;
        pass_up = 1;
    }
    else
        pass_up = 0;

    //if (!pass_up)
    if (pass_up)
    {
        //struct hci_event_hdr hdr;
        uint8 desc = (H5_PROC_BUF[1] & 0x0f);

        if (desc != H5_ACK_PKT && desc != H5_LE_PKT)
        {

            skb_to_pbuf(skb);
            pbuf_header(skb, -4);
            //printf("skb = %x\n",(uint32)skb);
            hci_recv_frame(skb);//this is a hci frame //wp
            //printf("f out\n");
        }
        else
        {
            /* Pull out H5 hdr */
            //skb_pull(h5->rx_skb, 4);
            BT_DEBUG("H5_ACK_PKT || H5_LE_PKT");
            skb_to_pbuf(skb);
            pbuf_header(skb, -4);
            if (need_check_bt_state)
            {
                //schedule_delayed_work(&bt_state_check_work, HZ * 60); //wp 20151016
            }
            hci_recv_frame(skb);//this is a hci frame //wp
            //pbuf_free(skb);
        }

        //h5->rx_state = H5_W4_PKT_DELIMITER;
        //h5->rx_skb = NULL;
    }
    else
    {
        pbuf_free(skb);
        //h5->rx_state = H5_W4_PKT_DELIMITER;
        //h5->rx_skb = NULL;
    }
}

_ATTR_LWBT_UARTIF_CODE_
static uint16 bscp_get_crc(struct h5_struct *h5)
{
    uint16 i = H5_RECV_BUF[h5->rx_skb->buflen - 2];
    uint16 j = H5_RECV_BUF[h5->rx_skb->buflen - 1];
    //return get_unaligned_be16(&h5->rx_skb->buf[h5->rx_skb->len - 2]);
    return  (j<< 8 | i);
}

/* Recv data */
_ATTR_LWBT_UARTIF_CODE_
static int h5_recv(struct phybusif_cb *hu, void *data, int count)
{
    struct h5_struct *h5 = hu->priv;
    //register unsigned char *ptr;
    unsigned char *ptr;
    //BT_DEBUG("hu %p count %d rx_state %d rx_count %ld",hu, count, h5->rx_state, h5->rx_count);

    ptr = data;
    while (count)
    {
        if (h5->rx_count)
        {
            if (*ptr == 0xc0)
            {
                //BT_DEBUG("Short H5 packet");
                pbuf_free(h5->rx_skb);
                h5->rx_state = H5_W4_PKT_START;
                h5->rx_count = 0;
            }
            else
                h5_unslip_one_byte(h5, *ptr);

            ptr++;
            count--;
            continue;
        }

        switch (h5->rx_state)
        {
            case H5_W4_HDR:
                if ((0xff & (uint8) ~ (H5_RECV_BUF[0] + H5_RECV_BUF[1] +
                                       H5_RECV_BUF[2])) != H5_RECV_BUF[3])
                {
                    BT_DEBUG("Error in H5 hdr checksum");
                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                if ( ( H5_RECV_BUF[0] & 0x80)  /* reliable pkt */
                     && (H5_RECV_BUF[0] & 0x07) != h5->rxseq_txack)
                {
                    //BT_DEBUG ("Out-of-order packet arrived, got %u expected %u",
                    //          H5_RECV_BUF[0] & 0x07, h5->rxseq_txack);
                    printf("e:got %u expected %u",
                              H5_RECV_BUF[0] & 0x07, h5->rxseq_txack);
                    h5->txack_req = 1;
                    hci_uart_tx_wakeup(hu);
                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                h5->rx_state = H5_W4_DATA;
                h5->rx_count = (H5_RECV_BUF[1] >> 4) +
                               (H5_RECV_BUF[2] << 4); /* May be 0 */
                continue;

            case H5_W4_DATA:
                if (H5_RECV_BUF[0] & 0x40)     /* pkt with crc */
                {
                    h5->rx_state = H5_W4_CRC;
                    h5->rx_count = 2;
                }
                else
                {
                    //h5_complete_rx_pkt(hu);
                    pbuf_queue_tail((PBUF**)&hu->h5_unread,h5->rx_skb);
                    if (H5_RECV_BUF[0] & 0x80)     /* reliable pkt */
                    {
                        //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
                        h5->rxseq_txack++;
                        h5->rxseq_txack %= 0x8;
                        h5->txack_req    = 1;

                        /* If needed, transmit an ack pkt */
                        //hci_uart_tx_wakeup(hu); //wp 20151016
                    }
                    h5->rx_skb = NULL;
                    h5->rx_state = H5_W4_PKT_DELIMITER;

                    HciServeIsrRequest();
                }
                continue;

            case H5_W4_CRC:
                if (bitrev16(h5->message_crc) != bscp_get_crc(h5))
                {
                    //BT_DEBUG ("Checksum failed: computed %04x received %04x",
                    //          bitrev16(h5->message_crc),
                    //          bscp_get_crc(h5));

                    pbuf_free(h5->rx_skb);
                    h5->rx_state = H5_W4_PKT_DELIMITER;
                    h5->rx_count = 0;
                    continue;
                }
                skb_trim(h5->rx_skb, h5->rx_skb->buflen - 2);
                //h5_complete_rx_pkt(hu);
                pbuf_queue_tail((PBUF**)&hu->h5_unread,h5->rx_skb);
                if (H5_RECV_BUF[0] & 0x80)     /* reliable pkt */
                {
                    //BT_DEBUG("Received seqno %u from card", h5->rxseq_txack);
                    h5->rxseq_txack++;
                    h5->rxseq_txack %= 0x8;
                    h5->txack_req    = 1;

                    /* If needed, transmit an ack pkt */
                    //hci_uart_tx_wakeup(hu); //wp 20151016
                }
                h5->rx_skb = NULL;
                h5->rx_state = H5_W4_PKT_DELIMITER;

                HciServeIsrRequest();
                continue;

            case H5_W4_PKT_DELIMITER:
                switch (*ptr)
                {
                    case 0xc0:
                        h5->rx_state = H5_W4_PKT_START;
                        break;
                    default:
                        //BT_DEBUG("Ignoring byte %02x", *ptr);
                        break;
                }
                ptr++;
                count--;
                break;

            case H5_W4_PKT_START:
                switch (*ptr)
                {
                    case 0xc0:
                        ptr++;
                        count--;
                        break;

                    default:
                        h5->rx_state = H5_W4_HDR;
                        h5->rx_count = 4;
                        h5->rx_esc_state = H5_ESCSTATE_NOESC;
                        H5_CRC_INIT(h5->message_crc);
                        //BT_DEBUG("H5_W4_PKT_START");
                        /* Do not increment ptr or decrement count
                         * Allocate packet. Max len of a H5 pkt=
                         * 0xFFF (payload) +4 (header) +2 (crc) */

                        //h5->rx_skb = bt_skb_alloc(0x1005, GFP_ATOMIC);
                        h5->rx_skb = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
                        if (!h5->rx_skb)
                        {
                            BT_DEBUG("Can't allocate mem for new packet");
                            h5->rx_state = H5_W4_PKT_DELIMITER;
                            h5->rx_count = 0;
                            return 0;
                        }
                        //h5->rx_skb->dev = (void *) hu->hdev; //wp
                        break;
                }
                break;
        }
    }
    return count;
}

_ATTR_LWBT_UARTIF_CODE_
void hci_uart_tx_wakeup(struct phybusif_cb *hu)
{
    //send data
    rkos_semaphore_give(g_cb.h5_data_send_sem);
}

/* Arrange to retransmit all messages in the relq. */
_ATTR_LWBT_UARTIF_CODE_
static void h5_timed_event(pTimer id)
{
    struct phybusif_cb *hu = rkos_get_timer_param(id);
    struct h5_struct *h5 = hu->priv;
    PBUF *skb;
    unsigned long flags;

    //BT_DEBUG("hu %p retransmitting %u pkts", hu, h5->unack.qlen);
    BT_DEBUG("h5->unack = 0x%04x", (uint32)h5->unack);
    //spin_lock_irqsave_nested(&h5->unack.lock, flags, SINGLE_DEPTH_NESTING); //wp
    rkos_semaphore_take(h5->unack_lock, MAX_DELAY);
    while ((skb = pbuf_dequeue_tail(&h5->unack)) != NULL)
    {
        h5->msgq_txseq = (h5->msgq_txseq - 1) & 0x07;
        pbuf_queue_head(&h5->rel, skb);
    }
    //spin_unlock_irqrestore(&h5->unack.lock, flags); //wp
    rkos_semaphore_give(h5->unack_lock);
    hci_uart_tx_wakeup(hu);// wp maybe to send the uart data
}
#if 0
static void h5_send_uevent(struct hci_uart* hu)
{
    char BT_restart[] = "BTRESTART=1";
    char *env_p[] =
    {
        BT_restart,
        NULL
    };
    int ret = -1;

    if (hu && hu->hdev)
    {
        ret = kobject_uevent_env(&hu->hdev->dev.kobj, KOBJ_OFFLINE, env_p);
        //BT_DEBUG("send event to upper layer:%x", ret);
    }
    else
        BT_DEBUG("h5_send_uevent without pointer exists");
}


static void h5_bt_state_err_worker(struct work_struct *private_)
{
    BT_DEBUG("Realtek: BT is NOT working now, try notify\n");

    if (hci_uart_info)
    {
        h5_send_uevent(hci_uart_info);
    }
    //mutex_unlock(&sem_exit);
    rkos_semaphore_give(sem_exit);
}

/*
 * timer function to check if controller state
*/
static void h5_bt_state_check_worker(struct work_struct *private_)
{
    struct hci_uart *hu = hci_uart_info;
    struct h5_struct *h5 = hu->priv;
    struct sk_buff* pollcmd = NULL;
    uint8 cmd[3] = {0};
    int ret=0;

    BT_DEBUG("Realtek to check4hung\n");
    ret = rkos_semaphore_take(sem_exit);

    if (ret !=0)
    {
        BT_DEBUG("Realtek mutex lock interrupted:%x, %s()\n", ret, __func__);
    }
    //send command and wait for any response.
    cmd[0] = 0x22;
    cmd[1] = 0xfc;
    //pollcmd = bt_skb_alloc(3, GFP_ATOMIC);
    pollcmd = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL);
    if (!pollcmd)
    {
        BT_DEBUG("allocate buffer for poll cmd error");
        return;
    }

    skb_put(pollcmd, sizeof(cmd));
    pollcmd->pkt_type = HCI_COMMAND_PKT;
    /*
     *  It's judged that controller has hung up
     *  if no response received within 3HZ
    */
    h5->is_checking = 1;
    schedule_delayed_work(&bt_state_err_work, HZ * 3);
    /*
     *  make sure the bt_state_err_work perform completely
    */
    memcpy(pollcmd->data, cmd, sizeof(cmd));
    pbuf_queue_head(&h5->rel, pollcmd);
    hci_uart_tx_wakeup(hu);
}

static int h5_config_state_check(void)
{
    char    *config_path=H5_BTSTATE_CHECK_CONFIG_PATH;
    char    *buffer=NULL;
    struct file   *filp=NULL;
    mm_segment_t old_fs = get_fs();

    int result=0;
    set_fs (KERNEL_DS);

    //open file
    filp = filp_open(config_path, O_RDONLY, 0);
    if (IS_ERR(filp))
    {
        //BT_DEBUG("open file fail?\n");
        result=-1;
        goto error2;
    }

    if(!(filp->f_op) || !(filp->f_op->read) )
    {
        BT_DEBUG("file %s cann't readable?\n",config_path);
        result = -1;
        goto error1;
    }

    buffer = kmalloc(16, GFP_KERNEL);
    if(buffer==NULL)
    {
        BT_DEBUG("alllocate mem for file fail?\n");
        result = -1;
        goto error1;
    }

    if(filp->f_op->read(filp, buffer, 16, &filp->f_pos)<0)
    {
        BT_DEBUG("read file error?\n");
        result = -1;
        goto error1;
    }

    if(memcmp(buffer,"0",1)==0)
    {
        //check bt state off
        need_check_bt_state = 0;
        BT_DEBUG("close check bt state");
    }

error1:
    if(buffer)
        kfree(buffer);

    if(filp_close(filp,NULL))
        BT_DEBUG("Config_FileOperation:close file fail\n");

error2:
    set_fs (old_fs);

    return result;
}

//for sleep check begin
static int h5_can_sleep(void)
{
    //BT_DEBUG("%s(), wake_sleep:%x, uport:%p\n", __func__, atomic_read(&hsi->wake_sleep), hsi->uport);

    return (&hsi->wake_sleep) && (hsi->uport!=NULL));
}
#define test_bit(x, State)  ((State >> x) & 0x00000001)

#define set_bit(x, State)    ( State |=(1<<x) )
#define CLEAR_BTSTATE(x, State)  ( State &= ~(1<<x))
static void h5_tx_sleep_wakeup(void)
{
    if (test_bit(STACK_SLEEP, flags))
    {
        //  BT_DEBUG("%s(): Realtek to wakeup TX\n", __func__);
        wake_lock(&hsi->wake_lock);
        mod_timer(&h5_tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
        atomic_dec(&(hsi->wake_sleep));
        clear_bit(STACK_SLEEP, &flags);
    }
}
/**
 * @brief@  main sleep work handling function which update the flags
 * and activate and deactivate UART ,check FIFO.
 */
static void h5_sleep_work(struct work_struct *work)
{
    if (h5_can_sleep())
    {
        //  BT_DEBUG("Realtek: can sleep...\n");
        /* already asleep, this is an error case */
        if (test_bit(STACK_SLEEP, &flags))
        {
            BT_DEBUG("Realtek: error, already asleep");
            return;
        }

        if (hsi->uport->ops->tx_empty(hsi->uport))
        {
            //      BT_DEBUG("going to sleep...\n");
            set_bit(STACK_SLEEP , flags);
            /* UART clk is not turned off immediately. Release
             * wakelock after 500 ms.
             */
            wake_lock_timeout(&hsi->wake_lock, HZ);
        }
        else
        {
            //      BT_DEBUG("tx buffer is not empty, modify timer...\n");
            /*lgh add*/
            hsi->wake_sleep--;
            /*lgh add end*/
            mod_timer(&h5_tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
            return;
        }
    }
    else
    {
        h5_tx_sleep_wakeup();
    }
}


/**
 * Handles transmission timer expiration.
 * @param data Not used.
 */
static void h5_sleep_tx_timer_expire(pTimer id)
{
#if 0
    unsigned long irq_flags;

    spin_lock_irqsave(&rw_lock, irq_flags);

//  BT_DEBUG("Realtek: Tx timer expired\n");

    /* were we silent during the last timeout? */
    if (!test_bit(STACK_TXDATA, &flags))
    {
        //  BT_DEBUG("Realtek: Tx has been idle\n");
        atomic_inc(&(hsi->wake_sleep));
        schedule_delayed_work(&sleep_workqueue, 0);
    }
    else
    {
        //  BT_DEBUG("Realtek: Tx data during last period\n");
        mod_timer(&h5_tx_timer, jiffies + (TX_TIMER_INTERVAL*HZ));
    }

    /* clear the incoming data flag */
    clear_bit(STACK_TXDATA, &flags);

    spin_unlock_irqrestore(&rw_lock, irq_flags);
#endif
}


static void h5_sleep_stack_xmit_data(void)
{
#if 0
    unsigned long irq_flags;

    spin_lock_irqsave(&rw_lock, irq_flags);

    set_bit(STACK_TXDATA, &flags);

    /*if tx is going to sleep
     */
    if (atomic_read(&(hsi->wake_sleep)) == 1)
    {
        //  BT_DEBUG("TX  was sleeping\n");
        h5_tx_sleep_wakeup();
    }

    spin_unlock_irqrestore(&rw_lock, irq_flags);
#endif
}

static int h5_parse_hci_event(struct notifier_block *this,
                              unsigned long event, void *data)
{
    struct hci_dev *hdev = (struct hci_dev *) data;
    struct hci_uart *hu;
    struct uart_state *state;

    if (!hdev)
        return NOTIFY_DONE;

//  BT_DEBUG("Realtek: hci event %ld\n", event);
    switch (event)
    {
        case HCI_DEV_REG:
            //      BT_DEBUG("Realtek: hci event %ld hdev = %p\n", event, hdev);
            if (!h5_hci_hdev)
            {
                h5_hci_hdev = hdev;
                hu  = (struct hci_uart *) hdev->driver_data;
                state = (struct uart_state *) (hu->tty->driver_data);
                hsi->uport = state->uart_port;
                rkos_mod_timer(h5_tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ),MAX_DELAY);
                wake_lock(&hsi->wake_lock);
            }

            //      BT_DEBUG("Realtek: start check\n");
            if(need_check_bt_state)
                schedule_delayed_work(&bt_state_check_work, HZ * 60);
            break;
        case HCI_DEV_UNREG:
            //      BT_DEBUG("Realtek: hci event %ld\n", event);

            rkos_delete_timer(h5_tx_timer);
            wake_lock_timeout(&hsi->wake_lock, HZ);
            h5_hci_hdev = NULL;
            hsi->uport = NULL;
            //      BT_DEBUG("Realtek: stop check\n");
            if(need_check_bt_state)
                cancel_delayed_work(&bt_state_check_work);
            break;

        case HCI_DEV_WRITE:
            h5_sleep_stack_xmit_data();
            break;
    }

    return NOTIFY_DONE;
}

/*
 * h5 sleep function init
 */
static void h5_sleep_init(void)
{
#if 0
    flags = 0; /* clear all status bits */

    hsi = kzalloc(sizeof(struct h5_sleep_info), GFP_KERNEL);
    if (hsi==NULL)
        return -ENOMEM;

    memset(hsi,0, sizeof(struct h5_sleep_info));

    /* Initialize spinlock. */
    spin_lock_init(&rw_lock);

    /* Initialize timer */
    init_timer(&h5_tx_timer);
    h5_tx_timer.function = h5_sleep_tx_timer_expire;
    h5_tx_timer.data = 0;

    atomic_set(&hsi->wake_sleep, 0);
    hci_register_notifier(&hci_event_nblock);


    wake_lock_init(&hsi->wake_lock, WAKE_LOCK_SUSPEND, "h5_sleep");
#endif

}
/*
   h5 sleep function exit
 */
static void h5_sleep_exit(void)
{
#if 0
    hci_unregister_notifier(&hci_event_nblock);
    del_timer(&h5_tx_timer);
    wake_lock_destroy(&hsi->wake_lock);
    kfree(hsi);
#endif
}
//for sleep check end
#endif
#define H5_RETRY 3
_ATTR_LWBT_UARTIF_CODE_
static int h5_open(struct phybusif_cb *hu)
{
    struct h5_struct *h5;

    BT_DEBUG("hu %p", hu);

    h5 = rkos_memory_malloc(sizeof(*h5));
    memset(h5, 0 ,sizeof(*h5));
    if (!h5)
        return -ENOMEM;

    hu->priv = h5;

    //init_timer(&h5->th5);
    h5->th5 = rkos_create_timer(20, 0, hu, h5_timed_event);
    h5->unack_lock = rkos_semaphore_create(1,1);
    //h5->th5.function = h5_timed_event;
    //h5->th5.data     = (u_long) hu;
    //h5_tx_timer = rkos_create_timer(20, 0,NULL, h5_sleep_tx_timer_expire);
    //h5_config_state_check();
    if (need_check_bt_state)
    {
        //mutex_init(&sem_exit);
        sem_exit = rkos_semaphore_create(1,1);
        hci_uart_info = hu;
    }

    h5->rx_state = H5_W4_PKT_DELIMITER;

    if (txcrc)
        h5->use_crc = 1;
    g_cb.data_recv_task_handle = RKTaskCreate2(phybusif_data_recv_task, NULL, NULL, "hci_data_recv", BT_HCI_DATA_RECV_TASK_STACK_SIZE*2,
                                 BT_HCI_DATA_RECV_TASK_PRIORITY, NULL);
    rkos_delay(10);
    g_cb.data_process_task_handle =  RKTaskCreate2(phybusif_data_process_task, NULL, NULL, "hci_data_proc", BT_HCI_DATA_PROCESS_TASK_STACK_SIZE,
                                     BT_HCI_DATA_PROCESS_TASK_PRIORITY, NULL);
    rkos_delay(10);
    g_cb.h5_send_data_task_handle =  RKTaskCreate2(phybusif_h5_data_send_task, NULL, NULL, "hci_send_proc", BT_HCI_H5_DATA_SEND_TASK_STACK_SIZE*4,
                                     BT_HCI_H5_DATA_SEND_TASK_PRIORITY, NULL);
    rkos_delay(10);

#if 1
    {
        uint8 sync_pkt[2]     = { 0x01, 0x7e};
        int retry = H5_RETRY;
        int timeout = 15; //150ms
    syncretry:

        timeout = 15; //150ms
        PBUF *sync = h5_prepare_pkt(h5, sync_pkt,2, H5_LE_PKT);
        BT_DEBUG("sync->buflen %d", sync->buflen);
        debug_hex(sync->payload, sync->buflen, 16);
        g_cb.hw_conrtol->write(sync->payload,sync->buflen);

        pbuf_free(sync);

        while(1)
        {
            if(h5->sync_resp_recved)
            {
                break;
            }
            rkos_delay(10);
            if(timeout-- == 0)
            {
                if(retry--)
                {
                    goto syncretry;
                }
                else
                {
                    goto err;
                }
            }
        }
        retry = H5_RETRY;
        uint8 conf_pkt[3]     = { 0x03, 0xfc,5};
    confretry:

        timeout = 15; //150ms

        PBUF *conf = h5_prepare_pkt(h5, conf_pkt,3, H5_LE_PKT);
        BT_DEBUG("conf->buflen = %d", conf->buflen);
        debug_hex(conf->payload, conf->buflen, 16);
        g_cb.hw_conrtol->write(conf->payload,conf->buflen);

        pbuf_free(conf);

        while(1)
        {
            if(h5->conf_resp_recved)
            {
                break;
            }
            rkos_delay(10);
            if(timeout-- == 0)
            {
                if(retry--)
                {
                    goto confretry;
                }
                else
                {
                    goto err;
                }
            }
        }

        PBUF *pure_ack = h5_prepare_pkt(h5, NULL,0, H5_ACK_PKT);
        BT_DEBUG("pure_ack->buflen = %d", pure_ack->buflen);
        debug_hex(pure_ack->payload, pure_ack->buflen, 16);
        g_cb.hw_conrtol->write(pure_ack->payload,pure_ack->buflen);

        pbuf_free(pure_ack);

    }
#endif

    return 0;

err:
    if (need_check_bt_state)
    {
        if(sem_exit)
        {
            rkos_semaphore_delete(sem_exit);
            sem_exit = NULL;
        }
    }
    pbuf_queue_purge(h5->unack);
    pbuf_queue_purge(h5->rel);
    pbuf_queue_purge(h5->unrel);
    //del_timer(&h5->th5);
    rkos_delete_timer(h5->th5);
    rkos_memory_free(h5);
    RKTaskDelete2(g_cb.data_recv_task_handle);
    RKTaskDelete2(g_cb.data_process_task_handle);
    RKTaskDelete2(g_cb.h5_send_data_task_handle);
    return ERR_TIMEOUT;
}
_ATTR_LWBT_UARTIF_CODE_
static int h5_close(struct phybusif_cb *hu)
{
    struct h5_struct *h5 = hu->priv;
    hu->priv = NULL;

    BT_DEBUG("hu %p", hu);

    pbuf_queue_purge(h5->unack);
    pbuf_queue_purge(h5->rel);
    pbuf_queue_purge(h5->unrel);
    //del_timer(&h5->th5);
    rkos_delete_timer(h5->th5);
    RKTaskDelete2(g_cb.data_recv_task_handle);
    RKTaskDelete2(g_cb.data_process_task_handle);
    RKTaskDelete2(g_cb.h5_send_data_task_handle);
    if (need_check_bt_state)
    {
        int ret = 0;
//        cancel_delayed_work(&bt_state_check_work);   //wp
//        ret = mutex_lock_interruptible(&sem_exit);  //wait work queue perform completely //del by wp
//        if (ret != 0)
//        {
//            BT_DEBUG("Realtek mutex unlocked:%x, %s()\n", ret, __func__);
//        }

        if(sem_exit)
        {
            rkos_semaphore_delete(sem_exit);
            sem_exit = NULL;
        }
        hci_uart_info = NULL;
    }
    rkos_semaphore_delete(h5->unack_lock);
    rkos_memory_free(h5);
    return 0;
}

_ATTR_LWBT_UARTIF_CODE_
int h5_init(void)
{
    int err =0;
    //h5_sleep_init();
    return err;
}
_ATTR_LWBT_UARTIF_CODE_
int h5_deinit(void)
{
    //for sleeep check
    //h5_sleep_exit();
    return 0;
}

#endif//BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H5
#endif

