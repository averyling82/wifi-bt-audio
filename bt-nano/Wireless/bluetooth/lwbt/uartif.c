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
#if BT_UART_INTERFACE_CONFIG == BT_UART_INTERFACE_H4

#ifdef _A2DP_SINK_
_ATTR_LWBT_DATA_ uint8 scoTxBuf[520];
#endif

//struct phybusif_cb *g_cbP;
_ATTR_LWBT_BSS_ struct phybusif_cb g_cb;

void HciServeIsrRequest(void);
void  phybusif_data_recv_task(void *arg);
void  phybusif_data_process_task(void *arg);
/*-----------------------------------------------------------------------------------*/
/* Initializes the physical bus interface
 */
/*-----------------------------------------------------------------------------------*/



_ATTR_LWBT_INIT_CODE_
err_t phybusif_init(bt_hw_control_t * hw_control)
{
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
    pbuf_init();
    BT_DEBUG();
    phybusif_reset(&g_cb);
    BT_DEBUG();
    g_cb.hw_conrtol = hw_control;
    g_cb.hci_data_proc_sem = rkos_semaphore_create(10, 0);
    if(g_cb.hci_data_proc_sem == NULL)
    {
        return ERR_MEM;
    }
    g_cb.data_recv_task_handle = RKTaskCreate2(phybusif_data_recv_task, NULL, NULL,"hci_data_recv", BT_HCI_DATA_RECV_TASK_STACK_SIZE*2*4,
                                    BT_HCI_DATA_RECV_TASK_PRIORITY, NULL);
    rkos_delay(10);
    g_cb.data_process_task_handle = RKTaskCreate2(phybusif_data_process_task, NULL, NULL, "hci_data_proc", BT_HCI_DATA_PROCESS_TASK_STACK_SIZE*4,
                                    BT_HCI_DATA_PROCESS_TASK_PRIORITY, NULL);
    rkos_delay(10);
    BT_DEBUG();

    return ERR_OK;
}

_ATTR_LWBT_INIT_CODE_
err_t phybusif_deinit(bt_hw_control_t * hw_control)
{
    pbuf_deinit();
    if(g_cb.hci_data_proc_sem)
    {
        rkos_semaphore_delete(g_cb.hci_data_proc_sem);
    }

    if(g_cb.data_recv_task_handle)
    {
        RKTaskDelete2(g_cb.data_recv_task_handle);
    }

    if(g_cb.data_process_task_handle)
    {
        RKTaskDelete2(g_cb.data_process_task_handle);
    }
    memset(&g_cb, 0, sizeof(struct phybusif_cb));
}


_ATTR_LWBT_UARTIF_CODE_
void hcievent_pbuf_input(struct pbuf *p)
{
#if 0
    struct pbuf *q;
    if(g_cb.hci_event_unread == NULL)
    {
        g_cb.hci_event_unread = p;
    }
    else
    {
        rkos_enter_critical();
        q = g_cb.hci_event_unread;
        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
        rkos_exit_critical();
    }
#else
    pbuf_queue_tail((void*)&(g_cb.hci_event_unread), p);
#endif
}

_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_event()
{
#if 0
    struct pbuf *p;

    if((g_cb.hci_event_unread == NULL))
    {
        return NULL;
    }
    else
    {
        //IntDisable(INT_ID17_UART);
        rkos_enter_critical();
        p = g_cb.hci_event_unread;
        g_cb.hci_event_unread = g_cb.hci_event_unread->next;
        p->next = NULL;
        //IntEnable(INT_ID17_UART);
        rkos_exit_critical();
        return p;
    }
#else
    return pbuf_dequeue((void*)&g_cb.hci_event_unread);
#endif
}
_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_acl()
{
#if 0
    struct pbuf *p;

    if((g_cb.hci_acl_unread == NULL))
    {
        return NULL;
    }
    else
    {
        //IntDisable(INT_ID17_UART);
        rkos_enter_critical();
        p = g_cb.hci_acl_unread;
        g_cb.hci_acl_unread = g_cb.hci_acl_unread->next;
        p->next = NULL;
        //IntEnable(INT_ID17_UART);
        rkos_exit_critical();

        return p;
    }
#else
    return pbuf_dequeue((void *)&g_cb.hci_acl_unread);
#endif
}
_ATTR_LWBT_UARTIF_CODE_
void hciacl_pbuf_input(struct pbuf *p)
{
#if 0
    struct pbuf *q;

    if(g_cb.hci_acl_unread == NULL)
    {
        g_cb.hci_acl_unread = p;
    }
    else
    {
        rkos_enter_critical();
        q = g_cb.hci_acl_unread;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
        rkos_exit_critical();
    }
#else
    pbuf_queue_tail((void*)&(g_cb.hci_acl_unread), p);
#endif
}

