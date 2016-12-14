#include "BspConfig.h"

#include <rtos_config.h>
#if !defined(FREE_RTOS)
#define LOG_TAG "MediaRenderer"
#include <utils/Log.h>
#include "../../lollipop_wifi/socket_ipc/lollipop_socket_ipc.h"
#else
#include <utils/dlna_log.h>
#endif
#include "typedef.h"
#include "BSP.h"
#include "device.h"
#include "FileDevice.h"
#include "FATDevice.h"
#include "dlna.h"
#include <renderer/dmr-action.h>
#include <player/player.h>
#include <cybergarage/upnp/std/av/cmediarenderer.h>

#ifdef _QPLAY_ENABLE
#include "qplay_list.h"
#include "cJSON.h"
#include "md5_32.h"

#define QPLAY_TRACKURIS "trackURIs"//"{\"trackURIs\""

#define QPLAYNULLLIST ""//"{\"TracksMetaData\":[]}"//
#define QPLAYMAXTRACKS "100"
#define MANUFACTURID "xxxxxx"
//#define PRESHAREDKEY "xxxxxx"
#define DEVICETYPEID "xxxx"

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

// TrackMedata 存放在文件中
extern int g_TrackMedata;
extern int g_TrackMedata_xml;
extern void * FileSysHDC;
//unsigned long g_TrackMedata_len;


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


typedef struct
{
   //存放每一首歌曲的位置偏移
  unsigned long music_offset[400];   //目前默认存放四百收歌曲
  int music_totalnum;
  unsigned long music_totalduration;
  int music_cur;
}MEDATA_FILE;

MEDATA_FILE  g_medata_p;

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



//增加qqplayer认证服务

#if defined(__arm__) && defined(__ARMCC_VERSION)
__attribute__((used, section("CG_QPLAY_SERVICE_DESCRIPTION"))) static char CG_QPLAY_SERVICE_DESCRIPTION[] =
#elif defined(__arm__) && defined(__GNUC__)
static char __attribute__((used, section("__CG_QPLAY_SERVICE_DESCRIPTION")))CG_QPLAY_SERVICE_DESCRIPTION[] =
#else

#endif

"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
"<scpd xmlns=\"urn:schemas-tencent-com:service-1-0\">\n"
"<specVersion>\n"
"<major>1</major>\n"
"<minor>0</minor>\n"
"</specVersion>\n"
"<actionList>\n"
/*"<action>\n"
"<name>SetNetwork</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>SSID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_SSID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>Key</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Key</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>AuthAlgo</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_AuthAlgo</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>CipherAlgo</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_CipherAlgo</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"*/
"<action>\n"
"<name>QPlayAuth</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>Seed</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Seed</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>Code</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_Code</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>MID</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_MID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>DID</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_DID</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
/*
"<action>\n"
"<name>InsertTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
*/
/*
"<action>\n"
"<name>RemoveTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfTracks</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
*/
"<action>\n"
"<name>GetTracksInfo</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfTracks</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"

"<action>\n"
"<name>SetTracksInfo</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>QueueID</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_QueueID</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>StartingIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_StartingIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NextIndex</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NextIndex</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>TracksMetaData</name>\n"
"<direction>in</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_TracksMetaData</relatedStateVariable>\n"
"</argument>\n"
"<argument>\n"
"<name>NumberOfSuccess</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
/*
"<action>\n"
"<name>GetTracksCount</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>NrTracks</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
*/
"<action>\n"
"<name>GetMaxTracks</name>\n"
"<argumentList>\n"
"<argument>\n"
"<name>MaxTracks</name>\n"
"<direction>out</direction>\n"
"<relatedStateVariable>A_ARG_TYPE_NumberOfTracks</relatedStateVariable>\n"
"</argument>\n"
"</argumentList>\n"
"</action>\n"
"</actionList>\n"


