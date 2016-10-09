#include "BspConfig.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include <string.h>
#include <stdlib.h>
#include "app_cfg.h"
#include "debug_print.h"
#include <lwip/sockets.h>

#define UDP_TEST_STACK_SIZE 128

#define UDP_TEST_SERVER_PRIO    17
#define UDP_TEST_CLIENT_PRIO    17

#define REMOTE_IP   ((u32_t)0xc0a80165UL)   /*192.168.1.101*/
static char g_server_ip[16];
static unsigned long  g_packet_count = 1000;
static unsigned int g_cli_buf_size = 1024;

unsigned char udp_start_server = 0;
unsigned char udp_start_client= 0;
unsigned char g_udp_terminate = 0;
HTC *udpcllient_task = NULL;
HTC *udpserver_task = NULL;

HTC  udp_server_task_tcb;
HTC  udp_client_task_tcb;

int udp_client(const char* server_ipaddr, int port, int count)
{
    int cli_sockfd;
    socklen_t addrlen;
    struct sockaddr_in srv_addr;
    int loop= 0;
    int buf_size = g_cli_buf_size;
    char *buffer;

    buffer = (char*)rkos_memory_malloc(buf_size);

    if(NULL == buffer)
    {
        DEBUGPRINT("\n\rUDP: Allocate client buffer failed.\n");
        return -1;
    }

    /*create socket*/
    memset(buffer, 0, buf_size);
    cli_sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (cli_sockfd < 0)
    {
        DEBUGPRINT("\n\rUDP: create socket failed!");
        goto EXIT2;
    }
    /* fill sockaddr_in*/
    addrlen=sizeof(struct sockaddr_in);
    memset(&srv_addr, 0, addrlen);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = inet_addr(server_ipaddr);
    srv_addr.sin_port=htons(port);

    /* send data to server*/
    while (loop < count && !g_udp_terminate)
    {
        if(sendto(cli_sockfd, buffer, buf_size, 0, (struct sockaddr*)&srv_addr, addrlen) < 0)
        {
            DEBUGPRINT("\n\rUDP: client send error!");
            goto EXIT1;
        }
        loop++;
    }

EXIT1:
    close(cli_sockfd);

EXIT2:
    //free buffer
    rkos_memory_free(buffer);

    return 0;
}

int udp_server(int port)
{
    int serv_sockfd;
    socklen_t addr_len;
    int serv_buf_len = 1500;
    char* serv_buf;
    struct sockaddr_in serv_addr;

    serv_buf = rkos_memory_malloc(serv_buf_len);

    if(NULL == serv_buf)
    {
        DEBUGPRINT("\n\rUDP: allocate server buffer failed.");
        return -1;
    }

    /*create socket*/
    serv_sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if (serv_sockfd < 0)
    {
        DEBUGPRINT("\n\rUDP: create server socket failed!");
        goto EXIT2;
    }

    /*fill the socket in*/
    addr_len = sizeof(struct sockaddr_in);
    memset(&serv_addr, 0, addr_len);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    /*bind*/
    if (bind(serv_sockfd, (struct sockaddr *)&serv_addr, addr_len)<0)
    {
        DEBUGPRINT("\n\rUDP: server bind failed!");
        goto EXIT1;
    }

    DEBUGPRINT("UDP: server waiting for client on port 5001...\n");

    while(!g_udp_terminate)
    {
        memset(&serv_buf, 0, serv_buf_len);
        recvfrom(serv_sockfd, serv_buf, serv_buf_len, 0, (struct sockaddr*)&serv_addr, &addr_len);
    }

EXIT1:
    close(serv_sockfd);

EXIT2:
    //free buffer
    rkos_memory_free(serv_buf);

    return 0;
}

void udp_client_task(void *param)
{
    int err;
    int port = 5001;
    int count = g_packet_count;

    DEBUGPRINT("\n\rUDP: start udp client! count = %d\n",count);

    udp_client(g_server_ip, port, count);

#if OS_CFG_DBG_EN
    DEBUGPRINT("\n\rudp client task: used stack = %d, free stack = %d", udp_client_task_tcb.StkUsed, udp_client_task_tcb.StkFree);
#endif

    DEBUGPRINT("\n\rUDP: udp client stopped!");
    udpcllient_task = NULL;

    rkos_task_delete(NULL);
}

