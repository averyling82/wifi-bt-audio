
#include "BspConfig.h"
#ifdef __WIFI_DLNA_C__
#define NOT_INCLUDE_OTHER

#include "typedef.h"
#include "RKOS.h"
#include "global.h"
#include "SysInfoSave.h"
#include "TaskPlugin.h"
#include "device.h"
#include "DriverInclude.h"
#include "DeviceInclude.h"
#include "http.h"
#include "AudioControlTask.h"
#include <controller.h>

#include "FreeRTOS.h"
#include "timers.h"

#define DLNA_AYNC 1

typedef  struct _MPLAYER_RESP_QUEUE
{
    uint32 cmd;
    int status;

}MPLAYER_RESP_QUEUE;

typedef  struct _MPLAYER_ASK_QUEUE
{
    uint32 cmd;
    char *url_buf;
    int state;
    int url_ms;
}MPLAYER_ASK_QUEUE;

typedef  struct _MPLAYER_TASK_DATA_BLOCK
{
    pQueue  MPLAYERAskQueue;
    pQueue  MPLAYERRespQueue;
}MPLAYER_TASK_DATA_BLOCK;

static MPLAYER_TASK_DATA_BLOCK * gpstMPLAYERData;

//#define PLAYER_TEST
ctrl_t mplayer_ctrl;
static HDC PAudio = NULL;
static HDC hFifo = NULL;
static pstate_t play_state;
static state_listener_t play_state_changed;
#ifdef PLAYER_TEST
static uint32 systime_backup =0;
#endif
int player_flag = 0;

typedef enum __MPLAY_STATE
{
    MPLAYER_STATE_PLAY = 0,
    MPLAYER_STATE_STOP,
    MPLAYER_STATE_SEEK,
    MPLAYER_STATE_PAUSE,
    MPLAYER_STATE_RESUME,
    MPLAYER_STATE_TRANSTRING,
    DLNA_STOP,
}MPLAY_STATE;


HDC *DlnaHttp_Pcb;
uint8 left_packet[512];
uint8 *dlna_packet = NULL;
uint16 dlna_left_len = 0;
rk_err_t dlna_write_fifo(uint8 *buf, uint16 write_len, uint32 mlen)
{
    uint16 i,j;
    rk_err_t ret;
    uint16 data_len;

    data_len = dlna_left_len + write_len;
    dlna_packet = rkos_memory_malloc(data_len);
    if(dlna_packet == NULL)
    {
        rk_printf("dlna_write_fifo malloc fail");
        return RK_ERROR;
    }

    if(dlna_left_len > 0)
    {
        rkos_memcpy(dlna_packet, left_packet, dlna_left_len);
    }

    rkos_memcpy(dlna_packet+dlna_left_len, buf, write_len);

    i = data_len/512;
    j = data_len %512;

    if(i>0)
    {
       ret = fifoDev_Write(hFifo, dlna_packet, i*512, SYNC_MODE, NULL);
       if(ret == RK_ERROR)
       {
            rk_printf("dlna fifo write error1");
            rkos_memory_free(dlna_packet);
            dlna_packet = NULL;
            return RK_ERROR;
       }
    }

    if(j > 0)
    {
        dlna_left_len = j;
        if(mlen  > 0)
        {
            rkos_memcpy(left_packet, dlna_packet+i*512, j);

        }
        else
        {
            dlna_left_len = 0;
            rkos_memset(left_packet, 0x00, 512);
            ret = fifoDev_Write(hFifo, dlna_packet+i*512, j, SYNC_MODE, NULL);
            if(ret == RK_ERROR)
            {
                 rk_printf("dlna fifo write error2");
                 rkos_memory_free(dlna_packet);
                 dlna_packet = NULL;
                 return RK_ERROR;
            }
        }
    }
    else
    {
        dlna_left_len = 0;
    }
    rkos_memory_free(dlna_packet);
    dlna_packet = NULL;

    return RK_SUCCESS;
}


