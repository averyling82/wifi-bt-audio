/*
********************************************************************************************
*
*  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\EMMC\EmmcDevice.c
* Owner: Aaron.sun
* Date: 2014.2.24
* Time: 10:35:32
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.2.24     10:35:32   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_EMMC_EMMCDEVICE_C__

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
/* SD Specification Version */
typedef enum _CARD_SPEC_VER
{
    /* MMC Specification Version*/
    MMC_SPEC_VER_10,            //MMC Specification Version 1.0 - 1.2
    MMC_SPEC_VER_14,            //MMC Specification Version 1.4
    MMC_SPEC_VER_20,            //MMC Specification Version 2.0 - 2.2
    MMC_SPEC_VER_31,            //MMC Specification Version 3.1 - 3.2 - 3.31
    MMC_SPEC_VER_40,            //MMC Specification Version 4.0
    SPEC_VER_MAX

}CARD_SPEC_VER;


typedef  struct _EMMC_DEVICE_CLASS
{

    DEVICE_CLASS stEmmcDevice;
    pSemaphore osEmmcOperReqSem;

    uint32           type;             //Card type
    uint16           rca;              //Relative Card Address
    uint32           TranSpeed;       //卡的最大数据传输速度，也就是卡的最大工作频率，单位KHz
    uint32           capability;       //Card capability,单位block数，每个block为512字节
    uint32           BootSize;         //boot partition size,单位sector(512B)
    bool             WriteProt;        //Mechanical write Protect switch state,
    bool             bPassword;        //用于指示卡是否是有密码的卡, TRUE:have password, FALSE:no password
    CARD_SPEC_VER    SpecVer;          //SD Specification Version
    uint16           EraseSize;        //this size is given as minimum number of write blocks that can be erased in a single erase command
    HDC              hSDC;
    uint32           GpioID;
    eGPIOPinNum_t    DevResetGPIO;
    BUS_WIDTH        BusWidth;

}EMMC_DEVICE_CLASS;



/* Card type */
#define UNKNOW_CARD             (0)         //无法识别或不支持的卡，不可用
#define SDIO                    (0x1 << 1)
#define SDHC                    (0x1 << 2)  //Ver2.00 High Capacity SD Memory Card
#define SD20                    (0x1 << 3)  //Ver2.00 Standard Capacity SD Memory Card
#define SD1X                    (0x1 << 4)  //Ver1.xx Standard Capacity SD Memory Card
#define MMC4                    (0x1 << 5)  //Ver4.xx MMC
#define MMC                     (0x1 << 6)  //Ver3.xx MMC
#define eMMC2G                  (0x1 << 7)  //Ver4.2 larter eMMC and densities greater than 2GB
#define SDXC                    (0x1 << 8)
#define UHS1                    (0x1 << 9)


#define EMMC_COMMON_QUEUE_LEN    16
#define EMMC_READ_QUEUE_LEN     16

#define ACCESS_NO_BOOT 0x0   //No access to boot partition (default)
#define ACCESS_BOOT1   0x1   //R/W boot partition 1
#define ACCESS_BOOT2   0x2   //R/W boot partition 2

#define NO_BOOT_EN     (0<<3) //Device not boot enabled (default)
#define BOOT1_EN       (1<<3) //Boot partition 1 enabled for boot
#define BOOT2_EN       (2<<3) //Boot partition 2 enabled for boot

