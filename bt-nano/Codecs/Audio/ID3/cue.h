#ifndef _CUE_H_
#define _CUE_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "AddrSaveMacro.h"

#define MAX_CUE_FIELD MEDIA_ID3_SAVE_CHAR_NUM
#define MAX_SONGS_NUM 128

enum CUEComInfo
{
    CUE_FILE,
    CUE_TRACK,
    CUE_PERFORMER,
    CUE_TITLE,
    CUE_INDEX_01,
    CUE_INDEX_00,
    CUE_NONINFO
};

typedef __packed struct _SONG_T
{
    uint32 start;  //seconds
    uint32 end;    //seconds
    uint16 performer[MAX_CUE_FIELD];
    uint16 title[MAX_CUE_FIELD];
    uint16 trackId[4];
} song_t;

typedef __packed struct _CUE_INFO
{
    long m_total_Song;
    uint16 file[MAX_CUE_FIELD];  //Ó³ÉäÎÄ¼þ
    song_t m_songs[MAX_SONGS_NUM];
} CUE_INFO;

int CUE_GetComInfo(char *tempbuf);

int CUE_GetMetaData(unsigned char *tempbuf, int remainSize, int iCueComInfo, unsigned char *cTargetBuf, uint8 EncodeMode);

int ParseCueInfo(HDC hFile, CUE_INFO * cue_info, uint8 OnlyFile);

#endif
