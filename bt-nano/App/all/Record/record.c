/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: App\Record\record.c
* Owner: chad.ma
* Date: 2016.1.4
* Time: 19:11:39
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    chad.ma     2016.1.4     19:11:39   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __APP_RECORD_RECORD_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "record.h"
#include "record_globals.h"
#include "RecordControlTask.h"


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
rk_err_t record_shell_proecess(HDC dev, uint8* pstr);
rk_err_t record_shell_create(HDC dev,uint8* pstr);
rk_err_t record_shell_delete(HDC dev,uint8* pstr);
rk_err_t record_shell_getinfo(HDC dev,uint8* pstr);
rk_err_t record_shell_resume(HDC dev,uint8* pstr);
rk_err_t record_shell_pause(HDC dev,uint8* pstr);
rk_err_t record_shell_stop(HDC dev,uint8* pstr);
rk_err_t record_shell_start(HDC dev,uint8 * pstr);
rk_err_t record_shell_prepare(HDC dev,uint8 * pstr);



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
_APP_RECORD_RECORD_SHELL_
static SHELL_CMD ShellRecordName[] =
{
    "create",record_shell_create,"create a recorder","record.create <wav | mp3>",
    "prepare",record_shell_prepare,"recorder prepare","record.prepare",
    "start",record_shell_start,"recorder start","record.start",
    "stop",record_shell_stop,"recorder stop","record.stop",
    "pause",record_shell_pause,"recorder pause","record.pause",
    "resume",record_shell_resume,"recorder resume","record.resume",
    "getinfo",record_shell_getinfo,"get recorder information","record.getinfo",
    "delete",record_shell_delete,"delete a recorder","recorder.delete",
    "encode",record_shell_proecess,"start recorder encode","recorder.encode",
    "\b",NULL,"NULL","NULL",                // the end
};


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: record_shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:13:46
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL API rk_err_t record_shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellRecordName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (Space != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellRecordName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellRecordName[i].CmdDes, pItem);
    if(ShellRecordName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellRecordName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: record_shell_proecess
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.9
** Time: 16:17:46
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_proecess(HDC dev, uint8* pstr)
{
    uint32 msg;
    uint8 pcm[1024];

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordPcmInput(pcm, 256);
    rk_printf("record_shell_proecess will return\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: record_shell_create
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.5
** Time: 11:49:19
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_create(HDC dev, uint8* pstr)
{
    RK_TASK_RECORDCONTROL_ARG Arg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    if(pstr[0] == ' ')
    {
         pstr++;
        if(StrCmpA(pstr, "wav", 3) == 0)
        {
            Arg.RecordEncodeType = RECORD_ENCODE_TYPE_WAV;
        }

        if(StrCmpA(pstr, "mp3", 3) == 0)
        {
            Arg.RecordEncodeType = RECORD_ENCODE_TYPE_XXX;
        }
    }


    Arg.RecordType          = RECORD_TYPE_MIC_STERO;
    Arg.RecordEncodeSubType   = 0;
    Arg.RecordSampleRate    = RECORD_SAMPLE_FREQUENCY_44_1KHZ;
    Arg.RecordChannel       = RECORD_CHANNEL_STERO;
    Arg.RecordDataWidth     = RECORD_DATAWIDTH_16BIT;
    Arg.pfRecordState = NULL;
    Arg.RecordSource        = 0;

    if(RKTaskCreate(TASK_ID_RECORDCONTROL, 0, &Arg, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("Record control task create failure");
         return RK_SUCCESS;
    }

    rk_printf("record_shell_create will return\n");

    return RK_SUCCESS;;
}
/*******************************************************************************
** Name: record_shell_delete
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:31:12
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_delete(HDC dev, uint8* pstr)
{
    uint32 msg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: record_shell_getinfo
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:26:00
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_getinfo(HDC dev, uint8* pstr)
{
    uint32 msg;
    msg = 1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordControlTask_SendCmd(RECORD_CMD_GETINFO,&msg, SYNC_MODE);
    rk_printf("record_shell_getinfo will return\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: record_shell_resume
** Input:uint8* pstr
** Return: rk_er_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:25:25
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_resume(HDC dev, uint8* pstr)
{
    uint32 msg;
    msg = 1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordControlTask_SendCmd(RECORD_CMD_RESUME,&msg, SYNC_MODE);
    rk_printf("record_shell_resume will return\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: record_shell_pause
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:24:09
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_pause(HDC dev, uint8* pstr)
{
    uint32 msg;
    msg = 1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordControlTask_SendCmd(RECORD_CMD_PAUSE,&msg, SYNC_MODE);
    rk_printf("record_shell_pause will return\n");

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: record_shell_stop
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:23:12
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_stop(HDC dev, uint8* pstr)
{
    uint32 msg;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordControlTask_SendCmd(RECORD_CMD_STOP,(void *)1, SYNC_MODE);

    rk_printf("record_shell_stop will return\n");
    return RK_SUCCESS;;
}
/*******************************************************************************
** Name: record_shell_start
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:20:46
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_start(HDC dev, uint8 * pstr)
{
    uint32 msg;
    msg = 1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    RecordControlTask_SendCmd(RECORD_CMD_START,&msg, SYNC_MODE);

    return RK_SUCCESS;;
}

/*******************************************************************************
** Name: record_shell_prepare
** Input:uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:19:00
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_prepare(HDC dev, uint8 * pstr)
{
    uint32 msg;
    msg = 1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    RecordControlTask_SendCmd(RECORD_CMD_PREPARE,&msg, SYNC_MODE);

    return RK_SUCCESS;;
}


#endif
