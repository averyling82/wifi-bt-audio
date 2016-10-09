#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "app_cfg.h"
#include "debug_print.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include "lwip/inet.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "freertos_types.h"
#ifndef TRUE
#define TRUE    1
#endif
#ifndef FALSE
#define FALSE   0
#endif

#define TTCP_TX_STACKSIZE           256
#define TTCP_RX_STACKSIZE           64

#define TTCP_MODE_TRANSMIT  0
#define TTCP_MODE_RECEIVE       1
#define TX_SENT_LOOP            0
#define TX_TIMEOUT_CB           1
#define TX_SENT_CB              2

//#define TX_MODE                   TX_TIMEOUT_CB
#define TX_MODE                 TX_SENT_LOOP
#define USE_TIMER_MSG           1
#define MSG_TIME                (10000*configTICK_RATE_HZ/1000) // msec
#define USE_PAYLOAD         1
#define CHECK_RECEIVE_BUF       0
#define DESTROY_RECEIVE_TTCP    0

#define TTCP_TEST_RX_PRIO       17
#define TTCP_TEST_TX_PRIO       17

typedef int OS_ERR;
typedef int OS_MSG_SIZE;


#define timer_get_ms()          (xTaskGetTickCount()*(1000/configTICK_RATE_HZ))
typedef xTimerHandle    timer_handle;
typedef xSemaphoreHandle sema_handle;

typedef void (ttcp_done_cb_t)(void *opaque, int result);

struct ttcp
{
    /* options */
    struct ip_addr addr;    /* host */
    uint16_t port;          /* -p */
    uint32_t nbuf;          /* -n */
    uint32_t buflen;        /* -l */
    uint8_t mode;           /* -t */
    uint8_t verbose;        /* -v */
    uint8_t udp;            /* -u */
    uint8_t tos;            /* -u */

    /* common */
    uint16_t print_cnt;
    uint32_t start_time;
    uint32_t last_time;
    uint64_t left;
    uint64_t recved;
    uint64_t last_bytes;
    ttcp_done_cb_t* done_cb;
    void* opaque;
    uint32_t tid;
    timer_handle thdl;
    timer_handle thdl_msg;

    /* TCP specific */
    struct tcp_pcb* tpcb;
    struct tcp_pcb* lpcb;
    char* payload;
    sema_handle xSemaphore;

    /* UDP specific */
    int udp_started;
    uint16_t udp_end_marker_left;
    struct udp_pcb* upcb;
};

unsigned char ttcp_stop = 0;

unsigned long tx_buf_len = 0;
unsigned long tx_buf_num = 0;
struct ip_addr remote_ip_addr;

_os_task ttcp_rx_task_tcb;
_os_task  ttcp_tx_task_tcb;

unsigned int  ttcp_rx_task_stk[TTCP_RX_STACKSIZE];
unsigned int  ttcp_tx_task_stk[TTCP_TX_STACKSIZE];

#if configGENERATE_RUN_TIME_STATS
static char cBuffer[ 512 ];
#endif

#if CHECK_RECEIVE_BUF
int cnt = 0 ;
char c = 0;
static void
ttcp_check_buf(struct ttcp *ttcp, struct pbuf *p)
{
    char PBPreamble[] = "PCAUSA PCATTCP Pattern";   // 22 Bytes
    struct pbuf *q;
    char *cp;
    int i, len;

    DEBUGPRINT("\n\rTTCP: p->tot_len = %d\n", p->tot_len);

    for (q = p; q != NULL; q = q->next)
    {
        cp = q->payload;
        len = q->len;
//      DEBUGPRINT("\n\rTTCP [%p]: q->len = %d\n", ttcp, q->len);
//      for(i=0;i<q->len;i++)
//          printf("0x%02x, ", cp[i]);
        if(cnt == ttcp->buflen && cnt > 22)
        {
            if(freertos_memcpy( cp, PBPreamble, 22 ))
                DEBUGPRINT("\n\rTTCP: Preamble error !!\n");
            len -= 22;
            cnt -= 22;
            cp += 22;
        }
        while( cnt-- > 0 && len-- > 0)
        {
            while( !isprint((c&0x7F)) )
            {
                c++;
            }
            if(*cp++ != (c++&0x7F))
                DEBUGPRINT("\n\rTTCP: cp(0x%02x) != c(0x%02x), cnt = %d !!\n", *cp, c, cnt);
        }
        if(cnt == 0)
            cnt = ttcp->buflen;
    }
    DEBUGPRINT("\n\r\n");
}
#endif

