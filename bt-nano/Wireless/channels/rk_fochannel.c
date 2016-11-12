/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Web\channels\rk_fochannel.c
* Owner: lyb
* Date: 2015.11.4
* Time: 11:18:21
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    lyb     2015.11.4     11:18:21   1.0
********************************************************************************************
*/


#include "BspConfig.h"

#ifdef __WEB_CHANNELS_RK_FOCHANNEL_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "typedef.h"
#include "sockets.h"
#include "ip_addr.h"
#include "SysInfoSave.h"
#include "ip_addr.h"
#include "wwd_structures.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/





//#define UDPLOST_PACKET

uint8 SetSample_flag = 0;
uint8 tcp_udp_server_flag = 0;

enum _Channel{
    cFrontLeft = 0,
    cFrontRight,
    cCenter,
    cSubwooffer,
    cSurroundLeft,
    cSurroundRight,
    cChannelMax
};

typedef struct _HeartBeat{
   int session;
   int seq;
   int channel;
}heartBeat;

typedef struct _udplost_request
{
   int session;
   int udpcmd;
   uint32 minnum;
   uint32 maxnum;
}udplost_request;

typedef struct _connectLine {
    int socketType;
    int session;
    //int channel;
    char mac[17];
    struct in_addr  serverip;
}connectLine;

typedef struct _cmdLine {
    int session;
    int channel;
    int volume;
    int depth;
    int samplerate;
}cmdLine;

typedef struct __CHANNELS_MESSAGE
{
    //int channel;
    int serverport;
    struct in_addr serverip;
    char mac[17];
}CHANNEL_MESSAGE;

typedef struct __CHANNEL_ASK_QUEUE
{
    CHANNEL_MESSAGE MSG;
    int session;
    int cmd;
    int channel;
    udplost_request udpmsg;
}CHANNEL_ASK_QUEUE;

typedef struct __CHANNEL_RESP_QUEUE
{
    int session;
    int result;
}CHANNEL_RESP_QUEUE;
typedef struct __TCPCONTROL_DATA_BLOCK
{
    pQueue TcpcontrolAskQueue;
    pQueue TcpcontrolRespQueue;
    int speaker;
   // int sock;
    //int connect_flag;
}TCPCONTROL_DATA_BLOCK;

typedef struct __TCPSTREAM_DATA_BLOCK
{
    pQueue TcpstreamAskQueue;
    pQueue TcpstreamRespQueue;

    //int sock;
    //int connect_flag;
}TCPSTREAM_DATA_BLOCK;

typedef struct __TCPBROAD_DATA_BLOCK
{
    pQueue TcpBroadAskQueue;
    pQueue TcpBroadRespQueue;
}TCPBROAD_DATA_BLOCK;

typedef struct __TCPCHANEL_DATA_BLOCK
{
    pQueue TcpChannelAskQueue;
    pQueue TcpChannelRespQueue;
}TCPCHANNEL_DATA_BLOCK;


typedef struct __TCPHEART_DATA_BLOCK
{
    pQueue TcpHeartAskQueue;
    pQueue TcpHeartRespQueue;
}TCPHEART_DATA_BLOCK;


enum
{
    CONNECT_FAIL = 0,
    CONNECT_SUCCESS = 1,
};
enum
{
    BROAD_CMD = 0,
    TCPCONTROL_CMD = 1,
    TCPSTREAM_CMD = 2,
    REBROAD_CMD,
    UDPPACKET_LOSTCMD,
    UP_Volume,
    DN_volume,
    RESTART_WIFI,
};

#define TCP_STREAM_RECV_LEN  1024
typedef struct __CHANNEL_PCB
{
    HDC hFifo;
    HDC pAudio;
    int session;
    int tcpControl_sock;
    int tcpControl_connectflag;
    int tcpStream_sock;
    int tcpStream_connectflag;
    int tcpStream_num;
    int tcpControl_num;
    int broad_sock;
    int tcpHeart_sock;
    int tcpHeart_connectflag;
}CHANNEL_PCB;

typedef struct _STREAM_DATA
{
    uint32 num;
    int stream_len;
    char stram_data[TCP_STREAM_RECV_LEN];
}STREAM_DATA;

typedef struct _STREAM_NODE
{
    STREAM_DATA stream_data;
    struct _STREAM_NODE *next;
}STREAM_NODE;

#define TCP_CONTROL_PORT  1234
#define TCP_STREAM_PORT  3067



#define CTLSOCKET 0
#define MEDIASOCKET 1
#define UDPLOST 2
#define CHANNELS_MESSAGE_LEN sizeof(CHANNEL_MESSAGE)
#define CONNECTION_LINE_LEN sizeof(connectLine)
#define CMD_LINE_LEN sizeof(cmdLine)

int set_flag = 0;

#ifdef UDPLOST_PACKET

#define  UDP_LOST_FRAME 125

typedef struct
{
    STREAM_DATA *data;
    uint32 seq;
    uint32 ready;
    uint32 rsd_end;
    uint32 rsd_count;
}UDP_LOST_BUF;

UDP_LOST_BUF udp_lost_buf[UDP_LOST_FRAME];

#define UDPLOSTIDX(seqno) ((int)(seqno) % UDP_LOST_FRAME)

uint8 packet_first = 0;
uint32 write_num = 0;
uint32 read_num = 0;

#endif

#define FIFO_LEVEL_MAX  45*1024
#define FIFO_LEVEL_MIN  10*1024

#define CHANNEL_TYPE  cSubwooffer// cFrontLeft// cSurroundLeft //cSurroundRight//cFrontRight//cCenter//cSubwooffer

uint32 cursample = 0;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static TCPCONTROL_DATA_BLOCK *gpstTcpControlTaskData = NULL;
static TCPSTREAM_DATA_BLOCK *gpstTcpStreamTaskData = NULL;
static TCPBROAD_DATA_BLOCK *gpstTcpBroadTaskData = NULL;
static TCPCHANNEL_DATA_BLOCK *gpstTcpChannelTaskData = NULL;
static TCPHEART_DATA_BLOCK *gpstTcpHeartTaskData = NULL;



static CHANNEL_PCB  gpstChannelPcb;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
static void BroadMessage_Parse(char *mbuf, int mbuf_len);
static void TcpMessage_Parse(char *mbuf, int mbuf_len);
static void TcpStream_WriteFifo(void *mbuf, int mbuf_len);

static void streambuf_push(STREAM_DATA *buf);
static void clear_udp_lost_buf(uint32 seqno);
static void streambuf_pop(void);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

void GpstChannelSet_volume(void)
{

    CHANNEL_ASK_QUEUE Broad_ask;

    AudioDev_SetVol(gpstChannelPcb.pAudio, 0);
    //Broad_ask.cmd = DN_volume;

    //rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &Broad_ask, MAX_DELAY);
#if 0
    int i=25;
    for(i = 25; i>0; i--)
    {
        AudioDev_SetVol(gpstChannelPcb.pAudio, i);
        rkos_sleep(10);
    }
#endif
}

void GpstChannelSet_volume1(void)
{
    CHANNEL_ASK_QUEUE Broad_ask;

    Broad_ask.cmd = UP_Volume;

    rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue,&Broad_ask, MAX_DELAY);
#if 0
    int i=0;
    for(i = 1; i < 26; i++)
    {
        AudioDev_SetVol(gpstChannelPcb.pAudio, i);
        rkos_sleep(10);
    }
#endif
}

void setVolume(int vol)
{

    int mvol = 0;
#if 0
    rk_printf("vol = %d", vol);
    switch(vol)
    {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
        mvol = vol;
        break;
      case 6:
        mvol = vol+1;
        break;
      case 7:
        mvol = vol + 2;
        break;
      case 8:
        mvol = vol + 3;
        break;
      case 9:
        mvol = vol + 4;
        break;
      case 10:
        mvol = vol + 5;
        break;
      case 11:
        mvol = vol + 6;
        break;
      case 12:
        mvol = vol + 7;
        break;
      case 13:
        mvol = vol + 8;
        break;
      case 14:
        mvol = vol + 9;
        break;
      case 15:
        mvol = vol + 10;
        break;
      default:

        rk_printf("vol err = %d", vol);
        return;

    }
#else

    mvol = vol*2;

#endif
    rk_printf("mvol = %d", mvol);
    AudioDev_SetVol(gpstChannelPcb.pAudio, mvol);
    return;

}



//config serer  broadcast 9000
#define RECV_NUM 128
#define WIFISPEAKER  "WIFISPEAKER"
#define IP    "IP"
#define PORT  "PORT"
#define WIFISPEAKERUDP   "WIFISPEAKER:UDP"
#define NANOD   "NANOD"
#define CHANNEL   "CHANNEL"
#define NANODOK   "NANOD:Ok"

