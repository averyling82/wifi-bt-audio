/*
********************************************************************************************
*
*            Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SD\SdDevice.c
* Owner: aaron.sun
* Date: 2014.12.11
* Time: 16:30:14
* Desc: sd / tf driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.12.11     16:30:14   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_SD_SDDEVICE_C__

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
typedef enum
{
    SD_SM_DEFAULT = 0, //SDR12
    SD_SM_HIGH_SPEED,  //SDR50
    SD_SM_SDR50,
    SD_SM_SDR104,
    SD_SM_DDR50,
    SD_SM_MAX
} SD_SPEED_MODE;


/* SD Specification Version */
typedef enum CARD_SPEC_VER_Enum
{
    /* SD Specification Version */
    SPEC_VER_INVALID,
    SD_SPEC_VER_10,             //SD Specification Version 1.0-1.01
    SD_SPEC_VER_11,             //SD Specification Version 1.1
    SD_SPEC_VER_20,             //SD Specification Version 2.0
    SD_SPEC_VER_30,             //SD Specification Version 3.0
    SD_SPEC_VER_MAX

}CARD_SPEC_VER_E;

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


#define SD_DEV_NUM 2



typedef  struct _SD_DEVICE_CLASS
{
    DEVICE_CLASS stSdDevice;
    pSemaphore osSdOperReqSem;

    uint32           type;             //Card type
    uint16           rca;              //Relative Card Address
    uint32           TranSpeed;       //卡的最大数据传输速度，也就是卡的最大工作频率，单位KHz
    uint32           capability;       //Card capability,单位block数，每个block为512字节
    uint32           BootSize;         //boot partition size,单位sector(512B)
    bool             WriteProt;        //Mechanical write Protect switch state,
    bool             bPassword;        //用于指示卡是否是有密码的卡, TRUE:have password, FALSE:no password
    uint16           EraseSize;        //this size is given as minimum number of write blocks that can be erased in a single erase command
    HDC              hSDC;
    uint32           SpecVer;
    eGPIOPinNum_t    DevResetGPIO;
    BUS_WIDTH        BusWidth;

}SD_DEVICE_CLASS;




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SD_DEVICE_CLASS * gpstSdDevISR[SD_DEV_NUM] = {(SD_DEVICE_CLASS *)NULL,(SD_DEVICE_CLASS *)NULL};



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
rk_err_t SdDevCheckHandler(HDC dev);
rk_err_t SdDev_GetSize(HDC dev, uint32 *size);
rk_err_t SDSwitchBusWidth(SD_DEVICE_CLASS * pstSdDev, BUS_WIDTH width);
rk_err_t SDSwitchSpeedMode(SD_DEVICE_CLASS * pstSdDev, uint32 mode);
rk_err_t SDSwitchFunction(SD_DEVICE_CLASS * pstSdDev);
rk_err_t SDIsWriteProt(SD_DEVICE_CLASS * pstSdDev);
void SDDecodeCSD(SD_DEVICE_CLASS * pstSdDev, uint32 * pCSD);
rk_err_t SDSendAppCmd(SD_DEVICE_CLASS * pstSdDev,
                      uint16 rca,
                      uint32 cmd,
                      uint32 cmdArg,
                      uint32 *responseBuf);

rk_err_t SDXX_Init(SD_DEVICE_CLASS * pstSdDev, BOOL SupportCmd8);
rk_err_t SdSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf);
rk_err_t _IdentifyUnknow(SD_DEVICE_CLASS * pstSdDev);
rk_err_t SdDevShellTest(HDC dev, uint8 * pstr);
rk_err_t SdDevShellDel(HDC dev, uint8 * pstr);
rk_err_t SdDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t SdDevShellPcb(HDC dev, uint8 * pstr);
void SdDevIntIsr1(void);
void SdDevIntIsr0(void);
void SdDevIntIsr(uint32 DevID);
rk_err_t SdDevDeInit(SD_DEVICE_CLASS * pstSdDev);
rk_err_t SdDevInit(SD_DEVICE_CLASS * pstSdDev);
rk_err_t SdDevResume(HDC dev);
rk_err_t SdDevSuspend(HDC dev, uint32 Level);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON API rk_err_t SdDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SdDev Read...
    SD_DEVICE_CLASS * pstSdDev =  (SD_DEVICE_CLASS *)dev;
    HDC hSdc;
    uint32 BlkCnt,status;
    rk_err_t ret;
    SDC_CMD_ARG stArg;
    uint32 mul;

    if (pstSdDev == NULL)
    {
        return RK_ERROR;
    }

    hSdc = pstSdDev->hSDC;


    if (len == 0)
    {
        ret = RK_PARA_ERR;
    }

    rkos_semaphore_take(pstSdDev->osSdOperReqSem, MAX_DELAY);
    mul = (pstSdDev->type & (SDHC | eMMC2G))? 0:9;

    if (1 == len)
    {
        stArg.Cmd = (SD_READ_SINGLE_BLOCK | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
        stArg.BlkSize = 512;
        stArg.CmdArg = LBA << mul;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc, &stArg);
        if (ret == len * 512)
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
        stArg.CmdArg = LBA << mul;
        stArg.Buf = buffer;
        stArg.Size = len * 512;
        stArg.RespBuf = NULL;

        ret = SdcDev_Read(hSdc, &stArg);
        if (ret == len * 512)
        {
            ret = len;
        }
        else
        {
            //rk_printf("sd read error");

            ret = RK_ERROR;
        }

        SdSendCmd(hSdc,
                  (SD_STOP_TRANSMISSION | SD_NODATA_OP | SD_RSP_R1B | STOP_CMD | NO_WAIT_PREV),
                  0,
                  &status);


    }

    rkos_semaphore_give(pstSdDev->osSdOperReqSem);

    return ret;

}