static void
ttcp_print_stats(struct ttcp *ttcp, int done)
{
    int os_err;
    uint32_t ms;
    uint64_t bytes = ttcp->mode == TTCP_MODE_TRANSMIT ?  (((uint64_t)ttcp->nbuf * (uint64_t)ttcp->buflen) - ttcp->left) : ttcp->recved;

    if (ttcp->verbose)
        DEBUGPRINT("\n\r\n");
    if(done)
    {
        ms = timer_get_ms();
        ms = ms - ttcp->start_time;
        DEBUGPRINT("\n\rTTCP: total time %u ms\n", ms);
        //DEBUGPRINT("\n\rTTCP [%p]: %llu bytes processed, total %d ms, average speed: %lld.%lld KB/s (%s/%s)\n" , ttcp, bytes, ms, bytes / ms, bytes % ms, ttcp->udp ? "udp" : "tcp", ttcp->mode == TTCP_MODE_TRANSMIT ? "tx" : "rx");
        DEBUGPRINT("\n\rTTCP: %u bytes processed, total %u ms, average speed: %u.%u KB/s (%s/%s)\n" ,
                   (uint32_t)bytes, ms, (uint32_t)bytes / ms, (uint32_t)bytes % ms, ttcp->udp ? "udp" : "tcp", ttcp->mode == TTCP_MODE_TRANSMIT ? "tx" : "rx");
    }
    else
    {
        uint32_t last_time = timer_get_ms();
        uint64_t last_bytes = bytes;
        ms = last_time - ttcp->last_time;
        bytes = last_bytes - ttcp->last_bytes;
        //DEBUGPRINT("\n\rTTCP [%p]: %llu bytes processed, new ms = %d, now speed: %lld.%lld KB/s (%s/%s)\n" , ttcp, last_bytes, timer_get_ms(), bytes / ms, bytes % ms, ttcp->udp ? "udp" : "tcp", ttcp->mode == TTCP_MODE_TRANSMIT ? "tx" : "rx");
//#ifdef CONFIG_WIFI_STA_MODE
        DEBUGPRINT("\n\rTTCP: %u bytes processed, new ms = %u, now speed: %u.%u KB/s (%s/%s)\n" ,
                   (uint32_t)last_bytes, last_time, (uint32_t)bytes / ms, (uint32_t)bytes % ms, ttcp->udp ? "udp" : "tcp", ttcp->mode == TTCP_MODE_TRANSMIT ? "tx" : "rx");
//#endif
        ttcp->last_time = last_time;
        ttcp->last_bytes = last_bytes;
    }
    //DEBUGPRINT("\n\r\n\r[MEM] available heap %d\n\r", rkos_GetFreeHeapSize());
}


static void
ttcp_destroy(struct ttcp* ttcp)
{
    int err;

    if (ttcp->tpcb)
    {
        tcp_arg(ttcp->tpcb, NULL);
        tcp_sent(ttcp->tpcb, NULL);
        tcp_recv(ttcp->tpcb, NULL);
        tcp_err(ttcp->tpcb, NULL);
        tcp_close(ttcp->tpcb);
    }

    if (ttcp->lpcb)
    {
        tcp_arg(ttcp->lpcb, NULL);
        tcp_accept(ttcp->lpcb, NULL);
        tcp_close(ttcp->lpcb);
    }

    if (ttcp->upcb)
    {
        udp_disconnect(ttcp->upcb);
        udp_remove(ttcp->upcb);
    }

    if (ttcp->payload)
        rkos_memory_free(ttcp->payload);

#if (TX_MODE == TX_TIMEOUT_CB)
    if (ttcp->thdl)
        xTimerDelete(ttcp->thdl, 1000);
#endif

#if USE_TIMER_MSG
    if (ttcp->thdl_msg)
        xTimerDelete(ttcp->thdl_msg, 1000);
#endif

#if (TX_MODE == TX_SENT_LOOP)
    if (ttcp->xSemaphore)
        vSemaphoreDelete(ttcp->xSemaphore);
#endif

    rkos_memory_free(ttcp);
}


