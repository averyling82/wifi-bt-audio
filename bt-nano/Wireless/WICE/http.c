/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: WICE\http.c
* Owner: lyb
* Date: 2015.6.26
* Time: 15:21:16
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    lyb     2015.6.26     15:21:16   1.0
********************************************************************************************
*/


#include "BspConfig.h"

#ifdef __WICE_HTTP_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include <stdlib.h>
#include <stdio.h>
#include "lwipopts.h"
#include "def.h"
#include "tcp.h"
#include "netif.h"
#include "ip.h"
#include "pbuf.h"
#include "api.h"
#include "dns.h"
#include "etharp.h"
#include "ip_addr.h"
#include <string.h>
#include "err.h"
#include "http.h"
#include "wwd_constants.h"
#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

//typedef struct
#define  DNSRETRYNUM  5
extern wiced_bool_t                 wiced_sta_link_up;
HDC hFifo;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct __HTTPPCB
{
    uint8 *ready_buf;
    uint16 al_len;
    uint32 total_len;
    uint8 delete_flag;
    struct netconn *http_conn;
    HTTP_URL_CALLFUNCTION HTTP_Status_Callback;
    HTTP_WR_FUNCTION  HTTP_Write_function;
    int type_flag;
    uint32 httptaskid;
    uint32 httpobjectid;
}HTTPPCB;


typedef struct __HTTPUPPCB
{
   // uint8 *dataname;
    uint32 datalength;
   // uint8 *host;
    uint8 delete_flag;
    HTTP_UP_READFUNCTION HTT_UP_ReadDdata;
    HTTP_UP_CALLFUNCTION HTTP_UP_CallFunction;
    struct netconn *httpup_conn;
    uint32 httpuptaskid;
    uint32 httpupobjectid;
}HTTPUPPCB;


int HttpObjectID = 0;
int HttpupObjectId = 0;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
static void AtoAddr(char *s, char *proto, char *user, char *pwrd, char *host, unsigned int *port, char *file);
static uint8 * strstri(uint8 *s, uint8 *pattern);
static char* strrmvspace(char *dst, const char *src);
static uint32 Http_HdParam(uint8 * dst, uint8 blen, uint8 * src);
static rk_err_t Http_Com(HTTPPCB *httppcb,uint8 *host, uint8 *file, uint32 StartLength);
static uint32 Http_Response(uint8 * src);
static int Http_Get_Header_Len(char *data, int dlen);
static void GetFile(HTTPPCB * httppcb);
unsigned int atou(const char *s);
uint32 tcp_connnect(ip_addr_t *ripaddr, uint16 localPort, uint16 remoteport, struct netconn **conn);

INIT API void HttpPcb_Delete(HTTPPCB *p);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef __SSL_MbedTLS__

#define SERVER_NAME "192.168.1.101"
//#define SERVER_NAME "localhost"
#define SERVER_PORT "4433"
#define GET_REQUEST "GET / HTTP/1.0\r\n\r\n"
#define DEBUG_LEVEL 3

static void my_debug( void *ctx, int level, const char *file, int line, const char *str )
{
    ((void) level);

    printf("%s:%04d: %s", file, line, str );
}

