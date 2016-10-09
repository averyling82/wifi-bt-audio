/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: rk-iot\App\Audio\music.c
* Owner: cjh
* Date: 2015.7.17
* Time: 16:38:34
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2015.7.17     16:38:34   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_AUDIO_MUSIC_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "DeviceInclude.h"
#include "music.h"
#include "effect.h"
#include "BSP.h"
#include "http.h"
#include "wiced_management.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/


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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t music_shell_shuffle_test(HDC dev, uint8 * pstr);
rk_err_t music_shell_set_rp(HDC dev, uint8 * pstr);
rk_err_t music_shell_set_sf(HDC dev, uint8 * pstr);
rk_err_t music_shell_ff_pause(HDC dev, uint8 * pstr);
rk_err_t music_shell_ff_resume(HDC dev, uint8 * pstr);
rk_err_t music_shell_mute(HDC dev, uint8 * pstr);
rk_err_t music_shell_unmute(HDC dev, uint8 * pstr);
rk_err_t music_shell_pause(HDC dev, uint8 * pstr);
rk_err_t music_shell_play(HDC dev, uint8 * pstr);
rk_err_t music_shell_stop(HDC dev, uint8 * pstr);
rk_err_t music_shell_prev(HDC dev, uint8 * pstr);
rk_err_t music_shell_next(HDC dev, uint8 * pstr);
rk_err_t music_shell_ffw(HDC dev, uint8 * pstr);
rk_err_t music_shell_ffw_stop(HDC dev, uint8 * pstr);
rk_err_t music_shell_ffd(HDC dev, uint8 * pstr);
rk_err_t music_shell_ffd_stop(HDC dev, uint8 * pstr);
rk_err_t music_shell_set_eq(HDC dev, uint8 * pstr);
rk_err_t music_shell_set_vol(HDC dev, uint8 * pstr);
rk_err_t music_shell_create(HDC dev, uint8 * pstr);
rk_err_t music_shell_del(HDC dev, uint8 * pstr);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
_APP_AUDIO_MUSIC_SHELL_
static SHELL_CMD ShellMusicName[] =
{
    "play",music_shell_play,"NULL","NULL",
    "stop",music_shell_stop,"NULL","NULL",
    "prev",music_shell_prev,"NULL","NULL",
    "next",music_shell_next,"NULL","NULL",
    "ffw",music_shell_ffw,"NULL","NULL",
    "ffws",music_shell_ffw_stop,"NULL","NULL",
    "ffd",music_shell_ffd,"NULL","NULL",
    "ffds",music_shell_ffd_stop,"NULL","NULL",
    "eq",music_shell_set_eq,"NULL","NULL",
    "sf",music_shell_set_sf,"NULL","NULL",
    "rp",music_shell_set_rp,"NULL","NULL",
    "vol",music_shell_set_vol,"NULL","NULL",
    "create",music_shell_create,"NULL","NULL",
    "pause",music_shell_pause,"NULL","NULL",
    "ffr",music_shell_ff_resume,"NULL","NULL",
    "ffp",music_shell_ff_pause,"NULL","NULL",
    "delete",music_shell_del,"NULL","NULL",
    "shuf",music_shell_shuffle_test,"test shuffle", "music.shuf\n",
    "\b",NULL,"NULL","NULL",               // the end
};