static void
ttcp_done(struct ttcp* ttcp, int result)
{
    OS_ERR err;

    if (result == 0)
        ttcp_print_stats(ttcp, 1);

    if (ttcp->done_cb)
        ttcp->done_cb(ttcp->opaque, result);

#if (TX_MODE == TX_TIMEOUT_CB)
    if (ttcp->thdl)
        xTimerStop(ttcp->thdl, 5000);
#endif

#if USE_TIMER_MSG
    if (ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        if (ttcp->thdl_msg)
            xTimerStop(ttcp->thdl_msg, 5000);
    }
#endif

    DEBUGPRINT("\n\rTTCP: ttcp_done !!!\n");
    if(ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        ttcp_destroy(ttcp);
    }
#if DESTROY_RECEIVE_TTCP
    if(ttcp->mode == TTCP_MODE_RECEIVE)  // Not destroy in TTCP_MODE_RECEIVE , it can accept next client to connect.
    {
        ttcp_destroy(ttcp);
    }
#endif
}


static void
tcp_timer_msg(timer_handle pxTimer)
{
    struct ttcp *ttcp = pvTimerGetTimerID( pxTimer );
    ttcp_print_stats(ttcp, 0);
#if configGENERATE_RUN_TIME_STATS
    vTaskGetRunTimeStats( ( signed char * ) cBuffer );
    printf( cBuffer );
#endif
}


static void
tcp_send_loop(struct ttcp *ttcp)
{
    err_t err;
    uint64_t len;
    uint32_t cnt = 0;
    OS_ERR os_err;

    DEBUGPRINT("\n\rTTCP: tcp_send_loop enter!\n");

    while(xSemaphoreTake(ttcp->xSemaphore, 30000/portTICK_RATE_MS) != pdTRUE)   // wait 30,000 ms
    {
        cnt++;
        DEBUGPRINT("\n\rTTCP: tcp_send_loop timeout count %d.\n", cnt);

    }

    if(ttcp->tpcb == NULL)
    {
        DEBUGPRINT("\n\rTTCP: tcp_send_loop connect error !!!\n");
        return;
    }

    while (ttcp->left && !ttcp_stop)
    {
        //if(tcp_sndbuf(ttcp->tpcb) == 0){
        if(tcp_sndbuf(ttcp->tpcb) <= 1500)
        {
            freertos_msleep(1);
            continue;
        }
        len = ttcp->left;
        /* don't send more than we have in the payload */
        if (len > ttcp->buflen)
            len = ttcp->buflen;
        /* We cannot send more data than space available in the send buffer. */
        if (len > tcp_sndbuf(ttcp->tpcb))
            len = tcp_sndbuf(ttcp->tpcb);
        do
        {
            err = tcp_write(ttcp->tpcb, ttcp->payload, len, 0);
            //if (err == ERR_MEM)
            //len /= 2;
            if (err == ERR_MEM)
                break;
        }
        while (err == ERR_MEM && len > 1 && !ttcp_stop);

        if (err == ERR_OK)
        {
            ttcp->left -= len;
        }
        else
        {
            DEBUGPRINT("\n\rTTCP: tcp_write failed\n");
            //delay_ms(1000);
            freertos_msleep(1);
        }
        if(ttcp->tpcb == NULL)
        {
            DEBUGPRINT("\n\rTTCP: tcp_send_loop exit because connection error !!!\n");
            return;
        }
    }

    cnt = 1000;
    while (cnt)
    {
        if (ttcp_stop)
        {
            break;
        }
        /* all sent - empty queue */
        if (ttcp->tpcb->snd_queuelen)
        {
            freertos_msleep(10);
        }
        else
        {
            break;
        }
        cnt--;
    }

    if (ttcp->tpcb->snd_queuelen)
    {
        DEBUGPRINT("\n\rTTCP: snd_queuelen is not empty !!!\n");
    }

    ttcp_done(ttcp, 0);
}

#if (TX_MODE == TX_TIMEOUT_CB || TX_MODE == TX_SENT_CB)
static void
tcp_send_data(struct ttcp *ttcp)
{
    OS_ERR err;
    uint64_t len;

    len = ttcp->left;
    /* don't send more than we have in the payload */
    if (len > ttcp->buflen)
        len = ttcp->buflen;

    /* We cannot send more data than space available in the send
        buffer. */
    if(tcp_sndbuf(ttcp->tpcb) == 0)
    {
        rkos_delay(1);
//      DEBUGPRINT("\n\rTTCP [%p]: tcp_write buf no space\n", ttcp);
#if (TX_MODE == TX_TIMEOUT_CB)
        xTimerStart(ttcp->thdl, 1);
#endif
        return;
    }
    if (len > tcp_sndbuf(ttcp->tpcb))
        len = tcp_sndbuf(ttcp->tpcb);

    do
    {
        err = tcp_write(ttcp->tpcb, ttcp->payload, len, 0);
        if (err == ERR_MEM)
            len /= 2;
    }
    while (err == ERR_MEM && len > 1);

    if (err == ERR_OK)
        ttcp->left -= len;
    else
        DEBUGPRINT("\n\rTTCP: tcp_write failed\n");
#if (TX_MODE == TX_TIMEOUT_CB)
    xTimerStart(ttcp->thdl, 1); // ttcp->tid = timer_sched_timeout_cb(0, TIMEOUT_ONESHOT, tcp_timeout_cb, ttcp);
#endif
}
#endif