int configtcp_socket = -1;
int brocadsend_sock = -1;
int brocadrecv_sock = -1;
struct sockaddr_in broad_sendaddr;
struct sockaddr_in  broad_recvaddr;
socklen_t broadrecv_len = 0;
char broad_recvbuf[RECV_NUM];
//uint8 broad_sendbuf[RECV_NUM];
uint8 server_ip[16];
//uint32 servertcp_port;
uint32 server_port;
int speaker_channel = -1;
char nanodmac[20];
extern uint32 wifi_ip_addr;

rk_err_t ConfigServerInit(void)
{
    struct sockaddr_in localaddr;
    int brocat_cast = 1;
    int ret = -1;
    int timeout = 5*100;

//set send broad_addr
     //rk_printf("set send and recv addr");
    memset(&broad_sendaddr, 0, sizeof(struct sockaddr_in));
    broad_sendaddr.sin_family = AF_INET;
    broad_sendaddr.sin_port = htons(9000);
    broad_sendaddr.sin_addr.s_addr = inet_addr("255.255.255.255");

//send recv broad_addr
     memset(&broad_recvaddr, 0, sizeof(struct sockaddr_in));
    broad_recvaddr.sin_family = AF_INET;
    broad_recvaddr.sin_port = htons(9002);
    broad_recvaddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    //send socket
     //rk_printf("send socket");
     brocadsend_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  ;
     if(brocadsend_sock < 0)
     {
        rk_printf("brocadsend socket fail");
        return FALSE;
     }

    ret = setsockopt(brocadsend_sock, SOL_SOCKET, SO_BROADCAST, &brocat_cast, sizeof(brocat_cast));
     if(ret < 0)
     {
        rk_printf("set opt fail");
        return FALSE;
    }

    //recv socket
     //rk_printf("recv socket");
     brocadrecv_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
     if(brocadrecv_sock < 0)
     {
        rk_printf("brocadrecv socket fail");
        return FALSE;
     }

     memset(&localaddr, 0, sizeof(localaddr));
     localaddr.sin_family = AF_INET;
     localaddr.sin_port = htons(9002);
     localaddr.sin_addr.s_addr = htonl(INADDR_ANY);

     ret = bind(brocadrecv_sock, (struct sockaddr*)&localaddr, sizeof(struct sockaddr_in));
     if(ret < 0)
     {
        rk_printf("brocadrecv_sock bind fail");
        return FALSE;
     }

     ret = setsockopt(brocadrecv_sock, SOL_SOCKET, SO_BROADCAST, &brocat_cast, sizeof(brocat_cast));
     if(ret < 0)
     {
        rk_printf("brocadrecv setopt fail");
        return FALSE;
     }

     //设置超时
      //rk_printf("set recv");
      ret = setsockopt(brocadrecv_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
      if(ret < 0)
      {
          rk_printf("brocadrecv set time out fail");
          return FALSE;
      }


      broadrecv_len = sizeof(struct sockaddr_in);
     return TRUE;
}

rk_err_t praseConfig(char *mbuf, int len)
{
    char *pp = NULL;
    char *pp1 = NULL;
    char port_buf[5] = {0};
    int i=0, j=0;

    pp= (char *)strstr(mbuf, WIFISPEAKER);
    if(pp == NULL)
    {
          rk_printf("no real wifiserver");
          return FALSE;
    }

    pp = (char *)strstr(mbuf, IP);
    if(pp== NULL)
    {
          rk_printf("no real ip");
          return FALSE;
    }

    pp1 = (char *)strstr(mbuf, PORT);
    if(pp1 == NULL)
    {
         rk_printf("no real PORT");
         return FALSE;
    }

    i = 3;
    while(pp[i] != ',')
    {
      server_ip[j++] = pp[i++];
       if(j>17)
       {
         rk_printf("ip addr fail");
         return FALSE;
       }
    }

    i = 5;
    j = 0;
    while(pp1[i] != '\0')
    {

        port_buf[j++] = pp1[i++];
        //rk_printf("j=%d,i = %d, %d", j,i,pp1[i]);
        if(j>4)
       {
           rk_printf("port err");
            return FALSE;
       }
    }

    port_buf[j] = '\0';
    printf("port = %s", port_buf);

    server_port = atoi(port_buf);

    rk_printf("server_ip = %s, port = %x", server_ip, server_port);

    return TRUE;
}

rk_err_t ConfigServer(void)
{
    rk_err_t ret = -1;
    int len = 0;

    ret = ConfigServerInit();
    if(ret == FALSE)
    {
      rk_printf("config server init fail");
      return FALSE;
    }
    rk_printf("config ServerInit ok");

    while(1)
    {

        if(rkos_GetFreeHeapSize() > 30*1024)
        {
           sendto(brocadsend_sock, WIFISPEAKERUDP, sizeof(WIFISPEAKERUDP), 0, (struct sockaddr*)&broad_sendaddr, sizeof(struct sockaddr_in));
           sendto(brocadsend_sock, WIFISPEAKERUDP, sizeof(WIFISPEAKERUDP), 0, (struct sockaddr*)&broad_sendaddr, sizeof(struct sockaddr_in));
          //rk_printf("send broadsock ok");
           memset(broad_recvbuf, 0, RECV_NUM);
           len = recvfrom(brocadrecv_sock, broad_recvbuf, RECV_NUM, 0, (struct sockaddr*)&broad_recvaddr, &broadrecv_len);
           if(len > 0)
           {
              rk_printf("broad_recv = %s", broad_recvbuf);
              ret = praseConfig(broad_recvbuf, len);  //广播服务收取到广播的ip和地址就可以推出服务
             if(ret == TRUE)
             {
                 break;
             }
          }
          else
          {
                rk_printf("recvfrom timeout");
          }
       }
        else
        {

            rkos_sleep(1000);
        }
    }

    close(brocadsend_sock);
    close(brocadrecv_sock);

#if 0
    configtcp_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(configtcp_socket < 0)
    {
      rk_printf("configtcp_socket socket fail");
     return FALSE;
    }

    remote_addr.sin_addr.s_addr = inet_addr(server_ip);
    remote_addr.sin_port = htons(server_port); //连接3066
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_len = sizeof(remote_addr);

    ret = connect(configtcp_socket, (struct sockaddr *)&remote_addr, remote_addr.sin_len);
    if(ret < 0)
    {
      rk_printf("connect fail");

     close(configtcp_socket);
      return FALSE;
    }


    wwd_wifi_get_mac_address( &mac, WWD_STA_INTERFACE );

    len = sprintf(m_buf1,"%02x.%02x.%02x.%02x.%02x.%02x",
        mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]);

    rk_printf("m_buf1 = %s, len = %d", m_buf1, len);
#endif

#if 0
    //memcpy(TL+1, (char*)&len, 4);
    m_buf[0] = 'L';
    memcpy(m_buf+1, (char*)&len, 4);
    memcpy(m_buf+5, m_buf1, len);

    rk_printf("m_buf = %s, len = %d", m_buf, len);

    debug_hex(m_buf, len, 16);
    ret = send(configtcp_socket, m_buf, len+5, 0); //mac 地址和ip地址发送信息ok
    if(ret < 0)
    {
        rk_printf("5.1 server error");
        close(configtcp_socket);
        return FALSE;
    }

    //等待box配置好的通道。

    ret = recv(configtcp_socket, TL, 5, 0);
    if(ret < 0 || TL[0] != 'L' )
    {
        rk_printf("box configmessag send error");
        close(configtcp_socket);
        return FALSE;
    }
    memset(&broad_recvbuf, 0, sizeof(broad_recvbuf));
    length = memcpy(&length, TL+1, 4);

    ret = recv(configtcp_socket, broad_recvbuf, length, 0);
    if(ret < 0)
    {
       rk_printf("box configmessag send error = %s", broad_recvbuf);
       close(configtcp_socket);
       return FALSE;
    }

    p = (char*)strstr(broad_recvbuf, NANOD);
    if(p == NULL)
    {
       rk_printf("nanod = %s", broad_recvbuf);
       close(configtcp_socket);
       return FALSE;
    }

    p = (char *)strstr(p, CHANNEL);
    if(p == NULL)
    {
       rk_printf("CHANNEL = %s", broad_recvbuf);
       close(configtcp_socket);
       return FALSE;
    }
    memset(TL, 0, 5);
    i=0, j=8;
    while(p[j] != '\n')
    {
        if(i>5)
        {
           rk_printf("channel = %s", broad_recvbuf);
           close(configtcp_socket);
           return FALSE;
        }
        TL[i++] = p[j++];
    }

    sscanf(TL, "%d", &speaker_channel);
    rk_printf("speaker_channel = %d", speaker_channel);
    memset(m_buf, 0x00, 128);
    length = 8;
    m_buf[0] = 'L';
    memcpy(m_buf+1, &length, 4);
    memcpy(m_buf+5, NANODOK, 8);

    ret = send(configtcp_socket, m_buf, 13, 0);
    if(ret < 0)
    {
         rk_printf("send ok fail %d", speaker_channel);
           close(configtcp_socket);
           return FALSE;

    }

    rkos_sleep(1000);
    close(configtcp_socket);
#endif

    return TRUE;

}


