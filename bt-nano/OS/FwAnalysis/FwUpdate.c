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
#include "FileDevice.h"
#include "Bsp.h"
#include "pmc.h"


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

	printf("\nFwRecovery Start\n");
    if(pUBuf == NULL)
    {
        printf("memory maloc fail");
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

    printf("\nFwRecovery Enter: src = %d, dst = %d, size = %d",SrcAddr, DstAddr, FwSize);

    memset (pUBuf, 0xFF, FW_BUF_LEN);
    LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9);  //先清除固件头

    SrcAddr += (FW_BUF_LEN>>9);                                //从固件的FW_BUF_LEN位置开始升级
    DstAddr += (FW_BUF_LEN>>9);

    for(i = 0; i<(FwSize-FW_BUF_LEN); i+=FW_BUF_LEN)
    {
        //DEBUG("i = %d", i);
        if ((FW_BUF_LEN>>9) != LunDev_Read(hLunFW, SrcAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
        {
            printf("\nread src fail1 = %d!", SrcAddr);
            while(1);
        }

        if ((FW_BUF_LEN>>9) != LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
        {
            printf("\nwrite dst fail1 = %d!", DstAddr);
            while(1);
        }

        SrcAddr += (FW_BUF_LEN>>9);                                //从固件的FW_BUF_LEN位置开始升级
        DstAddr += (FW_BUF_LEN>>9);
    }

    SrcAddr -= (FwSize>>9);
    DstAddr -= (FwSize>>9);

    if ((FW_BUF_LEN>>9) != LunDev_Read(hLunFW, SrcAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
    {
        printf("\nread src fai2 = %d!", SrcAddr);
        return RK_ERROR;
    }

    if ((FW_BUF_LEN>>9) != LunDev_Write(hLunFW, DstAddr + FW_SYS_OFFSET, pUBuf, FW_BUF_LEN>>9))
    {
        printf("\nwrite dst fail2 = %d!", DstAddr);
        return RK_ERROR;
    }

    RKDev_Close(hLunFW);
    rkos_memory_free(pUBuf);

    printf("\nFwRecovery Exit");
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
    FILE_ATTR stFileAttr;
    HDC hFile;
    HDC hLunFW;

    rk_printf("FwUpdate Enter");

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
            return DATA_LOSS;
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
    rkos_memory_free(pUBuf);
    FileDev_CloseFile(hFile);
    RKDev_Close(hLunFW);
    rk_printf("FwUpdate Exit");
    PmuSetSysRegister(3, (0x18CF|0xFFFF0000)); //restart Fw2
    SystemReset();

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
                    printf("\nidb error, SysProgDiskCapacity = %d, LunSize = %d", IdSec1->SysProgDiskCapacity, LunSize);
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
            printf("\nfw1 compare error!");
        }
        else
        {
            FW1Valid = 1;
            Fw1Size = pFWHead1->FwEndOffset;
        }
    }
    else
    {
        printf("fw1 Sign error!\n");
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
        printf("\ni = %d addr = %d, SPI_FW_OFFSET = %d", i, addr, FW_SYS_OFFSET);
        LunDev_Read(hLunFW, addr, TmpBuf2, 1);
        if (0 != memcmp(pFWHead2->FwSign, FwSign, 8))
        {
            (1==FW1Valid)? (addr += (FW_ALIGN_SIZE>>9)) : (addr -= (FW_ALIGN_SIZE>>9));
        }
        else
        {
            LunDev_Read(hLunFW, addr+(pFWHead2->FwEndOffset>>9),TmpBuf1, 1);
            if (0 != memcmp(TmpBuf2, TmpBuf1, 512))
                printf("\nfw2 compare error! 0x%x", addr);
            else
            {
                FW2Valid = 1;
            }

            break;
        }

    }

    if (i >= 32 || addr <= FW_SYS_OFFSET)
    {
        printf("\nNo find fw2!");
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
        printf("fw1 && fw2 error!\n");
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

    printf("FwSysOffset = %d\nFW_SYS_OFFSET=%d\nSysProgRawDiskCapacity=%d\nSysProgRawDiskCapacity=%d\n", FwSysOffset,FW_SYS_OFFSET,SysProgRawDiskCapacity,SysProgRawDiskCapacity);//FwSysOffset以好的fw为基准。

    RKDev_Close(hLunFW);

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