#ifdef _A2DP_SINK_
_ATTR_LWBT_UARTIF_CODE_
struct pbuf * get_hci_sco()
{
#if 0
    struct pbuf *p;

    if((g_cb.hci_sco_unread == NULL))
    {
        return NULL;
    }
    else
    {
        //IntDisable(INT_ID17_UART);
        rkos_enter_critical();
        p = g_cb.hci_sco_unread;
        g_cb.hci_sco_unread = g_cb.hci_sco_unread->next;
        p->next = NULL;
        //IntEnable(INT_ID17_UART);
        rkos_exit_critical();

        return p;
    }
#else
    pbuf_dequeue((void *)&g_cb.hci_sco_unread);
#endif
}
_ATTR_LWBT_UARTIF_CODE_
void hcisco_pbuf_input(struct pbuf *p)
{
#if 0
    struct pbuf *q;

    if(g_cb.hci_sco_unread == NULL)
    {
        g_cb.hci_sco_unread = p;
    }
    else
    {
        rkos_enter_critical();
        q = g_cb.hci_sco_unread;

        while(q->next)
        {
            q = q->next;
        }

        q->next = p;
        rkos_exit_critical();
    }
#else
    pbuf_queue_tail((void*)&(g_cb.hci_sco_unread), p);
#endif
}
#endif

_ATTR_LWBT_UARTIF_CODE_
void phybusif_waitsend_pbuf_input(struct pbuf *p)
{
#if 0
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
#else
    pbuf_queue_tail((void*)&(g_cb.waitsend), p);
#endif
}

/*-----------------------------------------------------------------------------------*/
_ATTR_LWBT_UARTIF_CODE_
int phybusif_reset(struct phybusif_cb *cb)
{
    /* Init new ctrl block */
    /* Alloc new pbuf. lwIP will handle dealloc */
    if((cb->p = pbuf_alloc(PBUF_RAW, PHY_FRAME_MAX_SIZE, PBUF_POOL)) == NULL)
    {

        BT_DEBUG();
        while(1);
        cb->q = cb->p; /* Make p the pointer to the head of the pbuf chain and q to the tail */

        cb->tot_recvd = 0;
        cb->recvd = 0;

        cb->state = W4_WAIT_BUF;
        return ERR_MEM; /* Could not allocate memory for pbuf */

    }
    cb->q = cb->p; /* Make p the pointer to the head of the pbuf chain and q to the tail */

    cb->tot_recvd = 0;
    cb->recvd = 0;

    cb->state = W4_PACKET_TYPE;
    return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
#if 0
_ATTR_LWBT_UARTIF_CODE_
int phybusif_input(unsigned char c)
{
    struct phybusif_cb *cb;
    unsigned char n;

    cb = &g_cb;

    switch(cb->state)
    {
        case W4_WAIT_BUF:
            phybusif_reset(cb);
            break;
        case W4_PACKET_TYPE:
            switch(c)
            {
                case HCI_ACL_DATA_PACKET:
                    cb->state = W4_ACL_HDR;
                    break;
                case HCI_EVENT_PACKET:
                    cb->state = W4_EVENT_HDR;
                    break;

                case HCI_SCO_DATA_PACKET:
                    cb->state = W4_SCO_HDR;
                    break;


                default:
                    if(cb->hw_conrtol)
                    {
                        if(cb->hw_conrtol->vendor_uart_recv)
                        {
                            cb->hw_conrtol->vendor_uart_recv(&c, 1);
                        }
                    }
                    break;
            }
            break;

        case W4_SCO_HDR:

            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_SCO_HDR_LEN)
            {
                cb->scohdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_SCO_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->scohdr->len > 0)
                {
                    cb->state = W4_SCO_DATA;
                }
                else
                {
#ifdef _A2DP_SINK_
                    hcisco_pbuf_input(cb->p);
#else
                    pbuf_free(cb->p);
#endif
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

            break;

        case W4_SCO_DATA:


            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;

            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);

            }

            if(cb->tot_recvd == cb->scohdr->len)
            {
#ifdef _A2DP_SINK_
                hcisco_pbuf_input(cb->p);
#else
                pbuf_free(cb->p);
#endif
                phybusif_reset(cb);
                HciServeIsrRequest();
                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }

            break;

        case W4_EVENT_HDR:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_EVENT_HDR_LEN)
            {
                cb->evhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_EVENT_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->evhdr->len > 0)
                {
                    cb->state = W4_EVENT_PARAM;
                }
                else
                {
                    hcievent_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();

                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

        case W4_EVENT_PARAM:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);
            }
            if(cb->tot_recvd == cb->evhdr->len)
            {

                hcievent_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();

                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }
            break;

        case W4_ACL_HDR:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;
            if(cb->recvd == HCI_ACL_HDR_LEN)
            {
                cb->aclhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_ACL_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->aclhdr->len > 0)
                {
                    cb->state = W4_ACL_DATA;
                }
                else
                {
                    hciacl_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
                }
            }
            break;

        case W4_ACL_DATA:
            ((uint8 *)cb->q->payload)[cb->recvd] = c;
            cb->tot_recvd++;
            cb->recvd++;

            if(cb->recvd == cb->q->len)
            {
                /* Pbuf full. alloc and add new tail to chain */
                cb->recvd = 0;
                pbuf_free(cb->p);
                phybusif_reset(cb);

            }

            if(cb->tot_recvd == cb->aclhdr->len)
            {
                hciacl_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();
                return ERR_OK; /* Since there most likley won't be any more data in the input buffer */
            }
            break;

        default:
            break;
    }

    return ERR_OK;
}
#endif
/*-----------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------*/
/* hci_serve():
 *
 * Called by main loop
 *
 */