#if (TX_MODE == TX_TIMEOUT_CB)

static void
tcp_timeout_cb(timer_handle pxTimer)
{
    struct ttcp *ttcp = pvTimerGetTimerID( pxTimer );

    if (ttcp->left > 0)
    {
        tcp_send_data(ttcp);
        if (ttcp->verbose)
        {
            if(0)
            {
                if (ttcp->print_cnt % 1000 == 0)
                    ttcp_print_stats(ttcp, 0);
            }
            else
            {
                DEBUGPRINT("\n\r.");
                if (ttcp->print_cnt % 80 == 0)
                    DEBUGPRINT("\n\r\n");
            }
            ttcp->print_cnt++;
        }
        return;
    }

    /* all sent - empty queue */
    if (ttcp->tpcb->snd_queuelen)
    {
        rkos_delay(10);
//      DEBUGPRINT("\n\rTTCP [%p]: snd queue not empty\n", ttcp);
        xTimerStart(ttcp->thdl, 1); // ttcp->tid = timer_sched_timeout_cb(0, TIMEOUT_ONESHOT, tcp_timeout_cb, ttcp);
    }
    else
    {
        ttcp_done(ttcp, 0);
    }
}

#endif

#if (TX_MODE == TX_SENT_CB)
static err_t
tcp_sent_cb(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    struct ttcp *ttcp = arg;

    if (ttcp->left > 0)
    {
        tcp_send_data(ttcp);
        if (ttcp->verbose)
        {
            DEBUGPRINT("\n\r.");
            if (ttcp->print_cnt % 80 == 0)
                DEBUGPRINT("\n\r\n");
            ttcp->print_cnt++;
        }
    }
    else if (pcb->snd_queuelen == 0)
    {
        ttcp_done(ttcp, 0);
    }
    return ERR_OK;
}
#endif  // TX_SENT_CB

static err_t
tcp_connect_cb(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    OS_ERR os_err;
    struct ttcp* ttcp = arg;
    DEBUGPRINT("\n\rTTCP: start time = %d , connect !!! \n", timer_get_ms());
    ttcp->left = (uint64_t)ttcp->nbuf * ttcp->buflen;
    ttcp->start_time = timer_get_ms();
    ttcp->last_time = ttcp->start_time;
    DEBUGPRINT("\n\rTTCP: start time = %d , connect !!! \n", ttcp->start_time);

#if (TX_MODE == TX_TIMEOUT_CB)
    tcp_send_data(ttcp);
#elif (TX_MODE == TX_SENT_CB)
    tcp_sent(tpcb, tcp_sent_cb);
    tcp_send_data(ttcp);
#elif (TX_MODE == TX_SENT_LOOP)
    xSemaphoreGive(ttcp->xSemaphore);
#endif
    return ERR_OK;
}


static void
tcp_conn_err_cb(void *arg, err_t err)
{
    OS_ERR    os_err;
    struct ttcp* ttcp = arg;

    DEBUGPRINT("\n\rTTCP: connection error !!!\n");

    ttcp->tpcb = NULL; /* free'd by lwip upon return */
#if (TX_MODE == TX_SENT_LOOP)
    if (ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        xSemaphoreGive(ttcp->xSemaphore);
    }
#endif
    ttcp_done(ttcp, err);
}


static err_t
tcp_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    struct ttcp* ttcp = arg;

    /* p will be NULL when remote end is done */
    if (p == NULL)
    {
        ttcp_done(ttcp, 0);
        return ERR_OK;
    }
    /* for print_stats() */
    ttcp->recved += p->tot_len;
    if (ttcp->verbose)
    {
        if(0)
        {
            if (ttcp->print_cnt % 1000 == 0)
                ttcp_print_stats(ttcp, 0);
        }
        else
        {
            DEBUGPRINT("\n\r.");
            if (ttcp->print_cnt % 80 == 0)
                DEBUGPRINT("\n\r\n");
        }
        ttcp->print_cnt++;
    }
#if CHECK_RECEIVE_BUF
    ttcp_check_buf(ttcp, p);
#endif
    tcp_recved(pcb, p->tot_len);
    pbuf_free(p);
    return ERR_OK;
}