void RK_ChannelTask_Enter(void)
{
    CHANNEL_ASK_QUEUE Channel_ask;
    CHANNEL_ASK_QUEUE Channel_ask1;
    CHANNEL_RESP_QUEUE Channel_control_resp;
    CHANNEL_RESP_QUEUE Channel_stream_resp;
    CHANNEL_RESP_QUEUE channel_tcpheart_resp;
    CHANNEL_ASK_QUEUE Broad_ask;
    FIFO_DEV_ARG stFifoArg;
    printf("rk_channeltask_enter\n");
    rk_err_t ret = 0;
    wiced_mac_t mac;
    int len = 0;

    RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, NULL, SYNC_MODE);
    //rk_wifi_usartconfig("cpsb-lyb-test",13,"123123123", 9);
    //rk_wifi_usartconfig("Rockchip-guest-5G",17,"RKguest5.0", 10);
    //rk_wifi_usartconfig("cpsb-wp-test",12,"123456789", 9);
    //rk_wifi_usartconfig("cpsb-lyb-test",13,"123456789", 9);
    //rk_wifi_usartconfig("cpsb-ykk-test",13,"yakir15060416783", 16);
    //rk_wifi_usartconfig("An",13,"123456789", 9);
    //rk_wifi_usartconfig("cpsb-ctf-test",13,"cp3b_ctf", 8);
   // rk_wifi_usartconfig("lyb",3,"123456789", 9);
    //rk_wifi_usartconfig("cpsb-sch-test",13,"123123123", 9);
    //rk_wifi_usartconfig("HUA",3,"12345678", 8);
    //rk_wifi_usartconfig("Rockchip-guest-5G",17,"RKguest5.0", 10);
    // rk_wifi_usartconfig("socket_test",11,"123456789", 9);

    while(1)
    {
        rkos_sleep(1000);
        //开机初始化会进行一次路由器加入申请，加入失败或者成功
        if((wifi_join_flag() == 1) || (wifi_join_flag() == 3))
        {
            break;
        }
    }

    if(wifi_connect_flag() != 1)  //wifi连接失败进入smartconfig
    {
        rkos_sleep(1000);
        rk_wifi_smartconfig();
		MainTask_SetStatus(MAINTASK_WIFICONFIG,1);//jjjhhh 20161110
    }
//   调用接口提示音当前进入smartconfig模式
     //        MusicEnter_smartconfig();

//如果连接失败只能上电重启
    //rk_printf("link = %d", wifi_connect_flag());
    do
    {
        rkos_sleep(1000);
    } while(wifi_connect_flag() != 1);  //wificonnect ok


    //while(1)
    //{
       // rk_wifi_relink_sysinfo(1);
        //rkos_sleep(1000);
    // }


    //get mac 信息
    wwd_wifi_get_mac_address( &mac, WWD_STA_INTERFACE );

    len = sprintf(nanodmac,"%02x.%02x.%02x.%02x.%02x.%02x",
        mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]);

    rk_printf("m_buf1 = %s, len = %d", nanodmac, len);

    while(!ConfigServer());

    rk_printf("get server ip and port ok");

    FREQ_EnterModule(FREQ_5);

//stFifoArg.BlockCnt = 200000;
    stFifoArg.BlockCnt = 50;
    stFifoArg.BlockSize = 1024;
    stFifoArg.UseFile = 0;
    stFifoArg.ObjectId = 0;
    stFifoArg.hReadFile = NULL;
    if (DeviceTask_CreateDeviceList(DEVICE_LIST_FIFO, &stFifoArg, SYNC_MODE) != RK_SUCCESS)
    {
      printf("fifo  device list create fail\n");
    }

    gpstChannelPcb.pAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);
    if (( gpstChannelPcb.pAudio  == NULL) || ( gpstChannelPcb.pAudio  == (HDC)RK_ERROR) || ( gpstChannelPcb.pAudio  == (HDC)RK_PARA_ERR))
    {
        printf("Audio device open failure\n");
        while(1);
    }
    gpstChannelPcb.hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    if ((gpstChannelPcb.hFifo == NULL) || (gpstChannelPcb.hFifo == (HDC)RK_ERROR) || (gpstChannelPcb.hFifo == (HDC)RK_PARA_ERR))
    {
        printf("Audio device open failure\n");
        while(1);
    }
   // fifoDev_SetTotalSize(gpstChannelPcb.hFifo , 0);
    RKTaskCreate(TASK_ID_MEDIA, 0, NULL, SYNC_MODE);

    fifoDev_SetTotalSize(gpstChannelPcb.hFifo , 0xffffffff);
    fifoDev_SetFIFOLevel(gpstChannelPcb.hFifo, FIFO_LEVEL_MAX, FIFO_LEVEL_MIN);

    //printf("rk_channle\n");

    Channel_ask1.MSG.serverport = server_port;
    Channel_ask1.MSG.serverip.s_addr = inet_addr(server_ip);
    memcpy(Channel_ask1.MSG.mac, nanodmac, 17);


    tcpcontrol_relink(&Channel_ask1);

#if 0
    gSysConfig.SpeakConfig.flag = 1;
    if(gSysConfig.SpeakConfig.flag == 1)  // read for system info
    {

        Channel_ask1.MSG.serverport = server_port;
        Channel_ask1.MSG.serverip.s_addr = inet_addr(server_ip);
        memcpy(Channel_ask1.MSG.mac, nanodmac, 17);   //mac 地址加上 .  共17挝?

        rkos_queue_send(gpstTcpControlTaskData->TcpcontrolAskQueue, &Channel_ask1, MAX_DELAY);
        rkos_queue_receive(gpstTcpControlTaskData->TcpcontrolRespQueue, &Channel_control_resp, MAX_DELAY);
        if(Channel_control_resp.result == RK_SUCCESS)
        {
            rkos_queue_send(gpstTcpHeartTaskData->TcpHeartAskQueue, &Channel_ask1, MAX_DELAY);
            rkos_queue_receive(gpstTcpHeartTaskData->TcpHeartRespQueue, &channel_tcpheart_resp, MAX_DELAY);
            printf("hearttaskdata = %d\n", channel_tcpheart_resp.result);
            if(channel_tcpheart_resp.result == RK_SUCCESS)
            {
                rkos_queue_send(gpstTcpStreamTaskData->TcpstreamAskQueue, &Channel_ask1, MAX_DELAY);
                rkos_queue_receive(gpstTcpStreamTaskData->TcpstreamRespQueue, &Channel_stream_resp, MAX_DELAY);

                if(Channel_stream_resp.result == RK_SUCCESS)
                {
                }
                else
                {
                   //RKTaskCreate(TASK_ID_BROAD_TASK, 0, NULL, SYNC_MODE);
                }
            }
            else
            {
                //RKTaskCreate(TASK_ID_BROAD_TASK, 0, NULL, SYNC_MODE);
            }

        }
        else
        {
             //RKTaskCreate(TASK_ID_BROAD_TASK, 0, NULL, SYNC_MODE);
        }

    }