#define NO_BOOT_ACK    (0<<6) //No boot acknowledge sent (default)
#define BOOT_ACK       (1<<6) //Boot acknowledge sent during boot operation
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static EMMC_DEVICE_CLASS * gpstEmmcDevInf[SDC_MAX];


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
rk_err_t EmmcDevCheckHandler(HDC dev);
rk_err_t EmmcShellHelp(HDC dev,  uint8 * pstr);
rk_err_t EmmcShellTest(HDC dev, uint8 * pstr);
rk_err_t EmmcShellCreate(HDC dev, uint8 * pstr);
rk_err_t EmmcShellPcb(HDC dev, uint8 * pstr);
rk_err_t EmmcSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf);
rk_err_t MMCIdentifyCard(EMMC_DEVICE_CLASS * pEmmcDev);
rk_err_t MMCDecodeCSD(EMMC_DEVICE_CLASS * pEmmcDev, uint32 * pCSD);
uint16 MMCGenerateRCA(EMMC_DEVICE_CLASS * pEmmcDev);
rk_err_t MMCSwitchFunction(EMMC_DEVICE_CLASS * pEmmcDev);
rk_err_t MMCInit(EMMC_DEVICE_CLASS * pEmmcDev);
rk_err_t MMC_PartitionConfig(EMMC_DEVICE_CLASS * pEmmcDev, uint8 value);
rk_err_t EmmcDevResume(HDC dev);
rk_err_t EmmcDevSuspend(HDC dev, uint32 Level);
rk_err_t EmmcDevDeInit(EMMC_DEVICE_CLASS * pstEmmcDev);
rk_err_t EmmcDevInit(EMMC_DEVICE_CLASS * pEmmcDev);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: EmmcDev_GetAreaSize
** Input:HDC dev, uint32 Area, uint32 * pSize
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.17
** Time: 10:49:22
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
COMMON API rk_err_t EmmcDev_GetAreaSize(HDC dev, uint32 Area, uint32 * pSize)
{
     EMMC_DEVICE_CLASS * pstEmmcDev =  (EMMC_DEVICE_CLASS *)(dev);

     if(pstEmmcDev == NULL)
     {
        return RK_ERROR;
     }

     if(Area == EMMC_AREA_USER)
     {
          *pSize = pstEmmcDev->capability;
     }
     else if(Area == EMMC_AREA_BOOT1)
     {
          *pSize = pstEmmcDev->BootSize;
     }
     else if(Area == EMMC_AREA_BOOT2)
     {
          *pSize = pstEmmcDev->BootSize;
     }
     return RK_SUCCESS;

}

