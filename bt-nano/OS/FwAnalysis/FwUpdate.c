/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: OS\FwAnalysis\FwUpdate.c
* Owner: aaron.sun
* Date: 2016.5.9
* Time: 17:38:08
* Version: 1.0
* Desc: fw update.c
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2016.5.9     17:38:08   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __OS_FWANALYSIS_FWUPDATE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "device.h"
#include "global.h"
#include "FwUpdate.h"
#include "FwAnalysis.h"
#include "LunDevice.h"
#ifdef _FS_
#include "FileDevice.h"
#endif
#include "Bsp.h"
#include "pmc.h"

#include "AudioControlTask.h"

#include "MsgDevice.h"
#include "http.h"
#include <string.h>


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//#define  _FW_CRC_

#define FW_CHECK_LEN        (8*1024)

#ifdef _SPI_BOOT_
#define FW_BUF_LEN          (FW_CHECK_LEN*8)//64K?
#else
#define FW_BUF_LEN          (FW_CHECK_LEN*1)
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _OTA_UPDATEFW_SUPPORT
#define MAX_URL_LEN 512
#define OTA_URL_HEADER "http://120.24.12.226:8800/otaUpgrade?"

#define OTA_FIRMWARE_PATH L"C:\\"
#define OTA_FIRMWARE_FILENAME L"NEWESTOTAFW.IMG";
#define OTA_FIRMWARE_FILE L"C:\\NEWESTOTAFW.IMG"

typedef enum __OTA_DOWNLOAD_STATUS
{
	OTA_DOWNLOAD_STATUS_UNKNOW = 0,
	OTA_DOWNLOAD_STATUS_SUCCESS,
	OTA_DOWNLOAD_STATUS_ERROR,
	OTA_DOWNLOAD_STATUS_SAVING,
	OTA_DOWNLOAD_STATUS_NUM
}OTA_DOWNLOAD_STATUS;

typedef struct __OTAINFOR
{
	int8 *url;
	OTA_DOWNLOAD_STATUS download_status;
	uint32 total_size;
	uint32 download_size;
	HDC hFile;
}OTAINFOR;
OTAINFOR g_OTAINFOR;

