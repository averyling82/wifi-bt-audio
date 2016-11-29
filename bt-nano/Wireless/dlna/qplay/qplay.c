#include "BspConfig.h"

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
#include <cybergarage/upnp/std/av/cmediarenderer.h>

#ifdef _QPLAY_ENABLE
#include "qplay_list.h"
#include "cJSON.h"
#include "md5_32.h"



#define QPLAYNULLLIST ""//"{\"TracksMetaData\":[]}"//
#define QPLAYMAXTRACKS "100"
#define MANUFACTURID "62900058"
//#define PRESHAREDKEY "xxxxxx"
#define DEVICETYPEID "nw11"

#define REMALLOCALLTRACK 1
#define ALLTARCKLENGTH 65536

//QPLAY API
static BOOL qplay_setnetwork_func(CgUpnpAction * action);
static BOOL qplay_auth_func(CgUpnpAction * action);
static BOOL qplay_insert_tracks(CgUpnpAction * action);
static BOOL qplay_remove_tracks(CgUpnpAction * action);
static BOOL qplay_remove_all_tracks(CgUpnpAction * action);
static BOOL qplay_get_tracks_info(CgUpnpAction * action);
static BOOL qplay_set_tracks_info(CgUpnpAction * action);
static BOOL qplay_get_tracks_count(CgUpnpAction * action);
static BOOL qplay_get_max_tracks(CgUpnpAction * action);
static BOOL qplay_get_lyric_supportType(CgUpnpAction * action);


//QPLAYLIST API
static BOOL player_set_qplay_list(int startindex);
static char *player_get_url_from_qplaylist(int index);


struct QPLAY_ACTION {
    const char *action_name;
    int (*callback) (CgUpnpAction *);
};

typedef struct __QPLAY_PLAYLIST {
    char *tracksMetadata;//string of the list
    char *queueID;
	int tracksNum;//song number of the list,start 1
	int tracksCur;//start 0
	int tracksDuration;//MediaDuration of the list
	cJSON* pTracksArray;//the playing node
	cJSON* pTracksRoot;//root node
}QPLAY_PLAYLIST;
QPLAY_PLAYLIST g_qplay_list;
player_t * g_player_p = NULL;



typedef enum {
	QPLAY_CMD_SETNETWORK = 0,//SetNetwork
	QPLAY_CMD_QPLAYAUTH,//QPlayAuth
	QPLAY_CMD_INSERTTRACKS,
	QPLAY_CMD_REMOVETRACKS,
	QPLAY_CMD_REMOVEALLTRACKS,
	QPLAY_CMD_GETTRACKSINFO,
	QPLAY_CMD_SETTRACKSINFO,
	QPLAY_CMD_GETTRACKSCOUNT,
	QPLAY_CMD_GETMAXTRACKS,
	QPLAY_CMD_GETLYRICSUPPORTTYPE,//GetLyricSupportType
	QPLAY_CMD_UNKNOWN,                   
	QPLAY_CMD_COUNT
} qplay_cmd ;

