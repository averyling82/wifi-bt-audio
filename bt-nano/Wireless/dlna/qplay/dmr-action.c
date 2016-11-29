/*
 *
 *  Rockchip device media renderer action process.
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
#include "../../lollipop_wifi/socket_ipc/lollipop_socket_ipc.h"
#else
#include <utils/dlna_log.h>
#endif

#include <renderer/dmr-action.h>
#include <player/player.h>

#ifdef _QPLAY_ENABLE
#include "qplay_list.h"
static G_QPLAY_TRANSPORT_STATE g_qqplayflag = QPLAY_TRANSPORT_STATE_DISABLE;
G_QPLAY_TRANSPORT_STATE get_qplay_seek_state(void)
{
	return g_qqplayflag;
}

void set_qplay_seek_state(G_QPLAY_TRANSPORT_STATE state)
{
	g_qqplayflag = state;		
}

#endif


//#pragma arm section code = "dlnaCode", rodata = "dlnaCode", rwdata = "dlnadata", zidata = "dlnadata"

#define BUF_SIZE (1024)

extern void stop_dlna_device(void);

static void reset_action()
{
#if !defined(FREE_RTOS)
    char msg[256] = {0};

    stop_dlna_device();

    sprintf(msg, "%s %s", IPC_RESTART_OR_RESET, "resetmode");

    lollipop_socket_client_send(SOCK_FILE_MAIN, msg);
#endif
}
extern int MplayerCtrl_Set_stating(int state);
static BOOL dmr_avtransport_actionreceived(CgUpnpAction * action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *actionName;
    CgUpnpArgument *arg;
    char *buf = NULL;
    //char buf[BUF_SIZE] = { 0 };

    actionName = (char *)cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    //ALOGD("Received a avtransport action: %s.", actionName);
    //return TRUE;
    /* GetTransportInfo*/
    if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETTRANSPORTINFO))//GetTransportInfo
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE);
        if (!arg)
            return FALSE;

        /* Set currnet transport state. */
        switch (player_get_state(cg_upnpav_dmr_getuserdata(dmr)))
        {
            case PLAYER_PLAYING:
                cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PLAYING);
                break;

            case PLAYER_STOPPED:
                cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_STOPPED);
                break;

            case PLAYER_PAUSED:
                cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_PAUSED_PLAYBACK);
                break;

            case PLAYER_TRANSITIONING:
                cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATE_TRANSITIONING);
                break;
        }

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTTRANSPORTSTATUS_OK);


        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTSPEED);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, "1");

        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETPOSITIONINFO))//GetPositionInfo
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACK);
        if (!arg)
            return FALSE;
#ifdef _QPLAY_ENABLE
		if(g_qqplayflag)
		{
			char trackstr[4] = {0};
			snprintf(trackstr,4,"%d",((int)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSCUR)+1));
			cg_upnp_argument_setvalue(arg, trackstr);//rk_printf("Track=%s\n",arg->value->value);
		}
		else
			cg_upnp_argument_setvalue(arg, "1");
#else
        cg_upnp_argument_setvalue(arg, "1");
#endif

        // Track duration.
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACK_DURATION);
        if (!arg)
            return FALSE;
        buf = malloc(BUF_SIZE);
        if (NULL == buf)
        {
            rk_printf("CG_UPNPAV_DMR_AVTRANSPORT_TRACK_DURATION malloc fail");
            return FALSE;
        }

        if (!player_get_duration(cg_upnpav_dmr_getuserdata(dmr), buf, BUF_SIZE))
            cg_upnp_argument_setvalue(arg, buf);rk_printf("TrackDuration=%s!",buf);

        // Track meta.
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACK_METADATA);
        if (!arg)
        {
            free(buf);
            buf = NULL;
            return FALSE;
        }
#ifdef _QPLAY_ENABLE
		if(g_qqplayflag)
		{
			char *trackmetadata = NULL;
			trackmetadata = player_get_curQPLAYtrack_metadata();
			if(trackmetadata )
			{
				cg_upnp_argument_setvalue(arg,trackmetadata );
				free(trackmetadata);
				trackmetadata = NULL;
				//rk_printf("TrackMetaData=%s!",arg->value->value);
			}
		}
		else
			cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);
#else
		cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);
#endif
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TRACK_URL);
        if (!arg)
        {
            free(buf);
            buf = NULL;
            return FALSE;
        }
#ifdef _QPLAY_ENABLE
		if(g_qqplayflag)
			cg_upnp_argument_setvalue(arg, player_get_url(cg_upnpav_dmr_getuserdata(dmr)));//rk_printf("TrackURI=%s!",arg->value->value);