void udp_server_task(void *param)
{
    int err;
    int port = 5001;

    DEBUGPRINT("\n\rUDP: start udp server!");

    udp_server(port);

#if OS_CFG_DBG_EN
    DEBUGPRINT("\n\rudp server task: used stack = %d, free stack = %d", udp_server_task_tcb.StkUsed, udp_server_task_tcb.StkFree);
#endif

    DEBUGPRINT("\n\rUDP: udp server stopped!");
    udpserver_task = NULL;

    rkos_task_delete(NULL);
}

void cmd_udp(int argc, char **argv)
{
    int err;

    g_udp_terminate = udp_start_server = udp_start_client = 0;
    if(argc < 2)
    goto Exit;
    g_packet_count = 10000;
    memset(g_server_ip, 0, 16);

    if(strcmp(argv[1], "-s") == 0 ||strcmp(argv[1], "s") == 0)
    {
        if(udpserver_task)
        {
            DEBUGPRINT("\r\nUDP: UDP Server is already running.");
            return;
        }
        else
        {
            udp_start_server = 1;
        }
    }
    else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "c") == 0)
    {
        if(udpcllient_task)
        {
            DEBUGPRINT("\r\nUDP: UDP Server is already running.");
            return;
        }
        else
        {
            if(argc < 4)
                goto Exit;
            strncpy(g_server_ip, argv[2], (strlen(argv[2])>16)? 16 : strlen(argv[2]));
            g_cli_buf_size = atoi(argv[3]);
            if(argc == 5)
            {
                g_packet_count = atoi(argv[4]);
            }

            udp_start_client= 1;
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {

        g_udp_terminate = 1;
    }
    else
    {

        goto Exit;

    }


    if(udp_start_server && (NULL == udpserver_task))
    {
#if 0
        OSTaskCreate((OS_TCB     *)&udp_server_task_tcb,
                     (CPU_CHAR   *)"udp_server",
                     (OS_TASK_PTR ) udp_server_task,
                     (void       *) 0,
                     (OS_PRIO     ) UDP_TEST_SERVER_PRIO,
                     (CPU_STK    *)&udp_server_task_stk[0],
                     (CPU_STK_SIZE) UDP_TEST_STACK_SIZE / 50,
                     (CPU_STK_SIZE) UDP_TEST_STACK_SIZE,
                     (OS_MSG_QTY  ) 16u,
                     (OS_TICK     ) 0u,
                     (void       *) 0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);
#endif
        udp_server_task_tcb = rkos_task_create(udp_server_task,"udp_server", NULL, UDP_TEST_STACK_SIZE*8, UDP_TEST_SERVER_PRIO, NULL);
        if (udp_server_task_tcb == NULL)
        {
            DEBUGPRINT("\n\rUDP ERROR: create udp server task failed.");

        }
        else
        {
            udpserver_task = &udp_server_task_tcb;
        }
    }

    if(udp_start_client && (NULL == udpcllient_task))
    {
#if 0
        OSTaskCreate((OS_TCB     *)&udp_client_task_tcb,
                     (CPU_CHAR   *)"udp_client",
                     (OS_TASK_PTR ) udp_client_task,
                     (void       *) 0,
                     (OS_PRIO     ) UDP_TEST_CLIENT_PRIO,
                     (CPU_STK    *)&udp_client_task_stk[0],
                     (CPU_STK_SIZE) UDP_TEST_STACK_SIZE / 50,
                     (CPU_STK_SIZE) UDP_TEST_STACK_SIZE,
                     (OS_MSG_QTY  ) 16u,
                     (OS_TICK     ) 0u,
                     (void       *) 0,
                     (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                     (OS_ERR     *)&err);
#endif
        udp_client_task_tcb = rkos_task_create(udp_client_task,"udp_client", NULL, UDP_TEST_STACK_SIZE*8, UDP_TEST_CLIENT_PRIO, NULL);
        if (udp_client_task_tcb == NULL)
        {
            DEBUGPRINT("\n\rUDP ERROR: create udp client task failed.");

        }
        else
        {
            udpcllient_task = &udp_client_task_tcb;
        }

    }

    return;

Exit:
    DEBUGPRINT("\r\nUDP: udp test command format error!");
    DEBUGPRINT("\r\nUsage: \"udp -s\" to start udp server or \"udp -c\" to start udp client\r\n");
    return;
}