void Player_Sate(unsigned int audio_state)
{

    if(player_flag == 0)
        return;


    if (audio_state == AUDIO_STATE_STOP)
    {
#ifdef PLAYER_TEST
        //uint32 state;

        //AudioPlayer_GetState(&state);
        if(state == AUDIO_STATE_STOP)
        {
             rk_printf("audio controlstop ****************************************");
        }
#else
        rk_printf("audio controlstop  ****************************************");
        MplayerCtrl_Set_stating(PLAYER_STOPPED);
#endif
    }
    else if(audio_state == AUDIO_STATE_PLAY)
    {
#ifdef PLAYER_TEST
        rk_printf("play ****************************************");
#else
        rk_printf("audio controlplay  ****************************************");
        MplayerCtrl_Set_stating(PLAYER_PLAYING);
#endif
    }
    else if(audio_state == AUDIO_STATE_PAUSE)
    {
        rk_printf("audio controlpause ****************************************");
        MplayerCtrl_Set_stating(PLAYER_PAUSED);
    }

}

int MplayerCtrl_Init(void)
{
#ifdef PLAYER_TEST
    RKTaskCreate(TASK_ID_DLNA_PLAYER, 0, NULL, SYNC_MODE);
#else
    RK_TASK_AUDIOCONTROL_ARG pArg;

    rk_printf("MplayerCtrl_Init");
    play_state = PLAYER_STOPPED;
    pArg.ucSelPlayType = SOURCE_FROM_DLNA;
    pArg.FileNum = 1;

    pArg.pfAudioState = Player_Sate;

    //memcpy(pArg.filepath, "http://192.168.1.100/%E6%89%93%E8%80%81%E8%99%8E.MP3", 200);

    RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &pArg, SYNC_MODE);
    RKTaskCreate(TASK_ID_DLNA_PLAYER, 0, NULL, SYNC_MODE);

    PAudio = RKDev_Open(DEV_CLASS_AUDIO, 0, NOT_CARE);
    if ((PAudio == NULL) || (PAudio == (HDC)RK_ERROR) || (PAudio == (HDC)RK_PARA_ERR))
    {
        rk_printf("Audio device open failure");

        return RK_ERROR;
    }

    hFifo = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    if ((hFifo == NULL) || (hFifo == (HDC)RK_ERROR) || (hFifo == (HDC)RK_PARA_ERR))
    {
        rk_printf("Audio device open failure");

        return RK_ERROR;
    }

#endif
    player_flag = 1;
    return 0;
}

int MplayerCtrl_Deinit(void)
{
    MPLAYER_ASK_QUEUE play_ask1 = {0};
    MPLAYER_RESP_QUEUE play_resp;
    MPLAYER_ASK_QUEUE play_ask;
    rk_err_t ret1 = 0;


    player_flag = 0;
    while(1)
    {
        ret1 = rkos_queue_receive(gpstMPLAYERData->MPLAYERAskQueue, &play_ask1, 0);
        if(ret1 == RK_ERROR)
        {
            break;
        }
        else
        {
            if(play_ask1.cmd == MPLAYER_STATE_TRANSTRING && play_ask1.url_buf != NULL)
            {
                rkos_memory_free(play_ask1.url_buf);
                play_ask1.url_buf = NULL;
            }
        }
    }

    play_state = PLAYER_STOPPED;
    if(PAudio != NULL)
    {
       RKDev_Close(PAudio);
       PAudio = NULL;
    }
    if(hFifo != NULL)
    {
       RKDev_Close(hFifo);
       hFifo = NULL;
    }

    play_ask.cmd = DLNA_STOP;
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);

    RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, DIRECT_MODE);
    RKTaskDelete(TASK_ID_DLNA_PLAYER, 0, DIRECT_MODE);
   rk_printf("mplayer ctrl_deinit");
    return 0;
}


void dlna_stop(void)
{
    MPLAYER_RESP_QUEUE play_resp;

    if(DlnaHttp_Pcb != NULL)
    {
        Http_Close(DlnaHttp_Pcb);
        DlnaHttp_Pcb = NULL;
    }
    play_resp.cmd = DLNA_STOP;
    rkos_queue_send(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);
    return;
}

int MplayerCtrl_Play(char *url)
{
    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_RESP_QUEUE play_resp;


    //printf("****MplayerCtrl_play, =  %d\n", play_state);
    //return;
    play_ask.cmd = MPLAYER_STATE_PLAY;
    play_ask.url_buf = url;

#if DLNA_AYNC
    rk_err_t ret = 0;

    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
        rk_printf("mplayerctrl_play send queue fail");
        return -1;
    }

    return 0;
