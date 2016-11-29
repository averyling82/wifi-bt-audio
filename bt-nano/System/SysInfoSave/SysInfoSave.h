/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\ModuleOverlay\SysInfoSave.h
* Owner: aaron.sun
* Date: 2015.10.28
* Time: 17:23:35
* Version: 1.0
* Desc: sys info save
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.28     17:23:35   1.0
********************************************************************************************
*/


#ifndef __SYSTEM_MODULEOVERLAY_SYSINFOSAVE_H__
#define __SYSTEM_MODULEOVERLAY_SYSINFOSAVE_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _RK_EQ_
#include "effect.h"
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_  __attribute__((section("system_moduleoverlay_sysinfosave_common")))
#define _SYSTEM_MODULEOVERLAY_SYSINFOSAVE_INIT_  __attribute__((section("system_moduleoverlay_sysinfosave_init")))
#define _SYSTEM_MODULEOVERLAY_SYSINFOSAVE_SHELL_  __attribute__((section("system_moduleoverlay_sysinfosave_shell")))

/*reserved area data storage,bit definition logic address LBA*/
#define SYSDATA_BLK         0

#define BOOKMARK_BLK        1

#define WIFI_AP_PW_BLK      2

#define XML_BUF_BLK         3

#define MEDIA_BLK           4

#define BL_LEVEL_1          1
#define BL_LEVEL_2          2
#define BL_LEVEL_3          3
#define BL_LEVEL_4          4
#define BL_LEVEL_5          5

#define BL_LEVEL_1_VALUE    80
#define BL_LEVEL_2_VALUE    60
#define BL_LEVEL_3_VALUE    40
#define BL_LEVEL_4_VALUE    20
#define BL_LEVEL_5_VALUE    1

#define BL_OFF_TIME_1       15*100  //毫秒
#define BL_OFF_TIME_2       30*100
#define BL_OFF_TIME_3       60*100
#define BL_OFF_TIME_4       3*60*100
#define BL_OFF_TIME_5       5*60*100
#define BL_OFF_TIME_6       30*60*100


//system keeping information for music module