#endif//_OTA_UPDATEFW_SUPPORT
#ifdef _FW_CRC_
_OS_FWANALYSIS_FWUPDATE_COMMON_
unsigned long gTable_Crc32[256]={
    0x00000000,0x04c10db7,0x09821b6e,0x0d4316d9,
    0x130436dc,0x17c53b6b,0x1a862db2,0x1e472005,
    0x26086db8,0x22c9600f,0x2f8a76d6,0x2b4b7b61,
    0x350c5b64,0x31cd56d3,0x3c8e400a,0x384f4dbd,
    0x4c10db70,0x48d1d6c7,0x4592c01e,0x4153cda9,
    0x5f14edac,0x5bd5e01b,0x5696f6c2,0x5257fb75,
    0x6a18b6c8,0x6ed9bb7f,0x639aada6,0x675ba011,
    0x791c8014,0x7ddd8da3,0x709e9b7a,0x745f96cd,
    0x9821b6e0,0x9ce0bb57,0x91a3ad8e,0x9562a039,
    0x8b25803c,0x8fe48d8b,0x82a79b52,0x866696e5,
    0xbe29db58,0xbae8d6ef,0xb7abc036,0xb36acd81,
    0xad2ded84,0xa9ece033,0xa4aff6ea,0xa06efb5d,
    0xd4316d90,0xd0f06027,0xddb376fe,0xd9727b49,
    0xc7355b4c,0xc3f456fb,0xceb74022,0xca764d95,
    0xf2390028,0xf6f80d9f,0xfbbb1b46,0xff7a16f1,
    0xe13d36f4,0xe5fc3b43,0xe8bf2d9a,0xec7e202d,
    0x34826077,0x30436dc0,0x3d007b19,0x39c176ae,
    0x278656ab,0x23475b1c,0x2e044dc5,0x2ac54072,
    0x128a0dcf,0x164b0078,0x1b0816a1,0x1fc91b16,
    0x018e3b13,0x054f36a4,0x080c207d,0x0ccd2dca,
    0x7892bb07,0x7c53b6b0,0x7110a069,0x75d1adde,
    0x6b968ddb,0x6f57806c,0x621496b5,0x66d59b02,
    0x5e9ad6bf,0x5a5bdb08,0x5718cdd1,0x53d9c066,
    0x4d9ee063,0x495fedd4,0x441cfb0d,0x40ddf6ba,
    0xaca3d697,0xa862db20,0xa521cdf9,0xa1e0c04e,
    0xbfa7e04b,0xbb66edfc,0xb625fb25,0xb2e4f692,
    0x8aabbb2f,0x8e6ab698,0x8329a041,0x87e8adf6,
    0x99af8df3,0x9d6e8044,0x902d969d,0x94ec9b2a,
    0xe0b30de7,0xe4720050,0xe9311689,0xedf01b3e,
    0xf3b73b3b,0xf776368c,0xfa352055,0xfef42de2,
    0xc6bb605f,0xc27a6de8,0xcf397b31,0xcbf87686,
    0xd5bf5683,0xd17e5b34,0xdc3d4ded,0xd8fc405a,
    0x6904c0ee,0x6dc5cd59,0x6086db80,0x6447d637,
    0x7a00f632,0x7ec1fb85,0x7382ed5c,0x7743e0eb,
    0x4f0cad56,0x4bcda0e1,0x468eb638,0x424fbb8f,
    0x5c089b8a,0x58c9963d,0x558a80e4,0x514b8d53,
    0x25141b9e,0x21d51629,0x2c9600f0,0x28570d47,
    0x36102d42,0x32d120f5,0x3f92362c,0x3b533b9b,
    0x031c7626,0x07dd7b91,0x0a9e6d48,0x0e5f60ff,
    0x101840fa,0x14d94d4d,0x199a5b94,0x1d5b5623,
    0xf125760e,0xf5e47bb9,0xf8a76d60,0xfc6660d7,
    0xe22140d2,0xe6e04d65,0xeba35bbc,0xef62560b,
    0xd72d1bb6,0xd3ec1601,0xdeaf00d8,0xda6e0d6f,
    0xc4292d6a,0xc0e820dd,0xcdab3604,0xc96a3bb3,
    0xbd35ad7e,0xb9f4a0c9,0xb4b7b610,0xb076bba7,
    0xae319ba2,0xaaf09615,0xa7b380cc,0xa3728d7b,
    0x9b3dc0c6,0x9ffccd71,0x92bfdba8,0x967ed61f,
    0x8839f61a,0x8cf8fbad,0x81bbed74,0x857ae0c3,
    0x5d86a099,0x5947ad2e,0x5404bbf7,0x50c5b640,
    0x4e829645,0x4a439bf2,0x47008d2b,0x43c1809c,
    0x7b8ecd21,0x7f4fc096,0x720cd64f,0x76cddbf8,
    0x688afbfd,0x6c4bf64a,0x6108e093,0x65c9ed24,
    0x11967be9,0x1557765e,0x18146087,0x1cd56d30,
    0x02924d35,0x06534082,0x0b10565b,0x0fd15bec,
    0x379e1651,0x335f1be6,0x3e1c0d3f,0x3add0088,
    0x249a208d,0x205b2d3a,0x2d183be3,0x29d93654,
    0xc5a71679,0xc1661bce,0xcc250d17,0xc8e400a0,
    0xd6a320a5,0xd2622d12,0xdf213bcb,0xdbe0367c,
    0xe3af7bc1,0xe76e7676,0xea2d60af,0xeeec6d18,
    0xf0ab4d1d,0xf46a40aa,0xf9295673,0xfde85bc4,
    0x89b7cd09,0x8d76c0be,0x8035d667,0x84f4dbd0,
    0x9ab3fbd5,0x9e72f662,0x9331e0bb,0x97f0ed0c,
    0xafbfa0b1,0xab7ead06,0xa63dbbdf,0xa2fcb668,
    0xbcbb966d,0xb87a9bda,0xb5398d03,0xb1f880b4,
};