#endif
    while(1)
    {
        rk_printf("receive gpst channel task data");
        rkos_queue_receive(gpstTcpChannelTaskData->TcpChannelAskQueue, &Broad_ask, MAX_DELAY);

        switch(Broad_ask.cmd)
        {
            case BROAD_CMD:
                rk_printf("broad_cmd = %d", wifi_connect_flag());
                while(wifi_connect_flag() != 1)  //wificonnect ok
                {
                    rkos_sleep(1000);
                }

                while(!ConfigServer());  //重新连接重启 广播包

                rk_printf("get server ip and port ok");

                Channel_ask1.MSG.serverport = server_port;
                Channel_ask1.MSG.serverip.s_addr = inet_addr(server_ip);
                memcpy(Channel_ask1.MSG.mac, nanodmac, 17);

                tcpcontrol_relink(&Channel_ask1);

            case REBROAD_CMD:
                rk_printf("reboard");
                if(gpstChannelPcb.tcpStream_connectflag == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpStream_sock, 0);
                    closesocket(gpstChannelPcb.tcpStream_sock);
                    gpstChannelPcb.tcpStream_connectflag = CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpControl_connectflag == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpControl_sock, 2);
                    closesocket(gpstChannelPcb.tcpControl_sock);
                    gpstChannelPcb.tcpControl_connectflag= CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpHeart_connectflag == CONNECT_SUCCESS)
                {
                     shutdown(gpstChannelPcb.tcpHeart_sock, 1);
                     closesocket(gpstChannelPcb.tcpHeart_sock);
                     gpstChannelPcb.tcpHeart_connectflag= CONNECT_FAIL;
                }
                RKTaskDelete(TASK_ID_TCPSTREAM_TASK, 0, SYNC_MODE);
                RKTaskDelete(TASK_ID_TCPHEART_TASK, 0, SYNC_MODE);
                RKTaskDelete(TASK_ID_TCPCONTROL_TASK, 0, SYNC_MODE);
                rkos_sleep(100);
                Channel_ask1.MSG.serverport = server_port;
                Channel_ask1.MSG.serverip.s_addr = inet_addr(server_ip);
                memcpy(Channel_ask1.MSG.mac, nanodmac, 17);

                tcpcontrol_relink(&Channel_ask1);
                break;

            case RESTART_WIFI:
                rk_printf("restart wifi");
                if(gpstChannelPcb.tcpStream_connectflag == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpStream_sock, 0);
                    closesocket(gpstChannelPcb.tcpStream_sock);
                    gpstChannelPcb.tcpStream_connectflag = CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpControl_connectflag == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpControl_sock, 2);
                    closesocket(gpstChannelPcb.tcpControl_sock);
                    gpstChannelPcb.tcpControl_connectflag= CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpHeart_connectflag == CONNECT_SUCCESS)
                {
                     shutdown(gpstChannelPcb.tcpHeart_sock, 1);
                     closesocket(gpstChannelPcb.tcpHeart_sock);
                     gpstChannelPcb.tcpHeart_connectflag= CONNECT_FAIL;
                }
                RKTaskDelete(TASK_ID_TCPSTREAM_TASK, 0, SYNC_MODE);
                RKTaskDelete(TASK_ID_TCPHEART_TASK, 0, SYNC_MODE);
                RKTaskDelete(TASK_ID_TCPCONTROL_TASK, 0, SYNC_MODE);
                rkos_sleep(100);
                //反初始化，重新进行连接
                rk_wifi_deinit();
                RKTaskDelete(TASK_ID_WIFI_APPLICATION,0,SYNC_MODE);
                rkos_sleep(1000);
                rkos_sleep(1000);
                rkos_sleep(1000);
                rkos_sleep(1000);
                rkos_sleep(1000);
                //rk_wifi5_restart();
                RKTaskCreate(TASK_ID_WIFI_APPLICATION, 0, (void *)WLAN_MODE_STA, SYNC_MODE);

                while(wifi_connect_flag() != 1)  //wificonnect ok
                {
                    rkos_sleep(1000);
                }
                while(!ConfigServer());  //重新连接重启 广播包
                rk_printf("restart get server ip and port ok");
                Channel_ask1.MSG.serverport = server_port;
                Channel_ask1.MSG.serverip.s_addr = inet_addr(server_ip);
                memcpy(Channel_ask1.MSG.mac, nanodmac, 17);

                tcpcontrol_relink(&Channel_ask1);
                break;

            case UDPPACKET_LOSTCMD:
#if 1
                char buf[64] = {0};
                int send_len = 0;
                int len = 0;
                udplost_request udplost_msg;
                CHANNEL_RESP_QUEUE udp_resp;

                printf("send udp lost packet min = %d= %d\n",Broad_ask.udpmsg.minnum, Broad_ask.udpmsg.maxnum);
                rkos_memcpy(&udplost_msg, &Broad_ask.udpmsg, sizeof(udplost_request));

                len = sizeof(udplost_request);
                buf[0] = 2;
                rkos_memcpy(buf+1, &len, sizeof(int));
                rkos_memcpy(buf+5, &udplost_msg, sizeof(udplost_request));
                send_len = 5+sizeof(udplost_request);

                send(gpstChannelPcb.tcpControl_sock, buf, send_len, 0);
                udp_resp.result = 1;
                rkos_queue_send(gpstTcpChannelTaskData, &udp_resp.result, MAX_DELAY);

#endif
                break;

            case UP_Volume:
                int j=0;
                for(j = 1; j < 26; j++)
                {
                    AudioDev_SetVol(gpstChannelPcb.pAudio, j);
                    rkos_sleep(10);
                }
                break;

            case DN_volume:
                int i=25;
                for(i = 25; i>0; i--)
                {
                    AudioDev_SetVol(gpstChannelPcb.pAudio, i);
                    rkos_sleep(10);
                }
                break;

            default:
                printf("tcp channel invold cmd \n");
                break;

        }

    }

}


