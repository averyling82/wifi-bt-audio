#include "BspConfig.h"

#include "app_cfg.h"
#include <string.h>
#include <stdlib.h>
#include <lwip/sockets.h>
#include "debug_print.h"

#include "lwip/netifapi.h"
#include "FreeRTOS.h"
#include "RKOS.h"
#include "lwip/ip_addr.h"
#include "freertos_types.h"

#define TCP_PACKET_COUNT            10000
#define TCP_TEST_STACK_SIZE 128
#define TCP_TEST_SERVER_PRIO    17
#define TCP_TEST_CLIENT_PRIO    17

unsigned int g_srv_buf_size = 1500;
unsigned int g_cli_buf_size = 1500;
_os_task *g_server_task = NULL;
_os_task *g_client_task = NULL;

unsigned char g_start_server = 0;
unsigned char g_start_client = 0;
unsigned char g_tcp_terminate = 0;
char g_server_ip[16];
unsigned long  g_packet_count = TCP_PACKET_COUNT;

_os_task tcp_server_task_tcb;
_os_task  tcp_client_task_tcb;

int tcp_client(const char* server_ipaddr, int port)
{
    int iCounter;
    short   sTestBufLen;
    struct sockaddr_in  sAddr;
    int iAddrSize;
    int iSockFD;
    int iStatus;
    long    lLoopCount = 0;
    char    *cBsdBuf = NULL;

    if(g_cli_buf_size > 4300)
        g_cli_buf_size = 4300;
    else if (g_cli_buf_size == 0)
        g_cli_buf_size = 1500;

    //cBsdBuf = mymalloc(SRAMEX, g_cli_buf_size);
    cBsdBuf = (char *)freertos_malloc(g_cli_buf_size);
    if(NULL == cBsdBuf)
    {
        DEBUGPRINT("\n\rTCP: allocate client buffer failed.\n");
        return -1;
    }

    for (iCounter = 0; iCounter < g_cli_buf_size; iCounter++)
    {
        cBsdBuf[iCounter] = (char)(iCounter % 10);
    }
    sTestBufLen  = g_cli_buf_size;

    FD_ZERO(&sAddr);
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(port);
    sAddr.sin_addr.s_addr = inet_addr(server_ipaddr);

    iAddrSize = sizeof(struct sockaddr_in);

    // creating a TCP socket
    iSockFD = socket(AF_INET, SOCK_STREAM, 0);
    if( iSockFD < 0 )
    {
        DEBUGPRINT("\n\rTCP ERROR: create tcp client socket fd error!");
        goto Exit1;
    }

    DEBUGPRINT("\n\rTCP: server IP=%s port=%d.", server_ipaddr, port);
    DEBUGPRINT("\n\rTCP: create socket %d.", iSockFD);
    DEBUGPRINT("\n\rTCP: g_cli_buf_size %d.", g_cli_buf_size);
    // connecting to TCP server
    iStatus = connect(iSockFD, (struct sockaddr *)&sAddr, iAddrSize);
    if (iStatus < 0)
    {
        DEBUGPRINT("\n\rTCP ERROR: tcp client connect server error! ");
        goto Exit;
    }

    DEBUGPRINT("\n\rTCP: connect server successfully.");
    // sending multiple packets to the TCP server
    while (lLoopCount < g_packet_count && !g_tcp_terminate)
    {
        // sending packet
        iStatus = send(iSockFD, cBsdBuf, sTestBufLen, 0 );
        if( iStatus <= 0 )
        {
            DEBUGPRINT("\r\nTCP ERROR: tcp client send data error!  iStatus:%d", iStatus);
            goto Exit;
        }
        lLoopCount++;
        //DEBUGPRINT("\n\rTcp: send data count:%d, status:%d", lLoopCount, iStatus);
    }

    DEBUGPRINT("\n\rTCP: sent %u packets successfully.", lLoopCount);

Exit:
    close(iSockFD);

Exit1:
    //myfree(SRAMEX, cBsdBuf);
    freertos_mfree(cBsdBuf, g_cli_buf_size);
    return 0;
}