static err_t
tcp_accept_cb(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    OS_ERR os_err;
    struct ttcp* ttcp = arg;

    ttcp->tpcb = newpcb;
    ttcp->recved = 0;
    ttcp->start_time = timer_get_ms();
    ttcp->last_time = ttcp->start_time;
    DEBUGPRINT("\n\rTTCP: start time = %d , accept !!!  \n", ttcp->start_time);

    tcp_recv(ttcp->tpcb, tcp_recv_cb);
    tcp_err(ttcp->tpcb, tcp_conn_err_cb);
    return ERR_OK;
}

static int
tcp_start(struct ttcp* ttcp)
{
    OS_ERR err;

    DEBUGPRINT("\n\rTTCP: tcp start enter!\n");

    ttcp->tpcb = tcp_new();
    if (ttcp->tpcb == NULL)
    {
        DEBUGPRINT("\n\rTTCP: could not allocate pcb\n");
        return -1;
    }
    // set tos, packet priority
    ttcp->tpcb->tos = ttcp->tos<<5;

#if USE_PAYLOAD
    ttcp->payload = (char *)freertos_malloc(ttcp->buflen);
    if (ttcp->payload == NULL)
    {
        DEBUGPRINT("\n\rTTCP: could not allocate payload\n");
        return -1;
    }
#endif

    tcp_arg(ttcp->tpcb,  ttcp);

#if USE_TIMER_MSG
    ttcp->thdl_msg = xTimerCreate( ( const signed char * ) "MsgTimer", MSG_TIME, pdTRUE, ttcp, tcp_timer_msg );
    xTimerStart(ttcp->thdl_msg, 1000);
#endif

    if (ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        tcp_err(ttcp->tpcb,  tcp_conn_err_cb);
        tcp_recv(ttcp->tpcb, tcp_recv_cb);
        if (tcp_connect(ttcp->tpcb, &ttcp->addr, ttcp->port, tcp_connect_cb) != ERR_OK)
        {
            DEBUGPRINT("\n\rTTCP: tcp connect failed\n");
            return -1;
        }
#if (TX_MODE == TX_TIMEOUT_CB)
        ttcp->thdl = xTimerCreate( ( const signed char * ) "SendTimer", 1, pdFALSE, ttcp, tcp_timeout_cb );
#elif (TX_MODE == TX_SENT_LOOP)
        vSemaphoreCreateBinary(ttcp->xSemaphore);
        xSemaphoreTake(ttcp->xSemaphore, 0);
        tcp_send_loop(ttcp);
#endif
    }
    else
    {
        tcp_bind(ttcp->tpcb, IP_ADDR_ANY, ttcp->port);
        ttcp->lpcb = tcp_listen(ttcp->tpcb);
        if (ttcp->lpcb == NULL)
        {
            DEBUGPRINT("\n\rTTCP: listen failed\n");
            return -1;
        }
        tcp_accept(ttcp->lpcb, tcp_accept_cb);
    }
    return 0;
}

#if (TX_MODE == TX_TIMEOUT_CB)
static void
udp_send_data(struct ttcp* ttcp);

static void udp_timeout_cb(timer_handle pxTimer)
{
    struct ttcp *ttcp = pvTimerGetTimerID( pxTimer );

    udp_send_data(ttcp);
}

#endif

static int
udp_send_bytes(struct ttcp* ttcp, uint32_t len)
{
    struct pbuf* p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (p == NULL)
    {
        DEBUGPRINT("\n\rTTCP: could not allocate pbuf\n");
        return -1;
    }

    if (udp_send(ttcp->upcb, p) != ERR_OK)
    {
        DEBUGPRINT("\n\rTTCP: udp_send() failed\n");
        pbuf_free(p);
        return -1;
    }

    pbuf_free(p);
    return 0;
}

static void
udp_send_data(struct ttcp* ttcp)
{
    OS_ERR os_err;

    /* send start marker first time */
    if (!ttcp->udp_started)
    {
        if (udp_send_bytes(ttcp, 4) == 0)
        {
            ttcp->udp_started = 1;
            ttcp->start_time = timer_get_ms();
            ttcp->last_time = ttcp->start_time;
        }
    }
    /* normal case */
    else if (ttcp->left)
    {
        /* send data */
        if (udp_send_bytes(ttcp, ttcp->buflen) == 0)
            ttcp->left -= ttcp->buflen;
    }
    /* end marker? */
    else if (ttcp->left == 0 && ttcp->udp_end_marker_left)
    {
        if (udp_send_bytes(ttcp, 4) == 0)
            ttcp->udp_end_marker_left--;
    }
    /* all end markers sent */
    else if (ttcp->left == 0)
    {
        ttcp_done(ttcp, 0);
        return;
    }
#if (TX_MODE == TX_TIMEOUT_CB)
    xTimerStart(ttcp->thdl, 1); // ttcp->tid = timer_sched_timeout_cb(0, TIMEOUT_ONESHOT, udp_timeout_cb, ttcp);
#endif
}