/*******************************************************************************
** Name: EmmcDev_SetArea
** Input:HDC dev, uint32 Area
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.17
** Time: 10:47:21
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
COMMON API rk_err_t EmmcDev_SetArea(HDC dev, uint32 Area)
{
     EMMC_DEVICE_CLASS * pstEmmcDev =  (EMMC_DEVICE_CLASS *)(dev);

     if(pstEmmcDev == NULL)
     {
        return RK_ERROR;
     }

     rk_err_t ret;

     if(Area == EMMC_AREA_BOOT1)
     {
         ret = MMC_PartitionConfig(pstEmmcDev, NO_BOOT_ACK|BOOT1_EN|ACCESS_BOOT1);
     }
     else if(Area == EMMC_AREA_BOOT2)
     {
         ret = MMC_PartitionConfig(pstEmmcDev, NO_BOOT_ACK|BOOT1_EN|ACCESS_BOOT2);
     }
     else if(Area == EMMC_AREA_USER)
     {
         ret = MMC_PartitionConfig(pstEmmcDev, NO_BOOT_ACK|BOOT1_EN|ACCESS_NO_BOOT);
     }

     return ret;

}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: EmmcDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 14:55:48
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
COMMON FUN rk_err_t EmmcDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < SDC_MAX; i++)
    {
        if(gpstEmmcDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: EmmcSendCmd
** Input: uint32  cmd,uint32  CmdArg,uint32 *RespBuf
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 16:55:28
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
rk_err_t EmmcSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf)
{
    SDC_CMD_ARG DevReqArg;
    DevReqArg.Cmd = cmd;
    DevReqArg.CmdArg = CmdArg;
    DevReqArg.RespBuf = RespBuf;
    return SdcDev_SendCmd(hSdc, &DevReqArg);
}


/*******************************************************************************
** Name: EmmcDevRead
** Input:HDC dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
rk_size_t EmmcDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //uint32 i;

    EMMC_DEVICE_CLASS * pstEmmcDev =  (EMMC_DEVICE_CLASS *)(dev);
    HDC hSdc = pstEmmcDev->hSDC;

    uint32 BlkCnt,status;
    rk_err_t ret;
    SDC_CMD_ARG stArg;

    if(len == 0)
    {
        ret = RK_PARA_ERR;
    }

    rkos_semaphore_take(pstEmmcDev->osEmmcOperReqSem, MAX_DELAY);

    if (1 == len)
    {
        stArg.Cmd = (SD_READ_SINGLE_BLOCK | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 512;
        stArg.CmdArg = LBA;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc, &stArg);
        if(ret == len * 512)
        {
            ret = len;
        }
        else
        {
            ret = RK_ERROR;
        }

    }
    else
    {

        stArg.Cmd = (SD_READ_MULTIPLE_BLOCK | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 512;
        stArg.CmdArg = LBA;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc, &stArg);
        if(ret == len * 512)
        {
            ret = len;
        }
        else
        {
            ret = RK_ERROR;
        }

        EmmcSendCmd(hSdc,
                        (SD_STOP_TRANSMISSION | SD_NODATA_OP | SD_RSP_R1B | STOP_CMD | NO_WAIT_PREV),
                        0,
                        &status);


    }

    rkos_semaphore_give(pstEmmcDev->osEmmcOperReqSem);

    return ret;
}

/*******************************************************************************
** Name: EmmcDevWrite
** Input:HDC dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_COMMON_
rk_size_t EmmcDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //uint32 i;
    EMMC_DEVICE_CLASS * pstEmmcDev =  (EMMC_DEVICE_CLASS *)(dev);
    HDC hSdc = pstEmmcDev->hSDC;

    uint32 BlkCnt,status;
    rk_err_t ret;
    SDC_CMD_ARG stArg;

    if(len == 0)
    {
        ret = RK_PARA_ERR;
    }

    rkos_semaphore_take(pstEmmcDev->osEmmcOperReqSem, MAX_DELAY);

    if (1 == len)
    {
        stArg.Cmd = (SD_WRITE_BLOCK | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 512;
        stArg.CmdArg = LBA;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Write(hSdc, &stArg);
        if(ret == len * 512)
        {
            ret = len;
        }
        else
        {
            ret = RK_ERROR;
        }

        SdcDev_DataTransIdle(hSdc);
    }
    else
    {
        stArg.Cmd = (SD_WRITE_MULTIPLE_BLOCK | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 512;
        stArg.CmdArg = LBA;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Write(hSdc, &stArg);
        if(ret == len * 512)
        {
            ret = len;
        }
        else
        {
            ret = RK_ERROR;
        }

        EmmcSendCmd(hSdc,
                        (SD_STOP_TRANSMISSION | SD_NODATA_OP | SD_RSP_R1B | STOP_CMD | NO_WAIT_PREV),
                        0,
                        &status);

        SdcDev_DataTransIdle(hSdc);

    }

     rkos_semaphore_give(pstEmmcDev->osEmmcOperReqSem);

    return ret;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: EmmcDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t EmmcDevDelete(uint32 DevID, void * arg)
{
    EMMC_DEVICE_CLASS * pstEmmcDev = gpstEmmcDevInf[DevID];
    EMMC_DEV_ARG * pstEmmcArg = (EMMC_DEV_ARG *)arg;

    if((pstEmmcDev == NULL) || (pstEmmcArg == NULL))
    {
        return RK_ERROR;
    }

    EmmcDevDeInit(pstEmmcDev);

    rkos_semaphore_delete(pstEmmcDev->osEmmcOperReqSem);

    pstEmmcArg->hSdc = pstEmmcDev->hSDC;

    rkos_memory_free(pstEmmcDev);

    EmmcDevDeHwInit(DevID, 0);

#ifndef _EMMC_BOOT_
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_EMMC_DEV);
#endif
#endif
    gpstEmmcDevInf[DevID] = NULL;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevCreate
** Input:void
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
HDC EmmcDev_Create(uint32 DevID, void * arg)
{
    EMMC_DEV_ARG * pstEmmcArg;
    DEVICE_CLASS* pstDev;
    EMMC_DEVICE_CLASS * pstEmmcDev;

    if(arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    pstEmmcArg = (EMMC_DEV_ARG *)arg;


    pstEmmcDev =  rkos_memory_malloc(sizeof(EMMC_DEVICE_CLASS));

    if (pstEmmcDev == NULL)
    {
        return pstEmmcDev;
    }

    pstEmmcDev->osEmmcOperReqSem        = rkos_semaphore_create(1,1);

    if ((pstEmmcDev->osEmmcOperReqSem) == 0)
    {

        rkos_semaphore_delete(pstEmmcDev->osEmmcOperReqSem);

        rkos_memory_free(pstEmmcDev);

        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstEmmcDev;

    pstDev->suspend = EmmcDevSuspend;
    pstDev->resume  = EmmcDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    pstEmmcDev->hSDC = pstEmmcArg->hSdc;

    pstEmmcDev->BusWidth = pstEmmcArg->BusWidth;

#ifndef _EMMC_BOOT_
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_EMMC_DEV, SEGMENT_OVERLAY_ALL);
#endif
#endif

    gpstEmmcDevInf[DevID] = NULL;

    EmmcDevHwInit(DevID, 0);

    if(EmmcDevInit(pstEmmcDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstEmmcDev->osEmmcOperReqSem);
        rkos_memory_free(pstEmmcDev);
        return (HDC) RK_ERROR;
    }

    gpstEmmcDevInf[DevID] = pstEmmcDev;

    return pstDev;

}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: MMCIdentifyCard
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:57:39
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t MMCIdentifyCard(EMMC_DEVICE_CLASS * pstEmmcDev)
{
    SDC_CMD_ARG DevReqArg;
    rk_err_t ret;
    uint32 temp;
    HDC hSdc;

    hSdc = pstEmmcDev->hSDC;

    if (hSdc == NULL)
    {
        return RK_PARA_ERR;
    }

    pstEmmcDev->type = UNKNOW_CARD;

    temp = BUS_WIDTH_1_BIT;

    ret = SdcDev_SetBusWidth(hSdc, temp);

    if(RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    temp = FOD_FREQ;


    ret = SdcDev_SetBusFreq(hSdc, temp);

    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    ret = EmmcSendCmd(hSdc, (SD_GO_IDLE_STATE | SD_NODATA_OP | SD_RSP_NONE | NO_WAIT_PREV | SEND_INIT), 0, NULL);
    if(RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    SDC_Delay(100);  // 27有发现CMD0发送完以后延时一下再发其他命令能提高卡的识别率

    ret = MMCInit(pstEmmcDev);

    return ret;

}

/*******************************************************************************
** Name: MMCDecodeCSD
** Input:HDC dev, uint32 * pCSD
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:40:48
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t MMCDecodeCSD(EMMC_DEVICE_CLASS * pEmmcDev, uint32 * pCSD)
{
    uint32           c_size = 0;
    uint32           c_size_mult = 0;
    uint32           read_bl_len = 0;
    uint32           transfer_rate_unit[4] = {10, 100, 1000, 10000};
    uint32           time_value[16] = {10/*reserved*/, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};
    uint8            erase_grp_size;
    uint8            erase_grp_mult;

    c_size      = (pCSD[1] >> 30) | ((pCSD[2] & 0x3FF) << 2);    //[73:62]
    c_size_mult = (pCSD[1] >> 15) & 0x7;                         //[49:47]
    read_bl_len = (pCSD[2] >> 16) & 0xF;                         //[83:80]

    erase_grp_size = (pCSD[1] >> 10) & 0x1F;                     //[46:42]
    erase_grp_mult = (pCSD[1] >> 5) & 0x1F;                     //[41:37]

    pEmmcDev->EraseSize =  (erase_grp_size + 1) * (erase_grp_mult + 1);

    pEmmcDev->SpecVer    = (CARD_SPEC_VER)(MMC_SPEC_VER_10 + ((pCSD[3] >> 26) & 0xF)); //[125:122]
    pEmmcDev->capability = (((c_size + 1)*(0x1 << (c_size_mult + 2))*(0x1 << read_bl_len)) >> 9);
    pEmmcDev->TranSpeed = transfer_rate_unit[pCSD[3] & 0x3]*time_value[(pCSD[3] >> 3) & 0x7]; //[103:96]
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: MMCGenerateRCA
** Input:HDC dev
** Return: uint16
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:39:20
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
uint16 MMCGenerateRCA(EMMC_DEVICE_CLASS * pEmmcDev)
{
    return (pEmmcDev->rca + 1);
}