#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);
    if((play_resp.cmd == MPLAYER_STATE_PLAY) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif

}


int MplayerCtrl_Playing(char *url)
{
    int dwRet = 0;
    uint32 state;
    RK_TASK_AUDIOCONTROL_ARG pArg;
    rk_err_t ret = RK_ERROR;

    rk_printf("paly play_state = %d", play_state);
    //return 0;
#ifdef PLAYER_TEST
    if(play_state == PLAYER_PLAYING)
    {
        return -1;
    }
    dwRet = HttpGet_Url(url, NULL, 0);
    if(dwRet == 0)
    {
        rk_printf("http ok");
        MplayerCtrl_Set_stating(PLAYER_PLAYING);
        systime_backup = xTaskGetTickCount();
        return 0;
    }
    else
    {
        rk_printf("http error");
        MplayerCtrl_Set_stating(PLAYER_STOPPED);
        return -1;
    }
#else

    if(MplayerCtrl_Get_state() == PLAYER_PLAYING)
    {
        //MplayerCtrl_Set_stating(PLAYER_PLAYING);
        return -1;
    }

#if DLNA_AYNC

    if(MplayerCtrl_Get_state() == PLAYER_TRANSITIONING)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, SYNC_MODE);
    }
    return 0;
#else

    if(play_state == PLAYER_PAUSED)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_RESUME, NULL, SYNC_MODE);
        MplayerCtrl_Set_stating(PLAYER_PLAYING);
        return 0;
    }
    else if(play_state == PLAYER_TRANSITIONING)
    {

        //MplayerCtrl_Set_stating(PLAYER_PLAYING);
        ret= AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, SYNC_MODE);
        if(ret == RK_SUCCESS)
        {
            MplayerCtrl_Set_stating(PLAYER_PLAYING);
            rk_printf("send play music ok");
            return 0;
        }
        else
        {
            MplayerCtrl_Set_stating(PLAYER_STOPPED);
            return -1;
        }
    }

#endif


#endif
}


int MplayerCtrl_Stop(void)
{

    rk_printf("****MplayerCtrl_Stop, =  %d", play_state);
    //return;
    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_ASK_QUEUE play_ask1 = {0};
    MPLAYER_RESP_QUEUE play_resp;

    play_ask.cmd = MPLAYER_STATE_STOP;

#if DLNA_AYNC
    rk_err_t ret = 0;
    rk_err_t ret1 = 0;

    while(1)
    {
        ret1 = rkos_queue_receive(gpstMPLAYERData->MPLAYERAskQueue, &play_ask1, 0);
        if(ret1 == RK_ERROR)
        {
            break;
        }
        else
        {
            if(play_ask1.cmd == MPLAYER_STATE_TRANSTRING && play_ask1.url_buf != NULL)
            {
                rkos_memory_free(play_ask1.url_buf);
                play_ask1.url_buf = NULL;
            }
        }
    }

    AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
    rk_printf("audio stop cmd ok");
    //xQueueReset(gpstMPLAYERData->MPLAYERAskQueue);
    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
          rk_printf("stop send queue fail");
          return -1;
    }

    return 0;
#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);

    if((play_resp.cmd == MPLAYER_STATE_STOP) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif

}


int MplayerCtrl_Stoping(void)
{
    rk_printf("MplayerCtrl_Stoping, =  %d", play_state);
    //return 0;
    //if (MplayerCtrl_Get_state() == PLAYER_STOPPED)
    //{
        //MplayerCtrl_Set_stating(PLAYER_STOPPED);
      //  return -1;
   // }

#ifdef PLAYER_TEST
    Http_Close();

    MplayerCtrl_Set_stating(PLAYER_STOPPED);
    return 0;
#else

    //AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
   // rk_printf("audio stop cmd ok");

    if(DlnaHttp_Pcb != NULL)
    {
        Http_Close(DlnaHttp_Pcb);
        DlnaHttp_Pcb = NULL;
    }
    MplayerCtrl_Set_stating(PLAYER_STOPPED);
    return 0;

#endif
}