int tcp_server(unsigned short port)
{
    struct sockaddr_in  sAddr;
    struct sockaddr_in  sLocalAddr;
    int iCounter;
    int iAddrSize;
    int iSockFD;
    int iStatus;
    int iNewSockFD;
    long    lLoopCount = 0;
    int iTestBufLen;
    int n;
    char    *cBsdBuf = NULL;

    if(g_srv_buf_size > 5000)
        g_srv_buf_size = 5000;
    else if (g_srv_buf_size == 0)
        g_srv_buf_size = 1500;

    cBsdBuf = (char *)freertos_malloc(g_srv_buf_size);
    if(NULL == cBsdBuf)
    {
        DEBUGPRINT("\n\rTCP: allocate server buffer failed.\n");
        return -1;
    }

    for (iCounter = 0; iCounter < g_srv_buf_size; iCounter++)
    {
        cBsdBuf[iCounter] = (char)(iCounter % 10);
    }
    iTestBufLen  = g_srv_buf_size;

    iSockFD = socket(AF_INET, SOCK_STREAM, 0);
    if( iSockFD < 0 )
    {
        goto Exit2;
    }

    DEBUGPRINT("\n\rTCP: create server socket %d\n\r", iSockFD);
    n = 1;
    setsockopt( iSockFD, SOL_SOCKET, SO_REUSEADDR,
                (const char *) &n, sizeof( n ) );
    memset((char *)&sLocalAddr, 0, sizeof(sLocalAddr));
    sLocalAddr.sin_family      = AF_INET;
    sLocalAddr.sin_len         = sizeof(sLocalAddr);
    sLocalAddr.sin_port        = htons(port);
    sLocalAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    iAddrSize = sizeof(sLocalAddr);

    // binding the TCP socket to the TCP server address
    iStatus = bind(iSockFD, (struct sockaddr *)&sLocalAddr, iAddrSize);
    if( iStatus < 0 )
    {
        DEBUGPRINT("\n\rTCP ERROR: bind tcp server socket fd error! ");
        goto Exit1;
    }
    DEBUGPRINT("\n\rTCP: bind successfully.");

    // putting the socket for listening to the incoming TCP connection
    iStatus = listen(iSockFD, 20);
    if( iStatus != 0 )
    {
        DEBUGPRINT("\n\rTCP ERROR: listen tcp server socket fd error! ");
        goto Exit1;
    }
    DEBUGPRINT("\n\rTCP: listen port %d", port);

Restart:
    iNewSockFD = -1;
    lLoopCount = 0;

    // waiting for an incoming TCP connection
    while( iNewSockFD < 0 )
    {
        // accepts a connection form a TCP client, if there is any
        // otherwise returns SL_EAGAIN
        int addrlen=sizeof(sAddr);
        iNewSockFD = accept(iSockFD, ( struct sockaddr *)&sAddr,
                            (socklen_t*)&addrlen);
        if( iNewSockFD < 0 )
        {
            DEBUGPRINT("\n\rTCP ERROR: accept tcp client socket fd error! ");
            goto Exit1;
        }
        DEBUGPRINT("\n\rTCP: accept socket %d successfully.", iNewSockFD);
    }

    // waits packets from the connected TCP client
    while (!g_tcp_terminate)
    {
        iStatus = recv(iNewSockFD, cBsdBuf, iTestBufLen, 0);  //MSG_DONTWAIT   MSG_WAITALL
        if( iStatus < 0 )
        {
            DEBUGPRINT("\n\rTCP ERROR: server recv data error iStatus:%d ", iStatus);
            goto Exit;
        }
        else if (iStatus == 0)
        {
            //DEBUGPRINT("\n\rTCP: recieved %u packets successfully.", iStatus);
            //close(iNewSockFD);
            //goto Restart;
            DEBUGPRINT("\n\rTCP ERROR: server recv data error iStatus:%d ", iStatus);
            goto Exit;
        }
        lLoopCount++;
    }

Exit:
    DEBUGPRINT("\n\close(iNewSockFD)\n");
    // close the connected socket after receiving from connected TCP client
    close(iNewSockFD);

Exit1:
    // close the listening socket
    close(iSockFD);

Exit2:
    //free buffer
    freertos_mfree(cBsdBuf, g_srv_buf_size);

    return 0;
}