static BOOL qplay_setnetwork_func(CgUpnpAction * action)
{
	char *ssid = NULL;
	char *key = NULL;
	char *authalgo = NULL;
	char *cipheralgo = NULL;
	CgUpnpArgument *arg = NULL;


	 //get SSID
	rk_printf("ssid");
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETNERWORK_SSID);
	if (!arg)
		return FALSE;
	ssid = cg_upnp_argument_getvalue(arg);
	if(NULL == ssid)
		return FALSE;
	rk_printf(" = %s\n", ssid);
	
	//get Key
	rk_printf("key");
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETNERWORK_KEY);
	if (!arg)
		return FALSE;
	key = cg_upnp_argument_getvalue(arg);
	if(NULL == key)
		return FALSE;
	rk_printf(" = %s\n", key);

	//get AuthAlgo
	rk_printf("authalgo");
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETNERWORK_AUTHALGO);
	if (arg)
	{
		authalgo = cg_upnp_argument_getvalue(arg);
		rk_printf(" = %s\n", authalgo);
	}
	else
		rk_printf("AuthAlgo is NULL!\n");

	//get CipherAlgo
	rk_printf("cipheralgo");
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETNERWORK_CIPHERALGO);
	if (arg)
	{
		cipheralgo = cg_upnp_argument_getvalue(arg);
		rk_printf(" = %s\n", cipheralgo);
	}
	else
		rk_printf("CipherAlgo is NULL!\n");

	//set network
	
	return TRUE;

}
static BOOL qplay_auth_func(CgUpnpAction * action)
{
	char *seed = NULL;
	char *manufactureid = MANUFACTURID;
	char str_seed_prekey[128] = {0};
	unsigned char presharedkey[17] = {0xe6, 0xb1, 0x9f, 0xe6, 0xb3, 0xa2, 0xe9, 0xbe, 0x99, 0x3a, 0x20, 0x61, 0x69, 0x72, 0x6d, 0x75,0x0};
	char *devicetypeid = DEVICETYPEID;
	char *md5code = NULL;
	//unsigned char md5code[16] = {0};
	CgUpnpArgument *arg;

	//get seed
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_QPLAYAUTH_SEED);
	if (!arg)
		return FALSE;
	seed = cg_upnp_argument_getvalue(arg);
	if(NULL == seed)
		return FALSE;
	rk_printf("seed = %s\n", seed);

	snprintf(str_seed_prekey,sizeof(str_seed_prekey),"%s%s",seed,presharedkey);
	//rk_printf("aa presharedkey=%s  aa\n\n",presharedkey);
	md5code = MD5Create(str_seed_prekey);//32bit
	if(NULL == md5code)
	{
        rk_printf("ERROR MD5Create failed\n");
		return FALSE;
	}

	//set code
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_QPLAYAUTH_CODE);
	if (!arg)
		return FALSE;
	cg_upnp_argument_setvalue(arg, md5code);
	
	//set mid
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_QPLAYAUTH_MID);
	if (!arg)
		return FALSE;
	cg_upnp_argument_setvalue(arg, manufactureid);
		
	//set did
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_QPLAYAUTH_DID);
	if (!arg)
		return FALSE;
	cg_upnp_argument_setvalue(arg, devicetypeid);

	return TRUE;
}

static BOOL qplay_insert_tracks(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	return TRUE;
}

static BOOL qplay_remove_tracks(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	return TRUE;
}

static BOOL qplay_remove_all_tracks(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	return TRUE;
}

static BOOL qplay_get_tracks_info(CgUpnpAction * action)
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);

	int rc = -1; 
	char *pstartindex = NULL;
	char *pnumberoftracks = NULL;
	char *tracksmetadata = NULL;
	int startindex = 0;
	int numberoftracks = 0; 
	CgUpnpArgument *arg;
	
	//get StartingIndex
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_GETTRACKSINFO_STINDEX);
	if (!arg)
		return FALSE;
	pstartindex = cg_upnp_argument_getvalue(arg);rk_printf("pstartindex=%s\n",pstartindex);
	if(NULL == pstartindex)
	{
		rk_printf("ERROR!NULL == pstartindex\n");
		return FALSE;
	}

	//get NumberOfTracks
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_GETTRACKSINFO_NUMTRK);
	if (!arg)
		return FALSE;
	pnumberoftracks = cg_upnp_argument_getvalue(arg);rk_printf("pnumberoftracks=%s\n",pstartindex);
	if(NULL == pnumberoftracks)
	{
		rk_printf("ERROR!NULL == pnumberoftracks\n");
		return FALSE;
	}
	startindex = atoi(pstartindex);
	numberoftracks = atoi(pnumberoftracks);

	//set TracksMetaData
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_GETTRACKSINFO_TRKDATA);
	if (!arg)
		return FALSE;
	if(g_qplay_list.tracksMetadata)
		cg_upnp_argument_setvalue(arg, g_qplay_list.tracksMetadata);
	else
		cg_upnp_argument_setvalue(arg, "NULL");
	rk_printf("TracksMetaData = %s", arg->value->value);

	return TRUE;
}

