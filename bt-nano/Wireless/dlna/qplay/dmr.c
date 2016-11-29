/*
 *
 *  Rockchip device media renderer.
 *
 *    2014-01-16 by rockchip IOT team.
 *
 *
 */
#include "BspConfig.h"
#ifdef __WIFI_DLNA_C__
#define NOT_INCLUDE_OTHER

#include <rtos_config.h>
#if !defined(FREE_RTOS)
#define LOG_TAG "MediaRenderer"
#include <utils/Log.h>
#else
#include <utils/dlna_log.h>
#endif

#include <player/player.h>
#include <renderer/dmr.h>
#include <renderer/dmr-action.h>
#include <utils/udn.h>
#include <utils/descriptions_load.h>
#include <renderer/dmr-http.h>
#include "dlna.h"
#include "RKOS.h"
//#pragma arm section code = "dlnaCode", rodata = "dlnaCode", rwdata = "dlnadata", zidata = "dlnadata"


#define DMR_DESCRIPTION_FILE "/system/etc/dlna/renderer/root.xml"
#define BUF_SIZE (512*2)

static CgUpnpAvRenderer* g_dmr;

pSemaphore sub_sem = NULL;

char *last_change_action = "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\">"
                           "<InstanceID val=\"0\"><TransportState val=\"%s\"/><CurrentTransportActions val=\"%s\"/></InstanceID></Event>\n\0";

char *last_change_state = "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\">"
                          "<InstanceID val=\"0\"><TransportState val=\"%s\"/></InstanceID></Event>\n\0";

char *last_change_duration = "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\">"
                             "<InstanceID val=\"0\"><TransportState val=\"%s\"/><CurrentTrackDuration val=\"%s\"/>"
                             "<CurrentMediaDuration val=\"%s\"/></InstanceID></Event>\n\0";

void dmr_nomedia_lastchange(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    //char buf[BUF_SIZE] = {0};
    char *buf;
    buf = malloc(BUF_SIZE);

    if(buf == NULL)
    {
        return;

    }
    memset(buf,0, BUF_SIZE);
    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);


    sprintf(buf, last_change_duration, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED, "00:00:00", "00:00:00");
    cg_upnp_statevariable_setvalue(stateVar, buf);

    //char play_duration[BUF_SIZE] = {0};
    char *play_duration;
    play_duration = malloc(BUF_SIZE);
    if(play_duration == NULL)
    {
        free(buf);
        return;

    }
    memset(play_duration,0, BUF_SIZE);
    player_get_duration(cg_upnpav_dmr_getuserdata(dmr), play_duration, BUF_SIZE);
    ALOGD("--test--play_duration = %s", play_duration);
    sprintf(buf, last_change_duration, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_NOMEDIAPRESENT, play_duration, play_duration);
    cg_upnp_statevariable_setvalue(stateVar, buf);

    sprintf(buf, last_change_action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED, "Play, Seek, X_DLNA_SeekTime");
    cg_upnp_statevariable_setvalue(stateVar, buf);
    ALOGD("buf= %s\n", buf);

    free(play_duration);
    free(buf);
    return;
}
void dmr_stop_lastchange(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    //char buf[BUF_SIZE] = {0};
    char *buf;
    buf = malloc(BUF_SIZE);

    if(buf == NULL)
    {
        return;

    }
    memset(buf,0, BUF_SIZE);
    if (player_get_url(cg_upnpav_dmr_getuserdata(dmr)))  //no first
    {
        //char play_duration[BUF_SIZE] = {0};
        char *play_duration;
        play_duration = malloc(BUF_SIZE);
        if(play_duration == NULL)
        {
            free(buf);
            return;

        }
        memset(play_duration,0, BUF_SIZE);
        player_get_duration(cg_upnpav_dmr_getuserdata(dmr), play_duration, BUF_SIZE);
        sprintf(buf, last_change_duration, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED, play_duration, play_duration);
        free(play_duration);
    }
    else
    {
        sprintf(buf, last_change_state, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED);
    }

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);

    cg_upnp_statevariable_setvalue(stateVar, buf);
    ALOGD("buf= %s\n", buf);
    free(buf);
    return;
}