_OS_FWANALYSIS_FWUPDATE_COMMON_
unsigned long CRC_32( unsigned char * aData, unsigned long aSize )
{
    unsigned long i;
    static unsigned long nAccum = 0;

    for ( i = 0; i < aSize; i++ )
        nAccum = ( nAccum << 8 ) ^ gTable_Crc32[( nAccum >> 24 ) ^ *aData++];

    return nAccum;
}
#endif



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
** Name: FwRecovery
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.10
** Time: 10:40:25
*******************************************************************************/
_OS_FWANALYSIS_FWUPDATE_COMMON_
COMMON API rk_err_t FwRecovery(void)
{
    int32 ret = -1;
    uint32 i;
    HDC hLunFW;
    uint32 DstAddr;
    uint32 SrcAddr;
    uint32 FwSize;
    uint8 *pUBuf = (uint8 *)rkos_memory_malloc(FW_BUF_LEN);

	rk_printf("\nFwRecovery Start\n");
    if(pUBuf == NULL)
    {
        rk_printf("memory maloc fail");
        return RK_ERROR;
    }

    hLunFW = RKDev_Open(DEV_CLASS_LUN,0,NOT_CARE);


    FwSize = SysProgRawDiskCapacity<<9;

    if(0==FW2Valid)//copy fw1 to fw2 
    {
       SrcAddr =  0;
       DstAddr =  SysProgRawDiskCapacity;
    }
    else if(0==FW1Valid)//copy fw2 to fw1 
    {
       SrcAddr =  SysProgRawDiskCapacity;
       DstAddr =  0;
    }

    rk_printf("\nFwRecovery Enter: src = %d, dst = %d, size = %d",SrcAddr, DstAddr, FwSize);

    memset (pUBuf, 0xFF, FW_BUF_LEN);
    LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9);  //先清除固件头

    SrcAddr += (FW_BUF_LEN>>9);                                //从固件的FW_BUF_LEN位置开始升级
    DstAddr += (FW_BUF_LEN>>9);

    for(i = 0; i<(FwSize-FW_BUF_LEN); i+=FW_BUF_LEN)
    {
        //DEBUG("i = %d", i);
        if ((FW_BUF_LEN>>9) != LunDev_Read(hLunFW, SrcAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
        {
            rk_printf("\nread src fail1 = %d!", SrcAddr);
            while(1);
        }

        if ((FW_BUF_LEN>>9) != LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
        {
            rk_printf("\nwrite dst fail1 = %d!", DstAddr);
            while(1);
        }

        SrcAddr += (FW_BUF_LEN>>9);                                //从固件的FW_BUF_LEN位置开始升级
        DstAddr += (FW_BUF_LEN>>9);
    }

    SrcAddr -= (FwSize>>9);
    DstAddr -= (FwSize>>9);

    if ((FW_BUF_LEN>>9) != LunDev_Read(hLunFW, SrcAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
    {
        rk_printf("\nread src fai2 = %d!", SrcAddr);
        return RK_ERROR;
    }

    if ((FW_BUF_LEN>>9) != LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
    {
        rk_printf("\nwrite dst fail2 = %d!", DstAddr);
        return RK_ERROR;
    }

    RKDev_Close(hLunFW);
    rkos_memory_free(pUBuf);

    rk_printf("\nFwRecovery Exit");
	return RK_SUCCESS;
}


/*******************************************************************************
** Name: FwUpdate
** Input:uint16 * path, uint32 ForceUpate
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.10
** Time: 11:43:25
*******************************************************************************/
_OS_FWANALYSIS_FWUPDATE_COMMON_
COMMON API rk_err_t FwUpdate(uint16 * path, uint32 ForceUpate)
{
    int32 ret = RK_ERROR;
    uint8 TmpBuf[512];
    PFIRMWARE_HEADER pFWHead;
    unsigned char FwSign[8] = {'R','K','n','a','n','o','F','W'};
    uint8 *pUBuf = (uint8 *)rkos_memory_malloc(FW_BUF_LEN);
    uint32 i, addr2, FwSize, LunSize;
    #ifdef _FS_
    FILE_ATTR stFileAttr;
    #endif
    HDC hFile;
    HDC hLunFW;

    rk_printf("FwUpdate Enter");
    #ifdef _FS_
    if(pUBuf == NULL)
    {
        return RK_ERROR;
    }

    if(FwSysOffset == (FW_SYS_OFFSET + SysProgRawDiskCapacity))
    {
        rk_printf("please after restart system, retry");
        return RESTART_SYSTEM;
    }

    stFileAttr.Path = path;
    stFileAttr.FileName = NULL;

	MainTask_SetStatus(MAINTASK_SYS_UPDATE_FW, 1);
    hFile = FileDev_OpenFile(FileSysHDC, NULL, NOT_CARE, &stFileAttr);
    if ((rk_err_t)hFile <= 0)
    {
        rk_printf("FileOpen Error!");
        return RK_ERROR;
    }

    hLunFW = RKDev_Open(DEV_CLASS_LUN,0,NOT_CARE);

    if (512 != FileDev_ReadFile(hFile, TmpBuf, 512))
    {
        rk_printf("FileRead Error!");
        goto UPDATE_ERROR;
    }

    pFWHead = (PFIRMWARE_HEADER)TmpBuf;
    //DEBUG("fwSign : %s",pFWHead->FwSign);

    if (0 != memcmp(pFWHead->FwSign, FwSign, 8))
    {
        rk_printf("Fw head Error!");
        goto UPDATE_ERROR;
    }

    FwSize = ((pFWHead->FwEndOffset + 16*1024+ FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE;
    if ((FwSize + 4) != stFileAttr.FileSize)
    {
        rk_printf("FwSize Error!");
        goto UPDATE_ERROR;
    }

    FileDev_FileSeek(hFile, SEEK_SET, pFWHead->FwEndOffset);
    FileDev_ReadFile(hFile, pUBuf, 512);
    if (0 != memcmp(TmpBuf, pUBuf, 512))
    {
        rk_printf("Fw end Error!");
        goto UPDATE_ERROR;
    }

    #ifdef _FW_CRC_
    {
        uint32 crc1, crc2;
        FileDev_FileSeek(hFile, SEEK_END, 4);
        if (4 != FileDev_ReadFile(hFile, (uint8*)&crc1, 4))
        {
            rk_printf("File Read crc error!");
            goto UPDATE_ERROR;
        }

        FileDev_FileSeek(hFile, SEEK_SET, 0);
        for (i=0; i<FwSize; i+= FW_BUF_LEN)
        {
            if (FW_BUF_LEN != FileDev_ReadFile(hFile, pUBuf, FW_BUF_LEN))
            {
                rk_printf("File Read Fw error!");
                goto UPDATE_ERROR;
            }
            crc2 = CRC_32( pUBuf, FW_BUF_LEN);
        }

        if (crc1 != crc2)
        {
            rk_printf("FW Crc Error!");
            goto UPDATE_ERROR;
        }
    }
    #endif


    rk_printf("Fw2 Update Start SysProgRawDiskCapacity=%d",SysProgRawDiskCapacity);

    LunDev_GetSize(hLunFW, &LunSize);

    if((FwSize / 256 + FW_SYS_OFFSET) > LunSize)
    {
        if(!ForceUpate)
        {
            rk_printf("data maybe loss FwSize = %d, LunSize = %d", (FwSize / 256 + FW_SYS_OFFSET), LunSize);
            //return DATA_LOSS;
			goto UPDATE_ERROR;//jjjhhh 20161110
        }
    }

    addr2 = SysProgRawDiskCapacity;                     //获取第二份固件地址
    memset (pUBuf, 0xFF, FW_BUF_LEN);

    if (LunDev_Write(hLunFW, addr2 + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9) != (FW_BUF_LEN>>9))
    {
        rk_printf("Fw2 Erase Error!");
        goto UPDATE_ERROR;
    }

    if (FwSize > (SysProgRawDiskCapacity<<9))      //固件大小有变大
    {
        addr2 = FwSize>>9;                          //第二份固件新的起始地址
        rk_printf("FwSize = %d",FwSize);
    }

    FileDev_FileSeek(hFile, SEEK_SET, FW_BUF_LEN);  //从固件的FW_BUF_LEN位置开始升级

    addr2 += (FW_BUF_LEN>>9);

    for (i = 0; i < (FwSize-FW_BUF_LEN); i+= FW_BUF_LEN)
    {
        rk_printf("addr2 = %d, i = %d", addr2, i);
        if (FW_BUF_LEN != FileDev_ReadFile(hFile, pUBuf, FW_BUF_LEN))
        {
            rk_printf("Read FW Error!");
            goto UPDATE_ERROR;
        }

        if (LunDev_Write(hLunFW, addr2 + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9) != (FW_BUF_LEN>>9))
        {
            rk_printf("Fw2 write Error!");
            goto UPDATE_ERROR;
        }

        addr2 += (FW_BUF_LEN>>9);
    }

    FileDev_FileSeek(hFile,SEEK_SET, 0);                       //再写第二份固件头
    if (FW_BUF_LEN != FileDev_ReadFile(hFile, pUBuf, FW_BUF_LEN))
    {
        rk_printf("Read FW Error!");
        goto UPDATE_ERROR;
    }

    addr2 -= (FwSize>>9);

    if (LunDev_Write(hLunFW, addr2 + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9) != (FW_BUF_LEN>>9))
    {
        rk_printf("Fw2 write Error!");
        goto UPDATE_ERROR;
    }

    rk_printf("Fw2 Update End");

    ret = RK_SUCCESS;

UPDATE_ERROR:
	MainTask_SetStatus(MAINTASK_SYS_UPDATE_FW, 0);
    rkos_memory_free(pUBuf);
    FileDev_CloseFile(hFile);
    RKDev_Close(hLunFW);
    rk_printf("FwUpdate Exit");
    PmuSetSysRegister(3, (0x18CF|0xFFFF0000)); //restart Fw2
    SystemReset();
    #endif
    return ret;
}

/*******************************************************************************
** Name: FwCheck
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.5.9
** Time: 17:39:44
*******************************************************************************/
_ATTR_SYS_INIT_CODE_
COMMON API void FwCheck(void)
{
    uint8 DataBuf[1024];
    uint8 * TmpBuf1;
    uint8 * TmpBuf2;
    PFIRMWARE_HEADER pFWHead1, pFWHead2;
    unsigned char FwSign[8] = {'R','K','n','a','n','o','F','W'};
    uint32 i, addr, LunSize, Fw1Size;
    HDC hLunFW;
    pIDSEC0 IdSec0;
    pIDSEC1 IdSec1;

#ifdef _OTA_UPDATEFW_SUPPORT
	memset(&g_OTAINFOR,0x0,sizeof(OTAINFOR));//init g_OTAINFOR----jjjjhhhh20161105
#endif
	rk_printf("FwCheck start\n");
    hLunFW = RKDev_Open(DEV_CLASS_LUN,0,NOT_CARE);


    FW1Valid = FW2Valid = 0;
    pFWHead1 = (PFIRMWARE_HEADER)DataBuf;
    pFWHead2 = (PFIRMWARE_HEADER)(DataBuf + 512);
    TmpBuf1  = DataBuf;
    TmpBuf2  = DataBuf + 512;

    //check id block
    for (i=0; i<FW_IDB_NUM; i++)  //在2个BLOCK里查找ID PAGE
    {
        if (2 != LunDev_Read(hLunFW, (i*FW_IDB_SIZE), DataBuf, 2))
        {
            continue;
        }

        IdSec0 = (pIDSEC0)DataBuf;
        if (IdSec0->magic == 0xFCDC8C3B)   //0x0ff0aa55 + rc4
        {
            IdSec1=(pIDSEC1)(DataBuf+512);      //IdSec1 NOT RC4
            if (IdSec1->ChipTag == CHIP_SIGN)
            {
                //spec->PageSize=IdSec1->pageSize;
                //spec->BlockSize=IdSec1->blockSize;

                LoaderVer    = IdSec1->LoaderVer;
                UpgradeTimes = IdSec1->UpgradeTimes;

                SysProgDiskCapacity = (uint32)(IdSec1->SysProgDiskCapacity)<<11;
                SysProgRawDiskCapacity = SysProgDiskCapacity>>1;
                LunDev_GetSize(hLunFW, &LunSize);
                if ((SysProgDiskCapacity - (3 << 11)) > LunSize)
                {
                    rk_printf("\nidb error, SysProgDiskCapacity = %d, LunSize = %d", IdSec1->SysProgDiskCapacity, LunSize);
                    while(1);
                }
                break;
            }
        }
    }


    addr = FW_SYS_OFFSET;

    LunDev_Read(hLunFW, addr, TmpBuf1, 1);
    if (0 == memcmp(pFWHead1->FwSign, FwSign, 8))
    {
        LunDev_Read(hLunFW, addr+(pFWHead1->FwEndOffset>>9),TmpBuf2, 1);
        if (0 != memcmp(TmpBuf1, TmpBuf2, 512))
        {
            rk_printf("\nfw1 compare error!");
        }
        else
        {
            FW1Valid = 1;
            Fw1Size = pFWHead1->FwEndOffset;
        }
    }
    else
    {
        rk_printf("fw1 Sign error!\n");
    }

    if (1==FW1Valid)
    {
        addr += ((((pFWHead1->FwEndOffset + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9);
        if((addr - FW_SYS_OFFSET) > LunSize)
        {
            addr -= ((((pFWHead1->FwEndOffset + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9);
            addr += SysProgRawDiskCapacity;
            FW1Valid = 0;
        }
    }
    else
    {
        addr += SysProgRawDiskCapacity;
    }

    for(i = 0; (i < 32) && (addr > FW_SYS_OFFSET); i++) //查找第二份固件,固件可能变大
    {
        rk_printf("\ni = %d addr = %d, SPI_FW_OFFSET = %d", i, addr, FW_SYS_OFFSET);
        LunDev_Read(hLunFW, addr, TmpBuf2, 1);
        if (0 != memcmp(pFWHead2->FwSign, FwSign, 8))
        {
            (1==FW1Valid)? (addr += (FW_ALIGN_SIZE>>9)) : (addr -= (FW_ALIGN_SIZE>>9));
        }
        else
        {
            LunDev_Read(hLunFW, addr+(pFWHead2->FwEndOffset>>9),TmpBuf1, 1);
            if (0 != memcmp(TmpBuf2, TmpBuf1, 512))
                rk_printf("\nfw2 compare error! 0x%x", addr);
            else
            {
                FW2Valid = 1;
            }

            break;
        }

    }

    if (i >= 32 || addr <= FW_SYS_OFFSET)
    {
        rk_printf("\nNo find fw2!");
    }

    if(PmuGetSysRegister(3) == (0x18CF|0xFFFF0000))
    {
        FW1Valid = 0;
    }
    else if(PmuGetSysRegister(3) == (0x18EF|0xFFFF0000))
    {
        FW2Valid = 0;
    }

    if (0==FW1Valid && 0==FW2Valid)    //两份都错了,表示机器变砖了
    {
        rk_printf("fw1 && fw2 error!\n");
        while(1);
    }

    if (0==FW2Valid) //第二份有出错
    {
        SysProgRawDiskCapacity = ((((Fw1Size + 16*1024 + FW_ALIGN_SIZE)/FW_ALIGN_SIZE)*FW_ALIGN_SIZE)>>9); //FW_ALIGN_SIZE=64K   //以M对齐,以sec为单位
    }
    else
    {
        SysProgRawDiskCapacity = addr - FW_SYS_OFFSET;
    }

    if (SysProgDiskCapacity < (SysProgRawDiskCapacity<<1))
    {
        SysProgDiskCapacity = (SysProgRawDiskCapacity<<1);
    }

    if(FW1Valid == 1)
    {
        FwSysOffset = FW_SYS_OFFSET;
    }
    else if(FW2Valid == 1)
    {
        FwSysOffset = FW_SYS_OFFSET + SysProgRawDiskCapacity;
    }

    rk_printf("FwSysOffset = %d\nFW_SYS_OFFSET=%d\nSysProgRawDiskCapacity=%d\nSysProgRawDiskCapacity=%d\n", FwSysOffset,FW_SYS_OFFSET,SysProgRawDiskCapacity,SysProgRawDiskCapacity);//FwSysOffset以好的fw为基准。

    RKDev_Close(hLunFW);

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _OTA_UPDATEFW_SUPPORT
_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN rk_err_t OTAFwUpdate(void)
{
	if(OTA_DOWNLOAD_STATUS_SUCCESS == g_OTAINFOR.download_status)
	{
		return FwUpdate(OTA_FIRMWARE_FILE, 0);
	}
	rk_printf("ERROR,can't update ota fw now:\ng_OTAINFOR.download_status=%d\n",g_OTAINFOR.download_status);
	return RK_ERROR;
}

_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN void DownloadStatusCallback(int status, void *httppcb)
{
    if((status == TCP_RECIVER_ERR) || (status == READ_DATA_ERR))
    {
        g_OTAINFOR.download_status= OTA_DOWNLOAD_STATUS_ERROR;
		if(g_OTAINFOR.hFile)
		{
			FileDev_CloseFile(g_OTAINFOR.hFile);
			g_OTAINFOR.hFile = NULL;
		}
    }/**/
    rk_printf("http status = %d", status);
}

_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN void DownloadFirmwareData(char *buf, uint16 write_len, uint32 mlen)//mlen:leavelength
{
	int ret = 0;
	FILE_ATTR stFileAttr;

	if(OTA_DOWNLOAD_STATUS_UNKNOW == g_OTAINFOR.download_status)
	{
		stFileAttr.Path = stFileAttr.Path = OTA_FIRMWARE_PATH;
    	stFileAttr.FileName = OTA_FIRMWARE_FILENAME;

    	g_OTAINFOR.hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
		if((rk_err_t)g_OTAINFOR.hFile > 0)//delete the old file
		{
			FileDev_CloseFile(g_OTAINFOR.hFile);
			g_OTAINFOR.hFile = NULL;
			if (FileDev_DeleteFile(FileSysHDC, NULL, &stFileAttr) != RK_SUCCESS)
			{
				rk_print_string("file delete failure");
				g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_ERROR;
				return;
			}
		}

		ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);//create a new one
		if (ret != RK_SUCCESS)
		{
			rk_print_string("file create failure\n");
			g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_ERROR;
			g_OTAINFOR.hFile = NULL;
			return;
		}
		g_OTAINFOR.hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
		if ((rk_err_t)g_OTAINFOR.hFile <= 0)
		{
			rk_printf("FileOpen FILE Failed!\n");
			g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_ERROR;
			g_OTAINFOR.hFile = NULL;
			return;
		}
		g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_SAVING;
		rk_printf("\n\n   DownloadOTAFirmware start ...********************\n");
	}

	if(OTA_DOWNLOAD_STATUS_SAVING == g_OTAINFOR.download_status)
	{
		FileDev_FileSeek(g_OTAINFOR.hFile, SEEK_SET, g_OTAINFOR.download_size);
		/*ret = FileDev_FileSeek(g_OTAINFOR.hFile, SEEK_SET, g_OTAINFOR.download_size);
		if(ret < 0)
		{
			rk_printf("FileDev_FileSeek Failed!!!g_OTAINFOR.download_size=%d********************\n\n",g_OTAINFOR.download_size);
			g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_ERROR;
			FileDev_CloseFile(g_OTAINFOR.hFile);
			g_OTAINFOR.hFile = NULL;
			return;
		}*/

		ret = FileDev_WriteFile(g_OTAINFOR.hFile, buf, write_len);
		if(ret != write_len)
		{
			rk_printf("FileDev_WriteFile Failed!!!********************\n\n");
			g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_ERROR;
			FileDev_CloseFile(g_OTAINFOR.hFile);
			g_OTAINFOR.hFile = NULL;
			return;
		}
		
		g_OTAINFOR.download_size += write_len;
		rk_printf("download_size=%ld,total_size=%ld,complete percent=%d%%!",g_OTAINFOR.download_size,
			g_OTAINFOR.total_size,((g_OTAINFOR.download_size*100)/g_OTAINFOR.total_size));
		//rk_printf("\r\033[k"); 
		if((0 == mlen) && (g_OTAINFOR.total_size == g_OTAINFOR.download_size))
		{
			rk_printf("\n   DownloadOTAFirmware success ...********************\n\n");
			g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_SUCCESS;
			FileDev_CloseFile(g_OTAINFOR.hFile);
			g_OTAINFOR.hFile = NULL;
		}
	}//if(OTA_DOWNLOAD_STATUS_SAVING == g_OTAINFOR.download_status)
}

_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN rk_err_t DownLoadOTAFirmware(void)
{
	uint8 dwRet = 0;
	void *httptestpcb = NULL;
	
	//init g_OTAINFOR
	g_OTAINFOR.download_status = OTA_DOWNLOAD_STATUS_UNKNOW;

	httptestpcb = HttpPcb_New(DownloadStatusCallback, DownloadFirmwareData, FILEWRITE);
	dwRet = HttpGet_Url(httptestpcb, g_OTAINFOR.url, 0);

	if(dwRet == RK_SUCCESS)
	{
		rk_printf("\nhttp ok\n");
	}
	else
	{
		rk_printf("\nhttp error\n");
	}
	
	while((g_OTAINFOR.download_status != OTA_DOWNLOAD_STATUS_SUCCESS) &&
		   (g_OTAINFOR.download_status != OTA_DOWNLOAD_STATUS_ERROR))// wait download result
		rkos_sleep(1000);//sleep 1S
	Http_Close(httptestpcb);
	return (g_OTAINFOR.download_status == OTA_DOWNLOAD_STATUS_SUCCESS)?(RK_SUCCESS):(RK_ERROR);

}

/*{"ST":"success","URL":"http://120.24.12.226/ota/FW/xx.bin","VS":"2.0.00.26","SIZE":14804938,"COMMENT":""}*/
_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN void GetNewFirmwareUrl(char *buf, uint16 write_len, uint32 mlen)//mlen:leavelength
{	
	//int8 *g_ota_newfw_url = NULL;
	int8 *p_start = NULL;
	int8 *p_end = NULL;
	
    if(NULL == buf || (mlen))//!!!
		return;
	rk_printf("\n===============\n buf=%s  \n\n write_len=%d\n mlen=%d\n================\n\n",buf,write_len,mlen);
	
	p_start = strstr(buf,"ST");
	if(p_start && (write_len - (p_start - buf) > 3))
	{	
		/****************get ST value start*****************/
		p_start += 3;
		while(p_start && (*p_start != '"'))
			p_start++;
		if(p_start)
			p_start += 1;
		else
		{
			rk_printf("ERROR:invalid ST json result:\nresult=%s",buf);
			return;
		}

		if(write_len - (p_start - buf) > 7)
		{	
		/****************get ST value end*****************/
			if(!StrCmpA(p_start,"success",7))
			{
				/****************get URL value start*****************/
            	p_start = strstr(buf,"URL");
				if(p_start && (write_len - (p_start - buf) > 4))
				{	
					p_start += 4;
					while(p_start && (*p_start != '"'))
						p_start++;
					if(p_start)
						p_start += 1;
					else
					{
						rk_printf("ERROR:invalid URL json result:\nresult=%s",buf);
						return;
					}
					p_end = p_start;
					while(p_end && (*p_end != '"'))
						p_end++;
					if(p_end && (*p_end == '"'))//get URL success
					{
						if(g_OTAINFOR.url)
						{
							rkos_memory_free(g_OTAINFOR.url);//free the old space
							g_OTAINFOR.url = NULL;
						}
						g_OTAINFOR.url = (char *)rkos_memory_malloc((p_end - p_start) + 1);
						if(g_OTAINFOR.url)
						{
							rkos_memcpy(g_OTAINFOR.url, p_start, (p_end - p_start));
							rk_printf("SUCCESS: get URL value success!!!\g_OTAINFOR.url=%s",g_OTAINFOR.url);
							/****************get SIZE value start*****************/
			            	p_start = strstr(buf,"SIZE");
							while(p_start && ((*p_start > '9') || (*p_start < '0')))
								p_start++;
							if(p_start)
								p_end = p_start;
							else
							{
								rk_printf("ERROR:invalid SIZE json result:\nresult=%s",buf);
								return;
							}
							while(p_end && (*p_end <= '9') && (*p_end >= '0'))
								p_end++;
							*p_end = 0;
							g_OTAINFOR.total_size = atoi(p_start);
							/****************get SIZE value end*****************/
							rk_printf("SUCCESS: get SIZE value success!!!\np_start=%s,g_OTAINFOR.size=%ld",p_start,g_OTAINFOR.total_size);
							return;							
						}
						else
						{
							rk_printf("ERROR:A...A..A..rkos_memory_malloc FAILED!\nresult=%s",buf);
							return;
						}
					}
				/****************get URL value end*****************/
				}//if(p_start && (write_len - (p_start - buf) > 4))
				else
				{
					rk_printf("ERROR:invalid URL json result:\nresult=%s",buf);
					return;
				}
		
			}
			else
			{
				rk_printf("ERROR:ST is not success:\nresult=%s",buf);
				return;
			}
		}//if(write_len - (p_start - buf) > 7)
		else
		{
			rk_printf("ERROR:invalid json result:\nresult=%s",buf);
			return;
		}
	}//if(p_start && (write_len - (p_start - buf) > 3))
	else
	{
	    rk_printf("ERROR:invalid json result:\nresult=%s",buf);
	}
}


/*http://120.24.12.226:8800/otaUpgrade?CU=EL-WM1M01-001&VC=1.0.01&SN=EL00E04D000003&ID=EL0000001&RL=1*/
_OS_FWANALYSIS_FWUPDATE_COMMON_
static COMMON FUN rk_err_t CheckOTAServer(uint8 *cu,uint8 *vc,uint8 *sn,int relase)
{
	uint8 dwRet = 0;
	void *httptestpcb = NULL;
	uint8 ota_url[MAX_URL_LEN] = {0};

	if(('0' == *vc) && ('.' != *(vc+1)))//ignore '0'
		vc += 1;
	dwRet = snprintf(ota_url,MAX_URL_LEN,"%sCU=%s&VC=%s&SN=%s&ID=%s&RL=%d",
		OTA_URL_HEADER,cu,vc,sn,sn,relase);/**/

	rk_printf("\n   CheckOTAServer http start********************\nota_url=%s\n\n",ota_url);
	//rkos_sleep(2000);//sleep 2S
	//init g_OTAINFOR
	g_OTAINFOR.url = NULL;
	g_OTAINFOR.total_size = 0;
	g_OTAINFOR.download_size= 0;
	//rk_printf("\nhttp 1111111111\n");

	httptestpcb = HttpPcb_New(NULL, GetNewFirmwareUrl, FILEWRITE);//rk_printf("\nhttp 222222222\n");
	dwRet = HttpGet_Url(httptestpcb,ota_url, 0);//"http://192.168.169.5/index.asp?mac=DDDD"
	//rk_printf("\nhttp 33333333\n");

	if(dwRet == RK_SUCCESS)
	{
		rk_printf("\nhttp ok\n");
	}
	else
	{
		rk_printf("\nhttp error\n");
	}
	
	rk_printf("\n\n  http close********************\n\n");
	
	rkos_sleep(2000);//sleep 2S
	Http_Close(httptestpcb);
	return (g_OTAINFOR.url)?(RK_SUCCESS):(RK_ERROR);
}

_OS_FWANALYSIS_FWUPDATE_COMMON_
COMMON API rk_err_t CheckOTAandUpdateFw(void)//check only once
{
	rk_err_t ret = 0;
	uint8 ota_cu[64] = {0};
	uint8 ota_vc[16] = {0};
	uint8 ota_sn[128] = {0};
	uint8 ota_id[16] = {0};

	if((0 == FW1Valid) || 
		(OTA_DOWNLOAD_STATUS_ERROR == g_OTAINFOR.download_status) || 
		(OTA_DOWNLOAD_STATUS_SUCCESS== g_OTAINFOR.download_status) ||
		(gSysConfig.PlayerType != SOURCE_FROM_HTTP))
	{
		rk_printf("Already checked or FW1Valid=%d\n",FW1Valid);
		return RK_ERROR;
	}


	if(RK_SUCCESS != FW_GetOTAParameter(ota_cu,ota_vc,ota_sn,ota_id))
	{
		rk_printf("FW_GetOTAParameter falid=\n");
		return RK_ERROR;
	}
	else
		rk_printf("ota_cu:%s\nota_vc:%s\nota_sn:%s\nota_id:%s  \n",ota_cu,ota_vc,ota_sn,ota_id);

	rkos_sleep(2000);//sleep 10S
	ret = CheckOTAServer(ota_cu,ota_vc,ota_sn,1);//check server if there has new fw
	if(RK_SUCCESS == ret)
	{	
		MainTask_SetStatus(MAINTASK_APP_DLNA_PLAYER, 0);
		ret = DownLoadOTAFirmware();//download fw
		if(RK_SUCCESS == ret)
			return OTAFwUpdate();//check the fw which download and update fw
	}
	else
		rk_printf("CheckOTAServer failed\n");
	return RK_ERROR;
}



#endif // #ifdef _OTA_UPDATEFW_SUPPORT end
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