/*-----------------------------------------------------------------------------------*/

_ATTR_LWBT_UARTIF_CODE_
void hci_serve(void)
{
    struct pbuf * eventP = NULL;
    struct pbuf * aclP = NULL;
    struct pbuf * scoP = NULL;

    while(1)
    {
        eventP = get_hci_event();
        if(eventP)
        {
            //printf("hci_serve event\n");
            hci_event_input(eventP);

            pbuf_free(eventP);

            //eventP = get_hci_event();
        }

        if(hci_can_send_packet_now())
        {
            aclP = get_hci_acl();
        }
        else
        {
            aclP = NULL;
        }
        if(aclP)
        {
            hci_acl_input(aclP);

            //aclP = get_hci_acl();
        }

#ifdef _A2DP_SINK_
        scoP = get_hci_sco();

        if(scoP)
        {
            hci_sco_input(scoP);

            //scoP = get_hci_sco();
        }
#endif
        if(eventP == NULL && aclP == NULL && scoP == NULL)
        {
            break;
        }

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

void HciServeIsrDisable(void)
{

}
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

    for(q = p; q != NULL; q = q->next)
    {
        ptr = q->payload;
        g_cb.hw_conrtol->write(ptr,q->len);

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

    memcpy(scoTxBuf, scohdr, 4);
    memcpy(scoTxBuf+4, data, datalen);
    g_cb.hw_conrtol->write(scoTxBuf,datalen+4);
}

#endif


_ATTR_LWBT_UARTIF_CODE_
static void h4_recv(struct phybusif_cb *hu, uint8 *data, int count)
{
    struct phybusif_cb *cb = hu;
    uint8 *ptr = data;
    //BT_DEBUG("count=%d\n", count);
    while(count)
    {
        //BT_DEBUG("cb->state = %d, count =%d\n",cb->state, count);
        if (cb->tot_recvd)
        {
            //printf("d=0x%02x ", *ptr);
            ((uint8 *)cb->p->payload)[cb->recvd] = *ptr;
            cb->tot_recvd--;
            cb->recvd++;
            ptr++;
            count--;
            if (cb->tot_recvd)
                continue;
        }
        switch(cb->state)
        {
            case W4_PACKET_TYPE:
                switch(*ptr)
                {
                    case HCI_ACL_DATA_PACKET:
                        cb->state = W4_ACL_HDR;
                        cb->recvd = 0;
                        cb->tot_recvd = HCI_ACL_HDR_LEN;
                        break;
                    case HCI_EVENT_PACKET:
                        cb->tot_recvd = HCI_EVENT_HDR_LEN;
                        cb->recvd = 0;
                        cb->state = W4_EVENT_HDR;
                        break;

                    case HCI_SCO_DATA_PACKET:
                        cb->state = W4_SCO_HDR;
                        cb->recvd = 0;
                        cb->tot_recvd = HCI_SCO_HDR_LEN;
                        break;

                    default:
                        if(cb->hw_conrtol)
                        {
                            if(cb->hw_conrtol->vendor_uart_recv)
                            {
                                cb->hw_conrtol->vendor_uart_recv(ptr, 1);
                                printf("get w4 packet type\n");
                            }
                        }
                        break;
                }
                ptr++;
                count--;
                continue;

            case W4_SCO_HDR:
                cb->scohdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_SCO_HDR_LEN);
                cb->recvd = 0;
                if(cb->scohdr->len > 0)
                {
                    cb->tot_recvd = cb->scohdr->len;
                    cb->state = W4_SCO_DATA;
                    if(cb->scohdr->len > cb->q->len)
                    {
                        BT_DEBUG("bt recv data err:len=%d\n", cb->scohdr->len);
                        pbuf_free(cb->p);
                        phybusif_reset(cb);
                    }
                }
                else
                {
#ifdef _A2DP_SINK_
                    hcisco_pbuf_input(cb->p);
#else
                    pbuf_free(cb->p);
#endif
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                continue;

            case W4_SCO_DATA:
#ifdef _A2DP_SINK_
                hcisco_pbuf_input(cb->p);
#else
                pbuf_free(cb->p);
#endif
                phybusif_reset(cb);
                HciServeIsrRequest();
                continue ; /* Since there most likley won't be any more data in the input buffer */

            case W4_EVENT_HDR:
                //printf("\nW4_EVENT_HDR:");
                //dump(cb->q->payload, HCI_EVENT_HDR_LEN);
                //printf("\n");
                cb->evhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_EVENT_HDR_LEN);
                cb->recvd = 0;
                //BT_DEBUG("event=0x%02x, len=%d\n", cb->evhdr->code, cb->evhdr->len);
                if(cb->evhdr->len > 0)
                {
                    cb->tot_recvd = cb->evhdr->len;
                    cb->state = W4_EVENT_PARAM;
                    if(cb->evhdr->len > cb->q->len)
                    {
                        BT_DEBUG("bt recv data err:len=%d\n", cb->evhdr->len);
                        pbuf_free(cb->p);
                        phybusif_reset(cb);
                    }
                }
                else
                {
                    hcievent_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                continue;

            case W4_EVENT_PARAM:
                //printf("\n get event stream:");
                //dump(cb->q->payload, cb->evhdr->len);
                //printf("\n");
                hcievent_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();

                continue ; /* Since there most likley won't be any more data in the input buffer */

            case W4_ACL_HDR:
                //printf("\nW4_EVENT_HDR:");
                //dump(cb->q->payload, HCI_ACL_HDR_LEN);
                //printf("\n");
                cb->aclhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_ACL_HDR_LEN);
                cb->recvd = 0;
                if(cb->aclhdr->len > 0)
                {
                    cb->tot_recvd = cb->aclhdr->len;
                    cb->state = W4_ACL_DATA;
                    if(cb->aclhdr->len > cb->q->len)
                    {
                        BT_DEBUG("bt recv data err:len=%d\n", cb->aclhdr->len);
                        pbuf_free(cb->p);
                        phybusif_reset(cb);
                    }
                }
                else
                {
                    hciacl_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                continue;

            case W4_ACL_DATA:
                //printf("\n get event stream:");
                //dump(cb->q->payload, cb->aclhdr->len);
                //printf("\n");
                hciacl_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();
                continue ; /* Since there most likley won't be any more data in the input buffer */

            default:
                break;
        }

    }
}

_ATTR_LWBT_UARTIF_CODE_
void  phybusif_data_recv_task(void *arg)
{
    unsigned char c;
    uint8 read_buf[32];
    int realsize = 0;
    struct phybusif_cb *cb;
    cb = &g_cb;
    int needlen = 0;
    int ret;
    //BT_DEBUG();
    while(1)
    {
#if 1
        realsize = cb->hw_conrtol->read(read_buf, 16);
        if (0)//(realsize)
        {
            printf("\n recived:");
            dump(read_buf, realsize);
            printf("\n");
        }
        if (realsize)
            h4_recv(cb, read_buf, realsize);
#else
        //BT_DEBUG("cb->state = %d\n",cb->state);
        switch(cb->state)
        {
            case W4_PACKET_TYPE:
                ret = cb->hw_conrtol->read(&c, 1);
                printf("%02x\n", c);
                //BT_DEBUG("c = %d\n",c);
                switch(c)
                {
                    case HCI_ACL_DATA_PACKET:
                        cb->state = W4_ACL_HDR;
                        break;
                    case HCI_EVENT_PACKET:
                        cb->state = W4_EVENT_HDR;
                        break;

                    case HCI_SCO_DATA_PACKET:
                        cb->state = W4_SCO_HDR;
                        break;

                    default:
                        if(cb->hw_conrtol)
                        {
                            if(cb->hw_conrtol->vendor_uart_recv)
                            {
                                cb->hw_conrtol->vendor_uart_recv(&c, 1);
                                printf("get w4 packet type\n");
                            }
                        }
                        break;
                }
                break;


            case W4_SCO_HDR:
                cb->hw_conrtol->read((uint8 *)cb->q->payload, HCI_SCO_HDR_LEN);
                cb->scohdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_SCO_HDR_LEN);
                if(cb->scohdr->len > 0)
                {
                    cb->state = W4_SCO_DATA;
                    if(cb->scohdr->len > cb->q->len) //for test
                    {
                        printf("bt recv data err\n");
                    }
                }
                else
                {
#ifdef _A2DP_SINK_
                    hcisco_pbuf_input(cb->p);
#else
                    pbuf_free(cb->p);
#endif
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                break;

                break;

            case W4_SCO_DATA:

                cb->hw_conrtol->read((uint8 *)cb->q->payload, cb->scohdr->len);
#ifdef _A2DP_SINK_
                hcisco_pbuf_input(cb->p);
#else
                pbuf_free(cb->p);
#endif
                phybusif_reset(cb);
                HciServeIsrRequest();
                break ; /* Since there most likley won't be any more data in the input buffer */

                break;

            case W4_EVENT_HDR:
                //printf("W4_EVENT_HDR\n");
                cb->hw_conrtol->read((uint8 *)cb->q->payload, HCI_EVENT_HDR_LEN);
                //dump(cb->q->payload, HCI_EVENT_HDR_LEN);
                //printf("\n");
                cb->evhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_EVENT_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                //DEBUG("event=0x%02x\n\n\n", cb->evhdr->code);
                if(cb->evhdr->len > 0)
                {
                    cb->state = W4_EVENT_PARAM;
                    if(cb->evhdr->len > cb->q->len) //for test
                    {
                        printf("bt recv data err\n");
                    }
                    //printf("get W4_EVENT_HDR\n");
                }
                else
                {
                    hcievent_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                break;

            case W4_EVENT_PARAM:
                cb->hw_conrtol->read((uint8 *)cb->q->payload, cb->evhdr->len);
                //printf("W4_EVENT_PARAM\n");
                //dump(cb->q->payload, cb->evhdr->len);
                //debug_hex((char*)cb->evhdr, cb->evhdr->len+2, 16);
                //printf("\n");
                hcievent_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();

                break ; /* Since there most likley won't be any more data in the input buffer */

            case W4_ACL_HDR:
                cb->hw_conrtol->read((uint8 *)cb->q->payload, HCI_ACL_HDR_LEN);
                cb->aclhdr = cb->p->payload;
                pbuf_header(cb->p, -HCI_ACL_HDR_LEN);
                cb->recvd = cb->tot_recvd = 0;
                if(cb->aclhdr->len > 0)
                {
                    cb->state = W4_ACL_DATA;
                }
                else
                {
                    hciacl_pbuf_input(cb->p);
                    phybusif_reset(cb);
                    HciServeIsrRequest();
                    break ; /* Since there most likley won't be any more data in the input buffer */
                }

                break;

            case W4_ACL_DATA:
                cb->hw_conrtol->read((uint8 *)cb->q->payload, cb->aclhdr->len);
                hciacl_pbuf_input(cb->p);
                phybusif_reset(cb);
                HciServeIsrRequest();
                break ; /* Since there most likley won't be any more data in the input buffer */

            default:
                break;
        }
#endif
    }
}

void  phybusif_data_process_task(void *arg)
{
    BT_DEBUG();
    while(1)
    {
        rkos_semaphore_take(g_cb.hci_data_proc_sem, MAX_DELAY);
        //printf("if task\n");
        hci_serve();
        //rkos_delay(10);
    }
}

#endif
#endif

