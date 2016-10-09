#include "BspConfig.h"

//#ifdef _PING_SHELL_
#include "RKOS.h"

#include "../lwip-1.4.1/src/include/lwip/sockets.h"
#include "../lwip-1.4.1/src/include/lwip/raw.h"
#include "../lwip-1.4.1/src/include/ipv4/lwip/icmp.h"
#include "../lwip-1.4.1/src/include/ipv4/lwip/inet_chksum.h"
#include "../lwip-1.4.1/src/include/ipv4/lwip/ip.h"

#define PING_IP "192.168.1.1"
#define PING_TO 1000
#define PING_ID 0xABCD
#define BUF_SIZE 1500
#define PING_STACK_SIZE 1024
#define PING_STACK_PRO 8
#define PING_DEFAULT_COUNT 4

static unsigned short ping_seq = 0;
static int infinite_loop = 0, ping_count = 0, data_size = 0, ping_interval = 0, ping_call = 0;
static char ping_ip[16];
HTC ping_task = NULL;

static void generate_ping_echo(unsigned char *buf, int size)
{
    int i;
    struct icmp_echo_hdr *pecho;

    for (i = 0; i < size; i++)
    {
        buf[sizeof(struct icmp_echo_hdr) + i] = (unsigned char) i;
    }

    pecho = (struct icmp_echo_hdr *) buf;
    ICMPH_TYPE_SET(pecho, ICMP_ECHO);
    ICMPH_CODE_SET(pecho, 0);
    pecho->chksum = 0;
    pecho->id = PING_ID;
    pecho->seqno = htons(++ping_seq);
    //pecho->seqno = ++ ping_seq;

    //Checksum includes icmp header and data. Need to calculate after fill up icmp header
    pecho->chksum = inet_chksum(pecho, sizeof(struct icmp_echo_hdr) + size);
}

void ping_test(void *param)
{
    int i, ping_socket, ret;
    int pint_timeout = PING_TO;
    struct sockaddr_in to_addr, from_addr;
    int from_addr_len = sizeof(struct sockaddr_in);
    int ping_size, reply_size;
    unsigned char ping_buf[BUF_SIZE], reply_buf[BUF_SIZE];
    unsigned int ping_time, reply_time, total_time = 0, received_count = 0;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *pecho;

    //Ping size = icmp header(8 bytes) + data size
    ping_size = sizeof(struct icmp_echo_hdr) + data_size;
    rk_printf("[%s] PING %s %d(%d) bytes of data", __FUNCTION__, ping_ip, data_size, sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr) + data_size);
    rk_printf("ping_count=%d, ping_interval = %d", ping_count,ping_interval);
    for (i = 0; (i < ping_count) || (infinite_loop == 1); i ++)
    {
        ping_socket = socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP);
        setsockopt(ping_socket, SOL_SOCKET, SO_RCVTIMEO, &pint_timeout, sizeof(pint_timeout));

        to_addr.sin_len = sizeof(to_addr);
        to_addr.sin_family = AF_INET;
        to_addr.sin_addr.s_addr = inet_addr(ping_ip);

        generate_ping_echo(ping_buf, data_size);
        ret = sendto(ping_socket, ping_buf, ping_size, 0, (struct sockaddr *) &to_addr, sizeof(to_addr));
        rk_count_clk_start();
        reply_size = recvfrom(ping_socket, reply_buf, sizeof(reply_buf), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_len);
        reply_time = rk_count_clk_end();
        if (reply_size >= (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
        {
            iphdr = (struct ip_hdr *)reply_buf;
            pecho = (struct icmp_echo_hdr *)(reply_buf + (IPH_HL(iphdr) * 4));

            if ((pecho->id == PING_ID) && (pecho->seqno == htons(ping_seq)))
            {
                rk_printf("%d bytes from %s: TTL=%d time=%d ms",reply_size - sizeof(struct ip_hdr), inet_ntoa(from_addr.sin_addr), iphdr->_ttl, reply_time);
                received_count++;
                total_time += reply_time;
            }
        }
        else
        {
            rk_printf("[%s] Request timeout for icmp_seq %d", __FUNCTION__, ping_seq);
        }
        close(ping_socket);
        rkos_sleep(ping_interval * 1000);
    }
    rk_printf("[%s] %d packets transmitted, %d received, %d%% packet loss, average %d ms", __FUNCTION__, ping_count, received_count, (ping_count-received_count)*100/ping_count, total_time/received_count);
    RKTaskDelete2(NULL);
}