void tcp_server_task(void *param)
{
    unsigned short port = 5001;
    int err;
    _os_task *g_server_task_backup;
    printf("\n\rTCP: start tcp Server!");

    if(g_start_server)
        tcp_server(port);

    printf("\n\rTCP: tcp server stopped!");
    g_server_task_backup = g_server_task;
    g_server_task = NULL;
    freertos_delete_task(g_server_task_backup);

}

void tcp_client_task(void *param)
{
    unsigned short port = 5001;
    int err;
    _os_task *g_client_task_backup;
    printf("\n\rTCP: start tcp client!");

    if(g_start_client)
        tcp_client(g_server_ip, port);

    printf("\n\rTCP: tcp client stopped!");
    g_client_task_backup = g_client_task;
    g_client_task = NULL;
    g_start_client = 0;
    freertos_delete_task(g_client_task_backup);

}

void cmd_tcp(int argc, char **argv)
{
    int err;

    g_tcp_terminate = g_start_server = g_start_client = 0;
    g_packet_count = 10000;
    memset(g_server_ip, 0, 16);

    if(argc < 2)
        goto Exit;

    if(strcmp(argv[1], "-s") == 0 ||strcmp(argv[1], "s") == 0)
    {
        if(g_server_task)
        {
            DEBUGPRINT("\n\rTCP: Tcp server is already running.");
            return;
        }
        else
        {
            g_start_server = 1;
            if(argc == 3)
                g_srv_buf_size = atoi(argv[2]);
        }
    }
    else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "c") == 0)
    {
        if(g_client_task)
        {
            DEBUGPRINT("\n\rTCP: Tcp client is already running. Please enter \"tcp stop\" to stop it.");
            return;
        }
        else
        {
            if(argc < 4)
                goto Exit;
            g_start_client = 1;
            strncpy(g_server_ip, argv[2], (strlen(argv[2])>16)? 16 : strlen(argv[2]));
            g_cli_buf_size = atoi(argv[3]);
            printf("g_cli_buf_size = %d\n",g_cli_buf_size);
            if(argc == 5)
                g_packet_count = atoi(argv[4]);
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        g_tcp_terminate = 1;
    }
    else
    {
        goto Exit;
    }

    if(g_start_server && (NULL == g_server_task))
    {
        err = freertos_create_task(&tcp_server_task_tcb, "tcp_server", TCP_TEST_STACK_SIZE*8, 0, TCP_TEST_SERVER_PRIO, tcp_server_task, NULL);
        if (pdPASS == err)
            g_server_task = &tcp_server_task_tcb;
        else
            printf("\n\rTCP ERROR: create tcp server task failed.");
    }

    if(g_start_client && (NULL == g_client_task))
    {
        err = freertos_create_task(&tcp_client_task_tcb, "tcp_client", TCP_TEST_STACK_SIZE*8, 0, TCP_TEST_CLIENT_PRIO, tcp_client_task, NULL);
        if (pdPASS == err)
            g_client_task = &tcp_client_task_tcb;
        else
            printf("\n\rTCP ERROR: create tcp client task failed.");
    }

    return;

Exit:
    printf("\n\rTCP: Tcp test command format error!");
    printf("\n\rUsage: \"tcp -s\" to start tcp server or \"tcp <-c *.*.*.*> <buf len> [count]]\" to start tcp client\n\r");
    return;
}