"<serviceStateTable>\n"
/*
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_SSID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Key</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_AuthAlgo</name>\n"
"<dataType>string</dataType>\n"
"<allowedValueList>\n"
"<allowedValue>open</allowedValue>\n"
"<allowedValue>shared</allowedValue>\n"
"<allowedValue>WPA</allowedValue>\n"
"<allowedValue>WPAPSK</allowedValue>\n"
"<allowedValue>WPA2</allowedValue>\n"
"<allowedValue>WPA2PSK</allowedValue>\n"
"</allowedValueList>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_CipherAlgo</name>\n"
"<dataType>string</dataType>\n"
"<allowedValueList>\n"
"<allowedValue>none</allowedValue>\n"
"<allowedValue>WEP</allowedValue>\n"
"<allowedValue>TKIP</allowedValue>\n"
"<allowedValue>AES</allowedValue>\n"
"</allowedValueList>\n"
"</stateVariable>\n"
*/

"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Seed</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_Code</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_MID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_DID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"

"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_QueueID</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_StartingIndex</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_NextIndex</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_NumberOfTracks</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"<stateVariable sendEvents=\"no\">\n"
"<name>A_ARG_TYPE_TracksMetaData</name>\n"
"<dataType>string</dataType>\n"
"</stateVariable>\n"
"</serviceStateTable>\n"
"</scpd>\n";


BOOL rk_qplay_init(CgUpnpAvRenderer *dmr)
{
    CgUpnpDevice *dev;
    CgUpnpService *service;
    CgUpnpAction *action;

    dev = cg_upnpav_dmr_getdevice(dmr);
    if (!dev)
        return FALSE;

    service = cg_upnp_device_getservicebytype(dev, RK_QPLAY_SERVICE_TYPE);
    if (!service)
    {
        rk_printf("no qplay server");
        return FALSE;
    }

#if 1
    char *xml_buf = NULL;
    unsigned long xml_len= 0;
    if(Cg_Get_Xml(XML_DMR_QPLAY_SERVER_DESCRIPTION, &xml_buf, &xml_len) == -1)
    {
       rk_printf("xml get dmr qplay fail\n");
       return NULL;
    }
    if (cg_upnp_service_parsedescription(service, xml_buf, xml_len) == FALSE)
    {
        free(xml_buf);
        return FALSE;
    }
    free(xml_buf);
#else
    if (cg_upnp_service_parsedescription(service, CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION, cg_strlen(CG_UPNPAV_DMR_CONNECTIONMANAGER_SERVICE_DESCRIPTION)) == FALSE)
        return FALSE;
#endif
    cg_upnp_service_setuserdata(service, dmr);
    for (action=cg_upnp_service_getactions(service); action; action=cg_upnp_action_next(action))
        cg_upnp_action_setuserdata(action, dmr);
	player_init_qplay_list();
   return TRUE;
}