static BOOL qplay_set_tracks_info(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);

	char *tracksmetadata = NULL;
	char *pstartingindex = NULL;
	char *pnextindex = NULL;
	char *queueid = NULL;
	int startingindex = 0;
	int nextindex = 0;
	int rc = 0;
	char successstr[4] = {0};
	CgUpnpArgument *arg;

	//get queueid
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_QUEUEID);
	if (!arg)
		return FALSE;
	queueid = cg_upnp_argument_getvalue(arg);rk_printf("queueid=%s\n",queueid);
	if(NULL == queueid)
	{
		rk_printf("ERROR!NULL == queueid\n");
		return FALSE;
	}
	player_set_qplay_queueID(queueid);
	set_qplay_seek_state(QPLAY_TRANSPORT_STATE_NOTSEEKING);

	//get StartingIndex
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_STINDEX);
	if (!arg)
		return FALSE;
	pstartingindex = cg_upnp_argument_getvalue(arg);rk_printf("pstartingindex=%s\n",pstartingindex);
	if(NULL == pstartingindex)
	{
		rk_printf("ERROR!NULL == pstartingindex\n");
	}
	else
		startingindex = atoi(pstartingindex);

	//get pnextindex----useless
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_NTINDEX);
	pnextindex = cg_upnp_argument_getvalue(arg);rk_printf("pnextindex=%s\n",pnextindex);
	
	//get TracksMetaData
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_TRKDATA);
	if (!arg)
		return FALSE;
	tracksmetadata = cg_upnp_argument_getvalue(arg);//rk_printf("tracksmetadata=%s\n",tracksmetadata);
	if(tracksmetadata)
	{
		player_reset_qplay_list(FALSE);
		g_qplay_list.tracksMetadata = tracksmetadata;
		player_set_qplay_list(startingindex);
	}
	
	//set NumberOfSuccess(the song num of TracksMetaData)
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_NSUCCESS);
	if (!arg)
		return FALSE;
	sprintf(successstr, "%d", g_qplay_list.tracksNum);
	cg_upnp_argument_setvalue(arg, successstr);
	rk_printf("NumberOfSuccess = %s", arg->value->value);

	return FALSE;

}

static BOOL qplay_get_tracks_count(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	return TRUE;
}

static BOOL qplay_get_max_tracks(CgUpnpAction * action)//just defined but unused
{    
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	CgUpnpArgument *arg;
	//Set MaxTracks
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_GETMAXTRACKS_MAXTRACKS);
	if (!arg)
		return FALSE;
	cg_upnp_argument_setvalue(arg, RK_QPLAY_SERVICE_MAXTRACKS);
		rk_printf("MaxTracks = %s", arg->value->value);
	return TRUE;
}

static BOOL qplay_get_lyric_supportType(CgUpnpAction * action)//just defined but unused
{
	rk_printf(">>>>>>>>>>>>>>>>%s:ENTER\n",__FUNCTION__);
	return FALSE;
}

static struct QPLAY_ACTION qplay_actions[] = {
	[QPLAY_CMD_SETNETWORK] = {RK_QPLAY_SERVICE_SETNERWORK, qplay_setnetwork_func},//SetNetwork
	[QPLAY_CMD_QPLAYAUTH] = {RK_QPLAY_SERVICE_QPLAYAUTH, qplay_auth_func},//QPlayAuth
	[QPLAY_CMD_INSERTTRACKS] = {RK_QPLAY_SERVICE_INSERTRACKS, qplay_insert_tracks},
	[QPLAY_CMD_REMOVETRACKS] =     {RK_QPLAY_SERVICE_REMOVETRACKS, qplay_remove_tracks},
	[QPLAY_CMD_REMOVEALLTRACKS] =  {RK_QPLAY_SERVICE_REMOVEALLTRACKS, qplay_remove_all_tracks},
	[QPLAY_CMD_GETTRACKSINFO] =              {RK_QPLAY_SERVICE_GETTRACKSINFO, qplay_get_tracks_info},
	[QPLAY_CMD_SETTRACKSINFO] =         {RK_QPLAY_SERVICE_SETTRACKSINFO, qplay_set_tracks_info},
	[QPLAY_CMD_GETTRACKSCOUNT] =     {RK_QPLAY_SERVICE_GETTRACKCOUNT, qplay_get_tracks_count},
	[QPLAY_CMD_GETMAXTRACKS] =          {RK_QPLAY_SERVICE_GETMAXTRACKS, qplay_get_max_tracks},
	[QPLAY_CMD_GETLYRICSUPPORTTYPE] = {RK_QPLAY_SERVICE_GETLYRICSUPPORTTYPE,qplay_get_lyric_supportType},
	[QPLAY_CMD_UNKNOWN] =                  {NULL, NULL}
};