void RK_BroadTask_Enter(void)
{

    struct sockaddr_in addr, recvaddr;
    int sock = -1;
    const int opt = -1;
    int nb = 0;
    char broad_buf[64] = {0};
    int broad_len = 0;
    socklen_t recvaddrlen ;
    int ret;

    printf("discover_LEN = %d,cmdline_len=%d,connection_len=%d\n",CHANNELS_MESSAGE_LEN,CMD_LINE_LEN, CONNECTION_LINE_LEN);

    while(1)
    {
        gpstChannelPcb.broad_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if(gpstChannelPcb.broad_sock == -1)
        {
            rk_printf("BroadTask Socket fail");
            continue;
        }

        nb = setsockopt(gpstChannelPcb.broad_sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
        if(nb == -1)
        {
            rk_printf("broadtask setsocket fail\n");
            continue;
        }

        addr.sin_family = AF_INET;
        addr.sin_port = htons(3066);
        addr.sin_addr.s_addr = htonl(IPADDR_ANY);
        addr.sin_len = sizeof(addr);

        nb = bind(gpstChannelPcb.broad_sock, (struct sockaddr *)&addr, addr.sin_len);
        if(nb == -1)
        {
            rk_printf("broadtask sock bind fail");
           continue;
        }
        printf("broadtask sock success\n");

        break;
    }
    while(1)
    {
        ret = recvfrom(gpstChannelPcb.broad_sock, broad_buf, 64, 0, (struct sockaddr*)&recvaddr, &recvaddrlen);
        if(ret > 0)
        {
            BroadMessage_Parse(broad_buf, ret);
        }

    }
}

extern uint32 wifi_ip_addr;
uint32 pre_num = 0;


void RK_TcpStreamTask_Enter(void)
{
    struct sockaddr_in serveraddr, clientaddr;
    int sock = -1;
    CHANNEL_ASK_QUEUE tcpstream_ack;
    CHANNEL_RESP_QUEUE tcpstream_resp;
    CHANNEL_ASK_QUEUE udpchannel_ask;
    CHANNEL_MESSAGE msg;
    connectLine connectMsg;
    int session;
    int ret = -1;
    //char buf[TCP_STREAM_RECV_LEN];
    STREAM_DATA buf;
    uint16 recv_len = sizeof(STREAM_DATA);
    struct ip_addr  ipaddr;
    //struct sockaddr udp_serveraddr;
    //socklen_t udp_server_len= sizeof(struct sockaddr);
    //udp_serveraddr.sa_family = AF_INET;
    //udp_serveraddr.

    while(1)
    {
        tcp_udp_server_flag = 0;
        int i = 0;
        while(1)
        {
            printf("RK_TcpStreamTask_Enter\n");
            rkos_queue_receive(gpstTcpStreamTaskData->TcpstreamAskQueue, &tcpstream_ack, MAX_DELAY);
            memcpy(&msg, &tcpstream_ack.MSG, sizeof(CHANNEL_MESSAGE));
            session = tcpstream_ack.session;

            tcpstream_resp.result = RK_ERROR;
            i++;

            if(gpstChannelPcb.tcpStream_connectflag == CONNECT_SUCCESS)
            {
                printf("tcpstream sock alread alive\n");
                closesocket(gpstChannelPcb.tcpStream_sock);
               // rkos_queue_send(gpstTcpStreamTaskData->TcpstreamRespQueue, &tcpstream_resp, MAX_DELAY);
               // continue;
            }


            gpstChannelPcb.tcpStream_connectflag= CONNECT_FAIL;

            gpstChannelPcb.tcpStream_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if( gpstChannelPcb.tcpStream_sock == -1)
            {
                printf("udpStreamTask Socket fail\n");
                rkos_queue_send(gpstTcpStreamTaskData->TcpstreamRespQueue, &tcpstream_resp, MAX_DELAY);
                continue;
            }

            clientaddr.sin_addr.s_addr = htonl(IPADDR_ANY);
            clientaddr.sin_port = htons(TCP_STREAM_PORT);
            clientaddr.sin_family = AF_INET;
            clientaddr.sin_len = sizeof(clientaddr);
#if 1
            ret = bind(gpstChannelPcb.tcpStream_sock, (struct sockaddr *)&clientaddr, clientaddr.sin_len);
            if(ret== -1)
            {
                printf("tcp stream bind fail\n");
                rkos_queue_send(gpstTcpStreamTaskData->TcpstreamRespQueue, &tcpstream_resp, MAX_DELAY);
                closesocket(gpstChannelPcb.tcpStream_sock);
                continue;
            }
#endif
            gpstChannelPcb.tcpStream_connectflag= CONNECT_SUCCESS;
            tcpstream_resp.result = RK_SUCCESS;

            break;
        }

        rkos_queue_send(gpstTcpStreamTaskData->TcpstreamRespQueue, &tcpstream_resp, MAX_DELAY);
        printf("tcpstream connect success\n");

        while(1)
        {
            //if(tcp_udp_server_flag == 1)
                //break;

            //直接接收数据往fifo中仍
           // ret = recvfrom(gpstChannelPcb.tcpStream_sock, &buf, recv_len, 0, &udp_serveraddr, &udp_server_len);
            //printf("r %d \n", set_flag);
            ret = recv(gpstChannelPcb.tcpStream_sock,  &buf, recv_len, 0);
            if(ret > 0)
            {

                if(set_flag == 1)
                {
                    rkos_sleep(2);
                    continue;
                }

#ifndef UDPLOST_PACKET
              //  printf("r = %d,%d\n", buf.num, rkos_GetFreeHeapSize());
                if(pre_num != buf.num)
                {
                    printf("\n n= %d = %d, %d\n", pre_num, buf.num, rkos_GetFreeHeapSize());
                }
                pre_num = buf.num + 1;
                //pre_num;
               //printf("r=%d,l=%d,n=%d\n",ret,buf.stream_len,buf.num);
               TcpStream_WriteFifo(&buf, buf.stream_len);
#else
                if(packet_first == 0)
                {
                    packet_first = 1;
                    TcpStream_WriteFifo(&buf, buf.stream_len);
                    clear_udp_lost_buf(read_num);
                    read_num = buf.num;
                    write_num = buf.num;
                    continue;
                }
               // printf(" %d\n", xPortGetFreeHeapSize());
                //printf("n = %d, r = %d\n", buf.num, read_num);
                //printf("r = %d,w = %d, b= %d\n",read_num,write_num, buf.num);
                if(udpseq_next(read_num, buf.num))
                {
                   TcpStream_WriteFifo(&buf, buf.stream_len);
                   clear_udp_lost_buf(read_num);
                   read_num = buf.num;
                   if(write_num < buf.num)
                        write_num = buf.num;
                }
                else if((read_num > buf.num) || read_num == buf.num)
                {
                    clear_udp_lost_buf(buf.num);
                    printf("lost num \n");
                    if(buf.num > write_num)
                       write_num = buf.num;
                }
                //printf("w= %d",write_num);

                streambuf_push(&buf);
                //rk_count_clk_start();
                streambuf_pop();
                //rkos_delay(10);
                //rk_count_clk_end();
#endif

            }

        }

    }
}

void RK_TcpControlTask_Enter(void)
{
    struct sockaddr_in serveraddr,  clientaddr;
    CHANNEL_ASK_QUEUE tcpcontrol_ask;
    CHANNEL_RESP_QUEUE tcpcontrol_resp;
    CHANNEL_ASK_QUEUE tcpchannel_ask;
    int ret = -1;
    int nb = -1;
    char buf[128] = {0};
    uint32 send_len = 0;
    CHANNEL_MESSAGE msg;
    cmdLine cmdMsg;
    connectLine connectMsg;
    uint32 recv_len;
    uint32 clientport = 0;

    while(1)
    {
        rkos_queue_receive(gpstTcpControlTaskData->TcpcontrolAskQueue, &tcpcontrol_ask, MAX_DELAY);
        int n=0;
        while(1)
        {
            printf("TcpControlTask_Enter\n");
            memcpy(&msg, &tcpcontrol_ask.MSG, sizeof(CHANNEL_MESSAGE));
            tcpcontrol_resp.result = RK_ERROR;

            while(1)
            {
                rkos_sleep(1000);
                if(wifi_connect_flag() == 1)
                    break;
            }

            if(gpstChannelPcb.tcpControl_connectflag == CONNECT_SUCCESS)
            {
                printf("tcp control already connect\n");
                closesocket(gpstChannelPcb.tcpControl_sock);
                //gpstChannelPcb.tcpControl_connectflag= CONNECT_FAIL;
                //rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);
               // continue;
            }
            gpstChannelPcb.tcpControl_connectflag = CONNECT_FAIL;

            gpstChannelPcb.tcpControl_sock= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(gpstChannelPcb.tcpControl_sock == -1)
            {
                rk_printf("TcpcontrolSocket fail\n");
                rkos_sleep(1000);
               // rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);
                continue;
            }
           //rk_printf("tcpcontorl port = %d, server_ip = %d\n", msg.serverport, server_ip);
            serveraddr.sin_addr.s_addr = msg.serverip.s_addr; //inet_addr(ip_addr_buf);
            serveraddr.sin_port = htons(msg.serverport);
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_len = sizeof(serveraddr);
            nb = connect(gpstChannelPcb.tcpControl_sock, (struct sockaddr *)&serveraddr, serveraddr.sin_len);
            if(nb == -1)
            {
                printf("tcpcontrol connect fail\n");
                closesocket(gpstChannelPcb.tcpControl_sock);
                if(rkos_GetFreeHeapSize() > 50*1024)
                {
                    rkos_sleep(1000);
                    n++;
                    continue;
                }
                else if(rkos_GetFreeHeapSize() < 50*1024 || wifi_connect_flag() != 1 || (n>10))
                {
                    tcpchannel_ask.cmd = RESTART_WIFI;
                    rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);
                    rkos_sleep(100);
                    rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &tcpchannel_ask, MAX_DELAY);
                    while(1)
                    {
                        rkos_sleep(1000);
                    }
                }
            }
            gpstChannelPcb.tcpControl_connectflag = CONNECT_SUCCESS;
            printf("tcpcontrol connect OK\n");

            break;
        }


        rkos_sleep(100);
        int len = sizeof(connectLine);
        //connectMsg.channel = msg.channel;
        connectMsg.serverip.s_addr = wifi_ip_addr;
        connectMsg.session = -1;
        connectMsg.socketType = CTLSOCKET;
        memcpy(connectMsg.mac, msg.mac, 17);

        buf[0] = 0;
        send_len = 5+sizeof(connectLine);

        memcpy(buf+1, (char*)&len, 4);
        memcpy(buf+5,(char*)&connectMsg,sizeof(connectLine));

        send(gpstChannelPcb.tcpControl_sock, buf, send_len, 0);
        memset(buf, 0x00, sizeof(buf));

        while(1)
        {
            rkos_sleep(100);
            ret = recv(gpstChannelPcb.tcpControl_sock, buf, 5, 0);
            printf("recv tcp cmdline ok ret : %d  = %d  = %c\n",ret, sizeof(cmdLine), buf[0]);
            debug_hex(buf, 5, 16);
            if(ret == 0)
            {
                rk_printf("serve close socket");
                tcpchannel_ask.cmd = REBROAD_CMD;
                rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);
                rkos_sleep(100);
                rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &tcpchannel_ask, MAX_DELAY);
                while(1)
                {
                    rkos_sleep(1000);
                }
                //continue;
            }
            if(ret > 0 )
            {
                if(ret == 5)
                {
                    rkos_memcpy(&recv_len,buf+1,4);

                    ret = recv(gpstChannelPcb.tcpControl_sock, &buf[6], recv_len, 0);
                    printf("111recv tcp cmdline ok ret : %d  = %d \n",ret, recv_len);
                    if( ret == sizeof(cmdLine))
                    {
                        // SetSample_flag = 0;
                         tcpcontrol_resp.result = RK_SUCCESS;
                         rkos_memcpy(&cmdMsg, &buf[6], ret);
                         gpstChannelPcb.session = cmdMsg.session;
                         speaker_channel = cmdMsg.channel;
                         //tcpcontrol_resp.session = cmdMsg.session;
                         printf("\ncmdMsg.samplerate = %d , %d",cmdMsg.samplerate,rkos_GetFreeHeapSize());

                         set_flag = 1;
                         AudioDev_SetSampleRate(gpstChannelPcb.pAudio, 0, cmdMsg.samplerate);
                         cursample = cmdMsg.samplerate;
                         set_flag = 0;
                         AudioDev_SetBit(gpstChannelPcb.pAudio, 0, 16);
                         setVolume(cmdMsg.volume);
                         // AudioDev_SetVol(gpstChannelPcb.pAudio, 31);

                         break;
                    }

                }
            }
        }
        rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);

        rkos_memset(buf, 0x00, 128);
        recv_len = 0;

        while(1)
        {

            rkos_sleep(10);
            ret = recv(gpstChannelPcb.tcpControl_sock, buf, 5, 0);
            //printf("ret = %d\n", ret);
           // debug_hex(buf, 5, 16);
            if(ret > 0)
            {
               if(ret == 5)
               {
                    rkos_memcpy(&recv_len, buf+1, 4);
                    ret = recv(gpstChannelPcb.tcpControl_sock, &buf[6], recv_len, 0);
                    TcpMessage_Parse(&buf[6], ret);
                }
            }
#if 1
            else
            {
                printf("tcp control recv comdline fail\n");
                tcp_udp_server_flag = 1;

                AudioDev_SetVol(gpstChannelPcb.pAudio, 0);
                rkos_sleep(10);
                #if 0
                if( gpstChannelPcb.tcpControl_connectflag = CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpControl_sock, 2);
                    closesocket(gpstChannelPcb.tcpControl_sock);
                    gpstChannelPcb.tcpControl_connectflag = CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpStream_sock == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpStream_sock, 2);
                    closesocket(gpstChannelPcb.tcpStream_sock);
                    gpstChannelPcb.tcpStream_connectflag = CONNECT_FAIL;
                }
                if(gpstChannelPcb.tcpHeart_connectflag == CONNECT_SUCCESS)
                {
                    shutdown(gpstChannelPcb.tcpHeart_sock, 2);
                    closesocket(gpstChannelPcb.tcpHeart_sock);
                    gpstChannelPcb.tcpHeart_connectflag = CONNECT_FAIL;
                }
                tcpchannel_ask.cmd = REBROAD_CMD;
                #endif
                rkos_sleep(100);
                tcpchannel_ask.cmd = REBROAD_CMD;
                rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &tcpchannel_ask, MAX_DELAY);
                break;
            }