//-----------HJL----------------
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define PACK_STRUCT_BEGIN __packed
#define PACK_STRUCT_STRUCT
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#elif defined(__arm__) && defined(__GNUC__)
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT  __attribute__ ((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x
#else
#error Unsupported tools.
#endif
#ifdef _MUSIC_
typedef PACK_STRUCT_BEGIN  struct _MUSICCONFIG
{
    uint16 FileNum;                 //current file number.
    uint32 CurTime;                 //current time.
    uint8  HoldOnPlaySaveFlag;
    uint8  RepeatMode;              //repeat mode
    uint8  RepeatModeBak;
    uint8  PlayOrder;               //play order
    uint32 ShuffleSeed;
    uint8  BassBoost;

    #ifdef  _RK_EQ_
    RKEffect Eq;                    //Eq
    #endif

    UINT32 HoldMusicFullInfoSectorAddr ;
    UINT32 HoldMusicSortInfoSectorAddr;
    UINT16 HoldMusicuiBaseSortId[4];
    UINT16 HoldMusicPlayType ;
    UINT16 HoldMusicTypeSelName[SYS_SUPPROT_STRING_MAX_LEN + 1];
    UINT16 HoldMusicucCurDeep ;
    UINT16 HoldMusicuiCurId[4]  ;
    UINT16 HoldMusicTotalFiles;
    UINT16 HoldClusSave;
}PACK_STRUCT_STRUCT MUSIC_CONFIG;
PACK_STRUCT_END
#endif


#ifdef _BLUETOOTH_
#define  BT_LINK_KEY_MAX_NUM 2    // do not modify, if you want to modify, must be Notice RK
typedef PACK_STRUCT_BEGIN struct _BT_LINK_KEY
{
    uint8 BdAddr[6];
    uint8 LinkKey[16];
    //#ifdef _A2DP_SOUCRE_
    uint8 name[32];
    //#endif
    uint8 KeyTpye;

}PACK_STRUCT_STRUCT BT_LINK_KEY;
PACK_STRUCT_END

typedef PACK_STRUCT_BEGIN struct _BLUETOOTHCONFIG
{
    uint8 LastConnectMac[6];                 //current file number it is use for breakpoint recover.

    BT_LINK_KEY BtLinkKey[BT_LINK_KEY_MAX_NUM];           //automatic browse switch
    uint8 KeyIndex;                     //automatic browse time
    uint8 PairedDevCnt;
    uint8 btConnected;
    uint8 btOpened;
    uint8 btCtrlTaskRun;
}PACK_STRUCT_STRUCT BLUETOOTH_CONFIG;
PACK_STRUCT_END
#endif

#ifdef __WEB_CHANNELS_RK_FOCHANNEL_C__
typedef PACK_STRUCT_BEGIN struct __CHANNELS_CONFIG
{
    uint32 serverip;
    uint32 serverport;
    uint32 volume;
    uint32 channel;
    uint32 bitrate;
    uint32 samplerate;
    uint8 flag;
}PACK_STRUCT_STRUCT CHANNELS_CONFIG;
PACK_STRUCT_END
#endif

#ifdef _RECORD_
typedef PACK_STRUCT_BEGIN struct _RECORDCONFIG
{
    uint8 RecordVol;                //record volume
    uint8 RecordQuality;            //record quality
    uint8 RecordFmt;                //record format :normal -- mp3/wav(adpcm), high quality -- pcm
    uint16 RecordIndex;

}PACK_STRUCT_STRUCT RECORD_CONFIG;
PACK_STRUCT_END
#endif

//system keeping information for FM module
#ifdef _RADIO_
typedef PACK_STRUCT_BEGIN struct _RADIOCONFIG
{
    uint8  FmArea;                  //receive area.
    uint8  FmStereo;                //stereo switch
    uint8  FmState;                 //current status, hand or auto,
    uint8  FmSaveNum;               //current saved stations number.
    uint16 FmFreq;                  //current saved station frequency.
    uint16 FmFreqArray[40];         //saved station frequency group.
    uint8  ScanSensitivity;//PAGE
    uint8  HoldOnPlaySaveFlag;//PAGE

}PACK_STRUCT_STRUCT RADIO_CONFIG;
PACK_STRUCT_END
#endif

typedef PACK_STRUCT_BEGIN struct _MEDIALIBCONFIG
{
    UINT16 gMusicFileNum;       // the total number of all music.
    UINT16 gID3TitleFileNum;    // the file number who has id3 title infomation.
    UINT16 gID3ArtistFileNum;   // the file number who has id3 artist infomation.
    UINT16 gID3AlbumFileNum;    // the file number who has id3 album infomation.
    UINT16 gID3GenreFileNum;    // the file number who has id3 genre infomation.
    UINT16 gMyFavoriteFileNum;  // the file number of stroe folder.
    UINT16 gRecordFmFileNum;
    UINT32 gRecordVoiceFileNum;
    UINT16 gTotalFileNum;
    UINT16 MediaUpdataFlag;     // the flag that media libary had update.

    UINT32 gJpegFileNum;
    UINT16 gJpegTotalFileNum;
}PACK_STRUCT_STRUCT MEDIALIB_CONFIG;
PACK_STRUCT_END

typedef PACK_STRUCT_BEGIN struct _MEDIABRO_DIR_TREE_STRUCT
{
    UINT8  MediaType;
    UINT16 MusicDirDeep;
    UINT16 Cursor[4];    //光标位置: 0~7
    UINT16 CurItemId[4]; //光标所指向的item在当前目录下的文件号
    UINT16 MusicDirBaseSortId[4];
    UINT32 ulFileFullInfoSectorAddr;
    UINT32 ulFileSortInfoSectorAddr;
}PACK_STRUCT_STRUCT MEDIABRO_DIR_TREE_STRUCT;
PACK_STRUCT_END

typedef PACK_STRUCT_BEGIN struct _MEDIA_FLODER_INFO_STRUCT
{
    UINT16 MusicDirDeep;
    UINT16 TotalSubDir;
    UINT16 Cursor[MAX_DIR_DEPTH];    //光标位置: 0~7
    UINT16 CurItemId[MAX_DIR_DEPTH]; //光标所指向的item在当前目录下的文件号
    UINT32 DirClus[MAX_DIR_DEPTH];
}PACK_STRUCT_STRUCT MEDIA_FLODER_INFO_STRUCT;
PACK_STRUCT_END

//system setting parameter structure definition.
typedef PACK_STRUCT_BEGIN struct _SYSCONFIG
{
    uint32 FirmwareFlag;            // "SAVE"
    UINT8  SysLanguage;             //current system language environment.
    UINT8  OutputVolume;            //system volume
    UINT8  SDEnable;                //sd card select
    UINT8  FMEnable;                //FM setting menu judge.
    UINT8  KeyNum;                  //it is used for 5.6.7 keys judgement.
    UINT8  Softin;

    UINT8  BeepEnabled;
    UINT8  FrenchRegion; // French region:1, Other region:0

    UINT8  DlnaNotOpen;
    //UINT8  SmartConfigEnabled;
    UINT8  DlnaEnabled;
    UINT8  AirPlayerEnabled;
    UINT8  playerOpenFail;
    uint32 SelPlayType;
    uint32 MediaLibrayStartLBA;
    uint32 WifiSSIDStartLBA;
    uint32 BookMarkStartLBA;
    uint32 XmlBufStartLBA;

#ifdef _USE_GUI_
    UINT8  BLmode;                  //backlight display mode.
    UINT8  BLevel;                  //backlight level
    UINT8  BLtime;                  //Screen off need time level
#endif

    UINT8  SysIdle;
    uint8  SysIdleStatus;
    uint32 SysIdle1EventTime;
    uint32 SysIdle2EventTime;
    uint32 SysIdle3EventTime;
    UINT32 ShutTime;                //auto close time
    uint32 PMTime;

#ifndef _USE_GUI_
    int8   BtControl;
    uint8  PlayerType;
#endif

#ifdef _MEDIA_MODULE_
    MEDIALIB_CONFIG MedialibPara;
    MEDIABRO_DIR_TREE_STRUCT MediaDirTreeInfo;
#endif

#ifdef _MUSIC_
    MUSIC_CONFIG MusicConfig;
#endif

#ifdef _BLUETOOTH_
    int8   BtOpened;
    BLUETOOTH_CONFIG BtConfig;
#endif

#ifdef __WEB_CHANNELS_RK_FOCHANNEL_C__
    CHANNELS_CONFIG SpeakConfig;
#endif

#ifdef _RECORD_
    RECORD_CONFIG RecordConfig;
#endif

#ifdef _RADIO_
    RADIO_CONFIG RadioConfig;
#endif

    uint32 battery_level;

    uint32 SafePoweroff;
    uint32 FirmwareCheck;            // "INFO"

    uint32 crc;    // don't change

}PACK_STRUCT_STRUCT SYSCONFIG,*PSYSCONFIG;
PACK_STRUCT_END

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern SYSCONFIG  gSysConfig;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void SaveSysInformation(int flag);
extern void LoadSysInformation(void);
#endif
