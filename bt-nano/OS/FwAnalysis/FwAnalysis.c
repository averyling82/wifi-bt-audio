/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\FwAnalysis\FwAnalysis.c
* Owner: aaron.sun
* Date: 2015.10.28
* Time: 15:27:33
* Version: 1.0
* Desc: Firmwave driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.28     15:27:33   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "device.h"
#include "RKOS.h"
#include "BSP.h"
#include "FwAnalysis.h"
#include "global.h"
#include "SysInfoSave.h"
#include "shelltask.h"
#ifdef _SPI_BOOT_
#include "FileDevice.h"
#endif



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define D_MODULE_CNT 64

#define RKDEVICE_SN_LEN 60
#define RKDEVICE_UID_LEN 30
#define RKDEVICE_MAC_LEN 6
#define RKDEVICE_WIFI_LEN 6
#define RKDEVICE_BT_LEN 6
#define RKNANO_SEC3_RESERVED_LEN 405

typedef __packed struct tagRKNANO_IDB_SEC3
{
    uint16  usSNSize;
    uint8   sn[RKDEVICE_SN_LEN];
    uint8   reserved[RKNANO_SEC3_RESERVED_LEN];
    uint8   uidSize;
    uint8   uid[RKDEVICE_UID_LEN];
    uint8   blueToothSize;
    uint8   blueToothAddr[RKDEVICE_BT_LEN];
    uint8   macSize;
    uint8   macAddr[RKDEVICE_MAC_LEN];
} RKNANO_IDB_SEC3, *PRKNANO_IDB_SEC3;

#define _SYSTEM_FWANALYSIS_FWANALYSIS_READ_  __attribute__((section("system_fwanalysis_fwanalysis_read")))
#define _SYSTEM_FWANALYSIS_FWANALYSIS_WRITE_ __attribute__((section("system_fwanalysis_fwanalysis_write")))
#define _SYSTEM_FWANALYSIS_FWANALYSIS_INIT_  __attribute__((section("system_fwanalysis_fwanalysis_init")))
#define _SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_  __attribute__((section("system_fwanalysis_fwanalysis_shell")))
#if defined(__arm__) && defined(__ARMCC_VERSION)
#define _SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_DATA_       _SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_
#elif defined(__arm__) && defined(__GNUC__)
#define _SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_DATA_  __attribute__((section("system_fwanalysis_fwanalysis_shell_data")))
#else
#error Unknown compiling tools.
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//uint32 MDReadData(MEMDEV_ID DevID, uint32 offset, uint32 len, void *buf);
FIRMWARE_INF gstFwInf;
static HDC ghLunFW,ghLunDB;
static D_SEGMENT_INFO * gpstEmpDsegment;
D_SEGMENT_INFO * gpstDsegment;
static D_SEGMENT_INFO  stDsegment[D_MODULE_CNT];
uint16  UpgradeTimes;
uint16  LoaderVer;
uint32 	SysProgDiskCapacity;
uint32 	SysProgRawDiskCapacity;
int32  FW1Valid, FW2Valid;
uint32 FwSysOffset;

#ifdef _SPI_BOOT_
HDC Codepage;
pSemaphore CodePageReqSem;
#endif

FONT_CODEPAGE_TYPE LcdCharCodePage[] =
{
    FONT_CODEPAGE_CP936,    //LANGUAGE_CHINESE_S               0      //Simplified Chinese
    FONT_CODEPAGE_CP950,    //LANGUAGE_CHINESE_T               1      //traditional chinese
    FONT_CODEPAGE_CP936,    //LANGUAGE_ENGLISH                 2      //english
    FONT_CODEPAGE_CP949,    //LANGUAGE_KOREAN                  3      //korean
    FONT_CODEPAGE_CP936,    //LANGUAGE_JAPANESE                4      //japanese
    FONT_CODEPAGE_CP1252,   //LANGUAGE_SPAISH                  5      //spanish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_FRENCH                  6      //french
    FONT_CODEPAGE_CP1252,   //LANGUAGE_GERMAN                  7      //german
    FONT_CODEPAGE_CP1252,   //LANGUAGE_ITALIAN                 8      //italian
    FONT_CODEPAGE_CP1252,   //LANGUAGE_PORTUGUESE              9      //portuguess
    FONT_CODEPAGE_CP1251,   //LANGUAGE_RUSSIAN                10      //russian
    FONT_CODEPAGE_CP1252,   //LANGUAGE_SWEDISH                11      //swedish
    FONT_CODEPAGE_CP874,    //LANGUAGE_THAI                   12      //thai
    FONT_CODEPAGE_CP1250,   //LANGUAGE_POLAND                 13      //polish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_DENISH                 14      //danish
    FONT_CODEPAGE_CP1252,   //LANGUAGE_DUTCH                  15      //dutch
    FONT_CODEPAGE_CP1253,   //LANGUAGE_HELLENIC               16      //greek
    FONT_CODEPAGE_CP1250,   //LANGUAGE_CZECHIC                17      //czech
    FONT_CODEPAGE_CP1254,   //LANGUAGE_TURKIC                 18      //trukish
    FONT_CODEPAGE_CP1255,   //LANGUAGE_RABBINIC               19      //hebrew
    FONT_CODEPAGE_CP1252,   //LANGUAGE_ARABIC                 20      //arabic
};