static void
udp_recv_cb(void *arg, struct udp_pcb *upcb, struct pbuf *p,
            struct ip_addr *addr, u16_t port)
{
    OS_ERR os_err;
    struct ttcp* ttcp = arg;

    /* got start marker? we might lose this so if we get it just reset
     * the timer
     */
    if (!ttcp->udp_started && p->tot_len <= 4)
    {
        ttcp->start_time = timer_get_ms();
        ttcp->last_time = ttcp->start_time;
        ttcp->udp_started = 1;
        goto out;
    }

    /* after receiving at least 1 byte, check end marker
     * don't check udp_started since we might have lost the start marker
     */
    if (ttcp->recved && p->tot_len <= 4)
    {
        ttcp_done(ttcp, 0);
        goto out;
    }

    /* for print_stats() */
    ttcp->recved += p->tot_len;
    if (ttcp->verbose)
    {
        DEBUGPRINT("\n\r.");
        if (ttcp->print_cnt % 80 == 0)
            DEBUGPRINT("\n\r\n");
        ttcp->print_cnt++;
    }

out:
    pbuf_free(p);
}


static int
udp_start(struct ttcp* ttcp)
{
    OS_ERR err;

    ttcp->udp_end_marker_left = 5;
    ttcp->upcb = udp_new();
    if (ttcp->upcb == NULL)
    {
        DEBUGPRINT("\n\rTTCP: could not allocate pcb\n");
        return -1;
    }

    if (ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        if (udp_connect(ttcp->upcb, &ttcp->addr, ttcp->port) != ERR_OK)
        {
            DEBUGPRINT("\n\rTTCP: udp connect failed\n");
            return -1;
        }
#if (TX_MODE == TX_TIMEOUT_CB)
        ttcp->thdl = xTimerCreate( ( const signed char * ) "SendTimer", 1, pdFALSE, ttcp, udp_timeout_cb );
#endif
        udp_send_data(ttcp);
    }
    else
    {
        udp_recv(ttcp->upcb, udp_recv_cb, ttcp);
    }

    return 0;
}


int
ttcp_start(struct ip_addr addr, uint16_t port, void *opaque,
           ttcp_done_cb_t *done_cb,
           uint8_t mode, uint32_t nbuf, uint32_t buflen, uint8_t udp, uint8_t verbose, uint8_t tos)
{
    struct ttcp* ttcp;
    int status;

    if (mode != TTCP_MODE_TRANSMIT && mode != TTCP_MODE_RECEIVE)
    {
        DEBUGPRINT("\n\rTTCP [-]: invalid mode\n");
        return -1;
    }

    if (nbuf == 0)
    {
        DEBUGPRINT("\n\rTTCP [-]: invalid nbuf\n");
        return -1;
    }

    if (buflen == 0)
    {
        DEBUGPRINT("\n\rTTCP [-]: invalid buflen\n");
        return -1;
    }

    ttcp = (struct ttcp* )freertos_malloc(sizeof(struct ttcp));
    if (ttcp == NULL)
    {
        DEBUGPRINT("\n\rTTCP [-]: could not allocate memory for ttcp\n");
        return -1;
    }
    memset(ttcp, 0, sizeof(struct ttcp));
    ttcp->addr = addr;
    ttcp->port = port;
    ttcp->nbuf = nbuf;
    ttcp->mode = mode;
    ttcp->left = (uint64_t)nbuf * buflen;
    ttcp->done_cb = done_cb;
    ttcp->opaque = opaque;
    ttcp->udp = udp;
    ttcp->verbose = verbose;
    ttcp->buflen = buflen;
    ttcp->tos = tos;
#if CHECK_RECEIVE_BUF
    cnt = ttcp->buflen;
#endif

    if(ttcp->mode == TTCP_MODE_TRANSMIT)
    {
        DEBUGPRINT("\n\rTTCP: port=%d, buflen=%d, nbuf=%d (%s/tx)\n",
                   ttcp->port, ttcp->buflen, ttcp->nbuf,
                   ttcp->udp ? "udp" : "tcp");
    }
    else
    {
        DEBUGPRINT("\n\rTTCP: port=%d, buflen=%d (%s/rx)\n",
                   ttcp->port, ttcp->buflen,
                   ttcp->udp ? "udp" : "tcp");
    }

    if (ttcp->udp)
        status = udp_start(ttcp);
    else
        status = tcp_start(ttcp);

    if (status)
        goto fail;

    return 0;

fail:
    ttcp_destroy(ttcp);
    return -1;
}

