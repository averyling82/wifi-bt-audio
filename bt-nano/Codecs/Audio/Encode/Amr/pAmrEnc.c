/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Codecs\Audio\Decode\Amr\pAmr.c
* Owner: cjh
* Date: 2016.5.26
* Time: 14:33:43
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.5.26     14:33:43   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "../include/audio_main.h"
#include "../include/audio_globals.h"
#include "../include/audio_file_access.h"
#include  "Record_globals.h"

#ifdef AMR_ENC_INCLUDE

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#pragma arm section code = "EncodeAmrCode", rodata = "EncodeAmrCode", rwdata = "EncodeAmrData", zidata = "EncodeAmrBss"

extern RecordBlock  gRecordBlock;
extern pSemaphore   osRecordEncodeOk;

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


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: AmrFunction
** Input:unsigned long ulIoctl, unsigned long ulParam1,unsigned long ulParam2,unsigned long ulParam3,unsigned long ulParam4
** Return: unsigned long
** Owner:cjh
** Date: 2016.5.26
** Time: 14:50:02
*******************************************************************************/
//_ATTR_AMRENC_TEXT_
API unsigned long AMREncFunction(unsigned long ulIoctl, unsigned long ulParam1,unsigned long ulParam2,unsigned long ulParam3)
{
    #ifdef A_CORE_DECODE
    #else

    switch (ulIoctl)
    {
        case SUBFN_CODEC_OPEN_ENC:
        {
            MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE_OPEN, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            MailBoxWriteA2BData(ulParam1, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
            rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
            if (gRecordBlock.encodeErr == 1)
            {
                return 0;
            }

            *(short**)ulParam2 = (short *)gRecordBlock.unenc_bufptr;
            *(long*)ulParam3 = gRecordBlock.unenc_length;
            return (1);
        }

        case SUBFN_CODEC_ENC_GETBUFFER:
        {
        retry:

            IntDisable(INT_ID_MAILBOX1);
            if(gRecordBlock.encodeOver == 1)
            {
                 //rk_printf("get enc buf");

                if (gRecordBlock.encodeErr == 1)
                {
                    IntEnable(INT_ID_MAILBOX1);
                    return 0;
                }

                gRecordBlock.encodeOver = 0;
                MailBoxWriteA2BCmd(MEDIA_MSGBOX_CMD_ENCODE, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                MailBoxWriteA2BData(ulParam2, MAILBOX_ID_0, MAILBOX_CHANNEL_1);
                IntEnable(INT_ID_MAILBOX1);
                rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);

                *(short**)ulParam1 = (short *)gRecordBlock.unenc_bufptr;
                *(long*)ulParam2 = gRecordBlock.unenc_length;
            }
            else
            {
                rk_printf("wait b core...");
                gRecordBlock.needencode = 1;
                IntEnable(INT_ID_MAILBOX1);
                rkos_semaphore_take(osRecordEncodeOk, MAX_DELAY);
                goto retry;
            }
            return(1);
        }

        case SUBFN_CODEC_ENCODE:
        {
            return (1);
        }

        case SUBFN_CODEC_GETBITRATE:
        {
             *(int *)ulParam1 = gRecordBlock.Bitrate;
             return(1);
        }

        default:
        {
            return 0;
        }
    }
    #endif
}

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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