FONT_CODEPAGE_TYPE U2CodePage[] =
{
    U2C_CODEPAGE_CP936,    //LANGUAGE_CHINESE_S               0      //Simplified Chinese
    U2C_CODEPAGE_CP950,    //LANGUAGE_CHINESE_T               1      //traditional chinese
    U2C_CODEPAGE_CP936,    //LANGUAGE_ENGLISH                 2      //english
    U2C_CODEPAGE_CP949,    //LANGUAGE_KOREAN                  3      //korean
    U2C_CODEPAGE_CP936,    //LANGUAGE_JAPANESE                4      //japanese
    U2C_CODEPAGE_CP1252,   //LANGUAGE_SPAISH                  5      //spanish
    U2C_CODEPAGE_CP1252,   //LANGUAGE_FRENCH                  6      //french
    U2C_CODEPAGE_CP1252,   //LANGUAGE_GERMAN                  7      //german
    U2C_CODEPAGE_CP1252,   //LANGUAGE_ITALIAN                 8      //italian
    U2C_CODEPAGE_CP1252,   //LANGUAGE_PORTUGUESE              9      //portuguess
    U2C_CODEPAGE_CP1251,   //LANGUAGE_RUSSIAN                10      //russian
    U2C_CODEPAGE_CP1252,   //LANGUAGE_SWEDISH                11      //swedish
    U2C_CODEPAGE_CP874,    //LANGUAGE_THAI                   12      //thai
    U2C_CODEPAGE_CP1250,   //LANGUAGE_POLAND                 13      //polish
    U2C_CODEPAGE_CP1252,   //LANGUAGE_DENISH                 14      //danish
    U2C_CODEPAGE_CP1252,   //LANGUAGE_DUTCH                  15      //dutch
    U2C_CODEPAGE_CP1253,   //LANGUAGE_HELLENIC               16      //greek
    U2C_CODEPAGE_CP1250,   //LANGUAGE_CZECHIC                17      //czech
    U2C_CODEPAGE_CP1254,   //LANGUAGE_TURKIC                 18      //trukish
    U2C_CODEPAGE_CP1255,   //LANGUAGE_RABBINIC               19      //hebrew
    U2C_CODEPAGE_CP1252,   //LANGUAGE_ARABIC                 20      //arabic
};

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
rk_err_t FWShellUpdate(HDC dev,  uint8 * pstr);
rk_err_t FWShellSn(HDC dev, uint8 * pstr);
rk_err_t LUNWriteDB(uint32 offset, uint32 len, void *buf);
rk_err_t LUNReadDB(uint32 offset, uint32 len, void *buf);
rk_err_t FWShellHelp(HDC dev, uint8 * pstr);
rk_err_t FWShellPcb(HDC dev, uint8 * pstr);
rk_err_t FWShellPcb(HDC dev, uint8 * pstr);
rk_err_t FWShellTest(HDC dev, uint8 * pstr);
rk_err_t FWShellSuspend(HDC dev, uint8 * pstr);
rk_err_t FWShellOpen(HDC dev, uint8 * pstr);
rk_err_t FWShellResume(HDC dev, uint8 * pstr);
rk_err_t FWShellRead(HDC dev, uint8 * pstr);
rk_err_t FWShellWrite(HDC dev, uint8 * pstr);
rk_err_t FWShellControl(HDC dev, uint8 * pstr);
rk_err_t FW_GetPicInfoWithIDNum(UINT16 pictureIDNum, PICTURE_INFO_STRUCT *psPictureInfo);
rk_err_t FW_SegmentAdd(uint32 SegmentID, SEGMENT_INFO_T * pSegment);
rk_err_t LUNReadFW(uint32 offset, uint32 len, void *buf);




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FW_CodePageDeInit
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.9.8
** Time: 15:52:00
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_CodePageDeInit(void)
{
#ifdef _SPI_BOOT_
   rkos_semaphore_delete(CodePageReqSem);
   FileDev_CloseFile(Codepage);
   Codepage = NULL;
#endif

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FwResume
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 15:45:06
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FwResume(void)
{
    if(ghLunFW == NULL)
    {
        ghLunFW = RKDev_Open(DEV_CLASS_LUN,0,NOT_CARE);
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FwSuspend
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 15:44:39
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FwSuspend(void)
{
    RKDev_Close(ghLunFW);
    ghLunFW = NULL;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_DBInit
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 10:01:13
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_DBInit(void)
{
    ghLunDB = RKDev_Open(DEV_CLASS_LUN,1,NOT_CARE);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FW_GetProductSn
** Input:void *pSn, max len must be 128
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.26
** Time: 11:48:20
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_GetProductSn(void *pSn)
{
    rk_err_t ret;
    uint8 DataBuf[512];
    PRKNANO_IDB_SEC3 pIDBSec3;
    uint32 i;

    //LUNReadFW(3, 1, DataBuf);
    LunDev_Read(ghLunFW,3 + FW_BOOT_OFFSET,DataBuf,1);

    pIDBSec3 = (PRKNANO_IDB_SEC3)DataBuf;

    if (pIDBSec3->usSNSize)
    {
        memcpy(pSn, pIDBSec3->sn, pIDBSec3->usSNSize>128?128:pIDBSec3->usSNSize);
    }
    ret = pIDBSec3->usSNSize;
    return ret;


}

/*******************************************************************************
** Name: FW_Ansi2UnicodeN
** Input:void *dat, Ucs2 * buf,int byteN, uint8 EncodeMode
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
rk_err_t FW_CodePageInit(void)
{
#ifdef _SPI_BOOT_
    FILE_ATTR stFileAttr;
    #ifdef _HIDDEN_DISK1_
    stFileAttr.Path = L"A:\\CodePage.bin";
    #else
    #ifdef _HIDDEN_DISK2_
    stFileAttr.Path = L"B:\\CodePage.bin";
    #else
    stFileAttr.Path = L"C:\\CodePage.bin";
    #endif
    #endif

    CodePageReqSem= rkos_semaphore_create(1,1);

    stFileAttr.FileName= NULL;
    Codepage= FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if((Codepage == NULL) || (Codepage == (HDC)RK_ERROR) || (Codepage == (HDC)RK_PARA_ERR))
    {
        printf("\nopen codepage fault\n");
        Codepage= NULL;
        return RK_ERROR;
    }
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_CodePageUnLoad
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.21
** Time: 11:05:16
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_CodePageUnLoad(void)
{
#ifdef _SPI_BOOT_
   rkos_semaphore_take(CodePageReqSem, MAX_DELAY);
   FileDev_CloseFile(Codepage);
   Codepage = NULL;
   rkos_semaphore_give(CodePageReqSem);
#endif

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_CodePageLoad
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.21
** Time: 11:14:53
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_CodePageLoad(void)
{
#ifdef _SPI_BOOT_
    FILE_ATTR stFileAttr;

    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);
    #ifdef _HIDDEN_DISK1_
    stFileAttr.Path = L"A:\\CodePage.bin";
    #else
    #ifdef _HIDDEN_DISK2_
    stFileAttr.Path = L"B:\\CodePage.bin";
    #else
    stFileAttr.Path = L"C:\\CodePage.bin";
    #endif
    #endif

    stFileAttr.FileName= NULL;
    Codepage = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
    if((Codepage == NULL) || (Codepage == (HDC)RK_ERROR) || (Codepage == (HDC)RK_PARA_ERR))
    {
        printf("\nopen codepage fault\n");
        Codepage= NULL;
        rkos_semaphore_give(CodePageReqSem);
        return RK_ERROR;
    }
    rkos_semaphore_give(CodePageReqSem);
#endif

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: FW_Ansi2UnicodeN
** Input:void *dat, Ucs2 * buf,int byteN, uint8 EncodeMode
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
uint32 FW_Ansi2UnicodeN(void *dat, Ucs2 * buf,int byteN, uint8 EncodeMode)
{
    uint8 temp[2], *src8;
    uint32 LanguageOffset, len= 0;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return 0;
    }
    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);

    src8 = (uint8 *)dat;
    while(byteN> 0)
    {
        if( *src8<=0x7F )
        {
            *buf= (uint16)*src8;
            buf++;
            src8++;
            byteN--;
        }
        else
        {
            temp[0]= src8[1];
            temp[1]= src8[0];
            LanguageOffset= LcdCharCodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*(uint16 *)temp*2);

            FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
            FileDev_ReadFile(Codepage, temp, 2);
            *buf= temp[0]<<8|temp[1];
            buf++;
            src8+= 2;
            byteN-= 2;
        }

        len+= 2;
        if(src8[0]==0 && src8[1]== 0)
        {
            *buf= 0;
            break;
        }
    }
    rkos_semaphore_give(CodePageReqSem);
#else
    src8 = (uint8 *)dat;
    while(byteN> 0)
    {
        if( *src8<=0x7F )
        {
            *buf= (uint16)*src8;
            buf++;
            src8++;
            byteN--;
        }
        else
        {
            temp[0]= src8[1];
            temp[1]= src8[0];
            LanguageOffset= LcdCharCodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*(uint16 *)temp*2);
            FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
            *buf= temp[0]<<8|temp[1];
            buf++;
            src8+= 2;
            byteN-= 2;
        }

        len+= 2;
        if(src8[0]==0 && src8[1]== 0)
        {
            *buf= 0;
            break;
        }
    }
#endif
    return len;
}

/*******************************************************************************
** Name: FW_Unicode2AnsiN
** Input:Ucs2 *dat, void * buf,int byteN, uint8 EncodeMode
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
uint32 FW_Unicode2AnsiN(Ucs2 *dat, void * buf,int byteN, uint8 EncodeMode)
{
    uint8 temp[2], *src8;
    uint32 LanguageOffset, len= 0;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return 0;
    }

    if( byteN%2!=0)
    {
        return 0;
    }

    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);

    src8 = (uint8 *)buf;
    while(byteN> 0)
    {
        if( *dat<=0x7F )
        {
            *src8= (uint8)*dat;
            src8++;
            len++;
        }
        else
        {
            LanguageOffset= U2CodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*dat*2);

            FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
            FileDev_ReadFile(Codepage, temp, 2);
            src8[0]= temp[1];
            src8[1]= temp[0];
            src8+= 2;
            len+= 2;
        }
        dat++;
        byteN-= 2;

        if(*dat==0)
        {
            src8[0]= 0;
            src8[1]= 0;
            break;
        }
    }
    rkos_semaphore_give(CodePageReqSem);
#else
    src8 = (uint8 *)buf;
    while(byteN> 0)
    {
        if( *dat<=0x7F )
        {
            *src8= (uint8)*dat;
            src8++;
            len++;
        }
        else
        {
            LanguageOffset= U2CodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*dat*2);
            FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
            src8[0]= temp[1];
            src8[1]= temp[0];
            src8+= 2;
            len+= 2;
        }
        dat++;
        byteN-= 2;

        if(*dat==0)
        {
            src8[0]= 0;
            src8[1]= 0;
            break;
        }
    }
#endif
    return len;
}

/*******************************************************************************
** Name: FW_Ansi2UnicodeStr
** Input:void *dat, Ucs2 * buf,uint8 EncodeMode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
rk_err_t FW_Ansi2UnicodeStr(void *dat, Ucs2 * buf,uint8 EncodeMode)
{
    uint8 temp[2], *src8;
    uint32 LanguageOffset;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);

    src8 = (uint8 *)dat;
    while(*src8!= 0 && *(src8+1)!= 0)
    {
        if( *src8<=0x7F )
        {
            *buf= (uint16)*src8;
            buf++;
            src8++;
        }
        else
        {
            temp[0]= src8[1];
            temp[1]= src8[0];
            LanguageOffset= LcdCharCodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*(uint16 *)temp*2);
            FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
            FileDev_ReadFile(Codepage, temp, 2);
            *buf= temp[0]<<8|temp[1];
            buf++;
            src8+= 2;
        }
    }
    *buf= 0;
    rkos_semaphore_give(CodePageReqSem);
#else
    src8 = (uint8 *)dat;
    while(*src8!= 0 && *(src8+1)!= 0)
    {
        if( *src8<=0x7F )
        {
            *buf= (uint16)*src8;
            buf++;
            src8++;
        }
        else
        {
            temp[0]= src8[1];
            temp[1]= src8[0];
            LanguageOffset= LcdCharCodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*(uint16 *)temp*2);
            FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
            *buf= temp[0]<<8|temp[1];
            buf++;
            src8+= 2;
        }
    }
    *buf= 0;

#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_Unicode2AnsiStr
** Input:Ucs2 *dat, void * buf,uint8 EncodeMode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
rk_err_t FW_Unicode2AnsiStr(Ucs2 *dat, void * buf,uint8 EncodeMode)
{
    uint8 temp[2], *src8;
    uint32 LanguageOffset;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);

    src8= (uint8 *)buf;
    while(*dat!= 0)
    {
        if( *dat<=0x7F )
        {
            *src8= (uint8)*dat;
            src8++;
        }
        else
        {
            LanguageOffset= U2CodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*dat*2);
            FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
            FileDev_ReadFile(Codepage, temp, 2);
            src8[0]= temp[1];
            src8[1]= temp[0];
            src8+= 2;
        }
        dat++;
    }
    src8[0]= 0;
    src8[1]= 0;
    rkos_semaphore_give(CodePageReqSem);
#else
    src8= (uint8 *)buf;
    while(*dat!= 0)
    {
        if( *dat<=0x7F )
        {
            *src8= (uint8)*dat;
            src8++;
        }
        else
        {
            LanguageOffset= U2CodePage[gSysConfig.SysLanguage]* 128 * 1024+ (*dat*2);
            FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
            src8[0]= temp[1];
            src8[1]= temp[0];
            src8+= 2;
        }
        dat++;
    }
    src8[0]= 0;
    src8[1]= 0;
#endif
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_Ansi2Unicode
** Input:uint16 dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
Ucs2 FW_Ansi2Unicode(uint16 dat, uint8 EncodeMode)
{
    Ucs2 uni= 0;
    uint8 temp[2];
    uint32 LanguageOffset= LcdCharCodePage[gSysConfig.SysLanguage]* 128 * 1024+ dat*2;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return 0xFFFF;
    }
    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);
    FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
    FileDev_ReadFile(Codepage, temp, 2);
    uni= temp[0]<<8|temp[1];
    rkos_semaphore_give(CodePageReqSem);
#else
    FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
    uni= temp[0]<<8|temp[1];
#endif
    return uni;
}

/*******************************************************************************
** Name: FW_Unicode2Ansi
** Input:uint16 dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
Ucs2 FW_Unicode2Ansi(uint16 dat, uint8 EncodeMode)
{
    Ucs2 uni= 0;
    uint8 temp[2];
    uint32 LanguageOffset= U2CodePage[gSysConfig.SysLanguage]* 128 * 1024+ dat*2;

#ifdef _SPI_BOOT_
    if(Codepage== NULL)
    {
        return 0xFFFF;
    }
    rkos_semaphore_take(CodePageReqSem, MAX_DELAY);
    FileDev_FileSeek(Codepage, SEEK_SET, LanguageOffset);
    FileDev_ReadFile(Codepage, temp, 2);
    uni= temp[0]<<8|temp[1];
    rkos_semaphore_give(CodePageReqSem);
#else
    FW_ReadFirmwaveByByte(gstFwInf.GBKLogicAddress+ LanguageOffset, (char *)temp, 2);
    uni= temp[0]<<8|temp[1];
#endif
    return uni;
}

/*******************************************************************************
** Name: FW_Resource_DeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.3.16
** Time: 17:08:56
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API void FW_Resource_DeInit(void)
{
    RKDev_Close(ghLunFW);
    RKDev_Close(ghLunDB);
}

/*******************************************************************************
** Name: FW_GetDBInf
** Input:DATABASE_INF * pstDataBaseInf
** Return: void
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:35:28
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API void FW_GetDBInf(DATABASE_INFO * pstDataBaseInf)
{
    #ifdef _EMMC_BOOT_
    pstDataBaseInf->BlockSize = EMMC_BLOCK_SIZE << 11;
    pstDataBaseInf->capacity = EMMC_LUN1_SIZE << 11;
    pstDataBaseInf->PageSize = EMMC_PAGE_SIZE;
    #endif

    #ifdef _SPI_BOOT_
    pstDataBaseInf->BlockSize = SPI_BLOCK_SIZE << 1;
    pstDataBaseInf->capacity = SPI_LUN1_SIZE << 11;
    pstDataBaseInf->PageSize = SPI_PAGE_SIZE;
    #endif
}
/*******************************************************************************
** Name: FW_ReadDataBaseBySector
** Input:uint8 * buf,  uint32 SecCnt
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 16:56:13
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_ReadDataBaseBySector(uint32 LBA, uint8 * buf,  uint32 SecCnt)
{
    return LUNReadDB(LBA, SecCnt, buf);
}

/*******************************************************************************
** Name: FW_WriteDataBaseBySector
** Input:uint8 * buf, uint32 SecCnt
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 16:54:24
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_WriteDataBaseBySector(uint32 LBA, uint8 * buf, uint32 SecCnt)
{
    return LUNWriteDB(LBA, SecCnt, buf);
}

/*******************************************************************************
** Name: FW_ReadDataBaseByByte
** Input:uint32 Addr, uint8 *pData, uint32 length
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 16:56:13
*******************************************************************************/
uint8   FlashBuf[3][512];
uint32  FlashSec[3] = {0xffffffff, 0xffffffff, 0xffffffff};

_OS_FWANALYSIS_FWANALYSIS_COMMON_
rk_err_t FW_ReadDataBaseByByte(uint32 Addr, uint8 *pData, uint32 length)
{
    uint32 sectorNum;
    uint32 ret = OK;
    uint32 sectorOffset;
    uint32 RawOffset = 0;
    uint8 bufindex = 0;
    uint32 bufMargin[3];

    sectorNum = Addr >> 9;
    sectorOffset = Addr & 511;

    while (length)
    {
        if (sectorOffset != 0 || length < 512)
        {
            uint16 i;
            uint16 count;


            if (FlashSec[0] == sectorNum)
            {
                bufindex = 0;
            }
            else if (FlashSec[1] == sectorNum)
            {
                bufindex = 1;
            }
            else if (FlashSec[3] == sectorNum)
            {
                bufindex = 2;
            }
            else
            {
                if (FlashSec[0] == 0xffffffff)
                {
                    bufindex = 0;
                }
                else if (FlashSec[1] == 0xffffffff)
                {
                    bufindex = 1;
                }
                else if (FlashSec[2] == 0xffffffff)
                {
                    bufindex = 2;
                }
                else
                {
                    if (FlashSec[0] > sectorNum)
                    {
                        bufMargin[0] = FlashSec[0] - sectorNum;
                    }
                    else
                    {
                        bufMargin[0] = sectorNum - FlashSec[0];
                    }

                    if (FlashSec[1] > sectorNum)
                    {
                        bufMargin[1] = FlashSec[1] - sectorNum;
                    }
                    else
                    {
                        bufMargin[1] = sectorNum - FlashSec[1];
                    }

                    if (FlashSec[2] > sectorNum)
                    {
                        bufMargin[2] = FlashSec[2] - sectorNum;
                    }
                    else
                    {
                        bufMargin[2] = sectorNum - FlashSec[2];
                    }


                    if ((bufMargin[0] < bufMargin[1]) && (bufMargin[0] < bufMargin[2]))
                    {
                        bufindex = 0;
                    }
                    else if ((bufMargin[1] < bufMargin[0]) && (bufMargin[1] < bufMargin[2]))
                    {
                        bufindex = 1;
                    }
                    else if ((bufMargin[2] < bufMargin[1]) && (bufMargin[2] < bufMargin[0]))
                    {
                        bufindex = 2;
                    }
                    else
                    {
                        bufindex = 0;
                    }
                }


                if (1 != FW_ReadDataBaseBySector(sectorNum,FlashBuf[bufindex],1))
                {
                    FlashSec[bufindex] = 0xffffffff;
                    return RK_ERROR;
                }
                else
                {
                    FlashSec[bufindex] = sectorNum;
                }

            }


            count = 512-sectorOffset;
            count = (count>length)? length : count;
            for (i=sectorOffset; i<count+sectorOffset; i++)
            {
                *pData++=FlashBuf[bufindex][i];
            }
            length-=count;
            sectorOffset=0;
        }
        else
        {
            if (1 != FW_ReadDataBaseBySector(sectorNum,pData,1))
            {
                return RK_ERROR;
            }
            pData +=  512;
            length -=  512;
        }

        sectorNum++;
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: FW_RemoveSegment
** Input:uint32 SegmentID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:38:57
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_RemoveSegment(uint32 SegmentID)
{
#ifndef _NOT_USE_SEGMENT_OVERLAY_

    D_SEGMENT_INFO * pDsegment, * pPrev;

    rkos_enter_critical();

    pDsegment = gpstDsegment;
    pPrev = NULL;

    while (pDsegment != NULL)
    {
        if (pDsegment->SegmentID == SegmentID)
        {

            if(pDsegment->LoadCnt > 1)
            {
                pDsegment->LoadCnt--;
                rkos_exit_critical();
                return RK_SUCCESS;
            }

            pDsegment->LoadCnt = 0;

            if (pPrev == NULL)
            {
                gpstDsegment = pDsegment->pstDsegment;
            }
            else
            {
                pPrev->pstDsegment = pDsegment->pstDsegment;
            }

            pDsegment->pstDsegment = gpstEmpDsegment;
            gpstEmpDsegment = pDsegment;
            rkos_exit_critical();
            return RK_SUCCESS;
        }

        pPrev = pDsegment;
        pDsegment = pDsegment->pstDsegment;
    }
    rkos_exit_critical();
    return  RK_SUCCESS;
#else
    return  RK_SUCCESS;
#endif

}
/*******************************************************************************
** Name: FW_LoadSegment
** Input:uint32 SegmentID, uint8 Type
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:38:00
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_LoadSegment(uint32 SegmentID, uint8 Type)
{
#ifndef _NOT_USE_SEGMENT_OVERLAY_

    uint32 CodeInfoAddr;
    uint8 * FlashBuf;
    SEGMENT_INFO_T * pSegment;
    uint32 CodeLogicAddress;
    uint32 LoadStartBase;
    rk_err_t ret;
    FIRMWARE_INFO_T * pFirmwareModuleInfo;
    uint8 *pBss;
    uint32 i;

    uint32 LoadBase;
    uint32 ImageBase;
    uint32 ImageLength;
    CodeLogicAddress = gstFwInf.CodeLogicAddress;
    LoadStartBase = gstFwInf.LoadStartBase;
    FlashBuf = rkos_memory_malloc(512);


    CodeInfoAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    CodeInfoAddr  = CodeInfoAddr + sizeof(pFirmwareModuleInfo->SegmentInfo.SegmentNum) + SegmentID * sizeof(SEGMENT_INFO_T);
    FW_ReadFirmwaveByByte(CodeInfoAddr, FlashBuf, sizeof(SEGMENT_INFO_T));


    pSegment = (SEGMENT_INFO_T *)FlashBuf;


    //rk_printf("SegmentID = %d", SegmentID);
retry:
    ret = FW_SegmentAdd(SegmentID, pSegment);

    if(ret == RK_EXIST)
    {
        rkos_memory_free(FlashBuf);
        return RK_SUCCESS;
    }
    else if(ret != RK_SUCCESS)
    {
        rkos_sleep(100);
        goto retry;
    }

    if (Type & SEGMENT_OVERLAY_CODE)
    {
        LoadBase    = pSegment->CodeLoadBase - LoadStartBase + CodeLogicAddress;
        ImageBase   = pSegment->CodeImageBase;
        ImageLength = pSegment->CodeImageLength;
        FW_ReadFirmwaveByByte(LoadBase, (uint8 *)ImageBase, ImageLength);
    }

    if (Type & SEGMENT_OVERLAY_DATA)
    {
        LoadBase    = pSegment->DataLoadBase - LoadStartBase + CodeLogicAddress;
        ImageBase   = pSegment->DataImageBase;
        ImageLength = pSegment->DataImageLength;
        FW_ReadFirmwaveByByte(LoadBase, (uint8 *)ImageBase, ImageLength);
    }

    if (Type & SEGMENT_OVERLAY_BSS)
    {
        ImageBase   =  pSegment->BssImageBase;
        ImageLength =  pSegment->BssImageLength;
        pBss = (uint8 *)ImageBase;
        for (i = 0; i < ImageLength; i++)
        {
            *pBss++ = 0;
        }
    }

    rkos_memory_free(FlashBuf);

    return RK_SUCCESS;

#else

    return RK_SUCCESS;

#endif

}


uint8   FlashBuf1[1][512];
uint32  FlashSec1[1] = {0xffffffff};
/*******************************************************************************
** Name: FW_ReadFirmwaveByByte
** Input:uint32 Addr, uint8 *pData, uint32 length
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:33:55
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_ReadFirmwaveByByte(uint32 Addr, uint8 *pData, uint32 length)
{
    uint32 sectorNum;
    uint32 ret = OK;
    uint32 sectorOffset;
    uint32 RawOffset = 0;
    uint8 bufindex = 0;
    uint32 bufMargin[3];

    sectorNum = Addr >> 9;
    sectorOffset = Addr & 511;

    while (length)
    {
        if (sectorOffset != 0 || length < 512)
        {
            uint16 i;
            uint16 count;


            if (FlashSec1[0] == sectorNum)
            {
                bufindex = 0;
            }
            else
            {
                bufindex = 0;

                if (1 != LUNReadFW(sectorNum, 1,FlashBuf1[bufindex]))
                {
                    FlashSec1[bufindex] = 0xffffffff;
                    return RK_ERROR;
                }
                else
                {
                    FlashSec1[bufindex] = sectorNum;
                }

            }


            count = 512-sectorOffset;
            count = (count>length)? length : count;
            for (i=sectorOffset; i<count+sectorOffset; i++)
            {
                *pData++=FlashBuf1[bufindex][i];
            }
            length-=count;
            sectorOffset=0;
        }
        else
        {
            if (1 != LUNReadFW(sectorNum, 1,pData))
            {
                return RK_ERROR;
            }
            pData +=  512;
            length -=  512;
        }
        sectorNum++;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FW_GetSegmentInfo
** Input:uint32 SegmentID, SEGMENT_INFO_T * Segment
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:35:04
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON API rk_err_t FW_GetSegmentInfo(uint32 SegmentID, SEGMENT_INFO_T * Segment)
{
    uint32 CodeInfoAddr;
    uint8  FlashBuf[512];
    SEGMENT_INFO_T * pSegment;
    uint32 CodeLogicAddress;
    uint32 LoadStartBase;
    rk_err_t ret;
    FIRMWARE_INFO_T * pFirmwareModuleInfo;

    CodeLogicAddress = gstFwInf.CodeLogicAddress;
    LoadStartBase = gstFwInf.LoadStartBase;


    CodeInfoAddr  = CodeLogicAddress + sizeof(pFirmwareModuleInfo -> LoadStartBase);
    CodeInfoAddr  = CodeInfoAddr + sizeof(pFirmwareModuleInfo->SegmentInfo.SegmentNum) + SegmentID * sizeof(SEGMENT_INFO_T);
    FW_ReadFirmwaveByByte(CodeInfoAddr, FlashBuf, sizeof(SEGMENT_INFO_T));

    pSegment = (SEGMENT_INFO_T *)FlashBuf;


    Segment->CodeLoadBase    = pSegment->CodeLoadBase - LoadStartBase + CodeLogicAddress;
    Segment->CodeImageBase   = pSegment->CodeImageBase;
    Segment->CodeImageLength = pSegment->CodeImageLength;

    Segment->DataLoadBase    = pSegment->DataLoadBase - LoadStartBase + CodeLogicAddress;
    Segment->DataImageBase   = pSegment->DataImageBase;
    Segment->DataImageLength = pSegment->DataImageLength;

    Segment->BssImageBase   = pSegment->BssImageBase;
    Segment->BssImageLength = pSegment->BssImageLength;

    return RK_SUCCESS;

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LUNWriteDB
** Input:uint32 offset, uint32 len, void *buf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:10:24
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON FUN rk_err_t LUNWriteDB(uint32 offset, uint32 len, void *buf)
{
    return LunDev_Write(ghLunDB,offset,buf,len);
}

/*******************************************************************************
** Name: LUNReadDB
** Input:uint32 offset, uint32 len, void *buf
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 17:03:51
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON FUN rk_err_t LUNReadDB(uint32 offset, uint32 len, void *buf)
{
    return LunDev_Read(ghLunDB,offset,buf,len);
}


/*******************************************************************************
** Name: FW_SegmentAdd
** Input:uint32 SegmentID, SEGMENT_INFO_T * pSegment
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:37:15
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON FUN rk_err_t FW_SegmentAdd(uint32 SegmentID, SEGMENT_INFO_T * pSegment)
{
    D_SEGMENT_INFO * pDsegment;

    rkos_enter_critical();

    pDsegment = gpstDsegment;

    while (pDsegment != NULL)
    {
        if(pDsegment->SegmentID == SegmentID)
        {
            pDsegment->LoadCnt++;
            rkos_exit_critical();
            return RK_EXIST;
        }

        if (((pSegment->CodeImageBase >= pDsegment->CodeImageBase) &&
                (pSegment->CodeImageBase < (pDsegment->CodeImageBase + pDsegment->CodeImageLen)))
                || ((pDsegment->CodeImageBase >= pSegment->CodeImageBase) &&
                    (pDsegment->CodeImageBase < (pSegment->CodeImageBase + pSegment->CodeImageLength))))
        {
            if((pDsegment->CodeImageLen != 0) && (pSegment->CodeImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err1 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->CodeImageBase >= pDsegment->DataImageBase) &&
                (pSegment->CodeImageBase < (pDsegment->DataImageBase + pDsegment->DataImageLen)))
                || ((pDsegment->DataImageBase >= pSegment->CodeImageBase) &&
                    (pDsegment->DataImageBase < (pSegment->CodeImageBase + pSegment->CodeImageLength))))
        {
            if((pDsegment->DataImageLen != 0) && (pSegment->CodeImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err2 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->CodeImageBase >= pDsegment->BssImageBase) &&
                (pSegment->CodeImageBase < (pDsegment->BssImageBase + pDsegment->BssImageLen)))
                || ((pDsegment->BssImageBase >= pSegment->CodeImageBase) &&
                    (pDsegment->BssImageBase < (pSegment->CodeImageBase + pSegment->CodeImageLength))))
        {
            if((pDsegment->BssImageLen != 0) && (pSegment->CodeImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err3 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->DataImageBase >= pDsegment->CodeImageBase) &&
                (pSegment->DataImageBase < (pDsegment->CodeImageBase + pDsegment->CodeImageLen)))
                || ((pDsegment->CodeImageBase >= pSegment->DataImageBase) &&
                    (pDsegment->CodeImageBase < (pSegment->DataImageBase + pSegment->DataImageLength))))
        {
            if((pDsegment->CodeImageLen != 0) && (pSegment->DataImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err4 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->DataImageBase >= pDsegment->DataImageBase) &&
                (pSegment->DataImageBase < (pDsegment->DataImageBase + pDsegment->DataImageLen)))
                || ((pDsegment->DataImageBase >= pSegment->DataImageBase) &&
                    (pDsegment->DataImageBase < (pSegment->DataImageBase + pSegment->DataImageLength))))
        {
            if((pDsegment->DataImageLen != 0) && (pSegment->DataImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err5 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->DataImageBase >= pDsegment->BssImageBase) &&
                (pSegment->DataImageBase < (pDsegment->BssImageBase + pDsegment->BssImageLen)))
                || ((pDsegment->BssImageBase >= pSegment->DataImageBase) &&
                    (pDsegment->BssImageBase < (pSegment->DataImageBase + pSegment->DataImageLength))))
        {
            if((pDsegment->BssImageLen != 0) && (pSegment->DataImageLength != 0))
            {
                rk_printf("module add err6 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                rkos_exit_critical();
                return RK_ERROR;
            }
        }

        if (((pSegment->BssImageBase >= pDsegment->CodeImageBase) &&
                (pSegment->BssImageBase < (pDsegment->CodeImageBase + pDsegment->CodeImageLen)))
                || ((pDsegment->CodeImageBase >= pSegment->BssImageBase) &&
                    (pDsegment->CodeImageBase < (pSegment->BssImageBase + pSegment->BssImageLength))))
        {
            if((pDsegment->CodeImageLen != 0) && (pSegment->BssImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err7 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->BssImageBase >= pDsegment->DataImageBase) &&
                (pSegment->BssImageBase < (pDsegment->DataImageBase + pDsegment->DataImageLen)))
                || ((pDsegment->DataImageBase >= pSegment->BssImageBase) &&
                    (pDsegment->DataImageBase < (pSegment->BssImageBase + pSegment->BssImageLength))))
        {
            if((pDsegment->DataImageLen != 0) && (pSegment->BssImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err8 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        if (((pSegment->BssImageBase >= pDsegment->BssImageBase) &&
                (pSegment->BssImageBase < (pDsegment->BssImageBase + pDsegment->BssImageLen)))
                || ((pDsegment->BssImageBase >= pSegment->BssImageBase) &&
                    (pDsegment->BssImageBase < (pSegment->BssImageBase + pSegment->BssImageLength))))
        {
            if((pDsegment->BssImageLen != 0) && (pSegment->BssImageLength != 0))
            {
                rkos_exit_critical();
                rk_printf("module add err9 src= %d, target = %d", pDsegment->SegmentID, SegmentID);
                return RK_ERROR;
            }
        }

        pDsegment = pDsegment->pstDsegment;
    }

    if (gpstEmpDsegment == NULL)
    {
        rkos_exit_critical();
        rk_printf("module too many target = %d", SegmentID);
        return RK_ERROR;
    }

    pDsegment = gpstEmpDsegment;

    gpstEmpDsegment = gpstEmpDsegment->pstDsegment;

    pDsegment->CodeImageBase = pSegment->CodeImageBase;
    pDsegment->CodeImageLen = pSegment->CodeImageLength;
    pDsegment->CodeLoadBase = pSegment->CodeLoadBase - gstFwInf.LoadStartBase + gstFwInf.CodeLogicAddress;
    pDsegment->DataImageBase = pSegment->DataImageBase;
    pDsegment->DataImageLen = pSegment->DataImageLength;
    pDsegment->DataLoadBase = pSegment->DataLoadBase - gstFwInf.LoadStartBase + gstFwInf.CodeLogicAddress;
    pDsegment->BssImageBase = pSegment->BssImageBase;
    pDsegment->BssImageLen = pSegment->BssImageLength;
    pDsegment->SegmentID = SegmentID;
    pDsegment->LoadCnt = 0;
    pDsegment->pstDsegment = gpstDsegment;
    gpstDsegment = pDsegment;
    rkos_exit_critical();
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LUNReadFW
** Input:uint32 offset, uint32 len, void *buf
** Return: uint32
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:32:43
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_COMMON_
COMMON FUN rk_err_t LUNReadFW(uint32 offset, uint32 len, void *buf)
{
    LunDev_Read(ghLunFW,offset + FwSysOffset,buf,len);
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FW_Resource_Init
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.10.28
** Time: 15:45:24
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_INIT_
INIT API void FW_Resource_Init(void)
{
    uint32 i,j=0;
    uint32 Len;
    uint32 SystemDefaultParaAddr;
    uint8  FlashBuf[1024];
    pIDSEC0 IdSec0;
    pIDSEC1 IdSec1;

    uint32 MenuLogicAddress0, MenuLogicAddress1;

    FIRMWARE_HEADER       *pFWHead;

    FIRMWARE_INF * pFW_Resourec_addr;
    FIRMWARE_INFO_T * pFirmwareModuleInfo;

    ghLunFW = RKDev_Open(DEV_CLASS_LUN,0,NOT_CARE);

    pFW_Resourec_addr =  &gstFwInf;


    ////////////////////////////////////////////////////////////////////////////
    //read resource module address.
    pFWHead = (FIRMWARE_HEADER *)FlashBuf;


    LUNReadFW(0,1, FlashBuf);

#ifdef _EMMC_BOOT_
    //EMMC_SYS_OFFSET * 512
    pFW_Resourec_addr->Font12LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT12].ModuleOffset;
    pFW_Resourec_addr->Font16LogicAddress = pFWHead->FontInfo.FontInfoTbl[FONT16].ModuleOffset;
    pFW_Resourec_addr->GBKLogicAddress  = pFWHead->CharEncInfo.CharEncInfoTbl[GBK].ModuleOffset;
    MenuLogicAddress0  = pFWHead->MenuInfo.MenuInfoTbl[0].ModuleOffset;
    MenuLogicAddress1  = pFWHead->MenuInfo.MenuInfoTbl[1].ModuleOffset;
    pFW_Resourec_addr->MenuLogicAddress  = MenuLogicAddress0; //if FM ENABLE else MenuLogicAddress1
    pFW_Resourec_addr->ImageLogicAddress = pFWHead->UiInfo.UiInfoTbl[0].ModuleOffset;
#endif

    pFW_Resourec_addr->CodeLogicAddress  =  pFWHead->CodeInfo.CodeInfoTbl[SYS_CODE].ModuleOffset;

    pFW_Resourec_addr->FirmwareYear = pFWHead->Year;
    pFW_Resourec_addr->FirmwareDate = pFWHead->Date;
    pFW_Resourec_addr->MasterVersion = pFWHead->MasterVer;
    pFW_Resourec_addr->SlaveVersion = pFWHead->SlaveVer;
    pFW_Resourec_addr->SmallVersion = pFWHead->SmallVer;

    FW_ReadFirmwaveByByte(pFW_Resourec_addr->CodeLogicAddress, FlashBuf, 512);
    pFW_Resourec_addr->LoadStartBase = *(uint32 *)FlashBuf;


    FW_ReadFirmwaveByByte(pFW_Resourec_addr->CodeLogicAddress, FlashBuf, 512);
    pFirmwareModuleInfo = (FIRMWARE_INFO_T *)FlashBuf;

    SystemDefaultParaAddr  = pFW_Resourec_addr->CodeLogicAddress + sizeof(pFirmwareModuleInfo->LoadStartBase);
    SystemDefaultParaAddr += sizeof(pFirmwareModuleInfo->SegmentInfo.SegmentNum);
    SystemDefaultParaAddr += pFirmwareModuleInfo->SegmentInfo.SegmentNum * sizeof(SEGMENT_INFO_T);
    pFW_Resourec_addr->SystemDefaultParaAddress = SystemDefaultParaAddr;

    {

        D_SEGMENT_INFO * pstEmpDsegment;
        SEGMENT_INFO_T segment;

        gpstEmpDsegment = &stDsegment[0];
        pstEmpDsegment = gpstEmpDsegment;
        pstEmpDsegment->LoadCnt = 0;

        for (i = 1; i < D_MODULE_CNT; i++)
        {
            pstEmpDsegment->pstDsegment = &stDsegment[i];
            pstEmpDsegment->LoadCnt = 0;
            pstEmpDsegment = pstEmpDsegment->pstDsegment;
        }

        pstEmpDsegment->pstDsegment = NULL;
        gpstDsegment = NULL;

        gpstDsegment = gpstEmpDsegment;
        gpstEmpDsegment = gpstEmpDsegment->pstDsegment;

        gpstDsegment->pstDsegment = NULL;

        FW_GetSegmentInfo(SEGMENT_ID_SYS, &segment);

        gpstDsegment->CodeImageBase = segment.CodeImageBase;
        gpstDsegment->CodeImageLen = segment.CodeImageLength;
        gpstDsegment->CodeLoadBase = segment.CodeLoadBase ;
        gpstDsegment->SegmentID = SEGMENT_ID_SYS;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef _USE_SHELL_
#ifdef _FW_ANLYSIS_SHELL_
_SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_DATA_
static SHELL_CMD ShellFWName[] =
{
    "inf",FWShellSn,"get fw inf","fw.inf",
    "update",FWShellUpdate,"up date fw","fw.update",
    "pcb", FWShellPcb, "get firmwave some information", "fw.pcb",
    "list", NULL, "get all segment information", "fw.list",
    "\b", NULL,"NULL","NULL",                         // the end
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: FWShell
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:HJ
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_
rk_err_t FWShell(HDC dev,  uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellFWName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellFWName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellFWName[i].CmdDes, pItem);
    if(ShellFWName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellFWName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: FWShellUpdate
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.11
** Time: 17:39:03
*******************************************************************************/
_OS_FWANALYSIS_FWANALYSIS_SHELL_
SHELL FUN rk_err_t FWShellUpdate(HDC dev,  uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    FwUpdate(L"C:\\RKNANOFW.IMG", 0);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FWShellSn
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.26
** Time: 14:36:56
*******************************************************************************/
typedef struct _SEGMENT_INFO_LIST
{
    struct _SEGMENT_INFO_LIST * pNext;
    uint32 SegMentID;
    SEGMENT_INFO_T  Segment;

} SEGMENT_INFO_LIST;

_OS_FWANALYSIS_FWANALYSIS_SHELL_
SHELL FUN rk_err_t FWShellSn(HDC dev, uint8 * pstr)
{
    uint8 buf[512];
    int32 size;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    size = FW_GetProductSn(buf);
    if(size > 0)
    {
        buf[size] = 0;
        printf("\nproduct sn: %s", buf);
    }
    else
    {
        printf("\nnot find sn");
    }

    printf("\n");

    buf[0] = 'V';
    buf[1] = 'e';
    buf[2] = 'r';
    buf[3] = ':';
    buf[4] = ' ';
    buf[5] = (uint8)((gstFwInf.MasterVersion >> 4) & 0x0f) + '0';
    buf[6] = (uint8)((gstFwInf.MasterVersion >> 0) & 0x0f) + '0';
    buf[7] = '.';
    buf[8] = (uint8)((gstFwInf.SlaveVersion >> 4) & 0x0f) + '0';
    buf[9] = (uint8)((gstFwInf.SlaveVersion >> 0) & 0x0f) + '0';
    buf[10] = '.';
    buf[11] = ((gstFwInf.SmallVersion >> 12) & 0x0f) + '0';
    buf[12] = ((gstFwInf.SmallVersion >> 8)  & 0x0f) + '0';
    buf[13] = ((gstFwInf.SmallVersion >> 4)  & 0x0f) + '0';
    buf[14] = ((gstFwInf.SmallVersion >> 0)  & 0x0f) + '0';
    buf[15] = '\n';

    buf[16] = 'D';
    buf[17] = 'a';
    buf[18] = 't';
    buf[19] = 'e';
    buf[20] = ':';

    buf[21] = (uint8)((gstFwInf.FirmwareYear >> 12) & 0x0f) + '0';
    buf[22] = (uint8)((gstFwInf.FirmwareYear >>  8) & 0x0f) + '0';
    buf[23] = (uint8)((gstFwInf.FirmwareYear >>  4) & 0x0f) + '0';
    buf[24] = (uint8)((gstFwInf.FirmwareYear >>  0) & 0x0f) + '0';

    buf[25] = '-';
    buf[26] = (uint8)((gstFwInf.FirmwareDate >> 12) & 0x0f) + '0';
    buf[27] = (uint8)((gstFwInf.FirmwareDate >>  8) & 0x0f) + '0';
    buf[28] = '-';
    buf[29] = (uint8)((gstFwInf.FirmwareDate >> 4) & 0x0f) + '0';
    buf[30] = (uint8)((gstFwInf.FirmwareDate >> 0) & 0x0f) + '0';
    buf[31] = 0;

    printf("%s", buf);

    printf("\nFirmave Size = %d", SysProgRawDiskCapacity * 2);

    #ifdef _EMMC_BOOT_
    printf("\nLun 0 Size = %d(firmware)", EMMC_LUN0_SIZE << 11);
    printf("\nLun 1 Size = %d(database)", EMMC_LUN1_SIZE << 11);
    printf("\nLun 2 Size = %d(C:)", rkos_get_storage_size() - ((EMMC_LUN0_SIZE + EMMC_LUN1_SIZE + EMMC_LUN3_SIZE + EMMC_LUN4_SIZE) << 11));
    printf("\nLun 3 Size = %d(A:)", EMMC_LUN3_SIZE << 11);
    printf("\nLun 4 Size = %d(B:)", EMMC_LUN4_SIZE << 11);
    #endif

    #ifdef _SPI_BOOT_
    printf("\nLun 0 Size = %d(firmware)", SPI_LUN0_SIZE << 11);
    printf("\nLun 1 Size = %d(database)", SPI_LUN1_SIZE << 11);
    printf("\nLun 2 Size = %d(C:)", rkos_get_storage_size() - ((SPI_LUN0_SIZE + SPI_LUN1_SIZE + SPI_LUN3_SIZE + SPI_LUN4_SIZE) << 11));
    printf("\nLun 3 Size = %d(A:)", SPI_LUN3_SIZE << 11);
    printf("\nLun 4 Size = %d(B:)", SPI_LUN4_SIZE << 11);
    #endif

    if(FW1Valid)
    {
        printf("\ncur fw is fw1");
    }
    else
    {
        printf("\ncur fw is fw2");
    }

    {
        uint8 * pbuf;
        uint32 i, j, oder;
        SEGMENT_INFO_LIST * p, *pCur, *pPrev, *pFirst;
        uint32 BootBase, BootSize, BootRealSize;
        uint32 ShellBase, ShellSize, ShellRealSize;
        uint32 AppBase, AppSize, AppRealSize;
        uint32 DriverBase, DriverSize, DriverRealSize;
        uint32 CurBase, CurSize, CurRealSize;


        pbuf = rkos_memory_malloc(MAX_SEGMENT_NUM * sizeof(SEGMENT_INFO_LIST));
        if(pbuf == NULL)
        {
            rk_printf("segment report maloc fail");
        }
        else
        {
            //search
            p = (SEGMENT_INFO_LIST *)pbuf;

            for(i = 0; i < MAX_SEGMENT_NUM; i++)
            {
                FW_GetSegmentInfo(i, &p->Segment);
                p->SegMentID = i;
                p->pNext = p + 1;
                p++;
            }

            p--;

            p->pNext = NULL;

            if(StrCmpA(pstr, "/m", 2) == 0)
            {
                oder = 1;
            }
            else if(StrCmpA(pstr, "/s", 2) == 0)
            {
                oder = 2;
            }
            else
            {
                oder = 0;
            }

            pFirst = (SEGMENT_INFO_LIST *)pbuf;

            //sort
            if(oder != 0)
            {
                for(i = (MAX_SEGMENT_NUM - 1); i > 0;  i--)
                {
                    pCur = pFirst;
                    pPrev = NULL;
                    j = 0;

                    while((pCur != NULL) && (pCur->pNext != NULL))
                    {
                        if(oder == 1)
                        {
                            if(pCur->Segment.CodeImageBase > pCur->pNext->Segment.CodeImageBase)
                            {
                                if(pPrev == NULL)
                                {
                                    pFirst = pCur->pNext;
                                    pCur->pNext = pCur->pNext->pNext;
                                    pFirst->pNext = pCur;
                                    pPrev = pFirst;
                                }
                                else
                                {
                                    pPrev->pNext = pCur->pNext;
                                    pCur->pNext = pCur->pNext->pNext;
                                    pPrev->pNext->pNext = pCur;
                                    pPrev = pPrev->pNext;
                                }

                            }
                            else
                            {
                                  pPrev = pCur;
                                  pCur = pCur->pNext;
                            }
                        }
                        else if(oder == 2)
                        {
                            if(pCur->Segment.CodeLoadBase > pCur->pNext->Segment.CodeLoadBase)
                            {
                                if(pPrev == NULL)
                                {
                                    pFirst = pCur->pNext;
                                    pCur->pNext = pCur->pNext->pNext;
                                    pFirst->pNext = pCur;
                                    pPrev = pFirst;
                                }
                                else
                                {
                                    pPrev->pNext = pCur->pNext;
                                    pCur->pNext = pCur->pNext->pNext;
                                    pPrev->pNext->pNext = pCur;
                                    pPrev = pPrev->pNext;
                                }
                            }
                            else
                            {
                                pPrev = pCur;
                                pCur = pCur->pNext;
                            }
                        }


                        j++;
                        if(j >= i)
                        {
                            break;
                        }
                    }
                }
            }

            //display
            pCur = pFirst;

            if(oder == 1)
            {
                BootBase = 0x00001000;
                BootSize = 0x0000b800;
                ShellBase = 0x0000c800;
                ShellSize = 0x00003800;
                AppBase = 0x03050000;
                AppSize = 0X00020000;
                DriverBase = 0x03070000;
                DriverSize = 0x0001C000;

                CurBase = BootBase;
                CurSize = BootSize;
                CurRealSize = 0;
            }

            do
            {
                if(oder == 1)
                {
                    if(pCur->Segment.CodeImageBase < CurBase)
                    {
                        rk_printf("this segment need check");
                    }
                    else if(pCur->Segment.CodeImageBase >= (CurBase + CurSize))
                    {
                        if(CurBase == BootBase)
                        {
                            rk_printf("boot base = %08x, boot size = %08x, boot remain size = %08x", CurBase, CurSize, CurSize - CurRealSize);
                            CurBase = ShellBase;
                            CurSize = ShellSize;
                            CurRealSize = 0;
                            if((pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) > (CurBase + CurRealSize))
                            {
                                CurRealSize = (pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) - CurBase;
                            }
                        }
                        else if(CurBase == ShellBase)
                        {
                            rk_printf("shell base = %08x, shell size = %08x, shell remain size = %08x", CurBase, CurSize, CurSize - CurRealSize);
                            CurBase = AppBase;
                            CurSize = AppSize;
                            CurRealSize = 0;
                            if((pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) > (CurBase + CurRealSize))
                            {
                                CurRealSize = (pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) - CurBase;
                            }
                        }
                        else if(CurBase == AppBase)
                        {
                            rk_printf("App base = %08x, App size = %08x, App remain size = %08x", CurBase, CurSize, CurSize - CurRealSize);
                            CurBase = DriverBase;
                            CurSize = DriverSize;
                            CurRealSize = 0;
                            if((pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) > (CurBase + CurRealSize))
                            {
                                CurRealSize = (pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) - CurBase;
                            }
                        }
                        else
                        {
                            rk_printf("this segment need check");
                        }


                    }
                    else if((pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) > (CurBase + CurSize))
                    {
                       rk_printf("this segment need check");
                    }
                    else
                    {
                        if((pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) > (CurBase + CurRealSize))
                        {
                            CurRealSize = (pCur->Segment.CodeImageBase + pCur->Segment.CodeImageLength) - CurBase;
                        }
                    }

                }

                rk_printf("%08x %08x %08x %08x %08x %08x %08x %08x %d",
                    pCur->Segment.CodeLoadBase,
                    pCur->Segment.CodeImageBase,
                    pCur->Segment.CodeImageLength,
                    pCur->Segment.DataLoadBase,
                    pCur->Segment.DataImageBase,
                    pCur->Segment.DataImageLength,
                    pCur->Segment.BssImageBase,
                    pCur->Segment.BssImageLength,
                    pCur->SegMentID);

                pCur = pCur->pNext;
            }while(pCur != NULL);

            if(oder == 1)
            {
                if(CurBase == DriverBase)
                {
                    rk_printf("Driver base = %08x, Dirver size = %08x, Driver remain size = %08x", CurBase, CurSize, CurSize - CurRealSize);
                }
            }

        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FWShellPcb
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:HJ
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_SYSTEM_FWANALYSIS_FWANALYSIS_SHELL_
rk_err_t FWShellPcb(HDC dev,  uint8 * pstr)
{

    return RK_SUCCESS;
}
#endif
#endif
#endif