static BOOL qplay_medatafile_prase()
{
   //rk_printf("qplay_medatafile_prase");

   //获取当前歌曲数据 g_qplay_list.tracksNum  获取当前总的播放时间
     HDC trakhFile;
     FILE_ATTR stFileAttr;
     long file_len;
     int seek = 0;
     char *ptr_duration = NULL;
     char *ptr=NULL;
     char *ptr_trackuri = NULL;
     char *readbuf = NULL;
     int readlen = 0;
     int track_num = 0;
     char durationbuf[8] = {0};
     int hour = 0;
  	 int minute = 0;
  	 int second = 0;
     unsigned long duration_time = 0;
     unsigned long offset = 0;
     int i = 1;

     stFileAttr.Path = L"C:\\";
     stFileAttr.FileName = (uint16*)TRACK_DATA;

     trakhFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
     if((int)trakhFile < 0)
     {
        rk_printf("player_set_qplay_list file open fail\n");
	    return FALSE;
     }
     FileDev_GetFileSize(trakhFile, &file_len);

     readbuf = (char *)malloc(1025);
     if(readbuf == NULL)
     {
        FileDev_CloseFile(trakhFile);
        rk_printf("player_set_qplay_list readbuf malloc fail\n");
        return FALSE;
     }
     rk_printf("qplay_medatafile_prase");
     memset(&g_medata_p, 0, sizeof(MEDATA_FILE));

     while(1)
     {
        readlen = FileDev_ReadFile(trakhFile, readbuf, 1024*i);
        if(readlen < 0)
        {
            free(readbuf);
            FileDev_CloseFile(trakhFile);
            rk_printf("qplay_medatafile_prase readlen fail");
            return FALSE;
        }

        readbuf[readlen] = '\0';

       // printf("\n readbuf = %s\n", readbuf);

        ptr_trackuri = (char *)strstr(readbuf, "\"trackURIs\"");
        if(ptr_trackuri == NULL)
        {
            i++;
            readbuf = (char *)realloc(readbuf, 1024*i+1);

            if(seek+readlen >= file_len)
                break;
            //printf("aa");
        }
        else
        {
             offset = seek;
             offset += ptr_trackuri - readbuf;
             ptr_duration = (char *)strstr(ptr_trackuri, "duration");
             if(ptr_duration == NULL)
             {
                 //FileDev_FileSeek(trakhFile, SEEK_SET, offset);
                 i++;
                 readbuf = (char *)realloc(readbuf, 1024*i+1);
                 continue;
             }

             ptr = (char *)strchr(ptr_duration, '}');
             if(ptr == NULL)    //时间数据不完整的话，则重新申请memory开始读取
             {
                 //FileDev_FileSeek(trakhFile, SEEK_SET, offset);
                 i++;
                 readbuf = realloc(readbuf, 1024*i+1);
                 continue;
             }
             else
             {
                g_medata_p.music_offset[track_num] = offset;
                track_num++;
                memcpy(durationbuf, ptr_duration+11, 8);
                durationbuf[7] = '\0';
                //rk_printf("duration = %s",durationbuf);
                sscanf(durationbuf, "%d:%02d:%02d", &hour, &minute, &second);
  		        duration_time += (hour * 3600 + minute * 60 + second);

                //rk_printf("trak = %d,offset = %d",track_num-1, g_medata_p.music_offset[track_num-1]);
                seek +=  ptr - readbuf + 1;
                FileDev_FileSeek(trakhFile, SEEK_SET, seek);
                if(i>1)
                {
                    i=1;
                    free(readbuf);
                    readbuf =  malloc(1024+1);
                }
            }
        }
        if(seek + 2 >= file_len)
        {
           break;
        }

     }

     g_medata_p.music_totalnum = track_num;
     g_medata_p.music_totalduration = duration_time;

     rk_printf("totalnum = %d,totalduratio = %d",g_medata_p.music_totalnum, g_medata_p.music_totalduration);

     free(readbuf);
     FileDev_CloseFile(trakhFile);

     return TRUE;
}

static char *player_get_url_from_MedataFile(int index)
{
     HDC trakhFile;
     FILE_ATTR stFileAttr;
     unsigned long file_len;
     char *ptr_Track = NULL;
     char *ptr=NULL;
     char *readbuf = NULL;
     unsigned int readlen = 0;
     unsigned long offset = 0;
     int i =0;
     int j = 0;

    // rk_printf("player_get_url_from_MedataFile = %d", index );
	// index 0 start
     if(index >= g_medata_p.music_totalnum || index < 0)  // 0- g_medata_p.music_totalnum -1
     {
        rk_printf("player_get_url_from_MedataFile index fail");
        return NULL;
     }

     // startfrom 1
     stFileAttr.Path = L"C:\\";
     stFileAttr.FileName = (uint16*)TRACK_DATA;

     trakhFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
     if((int)trakhFile < 0)
     {
        rk_printf("player_get_url_from_MedataFile file open fail\n");
	    return FALSE;
     }
     FileDev_GetFileSize(trakhFile, &file_len);

   #if 0
     FileDev_FileSeek(trakhFile, SEEK_SET, 0);
     readbuf = malloc(file_len+1);
     readlen = FileDev_ReadFile(trakhFile, readbuf, file_len);
     readbuf[readlen] = '\0';
      printf("\n MedataFileradbuf = %s \n", readbuf);
      free(readbuf);
   #endif

     offset = g_medata_p.music_offset[index];
     if(index ==  g_medata_p.music_totalnum - 1)
     {
         readlen =file_len - g_medata_p.music_offset[index];
     }
     else
     {
        readlen = g_medata_p.music_offset[index+1] - g_medata_p.music_offset[index];
     }

     rk_printf("index = %d, readlen  = %d, offset = %d",index, readlen, offset);
     readbuf = malloc(readlen+1);

     FileDev_FileSeek(trakhFile, SEEK_SET, offset);
     readlen = FileDev_ReadFile(trakhFile, readbuf, readlen);
     if(readlen < 0)
     {
         rk_printf("player_get_url_from_MedataFile fail");
         free(readbuf);
         FileDev_CloseFile(trakhFile);
         return FALSE;
     }
     readbuf[readlen] = '\0';
    // printf("\n MedataFileradbuf22 = %s \n", readbuf);

    // ptr_Track = (char *)strstr(readbuf, "\"trackURIs\"");

     ptr_Track = (char *)strstr(readbuf, "http");
     ptr = (char *)strchr(ptr_Track, '"');

     char *url_buf = NULL;
     int url_len =0;

     url_len = ptr - ptr_Track;  //
     url_buf = malloc(url_len);
     memcpy(url_buf, ptr_Track, url_len);

     url_buf[url_len] = '\0';
     printf("\nurl_buf = %s\n", url_buf);

     #if 0
     i=0;
     j = 0;
     while((ptr_Track+14+i) <= (ptr-2))
     {
        //if(*(ptr_Track+15+i) != '\\')
        //{
          url_buf[j] = *(ptr_Track+14+i);
          i++;
          j++;
        //}
        //else
        //{
          //i++;
        //}
     }
     url_buf[j-1]  = '\0';
     //printf("\nurl_buf = %s\n", url_buf);
     #endif
     g_medata_p.music_cur = index;
     free(readbuf);
     FileDev_CloseFile(trakhFile);

     return url_buf;
}


