/*
 * Apple RTP protocol handler. This file is part of Shairport.
 * Copyright (c) James Laird 2013
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/def.h"

#include "commons.h"
#include "airplay_player.h"
#include "type.h"
#include "external.h"

// only one RTP session can be active at a time.
static int running = 0;
static int rtp_please_shutdown = 0;

static SOCKADDR rtp_client;
static int sock;

static HTC rtp_thread;

#define RTP_STACK_SIZE 128

static void rtp_receiver(void *arg) {
    // we inherit the signal mask (SIGUSR1)
    uint8_t packet[2048], *pktp;
    ssize_t nread;

    while (1) {
        if (rtp_please_shutdown){
            AIRPLAY_DEBUGF("rtp_please_shutdown: %d\n", rtp_please_shutdown);
            break;
        }
        nread = recv(sock, packet, sizeof(packet), 0);
        //printf("nread: %d\r\n", nread);
        if (nread < 0)
            break;

        ssize_t plen = nread;
        uint8_t type = packet[1] & ~0x80;
        if (type == 0x54){ // sync
            //printf("\ncontrol packet\n");
            continue;
        }
        if (type == 0x60 || type == 0x56) {   // audio data / resend
            pktp = packet;
            if (type==0x56) {
                pktp += 4;
                plen -= 4;
            }
            seq_t seqno = ntohs(*(unsigned short *)(pktp+2));

            pktp += 12;
            plen -= 12;

            // check if packet contains enough content to be reasonable
            if (plen >= 16) {
                player_put_packet(seqno, pktp, plen);
                continue;
            }
            if (type == 0x56 && seqno == 0) {
                AIRPLAY_DEBUGF("resend-related request packet received, ignoring.\n");
                continue;
            }
            AIRPLAY_DEBUGF("Unknown RTP packet of type 0x%02X length %d seqno %d\n", type, nread, seqno);
            continue;
        }
        AIRPLAY_DEBUGF("Unknown RTP packet of type 0x%02X length %d\n", type, nread);
    }

    AIRPLAY_DEBUGF("RTP thread interrupted. terminating.\n");
    AIRPLAY_DEBUGF("rtp_receiver close socket fd: %d\n", sock);
    close(sock);

#if 0
    RKTaskDelete2(NULL);
    return;
#else
    while(1)
        rkos_delay(5000); //5s
#endif
}

static const char* f_a(struct sockaddr *fsa) {
    static char string[16];
    void *addr;
    {
        struct sockaddr_in *sa = (struct sockaddr_in*)(fsa);
        addr = &(sa->sin_addr);
    }
    return inet_ntop(fsa->sa_family, addr, string, sizeof(string));
}

static int bind_port(SOCKADDR *remote) {
    struct addrinfo hints, *info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = remote->SAFAMILY;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = getaddrinfo(NULL, "0", &hints, &info);
    if (ret){
        AIRPLAY_DEBUGF("failed to get usable addrinfo?!\n");
        shairport_shutdown();
    }

    sock = socket(remote->SAFAMILY, SOCK_DGRAM, IPPROTO_UDP);
    AIRPLAY_DEBUGF("socket rtp sock: %d\n", sock);
    ret = bind(sock, info->ai_addr, info->ai_addrlen);

    freeaddrinfo(info);

    if (ret < 0){
        AIRPLAY_DEBUGF("could not bind a UDP port!\n");
        shairport_shutdown();
    }

    int sport;
    SOCKADDR local;
    socklen_t local_len = sizeof(local);
    getsockname(sock, (struct sockaddr*)&local, &local_len);
#ifdef AF_INET6
    if (local.SAFAMILY == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&local;
        sport = htons(sa6->sin6_port);
    } else
#endif
    {
        struct sockaddr_in *sa = (struct sockaddr_in*)&local;
        sport = htons(sa->sin_port);
    }

    return sport;
}

int rtp_setup(SOCKADDR *remote, int cport, int tport) {
    if (running){
        AIRPLAY_DEBUGF("rtp_setup called with active stream!\n");
        shairport_shutdown();
    }

    AIRPLAY_DEBUGF("rtp_setup: cport=%d tport=%d\n", cport, tport);

    // we do our own timing and ignore the timing port.
    // an audio perfectionist may wish to learn the protocol.

    memcpy(&rtp_client, remote, sizeof(rtp_client));
#ifdef AF_INET6
    if (rtp_client.SAFAMILY == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6*)&rtp_client;
        sa6->sin6_port = htons(cport);
    } else
#endif
    {
        struct sockaddr_in *sa = (struct sockaddr_in*)&rtp_client;
        sa->sin_port = htons(cport);
    }

    int sport = bind_port(remote);

    rtp_please_shutdown = 0;

    rtp_thread = (HTC)RKTaskCreate2(rtp_receiver, NULL, NULL, (int8_t *)"rtp_thread", RTP_STACK_SIZE  * 2 * 4, TASK_PRIORITY_RTSP, NULL);
    if(rtp_thread == NULL)
    {
        AIRPLAY_DEBUGF("Failed to create RTP receiver task!\n");
        shairport_shutdown();
    }

    running = 1;
    return sport;
}

extern int write_fifo;
void rtp_shutdown(void) {
    if (!running){
        AIRPLAY_DEBUGF("rtp_shutdown called without active stream!\n");
        shairport_shutdown();
    }

    AIRPLAY_DEBUGF("shutting down RTP thread\n");
    rtp_please_shutdown = 1;
    running = 0;
    AIRPLAY_DEBUGF("close Rtp sock: %d\n", sock);
    close(sock);

    while(write_fifo)
    {
        printf("wait write fifo ok....\n");
        rkos_sleep(10);
    }
    RKTaskDelete2(rtp_thread);
}

void rtp_request_resend(seq_t first, seq_t last) {
    if (!running){
        AIRPLAY_DEBUGF("rtp_request_resend called without active stream!\n");
        shairport_shutdown();
    }

    AIRPLAY_DEBUGF("requesting resend on %d packets (%04X:%04X)\n",
         seq_diff(first,last) + 1, first, last);

    char req[8];    // *not* a standard RTCP NACK
    req[0] = 0x80;
    req[1] = 0x55|0x80;  // Apple 'resend'
    *(unsigned short *)(req+2) = htons(1);  // our seqnum
    *(unsigned short *)(req+4) = htons(first);  // missed seqnum
    *(unsigned short *)(req+6) = htons(last-first+1);  // count

    sendto(sock, req, sizeof(req), 0, (struct sockaddr*)&rtp_client, sizeof(rtp_client));
}
#endif

