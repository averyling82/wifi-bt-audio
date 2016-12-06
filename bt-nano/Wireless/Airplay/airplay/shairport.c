/*
 * Shairport, an Apple Airplay receiver
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "BspConfig.h"
#ifdef __WIFI_AIRPLAY_C__

#include "commons.h"
#include "rtsp.h"
#include "mdns.h"
#include "type.h"
#include "external.h"
#include "airplay_heap.h"
#include "airplay_md5.h"
#include "airplay_mplayer_ctrl.h"
#include "airplay.h"

#define READ_BUF_SIZE 512
int delete_airplay_task = 0;

extern void airplay_end();
void shairport_shutdown(void) {
    printf("Shutting down...\n");
    airplay_end();
}

static int get_device_name(char* buf, int len)
{
    //strcpy(config.apname, "RK-NANOD-IOT");
    GetAirplayDeviceName(config.apname);
    return 0;
}

//tiantian? ????????????????????????????
static void event_listener(void *arg)
{
#if 0
    while (1) {
        char rcvBuf[READ_BUF_SIZE] = { 0 };
        lollipop_socket_server_rcv(SOCK_FILE_AIRPLAY, rcvBuf, READ_BUF_SIZE);

        if (NULL == rcvBuf)
            continue;

        if (strstr(rcvBuf, IPC_VOLUME_CHANGE) == rcvBuf) {
            int vol = system_volume_get();

            player_volume_linear((double)vol / 100.0);
            printf("Volume update: %d\r\n", vol);
        } else if (strstr(rcvBuf, IPC_NETWORK_CHANGE) == rcvBuf) {
            char *buf_tok = NULL;

            buf_tok = strtok(rcvBuf, " ");
            if (buf_tok) {
                buf_tok = strtok(NULL, " ");
                printf("buf_tok: %s\r\n", buf_tok);
            }

            if (buf_tok) {
                if (!strncmp(buf_tok, MSG_NETWORK_CONNECTED, strlen(MSG_NETWORK_CONNECTED))) {
                    printf("Airplay MSG_NETWORK_CONNECTED\r\n");
                } else if (!strncmp(buf_tok, MSG_NETWORK_DISCONNECTED, strlen(MSG_NETWORK_DISCONNECTED))) {
                    printf("Airplay MSG_NETWORK_DISCONNECTED\r\n");
                }
                printf("Networ changed!\r\n");
                shairport_shutdown();
            }
        }
    }

    //pthread_exit((void *)0);
    RKTaskDelete2(NULL);
    return NULL;
#endif
}

static void create_event_listener(void)
{
#if 0
    void *t_retval;
    xTaskHandle key_handle;
    xTaskCreate(event_listener, (int8_t *)"event_listener", configMINIMAL_STACK_SIZE * 2 * 16, (void*)NULL, MAIN_TASK_PRIO, &key_handle);
#endif
}

int airplay_init_state()
{
    return config.airplay_init_status;
}
void airplay_main()
{
    AIRPLAY_DEBUGF("-----airplay_main-----\n");

#if 0   //rsa test
    test_apple_challenge();
    //test_announce();
    while(1);
#endif
    Airplay_MplayerCtrl_Init();

    memset(&config, 0, sizeof(config));

    create_event_listener();

    // set defaults
    config.port = 5002;

    char hostname[100];
    gethostname(hostname, 100);

    config.apname = airplay_malloc(100);
    memset(config.apname, 0, 100);
    get_device_name(config.apname, 100);

    // mDNS supports maximum of 63-character names (we append 13).
    if (strlen(config.apname) > 50 || strlen(config.apname) < 4) {
        AIRPLAY_DEBUGF("Cannot get my name, ready to die.\n");
        AIRPLAY_DEBUGF("Supplied name too long (max 50 characters)\n");
        shairport_shutdown();
    }

    AIRPLAY_DEBUGF("My name is %s\n", config.apname);

    uint8_t ap_md5[16]={0};
    MD5_CTX ctx;
    MD5Init(&ctx);
    MD5Update(&ctx, config.apname, strlen(config.apname));
    MD5Final(ap_md5, &ctx);
    memcpy(config.hw_addr, ap_md5, sizeof(config.hw_addr));

    rtsp_listen_loop();

    if(!delete_airplay_task)
    {
        // should not.
        shairport_shutdown();
    }
    delete_airplay_task = 0;
}

void airplay_end()
{
    //printf("\n airplay_end start\n");
    delete_airplay_task = 1;

    rtsp_shutdown_stream();
    cleanup_threads();
    mdns_unregister();

    if(config.mdns_name)
        airplay_free(config.mdns_name);

    if(config.password)
        airplay_free(config.password);

    if(config.apname)
        airplay_free(config.apname);

    close_rtsp_listen_loop();

    Airplay_MplayerCtrl_Deinit();
    //printf("\n airplay_end end\n");
}
#endif

