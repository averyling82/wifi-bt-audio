/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\ModuleOverlay\SysInfoSave.c
* Owner: aaron.sun
* Date: 2015.10.28
* Time: 17:23:22
* Version: 1.0
* Desc: sys info save
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.28     17:23:22   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __SYSTEM_MODULEOVERLAY_SYSINFOSAVE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "rkos.h"
#include "SysInfoSave.h"
#include "menuresourceID.h"


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
static  const char crc_table[256] = {
    0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53,
    0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
    0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E,
    0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
    0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4,
    0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
    0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19,
    0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
    0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40,
    0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
    0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D,
    0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
    0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7,
    0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
    0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A,
    0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
    0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75,
    0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
    0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8,
    0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
    0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2,
    0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
    0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F,
    0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
    0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66,
    0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
    0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB,
    0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
    0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1,
    0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
    0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C,
    0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4
};


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
SYSCONFIG  gSysConfig;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
void WriteReservedData(uint32 block, uint8 * Buf, int16 Length);
uint8 check_crc8(const char *data, size_t len);
uint32 GetSysReservedNextPage(uint32 block);
void ReadReservedData(uint32 block , uint8 *Buf, int16 Length);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SaveSysInformation
** Input:int flag
** Return: void
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:28:01
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_
COMMON API void SaveSysInformation(int flag)
{
    uint8  TempBuffer[1024];
    memset(TempBuffer, 0xff, 1024);

    if(flag)
        gSysConfig.SafePoweroff = 0x55aa;
    else
        gSysConfig.SafePoweroff = 0;

    {
        uint32 crcval;

        //crc长度以bit为单位，最后四个字节保存crc值，不参与crc校验
        gSysConfig.crc = check_crc8((const char *)&gSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("gSysConfig.crc = %d", gSysConfig.crc);
    }
    memcpy(TempBuffer, (uint8*)&gSysConfig, sizeof(SYSCONFIG));



    WriteReservedData(SYSDATA_BLK, TempBuffer, 1024);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: WriteReservedData
** Input:uint32 block, uint8 * Buf, int16 Length
** Return: void
** Owner:aaron.sun
** Date: 2015.12.25
** Time: 17:53:47
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_
COMMON FUN void WriteReservedData(uint32 block, uint8 * Buf, int16 Length)
{
    uint32 Page, PageLen;
    uint32 SysAreaSecPerPage;
    uint32 SysAreaPagePerBlock;
    uint32 LBA;
    uint32 Sectors;

    DATABASE_INFO stDataBaseInf;

    FW_GetDBInf(&stDataBaseInf);


    SysAreaSecPerPage   = stDataBaseInf.PageSize;
    SysAreaPagePerBlock = stDataBaseInf.BlockSize / SysAreaSecPerPage;

    PageLen = (Length + (SysAreaSecPerPage<<9) - 1) / (SysAreaSecPerPage<<9);
    Page = GetSysReservedNextPage(block);

    if(Page == 0)
    {
        Page = SysAreaPagePerBlock;
    }
    else
    {
        Page--;
    }

    LBA = stDataBaseInf.BlockSize * block;
    LBA += Page * SysAreaSecPerPage;
    Sectors = (Length + 512 - 1) / 512;

    rk_printf("W PageLen = %d, SysAreaSecPerPage = %d", PageLen, SysAreaSecPerPage);
    FW_WriteDataBaseBySector(LBA , Buf, Sectors);

}

/*******************************************************************************
** Name: check_crc8
** Input:const char *data, size_t len
** Return: uint8
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:47:43
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_
COMMON FUN uint8 check_crc8(const char *data, size_t len)
{
    char crc = 0x0f;
    size_t i;
    char octet;

    for (i = 0; i < len / 8; i++)
        crc = crc_table[crc^data[i]];

    octet = data[i];
    for (i = 0; i < len % 8; i++) {
        char bit = ((octet^crc) & 0x80) >> 7;

        crc = ((crc & 0x7f) << 1) ^ (bit ? 0x1d : 0);

        octet = octet << 1;
    }

    return crc;
}

/*******************************************************************************
** Name: GetSysReservedNextPage
** Input:uint32 block
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:43:34
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_
COMMON FUN uint32 GetSysReservedNextPage(uint32 block)
{
    uint32 i, Page;
    uint32 SysSecPerPage;
    uint32 SysPagePerBlock;
    uint8  FlashBuf[512];
    uint32 LBA;
    DATABASE_INFO stDataBaseInfo;

    FW_GetDBInf(&stDataBaseInfo);

    Page = 0;

    {
        SysSecPerPage   = stDataBaseInfo.PageSize;
        SysPagePerBlock = stDataBaseInfo.BlockSize/SysSecPerPage;

        LBA = stDataBaseInfo.BlockSize * block;

        //从Block的起始Page + 偏移地址 开始搜索空的Page
        for (Page = 0; Page < SysPagePerBlock; Page++)
        {
            FW_ReadDataBaseBySector(LBA + Page * SysSecPerPage, FlashBuf,  1);

            if(FlashBuf[0] != 'S')
            {
                break;
            }
            else if(FlashBuf[1] != 'A')
            {
                break;
            }
            else if(FlashBuf[2] != 'V')
            {
                break;
            }
            else if(FlashBuf[3] != 'E')
            {
                break;
            }

        }
    }

    if(Page == SysPagePerBlock)
    {
        Page = 0;
    }

    return Page;
}

/*******************************************************************************
** Name: ReadReservedData
** Input:uint32 block , uint8 *Buf, int16 Length
** Return: void
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:30:52
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_COMMON_
COMMON FUN void ReadReservedData(uint32 block , uint8 *Buf, int16 Length)
{
    uint32 i, Page, PageLen, Sectors;
    uint32 SysAreaSecPerPage;
    uint32 SysAreaPagePerBlock;

    uint32 LBA;

    DATABASE_INFO stDataBaseInf;

    FW_GetDBInf(&stDataBaseInf);

    SysAreaSecPerPage   = stDataBaseInf.PageSize;
    SysAreaPagePerBlock = stDataBaseInf.BlockSize/ SysAreaSecPerPage;

    PageLen = (Length + (SysAreaSecPerPage<<9) - 1) / (SysAreaSecPerPage<<9);
    Page = GetSysReservedNextPage(block);

    if(Page == 0)
    {
        Page = SysAreaPagePerBlock;
    }
    else
    {
        Page--;
    }

    LBA = stDataBaseInf.BlockSize * block;
    LBA += Page * SysAreaSecPerPage;
    Sectors = (Length + 512 - 1) / 512;

    rk_printf("R PageLen = %d, SysAreaSecPerPage = %d", PageLen, SysAreaSecPerPage);
    FW_ReadDataBaseBySector(LBA, Buf, Sectors);

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LoadSysInformation
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:27:37
*******************************************************************************/
_SYSTEM_MODULEOVERLAY_SYSINFOSAVE_INIT_
INIT API void LoadSysInformation(void)
{
    uint32 i,j = 0;
    uint32 Len;
    uint32 SystemDefaultParaAddr;
    uint8  * TempBuffer;
    uint8  * FlashBuf;

    uint16 FirmwareYear;
    uint16 FirmwareDate;
    uint16 MasterVersion;
    uint16 SlaveVersion;
    uint16 SmallVersion;
    uint32 MenuLogicAddress0, MenuLogicAddress1;

    SYSCONFIG             *pSysConfig;
    FIRMWARE_INFO_T       *pFirmwareModuleInfo;
    SYSTEM_DEFAULT_PARA_T *pSystemDefaultPara;
    FIRMWARE_HEADER       *pFWHead;
    BOOL CleanHistortyFlag = FALSE;
    HDC hPwmDev = NULL;
    UINT32 rate;
	

    TempBuffer = rkos_memory_malloc(1024);
    if(TempBuffer == NULL)
    {
        return;
    }

    FlashBuf = rkos_memory_malloc(512);
    if(FlashBuf == NULL)
    {
        rkos_memory_free(TempBuffer);
        return;
    }

    FW_ReadFirmwaveByByte(gstFwInf.SystemDefaultParaAddress,  FlashBuf, sizeof(SYSTEM_DEFAULT_PARA_T));
    pSystemDefaultPara = (SYSTEM_DEFAULT_PARA_T *)FlashBuf;

    ReadReservedData(SYSDATA_BLK, TempBuffer, 1024);

    pSysConfig = (SYSCONFIG *)TempBuffer;
    // Check "SAVE INFO"
    {
        uint32 crcval;
        //crc长度以bit为单位，最后四个字节保存crc值，不参与crc校验
        crcval = check_crc8((const char *)pSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("crcval = %d, pconfig->crc = %d", crcval, pSysConfig->crc);
        if (crcval != pSysConfig->crc)
        {
            DEBUG("warning,crcval != pSysConfig->crc\n");
            memset(TempBuffer, 0xff, 1024);
            #ifdef ENABLE_MBR
            //SendMsg(MSG_FORMAT_DISK);
            #endif
        }
        else
        {
            if ((pSysConfig->FirmwareFlag != 0x45564153) || (pSysConfig->FirmwareCheck != 0x4f464e49))
            {
                DEBUG("warning,upgrade poweron and some error?\n");
                // if no "SAVEINFO" Flag, 1st upgrade poweron and some error, need clear configer Param
                // and format disk
                memset(TempBuffer, 0xff, 1024);

                #ifdef ENABLE_MBR
                //SendMsg(MSG_FORMAT_DISK);
                #endif
            }
        }
        gSysConfig.crc = crcval;
    }

    gSysConfig.FirmwareFlag  = 0x45564153;      // "SaveInfo" Flag
    gSysConfig.FirmwareCheck = 0x4f464e49;      //


    #ifdef _CDROM_
    gSysConfig.bShowCdrom= pSysConfig->bShowCdrom & 0x01;
    #endif

    //system language.
    gSysConfig.SysLanguage = pSysConfig->SysLanguage;

    if (gSysConfig.SysLanguage > (TOTAL_LANAUAGE_NUM - 1))
    {
        gSysConfig.SysLanguage = (uint8)pSystemDefaultPara->gLanguage;
    }

    //system volume
    gSysConfig.OutputVolume = pSysConfig->OutputVolume;
    if (gSysConfig.OutputVolume > 32)
    {
        gSysConfig.OutputVolume = (uint8)pSystemDefaultPara->Volume;
    }
    
#ifdef _USE_GUI_
    //backlight mode
    gSysConfig.BLmode = pSysConfig->BLmode;
    if (gSysConfig.BLmode > 1)
    {
        gSysConfig.BLmode = (uint8)pSystemDefaultPara->BLMode;
    }

    //backlight level
    gSysConfig.BLevel = pSysConfig->BLevel;
    if (gSysConfig.BLevel > 5)
    {
        gSysConfig.BLevel = (uint8)pSystemDefaultPara->BLevel;
    }

    //backlight off need time level
    gSysConfig.BLtime = pSysConfig->BLtime;
    if (gSysConfig.BLtime > 6)
    {
        gSysConfig.BLtime = (uint8)pSystemDefaultPara->BLtime;
    }

    switch (gSysConfig.BLtime)
    {
        case 1:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_1; break;
        case 2:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_2; break;
        case 3:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_3; break;
        case 4:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_4; break;
        case 5:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_5; break;
        case 6:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_6; break;
        default:
           gSysConfig.SysIdle1EventTime = BL_OFF_TIME_1; break;
    }
#else
    gSysConfig.SysIdle1EventTime = BL_OFF_TIME_1;
#endif

    #ifndef _USE_GUI_
    gSysConfig.PlayerType = pSysConfig->PlayerType;
    if ((gSysConfig.PlayerType<=SOURCE_FROM_NET)||(gSysConfig.PlayerType>=SOURCE_FROM_FILE_BROWSER))
    {

        #ifdef _BLUETOOTH_
			gSysConfig.PlayerType = SOURCE_FROM_BT;
        #endif

		#if defined(_DRIVER_WIFI__) || defined(_ENABLE_WIFI_BLUETOOTH)
        	gSysConfig.PlayerType = SOURCE_FROM_DLNA;
		#endif
		printf ("\n1:--------------gSysConfig.PlayerType=0x%x\n",gSysConfig.PlayerType);
    }
    #endif
#if 0
	//DEBUG("jjjhhh----startmode-----------\n");
    gSysConfig.PlayerType = SOURCE_FROM_FILE_BROWSER;//start airplay when poweron ----jjjhhh  SOURCE_FROM_FILE_BROWSER
#endif

    //automatic shutdown time
    gSysConfig.ShutTime = pSysConfig->ShutTime;
    if (gSysConfig.ShutTime > 5)
    {
        gSysConfig.ShutTime = (uint8)pSystemDefaultPara->SettingPowerOffTime;
    }

    gSysConfig.BeepEnabled= pSysConfig->BeepEnabled;
    if (gSysConfig.BeepEnabled > 1)
    {
        gSysConfig.BeepEnabled = (uint8)pSystemDefaultPara->BeepEnabled;
    }

    #ifdef _VIDEO_
    gSysConfig.VideoConfig.FileNum = pSysConfig->VideoConfig.FileNum;

    if (gSysConfig.VideoConfig.FileNum == 0)
    {
        gSysConfig.VideoConfig.FileNum = 1;
    }

    gSysConfig.VideoConfig.HoldOnPlaySaveFlag = pSysConfig->VideoConfig.HoldOnPlaySaveFlag;
    if (gSysConfig.VideoConfig.HoldOnPlaySaveFlag > 1)
    {
        gSysConfig.VideoConfig.HoldOnPlaySaveFlag = 0;
    }

    gSysConfig.VideoConfig.Video_Current_FrameNum = pSysConfig->VideoConfig.Video_Current_FrameNum;
    #endif

    #ifdef _MUSIC_
    gSysConfig.MusicConfig.HoldOnPlaySaveFlag = pSysConfig->MusicConfig.HoldOnPlaySaveFlag;

    if (gSysConfig.MusicConfig.HoldOnPlaySaveFlag > 1)
    {
        gSysConfig.MusicConfig.HoldOnPlaySaveFlag = 0;
    }

    gSysConfig.MusicConfig.FileNum = pSysConfig->MusicConfig.FileNum;
    if (gSysConfig.MusicConfig.FileNum == 0)
    {
        gSysConfig.MusicConfig.FileNum = 1;
    }

    gSysConfig.MusicConfig.CurTime = pSysConfig->MusicConfig.CurTime;

    gSysConfig.MusicConfig.RepeatModeBak = pSysConfig->MusicConfig.RepeatMode;
    if (gSysConfig.MusicConfig.RepeatModeBak > AUDIO_REPEAT1)
    {
        gSysConfig.MusicConfig.RepeatModeBak = (uint8)pSystemDefaultPara->MusicRepMode;
    }

    gSysConfig.MusicConfig.RepeatMode = pSysConfig->MusicConfig.RepeatMode;
    if (gSysConfig.MusicConfig.RepeatMode > AUDIO_REPEAT)
    {
        gSysConfig.MusicConfig.RepeatMode = (uint8)pSystemDefaultPara->MusicRepMode;
    }

    gSysConfig.MusicConfig.PlayOrder = pSysConfig->MusicConfig.PlayOrder;
    if (gSysConfig.MusicConfig.PlayOrder > AUDIO_RAND)
    {
        gSysConfig.MusicConfig.PlayOrder = (uint8)pSystemDefaultPara->MusicPlayOrder;
    }
    gSysConfig.MusicConfig.BassBoost= pSysConfig->MusicConfig.BassBoost;
    if (gSysConfig.MusicConfig.BassBoost > 1)
    {
        gSysConfig.MusicConfig.BassBoost = (uint8)pSystemDefaultPara->MusicBassBoost;
    }

    #ifdef _RK_EQ_
    gSysConfig.MusicConfig.Eq.Mode = pSysConfig->MusicConfig.Eq.Mode;
    if (gSysConfig.MusicConfig.Eq.Mode > EQ_NOR)
    {
        gSysConfig.MusicConfig.Eq.Mode = pSystemDefaultPara->MusicEqSel;
    }

    for(i = 0;i < 5;i++)
    {
        gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = pSysConfig->MusicConfig.Eq.RKCoef.dbGain[i];
        if ((UINT16)(gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i]) > 7)
        {
            gSysConfig.MusicConfig.Eq.RKCoef.dbGain[i] = 3;
        }
    }
    #endif



    #ifdef _MUSIC_
    //7.30 azg 修改连续两次关机后，断点不能正常播放的bug，以及由断点音乐退回媒体库界面时，光标定位不准的bug
    gSysConfig.MusicConfig.HoldMusicFullInfoSectorAddr = pSysConfig->MusicConfig.HoldMusicFullInfoSectorAddr;
    gSysConfig.MusicConfig.HoldMusicSortInfoSectorAddr =  pSysConfig->MusicConfig.HoldMusicSortInfoSectorAddr ;
    gSysConfig.MusicConfig.HoldMusicPlayType           = pSysConfig->MusicConfig.HoldMusicPlayType;
    gSysConfig.MusicConfig.HoldMusicTotalFiles         = pSysConfig->MusicConfig.HoldMusicTotalFiles;

    gSysConfig.MusicConfig.HoldMusicucCurDeep          = pSysConfig->MusicConfig.HoldMusicucCurDeep;
    if(gSysConfig.MusicConfig.HoldMusicucCurDeep > 3)
    {
        gSysConfig.MusicConfig.HoldMusicucCurDeep = 3;
    }

    for(i = 0; i <= gSysConfig.MusicConfig.HoldMusicucCurDeep; i++)
    {
        gSysConfig.MusicConfig.HoldMusicuiBaseSortId[i] = pSysConfig->MusicConfig.HoldMusicuiBaseSortId[i] ;
        gSysConfig.MusicConfig.HoldMusicuiCurId[i]      = pSysConfig->MusicConfig.HoldMusicuiCurId[i];
    }

    for (i = 0; i < (SYS_SUPPROT_STRING_MAX_LEN + 1); i++)
    {
        gSysConfig.MusicConfig.HoldMusicTypeSelName[i] = pSysConfig->MusicConfig.HoldMusicTypeSelName[i];
    }

    gSysConfig.MusicConfig.ShuffleSeed = pSysConfig->MusicConfig.ShuffleSeed;
    if((gSysConfig.MusicConfig.ShuffleSeed == 0) ||(gSysConfig.MusicConfig.ShuffleSeed > 4096))
    {
        gSysConfig.MusicConfig.ShuffleSeed = 1;
    }

    #endif

    #endif

    #ifdef _RADIO_
    gSysConfig.RadioConfig.FmArea = pSysConfig->RadioConfig.FmArea;
    if (gSysConfig.RadioConfig.FmArea > 3)
    {
        gSysConfig.RadioConfig.FmArea = (uint8)pSystemDefaultPara->FMArea;
    }

    gSysConfig.RadioConfig.FmStereo = pSysConfig->RadioConfig.FmStereo;
    if (gSysConfig.RadioConfig.FmStereo > 1)
    {
        gSysConfig.RadioConfig.FmStereo = (uint8)pSystemDefaultPara->FMStereo;
    }

    gSysConfig.RadioConfig.FmState= pSysConfig->RadioConfig.FmState;
    if (gSysConfig.RadioConfig.FmState > 1)
    {
        gSysConfig.RadioConfig.FmState = 0;
    }

    gSysConfig.RadioConfig.FmSaveNum = pSysConfig->RadioConfig.FmSaveNum;
    if (gSysConfig.RadioConfig.FmSaveNum > 40)
    {
        gSysConfig.RadioConfig.FmSaveNum = 0;
    }

    gSysConfig.RadioConfig.FmFreq = pSysConfig->RadioConfig.FmFreq;

    for (i = 0; i < 40; i++)
    {
        gSysConfig.RadioConfig.FmFreqArray[i] = pSysConfig->RadioConfig.FmFreqArray[i];
    }

    gSysConfig.RadioConfig.ScanSensitivity= pSysConfig->RadioConfig.ScanSensitivity;
    if (gSysConfig.RadioConfig.ScanSensitivity > 1)
    {
        gSysConfig.RadioConfig.ScanSensitivity= pSystemDefaultPara->FMScanSensitivity;;
    }

    gSysConfig.RadioConfig.HoldOnPlaySaveFlag = pSysConfig->RadioConfig.HoldOnPlaySaveFlag;
    if (gSysConfig.RadioConfig.HoldOnPlaySaveFlag > 2)
    {
        gSysConfig.RadioConfig.HoldOnPlaySaveFlag= 0;
    }

    #endif

    #ifdef _RECORD_
    gSysConfig.RecordConfig.RecordQuality = pSysConfig->RecordConfig.RecordQuality;
    if (gSysConfig.RecordConfig.RecordQuality > 1)
    {
        gSysConfig.RecordConfig.RecordQuality = 0;
    }

    gSysConfig.RecordConfig.RecordFmt = pSysConfig->RecordConfig.RecordFmt;
    if (gSysConfig.RecordConfig.RecordFmt > 1)
    {
        gSysConfig.RecordConfig.RecordFmt= 0;
    }

    gSysConfig.RecordConfig.RecordVol = pSysConfig->RecordConfig.RecordVol;
    if (gSysConfig.RecordConfig.RecordVol > 5)
    {
        gSysConfig.RecordConfig.RecordVol = 2;
    }

    gSysConfig.RecordConfig.RecordIndex = pSysConfig->RecordConfig.RecordIndex;
    if((gSysConfig.RecordConfig.RecordIndex > 9999) || (gSysConfig.RecordConfig.RecordIndex == 0))
    {
        gSysConfig.RecordConfig.RecordIndex = 1;
    }
    #endif

    #ifdef _EBOOK_
    gSysConfig.TextConfig.FileNum = pSysConfig->TextConfig.FileNum;
    if (gSysConfig.TextConfig.FileNum == 0)
    {
        gSysConfig.TextConfig.FileNum = 1;
    }

    gSysConfig.TextConfig.AutoPlay = pSysConfig->TextConfig.AutoPlay;
    if (gSysConfig.TextConfig.AutoPlay > 1)
    {
        gSysConfig.TextConfig.AutoPlay = 1;
    }

    gSysConfig.TextConfig.AutoPlayTime = pSysConfig->TextConfig.AutoPlayTime;
    if (gSysConfig.TextConfig.AutoPlayTime > 5)
    {
        gSysConfig.TextConfig.AutoPlayTime = (uint8)pSystemDefaultPara->TextAutoPlayTime;
    }
    #endif

    #ifdef _PICTURE_
    gSysConfig.PicConfig.FileNum = pSysConfig->PicConfig.FileNum;
    if (gSysConfig.PicConfig.FileNum == 0)
    {
        gSysConfig.PicConfig.FileNum = 1;
    }

    gSysConfig.PicConfig.AutoPlaySwitch = pSysConfig->PicConfig.AutoPlaySwitch;
    if (gSysConfig.PicConfig.AutoPlaySwitch > 1)
    {
        gSysConfig.PicConfig.AutoPlaySwitch = 0;
    }

    gSysConfig.PicConfig.AutoPlayTime = pSysConfig->PicConfig.AutoPlayTime;
    if (gSysConfig.PicConfig.AutoPlayTime > 5)
    {
        gSysConfig.PicConfig.AutoPlayTime = (uint8)pSystemDefaultPara->PicAutoPlayTime;
    }
    #endif

    //#ifdef _WIFI_FOR_SYSTEM_
    //gSysConfig.SmartConfigEnabled = pSysConfig->SmartConfigEnabled;
    //if((gSysConfig.SmartConfigEnabled != 0) || (gSysConfig.SmartConfigEnabled != 1))
    //    gSysConfig.SmartConfigEnabled = 0;
    //#endif

    #ifdef _MEDIA_MODULE_ //媒体库
    gSysConfig.MedialibPara.gID3AlbumFileNum     = pSysConfig->MedialibPara.gID3AlbumFileNum;
    if(gSysConfig.MedialibPara.gID3AlbumFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3AlbumFileNum = 0;

    gSysConfig.MedialibPara.gID3ArtistFileNum    = pSysConfig->MedialibPara.gID3ArtistFileNum;
    if(gSysConfig.MedialibPara.gID3ArtistFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3ArtistFileNum = 0;

    gSysConfig.MedialibPara.gID3TitleFileNum     = pSysConfig->MedialibPara.gID3TitleFileNum;
    if(gSysConfig.MedialibPara.gID3TitleFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3TitleFileNum = 0;

    gSysConfig.MedialibPara.gID3GenreFileNum     = pSysConfig->MedialibPara.gID3GenreFileNum;
    if(gSysConfig.MedialibPara.gID3GenreFileNum == 0xffff)
        gSysConfig.MedialibPara.gID3GenreFileNum = 0;

    gSysConfig.MedialibPara.gMusicFileNum        = pSysConfig->MedialibPara.gMusicFileNum;
    if(gSysConfig.MedialibPara.gMusicFileNum == 0xffff)
        gSysConfig.MedialibPara.gMusicFileNum = 0;

    gSysConfig.MedialibPara.gMyFavoriteFileNum   = pSysConfig->MedialibPara.gMyFavoriteFileNum;
    if(gSysConfig.MedialibPara.gMyFavoriteFileNum == 0xffff)
        gSysConfig.MedialibPara.gMyFavoriteFileNum = 0;

    gSysConfig.MedialibPara.gRecordFmFileNum = pSysConfig->MedialibPara.gRecordFmFileNum;
    if(gSysConfig.MedialibPara.gRecordFmFileNum == 0xffff)
        gSysConfig.MedialibPara.gRecordFmFileNum = 0;


    gSysConfig.MedialibPara.gRecordVoiceFileNum = pSysConfig->MedialibPara.gRecordVoiceFileNum;
    if(gSysConfig.MedialibPara.gRecordVoiceFileNum == 0xffffffff)
        gSysConfig.MedialibPara.gRecordVoiceFileNum= 0;


    gSysConfig.MedialibPara.MediaUpdataFlag = pSysConfig->MedialibPara.MediaUpdataFlag;
    if(gSysConfig.MedialibPara.MediaUpdataFlag > 1)
    {
        rk_printf("gSysConfig.MedialibPara.MediaUpdataFlag = %d", gSysConfig.MedialibPara.MediaUpdataFlag);
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;
    }

    gSysConfig.MedialibPara.gJpegFileNum        = pSysConfig->MedialibPara.gJpegFileNum;
    if(gSysConfig.MedialibPara.gJpegFileNum == 0xffff)
        gSysConfig.MedialibPara.gJpegFileNum = 0;

    #ifdef _M3U_
    gSysConfig.MedialibPara.gM3uFileNum        = pSysConfig->MedialibPara.gM3uFileNum;
    if(gSysConfig.MedialibPara.gM3uFileNum == 0xffff)
        gSysConfig.MedialibPara.gM3uFileNum = 0;
    #endif

    #endif


    gSysConfig.SafePoweroff = pSysConfig->SafePoweroff;

    #ifdef _MEDIA_MODULE_ //媒体库
    if (gSysConfig.SafePoweroff != 0x55aa)
    {
        rk_printf("gSysConfig.SafePoweroff = %d", gSysConfig.SafePoweroff);
        gSysConfig.MedialibPara.MediaUpdataFlag = 1;
    }
    //gSysConfig.MedialibPara.MediaUpdataFlag = 1;    //test
    #endif

#ifdef _BLUETOOTH_
    gSysConfig.BtConfig.KeyIndex = pSysConfig->BtConfig.KeyIndex;
    if(gSysConfig.BtConfig.KeyIndex >= BT_LINK_KEY_MAX_NUM)
    {
        memset(&gSysConfig.BtConfig, 0x00, sizeof(BLUETOOTH_CONFIG));
    }
    else
    {
        memcpy(&gSysConfig.BtConfig,&pSysConfig->BtConfig, sizeof(BLUETOOTH_CONFIG));
    }
    gSysConfig.BtConfig.btCtrlTaskRun = 0;
    gSysConfig.BtOpened = 0;
#endif

    {
        DATABASE_INFO  stDataBaseInf;

        FW_GetDBInf(&stDataBaseInf);

        gSysConfig.MediaLibrayStartLBA =  MEDIA_BLK * stDataBaseInf.BlockSize;
        gSysConfig.XmlBufStartLBA = XML_BUF_BLK * stDataBaseInf.BlockSize;
        gSysConfig.WifiSSIDStartLBA = WIFI_AP_PW_BLK * stDataBaseInf.BlockSize;
        gSysConfig.BookMarkStartLBA = BOOKMARK_BLK * stDataBaseInf.BlockSize;
    }

    gSysConfig.SafePoweroff = 0;
    {
        uint32 crcval;

        //crc长度以bit为单位，最后四个字节保存crc值，不参与crc校验
        gSysConfig.crc = check_crc8((const char *)&gSysConfig, (sizeof(SYSCONFIG) - 4) * 8);
        DEBUG("gSysConfig.crc = %d", gSysConfig.crc);
    }
    memcpy(TempBuffer, (uint8*)&gSysConfig, sizeof(SYSCONFIG));
    WriteReservedData(SYSDATA_BLK, TempBuffer, 1024);

    rkos_memory_free(TempBuffer);
    rkos_memory_free(FlashBuf);



}



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