#endif
        //cg_upnp_argument_setvalue(arg, "1");

        // Relative time.
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_REL_TIME);
        if (!arg)
        {
            free(buf);
            buf = NULL;
            return FALSE;
        }

        memset(buf, 0, BUF_SIZE);

        if (!player_current_time(cg_upnpav_dmr_getuserdata(dmr), buf, BUF_SIZE))
            cg_upnp_argument_setvalue(arg, buf); rk_printf("RelTime=%s",buf);
        free(buf);
        buf = NULL;
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_ABS_TIME);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_REL_COUNT);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, "65535");

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_ABS_COUNT);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, "65535");

        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SETAVTRANSPORTURI))//SetAVTransportURI
    {
        ALOGI("AV transport action received, action name: %s!", actionName);
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
        if (!arg)
            return FALSE;

        //ALOGI("Set url: %s!", cg_upnp_argument_getvalue(arg));
	#ifdef _QPLAY_ENABLE
		char *av_rui = cg_upnp_argument_getvalue(arg);
		rk_printf("Set url: %s!",av_rui);
		if(av_rui&&strstr(av_rui,QPLAYFLAG))//QPLAY MODE
		{
			if(g_qqplayflag)
				set_qplay_seek_state(QPLAY_TRANSPORT_STATE_SEEKING);//just for not call "player_seek_next_qplay_index" when "player_stop"
			player_stop(cg_upnpav_dmr_getuserdata(dmr));//stop playing
			player_set_qplay_queueID(av_rui+8);
        	g_qqplayflag = QPLAY_TRANSPORT_STATE_NOTSEEKING;
		}
		else
		{
			g_qqplayflag = QPLAY_TRANSPORT_STATE_DISABLE;
			player_reset_qplay_list(TRUE);
			player_set_url(cg_upnpav_dmr_getuserdata(dmr), cg_upnp_argument_getvalue(arg));
		}
	#else
			
			rk_printf("Set url: %s!", cg_upnp_argument_getvalue(arg));
			player_set_url(cg_upnpav_dmr_getuserdata(dmr), cg_upnp_argument_getvalue(arg));
	#endif
        char *last_uri = malloc(BUF_SIZE);
        unsigned int len = 0;
        if (NULL == last_uri)
        {
            rk_printf("CG_UPNPAV_DMR_AVTRANSPORT_SETAVTRANSPORTURI malloc fail");
            return FALSE;
        }

        len =   sprintf(last_uri,"<Event xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\">\n"
                "<InstanceID val=\"0\">\n"
                "<NumberOfTracks val=\"1\"/>\n"
                "<CurrentTransportActions val=\"Play\"/>\n"
                "<PlaybackStorageMedium val=\"NETWORK\"/>\n"
                "<AVTransportURI val=\"%s\"/>\n"
                //"<AVTransportURIMetaData val=\"%s\"/>\n"
                "</InstanceID>\n"
                "</Event>",cg_upnp_argument_getvalue(arg));

        rk_printf("last_uri = 0x%08x, size = %d, len = %d", last_uri, BUF_SIZE, len);

        cg_upnpav_dmr_setavtransportlastchange(dmr, last_uri);
        free(last_uri);
        last_uri = NULL;

        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PLAY))//Play
    {
        player_play(cg_upnpav_dmr_getuserdata(dmr));
        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_PAUSE))//Pause
    {
        player_pause(cg_upnpav_dmr_getuserdata(dmr));
        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_STOP))//Stop
    {
        rk_printf("telephone...........stop...................");
        player_stop(cg_upnpav_dmr_getuserdata(dmr));
        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_SEEK))//Seek
    {
        CgUpnpArgument * unit, *target;

        unit = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_UNIT);
        ALOGI("Seek, Unit: %s\n", cg_upnp_argument_getvalue(unit));
        if (unit && cg_streq(cg_upnp_argument_getvalue(unit), CG_UPNPAV_DMR_AVTRANSPORT_RELTIME))
        {
            target = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);

            ALOGI("Seek, target: %s\n", cg_upnp_argument_getvalue(target));

            if (target)
            {
                unsigned int hour, minute, second;

                sscanf(cg_upnp_argument_getvalue(target), "%d:%d:%d", &hour, &minute, &second);

                ALOGI("Read time: hour - %d, minute - %d, second - %d, msecond - %d\n", hour, minute, second, (((hour * 60) + minute) * 60 + second) * 1000);

                player_seek(cg_upnpav_dmr_getuserdata(dmr), (((hour * 60) + minute) * 60 + second) * 1000);
                return TRUE;
            }
        }