/*******************************************************************************
** Name: Https_Test
** Input:void
** Return: void
** Owner:yanghong
** Date: 2015.10.28
** Time: 10:52:50
*******************************************************************************/
COMMON API void Https_Test(void)
{
    int ret, len;
    mbedtls_net_context server_fd;
    uint32_t flags;
    unsigned char buf[1024];
    const char *pers = "ssl_client1";
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif
    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_ctr_drbg_init( &ctr_drbg );
#if 1
    printf( "\n  0. Seeding the random number generator..." );
    mbedtls_entropy_init( &entropy );
    if ( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg,
                                        mbedtls_entropy_func,
                                        &entropy,
                                        (const unsigned char *) pers,
                                        strlen( pers ) ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret );
        goto exit;
    }
    printf( " ok\n");

    /*
     * 0. Initialize certificates
     */
    printf( "  0. Loading the CA root certificate ..." );
    ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) mbedtls_test_cas_pem,
                                  mbedtls_test_cas_pem_len );
    if ( ret < 0 )
    {
        printf( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
        goto exit;
    }
    printf( " ok (%d skipped)\n", ret );
#endif
    /*
     * 1. Start the connection
     */
    printf( "\n  1. Connecting to tcp/%s/%s...", SERVER_NAME, SERVER_PORT );
    if ( ( ret = mbedtls_net_connect( &server_fd, SERVER_NAME, SERVER_PORT, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
        goto exit;
    }
    printf( " ok\n");

    /*
     * 2. Setup stuff
     */
    printf( "  2. Setting up the SSL/TLS structure..." );
    if ( ( ret = mbedtls_ssl_config_defaults( &conf,
                 MBEDTLS_SSL_IS_CLIENT,
                 MBEDTLS_SSL_TRANSPORT_STREAM,
                 MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret );
        goto exit;
    }
    printf( " ok\n" );

    printf("\n*********Default ciphersuite:");
    for (int i = 0; conf.ciphersuite_list[0][i] != 0; ++i)
    {
        printf("%d, ", conf.ciphersuite_list[0][i]);
    }
    printf("***********\n");
#if 1
    /* 3. OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example
     */
    mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

    if ( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret );
        goto exit;
    }
    if ( ( ret = mbedtls_ssl_set_hostname( &ssl, "mbed TLS Server 1" ) ) != 0 )
    {
        printf( " failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret );
        goto exit;
    }
    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );
#endif

    /*
     * 4. Handshake
     */
    printf( "  4. Performing the SSL/TLS handshake..." );
    while ( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
            goto exit;
        }
    }
    printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    printf( "  5. Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if ( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];
        printf( " failed\n" );
        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );
        printf( "%s\n", vrfy_buf );
    }
    else
        printf( " ok\n" );

    /*
     * 6. Write the GET request
     */
    printf( "  6.> Write to server:" );
    len = sprintf( (char *) buf, GET_REQUEST );
    while ( ( ret = mbedtls_ssl_write( &ssl, buf, len ) ) <= 0 )
    {
        if ( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            printf( " failed\n  ! mbedtls_ssl_write returned %d\n\n", ret );
            goto exit;
        }
    }
    len = ret;
    printf( " %d bytes written\n\n%s", len, (char *) buf );

    /*
     * 7. Read the HTTP response
     */
    printf( "  7.< Read from server:" );
    do
    {
        len = sizeof( buf ) - 1;
        memset( buf, 0, sizeof( buf ) );
        ret = mbedtls_ssl_read( &ssl, buf, len );
        if ( ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
            continue;
        if ( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
            break;
        if ( ret < 0 )
        {
            printf( "failed\n  ! mbedtls_ssl_read returned %d\n\n", ret );
            break;
        }
        if ( ret == 0 )
        {
            printf( "\n\nEOF\n\n" );
            break;
        }
        len = ret;
        printf( " %d bytes read\n\n%s", len, (char *) buf );
    }
    while ( 1 );
    mbedtls_ssl_close_notify( &ssl );

exit:
#ifdef MBEDTLS_ERROR_C
    if ( ret != 0 )
    {
        char error_buf[100];
        mbedtls_strerror( ret, error_buf, 100 );
        printf("Last error was: %d - %s\n\n", ret, error_buf );
    }
#endif
    mbedtls_net_free( &server_fd );
    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    return;
}
#endif
#if 1
/*******************************************************************************
** Name: Http_Test
** Input:void
** Return: void
** Owner:lyb
** Date: 2015.8.14
** Time: 11:33:50
*******************************************************************************/
rk_err_t dlna_http_test(char *buf, uint16 write_len, uint32 mlen)
{
   // rk_printf("test");
    return RK_SUCCESS;
}
COMMON API void Http_Test(void)
{
    uint8 dwRet;
    void *httptestpcb = NULL;

    while(1)
    {
         printf("\n   http start********************\n\n");
         printf("\n   http start********************\n\n");

         httptestpcb = HttpPcb_New(NULL, dlna_http_test, 3);
         dwRet = HttpGet_Url(httptestpcb, "http://192.168.0.103/%E8%BF%99%E6%98%AF%E7%88%B1.MP3", 0);

         if(dwRet == RK_SUCCESS)
         {
             printf("\nhttp ok\n");
         }
         else
         {
             printf("\nhttp error\n");
         }

         printf("\n\n  http close********************\n\n");
         printf("\n\n  http close********************\n\n");

         //rkos_sleep(100);
         rkos_sleep(2000);
        Http_Close(httptestpcb);

         //rkos_sleep(1000);
        // rkos_sleep(2000);
    }
}
#endif
#if 0
COMMON API rk_err_t HttpTask_Get(uint32 len)
{

    HTTP_ASK_QUEUE HttpAskQueue;
    HTTP_RESP_QUEUE HttpRespQueue;

    memset(&HttpAskQueue, 0x00, sizeof(HTTP_ASK_QUEUE ));
    HttpAskQueue.len = len;
    printf("HttpAskQueue.len  = %d\n", HttpAskQueue.len);
    rkos_queue_send(gpstHTTPData->HTTPAskQueue, &HttpAskQueue, MAX_DELAY);
    rkos_queue_receive(gpstHTTPData->HTTPRespQueue, &HttpRespQueue, MAX_DELAY);
    if (HttpRespQueue.status == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}
#endif




/*******************************************************************************
** Name: HTTPTask_Enter
** Input:void
** Return: void
** Owner:lyb
** Date: 2015.6.26
** Time: 15:26:43
*******************************************************************************/
COMMON API void HTTPTask_Enter(HDC httppcb)
{
     GetFile((HTTPPCB*)httppcb);
}


/*******************************************************************************
** Name: Http_Close
** Input:void
** Return: void
** Owner:lyb
** Date: 2015.8.13
** Time: 20:58:57
*******************************************************************************/
COMMON API void Http_Close(void *httppcb)
{
    HTTPPCB *pcb = (HTTPPCB *)httppcb;
    uint32 task_id;
    uint32 object_id;

    if(pcb == NULL)
        return;

    rkos_enter_critical();
    pcb->delete_flag = 1;
    task_id = pcb->httptaskid;
    object_id = pcb->httpobjectid;
    rkos_exit_critical();

    rk_printf("httpclose");
    while(RKTaskFind(task_id, object_id) != NULL)
    {
        rkos_sleep(10);
    }

    HttpPcb_Delete(pcb);
    rk_printf("httpclose ok");
    return;
}

void Http_Init(void)
{

}


/*******************************************************************************
** Name: HttpGet_Url
** Input:uint8
** Return: int
** Owner:
** Date: 2015.6.24
** Time: 8:50:20
*******************************************************************************/
rk_err_t HttpGet_Url(void *mHttpPcb,uint8 *pburl, uint32 StartLength)
{
    uint8 proto[8];
    uint8 *bHost;
    uint8 *bFile;
    unsigned int port;
    uint8 localport;
    uint8 trycount;
    ip_addr_t stIp;
    int ResStatus;
    HTTPPCB *HttpPcb = (HTTPPCB *)mHttpPcb;

    if(!netif_default_is_up())
    {
        rk_print_string("wiced is down\n");
        return RK_ERROR;
    }

    bHost = rkos_memory_malloc(512);
    if(NULL == bHost)
    {
        rk_printf("http bHost malloc fail");
        return RK_ERROR;
    }

    bFile = rkos_memory_malloc(512);
    if(NULL == bFile)
    {
        rk_printf("http bFile malloc fail");
        rkos_memory_free(bHost);
        return RK_ERROR;
    }
    //printf("pburl=  %s\n", pburl);
    AtoAddr(pburl, proto, 0, 0, bHost, &port, bFile);   //get host, port and file
    if (strcmp(proto, "http") != 0)
    {
        rk_printf("not http message\n");
        rkos_memory_free(bHost);
        rkos_memory_free(bFile);
        return RK_ERROR;
    }

    stIp.addr = 0;
    for (trycount = 0; trycount < DNSRETRYNUM; trycount++)  //get ip by dns
    {
        if ( netconn_gethostbyname(bHost, &stIp) == 0)
        {
            rk_printf("dns host and ip:%s = %x", bHost, stIp.addr);
            break;
        }
        else
        {
            rk_printf("dns fail,dns host and ip:%s", bHost);
            etharp_init();
        }
    }
    if ((trycount >= DNSRETRYNUM) && (stIp.addr == 0))
    {
        rk_printf("DNS Error!!!");
        rkos_memory_free(bHost);
        rkos_memory_free(bFile);
        return RK_ERROR;
    }
    localport  = (localport + 1) % 256 + 4096;  //get local port
    trycount = 0;
    do
    {
        ResStatus = tcp_connnect(&stIp, localport, port, &(HttpPcb->http_conn));
        trycount++;
    }
    while (( ResStatus == RK_ERROR) && trycount < 2);

    if( ResStatus == RK_ERROR)
    {
        rk_printf("HTTP:tcp connet fail");
        rkos_memory_free(bHost);
        rkos_memory_free(bFile);
        return RK_ERROR;
    }

    rk_printf("tcp connect ok");
    ResStatus = Http_Com(HttpPcb, bHost, bFile, StartLength);
    if(ResStatus == 0)
    {
        rkos_memory_free(bHost);
        rkos_memory_free(bFile);
        return RK_SUCCESS;
    }
    else
    {
       rkos_memory_free(bHost);
       rkos_memory_free(bFile);
       netconn_delete(HttpPcb->http_conn);

       return RK_ERROR;
    }
}

static rk_err_t Http_Com(HTTPPCB *httppcb, uint8 *host, uint8 *file, uint32 StartLength)
{
    uint16 tx_len;
    struct pbuf *tcp_buf;
    uint16 rx_len = 0;;
    uint32 HttpHeader_len;
    uint8 pbuf[32];
    uint8 trycnt = 0;
    int Httpstatus = 0;
    int ResStatus = 0;
    err_t err;
    char *position_now = NULL;
    char *position_end = NULL;
    uint8 *data_buf= NULL;
    char *head_buf = NULL;
    char *head_buf2 = NULL;
    uint16 head_len = 0;
    uint16 head_len2 = 0;
    uint16 data_received = 0;

    head_buf = rkos_memory_malloc(1024);
    if(head_buf == NULL)
    {
        rk_printf("head_buf malloc fail");
        return RK_ERROR;
    }
    head_len = 1024;
    tx_len = sprintf(head_buf, "GET %s HTTP/1.0\r\n"
                     "Host: %s\r\n"
                     "User-Agent: RkNanocPlayer\r\n"
                     "Ultravox-transport-type: TCP\r\n"
                     "Accept: */*\r\n"
                     "Icy-MetaData: 1\r\n"
                     "Range: bytes=%d-\r\n"
                     "Connection: close\r\n"
                     "\r\n",file, host, StartLength);
    printf("head_buf = %s\n", head_buf);
    rk_printf(" tx_len = %d", tx_len);
    netconn_write(httppcb->http_conn, head_buf, tx_len, NETCONN_COPY);  //set message to recive file
    rkos_memory_free(head_buf);

    /*get data header information*/
    while (1)
    {
        err = netconn_recv_tcp_pbuf(httppcb->http_conn, &tcp_buf);
        if(err != ERR_OK )
        {
            rk_printf("netconn_recv_tcp_pbuf fail");
            return RK_ERROR;
        }
        rx_len = tcp_buf->tot_len;
        head_len = rx_len;
        if (rx_len > 0)
        {
            head_buf = rkos_memory_malloc(head_len);
            if(head_buf == NULL)
            {
                rk_printf("rx head malloc fail\n");
                return RK_ERROR;
            }

            pbuf_copy_partial(tcp_buf, head_buf, head_len, 0);
            pbuf_free(tcp_buf);
           // printf(" head_buf = %s\n", head_buf);
            Httpstatus = Http_Response(head_buf);
            rk_printf(" l = %d  Httpstatus = %d",rx_len, Httpstatus);
            if(Httpstatus == 302)  //??¨￠??¨??ê???D???¨¨? url
            {
                 char *buf = NULL;

                 buf = rkos_memory_malloc(rx_len);
                 if(buf == NULL)
                 {
                    rk_printf("http buf mallco fail");
                    rkos_memory_free(head_buf);
                    return RK_ERROR;
                 }
                 if(httppcb->http_conn != NULL)
                 {
                     netconn_delete(httppcb->http_conn);
                     httppcb->http_conn = NULL;
                 }
                 position_now = strstr(head_buf, "Location:");
                 if(position_now == NULL)
                 {
                    position_now = strstr(head_buf, "LOCATION:");
                 }

                 position_now = strstr(position_now, "http:");
                 if(position_now == NULL)
                 {
                    position_now = strstr(position_now, "HTTP:");
                 }
                 position_end = strstr(position_now,  "Cache-Control:");
                 if(position_end != NULL)
                    position_end = '\0';

                 rkos_memcpy(buf,  position_now, strlen(position_now));
                 rkos_memory_free(head_buf);
                 printf(" buf = %s\n", buf);
                 if(HttpGet_Url(httppcb, buf, 0) == RK_SUCCESS)
                 {
                    rkos_memory_free(buf);
                    return RK_SUCCESS;
                 }
                 rkos_memory_free(buf);
                 return RK_ERROR;
            }
            else if ((Httpstatus != 200) && (Httpstatus != 206))
            {
               rk_printf("http status error");
               rkos_memory_free(head_buf);
               return RK_ERROR;
            }

            break;
        }
        else
        {
            pbuf_free(tcp_buf);
        }

    }

   // printf("head_buf22222 = %s\n", head_buf);
    trycnt = 0;
    while (1)
    {
        if (Http_HdParam(pbuf, 31, head_buf) == 0)
        {
            httppcb->total_len= atoi(pbuf);
        }

        HttpHeader_len= Http_Get_Header_Len(head_buf, head_len);
        if (HttpHeader_len)
        {
            //rk_printf("head_len = %d, httpHead_len = %d", head_len, HttpHeader_len);
            data_received = head_len - HttpHeader_len; //already recieve data
            break;
        }
        rx_len = 0;
        err = netconn_recv_tcp_pbuf(httppcb->http_conn, &tcp_buf);// ......
        if(err != ERR_OK )
        {
            rkos_memory_free(head_buf);
            return RK_ERROR;
        }
        else
        {
            rx_len = tcp_buf->tot_len;
            if(rx_len > 0)
            {
                rk_printf("already");
                head_buf2 = rkos_memory_malloc(head_len);
                if(head_buf2 == NULL)
                {
                    rk_printf("head_buf2 malloc fail");
                    rkos_memory_free(head_buf);
                    pbuf_free(tcp_buf);
                    return RK_ERROR;
                }
                rkos_memcpy(head_buf2, head_buf, head_len);
                rkos_memory_free(head_buf);
                head_len2 = head_len;

                head_len += rx_len;
                head_buf = rkos_memory_malloc(head_len);
                if(head_buf == NULL)
                {
                    rk_printf("http_ele.headbuf mallco fail22");
                    rkos_memory_free(head_buf2);
                    pbuf_free(tcp_buf);
                    return RK_ERROR;
                }

                rkos_memcpy(head_buf, head_buf2, head_len2);
                pbuf_copy_partial(tcp_buf, head_buf+head_len2, rx_len, 0);
                pbuf_free(tcp_buf);
                rkos_memory_free(head_buf2);
            }
            else
            {
                pbuf_free(tcp_buf);
            }
        }

        trycnt++;
        if(trycnt > DNSRETRYNUM)
        {
            rk_printf("trycnt fail");
            rkos_memory_free(head_buf);
            return RK_ERROR;
        }
    }

    //rk_printf("total = %d", httppcb->total_len);
    if(httppcb->total_len == 0)
    {
        rkos_memory_free(head_buf);
        return RK_ERROR;
    }

    httppcb->al_len = data_received;
    if(httppcb->al_len > 0)
    {
        if(httppcb->al_len > 65536)
        {
            rk_printf("httppcb->al_lenl error = %d", httppcb->al_len);
            rkos_memory_free(head_buf);
            return RK_ERROR;
        }

        httppcb->ready_buf = rkos_memory_malloc(httppcb->al_len);
        if(httppcb->ready_buf == NULL)
        {
            rk_printf("httppcb ready_buf malloc fail = %d", httppcb->al_len);
            rkos_memory_free(head_buf);
            return RK_ERROR;
        }
        rkos_memcpy(httppcb->ready_buf, head_buf+HttpHeader_len, httppcb->al_len);
    }
    rkos_memory_free(head_buf);
    //rk_printf("total11 = %d", httppcb->total_len);
    httppcb->httptaskid = TASK_ID_HTTP;
    httppcb->httpobjectid = HttpObjectID;

    if(RKTaskCreate(TASK_ID_HTTP, HttpObjectID, httppcb, SYNC_MODE) == RK_SUCCESS)
    {
        HttpObjectID++;
        return RK_SUCCESS;
    }
    else
    {
        if(httppcb->ready_buf != NULL)
        {
            rkos_memory_free(httppcb->ready_buf);
            httppcb->ready_buf = NULL;
        }

        return RK_ERROR;
    }
}

static void GetFile(HTTPPCB * httppcb)
{
    err_t err = -1;
    uint32 n=0;
    uint32 leavelength;
    uint32 tot_len = 0;
    uint32 fifo_len = 0;
    uint8 *fifo_write = NULL;
    uint32 fifo_memlen = 0;
    int ret;
    struct pbuf *tcp_buf = NULL;

    uint32 task_id = httppcb->httptaskid;
    uint32 object_id = httppcb->httpobjectid;

    if(httppcb->type_flag == FIFOWRITE)
    {
        hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
        if ((hFifo == NULL) || (hFifo == (HDC)RK_ERROR) || (hFifo == (HDC)RK_PARA_ERR))
        {
            rk_print_string("fifo device open failure");
            err = FIFO_OPEN_ERR;
            if(httppcb->al_len > 0)
            {
              rkos_memory_free(httppcb->ready_buf);
            }
            goto OVER;

           // return;
        }

        fifoDev_SetTotalSize(hFifo, 0);
        fifoDev_SetTotalSize(hFifo, httppcb->total_len);
    }


    if(httppcb->al_len == 0)
    {
        leavelength = httppcb->total_len;
        tot_len = 0;

    }
    else
    {
        leavelength = httppcb->total_len -  httppcb->al_len;
        tot_len += httppcb->al_len;
        httppcb->HTTP_Write_function(httppcb->ready_buf, httppcb->al_len, leavelength);
        rkos_memory_free(httppcb->ready_buf);
    }

    //rkos_memory_malloc_display(1);
    do
    {
        if(httppcb->delete_flag == 1)
        {
            err = DELEE_FLAG;
            break;
        }
        //rk_printf("g");
        err = netconn_recv_tcp_pbuf(httppcb->http_conn, &tcp_buf);
        if(err == ERR_RST || err == ERR_MEM)
        {
            rk_printf("err_rst");
            if(tcp_buf != NULL)
            {
                pbuf_free(tcp_buf);
            }
            continue;
        }

        n++;
        if(n%100 == 0)
        {
            //rk_printf("n = %d", n);
        }

        if (err == ERR_OK)
        {
            if(tcp_buf->tot_len > 0)
            {
                //leavelength = leavelength - tcp_buf->tot_len;
                if(fifo_memlen < tcp_buf->tot_len)
                {
                    if(fifo_write != NULL)
                    {
                        rkos_memory_free(fifo_write);
                        fifo_write = NULL;
                    }

                    fifo_write = rkos_memory_malloc(tcp_buf->tot_len);
                    if(fifo_write == NULL)
                    {
                        rk_printf("fifo_write malloc fail");
                        pbuf_free(tcp_buf);
                        err = MALLOC_ERR;
                        goto OVER;
                    }
                    fifo_memlen = tcp_buf->tot_len;
                }

                fifo_len = tcp_buf->tot_len;
                pbuf_copy_partial(tcp_buf, fifo_write, fifo_len, 0);
                pbuf_free(tcp_buf);

                leavelength = leavelength - fifo_len;
                tot_len += fifo_len;
                ret = httppcb->HTTP_Write_function(fifo_write, fifo_len, leavelength);
                if(ret == RK_ERROR)
                {
                    err = FIFO_WRITE_ERR;
                    rkos_memory_free(fifo_write);
                    fifo_write = NULL;
                    goto OVER;
                }
            }
            else
            {
                pbuf_free(tcp_buf);
            }
        }
        else
        {
            if(tcp_buf != NULL)
            {
                pbuf_free(tcp_buf);
            }
        }
        //rkos_memory_malloc_display(0);
    }while ((err == ERR_OK) && (leavelength > 0));
    rk_printf("http get ok = %d = %d", err, tot_len);
    netconn_close(httppcb->http_conn);
    netconn_delete(httppcb->http_conn);

    if(fifo_write != NULL)
    {
      rkos_memory_free(fifo_write);
    }

    if(httppcb->type_flag == FIFOWRITE)
    {
       RKDev_Close(hFifo);
    }

    if( httppcb->HTTP_Status_Callback != NULL)
       httppcb->HTTP_Status_Callback(err, httppcb);

    RKTaskDelete(task_id, object_id, ASYNC_MODE);

    while(1)
    {
        rkos_sleep(1000);
        rk_printf(" OK HTTP task delete ASYNC_MODE");
    }
OVER:

    //rk_printf("err2222= %d, Http_Ele.running = %d", err, Http_Ele.running);
    rk_printf("http get error = %d = %d", err, tot_len);
    netconn_close(httppcb->http_conn);
    netconn_delete(httppcb->http_conn);

    if(httppcb->type_flag == FIFOWRITE)
    {
       RKDev_Close(hFifo);
    }

    if( httppcb->HTTP_Status_Callback != NULL)
      httppcb->HTTP_Status_Callback(err, httppcb);

    RKTaskDelete(task_id, object_id, ASYNC_MODE);

    while(1)
    {
        rkos_sleep(1000);
        rk_printf(" ERR HTTP task delete ASYNC_MODE");
    }

}

uint32 tcp_connnect(ip_addr_t *ripaddr, uint16 localPort, uint16 remoteport, struct netconn **conn)
{

    err_t err;
    struct ip_addr *l_ipaddr;
    l_ipaddr = ripaddr;


    if (remoteport == 0)
        remoteport = 80;


    *conn = netconn_new(NETCONN_TCP);
    if (*conn == NULL)
    {
        rk_printf(" netconn_new alloc fail");
        return RK_ERROR;
    }

    //netconn_bind(*conn, IP_ADDR_ANY, localPort);
    //netconn_listen(conn);
    err = netconn_connect(*conn, l_ipaddr, remoteport);
    if (err == ERR_OK)
    {
        (*conn)->recv_timeout = 100000;
        (*conn)->send_timeout = 100000;
        return RK_SUCCESS;
    }
    else
    {
        netconn_delete(*conn);
        *conn = NULL;
        return RK_ERROR;
    }
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


static void AtoAddr(char *s, char *proto, char *user, char *pwrd, char *host, unsigned int *port, char *file)
{
    /*
    for example:s = "http://api.xbd61.com/ApiForM/TjMusicList.php?Apiadmin=xiaobuding"
    */
    if (proto)
    {
        *proto = 0;
    }
    if (user)
    {
        *user  = 0;
    }
    if (pwrd)
    {
        *pwrd  = 0;
    }
    if (host)
    {
        *host  = 0;
    }
    if (port)
    {
        *port  = 0;
    }
    if (file)
    {
        *file++ = '/';
        *file = 0;
    }

    while (*s == ' ')
    {
        s++;    //skip spaces
    }

    //get proto "http://"
    if (StrCmpA(s, "http://", 7) == 0)
    {
        s += 7;
        if (proto)
        {
            strcpy(proto, "http");
        }
    }
    else
    {
        return;
    }

    //get user & password
    if (strstr(s, "@") != 0)//find string "@" form string "s",
    {
        while (*s && (*s!=':'))
        {
            if (user)
            {
                *user++ = *s;
                *user   = 0;
            }
            s++;
        }
        s++; //skip ":"
        while (*s && (*s!='@'))
        {
            if (pwrd)
            {
                *pwrd++ = *s;
                *pwrd   = 0;
            }
            s++;
        }
        s++; //skip "@"
    }

    //get host:"api.xbd61.com"
    while (*s && (*s!=':') && (*s!='/'))
    {
        if (host)
        {
            *host++ = *s;
            *host   = 0;
        }
        s++;
    }

    //get port.the part:"80"
    if (*s == ':')
    {
        s++; //skip ":"
        if (port)
        {
            *port = atoi(s);
        }
        while (isdigit(*s))//skip port
        {
            s++;
        };
    }

    //get file:"ApiForM/TjMusicList.php?Apiadmin=xiaobuding"
    if (*s == '/')// "/ApiForM/TjMusicList.php?Apiadmin=xiaobuding"
    {
        s++; //skip "/"
        while (*s && !isspace(*s))
        {
            if (file)
            {
                *file++ = *s;
                *file   = 0;
            }
            s++;
        }
    }
    return;
}

static int Http_Get_Header_Len(char *data, int dlen)
{
    int i;

    if ((data == NULL) || (dlen < 4))
    {
        return 0;
    }

    for (i = 0; i < dlen; i++)
    {
        if ((data[i+0] == '\r') && (data[i+1] == '\n') &&
                (data[i+2] == '\r') && (data[i+3] == '\n'))
        {
            return (i + 4);
        }
    }

    return 0;
}
static uint32 Http_Response(uint8 * src)
{
    unsigned int search = 16;

    /* skip spaces*/
    while ((*src==' ') && search)
    {
        src++;
        search--;
    }
    if ((StrCmpA(src, "ICY", 3)  == 0) ||
            (StrCmpA(src, "HTTP", 4) == 0) ||
            (StrCmpA(src, "RTSP", 4) == 0))
    {
        /* skip proto name*/
        while (*src && (*src!=' ') && search)
        {
            src++;
            search--;
        }
        /* skip spaces*/
        while ((*src==' ') && search)
        {
            src++;
            search--;
        }
        if (search)
        {
            return atoi(src);
        }
    }

    return 0;
}


static uint32 Http_HdParam(uint8 * dst, uint8 blen, uint8 * src)
{
    char *ptr;
    uint8 *param = "Content-Length:";
    uint8 *param1 = "CONTENT-LENGTH:";
    uint8 *src1,*src2;
    ptr = dst; //save dst

    src1 = strstri(src, param);
    if (src1)
    {
        src1 += strlen(param);
        for (; blen != 0; blen--)
        {
            if ((*src1==0) || (*src1=='\n') || (*src1=='\r'))
            {
                break;
            }
            *dst++ = *src1++;
        }
        *dst = 0;
        //remove space at start and end of string
        strrmvspace(ptr, ptr);
        if (strlen(ptr))
        {
            return RETURN_OK;
        }
    }
    else
    {
        src2 = strstri(src, param1);
        if (src2)
        {
            src2 += strlen(param1);
            for (; blen != 0; blen--)
            {
                if ((*src2==0) || (*src2=='\n') || (*src2=='\r'))
                {
                    break;
                }
                *dst++ = *src2++;
            }
            *dst = 0;
            strrmvspace(ptr, ptr);
            if (strlen(ptr))
            {
                return RETURN_OK;
            }
        }
    }

    return RETURN_FAIL;
}


static uint8 * strstri(uint8 *s, uint8 *pattern)
{
    size_t len;

    if (*pattern)
    {
        len = strlen(pattern);
        while (*s)
        {
            if (StrCmpA(s, pattern, len) == 0)
            {
                return s;
            }
            s++;
        }
    }

    return 0;
}


static char* strrmvspace(char *dst, const char *src)//remove the space at the end of head and tail...
{
    unsigned int i;

    if (*src == 0)
    {
        *dst = 0;
    }
    else
    {
        //at start
        for (i=0; isspace(src[i]); i++);
        strcpy(dst, &src[i]);

        //at end
        i=strlen(dst)-1;
        for (i=strlen(dst)-1; isspace(dst[i]); i--)
        {
            dst[i] = 0;
        }
    }

    return dst;
}


unsigned int atou(const char *s)
{
    unsigned int value=0;

    if (!s)
    {
        return 0;
    }

    while (*s)
    {
        if ((*s >= '0') && (*s <= '9'))
        {
            value = (value*10) + (*s-'0');
        }
        else
        {
            break;
        }
        s++;
    }

    return value;
}


HDC HttpPcb_New(HTTP_URL_CALLFUNCTION functioncallback, HTTP_WR_FUNCTION write_function, int flag)
{
    HTTPPCB *HttpPcb_new = NULL;

    HttpPcb_new = rkos_memory_malloc(sizeof(HTTPPCB));
    if(NULL == HttpPcb_new)
    {
        return NULL;
    }

    //rkos_memset(HttpPcb_new, 0, sizeof(HTTPPCB));

    HttpPcb_new->HTTP_Status_Callback = functioncallback;
    HttpPcb_new->HTTP_Write_function = write_function;
    HttpPcb_new->type_flag = flag;
    HttpPcb_new->delete_flag = 0;
    HttpPcb_new->al_len = 0;
    HttpPcb_new->http_conn = NULL;
    HttpPcb_new->ready_buf = NULL;
    HttpPcb_new->total_len = 0;

    return (HDC)HttpPcb_new;

}

INIT API void HttpPcb_Delete(HTTPPCB *p)
{
    if(p != NULL)
    {

        rkos_memory_free(p);
        p = NULL;
    }
    return;
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: HTTPTask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:lyb
** Date: 2015.6.26
** Time: 15:26:43
*******************************************************************************/
INIT API rk_err_t HTTPTask_DeInit(void *pvParameters)
{
#if 0
    if(Http_Ele.http_conn != NULL)
    {
       //rk_printf("deletehttpele");
       //rkos_sleep(1);
       if(Http_Ele.delete_flag == 1)
       {
           rkos_semaphore_take(HttpRec_Sem, MAX_DELAY);
       }
       netconn_shutdown(Http_Ele.http_conn, 1, 0);
       netconn_close(Http_Ele.http_conn);
       netconn_delete(Http_Ele.http_conn);
       Http_Ele.http_conn = NULL;
    }
#if HTTP_TEST
    //rk_printf("httptaskdeinit start");
    //rkos_sleep(1);
    if(Http_Ele.delete_flag == 1)
    {
        rkos_semaphore_take(HttpFifo_Sem, MAX_DELAY);
    }
#endif


    rkos_sleep(100);
    rk_printf("httptaskdeinit\r\n");

#endif
   return RK_SUCCESS;
}
/*******************************************************************************
** Name: HTTPTask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:lyb
** Date: 2015.6.26
** Time: 15:26:43
*******************************************************************************/
INIT API rk_err_t HTTPTask_Init(void *pvParameters, void *arg)
{
#if 0
    rk_printf("HTTPTask_Init");
#if HTTP_TEST
    HttpFifo_Sem = rkos_semaphore_create(1, 1);
    if(HttpFifo_Sem == NULL)
    {
        rk_printf("HttpFifosem create fail");
    }
#endif
    HttpRec_Sem = rkos_semaphore_create(1, 1);
    if(HttpRec_Sem == NULL)
    {
        rk_printf("HttpFifo_Sem create fail");
    }

 #endif
 return RK_SUCCESS;
}



HDC HttpUpPcb_New(HTTP_UP_READFUNCTION DataUpFunction, HTTP_UP_CALLFUNCTION HttpUp_CallFunction)
{
    HTTPUPPCB *HttpUp_Pcb = NULL;
    uint16 len;

    HttpUp_Pcb = rkos_memory_malloc(sizeof(HTTPUPPCB));
    if(HttpUp_Pcb == NULL)
    {
        return NULL;
    }

    HttpUp_Pcb->HTT_UP_ReadDdata = DataUpFunction;
    HttpUp_Pcb->datalength = 0;
    //HttpUp_Pcb->dataname = NULL;
    HttpUp_Pcb->delete_flag = 0;
   // HttpUp_Pcb->host = NULL;
    HttpUp_Pcb->HTTP_UP_CallFunction = HttpUp_CallFunction;
    HttpUp_Pcb->httpup_conn = NULL;

    return (void *)HttpUp_Pcb;
}



void HttpUp_Delete(HTTPUPPCB *httpup_pcb)
{
    if(httpup_pcb != NULL)
    {
        rkos_memory_free(httpup_pcb);
        httpup_pcb = NULL;
    }

    return;
}


void HttpUp_Close(void *httpup_pcb)
{
    HTTPUPPCB *pcb = (HTTPUPPCB *)httpup_pcb;
    uint32 task_id;
    uint32 object_id;

    if(pcb == NULL)
    {
        return;
    }
    rkos_enter_critical();
    pcb->delete_flag = 1;
    task_id = pcb->httpuptaskid;
    object_id = pcb->httpupobjectid;
    rkos_exit_critical();

    rk_printf("httpupclose");
    while(RKTaskFind(task_id, object_id) != NULL)
    {
        rkos_sleep(10);
    }

    HttpUp_Delete(pcb);
    rk_printf("httpupclose ok");
    return;
}


uint32 tcpup_connnect(ip_addr_t *ripaddr, uint16 localPort, uint32 remoteport, struct netconn **conn)
{

    err_t err;
    struct ip_addr *l_ipaddr;
    l_ipaddr = ripaddr;

    *conn = netconn_new(NETCONN_TCP);
    if (*conn == NULL)
    {
        rk_printf(" netconn_new alloc fail");
        return RK_ERROR;
    }

    err = netconn_connect(*conn, l_ipaddr, remoteport);
    if (err == ERR_OK)
    {
        (*conn)->recv_timeout = 10000;
        (*conn)->send_timeout = 10000;
        return RK_SUCCESS;
    }
    else
    {
        netconn_delete(*conn);
        *conn = NULL;
        return RK_ERROR;
    }
}


rk_err_t HttpPost_Connect(HTTPUPPCB *mpcb, uint8 *host, uint32 port)
{
    uint8 trycount;
    ip_addr_t stIp;
    rk_err_t ResStatus;


    stIp.addr = 0;
    for (trycount = 0; trycount < DNSRETRYNUM; trycount++)  //get ip by dns
    {
        if ( netconn_gethostbyname(host, &stIp) == 0)
        {
            rk_printf("up dns host and ip:%s = %x", host, stIp.addr);
            break;
        }
        else
        {
            rk_printf("up fail,dns host and ip:%s", host);
            etharp_init();
        }
    }
    if ((trycount >= DNSRETRYNUM) && (stIp.addr == 0))
    {
        rk_printf("DNS Error!!!");
        return RK_ERROR;
    }

    trycount = 0;
    do
    {
        ResStatus = tcpup_connnect(&stIp, 0, port, &(mpcb->httpup_conn));
        trycount++;
    }
    while (( ResStatus == RK_ERROR) && trycount < 2);
    if( ResStatus == RK_ERROR)
    {
        rk_printf("HTTPup:tcp connet fail");
        return RK_ERROR;
    }

    return RK_SUCCESS;

}

rk_err_t HttpPost_up(void *mhttpup_pcb, uint8 *host, uint32 port, uint8 *serverpath, uint8 *name, uint32 datalength)
{
    HTTPUPPCB *httpup_pcb = (HTTPUPPCB *)mhttpup_pcb;
    rk_err_t res;
    uint16 tx_len;
    uint8 *head_buf = NULL;
    uint16 head_len = 0;
    struct pbuf *tcp_buf;
    int err;
    int Httpstatus = 0;
    //char *boundary = "abcedfg";


    res = HttpPost_Connect(httpup_pcb, host, port);
    if(res == RK_ERROR)
    {
        return RK_ERROR;
    }

    head_buf = rkos_memory_malloc(1024);
    if(head_buf == NULL)
    {
        rk_printf("post head buf malloc fail");
        netconn_delete(httpup_pcb->httpup_conn);
        return RK_ERROR;
    }

    tx_len = sprintf(head_buf, "POST %s HTTP/1.1\r\n"
                     "Host:%s\r\n"
                     "User-Agent:RkNanocPlayer\r\n"
                     "Accept: */*\r\n"
                     "Cache-Control:no-cache\r\n"
                     "Content-Length:%d\r\n"
                     //"Content-Disposition:form-data\r\n"
                     "Filename=%s\r\n"
                     //"Expect:100-continue\r\n"
                     "Accept-Encoding: gzip, deflate\r\n"
                    // "Content-Type:multipart/form-data; boundary=\"9d832598-abbb-4e2a-bb5c-a99aed0f8733\" \r\n"
                     "Content-Type:multipart/form-data; boundary=\"9d832598-abbb-4e2a\" \r\n"
                     "Connection:Keep-Alive\r\n"
                     "\r\n",serverpath, host, datalength, name);
    printf("up head_buf11 = %s\n, httpup_conn = %x", head_buf,httpup_pcb->httpup_conn);
    netconn_write(httpup_pcb->httpup_conn, head_buf, tx_len, NETCONN_COPY);
    rkos_memory_free(head_buf);

    httpup_pcb->datalength = datalength;
    httpup_pcb->httpuptaskid = TASK_ID_HTTP_UP;
    httpup_pcb->httpupobjectid = HttpupObjectId;

    if(RKTaskCreate(TASK_ID_HTTP_UP, HttpupObjectId, httpup_pcb, SYNC_MODE) == RK_SUCCESS)
    {
        HttpupObjectId++;
        return RK_SUCCESS;
    }
    else
    {
        netconn_delete(httpup_pcb->httpup_conn);
        return RK_ERROR;
    }

}


COMMON API void HTTPTaskUp_Enter(HDC mpcb)
{
     uint8 *read_buf;
     uint16 read_len = 2048;
     uint16 tx_len = 0;
     struct pbuf *tcp_buf;
     int err = ERR_OK;
     uint8 *head_buf = NULL;
     uint16 head_len;
     rk_err_t res;
     // int n=0;
     HTTPUPPCB *httpup_pcb = (HTTPUPPCB*)mpcb;
     uint32 left_len = httpup_pcb->datalength;
     uint32 taskid = httpup_pcb->httpuptaskid;
     uint32 objectid = httpup_pcb->httpupobjectid;

     read_buf = rkos_memory_malloc(2048);
     if(read_buf == NULL)
     {
        rk_printf("read_buf malloc fail");
        err = MALLOC_ERR;
        goto OVER;
     }
   // rk_printf(" flag = %d", httpup_pcb->delete_flag);
     do
     {
        if(httpup_pcb->delete_flag == 1)
        {
            err = DELEE_FLAG;
            break;
        }
       // printf("l = %d\n", left_len);
        if(left_len < 2048)
        {
            tx_len = httpup_pcb->HTT_UP_ReadDdata(read_buf, left_len);
            //printf("tx_len = %d\n", tx_len);
            if(tx_len != left_len )
            {
                rkos_memory_free(read_buf);
                err = READ_DATA_ERR;
                goto OVER;
            }

            left_len -= tx_len;
        }
        else
        {
            tx_len = httpup_pcb->HTT_UP_ReadDdata(read_buf, 2048);
            if(tx_len != 2048)
            {
                rkos_memory_free(read_buf);
                err = READ_DATA_ERR;
                goto OVER;
            }
            left_len -= tx_len;
        }
        //printf("r\n");
        err = netconn_write(httpup_pcb->httpup_conn, read_buf, tx_len, NETCONN_COPY);
        //printf(" %d = %d\n", err, n);
        //n++;
        if(err != ERR_OK)
        {
            rkos_memory_free(read_buf);
            goto OVER;
        }
     }while(left_len>0);
     rkos_memory_free(read_buf);

     if(httpup_pcb->delete_flag == 0)
     {

         err = netconn_recv_tcp_pbuf(httpup_pcb->httpup_conn, &tcp_buf);
         rk_printf("err = %d", err);
         if(err != ERR_OK)
         {
             rk_printf("up netconn_recv_tcp_pbuf fail");
             goto OVER;
         }

         head_len = tcp_buf->tot_len;
         head_buf = rkos_memory_malloc(head_len);
         if(head_buf == NULL)
         {
            rk_printf("up recv head malloc fail");
            err = MALLOC_ERR;
            goto OVER;
         }
         pbuf_copy_partial(tcp_buf, head_buf, head_len, 0);
         pbuf_free(tcp_buf);
         printf("up head_buf22 = %s\n", head_buf);
         rkos_memory_free(head_buf);
    }


     netconn_close(httpup_pcb->httpup_conn);
     netconn_delete(httpup_pcb->httpup_conn);

     if(httpup_pcb->HTTP_UP_CallFunction != NULL)
         httpup_pcb->HTTP_UP_CallFunction(err, httpup_pcb);

     RKTaskDelete(taskid, objectid, ASYNC_MODE);
     while(1)
     {
         rkos_sleep(1000);
         rk_printf(" OK HTTPUP task delete ASYNC_MODE");
     }

OVER:
     netconn_close(httpup_pcb->httpup_conn);
     netconn_delete(httpup_pcb->httpup_conn);

    if(httpup_pcb->HTTP_UP_CallFunction != NULL)
         httpup_pcb->HTTP_UP_CallFunction(err, httpup_pcb);

     RKTaskDelete(taskid, objectid, ASYNC_MODE);
     while(1)
     {
         rkos_sleep(1000);
         rk_printf(" ERR HTTPUP task delete ASYNC_MODE");
     }

}


//add lyb httpup test

HDC hFileDev1;
HDC hFile1;


int read_data(uint8 *buf, uint16 read_len)
{
    return( FileDev_ReadFile(hFile1, buf, read_len));
}



void httpup_test(void)
{

    FILE_ATTR stFileAttr;
    uint8 upname[20] = "这是爱.MP3";
    uint8 uphost[20] = "192.168.0.106";
    HTTPUPPCB *httpup_pcb;
    int i=0;
    rk_err_t res;

    hFileDev1 = RKDev_Open(DEV_CLASS_FILE, 0, NOT_CARE);
    if ((hFileDev1 == NULL) || (hFileDev1 == (HDC)RK_ERROR) || (hFileDev1 == (HDC)RK_PARA_ERR))
    {
        rk_printf("httpup_test File device open failure");
        while(1);
    }

    #if 0
    upname[strlen(upname)] = 0;
    while(upname[i] != 0)
        upname1[i] = upname[i];
   #endif
    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = L"这是爱.MP3";

    hFile1 = FileDev_OpenFile(hFileDev1, NULL, READ_CTRL, &stFileAttr);
    if((rk_err_t)hFile1 <= 0)
    {
        rk_printf("httpup test hfile open fail");
        while(1);
    }
    while(1)
    {

        FileDev_FileSeek(hFile1, SEEK_SET, 0);

        httpup_pcb = HttpUpPcb_New(read_data,  NULL);
        if(httpup_pcb == NULL)
        {
            rk_printf("httpup_pcb malloc fail");
            while(1);
        }

        res = HttpPost_up(httpup_pcb, uphost, 80, "/", upname, stFileAttr.FileSize);
        if(res == RK_ERROR)
        {
            rk_printf("httppost url fail");
        }
        else
        {
            rk_printf("httppost url success");
        }
        rkos_sleep(30000);
        HttpUp_Close(httpup_pcb);
        rkos_sleep(100);

    }
    return;
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#endif