int qplay_httptrackmedata_packet_write_file(CgHttpPacket *httpPkt, CgSocket *sock,unsigned long conLen)
{

    HDC hFile;
    FILE_ATTR stFileAttr;
    ssize_t leftLen = conLen;
    char *content = NULL;
    ssize_t readLen, readLen1;
    int tries = 0;

    if(conLen < CG_HTTP_LEN_MAX)
        return 2;    // 等于2列表比较少不用进行文件缓存

    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = (uint16*)HTTP_CONTENT;

    hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if((int)hFile > 0)
    {
        FileDev_CloseFile(hFile);
        FileDev_DeleteFile(FileSysHDC, NULL, &stFileAttr);
    }

    if(FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr) == RK_SUCCESS)
    {
        hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    }

    if((int)hFile <= 0)
    {
        rk_printf("create file fail\r\n");
        return FALSE;
    }

    content = NULL;
    content = (char *)malloc(512+1);
    if (content == NULL)
    {
        cg_log_debug_s("http file Memory malloc problem!\n");
        return FALSE;
    }
    memset(content, 0, 512);
    readLen = 0;

    while (readLen < conLen && tries < 20)
    {
        if(conLen - readLen >= 512)
        {
            readLen1 = cg_socket_read(sock, content, 512);
        }
        else
        {
            readLen1 = cg_socket_read(sock, content, conLen - readLen );
        }
        if (readLen1 <= 0)
        {
            readLen1 = 0;
            tries++;

        }
        else
        {
            tries = 0;
            content[readLen1] = '\0';
            FileDev_WriteFile(hFile, content, readLen1);
            readLen += readLen1;
        }

        if(tries>20)
        {
             free(content);
             FileDev_CloseFile(hFile);
             return FALSE;
        }
    }

    httpPkt->content = NULL;
    httpPkt->cgfile_flag = 1;
    httpPkt->conlen = conLen;
    FileDev_CloseFile(hFile);
    free(content);
}
//áD±í·μ??
void qplay_httptrackmedata_response_from_file(CgHttpPacket *httpPkt, CgSocket *sock)
{
    HDC Track_hFile;
    FILE_ATTR stFileAttr;
    char *read_buf = NULL;
    unsigned long fileseek = 0;
    long read_len = 0;
    int i = 1;
    unsigned long filelen = 0;
    int ret = 0;
   // char *cur_ptr = NULL;
    char *next_ptr = NULL;
    char *send_buf = NULL;
    long send_len = 0;
    char *file_ptr = NULL;
    char *content;
    size_t contentLen;

    content = cg_http_packet_getcontent(httpPkt);
    contentLen = cg_http_packet_getcontentlength(httpPkt);

   // cur_ptr = content;
    next_ptr = (char *)strstr(content, "<CurrentURIMetaData>");
    if(next_ptr == NULL)
    {
        printf("current nextptr = NULL");
        return;
    }
    send_len = next_ptr-content+20;
    send_buf = malloc(send_len+ 1);
    memcpy(send_buf, content, send_len);
    send_buf[send_len] = '\0';

   // printf("\n send_buf = %s", send_buf);
    cg_socket_write(sock, send_buf, send_len);
    free(send_buf);

   // printf("\nsocket ok\n");
    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = (uint16 *)HTTP_CONTENT;
    Track_hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if((int)Track_hFile < 0)
    {
        rk_printf("cg_http_packet_post file open file");
        return;
    }
    FileDev_GetFileSize(Track_hFile, &filelen);
    if(filelen <= 0)
    {
        rk_printf("cg_http_packet_post <= 0");
        FileDev_CloseFile(Track_hFile);
        return;
    }

    read_buf = malloc(1025);
    if(read_buf == NULL)
    {
        rk_printf("cg_http_packet_post read_buf malloc fail");
        FileDev_CloseFile(Track_hFile);
        return;
    }

    FileDev_FileSeek(Track_hFile, SEEK_SET, 0);
    i=1;
    while(1)
    {
        read_len = FileDev_ReadFile(Track_hFile, read_buf, 1024*i);
        read_buf[read_len] = '\0';
        //printf("\nread_buf = %s   &&&&&&&& memory = %d\n", read_buf, rkos_GetFreeHeapSize());
        file_ptr = (char *)strstr(read_buf, "<TracksMetaData>"); //<TracksMetaData>
        if(file_ptr == NULL)
        {
           i++;
           read_buf = realloc(read_buf,1024*i+1);
        }
        else
        {
           fileseek = file_ptr-read_buf+16;
           FileDev_FileSeek(Track_hFile, SEEK_SET, fileseek);
           break;
        }
    }

    while(1)
    {
        read_len = FileDev_ReadFile(Track_hFile, read_buf, 1024);
        if(read_len > 0)
        {

            file_ptr = (char *)strstr(read_buf, "</TracksMetaData>"); //</TracksMetaData>
            if(file_ptr == NULL)
            {
                read_buf[read_len] = '\0';
                //printf("%s", read_buf);
                cg_socket_write(sock, read_buf, read_len);
                fileseek += read_len -20;
                FileDev_FileSeek(Track_hFile, SEEK_SET, fileseek);
            }
            else
            {
                send_len = file_ptr - read_buf;
                read_buf[send_len] = '\0';
                //printf("%s", read_buf);
                cg_socket_write(sock, read_buf, read_len);

                free(read_buf);
                FileDev_CloseFile(Track_hFile);
                break;
            }
       }
    }
//</CurrentURIMetaData>
    //cur_ptr = content;
    next_ptr = (char *)strstr(content, "<\/CurrentURIMetaData");
    if(next_ptr == NULL)
    {
        printf("current nextptr = NULL");
        return;
    }
    send_len =contentLen - (next_ptr-content);
    send_buf = malloc(send_len+ 1);
    memcpy(send_buf, next_ptr, send_len);
    send_buf[send_len] = '\0';

    //printf("%s\n", send_buf);
    cg_socket_write(sock, send_buf, send_len);
    free(send_buf);

    return;
}



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
	md5code = Qplay_MD5Create(str_seed_prekey);//32bit
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