/*******************************************************************************
** Name: MMCSwitchFunction
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:37:54
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t MMCSwitchFunction(EMMC_DEVICE_CLASS * pstEmmcDev)
{
    BUS_WIDTH   wide = BUS_WIDTH_INVALID;
    uint32           value = 0;
    uint32           status = 0;
    int32            ret = RK_SUCCESS;
    uint32           DataBuf[512/4];
    uint8           *pBuf = NULL;

    HDC hSdc = pstEmmcDev->hSDC;
    uint32 FreqKhz;
    SDC_CMD_ARG stArg;

    if(pstEmmcDev->SpecVer < MMC_SPEC_VER_40)
    {
        return RK_ERROR;
    }

    pBuf = (uint8*)DataBuf;


    stArg.Cmd = (MMC4_SEND_EXT_CSD | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
    stArg.BlkSize = 512;
    stArg.CmdArg = 0;
    stArg.Buf = (uint8 *)DataBuf;
    stArg.Size = 512;
    stArg.RespBuf = NULL;

    ret = SdcDev_Read(hSdc, &stArg);
    if (ret < RK_SUCCESS)
    {
         goto SDC_ERROR;
    }

    pBuf = (uint8 *)DataBuf;

    pstEmmcDev->BootSize = pBuf[226]*256;  // *128K

    value = ((pBuf[215] << 24) | (pBuf[214] << 16) | (pBuf[213] << 8) | pBuf[212]);//[215--212]  sector count
    if(value)
    {
        pstEmmcDev->capability = value;
    }

    if (pBuf[196] & 0x3) //支持高速模式
    {
        ret = EmmcSendCmd(hSdc, \
                         (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                         ((0x3 << 24) | (185 << 16) | (0x1 << 8)), \
                         &status);

        if((RK_SUCCESS == ret) && ((status & (0x1 << 7)) == 0x0))
        {
            ret = EmmcSendCmd(hSdc, (SD_SEND_STATUS | SD_NODATA_OP | SD_RSP_R1 | NO_WAIT_PREV), (pstEmmcDev->rca << 16), &status);
            if ((RK_SUCCESS == ret) && ((status & (0x1 << 7)) == 0x0))
            {
                if (pBuf[196] & 0x2) // 52M
                {
                    FreqKhz = MMCHS_52_FPP_FREQ;

                    ret = SdcDev_SetBusFreq(hSdc,FreqKhz);
                    if (RK_SUCCESS == ret)
                    {
                        pstEmmcDev->TranSpeed = MMCHS_52_FPP_FREQ;
                    }
                }
                else  // 26M
                {
                    FreqKhz = MMCHS_26_FPP_FREQ;

                    ret = SdcDev_SetBusFreq(hSdc,FreqKhz);
                    if (RK_SUCCESS == ret)
                    {
                        pstEmmcDev->TranSpeed = MMCHS_26_FPP_FREQ;
                    }
                }
            }
        }
    }

    //切换高速模式有不成功不直接return，线宽的切换可以继续
    //切换线宽放在高速模式切换之后做，这样可以顺便检查一下在高速模式下用较宽的数据线会不会出错


    wide = pstEmmcDev->BusWidth;

    if (wide == BUS_WIDTH_4_BIT)
    {
        value = 0x1;

        ret = SdcDev_SetBusWidth(hSdc, wide);
        if (RK_SUCCESS != ret)
        {
             goto SDC_ERROR;
        }

        //下面两个命令都不要检查返回值是否成功，因为它们的CRC会错
        pBuf[0] = 0x5A;
        pBuf[1] = 0x5A;
        pBuf[2] = 0x5A;
        pBuf[3] = 0x5A;

        stArg.Cmd = (MMC4_BUSTEST_W | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 4;
        stArg.CmdArg = 0;
        stArg.Buf = pBuf;
        stArg.Size = 4;
        stArg.RespBuf = NULL;


        ret = SdcDev_Write(hSdc,&stArg);


        pBuf[0] = 0x00;
        pBuf[1] = 0x00;
        pBuf[2] = 0x00;
        pBuf[3] = 0x00;

        stArg.Cmd = (MMC4_BUSTEST_R | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 4;
        stArg.CmdArg = 0;
        stArg.Buf = pBuf;
        stArg.Size = 4;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc, &stArg);



        if (pBuf[0] != 0xA5)
        {

        }

    }
    else if (wide == BUS_WIDTH_8_BIT)
    {
        value = 0x2;

        ret = SdcDev_SetBusWidth(hSdc, wide);

        if (RK_SUCCESS != ret)
        {
            return RK_ERROR;
        }
        //下面两个命令都不要检查返回值是否成功，因为它们的CRC会错
        pBuf[0] = 0x55;
        pBuf[1] = 0xAA;
        pBuf[2] = 0x55;
        pBuf[3] = 0xAA;
        pBuf[4] = 0x55;
        pBuf[5] = 0xAA;
        pBuf[6] = 0x55;
        pBuf[7] = 0xAA;

        stArg.Cmd = (MMC4_BUSTEST_W | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 8;
        stArg.CmdArg = 0;
        stArg.Buf = pBuf;
        stArg.Size = 8;

        stArg.RespBuf = NULL;

        ret = SdcDev_Write(hSdc, &stArg);


        pBuf[0] = 0x00;
        pBuf[1] = 0x00;
        pBuf[2] = 0x00;
        pBuf[3] = 0x00;
        pBuf[4] = 0x00;
        pBuf[5] = 0x00;
        pBuf[6] = 0x00;
        pBuf[7] = 0x00;

        stArg.Cmd = (MMC4_BUSTEST_R | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 8;
        stArg.CmdArg = 0;
        stArg.Buf = pBuf;
        stArg.Size = 8;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc,&stArg);


        if ((pBuf[0] != 0xAA)
             || (pBuf[1] != 0x55))
        {

        }
    }
    else
    {

    }

    ret = EmmcSendCmd(hSdc, \
                         (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                         ((0x3 << 24) | (183 << 16) | (value << 8)), \
                         &status);
    if ((RK_SUCCESS != ret) || (status & (0x1 << 7)))
    {
         goto SDC_ERROR;;
    }
    ret = EmmcSendCmd(hSdc, (SD_SEND_STATUS | SD_NODATA_OP | SD_RSP_R1 | NO_WAIT_PREV), (pstEmmcDev->rca << 16), &status);
    if ((RK_SUCCESS != ret) || (status & (0x1 << 7)))
    {
         goto SDC_ERROR;;
    }

SDC_ERROR:
    return ret;
}

/*******************************************************************************
** Name: MMCInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:36:45
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t MMCInit(EMMC_DEVICE_CLASS * pstEmmcDev)
{
    uint32           type = UNKNOW_CARD;
    uint32           LongResp[4];
    uint32           status = 0;
    uint16           rca = 0;
    uint32           i = 0;
    int32            ret = RK_SUCCESS;

    pstEmmcDev->rca = 2;

    /**************************************************/
    // 让卡进入Ready State
    /**************************************************/
    for (i=0; i<((FOD_FREQ*1000)/(48+2)); i++)
    {
        ret = EmmcSendCmd(pstEmmcDev->hSDC, (MMC_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R3 | WAIT_PREV), 0x40ff8000, &status);
        if (RK_SUCCESS == ret)
        {
            if (status & 0x80000000)
            {
                if ((0x80ff8000 == status) || (0x80ff8080 == status))
                {
                    type = MMC;
                    break;
                }
                else if((0xc0ff8000 == status) || (0xc0ff8080 == status))
                {
                    type = eMMC2G;
                    break;
                }
                else
                {
                    ret = RK_ERROR;
                    break;
                }
            }
        }
        else
        {
            /* error occured */
            ret = RK_ERROR;
            break;
        }
    }

    if (ret != RK_SUCCESS)
    {
        return RK_ERROR;
    }
    //长时间busy
    if (((FOD_FREQ*1000)/(48+2)) == i)
    {
        return RK_ERROR;
    }
    /**************************************************/
    // 让卡进入Stand-by State
    /**************************************************/
    memset(LongResp, 0, sizeof(LongResp));
    ret = EmmcSendCmd(pstEmmcDev->hSDC, (SD_ALL_SEND_CID | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), 0, LongResp);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    //decode CID
    //_MMC_DecodeCID(LongResp, pCard);

    //generate a RCA
    rca = MMCGenerateRCA(pstEmmcDev);
    ret = EmmcSendCmd(pstEmmcDev->hSDC, (MMC_SET_RELATIVE_ADDR | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    pstEmmcDev->rca = rca;

    memset(LongResp, 0, sizeof(LongResp));
    ret = EmmcSendCmd(pstEmmcDev->hSDC, (SD_SEND_CSD | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), (rca << 16), LongResp);
    if (RK_SUCCESS != ret)
    {
        return ret;
    }
    //decode CSD
    MMCDecodeCSD(pstEmmcDev,LongResp);

    pstEmmcDev->TranSpeed = (pstEmmcDev->TranSpeed > MMC_FPP_FREQ) ? MMC_FPP_FREQ : (pstEmmcDev->TranSpeed);

    rk_printf("Emmc0 TranSpeed = %d", pstEmmcDev->TranSpeed);


    ret = SdcDev_SetBusFreq(pstEmmcDev->hSDC, pstEmmcDev->TranSpeed);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    /**************************************************/
    // 让卡进入Transfer State
    /**************************************************/
    ret = EmmcSendCmd(pstEmmcDev->hSDC, (SD_SELECT_DESELECT_CARD | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), (rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    /* 协议规定不管是SD1.X或者SD2.0或者SDHC都必须支持block大小为512, 而且我们一般也只用512，因此这里直接设为512 */
    ret = EmmcSendCmd(pstEmmcDev->hSDC, (SD_SET_BLOCKLEN | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 512, &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    pstEmmcDev->WriteProt = FALSE;  //MMC卡都没有写保护
    //卡输入开启密码在这里做
    if (status & CARD_IS_LOCKED)
    {
        pstEmmcDev->bPassword = TRUE;
    }
    else
    {
        pstEmmcDev->bPassword = FALSE;
        MMCSwitchFunction(pstEmmcDev);
    }
    pstEmmcDev->type |= type;
    return RK_SUCCESS ;
}

/*******************************************************************************
** Name: MMC_PartitionConfig
** Input:HDC dev, uint8 value
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 11:29:16
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t MMC_PartitionConfig(EMMC_DEVICE_CLASS * pEmmcDev, uint8 value)
{
    uint32           status = 0;
    int32            ret = RK_SUCCESS;

    ret = EmmcSendCmd(pEmmcDev->hSDC, \
                         (MMC4_SWITCH_FUNC | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), \
                         ((0x3 << 24) | (179 << 16) | (value << 8)), \
                         &status);

    if((RK_SUCCESS == ret) && ((status & (0x1 << 7)) != 0x0))
    {
        ret = RK_ERROR;
    }
    return ret;
}


/*******************************************************************************
** Name: EmmcDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t EmmcDevResume(HDC dev)
{
     EMMC_DEVICE_CLASS * pstEmmcDev = (EMMC_DEVICE_CLASS *)dev;
     if(EmmcDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     EmmcDevHwInit(pstEmmcDev->stEmmcDevice.DevID, 0);
     EmmcDevInit(pstEmmcDev);

     RKDev_Resume(pstEmmcDev->hSDC);

     pstEmmcDev->stEmmcDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t EmmcDevSuspend(HDC dev, uint32 Level)
{
    EMMC_DEVICE_CLASS * pstEmmcDev = (EMMC_DEVICE_CLASS *)dev;
    if(EmmcDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstEmmcDev->stEmmcDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstEmmcDev->stEmmcDevice.State = DEV_SATE_IDLE2;
    }

    EmmcDevDeInit(pstEmmcDev);
    EmmcDevDeHwInit(pstEmmcDev->stEmmcDevice.DevID, 0);

    RKDev_Suspend(pstEmmcDev->hSDC);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: EmmcDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t EmmcDevDeInit(EMMC_DEVICE_CLASS * pstEmmcDev)
{
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
_DRIVER_EMMC_EMMCDEVICE_INIT_
rk_err_t EmmcDevInit(EMMC_DEVICE_CLASS * pEmmcDev)
{
    return MMCIdentifyCard(pEmmcDev);
}

#ifdef _USE_SHELL_
#ifdef _EMMC_DEV_SHELL_
_DRIVER_EMMC_EMMCDEVICE_DATA_
static  SHELL_CMD ShellEmmcName[] =
{
    "pcb",NULL,"NULL","NULL",
    "create",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "help",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",   // the end
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: EmmcShell
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_SHELL_
rk_err_t EmmcDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellEmmcName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            ret = EmmcShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = EmmcShellCreate(dev,pItem);
            break;

        case 0x02:
            ret = EmmcShellTest(dev,pItem);
            break;

        case 0x03:  //help
            ret = EmmcShellHelp (dev,pItem);
            break;

        case 0x04:
            //ret = ShellCustomParsing(pItem,UartDeviceHandler);
            break;

        default:
            ret = RK_ERROR;
            break;
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
** Name: EmmcShellHelp
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:chad.Ma
** Date: 2014.11.3
** Time: 16:25:09
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_SHELL_
SHELL FUN rk_err_t EmmcShellHelp(HDC dev,  uint8 * pstr)
{
    pstr--;

    if( StrLenA((uint8 *) pstr) != 0)
        return RK_ERROR;

    rk_print_string("emmc命令集提供了一系列的命令对emmc进行操作\r\n");
    rk_print_string("emmc包含的子命令如下:           \r\n");
    rk_print_string("pcb       显示pcb信息           \r\n");
    rk_print_string("open      打开emmc              \r\n");
    rk_print_string("test      测试emmc命令          \r\n");
    rk_print_string("help      显示emmc命令帮助信息  \r\n");

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: EmmcShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.30
** Time: 13:48:37
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_SHELL_
SHELL FUN rk_err_t EmmcShellTest(HDC dev, uint8 * pstr)
{
    HDC hEmmcDev;
    uint32 TotalSec;
    uint8 * pBuf;
    int32 BufSec;
    uint32 i, j;
    rk_err_t ret;

#ifdef SHELL_HELP
    pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("emmc.test : emmc test cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif


    hEmmcDev = RKDev_Open(DEV_CLASS_EMMC, 0, NOT_CARE);
    if(hEmmcDev == NULL)
    {
        rk_print_string("emmc device open failure");
        return RK_SUCCESS;
    }

    TotalSec = 0;

    ret = EmmcDev_SetArea(hEmmcDev, EMMC_AREA_USER);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("emmc USER failure");
        return RK_SUCCESS;
    }


    ret = EmmcDev_GetAreaSize(hEmmcDev, EMMC_AREA_USER, &TotalSec);

    if((ret != RK_SUCCESS) || TotalSec == 0)
    {
        rk_print_string("emmc get size failure");
        return RK_SUCCESS;
    }

    for(BufSec = 128; BufSec > 0; BufSec--)
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

    //TotalSec = 0x3200; // 100M

    rk_printf("test start = %d\n", BufSec);

    for(i = 1;  i < (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];


        ret = EmmcDev_Write(hEmmcDev, i - 1, pBuf, BufSec);

        rk_printf("write: LBA = 0x%08x, Len = %d", i, BufSec);

        if(ret != BufSec)
        {
            rk_print_string("emmc write error");
        }
    }

    if(TotalSec % BufSec != 0)
    {


        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];



        ret = EmmcDev_Write(hEmmcDev, i - 1, pBuf, TotalSec % BufSec);

        rk_printf("write: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if(ret != TotalSec % BufSec)
        {
            rk_print_string("emmc write error");
        }
    }

    rk_printf("test end");

    for(i = 1;  i < (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        ret = EmmcDev_Read(hEmmcDev, i - 1, pBuf, BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, BufSec);

        if(ret != BufSec)
        {
            rk_print_string("emmc read error");
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_print_string("emmc data error");
        }


        for(j = 2; j < BufSec * 512; j++)
        {
            if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                 rk_print_string("emmc data error");
            }
        }

    }

    if(TotalSec % BufSec != 0)
    {

        ret = EmmcDev_Read(hEmmcDev, i - 1, pBuf, TotalSec % BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if(ret != TotalSec % BufSec)
        {
            rk_print_string("emmc read error");
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_print_string("emmc data error");
        }


        for(j = 2; j < (TotalSec % BufSec) * 512; j++)
        {
            if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                 rk_print_string("emmc data error");
            }
        }

    }

    rkos_memory_free(pBuf);
    RKDev_Close(hEmmcDev);

    return RK_SUCCESS;


}

/*******************************************************************************
** Name: EmmcShellPcb
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_SHELL_
rk_err_t EmmcShellPcb(HDC dev,  uint8 * pstr)
{
#ifdef SHELL_HELP
    pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA((uint8 *)pstr, "help", 4) == 0)
        {
            rk_print_string("emmc.pcb : emmc pcb info .\r\n");
            return RK_SUCCESS;
        }
    }
#endif
    // TODO:
    //add other code below:
    //...

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
_DRIVER_EMMC_EMMCDEVICE_SHELL_
rk_err_t EmmcShellCreate(HDC dev,  uint8 * pstr)
{
    HDC hEmmc;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    //uint32 i;
    //uint8 DevPath[16], len;
    EMMC_DEV_ARG stEmmcDevArg;

#ifdef SHELL_HELP
    pstr--;
    if(pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if(StrCmpA((uint8 *)pstr, "help", 4) == 0)
        {
            rk_print_string("emmc.open : open emmc.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    stEmmcDevArg.hSdc = RKDev_Open(DEV_CLASS_SDC, 0, NOT_CARE);

    if((stEmmcDevArg.hSdc == NULL) || (stEmmcDevArg.hSdc == (HDC)RK_ERROR) || (stEmmcDevArg.hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("SDC0 open failure");
        return RK_SUCCESS;
    }

    stEmmcDevArg.BusWidth = EMMC_DEV0_BUS_WIDTH;

    ret = RKDev_Create(DEV_CLASS_EMMC, 0, &stEmmcDevArg);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("emmc open failure");
        return RK_SUCCESS;
    }


    hEmmc = RKDev_Open(DEV_CLASS_EMMC, 0, NOT_CARE);

    ret = EmmcDev_SetArea(hEmmc, EMMC_AREA_BOOT1);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("emmc BOOT failure");
        return RK_SUCCESS;
    }

    RKDev_Close(hEmmc);

    return RK_SUCCESS;

}
#endif
#endif
#endif