#endif
        }

    }

    while(1)//等待外部命令删除该线程
    {
        rkos_sleep(1000);
    }

}

void RK_TcpHeartTask_Enter(void)
{

    struct sockaddr_in serveraddr,  clientaddr;
    CHANNEL_ASK_QUEUE tcpheart_ask;
    CHANNEL_RESP_QUEUE tcpheart_resp;
    int ret = -1;
    int nb = -1;
    char buf[128] = {0};
    uint32 send_len = 0;
    uint32 recv_len;
    CHANNEL_MESSAGE msg;
    //cmdLine cmdMsg;
    //connectLine connectMsg;
    heartBeat heartline;
    printf("rk_TcpHeartTask_Enter\n");


    rkos_queue_receive(gpstTcpHeartTaskData->TcpHeartAskQueue, &tcpheart_ask, MAX_DELAY);
    while(1)
    {
        printf("TcpHeartTask_Enter\n");
        memcpy(&msg, &tcpheart_ask.MSG, sizeof(CHANNEL_MESSAGE));
        tcpheart_resp.result = RK_ERROR;

        if(gpstChannelPcb.tcpHeart_connectflag == CONNECT_SUCCESS)
        {
            printf("tcp heart already connect\n");
            closesocket(gpstChannelPcb.tcpHeart_sock);
            //gpstChannelPcb.tcpHeart_connectflag= CONNECT_FAIL;
            //continue;
        }
        gpstChannelPcb.tcpHeart_connectflag= CONNECT_FAIL;

        gpstChannelPcb.tcpHeart_sock= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(gpstChannelPcb.tcpHeart_sock == -1)
        {
            rk_printf("TcpcHeart socket fail\n");
            continue;
        }
        gpstChannelPcb.tcpHeart_connectflag = CONNECT_SUCCESS;
        break;
    }
    tcpheart_resp.result = RK_SUCCESS;
    rkos_queue_send(gpstTcpHeartTaskData->TcpHeartRespQueue, &tcpheart_resp, MAX_DELAY);

    serveraddr.sin_addr.s_addr = msg.serverip.s_addr; //
    serveraddr.sin_port = htons(3288);//0,1 3288; 2,3 3289; 4,5 3290 0 :3288, 1:3289 ,2:3290; 3:3291; 4:3292; 5:3293
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_len = sizeof(serveraddr);

    buf[0] = 2;
    heartline.session = gpstChannelPcb.session;
    heartline.channel = speaker_channel;
    heartline.seq = 0;
    send_len = 5 + sizeof(heartBeat);

    rkos_memcpy(buf+1, (char*)send_len, 4);
    rkos_memcpy(buf+5, (char*)&heartline, sizeof(heartBeat));
    rkos_sleep(100);
    while(1)
    {
        //printf("u\n");
        #if 1
        if(rkos_GetFreeHeapSize() > 30*1024)
        {
            sendto(gpstChannelPcb.tcpHeart_sock, buf, send_len, 0, (const struct sockaddr*)&serveraddr, serveraddr.sin_len);
            //send(gpstChannelPcb.tcpHeart_sock, buf, send_len, 0);
            heartline.seq ++;
            rkos_memcpy(buf+5, (char*)&heartline, sizeof(heartBeat));
            rkos_sleep(10);
        }
        else
        {
            rkos_sleep(1000);
        }
        #endif
        //rkos_sleep(1000);

    }

}

void tcpcontrol_close(void)
{
    CHANNEL_ASK_QUEUE tcpchannel_ask;
    CHANNEL_RESP_QUEUE tcpcontrol_resp;

    AudioDev_SetVol(gpstChannelPcb.pAudio, 0);

    if(gpstTcpControlTaskData->speaker == 1)  //tcp没有连接speaker上来需要重连的时候需要退出 tcp队列
    {
        rk_printf("no channle speaker ,wifi restart");
        tcpcontrol_resp.result = RK_ERROR;
        rkos_queue_send(gpstTcpControlTaskData->TcpcontrolRespQueue, &tcpcontrol_resp, MAX_DELAY);
    }

    printf("wifi relink tcp udp = %d \n", rkos_GetFreeHeapSize());
    speaker_channel = 0;  //待配置的channel

#if 1
    if(gpstChannelPcb.tcpHeart_connectflag == CONNECT_SUCCESS)
    {
         shutdown(gpstChannelPcb.tcpHeart_sock, 2);
         closesocket(gpstChannelPcb.tcpHeart_sock);
         gpstChannelPcb.tcpHeart_connectflag= CONNECT_FAIL;
    }

    RKTaskDelete(TASK_ID_TCPHEART_TASK, 0, SYNC_MODE);

    if(gpstChannelPcb.tcpStream_connectflag== CONNECT_SUCCESS)
    {
        shutdown(gpstChannelPcb.tcpStream_sock, 2);
        closesocket(gpstChannelPcb.tcpStream_sock);
        gpstChannelPcb.tcpStream_connectflag = CONNECT_FAIL;
    }
    if(gpstChannelPcb.tcpControl_connectflag == CONNECT_SUCCESS)
    {
        shutdown(gpstChannelPcb.tcpControl_sock, 2);
        closesocket(gpstChannelPcb.tcpControl_sock);
        gpstChannelPcb.tcpControl_connectflag= CONNECT_FAIL;
    }


     RKTaskDelete(TASK_ID_TCPSTREAM_TASK, 0, SYNC_MODE);
     RKTaskDelete(TASK_ID_TCPCONTROL_TASK, 0, SYNC_MODE);
     //rkos_delay(100);
#endif

     tcpchannel_ask.cmd = BROAD_CMD;
      rk_printf("tcp BROAD_CMD");
     rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &tcpchannel_ask, MAX_DELAY);

}

