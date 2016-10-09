/*
********************************************************************************
*
*     Copyright (c): , Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* File Name  cue.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               tiantian      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"
#ifdef _RK_ID3_

#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "Bsp.h"
#include "global.h"
#include "SysInfoSave.h"
#include "device.h"
#include "FileDevice.h"

#include "cue.h"
#include "id3.h"

#ifdef _RK_CUE_

unsigned char CUETempBuff[MAX_ID3_FIND_SIZE];

/*
*****************************************************
* Name:     CUE_GetComInfo
* Desc:     读取CUE信息
* Param:    字符缓冲
* Return:   CUE类型
* Global:
* Note:
* History:  <author>          <time>        <version>
*           Tiantian        2013-9-6
* Log:
*****************************************************
*/
int CUE_GetComInfo(char *tempbuf)
{
    int iCueComInfo = CUE_NONINFO;

    switch (tempbuf[0]) {
        case 'F':
            if (tempbuf[1]  == 'I' && tempbuf[2]  == 'L' && tempbuf[3]  == 'E') {
                iCueComInfo = CUE_FILE;
            }
            break;

        case 'T':
            switch (tempbuf[1]){
                case 'R':
                    if (tempbuf[2]  == 'A' && tempbuf[3]  == 'C' && tempbuf[4]  == 'K'){
                        iCueComInfo = CUE_TRACK;
                    }
                    break;

                case 'I':
                    if (tempbuf[2]  == 'T' && tempbuf[3]  == 'L' && tempbuf[4]  == 'E'){
                        iCueComInfo = CUE_TITLE;
                    }
                    break;
            }
            break;

        case 'P':
            if (tempbuf[1]  == 'E' && tempbuf[2]  == 'R' && tempbuf[3]  == 'F'
                && tempbuf[4]  == 'O' && tempbuf[5]  == 'R' && tempbuf[6]  == 'M' ) {
                    iCueComInfo = CUE_PERFORMER;
            }
            break;

        case 'I':
            if (tempbuf[1]  == 'N' && tempbuf[2]  == 'D' && tempbuf[3]  == 'E' && tempbuf[4]  == 'X'
                && tempbuf[5]  == ' ' && tempbuf[6]  == '0' && tempbuf[7]  == '1') {
                    iCueComInfo = CUE_INDEX_01;
            }

            if (tempbuf[1]  == 'N' && tempbuf[2]  == 'D' && tempbuf[3]  == 'E' && tempbuf[4]  == 'X'
                && tempbuf[5]  == ' ' && tempbuf[6]  == '0' && tempbuf[7]  == '0') {
                    iCueComInfo = CUE_INDEX_00;
            }
            break;
    }

    return iCueComInfo;
}

/*
*****************************************************
* Name:     CUE_GetMetaData
* Desc:     读取CUE信息
* Param:    字符缓冲，剩余长度，CUE类型，目标缓冲
* Return:
* Global:
* Note:
* History:  <author>          <time>        <version>
*           Tiantian         2013-9-6
* Log:
*****************************************************
*/
int CUE_GetMetaData(unsigned char *tempbuf, int remainSize, int iCueComInfo, unsigned char *cTargetBuf, uint8 EncodeMode)
{
    int i;
    int iStringSize = 0;

    if (remainSize <= 0)
        return 0;

    for (i = 0; i < remainSize; i++) {
        if(tempbuf[i] == '\r' && tempbuf[i + 1] == '\n') {
            if (iCueComInfo == CUE_INDEX_00 || iCueComInfo == CUE_INDEX_01) {
                iStringSize = i;
                memcpy(cTargetBuf, tempbuf, iStringSize);
                cTargetBuf[iStringSize] = '\0';

                return iStringSize;
            }
            else if (iCueComInfo == CUE_FILE || iCueComInfo == CUE_TRACK)
                iStringSize = i - 6; // " WAVE ||  AUDIO
            else
                iStringSize = i - 1; // "

            break;
        }
    }
#if 0
{
    int t;
    switch(iCueComInfo) {
        case CUE_FILE:
            printf("FILE\n");
            break;
        case CUE_TRACK:
            printf("TRACK\n");
            break;
        case CUE_TITLE:
            printf("TITLE\n");
            break;
        case CUE_PERFORMER:
            printf("PERFORMER\n");
            break;
        case CUE_INDEX_00:
            printf("INDEX_00\n");
            break;
        case CUE_INDEX_01:
            printf("INDEX_01\n");
            break;
    }

    for(t = 0; t < iStringSize; t++) {
        printf("%x ", tempbuf[t]);
    }
    printf("\n\n");
}
#endif
    Mp3Id3ToUnicode((UINT16*)cTargetBuf, tempbuf, iStringSize, EncodeMode);

#if 0
{
    int t;
    for(t = 0; t < 25; t++) {
        printf("%x ", cTargetBuf[t]);
    }
    printf("\n\n\n\n");
}
#endif
    return iStringSize;
}

