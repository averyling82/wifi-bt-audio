#include "BspConfig.h"
#ifdef __WIFI_DLNA_C__
#define NOT_INCLUDE_OTHER

#include <rtos_config.h>
#if !defined(FREE_RTOS)
#define LOG_TAG "DlnaMediaPlayer"
#include <utils/Log.h>
#include <VolumeClientProxy.h>
#include <MedialibClientProxy.h>
#include <libgen.h>
#else
#include <utils/dlna_log.h>
#endif

#include <player/player.h>
#include <cybergarage/net/cinterface.h>
#include <server/dms-playlist.h>

#ifndef NULL
#define NULL 0
#endif

//#pragma arm section code = "dlnaCode", rodata = "dlnaCode", rwdata = "dlnadata", zidata = "dlnadata"

#define MAX_ADDRESS_LEN (256)

extern ctrl_t mplayer_ctrl;
extern void dms_string_addvalue (char **str, const char *value);

static int check_bind_address(char* url)
{
    CgNetworkInterfaceList *netIfList;
    CgNetworkInterface *netIf;
    char *bindAddr;
    int ssdpCount, i;
    //char ip_address[MAX_ADDRESS_LEN] = {0};
    char *ip_address = NULL;

    ip_address = malloc(MAX_ADDRESS_LEN);
    if(NULL == ip_address)
    {
        rk_printf("\n check_bind_address malloc fail\n");
        return 0;
    }

    sscanf(url, "http://%[0123456789.]:", ip_address);

    netIfList = cg_net_interfacelist_new();
    cg_net_gethostinterfaces(netIfList);

    for (netIf = cg_net_interfacelist_gets(netIfList); netIf; netIf = cg_net_interface_next(netIf)) {
        bindAddr = cg_net_interface_getaddress(netIf);

        if (cg_strlen(bindAddr) <= 0)
            continue;
        if(strcmp(ip_address, bindAddr) == 0) {
            cg_net_interfacelist_delete(netIfList);
            free(ip_address);
            return 0;
        }
    }
    free(ip_address);
    cg_net_interfacelist_delete(netIfList);
    return 1;

}

char *player_check_url(char *url)
{
    char *path = NULL;
    char *filename = NULL;
    int id = 0;
    int ret = 0;


    ret = check_bind_address(url);
    if (0 == ret) {
        filename = (char *)basename(url);
        id = atoi(filename);
        if (id <= 0 )
            return NULL;

        filename = (char *)playlist_load_string(id, "path");
        if (NULL == filename)
            return NULL;

        path = (char *)cg_strdup("\"");
        dms_string_addvalue(&path, filename);
        dms_string_addvalue(&path, "\"");

        return path;
    }

    return NULL;
}

int player_get_volume(player_t * player)
{
    //return system_volume_get();
      if (player && player->controller->get_volume)
        return(player->controller->get_volume());
    return -1;
}

int player_set_volume(player_t * player, int volume)
{
    //system_volume_set((float)volume);

    if (player && player->controller->set_volume)
        player->controller->set_volume(volume);

    return -1;
}

int player_play(player_t * player)
{
    int ret;

    if (!player || !player->controller->get_state) {
        ALOGE("Cannot find controller.");
        return -1;
    }

    if (player->controller->get_state() == PLAYER_PAUSED) {
        return player->controller->resume();
    } else {
        ret = player->controller->play(player->url);
        //player_clear_url(player);
        return ret;
    }

}

int player_stop(player_t * player)
{
	rk_printf("player_stop...\n");
    if (player && player->controller->stop)
        return player->controller->stop();

    return -1;
}

int player_pause(player_t * player)
{
    if (player && player->controller->pause)
        return player->controller->pause();

    return -1;
}

int player_resume(player_t * player)
{
    if (player && player->controller->resume)
        return player->controller->resume();

    return -1;
}

int player_current_time(player_t* player, char* buf, int len)
{
    if (player && player->controller->current_time)
        return player->controller->current_time(buf, len);

    return -1;
}

int player_get_duration(player_t* player, char* buf, int len)
{
    if (player && player->controller->get_duration)
        return player->controller->get_duration(buf, len);

    return -1;
}

int player_seek(player_t * player, int ms)
{
    if (player && player->controller->seek)
        return player->controller->seek(player->url,ms);

    return -1;
}

int player_set_url(player_t * player, char * url)
{
    char *path = NULL;

    if (!player)
        return -1;

    if (player->url)
        free(player->url);

    path = player_check_url(url);
    if (path != NULL)
    {
    	player->url = path;
		rk_printf("path=%s\n",path);
	}
    else
    {
    	player->url = cg_strdup(url);
		//rk_printf("player->url=%s\n",player->url);
	}

    if (player->controller && player->controller->set_state)
	{
		player->controller->set_state(PLAYER_TRANSITIONING, url);
	}

    return 0;
}

char *player_get_url(player_t * player)
{
    if (!player)
        return NULL;

    return player->url;
}

void player_clear_url(player_t * player)
{
    if (!player)
        return;

    free(player->url);
    player->url = NULL;
}

pstate_t player_get_state(player_t * player)
{
    if (player && player->controller->get_state)
        return player->controller->get_state();

    return PLAYER_STOPPED;
}

int player_state_listener(player_t * player, state_listener_t l)
{
    if (player && player->controller->state_listener)
        return player->controller->state_listener(l);

    return -1;
}

int player_create(player_t ** player)
{
    player_t * p = NULL;

    (*player) = NULL;

    p = (player_t *)malloc(sizeof(player_t));
    if (!p)
        return -1;
    memset(p, 0, sizeof(player_t));

    // Create controller.
    p->controller = &mplayer_ctrl;
    if (p->controller->init()) {
        goto err1;
    }

    (*player) = p;

    ALOGD("Create a new player.");

    return 0;

err1:
    free(p);
    return -1;
}
int player_destroy(player_t * player)
{
    ALOGD("Destroy a player.");

    if (player->url)
        free(player->url);

    player->controller->deinit();

    free(player);

    player = NULL;

    return 0;
}

//#pragma arm section code
#endif