int MplayerCtrl_Pause(void)
{

    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_RESP_QUEUE play_resp;
    //return;
    play_ask.cmd = MPLAYER_STATE_PAUSE;
    rk_printf("MplayerCtrl_Pause = %d", play_state);

#if DLNA_AYNC
    rk_err_t ret = 0;

    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
        rk_printf("send pause queue fail");
        return -1;
    }
    return 0;
#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);
    if((play_resp.cmd == MPLAYER_STATE_PAUSE) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif

}

int MplayerCtrl_Pausing(void)
{

    //return 0;
#ifdef PLAYER_TEST
   if (play_state == PLAYER_PLAYING){
        MplayerCtrl_Set_stating(PLAYER_PAUSED);
        return 0;
    }
    return -1;

#else

    if (play_state == PLAYER_PAUSED){
        //MplayerCtrl_Set_stating(PLAYER_PAUSED);
        return -1;
    }

    if (play_state == PLAYER_PLAYING){
        AudioControlTask_SendCmd(AUDIO_CMD_PAUSE, NULL, SYNC_MODE);
        rk_printf("audio send pause ok");
    }

    //MplayerCtrl_Set_stating(PLAYER_PAUSED);
    return 0;
#endif
}

int MplayerCtrl_Resume(void)
{
    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_RESP_QUEUE play_resp;
    play_ask.cmd = MPLAYER_STATE_RESUME;
    rk_printf("MplayerCtrl_Resume = %d", play_state);

    //return;
#if DLNA_AYNC
    rk_err_t ret = 0;
    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
        rk_printf("resume send queue fail");
        return -1;
    }

    return 0;
#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);

    if((play_resp.cmd == MPLAYER_STATE_RESUME) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif
}

int MplayerCtrl_Resuming(void)
{

    //return 0;

#ifdef PLAYER_TEST
    if (play_state == PLAYER_PAUSED) {

        //MplayerCtrl_Set_state(PLAYER_TRANSITIONING);
        //rkos_delay(20);
        MplayerCtrl_Set_stating(PLAYER_PLAYING);
        //mp_send_get_pos_cmd();
        return 0;
    }
#else
   if (play_state == PLAYER_PLAYING){
        //MplayerCtrl_Set_stating(PLAYER_PLAYING);
        return -1;
    }
    AudioControlTask_SendCmd(AUDIO_CMD_RESUME, NULL, SYNC_MODE);
    //MplayerCtrl_Set_stating(PLAYER_PLAYING);

    return 0;
#endif
}

int MplayerCtrl_Seek(char *url, int ms)
{
    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_RESP_QUEUE play_resp;

    play_ask.cmd = MPLAYER_STATE_SEEK;
    play_ask.url_ms = ms;
    play_ask.url_buf = url;

    //return 0;
#if DLNA_AYNC
    rk_err_t ret = 0;

    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
        rk_printf("seek send queue fail");
        return -1;
    }
    return 0;
#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);

    if((play_resp.cmd == MPLAYER_STATE_SEEK) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif

}

int MplayerCtrl_Seeking(char *url, int ms)
{
    int ret = 0;

    rk_printf("MplayerCtrl_Seek ms = %d", ms);

    AudioControlTask_SendCmd(AUDIO_CMD_SEEKTO, (void *)ms, SYNC_MODE);

    MplayerCtrl_Set_stating(PLAYER_PLAYING);

    return 0;
}

int MplayerCtrl_Current_time(char* buf, int len)
{
    int hour;
    int minute;
    int second;
    uint32 time;
#ifdef PLAYER_TEST
    if(play_state == PLAYER_STOPPED)
    {
        time = 0;
        systime_backup = xTaskGetTickCount();
    }
    else
    {
        rk_printf("MplayerCtrl_Current_time");
        time = (xTaskGetTickCount()- systime_backup)*10;
        //systime_backup = xTaskGetTickCount();
    }

    hour = time/(1000*3600);
    time = time%(1000*3600);
    minute = time/(1000*60);
    time = time%(1000*60);
    second = time/1000;

    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);

    return 0;