void tcpcontrol_relink(void* channel_ask)
{
    CHANNEL_ASK_QUEUE *Channel_ask1;
    CHANNEL_RESP_QUEUE Channel_control_resp;
    CHANNEL_RESP_QUEUE Channel_stream_resp;
    CHANNEL_RESP_QUEUE channel_tcpheart_resp;

    //rk_printf("relink port = %d, server_ip = %s\n", server_port, server_ip);

    Channel_ask1 = (CHANNEL_ASK_QUEUE*)channel_ask;

    RKTaskCreate(TASK_ID_TCPSTREAM_TASK, 0, NULL, SYNC_MODE);
    RKTaskCreate(TASK_ID_TCPHEART_TASK, 0, NULL, SYNC_MODE);
    RKTaskCreate(TASK_ID_TCPCONTROL_TASK, 0, NULL, SYNC_MODE);
    rkos_delay(100);

    gpstTcpControlTaskData->speaker = 1;
    rkos_queue_send(gpstTcpControlTaskData->TcpcontrolAskQueue, Channel_ask1, MAX_DELAY);
    rkos_queue_receive(gpstTcpControlTaskData->TcpcontrolRespQueue, &Channel_control_resp, MAX_DELAY);
    gpstTcpControlTaskData->speaker = 0;
    if(Channel_control_resp.result == RK_SUCCESS)
    {

        rkos_queue_send(gpstTcpHeartTaskData->TcpHeartAskQueue, Channel_ask1, MAX_DELAY);
        rkos_queue_receive(gpstTcpHeartTaskData->TcpHeartRespQueue, &channel_tcpheart_resp, MAX_DELAY);
        if(channel_tcpheart_resp.result == RK_SUCCESS)
        {

            rkos_queue_send(gpstTcpStreamTaskData->TcpstreamAskQueue, Channel_ask1, MAX_DELAY);
            rkos_queue_receive(gpstTcpStreamTaskData->TcpstreamRespQueue, &Channel_stream_resp, MAX_DELAY);

            if(Channel_stream_resp.result == RK_SUCCESS)
            {
#if 0
                 gSysConfig.SpeakConfig.serverip = Channel_ask.MSG.serverip.s_addr;
                 gSysConfig.SpeakConfig.serverport = Channel_ask.MSG.serverport;
                 gSysConfig.SpeakConfig.channel = Channel_ask.MSG.channel;
                 //gSysConfig.SpeakConfig.bitrate = Channel_ask.MSG.bitrate;
                 //gSysConfig.SpeakConfig.samplerate = Channel_ask.MSG.samplerate;
                 //gSysConfig.SpeakConfig.volume = Channel_ask.MSG.volume;
                 gSysConfig.SpeakConfig.flag = 1;
#endif
            }
        }
    }
}





/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint8 streambuf[2*TCP_STREAM_RECV_LEN] = {0};
uint8 left_streambuf[1] = {0};
uint8 left_streamlen = 0;
//uint8 flag = 0;
static void TcpStream_WriteFifo(void *mbuf, int mbuf_len)
{
    STREAM_DATA *pstream = (STREAM_DATA*)mbuf;
    uint16 i=0;
    uint16 k = 0;


    if(pstream->stream_len != 1024)
        return;

    k = pstream->stream_len/2;

    for(i = 0; i < k; i++)
    {
        streambuf[4 * i] = pstream->stram_data[i * 2];
        streambuf[4 * i + 1] = pstream->stram_data[i * 2 + 1];
        streambuf[4 * i + 2] = pstream->stram_data[i * 2];
        streambuf[4 * i + 3] = pstream->stram_data[i * 2 + 1];
     }


     fifoDev_Write(gpstChannelPcb.hFifo, streambuf, k*4, SYNC_MODE, NULL);

     return;
}

static void TcpMessage_Parse(char *mbuf, int mbuf_len)
{
    cmdLine cmdMsg;
    CHANNEL_ASK_QUEUE Channel_ask;
    int i = 0;

    //return;
    if(mbuf_len == sizeof(cmdLine))
    {
        rkos_memcpy(&cmdMsg, mbuf, mbuf_len);

        //printf("reset************************************************\n");
        //gpstChannelPcb.session = cmdMsg.session;
       // AudioDev_SetVol(gpstChannelPcb.pAudio, 0);
        //rkos_sleep(500);
        rk_printf("sp = %d\n",cmdMsg.samplerate);
        //rk_printf("a");
     #if 1
        AudioDev_SetVol(gpstChannelPcb.pAudio, 0);
        if(cursample != cmdMsg.samplerate);
        {
           set_flag = 1;
           AudioDev_SetSampleRate(gpstChannelPcb.pAudio, 0, cmdMsg.samplerate);
           cursample = cmdMsg.samplerate;
           set_flag = 0;
        }

        setVolume(cmdMsg.volume);
      // AudioDev_SetVol(gpstChannelPcb.pAudio, 31);
      #endif
       // fifoDev_SetTotalSize(gpstChannelPcb.hFifo , 0xffffffff);


    }
    return;
}