BOOL qplay_actionreceived(CgUpnpAction * action)
{
    CgUpnpAvRenderer *dmr;
    CgUpnpDevice *dev;
    char *actionName;
    char *buf = NULL;
	int i = 0;

    rk_printf("qplay_action received");

    actionName = (char *)cg_upnp_action_getname(action);
    if (cg_strlen(actionName) <= 0)
        return FALSE;

    dev = (CgUpnpDevice *)cg_upnp_service_getdevice(cg_upnp_action_getservice(action));
    if (!dev)
        return FALSE;

    dmr = (CgUpnpAvRenderer *)cg_upnp_device_getuserdata(dev);
    if (!dmr)
        return FALSE;
	if(NULL == g_player_p)
		g_player_p = (player_t *)cg_upnpav_dmr_getuserdata(dmr);
    rk_printf("qplay_action received %s", actionName);

	for(i = 0; i < QPLAY_CMD_UNKNOWN;i++)
	{
		if (cg_streq(actionName, qplay_actions[i].action_name))
		{
			return (qplay_actions[i].callback)(action);
		}
	}

	rk_printf("ERROR!UNKNOW QPLAY ACTION...\n");
    return FALSE;
}


/***********************************************************************************/
/********************************QPLAY PLAYLIST LOCAL API****************************/
/***********************************************************************************/
static BOOL player_set_qplay_list(int startindex)//the startindex is useless
{
	cJSON* item= NULL;
	cJSON* itemchild=NULL;
	int hour = 0;
	int minute = 0;
	int second = 0;
	int i = 0;

	g_qplay_list.pTracksRoot = cJSON_Parse(g_qplay_list.tracksMetadata);
	if(NULL == g_qplay_list.pTracksRoot)
	{
		rk_printf("ERROR list0\n");
		return FALSE;
	}
	g_qplay_list.pTracksArray = cJSON_GetObjectItem (g_qplay_list.pTracksRoot, "TracksMetaData" );
	if(NULL == g_qplay_list.pTracksArray)
	{
		rk_printf("ERROR list1\n");
		return FALSE;
	}
	g_qplay_list.tracksNum = cJSON_GetArraySize(g_qplay_list.pTracksArray);
	if(0 == g_qplay_list.tracksNum)
	{
		rk_printf("ERROR list2\n");
		return FALSE;
	}

	for(i = 0;i < g_qplay_list.tracksNum;i++)//get total duration
	{
		item = cJSON_GetArrayItem(g_qplay_list.pTracksArray, i);
		if(NULL == item)
		{
			rk_printf("ERROR list3\n");
			return FALSE;
		}

		itemchild = item->child;
		while(itemchild)
		{
			if(strcmp(itemchild->string, "duration") == 0)
			{
			    sscanf(itemchild->valuestring, "%d:%02d:%02d", &hour, &minute, &second);
			    g_qplay_list.tracksDuration += (hour * 3600 + minute * 60 + second);
				break;
			}
			itemchild = itemchild->next;
		}

	}
	return TRUE;	
}

static char *player_get_url_from_qplaylist(int index)//(int handle, char *str)
{
	cJSON* item= NULL;
	cJSON* itemchild=NULL;
	
	if(NULL == g_qplay_list.pTracksArray)
	{
		rk_printf("ERROR! NULL list0\n");
		return NULL;
	}

	item = cJSON_GetArrayItem(g_qplay_list.pTracksArray, index);//printf("yyyyyyyy\n");
	if(NULL == item)
	{
		rk_printf("ERROR!NULL list1\n");
		return NULL;
	}

	itemchild = item->child;
	while(itemchild)
	{
		if(strcmp(itemchild->string, "trackURIs") == 0)
		{
			printf("get trackURIs success\n");
			return itemchild->child->valuestring;
		}
		itemchild = itemchild->next;
	}
	printf("get trackURIs failed\n");
	return NULL;
}


/***********************************************************************************/
/********************************QPLAY PLAYLIST API**********************************/
/***********************************************************************************/

void player_init_qplay_list(void)
{
	memset(g_qplay_list,0x0,sizeof(QPLAY_PLAYLIST));
	g_player_p = NULL;
}

BOOL player_set_qplay_queueID(char *newid)
{
	if(g_qplay_list.queueID)
	{
		free(g_qplay_list.queueID);
		g_qplay_list.queueID = NULL;
	}
	g_qplay_list.queueID =(char *) malloc(strlen(newid) + 9);
	if(g_qplay_list.queueID)
	{
		sprintf(g_qplay_list.queueID,"%s%s",QPLAYFLAG,newid);
		return TRUE;
	}
	return FALSE;
}