#ifdef _QPLAY_ENABLE
		else if (unit && cg_streq(cg_upnp_argument_getvalue(unit), "TRACK_NR"))
		{
			target = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_TARGET);
			
			rk_printf("Seek, TRACK_NR: %s\n", cg_upnp_argument_getvalue(target));
			
			if (target)
			{
				char *ptrack_nr = cg_upnp_argument_getvalue(target);
				int track_nr = 0;
				if(NULL == ptrack_nr)
				{
					rk_printf("ERROR! NULL == ptrack_nr\n");
					return FALSE;
				}
				track_nr = atoi(ptrack_nr);
				set_qplay_seek_state(QPLAY_TRANSPORT_STATE_SEEKING);//just for not call "player_seek_next_qplay_index" when "player_stop"
				player_stop(cg_upnpav_dmr_getuserdata(dmr));//stop current playing when user seeking
				set_qplay_seek_state(QPLAY_TRANSPORT_STATE_SEEKING);
				//MplayerCtrl_Set_stating(PLAYER_TRANSITIONING);
				return player_seek_qplay_index((track_nr>0)?track_nr:0);
			}
		}
#endif
        return FALSE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_RESET))
    {
        ALOGD("Ready to reset.");
        reset_action();
        return TRUE;
    }
    else if (cg_streq(actionName, CG_UPNPAV_DMR_AVTRANSPORT_GETMEDIAINFO))//GetMediaInfo
    {
        // NrTracks
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_NRTRACKS);
        if (arg)
        {
		#ifdef _QPLAY_ENABLE
			if(g_qqplayflag)
			{
				char nrtracks[4] = {0};
				snprintf(nrtracks,sizeof(nrtracks),"%d", (int)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSNUM));
				cg_upnp_argument_setvalue(arg, nrtracks);
			}
			else
		#endif
        	cg_upnp_argument_setvalue(arg, "1");
		}

        buf = malloc(BUF_SIZE);
        if (NULL == buf)
        {
            rk_printf("dmr_avtransport_action malloc fail");
            return FALSE;
        }
        // MediaDuration
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_MEDIA_DURATION);
        if (arg)
        {
		#ifdef _QPLAY_ENABLE
			if(g_qqplayflag)
			{
				char tracksDuration[16] = {0};
				int hour = 0,minute = 0, sec = 0;

				hour = (int)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSDURATION)/3600;
				minute = ((int)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSDURATION)%3600)/60;
				sec = ((int)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSDURATION)%3600)%60;
				sprintf(tracksDuration,"%02d:%02d:%02d",hour,minute,sec); 
				cg_upnp_argument_setvalue(arg, tracksDuration);
				rk_printf("MediaDuration = %s\n", arg->value->value);
			}
			else
		#endif
            if (!player_get_duration(cg_upnpav_dmr_getuserdata(dmr), buf, BUF_SIZE))
            {
                rk_printf("play get duration11 ");
                cg_upnp_argument_setvalue(arg, buf);
            }
        }
        free(buf);
        buf = NULL;
        // CurrentURI
        //arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_AVTRANSPORT_CURRENTURI);
        if (arg)
        {
		#ifdef _QPLAY_ENABLE
			if(g_qqplayflag)//use the QPLAY list queueID
			{	
				cg_upnp_argument_setvalue(arg, (char *)player_get_qplaylist_value(QPLAY_LIST_VALUE_QUEUEID));
				//rk_printf("CurrentURI = %s\n", arg->value->value);
			}
			else
		#endif
            	cg_upnp_argument_setvalue(arg, player_get_url(cg_upnpav_dmr_getuserdata(dmr)));
        }
	#ifdef _QPLAY_ENABLE
        // CurrentURIMetaData
        if(g_qqplayflag)
        {
        	arg = NULL;
        	arg = cg_upnp_action_getargumentbyname(action,"CurrentURIMetaData");
        	if (!arg)
            	return FALSE;
        	cg_upnp_argument_setvalue(arg, (char *)player_get_qplaylist_value(QPLAY_LIST_VALUE_TRACKSMETADATA));
			//rk_printf("CurrentURIMetaData = %s\n", arg->value->value);
    	}
	#endif

#if 0

        // NextURI
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action,"NextURI");
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);

        // NextURIMetaData
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action,"NextURIMetaData");
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);

        // PlayMedium
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action,"PlayMedium");
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);

        // RecordMedium
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action,"RecordMedium");
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);

        // WriteStatus
        arg = NULL;
        arg = cg_upnp_action_getargumentbyname(action,"WriteStatus");
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, CG_UPNPAV_DMR_AVTRANSPORT_NOT_IMPLEMENTED);
#endif

        return TRUE;
    }
    else
    {
        ALOGE("AV transport unknown action received, action name: %s!", actionName);
    }

    return FALSE;
}

