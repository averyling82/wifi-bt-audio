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
rk_err_t record_shell_proecess(uint8* pstr);
rk_err_t record_shell_create(uint8* pstr);
rk_err_t record_shell_delete(uint8* pstr);
rk_err_t record_shell_help();
rk_err_t record_shell_getinfo(uint8* pstr);
rk_err_t record_shell_resume(uint8* pstr);
rk_err_t record_shell_pause(uint8* pstr);
rk_err_t record_shell_stop(uint8* pstr);
rk_err_t record_shell_start(uint8 * pstr);
rk_err_t record_shell_prepare(uint8 * pstr);



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
    "create",NULL,"NULL","NULL",
    "prepare",NULL,"NULL","NULL",
    "start",NULL,"NULL","NULL",
    "stop",NULL,"NULL","NULL",
    "pause",NULL,"NULL","NULL",
    "resume",NULL,"NULL","NULL",
    "getinfo",NULL,"NULL","NULL",
    "delete",NULL,"NULL","NULL",
    "encode",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
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
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        printf("\n StrCnt = 0 \n");
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellRecordName, pItem, StrCnt);
    if (ret < 0)
    {
        printf("\n ret < 0 \n");
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;         //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            ret = record_shell_create(pItem);
            break;
        case 0x01:
            ret = record_shell_prepare(pItem);
            break;

        case 0x02:
            ret = record_shell_start(pItem);
            break;

        case 0x03:
            ret = record_shell_stop(pItem);
            break;

        case 0x04:
            ret = record_shell_pause(pItem);
            break;

        case 0x05:
            ret = record_shell_resume(pItem);
            break;

        case 0x06:
            ret = record_shell_getinfo(pItem);
            break;

        case 0x07:
            ret = record_shell_delete(pItem);
            break;

        case 0x08:
            ret = record_shell_proecess(pItem);
            break;

        case 0x09:
            ret = record_shell_help();
            break;

        default:
            ret = RK_ERROR;
            break;
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
SHELL FUN rk_err_t record_shell_proecess(uint8* pstr)
{
    uint32 msg;
    uint8 pcm[1024];

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.process : process pcm data,send to B core encode and write file\r\n");
            return RK_SUCCESS;
        }
    }
#endif
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
SHELL FUN rk_err_t record_shell_create(uint8* pstr)
{
    RK_TASK_RECORDCONTROL_ARG Arg;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 3) == 0)
        {
            rk_print_string("music.open : open music    \r\n");
            return RK_SUCCESS;
        }
    }
    else if(pstr[0] == ' ')
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
#endif

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
SHELL FUN rk_err_t record_shell_delete(uint8* pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("record.delete : delete record task    \r\n");
            return RK_SUCCESS;
        }
    }
#endif

    RKTaskDelete(TASK_ID_RECORDCONTROL, 0, SYNC_MODE);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: record_shell_help
** Input:uint8* pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2016.1.4
** Time: 19:26:23
*******************************************************************************/
_APP_RECORD_RECORD_SHELL_
SHELL FUN rk_err_t record_shell_help()
{
    printf("record命令集提供了一系列的命令对record进行操作\r\n");
    printf("record包含的子命令如下:            \r\n");

    printf("record.create      : 创建录音线程\r\n");
    printf("record.prepare     : 录音准备\r\n");
    printf("record.start       : 开始录音\r\n");
    printf("record.stop        : 录音\r\n");
    printf("record.pause       : 录音暂停\r\n");
    printf("record.resume      : 录音恢复\r\n");
    printf("record.getinfo     : 获取录音信息\r\n");
    printf("record.encode      : 将获取到的录音数据处理后编码再写文件\r\n");
    printf("record.delete      : 删除录音线程\r\n");
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
SHELL FUN rk_err_t record_shell_getinfo(uint8* pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.getinfo : pause record \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    msg = 1;
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
SHELL FUN rk_err_t record_shell_resume(uint8* pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.resume : pause record \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    msg = 1;
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
SHELL FUN rk_err_t record_shell_pause(uint8* pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.pause : pause record \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    msg = 1;
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
SHELL FUN rk_err_t record_shell_stop(uint8* pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.stop : stop record \r\n");
            return RK_SUCCESS;
        }
    }
#endif

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
SHELL FUN rk_err_t record_shell_start(uint8 * pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.start : start record \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    msg = 1;
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
SHELL FUN rk_err_t record_shell_prepare(uint8 * pstr)
{
    uint32 msg;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            printf("record.prepare : prepare record \r\n");
            return RK_SUCCESS;
        }
    }
#endif
    msg = 1;
    RecordControlTask_SendCmd(RECORD_CMD_PREPARE,&msg, SYNC_MODE);

    return RK_SUCCESS;;
}


#endif