void player_reset_qplay_list(BOOL resetPlayer)
{
	if(g_qplay_list.pTracksRoot)
	{
		cJSON_Delete(g_qplay_list.pTracksRoot);
	}
	g_qplay_list.tracksMetadata = NULL;
	g_qplay_list.tracksNum = 0;
	g_qplay_list.tracksCur = 0;
	g_qplay_list.tracksDuration = 0;
	g_qplay_list.pTracksRoot = NULL;
	g_qplay_list.pTracksArray = NULL;
	if(resetPlayer)
	{
		if(g_qplay_list.queueID)
		{
			free(g_qplay_list.queueID);
			g_qplay_list.queueID = NULL;
		}
		g_player_p = NULL;
	}
}


void *player_get_qplaylist_value(G_QPLAY_LIST_VALUE type)
{
	switch(type)
	{
		case QPLAY_LIST_VALUE_TRACKSMETADATA:
			return (void *)g_qplay_list.tracksMetadata;
		case QPLAY_LIST_VALUE_QUEUEID:
			return (void *)g_qplay_list.queueID;
		case QPLAY_LIST_VALUE_TRACKSNUM:
			return (void *)g_qplay_list.tracksNum;
		case QPLAY_LIST_VALUE_TRACKSCUR:
			return (void *)g_qplay_list.tracksCur;
		case QPLAY_LIST_VALUE_TRACKSDURATION:
			return (void *)g_qplay_list.tracksDuration;
		case QPLAY_LIST_VALUE_PTRACKSROOT:
			return (void *)g_qplay_list.pTracksRoot;					
		case QPLAY_LIST_VALUE_PTRACKSARRAY:
			return (void *)g_qplay_list.pTracksArray;
	}
}

char *player_get_curQPLAYtrack_metadata(void)
{
	cJSON* curtrack= NULL;

	curtrack = cJSON_GetArrayItem(g_qplay_list.pTracksArray, g_qplay_list.tracksCur);
	if(curtrack)
	{
		return cJSON_PrintUnformatted(curtrack);
	}
	return NULL;
}


BOOL player_seek_qplay_index(int index)
{
	char *old_url = NULL;
	int newindex = index-1;//JSON START WITH 0
	char *newurl = NULL;
	
	printf("qplay seek index=%d\n",index);
	set_qplay_seek_state(QPLAY_TRANSPORT_STATE_SEEKING);
	if(NULL == g_player_p)
	{
		rk_printf("ERROR,g_player_p is NULL!\n");
		return FALSE;
	}

	if(newindex > g_qplay_list.tracksNum || newindex<0)
	{
	    rk_printf("invalid index %d %d\n",newindex,g_qplay_list.tracksNum);
	    newindex = 0;
	}
	
	newurl = player_get_url_from_qplaylist(newindex);
	//if(newurl)
	//	printf("\nnewurl=%s\n",newurl);
	old_url = player_get_url(g_player_p);
	//if(old_url)
	//	printf("\nold_url=%s\n",old_url);
	if(newurl && old_url && (!strcmp(newurl,old_url)))
	{
	    printf("This url is already playing!\n");
		g_qplay_list.tracksCur = newindex;
	    return TRUE;
	}
	if(NULL == newurl)
	{
		rk_printf("ERROR,newurl is NULL\n");
		return FALSE;
	}

	player_set_url(g_player_p,newurl);
	g_qplay_list.tracksCur = newindex;
	//player_play(g_player_p);
	return TRUE;
}

extern int MplayerCtrl_Set_stating(int state);
BOOL player_seek_next_qplay_index(void)
{
	int newindex = g_qplay_list.tracksCur + 1;

	rk_printf("player seek next qplay_index\n");
	if(NULL == g_player_p)
	{
		rk_printf("ERROR,g_player_p 000\n");
		return TRUE;
	}
	
	if(newindex >= g_qplay_list.tracksNum)
		newindex = 0;
	if(TRUE == player_seek_qplay_index(newindex+1))//player_seek_qplay_index WIHILL CALL (INDEX-1)
	{	
		//MplayerCtrl_Set_stating(PLAYER_TRANSITIONING);
		player_play(g_player_p);//rk_printf("player_play 111\n");
		return TRUE;
	}/**/
	return FALSE;
}
#endif
