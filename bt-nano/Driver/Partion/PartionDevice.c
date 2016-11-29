/*
********************************************************************************************
*
*   Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..call\Driver\Partion\PartionDevice.c
* Owner: Aaron.sun
* Date: 2014.3.12
* Time: 11:49:36
* Desc: Partion device class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.3.12     11:49:36   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_PARTION_PARTIONDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/




typedef  struct _PAR_DEVICE_CLASS
{
    DEVICE_CLASS stParDevice;
    HDC hLun;

    uint32 ParID;
    uint32 ParStartLBA;
    uint32 ParTotalSec;
    uint32 VolumeType;

}PAR_DEVICE_CLASS;




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_ static PAR_DEVICE_CLASS * gpstParDevInf[PARTION_MAX_NUM];

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
rk_err_t ParDevCheckHandler(HDC dev);
rk_err_t ParDevResume(HDC dev, uint32 Level);
rk_err_t ParDevSuspend(HDC dev, uint32 Level);
rk_err_t ParShellHelp(HDC dev,  uint8 * pstr);
rk_err_t ParShellPcb(HDC dev, uint8 * pstr);
rk_err_t ParShellCreate(HDC dev, uint8 * pstr);
rk_err_t ParShellFormat(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ParDev_FormatFat
** Input:HDC dev, uint8 * pVolumeName
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.3
** Time: 9:46:41
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_
COMMON API rk_err_t ParDev_FormatFat(HDC dev, uint8 * pVolumeName)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;
    HDC hLun;
    rk_err_t ret;
    uint32 TotLogicSec;
    uint32 DBRAddr;
    uint16 i;
    uint8 * pBuf, * DbrBuf, * FSInfoBuf;
    uint32 TmpVal1;
    uint32 TmpVal2;
    uint32 EraseSectors;
    uint32 FAT1LBA;
    uint32 RootDirAddr, secPerClus;
    int32 BufSec;
    uint32 fat12;

    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    hLun = pstParDev->hLun;

    for (BufSec = 128; BufSec > 0; BufSec--)
    {
        pBuf = rkos_memory_malloc(512 * BufSec);

        if (pBuf != NULL)
        {
            break;
        }
    }

    if (BufSec <= 0)
    {
        return RK_ERROR;
    }

    DbrBuf = pBuf;

    FSInfoBuf = pBuf + 512;

//write DBR
    memset(DbrBuf,0,512);

    DbrBuf[0]  = 0xeb;
    DbrBuf[1]  = 0x58;//0x3e
    DbrBuf[2]  = 0x90;

    DbrBuf[3]  = 'M';
    DbrBuf[4]  = 'S';
    DbrBuf[5]  = 'D';
    DbrBuf[6]  = 'O';
    DbrBuf[7]  = 'S';
    DbrBuf[8]  = '5';
    DbrBuf[9]  = '.';
    DbrBuf[10] = '0';

    //2 0x0b-0x23:BPB
    DbrBuf[12] = 0x02;                                          //每扇区512B
    DbrBuf[16] = 0x02;                                          //FAT份数
    DbrBuf[18] = 0x02;                                          //Boot Cnt

    DbrBuf[21] = 0xf8;                                          //硬盘

    DbrBuf[24] = 63;                                            //每磁道扇区数
    DbrBuf[26] = 255;                                           //磁头数

    DbrBuf[36] = 0x80;
    DbrBuf[38] = 0x29;

    //2 0x01fe-0x01ff:signtrue
    DbrBuf[510] = 0x55;
    DbrBuf[511] = 0xaa;


    TotLogicSec = pstParDev->ParTotalSec;

    if(TotLogicSec < 2048)// 1M
    {
        DbrBuf[13] = 0;
        goto ERROR1;
    }
    else if(TotLogicSec < 8192) // 4M
    {
        fat12 = 1;
        DbrBuf[13] = 2;
    }
    else if(TotLogicSec < 16384) //8M
    {
        fat12 = 1;
        DbrBuf[13] = 4;
    }
    else if(TotLogicSec < 32680) // 16M
    {
        fat12 = 1;
        DbrBuf[13] = 8;
    }
    else if (TotLogicSec < 262144) //128M
    {
       fat12 = 0;
       DbrBuf[13] = 2;
    }
    else if (TotLogicSec < 524288) //256M
    {
       fat12 = 0;
       DbrBuf[13] = 4;
    }
    else if (TotLogicSec < 1048576) //512M
    {
        fat12 = 0;
        DbrBuf[13] = 8;
    }
    else if (TotLogicSec < 2097152) // 1G
    {
        fat12 = 0;
        DbrBuf[13] = 16;
    }
    else
    {
        DbrBuf[13] = 0;
        goto ERROR1;
    }

    if(fat12 == 1)
    {
        DbrBuf[54] = 'F';                                           //文件系统类型(8B)
        DbrBuf[55] = 'A';
        DbrBuf[56] = 'T';
        DbrBuf[57] = '1';
        DbrBuf[58] = '2';
        DbrBuf[59] = ' ';
        DbrBuf[60] = ' ';
        DbrBuf[61] = ' ';
    }
    else
    {
        DbrBuf[54] = 'F';                                           //文件系统类型(8B)
        DbrBuf[55] = 'A';
        DbrBuf[56] = 'T';
        DbrBuf[57] = '1';
        DbrBuf[58] = '6';
        DbrBuf[59] = ' ';
        DbrBuf[60] = ' ';
        DbrBuf[61] = ' ';
    }

    if (TotLogicSec > 65535)                                    //NOT 128M(256M~1G)
    {
        DbrBuf[32] = (uint8)((TotLogicSec      ) & 0x00ff);     //大扇区数
        DbrBuf[33] = (uint8)((TotLogicSec >>  8) & 0x00ff);
        DbrBuf[34] = (uint8)((TotLogicSec >> 16) & 0x00ff);
        DbrBuf[35] = (uint8)((TotLogicSec >> 24));
    }
    else
    {
        DbrBuf[19] = (uint8)(TotLogicSec & 0x00ff);             //小扇区数
        DbrBuf[20] = (uint8)((TotLogicSec >> 8) & 0x00ff);
    }


    if(fat12)
    {
        TmpVal1 = TotLogicSec;
        TmpVal2 = (TmpVal1 * 3) / (DbrBuf[13] * 512 * 2);
    }
    else
    {
        TmpVal1 = TotLogicSec;
        TmpVal2 = (TmpVal1 * 2) / (DbrBuf[13] * 512);
    }

    DbrBuf[14] = DbrBuf[13] - (TmpVal2 * 2) % DbrBuf[13];

    rk_printf("TmpVal1 = %d, TmpVal2 = %d, DbrBuf[14] = %d", TmpVal1, TmpVal2, DbrBuf[14]);

    DbrBuf[22] = (uint8)((TmpVal2      ) & 0x00ff);             //FATSz
    DbrBuf[23] = (uint8)((TmpVal2 >>  8) & 0x00ff);

    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 0, DbrBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

    //write fat table;
    EraseSectors = ((uint32)DbrBuf[23] << 8) + DbrBuf[22];

    FAT1LBA = ((uint16)DbrBuf[15] << 8) + DbrBuf[14] + pstParDev->ParStartLBA;
    secPerClus = DbrBuf[13];

    memset(pBuf, 0, 512 * BufSec);

    if(fat12 == 1)
    {
        pBuf[0] = 0xF8;
        pBuf[1] = 0xFF;
        pBuf[2] = 0xFF;
    }
    else
    {
        pBuf[0] = 0xF8;
        pBuf[1] = 0xFF;
        pBuf[2] = 0xFF;
        pBuf[3] = 0xFF;
    }

    ret = LunDev_Write(hLun, FAT1LBA, pBuf, BufSec);

    if (ret != BufSec)
    {
        goto ERROR1;
    }

    memset(pBuf, 0, 512 * BufSec);

    for (i = BufSec; i < (EraseSectors / BufSec) * BufSec; i += BufSec)
    {
        ret = LunDev_Write(hLun, FAT1LBA + i, pBuf, BufSec);
        //rk_printf("LBA = %d, cnt = %d", i, BufSec);

        if (ret != BufSec)
        {
            goto ERROR1;
        }
    }

    if (EraseSectors % BufSec != 0)
    {
        ret = LunDev_Write(hLun, FAT1LBA + i, pBuf, EraseSectors % BufSec);

        //rk_printf("LBA = %d, cnt = %d", i, EraseSectors % BufSec);

        if (ret != BufSec)
        {
            goto ERROR1;
        }
    }

    RootDirAddr = FAT1LBA + EraseSectors * 2;

    memset(FSInfoBuf, 0, 512);

    if (pVolumeName != NULL)
    {
        for (i = 0; i < 11; i++)
        {
            FSInfoBuf[i] = pVolumeName[i];
        }

        FSInfoBuf[12] = 0x08; //volume ID
    }

    ret = LunDev_Write(hLun, RootDirAddr, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

    secPerClus = ((uint32)DbrBuf[18] << 8) + DbrBuf[17];

    for (i = 1; i < secPerClus; i++)
    {
        ret = LunDev_Write(hLun, RootDirAddr + i, FSInfoBuf, 1);
        if (ret != 1)
        {
            goto ERROR1;
        }
    }

    return RK_SUCCESS;

ERROR1:
    rkos_memory_free(pBuf);
    return RK_ERROR;

}

/*******************************************************************************
** Name: ParDev_FormatFat32
** Input:HDC dev, uint8 * pVolumeName
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.21
** Time: 8:40:23
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_
COMMON API rk_err_t ParDev_FormatFat32(HDC dev, uint8 * pVolumeName)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;
    HDC hLun;
    rk_err_t ret;
    uint32 TotLogicSec;
    uint32 DBRAddr;
    uint16 i;
    uint8 * pBuf, * DbrBuf, * FSInfoBuf;
    uint32 TmpVal1;
    uint32 TmpVal2;
    uint32 EraseSectors;
    uint32 FAT1LBA;
    uint32 RootDirAddr, secPerClus;
    int32 BufSec;

    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    hLun = pstParDev->hLun;

    for (BufSec = 128; BufSec > 0; BufSec--)
    {
        pBuf = rkos_memory_malloc(512 * BufSec);

        if (pBuf != NULL)
        {
            break;
        }
    }

    if (BufSec <= 0)
    {
        return RK_ERROR;
    }

    DbrBuf = pBuf;

    FSInfoBuf = pBuf + 512;


//write DBR
    memset(DbrBuf,0,512);

    DbrBuf[0]  = 0xeb;
    DbrBuf[1]  = 0x58;//0x3e
    DbrBuf[2]  = 0x90;

    DbrBuf[3]  = 'M';
    DbrBuf[4]  = 'S';
    DbrBuf[5]  = 'D';
    DbrBuf[6]  = 'O';
    DbrBuf[7]  = 'S';
    DbrBuf[8]  = '5';
    DbrBuf[9]  = '.';
    DbrBuf[10] = '0';

    //2 0x0b-0x23:BPB
    DbrBuf[12] = 0x02;                                          //每扇区512B
    DbrBuf[14] = 0x20;                                          //保留扇区数 0x20
    DbrBuf[16] = 0x02;                                          //FAT份数

    DbrBuf[21] = 0xf8;                                          //硬盘

    DbrBuf[24] = 63;                                            //每磁道扇区数
    DbrBuf[26] = 255;                                           //磁头数

    DbrBuf[44] = 2;                                             //BPB_RootClus
    DbrBuf[48] = 1;                                             //BPB_FSInfo ,Usually 1.
    DbrBuf[50] = 6;                                             //BPB_BkBootSec , 6.
    DbrBuf[66] = 0x29;                                          //扩展引导标签,必须是0x28或0x29

    DbrBuf[82] = 'F';                                           //文件系统类型(8B)
    DbrBuf[83] = 'A';
    DbrBuf[84] = 'T';
    DbrBuf[85] = '3';
    DbrBuf[86] = '2';
    DbrBuf[87] = ' ';
    DbrBuf[88] = ' ';
    DbrBuf[89] = ' ';

    //2 0x01fe-0x01ff:signtrue
    DbrBuf[510] = 0x55;
    DbrBuf[511] = 0xaa;


    TotLogicSec = pstParDev->ParTotalSec;

    if (TotLogicSec < 532480) //260M
    {
        DbrBuf[13] = 0;
        goto ERROR1;
    }
    else if (TotLogicSec <= 16777216) //8G
    {
        DbrBuf[13] = 1;
    }
    else if (TotLogicSec <= 33554432) //16G
    {
        DbrBuf[13] = 8;
    }
    else if (TotLogicSec <= 67108864) //32G
    {
        DbrBuf[13] = 16;
    }
    else if (TotLogicSec <= 0xFFFFFFFF) //>32G
    {
        DbrBuf[13] = 32;
    }


    DbrBuf[32] = (uint8)((TotLogicSec      ) & 0x00ff);     //大扇区数
    DbrBuf[33] = (uint8)((TotLogicSec >>  8) & 0x00ff);
    DbrBuf[34] = (uint8)((TotLogicSec >> 16) & 0x00ff);
    DbrBuf[35] = (uint8)((TotLogicSec >> 24));


    TmpVal1 = TotLogicSec - DbrBuf[14];                         //总扇区-保留扇区-根目录扇区
    TmpVal2 = TmpVal1 / ((128 * DbrBuf[13]) + 2) + 7;           //1026
    TmpVal2 &= 0xFFFFFFF8;                                      //保证4KB对齐

    DbrBuf[36] = (uint8)((TmpVal2      ) & 0x00ff);             //FATSz
    DbrBuf[37] = (uint8)((TmpVal2 >>  8) & 0x00ff);
    DbrBuf[38] = (uint8)((TmpVal2 >> 16) & 0x00ff);
    DbrBuf[39] = (uint8)((TmpVal2 >> 24));

    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 0, DbrBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }


    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 6, DbrBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

//WriteFSInfo_32
    memset(FSInfoBuf,0,512);

    FSInfoBuf[0]   = 'R';
    FSInfoBuf[1]   = 'R';
    FSInfoBuf[2]   = 'a';
    FSInfoBuf[3]   = 'A';

    FSInfoBuf[484] = 'r';
    FSInfoBuf[485] = 'r';
    FSInfoBuf[486] = 'A';
    FSInfoBuf[487] = 'a';

    FSInfoBuf[488] = 0xFF;
    FSInfoBuf[489] = 0xFF;
    FSInfoBuf[490] = 0xFF;
    FSInfoBuf[491] = 0xFF;

    FSInfoBuf[492] = 02;

    FSInfoBuf[510] = 0x55;
    FSInfoBuf[511] = 0xaa;


    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 1, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }


    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 7, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

    memset(FSInfoBuf, 0, 510);


    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 2, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }


    ret = LunDev_Write(hLun, pstParDev->ParStartLBA + 8, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

    EraseSectors = ((uint32)DbrBuf[39] << 8) + DbrBuf[38];
    EraseSectors <<= 16;
    EraseSectors += ((uint32)DbrBuf[37] << 8) + DbrBuf[36];

    FAT1LBA = ((uint16)DbrBuf[15] << 8) + DbrBuf[14] + pstParDev->ParStartLBA;
    secPerClus = DbrBuf[13];

    memset(pBuf, 0, 512 * BufSec);

    pBuf[0] = 0xF8;
    pBuf[1] = 0xFF;
    pBuf[2] = 0xFF;
    pBuf[3] = 0x0F;
    pBuf[4] = 0xFF;
    pBuf[5] = 0xFF;
    pBuf[6] = 0xFF;
    pBuf[7] = 0x0F;
    pBuf[8] = 0xFF;
    pBuf[9] = 0xFF;
    pBuf[10] = 0xFF;
    pBuf[11] = 0x0F;

    ret = LunDev_Write(hLun, FAT1LBA, pBuf, BufSec);

    if (ret != BufSec)
    {
        goto ERROR1;
    }

    memset(pBuf, 0, 512 * BufSec);

    for (i = BufSec; i < (EraseSectors / BufSec) * BufSec; i += BufSec)
    {
        ret = LunDev_Write(hLun, FAT1LBA + i, pBuf, BufSec);
        //rk_printf("LBA = %d, cnt = %d", i, BufSec);

        if (ret != BufSec)
        {
            goto ERROR1;
        }
    }

    if (EraseSectors % BufSec != 0)
    {
        ret = LunDev_Write(hLun, FAT1LBA + i, pBuf, EraseSectors % BufSec);

        //rk_printf("LBA = %d, cnt = %d", i, EraseSectors % BufSec);

        if (ret != BufSec)
        {
            goto ERROR1;
        }
    }

    RootDirAddr = FAT1LBA + EraseSectors * 2;

    memset(FSInfoBuf, 0, 512);

    if (pVolumeName != NULL)
    {
        for (i = 0; i < 11; i++)
        {
            FSInfoBuf[i] = pVolumeName[i];
        }

        FSInfoBuf[12] = 0x08; //volume ID
    }

    ret = LunDev_Write(hLun, RootDirAddr, FSInfoBuf, 1);

    if (ret != 1)
    {
        goto ERROR1;
    }

    for (i = 1; i < secPerClus; i++)
    {
        ret = LunDev_Write(hLun, RootDirAddr + i, FSInfoBuf, 1);
        if (ret != 1)
        {
            goto ERROR1;
        }
    }

    return RK_SUCCESS;

ERROR1:
    rkos_memory_free(pBuf);
    return RK_ERROR;

}


/*******************************************************************************
** Name: ParDevWrite
** Input:HDC dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.12
** Time: 11:50:11
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_
COMMON API rk_err_t ParDev_Write(HDC dev, uint32 LBA, uint8 * buffer, uint32 len)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;

    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    return LunDev_Write(pstParDev->hLun, LBA + pstParDev->ParStartLBA, buffer, len);


}

/*******************************************************************************
** Name: ParDevRead
** Input:HDC dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.12
** Time: 11:50:11
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_
COMMON API rk_err_t ParDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;

    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    return LunDev_Read(pstParDev->hLun, LBA + pstParDev->ParStartLBA, buffer, len);


}

/*******************************************************************************
** Name: ParDev_GetVolumeType
** Input:HDC dev, uint32 * VolumeType
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.9
** Time: 15:32:23
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_INIT_
COMMON API rk_err_t ParDev_GetVolumeType(HDC dev, uint32 * VolumeType)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;

    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    *VolumeType = pstParDev->VolumeType;

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
** Name: ParDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 14:36:10
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_COMMON_
COMMON FUN rk_err_t ParDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < PARTION_MAX_NUM; i++)
    {
        if(gpstParDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ParDevCreate
** Input:void * Args
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.3.12
** Time: 11:50:11
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_INIT_
INIT API HDC ParDev_Create(uint32 DevID, void * arg)
{
    PAR_DEVICE_ARG * pParArg = (PAR_DEVICE_ARG *)arg;

    DEVICE_CLASS * pstDev;
    PAR_DEVICE_CLASS * pstParDev;

    if (pParArg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }
    pstParDev =  rkos_memory_malloc(sizeof(PAR_DEVICE_CLASS));
    if (pstParDev == NULL)
    {
        return pstParDev;
    }

    pstParDev->hLun = pParArg->hLun;
    pstParDev->ParStartLBA = pParArg->ParStartLBA;
    pstParDev->ParTotalSec = pParArg->ParTotalSec;
    pstParDev->VolumeType = pParArg->VolumeType;

    pstDev = (DEVICE_CLASS *)pstParDev;

    pstDev->SuspendMode = ENABLE_MODE;
    pstDev->suspend = ParDevSuspend;
    pstDev->resume  = ParDevResume;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_PAR_DEV, SEGMENT_OVERLAY_ALL);
#endif

    gpstParDevInf[DevID] = pstParDev;

    return pstDev;

}

/*******************************************************************************
** Name: ParDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.12
** Time: 11:50:11
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_INIT_
INIT API rk_err_t ParDev_Delete(uint32 DevID, void * arg)
{
    PAR_DEVICE_CLASS * pstParDev  = gpstParDevInf[DevID];
    PAR_DEVICE_ARG * pstParArg =  (PAR_DEVICE_ARG *)arg;
    if (pstParDev == NULL)
    {
        return RK_ERROR;
    }

    if (pstParArg == NULL)
    {
        return RK_PARA_ERR;
    }

    pstParArg->hLun = pstParDev->hLun;

    rkos_memory_free(pstParDev);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_PAR_DEV);
#endif

    gpstParDevInf[DevID] = NULL;

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ParDevResume
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 14:21:35
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_INIT_
INIT FUN rk_err_t ParDevResume(HDC dev, uint32 Level)
{
     PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;
     if(ParDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     RKDev_Resume(pstParDev->hLun);

     pstParDev->stParDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: ParDevSuspend
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 14:20:26
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_INIT_
INIT FUN rk_err_t ParDevSuspend(HDC dev, uint32 Level)
{
    PAR_DEVICE_CLASS * pstParDev = (PAR_DEVICE_CLASS *)dev;
    if(ParDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstParDev->stParDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstParDev->stParDevice.State = DEV_SATE_IDLE2;
    }

    RKDev_Suspend(pstParDev->hLun);

    return RK_SUCCESS;
}




#ifdef _USE_SHELL_
#ifdef _PAR_DEV_SHELL_

_DRIVER_PARTION_PARTIONDEVICE_SHELL_DATA_
static SHELL_CMD ShellParName[] =
{
    "pcb",ParShellPcb,"list pariton device control block","par.pcb [/objectid]\n",
    "create",ParShellCreate,"create a partion device","par.create [/objectid]\n",
    "delete",NULL,"delete a partion device","par.delete [/objectid]\n",
    "format",ParShellFormat,"formate a partion device","par.format <[/fat]|[/fat32]>\n",
    "\b",NULL,"NULL","NULL",                          // the end
};
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ParShell
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.12
** Time: 11:50:11
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_SHELL_
SHELL API rk_err_t ParDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellParName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellParName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellParName[i].CmdDes, pItem);
    if(ShellParName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellParName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: ParShellFormat
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.17
** Time: 15:08:49
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_SHELL_
SHELL FUN rk_err_t ParShellFormat(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t  ret;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "/fat32", 6) == 0)
    {
        HDC hPar;
        hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
        if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
        {
            rk_print_string("par0 open failure");
            return RK_SUCCESS;
        }

        ret = ParDev_FormatFat32(hPar, NULL);
        RKDev_Close(hPar);
    }
    else if(StrCmpA(pstr, "/fat", 4) == 0)
    {
        HDC hPar;
        hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
        if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
        {
            rk_print_string("par0 open failure");
            return RK_SUCCESS;
        }

        ret = ParDev_FormatFat(hPar, NULL);
        RKDev_Close(hPar);
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: ParShellPcb
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_SHELL_
rk_err_t ParShellPcb(HDC dev,  uint8 * pstr)
{
    // TODO:
    //add other code below:
    //...
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_PARTION_PARTIONDEVICE_SHELL_
rk_err_t ParShellCreate(HDC dev, uint8 * pstr)
{
    HDC hPar, hLun;
    PAR_DEVICE_ARG stCreateArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint8 buf[512];
    uint32 i;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nlun0 open failure");
        return RK_SUCCESS;
    }

    stCreateArg.hLun = hLun;
    stCreateArg.ParStartLBA = 0;

    ret = LunDev_GetSize(hLun, &stCreateArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun0 get size failure");
        return RK_SUCCESS;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\npar0 create failure");
        return RK_SUCCESS;
    }

    return RK_SUCCESS;
}
#endif
#endif
#endif