//add lyb 2016/11/29
    free(md5code);
//end
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

    if(g_TrackMedata_xml == 1)
    {
      qplay_medatafile_prase();
      g_TrackMedata = 1;
    }
    else
    {
      g_TrackMedata = 0;
    }

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
    if(g_TrackMedata == 1)
    {
         player_reset_qplay_list(FALSE);
         player_set_qplay_list(startingindex);
    }
    else if(tracksmetadata)
	{
        player_reset_qplay_list(FALSE);
		g_qplay_list.tracksMetadata = tracksmetadata;  //g_TrackMedata =1 "NULL"
		player_set_qplay_list(startingindex);
	}
	
	//set NumberOfSuccess(the song num of TracksMetaData)
	arg = cg_upnp_action_getargumentbyname(action, RK_QPLAY_SERVICE_SETTRACKSINFO_NSUCCESS);
	if (!arg)
		return FALSE;
	sprintf(successstr, "%d", g_qplay_list.tracksNum);
	cg_upnp_argument_setvalue(arg, successstr);
	rk_printf("NumberOfSuccess = %s", arg->value->value);

	return TRUE;

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

    //rk_printf("qplay_action received");

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

    if(g_TrackMedata == 0) // 没有写到文件里直接进行json解析
    {
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
    }
    else   //存放到文本中需要从文本中进行解析
    {
         g_qplay_list.tracksDuration = g_medata_p.music_totalduration;
         g_qplay_list.tracksNum = g_medata_p.music_totalnum;
    }

	return TRUE;
}