/*******************************************************************************
** Name: music_shell
** Input:(HDC dev,  uint8 * pstr)
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL API rk_err_t music_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellMusicName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellMusicName[i].CmdDes, pItem);
    if(ShellMusicName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellMusicName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: music_shell_shuffle_test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.22
** Time: 11:30:40
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_shuffle_test(HDC dev, uint8 * pstr)
{
    uint32 i, j , seed;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    return RK_SUCCESS;


}
/*******************************************************************************
** Name: music_shell_set_rp
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.21
** Time: 19:47:02
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_set_rp(HDC dev, uint8 * pstr)
{

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if (StrCmpA(pstr, "/d", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUIDO_FOLDER_REPEAT, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/o", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUDIO_REPEAT, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/e", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_REPEATSET, (void *)AUDIO_FOLDER_ONCE, SYNC_MODE);
        return RK_SUCCESS;
    }

    return RK_ERROR;


}

/*******************************************************************************
** Name: music_shell_set_sf
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.21
** Time: 19:45:32
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_set_sf(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if (StrCmpA(pstr, "/on", 3) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_SHUFFLESET, (void *)AUDIO_RAND, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/off", 4) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_SHUFFLESET, (void *)AUDIO_INTURN, SYNC_MODE);
        return RK_SUCCESS;
    }
    return RK_ERROR;

}

/*******************************************************************************
** Name: music_shell_ff_pause
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.6
** Time: 9:10:33
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_ff_pause(HDC dev, uint8 * pstr)
{

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    AudioControlTask_SendCmd(AUDIO_CMD_FF_PAUSE, NULL, SYNC_MODE);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_ff_resume
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.23
** Time: 14:52:44
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_ff_resume(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    AudioControlTask_SendCmd(AUDIO_CMD_FF_RESUME, NULL, SYNC_MODE);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_pause
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.23
** Time: 14:51:20
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL FUN rk_err_t music_shell_pause(HDC dev, uint8 * pstr)
{
    uint32 msg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }    //AudioPause();


    msg = 1;
    AudioControlTask_SendCmd(AUDIO_CMD_PAUSE, &msg, SYNC_MODE);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_play
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_play(HDC dev, uint8 * pstr)
{
    uint32 msg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    msg = 1;
    AudioControlTask_SendCmd(AUDIO_CMD_RESUME, &msg, SYNC_MODE);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_stop
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_stop(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }    AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_prev
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_prev(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    AudioControlTask_SendCmd(AUDIO_CMD_PREVFILE, NULL, SYNC_MODE);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_next
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_next(HDC dev, uint8 * pstr)
{
    uint32 msg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    msg = 1;
    AudioControlTask_SendCmd(AUDIO_CMD_NEXTFILE, &msg, SYNC_MODE);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_ffw
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_ffw(HDC dev, uint8 * pstr)
{
    uint32 msg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    msg = 2000;
    AudioControlTask_SendCmd(AUDIO_CMD_FFW, (void *)msg, SYNC_MODE);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_ffw_stop
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_ffw_stop(HDC dev, uint8 * pstr)
{
    //uint32 msg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    AudioControlTask_SendCmd(AUDIO_CMD_FF_STOP, NULL, SYNC_MODE);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_ffd
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_ffd(HDC dev, uint8 * pstr)
{
    uint32 msg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    msg = 2000;
    AudioControlTask_SendCmd(AUDIO_CMD_FFD, (void *)msg, SYNC_MODE);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_ffd_stop
** Input:(HDC dev,  uint8 * pstr)
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_ffd_stop(HDC dev, uint8 * pstr)
{
    //uint32 msg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    AudioControlTask_SendCmd(AUDIO_CMD_FF_STOP, NULL, SYNC_MODE);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_set_eq
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_set_eq(HDC dev, uint8 * pstr)
{
    //MUSIC_CLASS * pstMusic;
    AUDIO_DEV_ARG stParg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if (StrCmpA(pstr, "/h", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_HEAVY, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/p", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_POP, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/j", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_JAZZ, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/un", 3) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_UNIQUE, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/c", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_CLASS, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/b", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_BASS, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/r", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_ROCK, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/us", 4) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_USER, SYNC_MODE);
        return RK_SUCCESS;
    }
    else if(StrCmpA(pstr, "/n", 2) == 0)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_EQSET, (void *)EQ_NOR, SYNC_MODE);
        return RK_SUCCESS;
    }


    return RK_SUCCESS;
}


/*******************************************************************************
** Name: music_shell_set_vol
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_set_vol(HDC dev, uint8 * pstr)
{
    //MUSIC_CLASS * pstMusic;
    AUDIO_DEV_ARG stParg;
    uint8 vol;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_create
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_create(HDC dev, uint8 * pstr)
{
    RK_TASK_AUDIOCONTROL_ARG Arg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    Arg.ucSelPlayType = SOURCE_FROM_FILE_BROWSER;
    Arg.FileNum = 0;
    Arg.pfAudioState = NULL;

    Arg.filepath[Ascii2Unicode(pstr, Arg.filepath, strlen(pstr)) / 2 ] = 0;

    if(RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("Audio control task create failure");
         return RK_SUCCESS;
    }

    #if 0
    FW_LoadSegment(SEGMENT_ID_AUDIOCONTROL_INIT, SEGMENT_OVERLAY_ALL);
    AudioControlTask_Init(NULL, &Arg);
    FW_RemoveSegment(SEGMENT_ID_AUDIOCONTROL_INIT);

    FW_LoadSegment(SEGMENT_ID_STREAMCONTROL_INIT, SEGMENT_OVERLAY_ALL);
    StreamControlTask_Init(NULL, NULL);
    FW_RemoveSegment(SEGMENT_ID_STREAMCONTROL_INIT);

    RKTaskCreate2(AudioControlTask_Enter, NULL, NULL, "audio", 4096, 127, NULL);
    RKTaskCreate2(StreamControlTask_Enter, NULL, NULL, "stream", 4096, 100, NULL);
    #endif

    #if 0
    ret = HttpTask_Get("http://192.168.1.101/I%20love%20you.pcm");
    if(ret == RK_SUCCESS)
        printf("\nhttp ok\n");
    else
        printf("\nhttp error\n");

    RKTaskCreate(TASK_ID_MEDIA, 0, NULL, SYNC_MODE);
    #endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: music_shell_del
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.7.17
** Time: 16:48:43
*******************************************************************************/
_APP_AUDIO_MUSIC_SHELL_
SHELL COMMON rk_err_t music_shell_del(HDC dev, uint8 * pstr)
{
    uint32 msg;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RKTaskDelete(TASK_ID_AUDIOCONTROL, 0, SYNC_MODE);
    return RK_SUCCESS;
}
#endif