/*******************************************************************************
** Name: SdDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON API rk_err_t SdDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SdDev Write...
    SD_DEVICE_CLASS * pstSdDev =  (SD_DEVICE_CLASS *)dev;
    HDC hSdc;
    uint32 BlkCnt,status;
    rk_err_t ret;
    SDC_CMD_ARG stArg;
    uint32 mul;

    if (pstSdDev == NULL)
    {
        return RK_ERROR;
    }

    {

        hSdc = pstSdDev->hSDC;

        if (len == 0)
        {
            ret = RK_PARA_ERR;
        }

        rkos_semaphore_take(pstSdDev->osSdOperReqSem, MAX_DELAY);
        mul = (pstSdDev->type & (SDHC | eMMC2G))? 0:9;
        if (1 == len)
        {
            stArg.Cmd = (SD_WRITE_BLOCK | SD_WRITE_OP | SD_RSP_R1 | WAIT_PREV);
            stArg.BlkSize = 512;
            stArg.CmdArg = LBA << mul;
            stArg.Buf = buffer;
            stArg.Size = len * 512;
            stArg.RespBuf = NULL;

            ret = SdcDev_Write(hSdc, &stArg);
            if (ret == len * 512)
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
            stArg.CmdArg = LBA << mul;
            stArg.Buf = buffer;
            stArg.Size = len * 512;
            stArg.RespBuf = NULL;

            ret = SdcDev_Write(hSdc, &stArg);
            if (ret == len * 512)
            {
                ret = len;
            }
            else
            {
                ret = RK_ERROR;
            }

            SdSendCmd(hSdc,
                      (SD_STOP_TRANSMISSION | SD_NODATA_OP | SD_RSP_R1B | STOP_CMD | NO_WAIT_PREV),
                      0,
                      &status);

            SdcDev_DataTransIdle(hSdc);

        }

        rkos_semaphore_give(pstSdDev->osSdOperReqSem);

        return ret;
    }

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.18
** Time: 15:36:17
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN rk_err_t SdDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < SD_DEV_NUM; i++)
    {
        if(gpstSdDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: SdDev_GetSize
** Input:HDC dev, uint32 *size
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.17
** Time: 14:48:29
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN rk_err_t SdDev_GetSize(HDC dev, uint32 *size)
{
    SD_DEVICE_CLASS * pstSdDev =  (SD_DEVICE_CLASS *)dev;
    *size = pstSdDev->capability;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SDIsWriteProt
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 15:15:44
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN rk_err_t SDIsWriteProt(SD_DEVICE_CLASS * pstSdDev)
{
    return RK_ERROR;
}

/*******************************************************************************
** Name: SDSendAppCmd
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 14:37:13
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN rk_err_t SDSendAppCmd(SD_DEVICE_CLASS * pstSdDev,
                                 uint16 rca,
                                 uint32 cmd,
                                 uint32 cmdArg,
                                 uint32 *responseBuf)
{
    rk_err_t  ret;
    uint32 status = 0;

    ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV),(rca << 16),&status);

    if (RK_SUCCESS != ret)
    {
        return ret;
    }

    if (status & 0x20)
    {

        ret = SdSendCmd(pstSdDev->hSDC, cmd, cmdArg, responseBuf);
    }
    else
    {
        ret = RK_ERROR;
    }

    return ret;
}
/*******************************************************************************
** Name: SdSendCmd
** Input:HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 14:06:57
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN rk_err_t SdSendCmd(HDC hSdc, uint32 cmd, uint32 CmdArg, uint32 *RespBuf)
{
    SDC_CMD_ARG DevReqArg;
    DevReqArg.Cmd = cmd;
    DevReqArg.CmdArg = CmdArg;
    DevReqArg.RespBuf = RespBuf;
    return SdcDev_SendCmd(hSdc, &DevReqArg);
}

/*******************************************************************************
** Name: SdDevIntIsr1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN void SdDevIntIsr1(void)
{
    //Call total int service...
    SdDevIntIsr(1);

}
/*******************************************************************************
** Name: SdDevIntIsr0
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN void SdDevIntIsr0(void)
{
    //Call total int service...
    SdDevIntIsr(0);

}
/*******************************************************************************
** Name: SdDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_COMMON_
COMMON FUN void SdDevIntIsr(uint32 DevID)
{
    uint32 SdDevIntType;

    //Get SdDev Int type...
    //SdDevIntType = GetIntType();
    if (gpstSdDevISR[DevID] != NULL)
    {
        //if (SdDevIntType & INT_TYPE_MAP)
        {
            //write serice code...
        }

        //wirte other int service...
    }

}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SdDevResume(HDC dev)
{
    SD_DEVICE_CLASS * pstSdDev = (SD_DEVICE_CLASS *)dev;
    if(SdDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    SdDevHwInit(pstSdDev->stSdDevice.DevID,0);
    SdDevInit(pstSdDev);

    RKDev_Resume(pstSdDev->hSDC);

    pstSdDev->stSdDevice.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SdDevSuspend(HDC dev, uint32 Level)
{
    SD_DEVICE_CLASS * pstSdDev = (SD_DEVICE_CLASS *)dev;
    if(SdDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstSdDev->stSdDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstSdDev->stSdDevice.State = DEV_SATE_IDLE2;
    }

    SdDevDeInit(pstSdDev);
    SdDevHwDeInit(pstSdDev->stSdDevice.DevID, 0);

    RKDev_Suspend(pstSdDev->hSDC);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SdDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT API rk_err_t SdDev_Delete(uint32 DevID, void * arg)
{
    //Check SdDev is not exist...
    SD_DEV_ARG * pArg;
    if (gpstSdDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    pArg = (SD_DEV_ARG *)arg;
    if(pArg != NULL)
        pArg->hSdc = gpstSdDevISR[DevID]->hSDC;

    //SdDev deinit...
    SdDevDeInit(gpstSdDevISR[DevID]);
    SdDevHwDeInit(DevID, 0);

    //Free SdDev memory...
    rkos_semaphore_delete(gpstSdDevISR[DevID]->osSdOperReqSem);
    rkos_memory_free(gpstSdDevISR[DevID]);

    //Delete SdDev...
    gpstSdDevISR[DevID] = NULL;

    //Delete SdDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_SD_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SdDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT API HDC SdDev_Create(uint32 DevID, void * arg)
{
    SD_DEV_ARG * pstSdDevArg;
    DEVICE_CLASS* pstDev;
    SD_DEVICE_CLASS * pstSdDev;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    // Create handler...
    pstSdDevArg = (SD_DEV_ARG *)arg;
    pstSdDev =  rkos_memory_malloc(sizeof(SD_DEVICE_CLASS));
    memset(pstSdDev, 0, sizeof(SD_DEVICE_CLASS));

    if (pstSdDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstSdDev->osSdOperReqSem  = rkos_semaphore_create(1,1);

    if ((pstSdDev->osSdOperReqSem) == 0)

    {
        rkos_semaphore_delete(pstSdDev->osSdOperReqSem);
        rkos_memory_free(pstSdDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstSdDev;
    pstDev->resume  = SdDevResume;
    pstDev->suspend = SdDevSuspend;
    pstDev->SuspendMode = ENABLE_MODE;


    //init arg...
    pstSdDev->hSDC = pstSdDevArg->hSdc;
    pstSdDev->BusWidth = pstSdDevArg->BusWidth;


    gpstSdDevISR[DevID] = NULL;
    //module overlay...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_SD_DEV, SEGMENT_OVERLAY_CODE);
#endif
    //device init...
    SdDevHwInit(DevID, 0);
    if (SdDevInit(pstSdDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstSdDev->osSdOperReqSem);
        rkos_memory_free(pstSdDev);
        return (HDC) RK_ERROR;
    }
    gpstSdDevISR[DevID] = pstSdDev;
    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SDSwitchBusWidth
** Input:SD_DEVICE_CLASS * pstSdDev, BUS_WIDTH width
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.15
** Time: 15:35:00
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SDSwitchBusWidth(SD_DEVICE_CLASS * pstSdDev, BUS_WIDTH width)
{
    uint32           data[(64/(8*4))];    //不知道会不会太大，导致栈溢出
    uint32           status = 0;
    rk_err_t         ret;
    SDC_CMD_ARG stArg;

    ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (pstSdDev->rca << 16), &status);

    if (RK_SUCCESS != ret)
    {
        return ret;
    }
    ret = SdSendCmd(pstSdDev->hSDC, (SDA_SET_BUS_WIDTH | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 2, &status);
    if (RK_SUCCESS != ret)
    {
        return ret;
    }
    ret = SdcDev_SetBusWidth(pstSdDev->hSDC, width);
    if (RK_SUCCESS != ret)
    {
        return ret;
    }

    //再读一下SCR，用于验证4bit数据线宽是否有问题，因为发现有的卡能支持4bit线宽，但是一旦切换到4bit线宽，就发生data start bit error
    ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (pstSdDev->rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return ret;
    }

    stArg.Cmd = (SDA_SEND_SCR | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
    stArg.BlkSize = (64 >> 3);
    stArg.CmdArg = 0;
    stArg.Buf = (uint8 *)data;
    stArg.Size = (64 >> 3);
    stArg.RespBuf = &status;

    ret = SdcDev_Read(pstSdDev->hSDC, &stArg);

    if (8 != (uint32)ret)
    {

    }
    else
    {
        ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (pstSdDev->rca << 16), &status);
        if (RK_SUCCESS != ret)
        {
            return ret;
        }
        ret = SdSendCmd(pstSdDev->hSDC, (SDA_SET_BUS_WIDTH | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 0, &status);
        if (RK_SUCCESS != ret)
        {
            return ret;
        }
        ret = SdcDev_SetBusWidth(pstSdDev->hSDC, BUS_WIDTH_1_BIT);
        if (RK_SUCCESS != ret)
        {
            return ret;
        }
    }
    return ret;
}

/*******************************************************************************
** Name: SDSwitchSpeedMode
** Input:SD_DEVICE_CLASS * pstSdDev, uint32 mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 15:31:26
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SDSwitchSpeedMode(SD_DEVICE_CLASS * pstSdDev, uint32 mode)
{
    uint32           data[(512/(8*4))];    //不知道会不会太大，导致栈溢出
    uint8           *pDataBuf = (uint8 *)data;
    uint8            tmp = 0;
    uint32           status = 0;
    rk_err_t         ret;
    uint32           i = 0;

    HDC hSdc = pstSdDev->hSDC;
    uint32 FreqKhz;
    SDC_CMD_ARG stArg;

    memset(data, 0x00, (512 >> 3));

    stArg.Cmd = (SD2_SWITCH_FUNC | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
    stArg.BlkSize = (512 >> 3);
    stArg.CmdArg = ((0x0U << 31) | (0xFFFFF0) | mode/*(0x1)*/);
    stArg.Buf = (uint8 *)data;
    stArg.Size = (512 >> 3);
    stArg.RespBuf = &status;

    ret = SdcDev_Read(hSdc, &stArg);


    if (stArg.Size != (uint32)ret)
    {
        return RK_ERROR;
    }


    //SD2_SWITCH_FUNC命令收到的数据也是Data Packet Format为Wide Width Data，数据也必须颠倒过来
    for (i=0; i<(512 >> 4); i++)
    {
        tmp         = pDataBuf[i];
        pDataBuf[i] = pDataBuf[(512 >> 3) - 1 - i];
        pDataBuf[(512 >> 3) - 1 - i] = tmp;
    }

    if ((pDataBuf[50] & (0x1<<mode)/*0x2*/)&& ((pDataBuf[47] & 0xF) == mode/*0x1*/))//bit 401:High Speed support//bit 379-376:whether function can be switched
    {
        if ((pDataBuf[46] == 0x0)                            //bit 375-368:indicate bit 273 defined
                || ((pDataBuf[46] == 0x1) && (!(pDataBuf[35] & (0x1<<mode)/*0x2*/))))//bit 273 defined:check whether High Speed ready
        {
            memset(data, 0x00, (512 >> 3));

            stArg.Cmd = (SD2_SWITCH_FUNC | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
            stArg.BlkSize = (512 >> 3);
            stArg.CmdArg = ((0x1U << 31) | (0xFFFFF0) | mode/*(0x1)*/);
            stArg.Buf = (uint8 *)data;
            stArg.Size = (512 >> 3);
            stArg.RespBuf = &status;

            ret = SdcDev_Read(hSdc, &stArg);


            if (stArg.Size != (uint32)ret)
            {
                return RK_ERROR;
            }

            for (i=0; i<(512 >> 4); i++)
            {
                tmp         = pDataBuf[i];
                pDataBuf[i] = pDataBuf[(512 >> 3) - 1 - i];
                pDataBuf[(512 >> 3) - 1 - i] = tmp;
            }

            if (((pDataBuf[47] & 0xF) == mode/*0x1*/)  //bit 379-376:whether function switched successful
                    && ((pDataBuf[47] & 0xF0) != 0xF0)
                    && ((pDataBuf[48] & 0xF) != 0xF)
                    && ((pDataBuf[48] & 0xF0) != 0xF0)
                    && ((pDataBuf[49] & 0xF) != 0xF)
                    && ((pDataBuf[49] & 0xF0) != 0xF0))
            {
                ret = SdcDev_SetBusFreq(hSdc, SDHC_FPP_FREQ);
                if (RK_SUCCESS != ret)
                {
                    return RK_ERROR;
                }
                pstSdDev->TranSpeed = SDHC_FPP_FREQ;
            }
        }
    }
    return ret;
}
/*******************************************************************************
** Name: SDSwitchFunction
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 15:19:33
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SDSwitchFunction(SD_DEVICE_CLASS * pstSdDev)
{
    BUS_WIDTH        wide = BUS_WIDTH_INVALID;
    uint32           data[(64/(8*4))];    //不知道会不会太大，导致栈溢出
    uint8           *pDataBuf = (uint8 *)data;
    uint8            tmp = 0;
    uint32           status = 0;
    rk_err_t         ret;
    uint32           i = 0;
    SDC_CMD_ARG stArg;

    //read card SCR, get SD specification version and check whether wide bus supported
    ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), (pstSdDev->rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    memset(data, 0x00, (64 >> 3));


    stArg.Cmd = (SDA_SEND_SCR | SD_READ_OP | SD_RSP_R1 | WAIT_PREV);
    stArg.BlkSize = (64 >> 3);
    stArg.CmdArg = 0;
    stArg.Buf = (uint8 *)data;
    stArg.Size = (64 >> 3);
    stArg.RespBuf = &status;

    ret = SdcDev_Read(pstSdDev->hSDC, &stArg);

    if ((uint32)ret != stArg.Size)
    {
        return RK_ERROR;
    }
    //我们SDMMC控制器接收到Data Packet Format为Wide Width Data的数据时，数据顺序会颠倒，
    //最高字节变成最低字节，最低字节变成最高字节，因此我们这边需要颠回来
    for (i=0; i<(64 >> 4); i++)
    {
        tmp         = pDataBuf[i];
        pDataBuf[i] = pDataBuf[(64 >> 3) - 1 - i];
        pDataBuf[(64 >> 3) - 1 - i] = tmp;
    }

    switch (pDataBuf[7] & 0xF) //bit 59-56:SD specification version
    {
        case 0:
            pstSdDev->SpecVer = SD_SPEC_VER_10;
            break;
        case 1:
            pstSdDev->SpecVer = SD_SPEC_VER_11;
            break;
        case 2:
            {
                pstSdDev->SpecVer = SD_SPEC_VER_20;
                if (pDataBuf[5] & 0x80)     // bit 47
                {
                    pstSdDev->SpecVer = SD_SPEC_VER_30;
                }
                break;
            }
        default:
            pstSdDev->SpecVer = SPEC_VER_INVALID;
            break;
    }


    if (pstSdDev->type != UHS1)
    {
        wide = pstSdDev->BusWidth;

        if ((wide == BUS_WIDTH_INVALID) || (wide == BUS_WIDTH_MAX))
        {
            ret = RK_ERROR;
        }
        //whether SDC iomux support wide bus and card internal support wide bus
        else if ((wide >= BUS_WIDTH_4_BIT) && (pDataBuf[6] & 0x4)) //bit 50:whether wide bus support
        {
            ret = SDSwitchBusWidth (pstSdDev, BUS_WIDTH_4_BIT);
        }
    }

    //切换线宽有不成功不直接return，高速模式的切换可以继续
    //switch to high speed mode
    if (pstSdDev->SpecVer >= SD_SPEC_VER_11)
    {
        SD_SPEED_MODE mode = SD_SM_HIGH_SPEED;

        if (pstSdDev->type == UHS1)
            mode = SD_SM_DDR50;

        ret = SDSwitchSpeedMode(pstSdDev, mode);
    }

    return ret;
}

/*******************************************************************************
** Name: SDDecodeCSD
** Input:SD_DEVICE_CLASS * pstSdDev, uint32 * pCSD
** Return: void
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 15:03:33
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN void SDDecodeCSD(SD_DEVICE_CLASS * pstSdDev, uint32 * pCSD)
{
    uint32           tmp = 0;
    uint32           c_size = 0;
    uint32           c_size_mult = 0;
    uint32           read_bl_len = 0;
    /*uint32           taac = 0;
    uint32           nsac = 0;
    uint32           r2w_factor = 0;*/
    uint32           transfer_rate_unit[4] = {10, 100, 1000, 10000};
    uint32           time_value[16] = {10/*reserved*/, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80};

    tmp = (pCSD[3] >> 30);
    if (tmp == 0)      //CSD version 1.0
    {
        c_size            = ((pCSD[2] & 0x3FF) << 2) | (pCSD[1] >> 30); //[73:62]
        c_size_mult       = (pCSD[1] >> 15) & 0x7;                      //[49:47]
        read_bl_len       = (pCSD[2] >> 16) & 0xF;                      //[83:80]
        pstSdDev->capability = (((c_size + 1)*(0x1 << (c_size_mult + 2))*(0x1 << read_bl_len)) >> 9);
        rk_printf("pstSdDev->capability = %d", pstSdDev->capability);
    }
    else if (tmp == 1) //CSD version 2.0
    {
        c_size            = (pCSD[1] >> 16) | ((pCSD[2] & 0x3F) << 16); //[69:48]
        pstSdDev->capability = ((c_size + 1) << 10);
        rk_printf("pstSdDev->capability = %d", pstSdDev->capability);
    }
    else
    {
        //reserved
    }

    pstSdDev->TranSpeed = transfer_rate_unit[pCSD[3] & 0x3]*time_value[(pCSD[3] >> 3) & 0x7]; //[103:96]
    rk_printf("pstSdDev->TranSpeed = %d", pstSdDev->TranSpeed);

}
/*******************************************************************************
** Name: SDXX_Init
** Input:SD_DEVICE_CLASS * pstSdDev, BOOL SupportCmd8
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 14:33:41
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SDXX_Init(SD_DEVICE_CLASS * pstSdDev, BOOL SupportCmd8)
{

    uint32           type = UNKNOW_CARD;
    uint32           longResp[4];
    uint32           status = 0;
    uint16           rca = 0;
    uint32           i = 0;
    rk_err_t         ret;
    uint32           Acmd41Arg = (TRUE ==SupportCmd8)? 0x41ff8000 : 0x00ff8000;

    SDC_Delay(200);

    /**************************************************/
    // 让卡进入Ready State
    /**************************************************/
    for (i=0; i<((FOD_FREQ*1000)/((48+48+2) << 1)); i++) //按照协议的规定，这里最长可以有1s的时间，我们根据每条命令执行的时间，把它换算成循环次数
    {
        /*0x41ff8000: sd3.0 arg 24bit is S18R*/
        ret = SDSendAppCmd(pstSdDev, 0, (SDA_SD_APP_OP_COND | SD_NODATA_OP | SD_RSP_R3 | WAIT_PREV), Acmd41Arg, &status);
        if (RK_SUCCESS == ret)
        {
            if (status & 0x80000000)
            {
                if (SupportCmd8)
                {
                    if ((0xc0ff8000 == status) || (0xc0ff8080 == status) || (0xc1ff8000 == status))
                    {
                        type = SDHC;
                    }
                    else if ((0x80ff8000 == status) || (0x80ff8080 == status))
                    {
                        type = SD20;
                    }
                    else
                    {
                        ret = RK_ERROR;
                    }
                }
                else
                {
                    type = SD1X;
                }
                break;
            }
        }
        else if (RK_TIMEOUT == ret)
        {
            ret = RK_ERROR;
            break;
        }
        else
        {
            ret = RK_ERROR;
            break;
        }
    }

    if (ret != RK_SUCCESS)
    {
        return RK_ERROR;
    }

    //长时间busy,SDM_VOLTAGE_NOT_SUPPORT
    if (((FOD_FREQ*1000)/((48+48+2) << 1)) == i)
    {
        return RK_ERROR;
    }

    /**************************************************/
    // 让卡进入Stand-by State
    /**************************************************/
    longResp[0] = 0;
    longResp[1] = 0;
    longResp[2] = 0;
    longResp[3] = 0;
    ret = SdSendCmd(pstSdDev->hSDC, (SD_ALL_SEND_CID | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), 0, longResp);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    //decode CID
    //SDDecodeCSD(pstSdDev,longResp);

    ret = SdSendCmd(pstSdDev->hSDC, (SD_SEND_RELATIVE_ADDR | SD_NODATA_OP | SD_RSP_R6 | WAIT_PREV), 0, &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    rca = (uint16)(status >> 16);

    pstSdDev->rca = rca;

    longResp[0] = 0;
    longResp[1] = 0;
    longResp[2] = 0;
    longResp[3] = 0;
    ret = SdSendCmd(pstSdDev->hSDC, (SD_SEND_CSD | SD_NODATA_OP | SD_RSP_R2 | WAIT_PREV), (rca << 16), longResp);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    //decode CSD
    SDDecodeCSD(pstSdDev,longResp);

    pstSdDev->TranSpeed = (pstSdDev->TranSpeed > SD_FPP_FREQ) ? SD_FPP_FREQ : (pstSdDev->TranSpeed);
    ret = SdcDev_SetBusFreq(pstSdDev->hSDC, pstSdDev->TranSpeed);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }
    /**************************************************/
    // 让卡进入Transfer State
    /**************************************************/
    ret = SdSendCmd(pstSdDev->hSDC, (SD_SELECT_DESELECT_CARD | SD_NODATA_OP | SD_RSP_R1B | WAIT_PREV), (rca << 16), &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    /* 协议规定不管是SD1.X或者SD2.0或者SDHC都必须支持block大小为512, 而且我们一般也只用512，因此这里直接设为512 */
    ret = SdSendCmd(pstSdDev->hSDC, (SD_SET_BLOCKLEN | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 512, &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    ret = SDSendAppCmd(pstSdDev, rca, (SDA_SET_CLR_CARD_DETECT | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 0, &status);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    pstSdDev->WriteProt = SDIsWriteProt(pstSdDev);

    //卡输入开启密码在这里做
    if (status & CARD_IS_LOCKED)
    {
        pstSdDev->bPassword = TRUE;
    }
    else
    {
        pstSdDev->bPassword = FALSE;
        SDSwitchFunction(pstSdDev);
    }

    pstSdDev->type |= type;
    return ret;

}

/*******************************************************************************
** Name: _IdentifyUnknow
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.12
** Time: 14:03:48
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t _IdentifyUnknow(SD_DEVICE_CLASS * pstSdDev)
{
    rk_err_t         ret;
    uint32           status = 0;
    uint32           nf;
    uint32           mp;

    ret = SdSendCmd(pstSdDev->hSDC, (SD2_SEND_IF_COND | SD_NODATA_OP | SD_RSP_R7 | WAIT_PREV), 0x1AA, &status);
    if (RK_SUCCESS == ret)
    {
        /* SDIO-only Card or SDIO-SDHC/SD2.0 Combo Card or SDIO-SDHC/SD2.0 only Card or SD2.0 or SDHC */
        nf = 0;
        mp = 0;
        ret = SdSendCmd(pstSdDev->hSDC, (SDIO_IO_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R4 | WAIT_PREV), 0, &status);
        if (RK_SUCCESS == ret)
        {
            nf = (status >> 28) & 0x7;
            mp = (status >> 27) & 0x1;
            if ((mp == 1) && (nf > 0) && (status & 0xFFFF00))
            {
                /* SDIO-SDHC/SD2.0 Combo Card */
                //SDIOHC_SD20_ComboInit(&cardInfo);
                pstSdDev->type = UNKNOW_CARD;
            }
            else if ((mp == 0) && (nf > 0) && (status & 0xFFFF00))
            {
                /* SDIO-only Card */
                //SDIO_OnlyInit(&cardInfo);
                pstSdDev->type = UNKNOW_CARD;
            }
            else if (mp == 1)
            {
                /* SDIO-SDHC/SD2.0 only Card */
                ret = SDXX_Init(pstSdDev, TRUE);
            }
            else
            {
                /* unknow card */
            }
        }
        else if (ret == RK_TIMEOUT)
        {
            /* SD2.0 or SDHC */
            ret = SDXX_Init(pstSdDev, TRUE);
        }
        else
        {
            /* must be error occured */
        }
    }
    else if (RK_TIMEOUT == ret)
    {
        /* SDIO-only Card or SDIO-SD1.X Combo Card or SDIO-SD1.X only Card or SD1.X or MMC or SD2.0 or later with voltage mismatch */
        nf = 0;
        mp = 0;
        ret = SdSendCmd(pstSdDev->hSDC, (SDIO_IO_SEND_OP_COND | SD_NODATA_OP | SD_RSP_R4 | WAIT_PREV), 0, &status);
        if (RK_SUCCESS== ret)
        {
            nf = (status >> 28) & 0x7;
            mp = (status >> 27) & 0x1;
            if ((mp == 1) && (nf > 0) && (status & 0xFFFF00))
            {
                /* SDIO-SD1.X Combo Card */
                //SDIO_SD1X_ComboInit(&cardInfo);
                pstSdDev->type = UNKNOW_CARD;
            }
            else if ((mp == 0) && (nf > 0) && (status & 0xFFFF00))
            {
                /* SDIO-only Card */
                //SDIO_OnlyInit(&cardInfo);
                pstSdDev->type = UNKNOW_CARD;
            }
            else if (mp == 1)
            {
                /* SDIO-SD1.X only Card */
                ret = SDXX_Init(pstSdDev, FALSE);
            }
            else
            {
                /* unknow card */
            }
        }
        else if (ret == RK_TIMEOUT)
        {
            /* SD1.X or MMC or SD2.0 or later with voltage mismatch */
            ret = SdSendCmd(pstSdDev->hSDC, (SD_APP_CMD | SD_NODATA_OP | SD_RSP_R1 | WAIT_PREV), 0, &status);
            if (RK_SUCCESS == ret)
            {
                /* SD1.X or SD2.0 or later with voltage mismatch */
                ret = SDXX_Init(pstSdDev, FALSE);
            }
            else if (RK_TIMEOUT == ret)
            {

            }
            else
            {
                /* must be error occured */
            }
        }
        else
        {
            /* must be error occured */
        }
    }
    else
    {
        /* must be error occured */
    }
    return ret;
}

/*******************************************************************************
** Name: SdDevDeInit
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SdDevDeInit(SD_DEVICE_CLASS * pstSdDev)
{
    //If ip device first step unregister int
    //IntDisable(INT_ID17_UART);
    //IntPendingClear(INT_ID17_UART);
    //IntUnregister(INT_ID17_UART);

    //If ip device second step close clk
    //ScuClockGateCtr(CLOCK_GATE_UART,0);
    return RK_ERROR;

}
/*******************************************************************************
** Name: SdDevInit
** Input:SD_DEVICE_CLASS * pstSdDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT FUN rk_err_t SdDevInit(SD_DEVICE_CLASS * pstSdDev)
{
    SDC_CMD_ARG DevReqArg;
    rk_err_t ret;
    uint32 temp;
    HDC hSdc;

    hSdc = pstSdDev->hSDC;
    //SdcDev_SetPower(hSdc, TRUE); //给卡供电

    SDC_Delay(100);

    if (hSdc == NULL)
    {
        return RK_PARA_ERR;
    }

    pstSdDev->type = UNKNOW_CARD;


    temp = BUS_WIDTH_1_BIT;

    ret = SdcDev_SetBusWidth(hSdc, temp);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    temp = FOD_FREQ;


    ret = SdcDev_SetBusFreq(hSdc, temp);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    ret = SdSendCmd(hSdc, (SD_GO_IDLE_STATE | SD_NODATA_OP | SD_RSP_NONE | NO_WAIT_PREV | SEND_INIT), 0, NULL);
    if (RK_SUCCESS != ret)
    {
        return RK_ERROR;
    }

    SDC_Delay(100);  //27有发现CMD0发送完以后延时一下再发其他命令能提高卡的识别率
    return _IdentifyUnknow(pstSdDev);
}


#ifdef _SD_DEV_SHELL_
_DRIVER_SD_SDDEVICE_SHELL_DATA_
static SHELL_CMD ShellSdName[] =
{
    "pcb",NULL,"NULL","NULL",
    "create",NULL,"NULL","NULL",
    "del",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SdDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_SHELL_
SHELL API rk_err_t SdDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSdName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:
            ret = SdDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = SdDevShellCreate(dev,pItem);
            break;

        case 0x02:
            ret = SdDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = SdDevShellTest(dev,pItem);
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
** Name: SdDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_SHELL_
SHELL FUN rk_err_t SdDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hSdDev;
    uint32 DevID;
    uint32 TotalSec;
    uint8 * pBuf;
    int32 BufSec;
    uint32 i, j;
    rk_err_t ret;

    //Get SdDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open SdDev...
    hSdDev = RKDev_Open(DEV_CLASS_SD, 0, NOT_CARE);
    if ((hSdDev == NULL) || (hSdDev == (HDC)RK_ERROR) || (hSdDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("SdDev open failure");
        return RK_SUCCESS;
    }

    //do test....


#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("sd.test : sd test cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    TotalSec = 0;

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

    TotalSec = 0x32000; // 100M

    rk_printf("test start = %d\n", BufSec);

    for (i = 0;  i < (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for (j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];


        ret = SdDev_Write(hSdDev, i, pBuf, BufSec);

        rk_printf("write: LBA = 0x%08x, Len = %d", i, BufSec);

        if (ret != BufSec)
        {
            rk_print_string("sd write error");
        }
    }

    if (TotalSec % BufSec != 0)
    {


        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for (j = 2; j < BufSec * 512; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];



        ret = SdDev_Write(hSdDev, i, pBuf, TotalSec % BufSec);

        rk_printf("write: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if (ret != TotalSec % BufSec)
        {
            rk_print_string("sd write error");
        }
    }

    rk_printf("test end");

    for (i = 0;  i < (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        ret = SdDev_Read(hSdDev, i, pBuf, BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, BufSec);

        if (ret != BufSec)
        {
            rk_print_string("sd read error");
        }

        if ((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
            rk_print_string("sd data error");
        }


        for (j = 2; j < BufSec * 512; j++)
        {
            if (pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                rk_print_string("sd data error");
            }
        }

    }

    if (TotalSec % BufSec != 0)
    {

        ret = SdDev_Read(hSdDev, i, pBuf, TotalSec % BufSec);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if (ret != TotalSec % BufSec)
        {
            rk_print_string("sd read error");
        }

        if ((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
            rk_print_string("sd data error");
        }


        for (j = 2; j < (TotalSec % BufSec) * 512; j++)
        {
            if (pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                rk_print_string("sd data error");
            }
        }

    }

    rkos_memory_free(pBuf);

    //close SdDev...
    RKDev_Close(hSdDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SdDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_SHELL_
SHELL FUN rk_err_t SdDevShellDel(HDC dev, uint8 * pstr)
{
    SD_DEVICE_CLASS * pstSdDev =  (SD_DEVICE_CLASS *)dev;

    if (RKDev_Delete(DEV_CLASS_SD, pstSdDev->stSdDevice.DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("SDDev delete failure");
    }
    printf("SDDev delete success");
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SdDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_SHELL_
SHELL FUN rk_err_t SdDevShellCreate(HDC dev, uint8 * pstr)
{
    SD_DEV_ARG stSdDevArg;
    rk_err_t ret;
    uint32 DevID;

    if (StrCmpA(pstr, "/0", 2) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "/1", 2) == 0)
    {
        DevID = 1;
    }
    else
    {
        return RK_ERROR;
    }

    //Init SdDev arg...
    stSdDevArg.hSdc = RKDev_Open(DEV_CLASS_SDC, 0, NOT_CARE);
    if ((stSdDevArg.hSdc == NULL) || (stSdDevArg.hSdc == (HDC)RK_ERROR) || (stSdDevArg.hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("SDC0 open failure");
        return RK_SUCCESS;
    }
    stSdDevArg.BusWidth = BUS_WIDTH_4_BIT;

    //Create SdDev...
    ret = RKDev_Create(DEV_CLASS_SD, DevID, &stSdDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("SdDev create failure");
    }
    if (RKDev_Close(stSdDevArg.hSdc) != RK_SUCCESS)
    {
        printf("sdc1 close failure\n");
        return RK_ERROR;
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SdDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.11
** Time: 16:30:47
*******************************************************************************/
_DRIVER_SD_SDDEVICE_SHELL_
SHELL FUN rk_err_t SdDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hSdDev;
    uint32 DevID;

    //Get SdDev ID...
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Display pcb...
    if (gpstSdDevISR[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("SdDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("    &stSdDevice---0x%08x", &gpstSdDevISR[DevID]->stSdDevice);
        rk_printf_no_time("    osSdOperReqSem---0x%08x", gpstSdDevISR[DevID]->osSdOperReqSem);
        //rk_printf_no_time("    usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("    usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("SdDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;

}



#endif
#endif