static BOOL dmr_renderingctrl_actionreceived(CgUpnpAction * action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *actionName;
    CgUpnpArgument *arg;

    actionName = (char *)cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    /* GetMute*/
    if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_GETMUTE))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTMUTE);
        if (!arg)
            return FALSE;
        cg_upnp_argument_setvalue(arg, "0");
        return TRUE;
    }

    /* GetVolume */
    if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_GETVOLUME))
    {
        char vol[128] = { 0 };

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_CURRENTVOLUME);
        if (!arg)
            return FALSE;

        sprintf(vol, "%d", player_get_volume(cg_upnpav_dmr_getuserdata(dmr)));
        ALOGD("Get volume: %s\n", vol);
        cg_upnp_argument_setvalue(arg, vol);
        return TRUE;
    }

    /* SetVolume. */
    if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_SETVOLUME))
    {
        int volume;

        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_RENDERINGCONTROL_DESIREDVOLUME);
        if (!arg)
            return FALSE;

        sscanf(cg_upnp_argument_getvalue(arg), "%d", &volume);

        ALOGD("Set volume: %d\n", volume);
        player_set_volume(cg_upnpav_dmr_getuserdata(dmr), volume);
        return TRUE;
    }

    /* SetMute. */
    if (cg_streq(actionName, CG_UPNPAV_DMR_RENDERINGCONTROL_SETMUTE))
    {
        player_set_volume(cg_upnpav_dmr_getuserdata(dmr), 0);
        return TRUE;
    }

    return FALSE;
}

static BOOL dmr_conmgr_actionreceived(CgUpnpAction * action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *actionName;
    CgUpnpArgument *arg;
    CgString *protocolInfos;
    CgUpnpAvProtocolInfo *protocolInfo;

    actionName = (char *)cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    /* GetProtocolInfo*/
    if (cg_streq(actionName, CG_UPNPAV_DMR_CONNECTIONMANAGER_GET_PROTOCOL_INFO))
    {
        arg = cg_upnp_action_getargumentbyname(action, CG_UPNPAV_DMR_CONNECTIONMANAGER_SINK);
        if (!arg)
            return FALSE;
        protocolInfos = cg_string_new();
        for (protocolInfo = cg_upnpav_dmr_getprotocolinfos(dmr); protocolInfo; protocolInfo = cg_upnpav_protocolinfo_next(protocolInfo))
        {
            if (0 < cg_string_length(protocolInfos))
                cg_string_addvalue(protocolInfos, ",");
            cg_string_addvalue(protocolInfos, cg_upnpav_protocolinfo_getstring(protocolInfo));
        }
        //ALOGD("GetProtocolInfo: %s", cg_string_getvalue(protocolInfos));
        cg_upnp_argument_setvalue(arg, cg_string_getvalue(protocolInfos));
        cg_string_delete(protocolInfos);
        return TRUE;
    }

    return FALSE;
}

BOOL dmr_actionreceived(CgUpnpAction * action)
{
    BOOL res = FALSE;
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAvRenderer *dmr;

    //ALOGE("Received an action, %s", cg_upnp_action_getname(action));

    service = cg_upnp_action_getservice(action);
    if (!service)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(service);
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;

    rk_printf("AV Action start, %s = %d", cg_upnp_action_getname(action),rkos_GetFreeHeapSize());
    //rk_printf("service, %s\r\n", cg_upnp_service_getservicetype(service));
    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_AVTRANSPORT_SERVICE_TYPE))
    {
        //ALOGE("AV Action start, %s", cg_upnp_action_getname(action));
        res = dmr_avtransport_actionreceived(action);
        //ALOGE("AV Action end.");
        return res;
    }

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_RENDERINGCONTROL_SERVICE_TYPE))
    {
        res = dmr_renderingctrl_actionreceived(action);
        return res;
    }

    if (cg_streq(cg_upnp_service_getservicetype(service), CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_TYPE))
    {
        res = dmr_conmgr_actionreceived(action);
        return res;
    }
#ifdef _QPLAY_ENABLE
    if (cg_streq(cg_upnp_service_getservicetype(service), RK_QPLAY_SERVICE_TYPE))
    {
        res = qplay_actionreceived(action);
        return res;
    }
#endif
    return FALSE;
}
//#pragma arm section code
#endif