#else

    if(play_state != PLAYER_PLAYING)
    {
        time = 0;
    }
    else
    {
        AudioPlayer_GetCurTime(&time);
        //printf("curtime = %d\n", time);
    }
    hour = time/(1000*3600);
    time = time%(1000*3600);
    minute = time/(1000*60);
    time = time%(1000*60);
    second = time/1000;

    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
        //rk_printf("current_time = %s\n",buf);

    return 0;
#endif
}




int MplayerCtrl_Get_duration(char* buf, int len)
{

    int hour;
    int minute;
    int second;
    uint32 time;
#ifdef PLAYER_TEST
    uint32 time1;
    static uint32 systime =0;
    rk_printf("MplayerCtrl_Get_duration remain = %d", rkos_GetFreeHeapSize);
    if(play_state == PLAYER_STOPPED)
    {
         time = 194*1000;
         time1 = 0;
         systime = xTaskGetTickCount();
    }
    else
    {
        time = 194*1000;
        time1 = 0;
    }
    time -= time1;
   // rk_printf("totle_time11 = %d\n", time);
    hour = time/(1000*3600);
    time = time%(1000*3600);
    minute = time/(1000*60);
    time = time%(1000*60);
    second = time/1000;

    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
    //printf("duration = %s\n",buf);
#else

    if(play_state != PLAYER_PLAYING)
    {
         time = 0;
    }
    else
    {

        AudioPlayer_GetTotalTime(&time);
    }
   // printf("totaltime = %d\n", time);


    hour = time/(1000*3600);
    time = time%(1000*3600);
    minute = time/(1000*60);
    time = time%(1000*60);
    second = time/1000;

    sprintf(buf, "%02d:%02d:%02d", hour, minute, second);
    //rk_printf("duration = %s\n",buf);

    return 0;
#endif
}

int MplayerCtrl_Get_state(void)
{
    return (int)play_state;
}

int MplayerCtrl_Set_state(int state, char *url)
{

    MPLAYER_ASK_QUEUE play_ask;
    MPLAYER_RESP_QUEUE play_resp;
    uint16 url_len = strlen(url);

    rk_printf("&&&&&&&&state = %d", state);


    play_ask.cmd = MPLAYER_STATE_TRANSTRING;
    play_ask.state = state;
    play_ask.url_buf = rkos_memory_malloc(url_len+1);


    if(play_ask.url_buf == NULL)
    {
        rk_printf("play_ask.url_buf malloc fail");
        return -1;
    }

    rkos_memcpy(play_ask.url_buf, url, url_len+1);


#if DLNA_AYNC
    rk_err_t ret =0;

    ret = rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, 0);
    if(ret == RK_ERROR)
    {
        rkos_memory_free(play_ask.url_buf);
        rk_printf("set state send queue fail");
        return -1;
    }

    //rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);
    return 0;

#else
    rkos_queue_send(gpstMPLAYERData->MPLAYERAskQueue, &play_ask, MAX_DELAY);
    rkos_queue_receive(gpstMPLAYERData->MPLAYERRespQueue, &play_resp, MAX_DELAY);
    if((play_resp.cmd == MPLAYER_STATE_TRANSTRING) && (play_resp.status == 0))
    {
        return 0;
    }
    else
    {
        return -1;
    }
#endif

}

int RK_MplayerCtrl_transtring(int state, char *url)
{
    int ret = 0;
    uint32 audio_state;

#if 1
    AudioPlayer_GetState(&audio_state);
    if(audio_state != AUDIO_STATE_STOP)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
        rk_printf("transtring stop audio ok");
        if(DlnaHttp_Pcb != NULL)
        {
            Http_Close(DlnaHttp_Pcb);
            DlnaHttp_Pcb = NULL;
        }
        MplayerCtrl_Set_stating(PLAYER_STOPPED);
        rkos_memory_free(url);
        return -1;
    }