void ttcp_test_rx(void *param)
{
    uint8_t mode = TTCP_MODE_RECEIVE;
    uint8_t verbose = 0;
    uint32_t buflen = 1460; //1024;
    uint32_t nbuf = 1024;
    uint16_t port = 5001; //5002;
    uint8_t udp = 0;
    uint8_t tos = 0;
    struct ip_addr addr = { 0 };
    OS_ERR err;

    DEBUGPRINT("\n\rTTCP: rx thread created");

    ttcp_start(addr, port, NULL, NULL, mode, nbuf, buflen, udp, verbose, tos);

    DEBUGPRINT("\n\rTTCP: rx thread stoped");

    freertos_delete_task(&ttcp_rx_task_tcb);
}

void ttcp_test_tx(void *param)
{
    uint8_t mode = TTCP_MODE_TRANSMIT;
    uint16_t port = 5001; //5002;
    uint8_t verbose = 0;
    uint8_t udp = 0;
    uint8_t tos = 0;
    OS_ERR err;

    DEBUGPRINT("\n\rTTCP: tx thread created");

    ttcp_start(remote_ip_addr, port, NULL, NULL, mode, tx_buf_num, tx_buf_len, udp, verbose, tos);

    DEBUGPRINT("\n\rTTCP: tx thread stopped");

    freertos_delete_task(&ttcp_tx_task_tcb);
}

//  test_mode = 0: rx mode
//  test_mode = 1: tx mode
void ttcp_test(int test_mode)
{
    OS_ERR err;
#if 0
    if (test_mode == 0)
    {
        DEBUGPRINT("\n\rTTCP: ttcp_test_rx");
//      if(sys_thread_new("ttcp_test_rx", ttcp_test_rx, NULL, STACKSIZE, tskIDLE_PRIORITY + 1 ) == NULL)
        OSTaskCreate((OS_TCB     *)&ttcp_rx_task_tcb,
                     (CPU_CHAR   *)"ttcp_test_rx",
                     (OS_TASK_PTR ) ttcp_test_rx,
                     (void       *) 0,
                     (OS_PRIO     ) TTCP_TEST_RX_PRIO,
                     (CPU_STK    *)&ttcp_rx_task_stk[0],
                     (CPU_STK_SIZE) TTCP_RX_STACKSIZE / 50,
                     (CPU_STK_SIZE) TTCP_RX_STACKSIZE,
                     (OS_MSG_QTY  ) 16u,
                     (OS_TICK     ) 0u,
                     (void       *) 0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);
        if (err != OS_ERR_NONE)
        {
            DEBUGPRINT("\n\rTTCP: OSTaskCreate ttcp_test_rx failed!");
        }
    }
    else
    {
        DEBUGPRINT("\n\rTTCP: ttcp_test_tx");
//      if(sys_thread_new("ttcp_test_tx", ttcp_test_tx, NULL, STACKSIZE, tskIDLE_PRIORITY + 1 ) == NULL)

        OSTaskCreate((OS_TCB     *)&ttcp_tx_task_tcb,
                     (CPU_CHAR   *)"ttcp_test_tx",
                     (OS_TASK_PTR ) ttcp_test_tx,
                     (void       *) 0,
                     (OS_PRIO     ) TTCP_TEST_TX_PRIO,
                     (CPU_STK    *)&ttcp_tx_task_stk[0],
                     (CPU_STK_SIZE) TTCP_TX_STACKSIZE / 50,
                     (CPU_STK_SIZE) TTCP_TX_STACKSIZE,
                     (OS_MSG_QTY  ) 16u,
                     (OS_TICK     ) 0u,
                     (void       *) 0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);

        if (err != OS_ERR_NONE)
        {
            DEBUGPRINT("\n\rTTCP: freertos_create_task tcp_test_tx failed!");
        }
    }
#endif
    if (test_mode == 0)
    {
        DEBUGPRINT("\n\rTTCP: ttcp_test_rx");
        err = freertos_create_task(&ttcp_rx_task_tcb, (uint8 *)"ttcp_test_rx", TTCP_RX_STACKSIZE*32, 0, TTCP_TEST_RX_PRIO, ttcp_test_rx, NULL);
        if (err != pdPASS)
        {
            DEBUGPRINT("\n\rTTCP: freertos_create_task ttcp_test_rx failed!");
        }
    }
    else
    {
        DEBUGPRINT("\n\rTTCP: ttcp_test_tx");
        err = freertos_create_task(&ttcp_tx_task_tcb, (uint8 *)"ttcp_test_tx", TTCP_TX_STACKSIZE*16, 0, TTCP_TEST_TX_PRIO, ttcp_test_tx, NULL);
        if (err != pdPASS)
        {
            DEBUGPRINT("\n\rTTCP: freertos_create_task ttcp_test_tx failed!");
        }
    }
}

