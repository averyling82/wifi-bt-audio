/*
********************************************************************************************
*
*          Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Driver\LUN\LUNDevice.c
* Owner: Aaron.sun
* Date: 2014.3.6
* Time: 16:05:11
* Desc: Lun Device class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.3.6     16:05:11   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_LUN_LUNDEVICE_C__

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
typedef  struct _PAR_INF
{
    uint32 dwStartLBA;
    uint32 dwParSize;
    uint32 dwVolumeType;

}PAR_INF;



typedef  struct _LUN_DEVICE_CLASS
{
    DEVICE_CLASS stLUNDevice;

    HDC hStorage;

    rk_size_t (* pfStorgeRead)(HDC dev, uint32 LBA, uint8* buffer, uint32 len);
    rk_size_t (* pfStorgeWrite)(HDC dev, uint32 LBA, uint8* buffer, uint32 len);

    uint32 dwStartLBA;
    uint32 dwEndLBA;
    uint32 dwParNum;
    uint32 dwSupportPar;
    #ifdef _FS_
    PAR_INF stParInf[PARTION_MAX_NUM];
    #endif

}LUN_DEVICE_CLASS;


#define PBR_OFFSET 0x40

unsigned char MBR[512] =
{
    0x33, 0xC0, 0x8E, 0xD0, 0xBC, 0x00, 0x7C, 0xFB, 0x50, 0x07, 0x50, 0x1F, 0xFC, 0xBE, 0x1B, 0x7C,
    0xBF, 0x1B, 0x06, 0x50, 0x57, 0xB9, 0xE5, 0x01, 0xF3, 0xA4, 0xCB, 0xBD, 0xBE, 0x07, 0xB1, 0x04,
    0x38, 0x6E, 0x00, 0x7C, 0x09, 0x75, 0x13, 0x83, 0xC5, 0x10, 0xE2, 0xF4, 0xCD, 0x18, 0x8B, 0xF5,
    0x83, 0xC6, 0x10, 0x49, 0x74, 0x19, 0x38, 0x2C, 0x74, 0xF6, 0xA0, 0xB5, 0x07, 0xB4, 0x07, 0x8B,
    0xF0, 0xAC, 0x3C, 0x00, 0x74, 0xFC, 0xBB, 0x07, 0x00, 0xB4, 0x0E, 0xCD, 0x10, 0xEB, 0xF2, 0x88,
    0x4E, 0x10, 0xE8, 0x46, 0x00, 0x73, 0x2A, 0xFE, 0x46, 0x10, 0x80, 0x7E, 0x04, 0x0B, 0x74, 0x0B,
    0x80, 0x7E, 0x04, 0x0C, 0x74, 0x05, 0xA0, 0xB6, 0x07, 0x75, 0xD2, 0x80, 0x46, 0x02, 0x06, 0x83,
    0x46, 0x08, 0x06, 0x83, 0x56, 0x0A, 0x00, 0xE8, 0x21, 0x00, 0x73, 0x05, 0xA0, 0xB6, 0x07, 0xEB,
    0xBC, 0x81, 0x3E, 0xFE, 0x7D, 0x55, 0xAA, 0x74, 0x0B, 0x80, 0x7E, 0x10, 0x00, 0x74, 0xC8, 0xA0,
    0xB7, 0x07, 0xEB, 0xA9, 0x8B, 0xFC, 0x1E, 0x57, 0x8B, 0xF5, 0xCB, 0xBF, 0x05, 0x00, 0x8A, 0x56,
    0x00, 0xB4, 0x08, 0xCD, 0x13, 0x72, 0x23, 0x8A, 0xC1, 0x24, 0x3F, 0x98, 0x8A, 0xDE, 0x8A, 0xFC,
    0x43, 0xF7, 0xE3, 0x8B, 0xD1, 0x86, 0xD6, 0xB1, 0x06, 0xD2, 0xEE, 0x42, 0xF7, 0xE2, 0x39, 0x56,
    0x0A, 0x77, 0x23, 0x72, 0x05, 0x39, 0x46, 0x08, 0x73, 0x1C, 0xB8, 0x01, 0x02, 0xBB, 0x00, 0x7C,
    0x8B, 0x4E, 0x02, 0x8B, 0x56, 0x00, 0xCD, 0x13, 0x73, 0x51, 0x4F, 0x74, 0x4E, 0x32, 0xE4, 0x8A,
    0x56, 0x00, 0xCD, 0x13, 0xEB, 0xE4, 0x8A, 0x56, 0x00, 0x60, 0xBB, 0xAA, 0x55, 0xB4, 0x41, 0xCD,
    0x13, 0x72, 0x36, 0x81, 0xFB, 0x55, 0xAA, 0x75, 0x30, 0xF6, 0xC1, 0x01, 0x74, 0x2B, 0x61, 0x60,
    0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x76, 0x0A, 0xFF, 0x76, 0x08, 0x6A, 0x00, 0x68, 0x00, 0x7C, 0x6A,
    0x01, 0x6A, 0x10, 0xB4, 0x42, 0x8B, 0xF4, 0xCD, 0x13, 0x61, 0x61, 0x73, 0x0E, 0x4F, 0x74, 0x0B,
    0x32, 0xE4, 0x8A, 0x56, 0x00, 0xCD, 0x13, 0xEB, 0xD6, 0x61, 0xF9, 0xC3, 0x49, 0x6E, 0x76, 0x61,
    0x6C, 0x69, 0x64, 0x20, 0x70, 0x61, 0x72, 0x74, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x61,
    0x62, 0x6C, 0x65, 0x00, 0x45, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x6C, 0x6F, 0x61, 0x64, 0x69, 0x6E,
    0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74,
    0x65, 0x6D, 0x00, 0x4D, 0x69, 0x73, 0x73, 0x69, 0x6E, 0x67, 0x20, 0x6F, 0x70, 0x65, 0x72, 0x61,
    0x74, 0x69, 0x6E, 0x67, 0x20, 0x73, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x44, 0x63, 0xBC, 0xC3, 0x49, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x0C, 0xFE, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0xE2, 0x0D, 0xCF, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0xAA
};

typedef __packed struct  _PART_TABLE_INFO
{
    uint8 bState;
    uint8 bStartHead;
    uint16 wBeginSec;
    uint8 bFsType;
    uint8 bEndHead;
    uint16 wEndSec;
    uint32 dwSecAddr;
    uint32 dwSecSize;

}PART_TABLE_INFO;



#define PAR_TYPE_FAT32                  0x01
#define PAR_TYPE_XENIX_ROOT             0x02
#define PAR_TYPE_XENIX_USER             0x03
#define PAR_TYPE_FA16_LOW_32M           0x04
#define PAR_TYPE_EXTENDED               0x05
#define PAR_TYPE_FAT16                  0x06
#define PAR_TYPE_NTFS_OR_HPFS           0x07
#define PAR_TYPE_AIX                    0x08
#define PAR_TYPE_AIX_BOOT_TABLE         0x09
#define PAR_TYPE_OS2_BOOT_MANAGE        0x0A
#define PAR_TYPE_WIN95_FAT32            0x0B
#define PAR_TYPE_WIN95_FAT32_LBA        0x0C
#define PAR_TYPE_WIN95_FAT16            0x0E
#define PAR_TYPE_WIN95_EXTENDED_8G      0X0F
#define PAR_TYPE_OPUS                   0X10
#define PAR_TYPE_HIDDEN_FAT12           0X11
#define PAR_TYPE_COMPAQ_DIAGNOST        0X12
#define PAR_TYPE_HIDDEN_FAT16           0X16
#define PAR_TYPE_HIDDEN_FAT16_LOW_32G   0X14
#define PAR_TYPE_HIDDEN_NTFS_OR_HPFS    0X17
#define PAR_TYPE_AST_WINDOWS_SWAP       0X18
#define PAR_TYPE_HIDDEN_FAT32           0X1B
#define PAR_TYPE_HIDDEN_FAT32_LBA       0X1C
#define PAR_TYPE_HIDDEN_LBA_VFAT        0X1E
#define PAR_TYPE_NEC_DOS                0X24
#define PAR_TYPE_PARTITION_MAGIC        0X3C
#define PAR_TYPE_VENIX_80286            0X40
#define PAR_TYPE_PPC_PREP_BOOT          0X41
#define PAR_TYPE_SFS                    0X42
#define PAR_TYPE_QNX4X                  0X4D
#define PAR_TYPE_QNX4X_2ND_PART         0X4E
#define PAR_TYPE_QNX4X_3RD_PART         0X4F
#define PAR_TYPE_ONTRACK_DM             0X50
#define PAR_TYPE_ONTRACK_DM6_AUX        0X51
#define PAR_TYPE_CP_M                   0X52
#define PAR_TYPE_ONTRACK_DM6_AUX1       0X53
#define PAR_TYPE_ONTRACK_DM6            0X54
#define PAR_TYPE_EZ_DRIVER              0X55
#define PAR_TYPE_GOLDEN_BOW             0X56
#define PAR_TYPE_PRIAM_EDISK            0X5C
#define PAR_TYPE_SPEED_STOR             0X61
#define PAR_TYPE_GNU_HURD_OR_SYS        0X63
#define PAR_TYPE_NOVELL_NETWARE         0X64
#define PAR_TYPE_NOVELL_NETWARE1        0X65
#define PAR_TYPE_DISK_SECURE_MULT       0X70
#define PAR_TYPE_PCIX                   0X75
#define PAR_TYPE_OLD_MINIX              0X80
#define PAR_TYPE_MINIX_OR_OLD_LINUX     0X81
#define PAR_TYPE_LINUX_SWAP             0X82
#define PAR_TYPE_LINUX                  0X83
#define PAR_TYPE_OS2_HIDDEN_C           0X84
#define PAR_TYPE_LINUX_EXTENDED         0X85
#define PAR_TYPE_NTFS_VOLMUE_SET        0X86
#define PAR_TYPE_NTFS_VOLUME_SET1       0X87
#define PAR_TYPE_AMOEBA                 0X93
#define PAR_TYPE_AMOEBA_BBT             0X94
#define PAR_TYPE_IBM_THINKPAD_HIDDEN    0XA0
#define PAR_TYPE_BSD_386                0XA5
#define PAR_TYPE_OPEN_BSD               0XA6
#define PAR_TYPE_NEXTSTEP               0XA7
#define PAR_TYPE_BSDI_FS                0XB7
#define PAR_TYPE_BSDI_SWAP              0XB8
#define PAR_TYPE_SOLARIS_BOOT_PARTION   0XBE
#define PAR_TYPE_DR_NOVELL_DOS_SECURED  0XC0
#define PAR_TYPE_DRDOS_SEC              0XC1
#define PAR_TYPE_DRDOS_SEC1             0XC4
#define PAR_TYPE_DRDOS_SEC2             0XC6
#define PAR_TYPE_SYRINX                 0XC7
#define PAR_TYPE_CP_M_CTOS              0XDB
#define PAR_TYPE_DOS_ACCESS             0XE1
#define PAR_TYPE_DOS_RO                 0XE3
#define PAR_TYPE_SPEEDSTOR              0XE4
#define PAR_TYPE_BEOS_FS                0XEB
#define PAR_TYPE_SPEEDSTOR1             0XF1
#define PAR_TYPE_DOS33_2_PARTION        0XF2
#define PAR_TYPE_SPEEDSTOR2             0XF4
#define PAR_TYPE_LAN_STEP               0XF3
#define PAR_TYPE_BBT                    0XFF




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_LUN_LUNDEVICE_COMMON_ static LUN_DEVICE_CLASS * gpstLunDevInf[LUN_MAX_NUM]
= {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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
rk_err_t LunCheckIdle(HDC dev);
rk_err_t LunCheckHandler(HDC dev);
rk_err_t LunDevResume(HDC dev);
rk_err_t LunDevSuspend(HDC dev, uint32 level);
rk_err_t AddDPT(LUN_DEVICE_CLASS * pstLunDev,  PART_TABLE_INFO * pstParTableInf, uint32 VolumeType);
rk_err_t PartionInit(LUN_DEVICE_CLASS * pstLunDev, uint32 MbrLba);
rk_err_t LunShellDelete(HDC dev, uint8 * pstr);
rk_err_t LunShellTest(HDC dev, uint8 * pstr);
rk_err_t LunShellPcb(HDC dev,  uint8 * pstr);
rk_err_t LunShellCreate(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _FS_
/*******************************************************************************
** Name: LunDev_GetParTotalCnt
** Input:HDC dev, uint32 * pParTotalCnt
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.18
** Time: 11:18:45
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API rk_err_t LunDev_GetParTotalCnt(HDC dev, uint32 * pParTotalCnt)
{
    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckIdle(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    return pstLunDev->dwParNum;
}

/*******************************************************************************
** Name: LunDev_GetPar
** Input:HDC dev,  uint32 * pVolumeType, uint32 * pStartLBA, uint32 * pTotalSize
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.17
** Time: 18:12:41
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API rk_err_t LunDev_GetPar(HDC dev, uint32 ParNum, uint32 * pVolumeType, uint32 * pStartLBA, uint32 * pTotalSize)
{
    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckIdle(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    if(ParNum >= pstLunDev->dwParNum)
    {
        return RK_ERROR;
    }


    *pVolumeType = pstLunDev->stParInf[ParNum].dwVolumeType;
    *pStartLBA = pstLunDev->stParInf[ParNum].dwStartLBA;
    *pTotalSize = pstLunDev->stParInf[ParNum].dwParSize;
    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: LunDev_GetSize
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.10
** Time: 9:55:39
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API rk_err_t LunDev_GetSize(HDC dev,  uint32 * pSize)
{
    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckIdle(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    *pSize = pstLunDev->dwEndLBA - pstLunDev->dwStartLBA + 1;
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LUNDevRead
** Input:HDC dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.6
** Time: 16:08:29
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API rk_size_t LunDev_Read(HDC dev, uint32 LBA, uint8 * buffer, uint32 len)
{
    uint32 i;

    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    HDC hStorage;

    if(LunCheckIdle(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    hStorage = pstLunDev->hStorage;

    return pstLunDev->pfStorgeRead(hStorage, LBA + pstLunDev->dwStartLBA, buffer, len);
}


/*******************************************************************************
** Name: LunDevWrite
** Input:HDC dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.6
** Time: 16:08:29
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API rk_size_t LunDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    uint32 i;

    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    HDC hStorage;

    if(LunCheckIdle(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    hStorage = pstLunDev->hStorage;

    return pstLunDev->pfStorgeWrite(hStorage, LBA + pstLunDev->dwStartLBA, buffer, len);
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: LunCheckIdle
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 10:16:05
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON FUN rk_err_t LunCheckIdle(HDC dev)
{
    LUN_DEVICE_CLASS * pstLunDev =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    pstLunDev->stLUNDevice.IdleTick = 0;

    if(pstLunDev->stLUNDevice.State != DEV_STATE_WORKING)
    {
        return DeviceTask_ResumeDevice(pstLunDev->stLUNDevice.DevClassID, pstLunDev->stLUNDevice.DevID, NULL, SYNC_MODE);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LunCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 10:15:45
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON FUN rk_err_t LunCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < LUN_MAX_NUM; i++)
    {
        if(gpstLunDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: LunDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 10:12:30
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON FUN rk_err_t LunDevResume(HDC dev)
{
    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    pstLunDev->stLUNDevice.State = DEV_STATE_WORKING;

    RKDev_Resume(pstLunDev->hStorage);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: LunDevSuspend
** Input:HDC dev, uint32 level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 9:09:45
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON FUN rk_err_t LunDevSuspend(HDC dev, uint32 level)
{
    LUN_DEVICE_CLASS * pstLunDev  =  (LUN_DEVICE_CLASS *)dev;

    if(LunCheckHandler(dev) != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    if(level == DEV_STATE_IDLE1)
    {
        pstLunDev->stLUNDevice.State = DEV_STATE_IDLE1;
    }
    else if(level == DEV_SATE_IDLE2)
    {
        pstLunDev->stLUNDevice.State = DEV_SATE_IDLE2;
    }

    RKDev_Suspend(pstLunDev->hStorage);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LUNDevCreate
** Input:void
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.3.6
** Time: 16:08:29
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_COMMON_
COMMON API HDC LunDev_Create(uint32 DevID, void * arg)
{
    LUN_DEV_ARG * pstLunArg = (LUN_DEV_ARG * )arg;
    DEVICE_CLASS* pstDev;
    LUN_DEVICE_CLASS * pstLunDev;


    if (pstLunArg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    pstLunDev =  rkos_memory_malloc(sizeof(LUN_DEVICE_CLASS));
    if (pstLunDev == NULL)
    {
        return pstLunDev;
    }

    pstDev = (DEVICE_CLASS *)pstLunDev;

    pstDev->suspend = LunDevSuspend;
    pstDev->resume  = LunDevResume;

    pstDev->SuspendMode = ENABLE_MODE;

    pstDev->Idle1EventTime = 10 * PM_TIME;

    pstLunDev->hStorage = pstLunArg->hStorage;
    pstLunDev->dwStartLBA = pstLunArg->dwStartLBA;
    pstLunDev->dwEndLBA = pstLunArg->dwEndLBA;
    pstLunDev->pfStorgeRead = pstLunArg->pfStorgeRead;
    pstLunDev->pfStorgeWrite = pstLunArg->pfStorgeWrite;
    pstLunDev->dwSupportPar = pstLunArg->dwSupportPar;
    pstLunDev->dwParNum = 0;

    #ifdef _FS_
    if(pstLunDev->dwSupportPar)
    {
        if(PartionInit(pstLunDev, 0) != RK_SUCCESS)
        {
            rkos_memory_free(pstLunDev);
            rk_printf("error");
            return NULL;
        }
    }
    #endif

    gpstLunDevInf[DevID] = pstLunDev;

    return pstLunDev;

}

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
#ifdef _FS_
/*******************************************************************************
** Name: AddDPT
** Input:LUN_DEVICE_CLASS * pstLunDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.12
** Time: 18:53:34
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_INIT_
INIT FUN rk_err_t AddDPT(LUN_DEVICE_CLASS * pstLunDev,  PART_TABLE_INFO * pstParTableInf, uint32 VolumeType)
{
    uint32 i;

    for(i = 0; i < pstLunDev->dwParNum; i++)
    {
        if(pstParTableInf->dwSecAddr < pstLunDev->stParInf[i].dwStartLBA)
        {
            if((pstParTableInf->dwSecAddr + pstParTableInf->dwSecSize) > pstLunDev->stParInf[i].dwStartLBA)
            {
                break;
            }
        }
        else if(pstParTableInf->dwSecAddr > pstLunDev->stParInf[i].dwStartLBA)
        {
            if((pstLunDev->stParInf[i].dwStartLBA + pstLunDev->stParInf[i].dwParSize) > pstParTableInf->dwSecAddr)
            {
                break;
            }
        }
        else
        {
            break;
        }

    }


    if(i < pstLunDev->dwParNum)
    {
        return RK_ERROR;
    }

    pstLunDev->stParInf[pstLunDev->dwParNum].dwVolumeType = VolumeType;
    pstLunDev->stParInf[pstLunDev->dwParNum].dwStartLBA = pstParTableInf->dwSecAddr;
    pstLunDev->stParInf[pstLunDev->dwParNum].dwParSize = pstParTableInf->dwSecSize;

    pstLunDev->dwParNum++;

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: PartionInit
** Input:LUN_DEVICE_CLASS * pstLunDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.12
** Time: 11:24:27
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_INIT_
INIT FUN rk_err_t PartionInit(LUN_DEVICE_CLASS * pstLunDev, uint32 MbrLba)
{
    int index;
    PART_TABLE_INFO * pstParTableInf;
    rk_err_t ret;
    uint8 MBRBuf[512];

    ret = pstLunDev->pfStorgeRead(pstLunDev->hStorage, pstLunDev->dwStartLBA + MbrLba, MBRBuf, 1);

    if(ret != 1)
    {
        return RK_ERROR;
    }

    index = 0;

    if((MBRBuf[510] != 0x55) || (MBRBuf[511] != 0xAA))
    {
        pstParTableInf = (PART_TABLE_INFO * )(MBRBuf + 0x1BE + index*0x10);//4
        pstParTableInf->dwSecAddr = 0;
        pstParTableInf->dwSecSize = pstLunDev->dwEndLBA - pstLunDev->dwStartLBA + 1;
        AddDPT(pstLunDev, pstParTableInf, VOLUME_TYPE_RAW);
        return RK_SUCCESS;
    }

    for(index = 0;index < 4;index++)
    {
        pstParTableInf = (PART_TABLE_INFO * )(MBRBuf + 0x1BE + index*0x10);//4个分区信息

        if((pstParTableInf->bState & 0X7F) != 0)
        {
            continue;
        }

        if(pstParTableInf->bFsType == 0)
        {
            continue;
        }
        else if((pstParTableInf->bFsType == PAR_TYPE_FAT32)
            || (pstParTableInf->bFsType == PAR_TYPE_WIN95_FAT32)
            || (pstParTableInf->bFsType == PAR_TYPE_WIN95_FAT32_LBA)
            || (pstParTableInf->bFsType == PAR_TYPE_HIDDEN_FAT32)
            || (pstParTableInf->bFsType == PAR_TYPE_HIDDEN_FAT32_LBA)
            || (pstParTableInf->bFsType == PAR_TYPE_FA16_LOW_32M)
            || (pstParTableInf->bFsType == PAR_TYPE_FAT16)
            || (pstParTableInf->bFsType == PAR_TYPE_WIN95_FAT16)
            || (pstParTableInf->bFsType == PAR_TYPE_HIDDEN_FAT12)
            || (pstParTableInf->bFsType == PAR_TYPE_HIDDEN_FAT16)
            || (pstParTableInf->bFsType == PAR_TYPE_HIDDEN_FAT16_LOW_32G))
        {
            AddDPT(pstLunDev, pstParTableInf, VOLUME_TYPE_FAT);
            if(pstLunDev->dwParNum >= PARTION_MAX_NUM)
            {
                return RK_SUCCESS;
            }
        }
    }

    if(pstLunDev->dwParNum == 0)
    {
        pstParTableInf = (PART_TABLE_INFO * )(MBRBuf + 0x1BE + 0 * 0x10);//4
        pstParTableInf->dwSecAddr = 0;
        pstParTableInf->dwSecSize = pstLunDev->dwEndLBA - pstLunDev->dwStartLBA + 1;
        AddDPT(pstLunDev, pstParTableInf, VOLUME_TYPE_RAW);
    }

    return RK_SUCCESS;

}
#endif

/*******************************************************************************
** Name: LUNDev_Delete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.6
** Time: 16:08:29
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_INIT_
INIT FUN rk_err_t LunDev_Delete(uint32 DevID, void * arg)
{
    LUN_DEVICE_CLASS * pstLunDev  = gpstLunDevInf[DevID];
    LUN_DEV_ARG * pstLunArg =  (LUN_DEV_ARG *)arg;
    if(pstLunDev == NULL)
    {
        return RK_ERROR;
    }

    if(pstLunArg == NULL)
    {
        return RK_PARA_ERR;
    }

    pstLunArg->hStorage = pstLunDev->hStorage;

    rkos_memory_free(pstLunDev);

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_PAR_DEV);
    #endif

    gpstLunDevInf[DevID] = NULL;

    return RK_SUCCESS;
}


#ifdef _USE_SHELL_
#ifdef _LUN_DEV_SHELL_
_DRIVER_LUN_LUNDEVICE_SHELL_DATA_
static SHELL_CMD ShellLunName[] =
{
    "pcb",LunShellPcb,"list lcd device pcb inf","lun.pcb [lun device object id]",
    "create",LunShellCreate,"create a lun device","lun.create </emmc | /sd | /spi | /usb>",
    "delete",LunShellDelete,"delete a lun device","lun.delete",
    "test",LunShellTest,"test lun device","lun.test [lun device object id]",
    "\b",NULL,"NULL","NULL",                         // the end
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: LunShell
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_SHELL_
rk_err_t LunDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellLunName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellLunName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellLunName[i].CmdDes, pItem);
    if(ShellLunName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellLunName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: LunShellDelete
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.8.18
** Time: 9:21:10
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_SHELL_
SHELL FUN rk_err_t LunShellDelete(HDC dev, uint8 * pstr)
{
    HDC hStorage, hLun;
    LUN_DEV_ARG stLunArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    if(RKDev_Delete(DEV_CLASS_LUN, 4, NULL) != RK_SUCCESS)
    {
        rk_print_string("lun4 delete failure");
    }
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: LunShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.5
** Time: 17:41:23
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_SHELL_
SHELL FUN rk_err_t LunShellTest(HDC dev, uint8 * pstr)
{
    HDC hLun;
    uint32 TotalSec;
    uint8 * pBuf;
    int32 BufSec;
    uint32 i, j;
    rk_err_t ret;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    DevID = String2Num(pstr);

    if(DevID >= LUN_MAX_NUM)
    {
        return RK_ERROR;
    }

    rk_printf("test lun %d", DevID);

    hLun = RKDev_Open(DEV_CLASS_LUN, DevID, NOT_CARE);
    if(hLun == NULL)
    {
        rk_print_string("LUN 3 device open failure");
        return RK_SUCCESS;
    }

    for(BufSec = 127; BufSec > 0; BufSec--)
    {
        pBuf = rkos_memory_malloc(512 * BufSec);

        if(pBuf != NULL)
        {
            break;
        }
    }

    if(BufSec <= 0)
    {
        return RK_ERROR;
    }

    LunDev_GetSize(hLun, &TotalSec);

    if(TotalSec > 0x32000)
    {
        TotalSec = 0x3200;
    }// 100M

    //TotalSec = BufSec * 2;

    rk_printf("test start = %d, %d\n", TotalSec, BufSec);

    for(i = 1;  i <= ((TotalSec / BufSec) * BufSec); i += BufSec)
    {

        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];


        ret = LunDev_Write(hLun, i - 1, pBuf, BufSec);

        rk_printf("write: LBA = 0x%08x, Len = %d", i - 1, BufSec);

        if(ret != BufSec)
        {
            rk_print_string("\nLun write error");
        }
    }

    if(TotalSec % BufSec != 0)
    {
        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];



        ret = LunDev_Write(hLun, i - 1, pBuf, TotalSec % BufSec);
        rk_printf("write: LBA = 0x%08x, Len = %d", i - 1, TotalSec % BufSec);

        if(ret != TotalSec % BufSec)
        {
            rk_print_string("\nLun write error");
        }
    }

    for(i = 1;  i <= ((TotalSec / BufSec) * BufSec); i += BufSec)
    {

        ret = LunDev_Read(hLun, i - 1, pBuf, BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i - 1, BufSec);

        if(ret != BufSec)
        {
            rk_print_string("\nlun read error");
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_printf("lun data error = %d", i);
        }
        else
        {
            for(j = 2; j < BufSec * 512; j++)
            {
                if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
                {
                     rk_printf("lun data error = %d", j);
                     break;
                }
            }
        }
    }

    if(TotalSec % BufSec != 0)
    {

        ret = LunDev_Read(hLun, i - 1, pBuf, TotalSec % BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if(ret != TotalSec % BufSec)
        {
            rk_printf("lun read error = %d", i);
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_print_string("\nlun data error");
        }
        else
        {
            for(j = 2; j < (TotalSec % BufSec) * 512; j++)
            {
                if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
                {
                     rk_printf("lun data error = %d", j);
                     break;
                }
            }
        }

    }

    rk_printf("test end");

    rkos_memory_free(pBuf);
}


/*******************************************************************************
** Name: EmmcShellPcb
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_LUN_LUNDEVICE_SHELL_
rk_err_t LunShellPcb(HDC dev,  uint8 * pstr)
{
    uint32 DevID;
    LUN_DEVICE_CLASS * pstLunDev;
    uint32 i;

    // TODO:
    //add other code below:
    //...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    DevID = String2Num(pstr);

    if(DevID >= LUN_MAX_NUM)
    {
        return RK_ERROR;
    }

    pstLunDev = gpstLunDevInf[DevID];

    if(pstLunDev == NULL)
    {
        rk_printf("lun%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstLunDevInf[%d]", DevID);
    rk_printf_no_time("    .stLunDevice");
    rk_printf_no_time("        .next = %08x",pstLunDev->stLUNDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstLunDev->stLUNDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstLunDev->stLUNDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstLunDev->stLUNDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstLunDev->stLUNDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstLunDev->stLUNDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstLunDev->stLUNDevice.resume);
    rk_printf_no_time("    .hStorage = %08x",pstLunDev->hStorage);
    rk_printf_no_time("    .pfStorgeRead = %08x",pstLunDev->pfStorgeRead);
    rk_printf_no_time("    .pfStorgeWrite = %08x",pstLunDev->pfStorgeWrite);
    rk_printf_no_time("    .dwStartLBA = %08x",pstLunDev->dwStartLBA);
    rk_printf_no_time("    .dwEndLBA = %08x",pstLunDev->dwEndLBA);
    rk_printf_no_time("    .dwParNum = %d",pstLunDev->dwParNum);
    rk_printf_no_time("    .dwSupportPar = %d",pstLunDev->dwSupportPar);

    #ifdef _FS_
    for(i = 0; i < PARTION_MAX_NUM; i++)
    {
        rk_printf_no_time("    .stParInf[%d]", i);
        rk_printf_no_time("        .dwStartLBA = %08x", pstLunDev->stParInf[i].dwStartLBA);
        rk_printf_no_time("        .dwParSize = %d", pstLunDev->stParInf[i].dwParSize);
        rk_printf_no_time("        .dwVolumeType = %d", pstLunDev->stParInf[i].dwVolumeType);
    }
    #endif

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
_DRIVER_LUN_LUNDEVICE_SHELL_
rk_err_t LunShellCreate(HDC dev, uint8 * pstr)
{
    HDC hStorage, hLun;
    LUN_DEV_ARG stLunArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    if(StrCmpA(pstr, "/emmc", 5) == 0)
    {
        #ifdef __DRIVER_EMMC_EMMCDEVICE_C__
        hStorage = RKDev_Open(DEV_CLASS_EMMC,0, NOT_CARE);
        if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
        {
            rk_print_string("emmc open failure");
            return RK_SUCCESS;
        }

        ret = EmmcDev_SetArea(hStorage, EMMC_AREA_USER);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("emmc set user area failure");
            return RK_SUCCESS;
        }

        StorageSise = 0;

        ret = EmmcDev_GetAreaSize(hStorage, EMMC_AREA_USER, &StorageSise);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("emmc get user area size failure");
            return RK_SUCCESS;
        }

        #ifdef _EMMC_BOOT_
        stLunArg.dwStartLBA = (EMMC_LUN0_SIZE + EMMC_LUN1_SIZE) << 11;
        #else
        stLunArg.dwStartLBA = 0;
        #endif

        stLunArg.dwEndLBA = StorageSise - 1;
        stLunArg.hStorage = hStorage;
        stLunArg.pfStorgeRead = EmmcDev_Read;
        stLunArg.pfStorgeWrite = EmmcDev_Write;

        ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("LUN0 Create Failure");
        }
        #endif

    }
    else if(StrCmpA(pstr, "/spi", 4) == 0)
    {
       #ifdef __DRIVER_SPINOR_SPINORDEVICE_C__

        hStorage = RKDev_Open(DEV_CLASS_SPINOR,0, NOT_CARE);
        if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
        {
            rk_print_string("spi nor open failure");
            return RK_SUCCESS;
        }

        ret = SpiNorDev_GetSize(hStorage, &stLunArg.dwEndLBA);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("spi nor get size failure");
            return RK_SUCCESS;
        }

        stLunArg.dwStartLBA = 0;
        stLunArg.dwEndLBA--;
        stLunArg.hStorage = hStorage;
        stLunArg.pfStorgeRead = SpiNorDev_Read;
        stLunArg.pfStorgeWrite = SpiNorDev_Write;

        ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("LUN0 Create Failure");
        }

      #endif

    }
    else if(StrCmpA(pstr, "/sd", 3) == 0)
    {
        #ifdef __DRIVER_SD_SDDEVICE_C__

        hStorage = RKDev_Open(DEV_CLASS_SD,0, NOT_CARE);
        if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
        {
            rk_print_string("sd open failure");
            return RK_SUCCESS;
        }

        ret = SdDev_GetSize(hStorage, &stLunArg.dwEndLBA);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("sd get size failure");
            return RK_SUCCESS;
        }

        stLunArg.dwStartLBA = 0;
        stLunArg.dwEndLBA--;
        stLunArg.hStorage = hStorage;
        stLunArg.pfStorgeRead = SdDev_Read;
        stLunArg.pfStorgeWrite = SdDev_Write;

        ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("LUN0 Create Failure");
        }

      #endif

    }
    else if(StrCmpA(pstr, "/usb", 4) == 0)
    {
        #ifdef __DRIVER_USBMSC_USBMSCDEVICE_C__
        #ifdef USB_HOST
        hStorage = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
        if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
        {
            rk_print_string("lun usbmsc open failure");
            return RK_SUCCESS;
        }
        stLunArg.dwStartLBA = 0;
        stLunArg.dwEndLBA--;
        stLunArg.hStorage = hStorage;
        stLunArg.pfStorgeRead = USBMSCHost_Read;
        stLunArg.pfStorgeWrite = USBMSCHost_Write;

        ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

        if (ret != RK_SUCCESS)
        {
            rk_print_string("LUN0 Create Failure");
        }
        rk_print_string("USB LUN0 Create Success");
        #endif
        #endif
    }
    else
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;

}
#endif
#endif
#endif