#endif

    if (play_state == PLAYER_TRANSITIONING){
        MplayerCtrl_Set_stating(PLAYER_TRANSITIONING);
        rkos_memory_free(url);
        return -1;
    }

    DlnaHttp_Pcb = HttpPcb_New(NULL, dlna_write_fifo, FIFOWRITE);
    if(DlnaHttp_Pcb == NULL)
    {
        rk_printf("dlan http pcb malloc fail");
        MplayerCtrl_Set_stating(PLAYER_STOPPED);
        return -1;
    }

    dlna_left_len = 0;
    //fifoDev_SetTotalSize(hFifo, 0);
    ret = HttpGet_Url(DlnaHttp_Pcb, url, 0);
    if(ret ==RK_ERROR)
    {
        if(url != NULL)
        {
            rkos_memory_free(url);
        }
        MplayerCtrl_Set_stating(PLAYER_STOPPED);
        return -1;
    }

    if(url != NULL)
    {
        rkos_memory_free(url);
    }

    MplayerCtrl_Set_stating(PLAYER_TRANSITIONING);
    return 0;

}

int MplayerCtrl_Set_stating(int state)
{
    rk_printf("MplayerCtrl_Set_state state = %d", state);

    //if (play_state == state) {
       // return -1;
   // }
    play_state = state;
    if (play_state_changed)
        play_state_changed(play_state);

    return 0;
}

int MplayerCtrl_State_listener(state_listener_t listener)
{
    rk_printf("MplayerCtrl_State_listener");
    play_state_changed = listener;
    return 0;
}

int MplayerCtrl_Set_volume(int volume)
{
    rk_printf("MplayerCtrl_Set_volume");

    if(PAudio != NULL)
    {
        if(volume/3 > 32)
            volume = 32;
        else
            volume = volume/3;
       // AudioSetVolume(volume);
        //AudioDev_SetVol(PAudio, volume);
        AudioControlTask_SendCmd(AUDIO_CMD_VOLUMESET, (void*)volume, SYNC_MODE);
    }
    return 0;
}

int MplayerCtrl_Get_volume(void)
{
    AUDIO_INFO mplayer;
    int volume = 0;

    AudioPlayer_GetAudioInfo(&mplayer);

    volume = mplayer.PlayVolume*3;

    return volume;
}

int Mplayer_Init(void)
{
    memset(mplayer_ctrl.name, 0x00, 128);
    mplayer_ctrl.init = MplayerCtrl_Init;
    mplayer_ctrl.deinit = MplayerCtrl_Deinit;
    mplayer_ctrl.play =  MplayerCtrl_Play;
    mplayer_ctrl.stop = MplayerCtrl_Stop;
    mplayer_ctrl.pause = MplayerCtrl_Pause;
    mplayer_ctrl.seek =MplayerCtrl_Seek;
    mplayer_ctrl.resume = MplayerCtrl_Resume;
    mplayer_ctrl.current_time = MplayerCtrl_Current_time;
    mplayer_ctrl.get_duration = MplayerCtrl_Get_duration;
    mplayer_ctrl.get_state = MplayerCtrl_Get_state;
    mplayer_ctrl.set_state = MplayerCtrl_Set_state;
    mplayer_ctrl.state_listener = MplayerCtrl_State_listener;
    mplayer_ctrl.set_volume = MplayerCtrl_Set_volume;
    mplayer_ctrl.get_volume = MplayerCtrl_Get_volume;
}