void dmr_transport_lastchange(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    //char buf[BUF_SIZE] = {0};
    char *buf;
    buf = malloc(BUF_SIZE);

    if(buf == NULL)
    {
        return;

    }
    memset(buf,0, BUF_SIZE);
    //if (player_get_state(cg_upnpav_dmr_getuserdata(dmr)) == PLAYER_TRANSITIONING){ //no first
    sprintf(buf,  last_change_action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_TRANSITIONING, "Stop");
//  }
#if 0
    else
    {
        char *play_url = player_get_url(cg_upnpav_dmr_getuserdata(dmr));
        char play_duration[BUF_SIZE] = {0};
        player_get_duration(cg_upnpav_dmr_getuserdata(dmr), play_duration, BUF_SIZE);
        if (!strcmp(play_duration, "00:00:00"))
        {
            ALOGD("--test-- goto play_duration");
            strcpy(play_duration, "00:04:01\0");
        }
        ALOGD("--test--playduarion = %s", play_duration);
        sprintf(buf,  "<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\"><InstanceID val=\"0\">"
                "<AVTransportURI val=\"%s\"/><TransportState val=\"%s\"/><CurrentTrackDuration val=\"%s\"/>"
                "<CurrentMediaDuration val=\"%s\"/></InstanceID></Event>\n",
                play_url,CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_TRANSITIONING, play_duration, play_duration);
    }
#endif

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);

    cg_upnp_statevariable_setvalue(stateVar, buf);
    ALOGD("buf= %s\n", buf);
    free(buf);
    return;
}

void dmr_playing_lastchange(CgUpnpAvRenderer *dmr)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    //char buf[BUF_SIZE] = {0};
    char *buf;
    buf = malloc(BUF_SIZE);

    if(buf == NULL)
    {
        return;

    }
    memset(buf,0, BUF_SIZE);
    sprintf(buf, last_change_action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PLAYING, "Pause, Stop, Seek, X_DLNA_SeekTime");

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);

    cg_upnp_statevariable_setvalue(stateVar, buf);
    ALOGD("buf= %s\n", buf);
    free(buf);
}

void dmr_pause_lastchange(CgUpnpAvRenderer *dmr, int value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    //char buf[BUF_SIZE] = {0};
    char *buf;
    buf = malloc(BUF_SIZE);

    if(buf == NULL)
    {
        return;

    }
    memset(buf,0, BUF_SIZE);
    if (value == 0)
        sprintf(buf, last_change_action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PAUSED_PLAYBACK, "Play, Stop");
    else
        sprintf(buf,  last_change_state, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PAUSED_PLAYBACK);

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_LASTCHANGE);

    cg_upnp_statevariable_setvalue(stateVar, buf);

    ALOGD("buf= %s\n", buf);

    free(buf);
}

void dmr_set_avt_lastchange(CgUpnpAvRenderer *dmr, char *value)
{

    if (strstr(value, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_NO_MEDIA))
        dmr_nomedia_lastchange(dmr);
    else if (strstr(value, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PAUSED_PLAYBACK))
    {
        dmr_pause_lastchange(dmr, 0);
        dmr_pause_lastchange(dmr, 1);
    }
    else if (strstr(value, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PLAYING))
    {
        //dmr_transport_lastchange(dmr);
        dmr_playing_lastchange(dmr);
    }
    else if (strstr(value, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED))
        dmr_stop_lastchange(dmr);
    else if (strstr(value, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_TRANSITIONING))
        dmr_transport_lastchange(dmr);

    return;
}

void dmr_seek_success_notify()
{
    dmr_playing_lastchange(g_dmr);
//  dmr_transport_lastchange(g_dmr);
}
void dmr_setplaystatechange(CgUpnpAvRenderer *dmr, char *value)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;

    ALOGD("Play state changed to: %s\n", value);

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_AVTRANSPORT_TRANSPORT_STATE);
    cg_upnp_statevariable_setvalue(stateVar, value);
    dmr_set_avt_lastchange(dmr, value);
}

void dmr_play_state_changed(pstate_t state)
{
    if (!g_dmr)
        return;

    rkos_semaphore_take(sub_sem , MAX_DELAY);

    switch (state)
    {
        case PLAYER_PLAYING:
            dmr_setplaystatechange(g_dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PLAYING);
            break;

        case PLAYER_STOPPED:
            //player_clear_url((player_t *)cg_upnpav_dmr_getuserdata(g_dmr));
            dmr_setplaystatechange(g_dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED);
            break;

        case PLAYER_PAUSED:
            dmr_setplaystatechange(g_dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PAUSED_PLAYBACK);
            break;

        case PLAYER_TRANSITIONING:
            dmr_setplaystatechange(g_dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_TRANSITIONING);
            break;
        // need add next
        case PLAYER_NO_MEDIA://use for rk player
            rkos_delay(1000);
            dmr_setplaystatechange(g_dmr, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_NO_MEDIA);
            ALOGD("PLAYER_NO_MEDIA\n");
            break;
    }
    rkos_semaphore_give(sub_sem);
}