/*
*****************************************************
* Name:     parserCueInfo
* Desc:     解析CUE信息
* Param:
* Return:
* Global:
* Note:
* History:  <author>          <time>        <version>
*           Tiantian         2013-9-6
* Log:
*****************************************************
*/
int ParseCueInfo(HDC fHandle, CUE_INFO * cue_info, uint8 OnlyFile)
{
    int i, j;
    int iCueComInfo;
    int iFileSize;
    int iReadSize;
    int iCopySize;

    int iCurrentOffset = 0;
    int iRemainSize = 0;
    int curRemainSize = 0;

    int iMetaCnt = -1;
    unsigned char timebuf[10];
    UINT8  EncodeMode = ID3_ENCODE_ISO88591;

    FileDev_GetFileSize(fHandle, &iFileSize);
    iRemainSize = iFileSize;
    iCurrentOffset = 0;

    //memset(cue_info, 0, sizeof(CUE_INFO));
    //////////////////////////////////////////////////////////////////////
    iReadSize = ID3_UpdateBufferData(fHandle, CUETempBuff, iRemainSize, 0);
    if(iReadSize < 0) {
        printf("CUE Can't read content\n");
        return 0;
    }

    if((CUETempBuff[0] == 0xFF) && (CUETempBuff[1] == 0xFE))
    {
        EncodeMode = ID3_ENCODE_UTF16;
    }
    else if((CUETempBuff[0] == 0xFE) && (CUETempBuff[1] == 0xFF))
    {
        EncodeMode = ID3_ENCODE_UTF16BE;
    }
    else if((CUETempBuff[0] == 0xEF) && (CUETempBuff[1] == 0xBB) && (CUETempBuff[2] == 0xBF))
    {
        EncodeMode = ID3_ENCODE_UTF8;
    }

    iCurrentOffset = iReadSize;
    iRemainSize    -= iReadSize;
    curRemainSize = iReadSize;

    for (i = 0, j = 0; i < iFileSize /*iReadSize*/; ) {
        iCueComInfo = CUE_GetComInfo(&CUETempBuff[j]);

        //FILE "xxxxx.xxx" WAVE, WAVE indicates that the file is an audio file.
        if (iCueComInfo == CUE_FILE) {
            iCopySize = CUE_GetMetaData(&CUETempBuff[j + 6], iReadSize - j - 6, iCueComInfo, (unsigned char *)cue_info->file, EncodeMode);

            if (iCopySize != 0) {
                j += (6 + iCopySize + 6);
                i += (6 + iCopySize + 6);
            }

            if(OnlyFile)
            {
                return 1;
            }
        }

        if (iCueComInfo == CUE_TRACK) { //一直找不到TRACE标签，错误的文件
            iMetaCnt = 1;
TRACK:
            if (j + 14 > iReadSize) {
                goto READDATA;
            } else {
                cue_info->m_total_Song++;
                iCopySize = CUE_GetMetaData(&CUETempBuff[j + 6], iReadSize - j - 6, iCueComInfo,
                                    (unsigned char *)cue_info->m_songs[cue_info->m_total_Song - 1].trackId, EncodeMode);
                if (iCopySize != 0) {
                    j += (6 + iCopySize + 6);    //跳过TRACK xx AUDIO
                    i += (6 + iCopySize + 6);
                }
            }

            for (; j < iReadSize; ) {
                iCueComInfo = CUE_GetComInfo(&CUETempBuff[j]);
                if (iCueComInfo != CUE_NONINFO) {
                    switch(iCueComInfo) {
                        //TRACK xx AUDIO
                        case CUE_TRACK:
                            goto TRACK;
                            break;

                        case CUE_PERFORMER:
                            //PERFORMER "xxx"
                            iCopySize = CUE_GetMetaData(&CUETempBuff[j + 11], iReadSize - j - 11, iCueComInfo,
                                                            (unsigned char *)cue_info->m_songs[cue_info->m_total_Song - 1].performer, EncodeMode);
                            if (iCopySize != 0) {
                                j += (11 + 1);
                                i += (11 + 1);
                            }

                            break;

                        case CUE_TITLE:
                            //TITLE "xxx"
                            iCopySize = CUE_GetMetaData(&CUETempBuff[j + 7], iReadSize - j - 7, iCueComInfo,
                                                            (unsigned char *)cue_info->m_songs[cue_info->m_total_Song - 1].title, EncodeMode);
                            if (iCopySize != 0) {
                                j += (7 + 1);
                                i += (7 + 1);
                            }

                            break;

                        case CUE_INDEX_01:
                        {
                            int min, sec, frame;
                            //INDEX 01 min:sec:frame(0~74)     75 frame == 1 sec
                            iCopySize = CUE_GetMetaData(&CUETempBuff[j + 9], iReadSize - j - 9, iCueComInfo, timebuf, EncodeMode);

                            if (iCopySize != 0) {
                                j += 9;
                                i += 9;

                                min = (timebuf[0] - 48) * 10 + (timebuf[1] - 48);   //48 = 0x30 即'0'
                                sec = (timebuf[3] - 48) * 10 + (timebuf[4] - 48);

                                frame = (timebuf[6] - 48) * 10 + (timebuf[7] - 48);
                                if(frame > 74)
                                    frame = 74;


                                cue_info->m_songs[cue_info->m_total_Song - 1].start = min * 60 * 1000 + sec * 1000 + (frame* 1000)/74;

                                if (cue_info->m_songs[cue_info->m_total_Song - 1 - 1].end == 0)
                                    cue_info->m_songs[cue_info->m_total_Song - 1 - 1].end = cue_info->m_songs[cue_info->m_total_Song - 1].start;

                            }
                            break;
                         }

                        case CUE_INDEX_00:
                        {
                            long min, sec, frame;
                            //INDEX 00 min:sec:frame(0~74)     75 frame == 1 sec
                            iCopySize = CUE_GetMetaData(&CUETempBuff[j + 9], iReadSize - j - 9, iCueComInfo, timebuf, EncodeMode);

                            if (iCopySize != 0) {
                                j += 9;
                                i += 9;

                                min = (timebuf[0] - 48) * 10 + (timebuf[1] - 48);
                                sec = (timebuf[3] - 48) * 10 + (timebuf[4] - 48);

                                frame = (timebuf[6] - 48) * 10 + (timebuf[7] - 48);
                                if(frame > 74)
                                    frame = 74;

                                if (cue_info->m_total_Song > 1)
                                    cue_info->m_songs[cue_info->m_total_Song - 1 - 1].end = min * 60 * 1000 + sec * 1000 + (frame* 1000)/74;
                            }
                            break;
                         }
                    }

                    j += iCopySize;
                    i += iCopySize;
                    curRemainSize = iReadSize - j;

                    if (iCopySize == 0 || curRemainSize <= 0)
                        goto READDATA;
                } else {
                    j++;
                    i++;
                    curRemainSize = iReadSize - j;
                }

                if (curRemainSize < 10) {
READDATA:
                    iCurrentOffset = i;
                    iRemainSize    = iFileSize - i;
                    //iCurrentOffset -= curRemainSize;
                    //iRemainSize    += curRemainSize;
                    iReadSize = ID3_UpdateBufferData(fHandle, CUETempBuff, iRemainSize, iCurrentOffset);
                    //if(iReadSize < 0) {
                    //    printf("Can't read content\n");
                    //    return 0;
                    //}
                    iCurrentOffset += iReadSize;
                    iRemainSize    -= iReadSize;
                    curRemainSize   = iReadSize;
                    j = 0;
                }
            }
        } else {
            j++;
            i++;
        }
    }

    //printf("cue_info->m_total_Song == %d\n", cue_info->m_total_Song);
    return iMetaCnt;
}

#endif
#endif