void do_ping_call(char *ip, int size, int count, int interval)
{
    if ((sizeof(struct icmp_echo_hdr) + size) > BUF_SIZE)
    {
        printf("\n\r%s BUF_SIZE(%d) is too small", __FUNCTION__, BUF_SIZE);
        return;
    }

    if (ip == NULL)
        strcpy(ping_ip, PING_IP);
    else
        strcpy(ping_ip, ip);

    ping_call = 0;
    ping_seq = 0;
    if (size == 0)
    {
        data_size = 24;
    }
    else
    {
        data_size = size;
    }
    ping_interval = interval;

    if (count == 0)
    {
        infinite_loop = 0;
        ping_count = 4;
    }
    else
    {
        infinite_loop = 0;
        ping_count = count;
    }
    if ((rk_err_t)RKTaskCreate2(ping_test, NULL, NULL, ((char *)"ping_test"), PING_STACK_SIZE, PING_STACK_PRO, NULL) == RK_ERROR)
        rk_printf("%s xTaskCreate failed", __FUNCTION__);
}

void do_ping_test(char *ip, int size, int count, int interval)
{
    if ((sizeof(struct icmp_echo_hdr) + size) > BUF_SIZE)
    {
        printf("\n\r%s BUF_SIZE(%d) is too small", __FUNCTION__, BUF_SIZE);
        return;
    }

    if (ip == NULL)
        strcpy(ping_ip, PING_IP);
    else
        strcpy(ping_ip, ip);

    ping_call = 0;
    ping_seq = 0;
    data_size = size;
    ping_interval = interval;

    if (count == 0)
    {
        infinite_loop = 1;
        ping_count = 0;
    }
    else
    {
        infinite_loop = 0;
        ping_count = count;
    }

    if ((rk_err_t)RKTaskCreate2(ping_test, NULL, NULL, ((char *)"ping_test"), PING_STACK_SIZE, PING_STACK_PRO, NULL) == RK_ERROR)
        printf("\n\r%s xTaskCreate failed", __FUNCTION__);
}


SHELL API rk_err_t Ping_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
    uint8 *psize, *pcount, *pIP_addr;
    uint8 size_len = 0, count_len = 0, ip_addr_len = 0;
    uint8 *size = NULL, *count = NULL, *ip_addr = NULL;
    uint16 size_num = 0, count_num = 0;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    while ((*(pstr + i) != NULL))
    {
        if (*(pstr + i) == '-')
        {
            ++i;
            if (*(pstr + i) == 'l')
            {
                i += 2;
                psize = pstr + i;
                while ((*(pstr + i) != ' ') && (*(pstr + i) != NULL))
                {
                    ++i;
                    ++size_len;
                }
                size = rkos_memory_malloc(size_len + 1);
                if (size != NULL)
                {
                    rkos_memcpy(size, psize, size_len);
                    size[size_len] = '\0';
                    printf ("size:%s\n", size);
                }
            }
            else if (*(pstr + i) == 'n')
            {
                i += 2;
                pcount = pstr + i;
                while ((*(pstr + i) != ' ') && (*(pstr + i) != NULL))
                {
                    ++i;
                    ++count_len;
                }
                count = rkos_memory_malloc(count_len + 1);
                if (count != NULL)
                {
                    rkos_memcpy(count, pcount, count_len);
                    count[count_len] = '\0';
                    printf ("count:%s\n", count);
                }
            }
        }
        else if (*(pstr + i) == ' ')
            ++i;
        else if (*(pstr + i) != NULL)
        {
            pIP_addr = pstr + i;
            while ((*(pstr + i) != ' ') && (*(pstr + i) != NULL))
            {
                ++i;
                ++ip_addr_len;
            }
            ip_addr = rkos_memory_malloc(ip_addr_len + 1);
            if (ip_addr != NULL)
            {
                rkos_memcpy(ip_addr, pIP_addr, ip_addr_len);
                ip_addr[ip_addr_len] = '\0';
                printf("ip_addr:%s\n", ip_addr);
            }
        }
    }

    size_num = String2Num(size);
    count_num = String2Num(count);
    //printf("size_num=%d, count_num=%d\n", size_num, count_num);

    //do_ping_test(ip_addr, size_num, count_num, 10);
    //do_ping_test(NULL, 120, 0, 10);
    do_ping_call(ip_addr, size_num, count_num, 5);
    //do_ping_call(ip_addr, 120, 0, 10);

    if (size != NULL)
        rkos_memory_free(size);
    if (count != NULL)
        rkos_memory_free(count);
    if (ip_addr != NULL)
        rkos_memory_free(ip_addr);

    return RK_SUCCESS;
}

//#endif