void dmr_setvolumechange(CgUpnpAvRenderer *dmr, int volume)
{
    CgUpnpService *service;
    CgUpnpStateVariable *stateVar;
    char vol[128] = { 0 };

    service = cg_upnp_device_getservicebyexacttype(dmr->dev, CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE);
    stateVar = cg_upnp_service_getstatevariablebyname(service, CG_UPNPAV_DMR_RENDERINGCONTROL_VOLUME);
    sprintf(vol, "%d", volume);
    cg_upnp_statevariable_setvalue(stateVar, vol);
}

int dmr_set_volume(CgUpnpAvRenderer *dmr, int volume)
{
    player_t * player = NULL;

    if (!dmr)
        return -1;

    player = (player_t *)cg_upnpav_dmr_getuserdata(dmr);
    if (!player)
        return -1;

    player_set_volume(player, volume);
    dmr_setvolumechange(dmr, volume);

    return 0;
}

void dmr_restart(CgUpnpAvRenderer * dmr)
{
    cg_upnpav_dmr_lock(dmr);

    cg_upnpav_dmr_start(dmr);

    cg_upnpav_dmr_unlock(dmr);
}

extern int Mplayer_Init(void);

CgUpnpAvRenderer *dmr_create(char* suffix)
{
    CgUpnpAvRenderer *dmr = NULL;
    char udn[UDN_LENGTH] = { 0 };
    char name[128] = { 0 };
    player_t * player = NULL;
    CgUpnpAvProtocolInfo *protocolInfo;

    dmr = cg_upnpav_dmr_new();
    if (!dmr) {
        ALOGE("Can not create a new media renderer\r\n");
        return NULL;
    }

    if(local_descriptions_load(dmr->dev) == FALSE)
        ALOGE("Can not load local descriptions\n");

    // Set device udn.
    if (!create_udn(udn, UDN_LENGTH))
        cg_upnp_device_setudn(dmr->dev, udn);

    // Set device friendly name.
    //sprintf(name, "Rockchip Media Renderer [%s]", suffix);
    GetDlnaDeviceName(name);
    cg_upnp_device_setfriendlyname(dmr->dev, name);
    // Set listeners.
    //cg_upnpav_dmr_sethttplistener(dmr, dmr_http_request);
    cg_upnpav_dmr_setactionlistener(dmr, dmr_actionreceived);

    // Create player.
    Mplayer_Init();
    player_create(&player);//rk_printf("--test--goin dmr_create4\r\n");
    cg_upnpav_dmr_setuserdata(dmr, player);//rk_printf("--test--goin dmr_create5\r\n");

    // Set state listener.
    player_state_listener(player, dmr_play_state_changed);//rk_printf("--test--goin dmr_create6\r\n");

    // Set protocol info.
    /*
    protocolInfo = cg_upnpav_protocolinfo_new();
    if (protocolInfo) {
        cg_upnpav_protocolinfo_setprotocol(protocolInfo, "http-get");
        cg_upnpav_protocolinfo_setmimetype(protocolInfo, "audio/mpeg;audio/ape");
        cg_upnpav_dmr_addprotocolinfo(dmr, protocolInfo);
    }*/
    //rk_printf("--test--goin dmr_create8\r\n");


    sub_sem = rkos_semaphore_create(1 , 1);
    if(sub_sem == NULL)
    {
        rk_printf("sub_sem malloc fail");
    }
    cg_upnpav_dmr_start(dmr);

    //rk_printf("--test--goin dmr_create9\r\n");
    ALOGD("Media renderer start, %s.remain = %d\r\n", cg_upnp_device_getudn(dmr->dev), rkos_GetFreeHeapSize());

    g_dmr = dmr;
    return dmr;
}
void dmr_destroy(CgUpnpAvRenderer * dmr)
{
    if (!dmr)
        return;


    cg_upnpav_dmr_stop(dmr);
    cg_upnpav_dmr_delete(dmr);

     //Destroy player.
    player_destroy(cg_upnpav_dmr_getuserdata(dmr));
    rkos_semaphore_delete(sub_sem);
    sub_sem = NULL;
    g_dmr = 0;
    ALOGD("Media renderer stop.\n");

    return;
}


//#pragma arm section code

#endif