static void BroadMessage_Parse(char *mbuf, int mbuf_len)
{
    CHANNEL_ASK_QUEUE Broad_ask;

    Broad_ask.cmd = BROAD_CMD;

    printf("broadmessage_parse channel_message_len = %d\n", CHANNELS_MESSAGE_LEN);

    if(mbuf_len == CHANNELS_MESSAGE_LEN)
    {
        memcpy(&Broad_ask.MSG, mbuf, sizeof(CHANNEL_MESSAGE));
        printf("serverip:0x%4x,port: %d\n",Broad_ask.MSG.serverip,\
            Broad_ask.MSG.serverport);
        rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &Broad_ask, MAX_DELAY);
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
rk_err_t RK_TcpStreamTask_Init(void *pvParameters, void *arg)
{
    TCPSTREAM_DATA_BLOCK *pTcpstreamTaskData;

    pTcpstreamTaskData = rkos_memory_malloc(sizeof(TCPSTREAM_DATA_BLOCK));
    if(pTcpstreamTaskData == NULL)
    {
        rk_printf("RK_ChannelTaskInit malloc fail");
        return RK_ERROR;
    }
    memset(pTcpstreamTaskData, 0x00, sizeof(TCPSTREAM_DATA_BLOCK));
    pTcpstreamTaskData->TcpstreamAskQueue= rkos_queue_create(1, sizeof(CHANNEL_ASK_QUEUE));
    pTcpstreamTaskData->TcpstreamRespQueue = rkos_queue_create(1, sizeof(CHANNEL_RESP_QUEUE));
    gpstTcpStreamTaskData = pTcpstreamTaskData;

    return RK_SUCCESS;
}

rk_err_t RK_TcpStreamTask_Deinit(void *pvParameters)
{
    rkos_queue_delete(gpstTcpStreamTaskData->TcpstreamAskQueue);
    rkos_queue_delete(gpstTcpStreamTaskData->TcpstreamRespQueue);
    rkos_memory_free(gpstTcpStreamTaskData);
    return RK_SUCCESS;

}

rk_err_t RK_TcpControlTask_Init(void *pvParameters, void *arg)
{
    TCPCONTROL_DATA_BLOCK *pTcpcontrolTaskData;

    pTcpcontrolTaskData = rkos_memory_malloc(sizeof(TCPCONTROL_DATA_BLOCK));
    if(pTcpcontrolTaskData == NULL)
    {
        rk_printf("RK_ChannelTaskInit malloc fail");
        return RK_ERROR;
    }
    memset(pTcpcontrolTaskData, 0x00, sizeof(TCPCONTROL_DATA_BLOCK));
    pTcpcontrolTaskData->TcpcontrolAskQueue = rkos_queue_create(1, sizeof(CHANNEL_ASK_QUEUE));
    pTcpcontrolTaskData->TcpcontrolRespQueue = rkos_queue_create(1, sizeof(CHANNEL_RESP_QUEUE));
    gpstTcpControlTaskData = pTcpcontrolTaskData;

    return RK_SUCCESS;
}
rk_err_t RK_TcpControlTask_Deinit(void *pvParameters)
{
    rkos_queue_delete(gpstTcpControlTaskData->TcpcontrolAskQueue);
    rkos_queue_delete(gpstTcpControlTaskData->TcpcontrolRespQueue);
    rkos_memory_free(gpstTcpControlTaskData);
    return RK_SUCCESS;
}


rk_err_t RK_TcpBroadTask_Init(void *pvParameters, void *arg)
{
   TCPBROAD_DATA_BLOCK *pTcpbroadTaskData;

    pTcpbroadTaskData = rkos_memory_malloc(sizeof(TCPBROAD_DATA_BLOCK));
    if(pTcpbroadTaskData == NULL)
    {
        rk_printf("RK_ChannelTaskInit malloc fail");
        return RK_ERROR;
    }
    memset(pTcpbroadTaskData, 0x00, sizeof(TCPBROAD_DATA_BLOCK));
    pTcpbroadTaskData->TcpBroadAskQueue = rkos_queue_create(1, sizeof(CHANNEL_ASK_QUEUE));
    pTcpbroadTaskData->TcpBroadRespQueue = rkos_queue_create(1, sizeof(CHANNEL_RESP_QUEUE));
    gpstTcpBroadTaskData = pTcpbroadTaskData;

    return RK_SUCCESS;
}
rk_err_t RK_TcpBroadTask_Deinit(void *pvParameters)
{
    rkos_queue_delete(gpstTcpBroadTaskData->TcpBroadAskQueue);
    rkos_queue_delete(gpstTcpBroadTaskData->TcpBroadRespQueue);
    rkos_memory_free(gpstTcpBroadTaskData);
    return RK_SUCCESS;
}

rk_err_t RK_TcpChannelTask_Init(void *pvParameters, void *arg)
{
   TCPCHANNEL_DATA_BLOCK *pTcpchannelTaskData;

    FREQ_EnterModule(FREQ_BLON);

    pTcpchannelTaskData = rkos_memory_malloc(sizeof(TCPBROAD_DATA_BLOCK));
    if(pTcpchannelTaskData == NULL)
    {
        rk_printf("RK_ChannelTaskInit malloc fail");
        return RK_ERROR;
    }
    memset(pTcpchannelTaskData, 0x00, sizeof(TCPBROAD_DATA_BLOCK));
    pTcpchannelTaskData->TcpChannelAskQueue = rkos_queue_create(1, sizeof(CHANNEL_ASK_QUEUE));
    pTcpchannelTaskData->TcpChannelRespQueue = rkos_queue_create(1, sizeof(CHANNEL_RESP_QUEUE));
    gpstTcpChannelTaskData = pTcpchannelTaskData;

    return RK_SUCCESS;
}
rk_err_t RK_TcpChannelTask_Deinit(void *pvParameters)
{
    rkos_queue_delete(gpstTcpChannelTaskData->TcpChannelAskQueue);
    rkos_queue_delete(gpstTcpChannelTaskData->TcpChannelRespQueue);
    rkos_memory_free(gpstTcpChannelTaskData);

    FREQ_ExitModule(FREQ_BLON);
    return RK_SUCCESS;
}


rk_err_t RK_TcpHeartTask_Init(void *pvParameters, void *arg)
{

    TCPHEART_DATA_BLOCK *pTcpHeartTaskData;

    pTcpHeartTaskData = rkos_memory_malloc(sizeof(TCPHEART_DATA_BLOCK));
    if(pTcpHeartTaskData == NULL)
    {
       rk_printf("RK_ChannelTaskInit malloc fail");
       return RK_ERROR;
    }
    memset(pTcpHeartTaskData, 0x00, sizeof(TCPHEART_DATA_BLOCK));
    pTcpHeartTaskData->TcpHeartAskQueue = rkos_queue_create(1, sizeof(CHANNEL_ASK_QUEUE));
    pTcpHeartTaskData->TcpHeartRespQueue = rkos_queue_create(1, sizeof(CHANNEL_RESP_QUEUE));
    gpstTcpHeartTaskData = pTcpHeartTaskData;

    return RK_SUCCESS;
}

rk_err_t RK_TcpHeartTask_Deinit(void *pvParameters)
{
    rkos_queue_delete(gpstTcpHeartTaskData->TcpHeartAskQueue);
    rkos_queue_delete(gpstTcpHeartTaskData->TcpHeartRespQueue);
    rkos_memory_free(gpstTcpHeartTaskData);
    return RK_SUCCESS;
}

#ifdef UDPLOST_PACKET
static int udpseq_next(uint32 a, uint32 b) {
    if (a == 0xFFFFFFFF && b == 0)
        return 1;

    if (a + 1 == b)
        return 1;

    return 0;
}


static void streambuf_push(STREAM_DATA *buf)
{
    UDP_LOST_BUF *recv_buf = &udp_lost_buf[UDPLOSTIDX(buf->num)];
    uint32 start = 0;
    uint32 end = 0;
    uint32 i = 0;
    UDP_LOST_BUF *rst_buf;
    CHANNEL_ASK_QUEUE udpchannel_ask;
    CHANNEL_RESP_QUEUE udpchannel_resp;

    if(write_num < buf->num && !udpseq_next(write_num, buf->num))
    {
        start = write_num+1;
        end = buf->num-1;

        for(i = start; i<=end; i++)
        {
            rst_buf = &udp_lost_buf[UDPLOSTIDX(i)];
            rst_buf->seq = i;
            rst_buf->ready = 0;
            rst_buf->rsd_count = 1;
            rst_buf->rsd_end = end;
        }

        udpchannel_ask.cmd = UDPPACKET_LOSTCMD;
        udpchannel_ask.udpmsg.udpcmd = UDPLOST;
        udpchannel_ask.udpmsg.minnum= start;
        udpchannel_ask.udpmsg.maxnum= end;
        udpchannel_ask.udpmsg.session = gpstChannelPcb.session;

        uint8 mbuf[64];
        int len = sizeof(udplost_request);
        mbuf[0] = 1;

        rkos_memcpy(mbuf+1, &len , sizeof(int));
        rkos_memcpy(mbuf+5,  &udpchannel_ask.udpmsg, len);

       // printf("send udp lost packet min = %d= %d\n",start, end);


        //if(gpstChannelPcb.tcpControl_sock != -1)
        //{
            send(gpstChannelPcb.tcpControl_sock, mbuf, len+5, 0);
       // }
        //rkos_queue_send(gpstTcpChannelTaskData->TcpChannelAskQueue, &udpchannel_ask, MAX_DELAY);
        //rkos_queue_receive(gpstTcpChannelTaskData->TcpChannelRespQueue, &udpchannel_resp, MAX_DELAY);
    }

    if(recv_buf->data != NULL)
    {
        clear_udp_lost_buf(buf->num);
    }
    //printf("pnum = %d, len = %d\n", buf->num, buf->stream_len);
    recv_buf->data = rkos_memory_malloc(sizeof(STREAM_DATA));
    if(recv_buf->data == NULL)
    {
        printf("stream udp buf mallco fail\n");
        while(1);
    }

    rkos_memcpy(recv_buf->data, buf, sizeof(STREAM_DATA));
    //printf("recv_buf = %d, recv->len = %d\n", recv_buf->data->num, recv_buf->data->stream_len);
    recv_buf->rsd_count = 0;
    recv_buf->ready = 1;

   // printf("buf_num = %d num = %d, rst_end = %d\n",buf->num,  recv_buf->data->num, recv_buf->rsd_end);

    if(write_num < buf->num)
    {
        write_num = buf->num;
    }

}


static void streambuf_pop(void)
{
    UDP_LOST_BUF *buf = &udp_lost_buf[UDPLOSTIDX(read_num + 1)];

    //printf("r = %d,w = %d, b= %d\n",r,w, buf->data->num);
    //printf("ready = %d\n", buf->ready);
    //printf("ready = %d,write_num = %d read_num = %d,rsd_cound = %d\n", buf->ready,write_num, read_num, buf->rsd_count);
    if (read_num == write_num) {
        //printf("Read too fast, nothing can pop in buffer.\n");
        return;
    }

    if (buf->ready) {
        TcpStream_WriteFifo(buf->data, buf->data->stream_len);
        clear_udp_lost_buf(read_num);
        read_num = buf->data->num;
        // Pop next.
        streambuf_pop();
    } else {  // No ready for send.
        if (buf->rsd_count > 10) {
            // Sorry, we cannot wait for this packet, it's time to drop it.
            uint32 start = buf->seq;
            uint32 end = buf->rsd_end;
            uint32 drop_count = end - start + 1, i;

            for (i = start; i <= end; i++)
                clear_udp_lost_buf(i);

            printf("\n!!!!!!!!!!!!\nReason: Resend fail, loss %d packets, %d - %d.\n", drop_count, start, end);
            //printf("lost\n");
            // Pop next to send buffer.
            read_num = end;
            streambuf_pop();
        } else {
            buf->rsd_count++;
        }
    }

}

void clear_udp_lost_buf(uint32 seqno)
{
    UDP_LOST_BUF *buf = &udp_lost_buf[UDPLOSTIDX(seqno)];

    if (buf->data) {
        rkos_memory_free(buf->data);
        buf->data = NULL;
    }

    memset(buf, 0, sizeof(UDP_LOST_BUF));
}
#endif

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