static char *player_get_url_from_qplaylist(int index)//(int handle, char *str)
{
	cJSON* item= NULL;
	cJSON* itemchild=NULL;


    if(g_TrackMedata == 0)
    {

        rk_printf("player_get_url_from_qplaylist index = %d", index);
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
    else
    {
        return player_get_url_from_MedataFile(index);
    }
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
			return (void *)(g_medata_p.music_cur);//g_qplay_list.tracksCur;
		case QPLAY_LIST_VALUE_TRACKSDURATION:
			return (void *)g_qplay_list.tracksDuration;
		case QPLAY_LIST_VALUE_PTRACKSROOT:
			return (void *)g_qplay_list.pTracksRoot;
		case QPLAY_LIST_VALUE_PTRACKSARRAY:
			return (void *)g_qplay_list.pTracksArray;
	}
}


char *player_get_curQPLAYtrack_metadata_from_file(void)
{
     HDC trakhFile;
     FILE_ATTR stFileAttr;
     long file_len;
     char *ptr_Track = NULL;
     char *ptr=NULL;
     char *readbuf = NULL;
     int readlen = 0;
     int offset = 0;

     stFileAttr.Path = L"C:\\";
     stFileAttr.FileName = (uint16*)TRACK_DATA;

     trakhFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
     if((int)trakhFile < 0)
     {
        rk_printf("player_get_url_from_MedataFile file open fail\n");
	    return FALSE;
     }
     FileDev_GetFileSize(trakhFile, &file_len);

     offset = g_medata_p.music_offset[g_medata_p.music_cur];
     if(g_medata_p.music_cur ==  g_medata_p.music_totalnum - 1)
     {
         readlen =file_len - g_medata_p.music_offset[g_medata_p.music_cur];
     }
     else
     {
        readlen = g_medata_p.music_offset[g_medata_p.music_cur+1] - g_medata_p.music_offset[g_medata_p.music_cur];
     }

     readbuf = (char *)malloc(readlen+1);
     if(readbuf == NULL)
     {
        FileDev_CloseFile(trakhFile);
        rk_printf("player_get_url_from_MedataFilet readbuf malloc fail\n");
        return FALSE;
     }

     //readbuf
     FileDev_FileSeek(trakhFile, SEEK_SET, offset);
     readlen = FileDev_ReadFile(trakhFile, readbuf, readlen-1);
     if(readlen < 0)
     {
         rk_printf("player_get_url_from_MedataFile fail");
         free(readbuf);
         FileDev_CloseFile(trakhFile);
         return FALSE;
     }
     readbuf[readlen] = '\0';
     return readbuf;
}


char *player_get_curQPLAYtrack_metadata(void)
{

    if(g_TrackMedata == 0)
    {
    	cJSON* curtrack= NULL;

    	curtrack = cJSON_GetArrayItem(g_qplay_list.pTracksArray, g_qplay_list.tracksCur);
    	if(curtrack)
    	{
    		return cJSON_PrintUnformatted(curtrack);
    	}
    }
    else
    {
        return player_get_curQPLAYtrack_metadata_from_file();
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
   //tracjsNum ?è?úμ?êy??￡?
	if(newindex >= g_qplay_list.tracksNum || newindex<0)
	{
	    rk_printf("invalid index %d %d\n",newindex,g_qplay_list.tracksNum);
	    newindex = 0;
	}

	newurl = player_get_url_from_qplaylist(newindex);
	if(newurl)
		printf("newurl=%s",newurl);
	old_url = player_get_url(g_player_p);
	//if(old_url)
	//	printf("\nold_url=%s\n",old_url);
	if(newurl && old_url && (!strcmp(newurl,old_url)))
	{
	    printf("This url is already playing!\n");
		g_qplay_list.tracksCur = newindex;
        if(g_TrackMedata == 1)
        {
             g_medata_p.music_cur = newindex;
        }
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