//--------------------------------------------------------
// PCATTCP.exe
// char usage[] = "Usage: PCATTCP -t/-r \n\
//         -l      length of bufs written to network (default 1024)\n\
//         -n      number of bufs written to network (default 1024)\n\
//         -p      port number to send to (default rx:5001, tx:5002)\n\
//         -u      udp\n\
//         -v      verbose\n\
//                 ip\n";
//--------------------------------------------------------
// ttcp
// receive mode  : ttcp r [length] , ex: ttcp r 1024 ; default port = 5001
// transmit mode : ttcp t [length] [number] [ip], ex: ttcp t 1024 2048 192.168.1.2 ; default port = 5002
// r + t mode    : ttcp rt [length] [number] [ip], ex: ttcp rt 1024 2048 192.168.1.2
//--------------------------------------------------------
void cmd_ttcp(int argc, char **argv)
{
    uint8_t mode = TTCP_MODE_RECEIVE;
    uint8_t udp = 0;
    uint8_t tos = 0;    // 0~7
    uint8_t verbose = 0;
    uint32_t buflen = 1460; //1024;
    uint32_t nbuf = 1024;
    uint16_t port = 5001; //5002;
    int ret = 0;
    struct ip_addr addr = { 0 };

    ttcp_stop = 0;

    if (strcmp(argv[1], "stop") == 0 && argc == 2)
    {
        ttcp_stop = 1;
    }
    else if (strcmp(argv[1], "r") == 0 && argc == 3)
    {
        // receive
        buflen = atoi(argv[2]);
        ret = ttcp_start(addr, port, NULL, NULL, mode, nbuf, buflen, udp, verbose, tos);
        //ttcp_test(0);
    }
    else if(strcmp(argv[1], "t") == 0 && argc == 5)
    {
        // transmit
        //mode = TTCP_MODE_TRANSMIT;
        remote_ip_addr.addr = inet_addr(argv[4]);
        if (remote_ip_addr.addr == 0)
        {
            DEBUGPRINT("\n\rTTCP: address type error!");
            return;
        }
        //port = 5001;
        tx_buf_len = atoi(argv[2]);
        tx_buf_num = atoi(argv[3]);
        printf("\n\rTTCP: remote ipaddr=%s,tx_buf_len=%d,tx_buf_num=%d\n\r", inet_ntoa(remote_ip_addr), tx_buf_len, tx_buf_num);
        //ret = ttcp_start(remote_ip_addr, port, NULL, NULL, mode, tx_buf_num, tx_buf_len, udp, verbose, tos);
        ttcp_test(1);
    }
    else if((strcmp(argv[1], "rt") == 0 || strcmp(argv[1], "tr") == 0) && argc == 5)
    {
        // receive
        buflen = atoi(argv[2]);
        ret = ttcp_start(addr, port, NULL, NULL, mode, nbuf, buflen, udp, verbose, tos);
        if(ret != 0)
            DEBUGPRINT("\n\rTTCP: ttcp_start rx fail!");

        // transmit
        //mode = TTCP_MODE_TRANSMIT;
        remote_ip_addr.addr = inet_addr(argv[4]);
        if (remote_ip_addr.addr == 0)
        {
            DEBUGPRINT("\n\rTTCP: address type error!");
            return;
        }
        //port = 5002;
        tx_buf_len = atoi(argv[2]);
        tx_buf_num = atoi(argv[3]);
        //ret = ttcp_start(remote_ip_addr, port, NULL, NULL, mode, tx_buf_num, tx_buf_len, udp, verbose, tos);
        ttcp_test(1);
    }
    else
    {
        DEBUGPRINT("\n\rTTCP: command format error!");
        return;
    }

    if(ret != 0)
        DEBUGPRINT("\n\rTTCP: ttcp_start fail!");
}