/*******************************************************************************
** Name: DLNATask_Resume
** Input:void
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
rk_err_t MPLAYERTask_Resume(void)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DLNATask_Suspend
** Input:void
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
rk_err_t MPLAYERTask_Suspend(void)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DLNATask_Enter
** Input:void
** Return: void
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
//_WEB_DLNA_DLNA_COMMON_
void MPLAYERTask_Enter(void)
{
    MPLAYER_ASK_QUEUE Play_Ask;
    MPLAYER_RESP_QUEUE Play_Resp;
    int ret = 0;


    while(1)
    {
        rk_printf("**MPLAYERTask_Enter** remain = %d", rkos_GetFreeHeapSize());
        rkos_queue_receive(gpstMPLAYERData->MPLAYERAskQueue, &Play_Ask, MAX_DELAY);
        switch(Play_Ask.cmd)
        {
            case MPLAYER_STATE_PLAY:
                //printf(" ***MplayerCtrl_Playing start remain = %d \n", rkos_GetFreeHeapSize());
                ret = MplayerCtrl_Playing(Play_Ask.url_buf);
                //printf(" ***MplayerCtrl_Playing end remain = %d\n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_PLAY;
                break;
            case MPLAYER_STATE_STOP:
                //printf(" ***MplayerCtrl_Stoping start remain = %d \n",  rkos_GetFreeHeapSize());
                ret = MplayerCtrl_Stoping();
                //printf(" ***MplayerCtrl_Stoping  end remain = %d\n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_STOP;
                break;
            case MPLAYER_STATE_SEEK:
                //printf(" ***MplayerCtrl_Seeking start remain = %d \n", rkos_GetFreeHeapSize());
                ret = MplayerCtrl_Seeking(Play_Ask.url_buf, Play_Ask.url_ms);
                //printf(" ***MplayerCtrl_Seeking end = %d\n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_SEEK;
                break;
            case MPLAYER_STATE_PAUSE:
               // printf(" ***MplayerCtrl_Pausing start remain = %d  \n", rkos_GetFreeHeapSize());
                ret = MplayerCtrl_Pausing();
                //printf(" ***MplayerCtrl_Pausing end  remain = %d  \n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_PAUSE;
                break;
            case MPLAYER_STATE_RESUME:
               // printf(" ***MplayerCtrl_Resuming start remain = %d  \n", rkos_GetFreeHeapSize());
                ret = MplayerCtrl_Resuming();
                //printf(" ***MplayerCtrl_Resuming end remain = %d  \n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_RESUME;
                break;
            case MPLAYER_STATE_TRANSTRING:
                //printf(" ***RK_MplayerCtrl_transtring start remain = %d  \n", rkos_GetFreeHeapSize());
                ret = RK_MplayerCtrl_transtring(Play_Ask.state, Play_Ask.url_buf);
                //printf(" ***RK_MplayerCtrl_transtring end remain = %d  \n", rkos_GetFreeHeapSize());
                //Play_Resp.cmd = MPLAYER_STATE_TRANSTRING;
                //rkos_queue_send(gpstMPLAYERData->MPLAYERRespQueue, &Play_Resp, MAX_DELAY);
                break;
            case DLNA_STOP:
                dlna_stop();
                break;
            default:
                break;
        }
#if DLNA_AYNC

#else
        Play_Resp.status = ret;
        rkos_queue_send(gpstMPLAYERData->MPLAYERRespQueue, &Play_Resp, MAX_DELAY);
#endif
    }
}


/*******************************************************************************
** Name: DLNATask_DeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
rk_err_t MPLAYERTask_DeInit(void *pvParameters)
{

    rkos_queue_delete(gpstMPLAYERData->MPLAYERAskQueue);
    rkos_queue_delete(gpstMPLAYERData->MPLAYERRespQueue);

#if DLNA_AYNC
    //rkos_semaphore_delete(gpstMPLAYERData->DLNAPLAYER_PlaySem);
    //rkos_semaphore_delete(gpstMPLAYERData->DLNAPLAYER_StopSem);
#endif

    rkos_memory_free(gpstMPLAYERData);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DLNATask_Init
** Input:void *pvParameters, void *arg
** Return: rk_err_t
** Owner:linyb
** Date: 2015.7.21
** Time: 18:11:12
*******************************************************************************/
rk_err_t MPLAYERTask_Init(void *pvParameters, void *arg)
{
    MPLAYER_TASK_DATA_BLOCK *mPLAYERDATE;


    mPLAYERDATE = rkos_memory_malloc(sizeof(MPLAYER_TASK_DATA_BLOCK));
    if(NULL == mPLAYERDATE)
        return RK_ERROR;

    mPLAYERDATE->MPLAYERAskQueue = rkos_queue_create(6, sizeof(MPLAYER_ASK_QUEUE));
    mPLAYERDATE->MPLAYERRespQueue = rkos_queue_create(6, sizeof(MPLAYER_RESP_QUEUE));

#if DLNA_AYNC
   // mPLAYERDATE->DLNAPLAYER_PlaySem = rkos_semaphore_create(1, 0);
   // mPLAYERDATE->DLNAPLAYER_StopSem = rkos_semaphore_create(1, 0);
#endif

    gpstMPLAYERData = mPLAYERDATE;

    return RK_SUCCESS;
}

#endif


