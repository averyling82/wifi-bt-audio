/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Driver\I2S\I2SDevice.c
* Owner: HJ
* Date: 2014.3.10
* Time: 13:51:36
* Desc: Uart Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
*      hj     2014.3.10     13:51:36   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_I2S_I2SDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "RKOS.h"
#include "BSP.h"
#include "device.h"
#include "HW_I2s.h"
#include "i2s.h"
#include "DriverInclude.h"
#include "AudioControlTask.h"
#include "I2sDevice.h"
#include "FwAnalysis.h"
#include "ShellTask.h"
#include "global.h"
#include "powermanager.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/


typedef  struct _I2S_TX_ITEM
{
    uint32* buffer;
    uint32 size;

}I2S_TX_ITEM;


typedef  struct _I2S_RX_ITEM
{
    uint32* buffer;
    uint32 size;
}I2S_RX_ITEM;


typedef  struct _I2S_DEVICE_CLASS
{
    DEVICE_CLASS stI2SDevice;            //device father class

    pSemaphore osI2SOperReqSem;
    pSemaphore osI2SReadOperSem;
    pSemaphore osI2SWriteOperSem;
    eI2S_SELECT Channel;
    I2S_Start_t txRxMode;
    uint32      dwTxNowNum;
    uint32      dwRxNowNum;         //chad.ma add
    uint32      I2S_Status;
    uint32      I2sItemID;
    uint32      I2sRxItemID;
    I2S_TX_ITEM stTxItem[2];
    I2S_RX_ITEM stRxItem[2];
    uint32 Cnt;

    uint32      DmaCh;
    uint32      WriteBusy;
    uint32      ReadBusy;            //chad.ma add
    uint32      WriteDataFlag;
    uint32      ReadDataFlag;        //chad.ma add
    HDC         hDma;
    //uint32      dwTxRxStatus;     //chad.ma close
    uint32      dwTxStatus;         //chad.ma add
    uint32      dwRxStatus;         //chad.ma add

    I2S_mode_t I2S_mode;
    eI2sCs_t i2sCS;
    I2sFS_en_t I2S_FS;
    eI2sFormat_t BUS_FORMAT;
    eI2sDATA_WIDTH_t Data_width;
    I2S_BUS_MODE_t I2S_Bus_mode;

    eI2sFormat_t RX_BUS_FORMAT;
    eI2sDATA_WIDTH_t Rx_Data_width;
    I2S_BUS_MODE_t Rx_I2S_Bus_mode;

}I2S_DEVICE_CLASS;

/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
#define PIO_WRITE_LEN 16
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static I2S_DEVICE_CLASS * gpstI2sDevISRHandler[I2S_DEV_MAX];
uint32 pSemTemp = 0;


_DRIVER_I2S_I2SDEVICE_COMMON_DATA_
DMA_CFGX I2S0ControlDmaCfg_TX  = {DMA_CTLL_I2S0_TX, DMA_CFGL_I2S0_TX, DMA_CFGH_I2S0_TX,0};
_DRIVER_I2S_I2SDEVICE_COMMON_DATA_
DMA_CFGX I2S0ControlDmaCfg_RX  = {DMA_CTLL_I2S0_RX, DMA_CFGL_I2S0_RX, DMA_CFGH_I2S0_RX,0};
_DRIVER_I2S_I2SDEVICE_COMMON_DATA_
DMA_CFGX I2S1ControlDmaCfg_TX  = {DMA_CTLL_I2S1_TX, DMA_CFGL_I2S1_TX, DMA_CFGH_I2S1_TX,0};

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
rk_err_t I2sDevCheckHandler(HDC dev);
rk_err_t I2sDevShellTest(HDC dev, uint8 * pstr);
rk_err_t I2SShellPcb(HDC dev, uint8 * pstr);
rk_err_t I2SDevShellDel(HDC dev, uint8 * pstr);
rk_err_t I2SDevShellCreate(HDC dev, uint8 * pstr);
void I2S_RW_DmaIntIsr(void);
I2sFS_en_t I2sDevShellBspGetFs(uint8 * pstr);
void I2S_Isr_test(void);
void I2sDevShell8987_Init(void);
void I2sDevShell5633_Init(void);
rk_err_t I2sDevShellSampleRateTest(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspPIO_Read(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspPIO_Write(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspDMA_Write(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspDMA_RW(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspDeinit(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspInit(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t I2sDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t I2SShellHelp(HDC dev, uint8 * pstr);

rk_err_t I2sDevResume(HDC dev);
rk_err_t I2sDevSuspend(HDC dev, uint32 Level);
void I2s0IntIsr(void);
void I2s1IntIsr(void);
void I2sIntIsr(uint32 i2sCH);
void I2s0DMAIntIsr(void);
void I2s1DMAIntIsr(void);
void I2sDMAIntIsr(uint32 i2sCH);

void I2s0DMAIntIsr_R(void);
void I2s1DMAIntIsr_R(void);
void I2sDMAIntIsr_R(uint32 i2sCH);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------

*/

//#define _i2sTest_

#ifdef _i2sTest_
_DRIVER_I2S_I2SDEVICE_SHELL_
uint8 outptr1[32][176] =
{
    //0
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //8
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //16
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //24
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    {
#include "test01_1K0_441.data"
    },
    //32
};
_DRIVER_I2S_I2SDEVICE_COMMON_
uint32 length1 = 176 * 32 / 4;
#endif


/*******************************************************************************
** Name: I2sDevControl
** Input:DEVICE_CLASS* dev, uint8  cmd, void *args
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:53:09
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
rk_err_t I2sDev_Control(HDC dev, uint8  cmd, void *args)
{
    I2S_DEVICE_CLASS * pI2SDev =  (I2S_DEVICE_CLASS *)(dev);
    I2S_DEVICE_CONFIG_REQ_ARG * pCmdReqArg = (I2S_DEVICE_CONFIG_REQ_ARG *)args;

    rkos_semaphore_take(pI2SDev->osI2SOperReqSem, MAX_DELAY);

    if (cmd == I2S_DEVICE_INIT_CMD)
    {
        //printf("\nI2SSampleRate=%d\n",pCmdReqArg->I2S_FS);
        rkos_enter_critical();

        pI2SDev->I2S_mode = pCmdReqArg->i2smode;
        pI2SDev->i2sCS = pCmdReqArg->i2sCS;
        pI2SDev->I2S_FS = pCmdReqArg->I2S_FS;
        pI2SDev->BUS_FORMAT = pCmdReqArg->BUS_FORMAT;
        pI2SDev->Data_width = pCmdReqArg->Data_width;
        pI2SDev->I2S_Bus_mode = pCmdReqArg->I2S_Bus_mode;

        pI2SDev->RX_BUS_FORMAT = pCmdReqArg->RX_BUS_FORMAT;
        pI2SDev->Rx_Data_width = pCmdReqArg->Rx_Data_width;
        pI2SDev->Rx_I2S_Bus_mode = pCmdReqArg->Rx_I2S_Bus_mode;

        I2SInit(pI2SDev->stI2SDevice.DevID,
                pCmdReqArg->i2smode,
                pCmdReqArg->i2sCS,
                pCmdReqArg->I2S_FS,
                pCmdReqArg->BUS_FORMAT,
                pCmdReqArg->Data_width,
                pCmdReqArg->I2S_Bus_mode,
                pCmdReqArg->RX_BUS_FORMAT,
                pCmdReqArg->Rx_Data_width,
                pCmdReqArg->Rx_I2S_Bus_mode);
        rkos_exit_critical();
        rkos_semaphore_give(pI2SDev->osI2SOperReqSem);
        return RK_SUCCESS;
    }
    else if (cmd == I2S_DEVICE_RST_CMD)
    {
        //ScuSoftResetCtr(SRST_I2S, 1);
        switch ((int32)pI2SDev->stI2SDevice.DevID)
        {
            case I2S_DEV0:
                ScuClockGateCtr(CLK_I2S0_SRC_GATE, 1);
                ScuClockGateCtr(PCLK_I2S0_GATE, 1);
                ScuSoftResetCtr(I2S0_SRST, 1);

                break;
            case I2S_DEV1:
                ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);
                ScuClockGateCtr(PCLK_I2S1_GATE, 1);
                ScuSoftResetCtr(I2S1_SRST, 1);

                break;
            default:
                break;
        }
        rkos_semaphore_give(pI2SDev->osI2SOperReqSem);
        return RK_SUCCESS;
    }
    else if(cmd == I2S_DEVICE_SET_FS)
    {
        pI2SDev->I2S_FS = (I2sFS_en_t)args;

        I2SInit(pI2SDev->stI2SDevice.DevID,
                pI2SDev->I2S_mode,
                pI2SDev->i2sCS,
                pI2SDev->I2S_FS,
                pI2SDev->BUS_FORMAT,
                pI2SDev->Data_width,
                pI2SDev->I2S_Bus_mode,
                pI2SDev->RX_BUS_FORMAT,
                pI2SDev->Rx_Data_width,
                pI2SDev->Rx_I2S_Bus_mode);

        //I2sSetSampleRate(pI2SDev->stI2SDevice.DevID,
        //                 pI2SDev->I2S_FS);
        rkos_semaphore_give(pI2SDev->osI2SOperReqSem);
        return RK_SUCCESS;
    }
    else if(cmd == I2S_DEVICE_SET_DW)
    {
        pI2SDev->Data_width = (eI2sDATA_WIDTH_t)args;

        I2sSetDataW(pI2SDev->stI2SDevice.DevID,
                    pI2SDev->BUS_FORMAT,
                    pI2SDev->Data_width,
                    pI2SDev->I2S_Bus_mode);

        rkos_semaphore_give(pI2SDev->osI2SOperReqSem);
        return RK_SUCCESS;
    }
    else if(cmd == I2S_DEVICE_SET_RX_DW)
    {
        pI2SDev->Rx_Data_width = (eI2sDATA_WIDTH_t)args;

        I2sSetRxDataW(pI2SDev->stI2SDevice.DevID,
                      pI2SDev->RX_BUS_FORMAT,
                      pI2SDev->Rx_Data_width,
                      pI2SDev->Rx_I2S_Bus_mode);

        rkos_semaphore_give(pI2SDev->osI2SOperReqSem);
        return RK_SUCCESS;
    }
}

/*******************************************************************************
** Name: I2sDevRead
** Input:DEVICE_CLASS* dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:48:03
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
rk_size_t I2sDev_Read(HDC dev, void* buffer, uint32 size,uint8 mode)
{
    uint32 i;
    rk_err_t ret;
    I2S_DEVICE_CLASS * pstI2SDev =  (I2S_DEVICE_CLASS *)dev;
    uint32 realsize;
    uint32 I2sRxItemID;

    rkos_enter_critical();

    if(pstI2SDev->ReadBusy == 0)
    {
        printf("\nRead i2s idle....");
    }

    if (pstI2SDev->I2sRxItemID == 0)
    {
        //printf("/////////// i2sitemid = 0 \n");
        pstI2SDev->stRxItem[1].buffer = (uint32*) buffer;

        if(size%4)
        {
            pstI2SDev->stRxItem[1].size   = (size+3)/4;
        }
        else
        {
            pstI2SDev->stRxItem[1].size   = size/4;
        }

        //pstI2SDev->stRxItem[1].size   = size/4;
        I2sRxItemID = 1;
    }
    else
    {
        //printf("###########  i2sitemid = 1 \n");
        pstI2SDev->stRxItem[0].buffer = (uint32*) buffer;

        if(size%4)
        {
            pstI2SDev->stRxItem[0].size   = (size+3)/4;
        }
        else
        {
            pstI2SDev->stRxItem[0].size   = size/4;
        }
        //pstI2SDev->stRxItem[0].size   = size/4;
        I2sRxItemID = 0;
    }

    pstI2SDev->ReadDataFlag = 1;
    rkos_exit_critical();//0
    //printf("\n take ReadDataFlag =%d \n", pstI2SDev->ReadDataFlag);

    if(pstI2SDev->ReadBusy == 1)
    {
        pstI2SDev->Cnt++;
        //printf("\n take ReadOperSem \n");
        rkos_semaphore_take(pstI2SDev->osI2SReadOperSem, MAX_DELAY);
    }


    if(pstI2SDev->ReadDataFlag == 1)
    {
        pstI2SDev->ReadBusy = 1;
        pstI2SDev->ReadDataFlag = 0;
        pstI2SDev->I2sRxItemID = I2sRxItemID;


        if (mode == SYNC_MODE)
        {

        }
        else if (mode == ASYNC_MODE)
        {
            if ((((uint32)buffer % 4) == 0)
                        && ((size % 4) == 0)
                        && (size > (I2S_FIFO_DEPTH * 4)))
            {

                //rk_err_t chl;
                pFunc pCallBack;

                if (pstI2SDev->stI2SDevice.DevID == I2S_DEV0)
                {
                    pCallBack = I2s0DMAIntIsr_R;
                }
                else if(pstI2SDev->stI2SDevice.DevID < I2S_DEV_MAX)
                {
                    pCallBack = I2s1DMAIntIsr_R;
                }

                //printf("read buffer addr = 0x%08x \n",pstI2SDev->stRxItem[i2sItemID].buffer);
                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma,
                                         AUDIO_READ_DMACHANNEL_IIS,
                                         I2sGetRxFIFOaddr(pstI2SDev->stI2SDevice.DevID),
                                         (uint32)pstI2SDev->stRxItem[I2sRxItemID].buffer,
                                         pstI2SDev->stRxItem[I2sRxItemID].size,
                                         &I2S0ControlDmaCfg_RX,
                                         pCallBack);

                if (ret != RK_SUCCESS)
                {
                    rkos_semaphore_give(pstI2SDev->osI2SReadOperSem);
                    return (rk_size_t)RK_ERROR;
                }

                #if 0
                if((pstI2SDev->txRxMode == I2S_START_DMA_TX) || (pstI2SDev->txRxMode = I2S_START_PIO_RTX))
                {
                    pstI2SDev->txRxMode = I2S_START_DMA_RTX;
                }
                else
                {
                    pstI2SDev->txRxMode = I2S_START_DMA_RX;
                }
                #else
                    pstI2SDev->txRxMode = I2S_START_DMA_RTX;
                #endif

                pstI2SDev->dwRxStatus = DEVICE_STATUS_ASYNC_DMA_READ;
                //printf("DMA read..txRxMode:%d\n\n\n\n\n",pstI2SDev->txRxMode);
                I2SDMAEnable(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_RX);
                I2SStart(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_RX);
                I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_FIFO_FULL);
            }
            else
            {
                #if 0
                if((pstI2SDev->txRxMode == I2S_START_PIO_TX) || (pstI2SDev->txRxMode = I2S_START_PIO_RTX))
                {
                    pstI2SDev->txRxMode = I2S_START_PIO_RTX;
                }
                else
                {
                    pstI2SDev->txRxMode = I2S_START_PIO_RX;
                }
                #else
                    pstI2SDev->txRxMode = I2S_START_PIO_RTX;
                #endif

                //pstI2SDev->txRxMode = I2S_START_PIO_RX;
                pstI2SDev->dwTxStatus = DEVICE_STATUS_ASYNC_PIO_READ;
                pstI2SDev->dwRxNowNum = I2S_PIO_Read(pstI2SDev->stI2SDevice.DevID,pstI2SDev->stRxItem[I2sRxItemID].buffer,(pstI2SDev->stRxItem[I2sRxItemID].size));

                if (pstI2SDev->I2S_Status == I2S_DEVICE_STOP)
                {
                    pstI2SDev->I2S_Status = I2S_DEVICE_START;
                    I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_RX);
                    I2SStart(pstI2SDev->stI2SDevice.DevID,pstI2SDev->txRxMode);
                }

                if (pstI2SDev->dwRxNowNum == pstI2SDev->stRxItem[I2sRxItemID].size)
                {
                    //rkos_semaphore_give(pstI2SDev->osI2SReadOperSem);
                    return RK_SUCCESS;
                }
                else if (pstI2SDev->dwRxNowNum < pstI2SDev->stRxItem[I2sRxItemID].size)
                {
                    pstI2SDev->stRxItem[I2sRxItemID].buffer += pstI2SDev->dwRxNowNum;
                    I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_RX);
                }
            }

            return RK_SUCCESS;
        }
    }
    //cjh //I2S_PIO_Read(eI2s_DEV I2S_DEV,uint32* buffer,uint32 size)
    //rkos_semaphore_give(pstI2SDev->osI2SReadOperSem);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: I2sDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.5
** Time: 17:58:08
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
COMMON FUN rk_err_t I2sDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < I2S_DEV_MAX; i++)
    {
        if(gpstI2sDevISRHandler[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: I2sDMAIntIsr
** Input:i2sCH
** Return: void
** Owner:chad.ma
** Date: 2016.01.12
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2sDMAIntIsr_R(uint32 i2sCH)
{
    int32 intstatus;
    uint32* buffer;
    uint32 IntEvent;
    rk_size_t realsize = 0;
    int32 ackcorestatus, ret;
    pFunc pCallBack;
    I2S_DEVICE_CLASS * pstI2SDev =  gpstI2sDevISRHandler[i2sCH];

    if (pstI2SDev != NULL)
    {
        pstI2SDev->ReadBusy = 0;
        //printf("read I2sDMAIntIsr_R ReadDataFlag=%d \n", pstI2SDev->ReadDataFlag);
        if(pstI2SDev->ReadDataFlag)
        {
            if (pstI2SDev->stI2SDevice.DevID == I2S_DEV0)
            {
                pCallBack = I2s0DMAIntIsr_R;
            }
            else if(pstI2SDev->stI2SDevice.DevID < I2S_DEV_MAX)
            {
                pCallBack = I2s1DMAIntIsr_R;
            }

            if (pstI2SDev->I2sRxItemID == 0)
            {
                //printf(" Rx size = %d\n", pstI2SDev->stRxItem[1].size);


                //printf("I2sDMAIntIsr_R DMAstart i2dItemID = 0 \n" );

                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma,
                                         AUDIO_READ_DMACHANNEL_IIS,
                                         I2sGetRxFIFOaddr(pstI2SDev->stI2SDevice.DevID),
                                         (uint32)pstI2SDev->stRxItem[1].buffer,
                                         pstI2SDev->stRxItem[1].size,
                                         &I2S0ControlDmaCfg_RX,
                                         pCallBack);
                pstI2SDev->I2sRxItemID = 1;
            }
            else
            {
                //printf(" Rx size = %d\n", pstI2SDev->stRxItem[0].size);

                //printf("I2sDMAIntIsr_R  DMAstart i2dItemID = 1\n");

                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma,
                                         AUDIO_READ_DMACHANNEL_IIS,
                                         I2sGetRxFIFOaddr(pstI2SDev->stI2SDevice.DevID),
                                         (uint32)pstI2SDev->stRxItem[0].buffer,
                                          pstI2SDev->stRxItem[0].size,
                                         &I2S0ControlDmaCfg_RX,
                                         pCallBack);

                pstI2SDev->I2sRxItemID = 0;
            }

            if (ret != RK_SUCCESS)
            {
                //printf("DmaDev_DmaStartIIS isr Error\n");
                return;
            }

            pstI2SDev->ReadDataFlag = 0;
            pstI2SDev->ReadBusy     = 1;

            pstI2SDev->Cnt--;
            rkos_semaphore_give_fromisr(pstI2SDev->osI2SReadOperSem);
        }
        else
        {
            //printf("\n\n dis DMA_RX \n\n");
            I2SDMADisable(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_RX);
            #ifndef _BROAD_LINE_OUT_
            I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_FIFO_FULL);
            #else
            I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_RX);
            #endif
        }
    }
}

/*******************************************************************************
** Name: I2s0DMAIntIsr
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2016.01.12
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s0DMAIntIsr_R(void)
{
    //printf("I2s0DMAIntIsr_R in\n");
    I2sDMAIntIsr_R(I2S_DEV0);
}

/*******************************************************************************
** Name: I2s1DMAIntIsr
** Input:void
** Return: void
** Owner:chad.ma
** Date: 2016.01.12
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s1DMAIntIsr_R(void)
{
    I2sDMAIntIsr_R(I2S_DEV1);
}

/*******************************************************************************
** Name: I2sRxDev_Idle
** Input:HDC dev
** Return: rk_size_t
** Owner:cjh
** Date: 2014.2.17
** Time: 13:46:51
*******************************************************************************/
rk_err_t I2sRxDev_Idle(HDC dev)
{
    I2S_DEVICE_CLASS * pstI2SDev =  (I2S_DEVICE_CLASS *)(dev);
    if(pstI2SDev->ReadBusy == 0)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}

rk_err_t I2sDev_Idle(HDC dev)
{
    I2S_DEVICE_CLASS * pstI2SDev =  (I2S_DEVICE_CLASS *)(dev);
    if(pstI2SDev->WriteBusy == 0)
    {
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }
}


/*******************************************************************************
** Name: I2sDevWrite
** Input:DEVICE_CLASS* dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_size_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:46:51
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
rk_err_t I2sDev_Write(HDC dev, uint32* buffer, uint32 size, uint8 mode)
{
    I2S_DEVICE_CLASS * pstI2SDev =  (I2S_DEVICE_CLASS *)(dev);

    rk_err_t ret;
    rk_size_t realsize = 0;
    uint32 i2sItemID;

    rkos_enter_critical();

    if(pstI2SDev->WriteBusy == 0)
    {
        printf("\nWrite i2s idle....");
    }
    //printf("\nWrite WriteBusy =%d", pstI2SDev->WriteBusy);
    if (pstI2SDev->I2sItemID == 0)
    {
        pstI2SDev->stTxItem[1].buffer = (uint32*) buffer;
        if(size%4)
        {
            pstI2SDev->stTxItem[1].size   = (size+3)/4;
        }
        else
        {
            pstI2SDev->stTxItem[1].size   = size/4;
        }
        i2sItemID = 1;
    }
    else
    {
        pstI2SDev->stTxItem[0].buffer = (uint32*) buffer;
        if(size%4)
        {
            pstI2SDev->stTxItem[0].size   = (size+3)/4;
        }
        else
        {
            pstI2SDev->stTxItem[0].size   = size/4;
        }
        i2sItemID = 0;
    }

    pstI2SDev->WriteDataFlag = 1;

    rkos_exit_critical();//0

    if(pstI2SDev->WriteBusy == 1)
    {
        pstI2SDev->Cnt++;
        //printf("WriteBusy take WriteOperSem.....\n");
        rkos_semaphore_take(pstI2SDev->osI2SWriteOperSem, MAX_DELAY);
    }

    if(pstI2SDev->WriteDataFlag == 1)
    {
        pstI2SDev->WriteBusy = 1;
        pstI2SDev->WriteDataFlag = 0;
        pstI2SDev->I2sItemID = i2sItemID;


        if (mode == SYNC_MODE)
        {
            return RK_SUCCESS;
        }
        else if (mode == ASYNC_MODE)
        {
            if ((((uint32)buffer % 4) == 0)
                    && ((size % 4) == 0)
                    && (size > (I2S_FIFO_DEPTH * 4)))
            {
                pFunc pCallBack;

                if (pstI2SDev->stI2SDevice.DevID == I2S_DEV0)
                {
                    pCallBack = I2s0DMAIntIsr;
                }
                else if(pstI2SDev->stI2SDevice.DevID < I2S_DEV_MAX)
                {
                    pCallBack = I2s1DMAIntIsr;
                }
                //printf("\nwrite buffer addr = 0x%08x \n",pstI2SDev->stTxItem[i2sItemID].buffer);
                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma, AUDIO_DMACHANNEL_IIS, (uint32)pstI2SDev->stTxItem[i2sItemID].buffer,
                      I2sGetTxFIFOaddr(pstI2SDev->stI2SDevice.DevID), pstI2SDev->stTxItem[i2sItemID].size,
                      &I2S0ControlDmaCfg_TX, pCallBack);

                if (ret != RK_SUCCESS)
                {
                    rkos_semaphore_give(pstI2SDev->osI2SWriteOperSem);
                    return (rk_size_t)RK_ERROR;
                }

                #if 0
                if((pstI2SDev->txRxMode == I2S_START_DMA_TX) || (pstI2SDev->txRxMode = I2S_START_DMA_RTX))
                {
                    pstI2SDev->txRxMode = I2S_START_DMA_RTX;
                }
                else
                {
                    pstI2SDev->txRxMode = I2S_START_DMA_TX;
                }
                #else
                    pstI2SDev->txRxMode = I2S_START_DMA_RTX;
                #endif

                pstI2SDev->dwTxStatus = DEVICE_STATUS_ASYNC_DMA_WRITE;

                //printf("DMA WRITE..txRxMode:%d\n\n\n\n\n",pstI2SDev->txRxMode);
                I2SDMAEnable(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_TX);
                I2SStart(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_TX);
                I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_FIFO_EMPTY);
                //rk_printf("write I2S_INT_FIFO_EMPTY:%d\n\n\n\n");

            }
            else
            {
                //rk_printf("PIO write..size:%d\n\n\n\n",size);
                if((pstI2SDev->txRxMode == I2S_START_PIO_RX) || (pstI2SDev->txRxMode = I2S_START_PIO_RTX))
                {
                    pstI2SDev->txRxMode = I2S_START_PIO_RTX;
                }
                else
                {
                    pstI2SDev->txRxMode = I2S_START_PIO_TX;
                }

                pstI2SDev->dwTxStatus = DEVICE_STATUS_ASYNC_PIO_WRITE;
                pstI2SDev->dwTxNowNum = I2S_PIO_Write(pstI2SDev->stI2SDevice.DevID,pstI2SDev->stTxItem[i2sItemID].buffer,((pstI2SDev->stTxItem[i2sItemID].size+3)/4));

                if (pstI2SDev->I2S_Status == I2S_DEVICE_STOP)
                {
                    pstI2SDev->I2S_Status = I2S_DEVICE_START;
                    I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_TX);
                    I2SStart(pstI2SDev->stI2SDevice.DevID,pstI2SDev->txRxMode);
                }

                if (pstI2SDev->dwTxNowNum == pstI2SDev->stTxItem[i2sItemID].size)
                {
                    return RK_SUCCESS;
                }
                else if (pstI2SDev->dwTxNowNum < pstI2SDev->stTxItem[i2sItemID].size)
                {
                    pstI2SDev->stTxItem[i2sItemID].buffer += pstI2SDev->dwTxNowNum;
                    I2SIntEnable(pstI2SDev->stI2SDevice.DevID,I2S_INT_TX);
                }
            }
            return RK_SUCCESS;
        }

        return RK_ERROR;
    }
}


/*******************************************************************************
** Name: I2sDMAIntIsr
** Input:i2sCH
** Return: void
** Owner:hj
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2sDMAIntIsr(uint32 i2sCH)
{
    int32 intstatus;
    uint32* buffer;
    uint32 IntEvent;
    rk_size_t realsize = 0;
    int32 ackcorestatus, ret;
    pFunc pCallBack;
    I2S_DEVICE_CLASS * pstI2SDev =  gpstI2sDevISRHandler[i2sCH];
    //printf("write I2sDMAIntIsr i2sCH=%d \n", i2sCH);
    if (pstI2SDev != NULL)
    {
        pstI2SDev->WriteBusy = 0;
        if(pstI2SDev->WriteDataFlag)
        {
            //I2SDMADisable(pstI2SDev->stI2SDevice.DevID,pstI2SDev->txRxMode);
            //DelayMs(50);

            if (pstI2SDev->stI2SDevice.DevID == I2S_DEV0)
            {
                pCallBack = I2s0DMAIntIsr;
            }
            else if(pstI2SDev->stI2SDevice.DevID < I2S_DEV_MAX)
            {
                pCallBack = I2s1DMAIntIsr;
            }

            if (pstI2SDev->I2sItemID == 0)
            {
                //printf("[1]size=%d buffer=0x%x hDma=0x%x TxFIFOaddr = 0x%x\n", pstI2SDev->stTxItem[1].size,pstI2SDev->stTxItem[1].buffer,pstI2SDev->hDma,I2sGetTxFIFOaddr(pstI2SDev->stI2SDevice.DevID));
                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma, AUDIO_DMACHANNEL_IIS, (uint32)pstI2SDev->stTxItem[1].buffer,
                      I2sGetTxFIFOaddr(pstI2SDev->stI2SDevice.DevID), pstI2SDev->stTxItem[1].size,
                      &I2S0ControlDmaCfg_TX, pCallBack);
                pstI2SDev->I2sItemID = 1;
            }
            else
            {
                //printf("[0]size=%d buffer=0x%x hDma=0x%x TxFIFOaddr = 0x%x\n", pstI2SDev->stTxItem[0].size,pstI2SDev->stTxItem[0].buffer,pstI2SDev->hDma,I2sGetTxFIFOaddr(pstI2SDev->stI2SDevice.DevID));
                ret = DmaDev_DmaStartIIS(pstI2SDev->hDma, AUDIO_DMACHANNEL_IIS, (uint32)pstI2SDev->stTxItem[0].buffer,
                      I2sGetTxFIFOaddr(pstI2SDev->stI2SDevice.DevID), pstI2SDev->stTxItem[0].size,
                      &I2S0ControlDmaCfg_TX, pCallBack);

                pstI2SDev->I2sItemID = 0;
            }

            if (ret != RK_SUCCESS)
            {
                return;
            }

            pstI2SDev->WriteDataFlag = 0;
            pstI2SDev->WriteBusy = 1;

            #if 0
            if (pstI2SDev->txRxMode == I2S_START_DMA_RX)
            {
                pstI2SDev->txRxMode = I2S_START_DMA_RTX;
            }
            else
            {
                pstI2SDev->txRxMode = I2S_START_DMA_TX;
            }
            #endif

            //I2SDMAEnable(pstI2SDev->stI2SDevice.DevID,pstI2SDev->txRxMode);

            pstI2SDev->Cnt--;
            rkos_semaphore_give_fromisr(pstI2SDev->osI2SWriteOperSem);
            //printf("give WriteOperSem \n");
        }
        else
        {
            //printf("\n\n dis DMA_TX \n\n");
            I2SDMADisable(pstI2SDev->stI2SDevice.DevID,I2S_START_DMA_TX);
            #ifndef _BROAD_LINE_OUT_
            I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_FIFO_EMPTY);
            #else
            I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_TX);
            #endif
        }
    }
}


/*******************************************************************************
** Name: I2s0DMAIntIsr
** Input:void
** Return: void
** Owner:HJ
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s0DMAIntIsr(void)
{
    I2sDMAIntIsr(I2S_DEV0);
}

/*******************************************************************************
** Name: I2s1DMAIntIsr
** Input:void
** Return: void
** Owner:HJ
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s1DMAIntIsr(void)
{
    I2sDMAIntIsr(I2S_DEV1);
}
/*******************************************************************************
** Name: I2sIntIsr
** Input:i2sCH
** Return: void
** Owner:hj
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2sIntIsr(uint32 i2sCH)
{
    int32 intstatus;
    uint32* buffer;
    uint32 IntEvent;
    rk_size_t realsize = 0;
    int32 ackcorestatus;
    uint32 i2sItemID;
    uint32 I2sRxItemID;
    I2S_DEVICE_CLASS * pstI2SDev =  gpstI2sDevISRHandler[i2sCH];

    if(pstI2SDev->I2sItemID == 0)
    {
        i2sItemID = 1;
    }
    else
    {
        i2sItemID = 0;
    }

    if(pstI2SDev->I2sRxItemID == 0)
    {
        I2sRxItemID = 1;
    }
    else
    {
        I2sRxItemID = 0;
    }

    IntEvent = I2SGetIntType(pstI2SDev->stI2SDevice.DevID);
    //printf("enter I2sIntIsr i2s:%d",pstI2SDev->stI2SDevice.DevID);
    if (IntEvent & TX_interrupt_active)
    {
        //printf("i2s TX_interrupt_active\n");
         if ((pstI2SDev->dwTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                    || (pstI2SDev->dwTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
        {
            if (pstI2SDev->stTxItem[i2sItemID].size == pstI2SDev->dwTxNowNum)
            {
                pstI2SDev->WriteBusy = 0;
                I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_TX);
                //printf("pio give WriteOperSem.....\n");
                rkos_semaphore_give_fromisr(pstI2SDev->osI2SWriteOperSem);

            }
            else if (pstI2SDev->stTxItem[i2sItemID].size > pstI2SDev->dwTxNowNum)
            {
                realsize = I2S_PIO_Write(pstI2SDev->stI2SDevice.DevID,pstI2SDev->stTxItem[i2sItemID].buffer,pstI2SDev->stTxItem[i2sItemID].size - pstI2SDev->dwTxNowNum);
                //pstI2SDev->dwTxNowNum += realsize;
                //printf("pstI2SDev->dwTxNowNum =%d\n",pstI2SDev->dwTxNowNum);
                pstI2SDev->stTxItem[i2sItemID].buffer += realsize;
                pstI2SDev->dwTxNowNum += realsize;
            }
        }
    }

    //chad.ma add
    if (IntEvent & RX_interrupt_active)
    {
         if ((pstI2SDev->dwRxStatus == DEVICE_STATUS_ASYNC_PIO_READ)
                    || (pstI2SDev->dwRxStatus == DEVICE_STATUS_SYNC_PIO_READ))
        {
            if (pstI2SDev->stRxItem[I2sRxItemID].size == pstI2SDev->dwRxNowNum)
            {
                pstI2SDev->ReadBusy = 0;
                I2SIntDisable(pstI2SDev->stI2SDevice.DevID,I2S_INT_RX);
                //printf("pio give ReadOperSem");
                rkos_semaphore_give_fromisr(pstI2SDev->osI2SReadOperSem);

            }
            else if (pstI2SDev->stRxItem[I2sRxItemID].size > pstI2SDev->dwRxNowNum)
            {
                uint32 leftsize;

                leftsize = pstI2SDev->stRxItem[I2sRxItemID].size - pstI2SDev->dwRxNowNum;
                realsize = I2S_PIO_Read(pstI2SDev->stI2SDevice.DevID,
                                        pstI2SDev->stRxItem[I2sRxItemID].buffer,
                                        leftsize);
                pstI2SDev->stRxItem[I2sRxItemID].buffer += realsize;
                pstI2SDev->dwRxNowNum += realsize;
            }
        }
    }

    if(IntEvent & TX_interrupt_fifo_empty)
    {
        //printf("i2s Tx fifo empty 0x%x size =%d dwTxNowNum =%d\n",IntEvent, pstI2SDev->stTxItem[i2sItemID].size, pstI2SDev->dwTxNowNum);
    }

    if(IntEvent & RX_interrupt_fifo_full)
    {
        //printf("i2s Rx fifo full 0x%x size =%d dwTxNowNum =%d\n",IntEvent, pstI2SDev->stRxItem[I2sRxItemID].size, pstI2SDev->dwRxNowNum);
    }
}
/*******************************************************************************
** Name: I2s0IntIsr
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s0IntIsr(void)
{
    I2sIntIsr(I2S_DEV0);
}

/*******************************************************************************
** Name: I2s1IntIsr
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_COMMON_
void I2s1IntIsr(void)
{
    I2sIntIsr(I2S_DEV1);
}

/*******************************************************************************
** Name: I2sDevCreate
** Input:void
** Return: DEVICE_CLASS *
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:40:31
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
HDC I2SDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS* pstI2SDevPublicHandler;
    I2S_DEVICE_CLASS * psI2SDevHandler;
    I2S_DEV_ARG *stI2sDevArg = (I2S_DEV_ARG *)arg;
    //rkos_enter_critical();
    //application UartDEVICE struct

    psI2SDevHandler =  rkos_memory_malloc(sizeof(I2S_DEVICE_CLASS));
    if (psI2SDevHandler == NULL)
    {
        return psI2SDevHandler;
    }

    psI2SDevHandler->osI2SOperReqSem     = rkos_semaphore_create(1,1);
    psI2SDevHandler->osI2SWriteOperSem = rkos_semaphore_create(1,0);
    psI2SDevHandler->osI2SReadOperSem = rkos_semaphore_create(1,0);

    if ((psI2SDevHandler->osI2SOperReqSem && psI2SDevHandler->osI2SReadOperSem
               && psI2SDevHandler->osI2SWriteOperSem) == 0)
    {

        rkos_semaphore_delete(psI2SDevHandler->osI2SOperReqSem);
        rkos_semaphore_delete(psI2SDevHandler->osI2SReadOperSem);
        rkos_semaphore_delete(psI2SDevHandler->osI2SWriteOperSem);

        rkos_memory_free(psI2SDevHandler);

        return (HDC) RK_ERROR;
    }

    psI2SDevHandler->txRxMode = I2S_START_NULL;
    psI2SDevHandler->I2S_Status = I2S_DEVICE_STOP;
    psI2SDevHandler->WriteBusy   = 0;
    psI2SDevHandler->ReadBusy = 0;
    pstI2SDevPublicHandler = (DEVICE_CLASS *)psI2SDevHandler;

    pstI2SDevPublicHandler->Idle1EventTime = 0;
    pstI2SDevPublicHandler->SuspendMode = ENABLE_MODE;

    pstI2SDevPublicHandler->suspend = I2sDevSuspend;
    pstI2SDevPublicHandler->resume  = I2sDevResume;
    pstI2SDevPublicHandler->DevID   = DevID;

    psI2SDevHandler->hDma = stI2sDevArg->hDma;
    psI2SDevHandler->Channel = stI2sDevArg->Channel;

    psI2SDevHandler->I2S_mode = stI2sDevArg->req_arg.i2smode;
    psI2SDevHandler->i2sCS = stI2sDevArg->req_arg.i2sCS;
    psI2SDevHandler->I2S_FS = stI2sDevArg->req_arg.I2S_FS;
    psI2SDevHandler->Data_width = stI2sDevArg->req_arg.Data_width;
    psI2SDevHandler->BUS_FORMAT = stI2sDevArg->req_arg.BUS_FORMAT;
    psI2SDevHandler->I2S_Bus_mode = stI2sDevArg->req_arg.I2S_Bus_mode;

    psI2SDevHandler->Rx_Data_width = stI2sDevArg->req_arg.Data_width;
    psI2SDevHandler->RX_BUS_FORMAT = stI2sDevArg->req_arg.BUS_FORMAT;
    psI2SDevHandler->Rx_I2S_Bus_mode = stI2sDevArg->req_arg.I2S_Bus_mode;


    I2SDevHwInit(DevID, psI2SDevHandler->Channel);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_I2S_DEV, SEGMENT_OVERLAY_ALL);
#endif

    //pSemTemp = (uint32)psI2SDevHandler->osI2SOperSem;
    //printf("pSemTemp = %x\n", pSemTemp);

    if (I2sDevInit(psI2SDevHandler) != RK_SUCCESS)
    {
        rkos_semaphore_delete(psI2SDevHandler->osI2SOperReqSem);
        rkos_semaphore_delete(psI2SDevHandler->osI2SReadOperSem);
        rkos_semaphore_delete(psI2SDevHandler->osI2SWriteOperSem);

        rkos_memory_free(psI2SDevHandler);
        gpstI2sDevISRHandler[pstI2SDevPublicHandler->DevID] = NULL;
        printf("create RockCodecDev fail");
        return (HDC) RK_ERROR;
    }
    gpstI2sDevISRHandler[pstI2SDevPublicHandler->DevID] = psI2SDevHandler;

    return psI2SDevHandler;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: I2sDevResume
** Input:DEVICE_CLASS *
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:57:19
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
rk_err_t I2sDevResume(HDC dev)
{
     I2S_DEVICE_CLASS * pstI2sDev = (I2S_DEVICE_CLASS *)dev;
     if(I2sDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }
     I2SDevHwInit(pstI2sDev->stI2SDevice.DevID, pstI2sDev->Channel);
     I2sDevInit(pstI2sDev);
     RKDev_Resume(pstI2sDev->hDma);
     pstI2sDev->stI2SDevice.State = DEV_STATE_WORKING;
     return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2sDevSuspend
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:56:05
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
rk_err_t I2sDevSuspend(HDC dev, uint32 Level)
{
    I2S_DEVICE_CLASS * pstI2sDev = (I2S_DEVICE_CLASS *)dev;
    if(I2sDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstI2sDev->stI2SDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstI2sDev->stI2SDevice.State = DEV_SATE_IDLE2;
    }

    while(I2sRxDev_Idle(pstI2sDev) == RK_ERROR)
    {

    }

    while(I2sDev_Idle(pstI2sDev) == RK_ERROR)
    {

    }

    I2sDevDeInit(pstI2sDev);
    I2sDevHwDeInit(pstI2sDev->stI2SDevice.DevID, pstI2sDev->Channel);
    RKDev_Suspend(pstI2sDev->hDma);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2sDevDelete
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
rk_err_t I2SDev_Delete(uint32 DevID, void * arg)
{
    I2S_DEV_ARG *stI2Sarg = arg;

#if 1
    while(I2sRxDev_Idle(gpstI2sDevISRHandler[DevID]) == RK_ERROR)
    {

    }

    while(I2sDev_Idle(gpstI2sDevISRHandler[DevID]) == RK_ERROR)
    {

    }

    stI2Sarg->hDma = gpstI2sDevISRHandler[DevID]->hDma;

    I2sDevDeInit(gpstI2sDevISRHandler[DevID]);
    I2sDevHwDeInit(gpstI2sDevISRHandler[DevID]->stI2SDevice.DevID, gpstI2sDevISRHandler[DevID]->Channel);

    rkos_semaphore_delete( gpstI2sDevISRHandler[DevID]->osI2SOperReqSem );
    rkos_semaphore_delete(gpstI2sDevISRHandler[DevID]->osI2SReadOperSem);
    rkos_semaphore_delete(gpstI2sDevISRHandler[DevID]->osI2SWriteOperSem);
    rkos_memory_free(gpstI2sDevISRHandler[DevID]);

    gpstI2sDevISRHandler[DevID] = NULL;
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_I2S_DEV);
#endif
#endif
    return RK_SUCCESS;
}



/*******************************************************************************
** Name: I2sDevDeInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:38:30
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
rk_err_t I2sDevDeInit(HDC dev)
{
    I2S_DEVICE_CLASS * pstI2SDev =  (I2S_DEVICE_CLASS *)dev;

    //I2SStop(pstI2SDev->txRxMode);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: I2sDevInit
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:44:46
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
rk_err_t I2sDevInit(I2S_DEVICE_CLASS * pstI2SDev)
{
    int32  ret = RK_SUCCESS;
    eI2s_DEV i2sDEV;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;

    if (NULL == pstI2SDev )
        return RK_PARA_ERR;

    stI2sDevArg.i2smode = pstI2SDev->I2S_mode;//;I2S_SLAVE_MODE
    stI2sDevArg.i2sCS = pstI2SDev->i2sCS; //I2S_EXT
    stI2sDevArg.I2S_FS = pstI2SDev->I2S_FS;//I2S_FS_44100Hz;
    stI2sDevArg.Data_width = pstI2SDev->Data_width;
    stI2sDevArg.BUS_FORMAT = pstI2SDev->BUS_FORMAT;
    stI2sDevArg.I2S_Bus_mode = pstI2SDev->I2S_Bus_mode;

    stI2sDevArg.Rx_Data_width = pstI2SDev->Data_width;
    stI2sDevArg.RX_BUS_FORMAT = pstI2SDev->BUS_FORMAT;
    stI2sDevArg.Rx_I2S_Bus_mode = pstI2SDev->I2S_Bus_mode;

#ifdef CODEC_24BIT //24bit
    ret = I2sDev_Control(pstI2SDev, I2S_DEVICE_INIT_CMD, &stI2sDevArg);
#else
    //ACodec_Set_I2S_Mode(TFS_TX_I2S_MODE,I2S_DATA_WIDTH16,IBM_TX_BUS_MODE_NORMAL,I2S_MST_MASTER);
    ret = I2sDev_Control(pstI2SDev, I2S_DEVICE_INIT_CMD, &stI2sDevArg);
#endif
    return ret;
}

#ifdef _I2S_DEV_SHELL_

_DRIVER_I2S_I2SDEVICE_SHELL_DATA_
static SHELL_CMD ShellI2SName[] =
{
    "pcb",I2SShellPcb,"I2s.pcb show I2s pcb info","NULL",
    "create",I2SDevShellCreate,"create I2s device","NULL",
    "delete",I2SDevShellDel,"delete I2s device","NULL",
    "test",I2sDevShellTest,"I2s test","NULL",
    "help",I2SShellHelp,"list for I2s shell cmd","NULL",
    "\b",NULL,"NULL","NULL",                          // the end
};

#ifdef SHELL_BSP
static SHELL_CMD ShellI2sBspName[] =
{
    "help",NULL,"NULL","NULL",
    "init",NULL,"NULL","NULL",
    "deinit",NULL,"NULL","NULL",
    "dmaw",NULL,"NULL","NULL",
    "dmar",NULL,"NULL","NULL",
    "piow",NULL,"NULL","NULL",
    "pior",NULL,"NULL","NULL",
    "SampleRate",NULL,"NULL","NULL",
    "alc5633q",NULL,"NULL","NULL",,//8
    "\b",NULL,"NULL","NULL",
};
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: I2SShell
** Input:HDC dev,  uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/

I2S_mode_t I2S_TEST_MODE;
eI2s_DEV I2S_DEV_TEST;

_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL API rk_err_t I2SShell(HDC dev,  uint8 * pstr)
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

    ret = ShellCheckCmd(ShellI2SName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                  //remove '.',the point is the useful item

    ShellI2SName[i].ShellCmdParaseFun(dev, pItem);
/*
    switch (i)
    {
        case 0x00:  //pcb
            ret = I2SShellPcb(dev,pItem);
            break;

        case 0x01:  //open
            ret = I2SShellOpen(dev,pItem);
            break;

        case 0x02:  //test
            ret = I2sDevShellTest(dev,pItem);
            break;

        case 0x03:  //bsp
#ifdef SHELL_BSP
            ret = I2sDevShellBsp(dev,pItem);
#endif
            break;

        case 0x04:  //help
#ifdef SHELL_HELP
            ret = I2SShellHelp(dev,pItem);
#endif
            break;

        case 0x05:  //resume
            //ret = I2SShellResume(dev,pItem);
            break;

        case 0x06:  //read
            //ret = I2SShellRead(dev,pItem);
            break;

        case 0x07:  //write
            //ret = I2SShellWrite(dev,pItem);
            break;

        case 0x08:  //control
            //ret = I2SShellControl(dev,pItem);
            break;


        default:
            ret = RK_ERROR;
            break;
    }
*/
    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define     I2S_8987CAdress       (0x1C << 1)

#define       I2S_WM8987_R0           0x00      //Left input volume
#define       I2S_WM8987_R1           0x01      //Right input volume
#define       I2S_WM8987_R2           0x02      //Lout1 volume
#define       I2S_WM8987_R3           0x03      //Rout1 volume
#define       I2S_WM8987_R4           0x04      //Reserved
#define       I2S_WM8987_R5           0x05      //ADC&DAC control
#define       I2S_WM8987_R6           0x06      //Reserved
#define       I2S_WM8987_R7           0x07      //Audio interface
#define       I2S_WM8987_R8           0x08      //Sample rate
#define       I2S_WM8987_R9           0x09      //Reserved
#define       I2S_WM8987_R10          0x0a      //Left DAC volume
#define       I2S_WM8987_R11          0x0b      //Right DAC volume
#define       I2S_WM8987_R12          0x0c      //Bass control
#define       I2S_WM8987_R13          0x0d      //Treble control
#define       I2S_WM8987_R15          0x0f      //Reset
#define       I2S_WM8987_R16          0x10     // //3D control
#define       I2S_WM8987_R17          0x11      //ALC1
#define       I2S_WM8987_R18          0x12      //ALC2
#define       I2S_WM8987_R19          0x13      //ALC3
#define       I2S_WM8987_R20          0x14      //Noise Gate
#define       I2S_WM8987_R21          0x15      //Left ADC volume
#define       I2S_WM8987_R22          0x16      //Right ADC volume
#define       I2S_WM8987_R23          0x17      //Additional control1
#define       I2S_WM8987_R24          0x18      //Additional control2
#define       I2S_WM8987_R25          0x19      //Pwr Mgmt1
#define       I2S_WM8987_R26          0x1a      //Pwr Mgmt2
#define       I2S_WM8987_R27          0x1b      //Additional control3
#define       I2S_WM8987_R31          0x1f      //ADC input mode
#define       I2S_WM8987_R32          0x20      //ADCL signal path
#define       I2S_WM8987_R33          0x21      //ADCR signal path
#define       I2S_WM8987_R34          0x22      //Left out mix1
#define       I2S_WM8987_R35          0x23      //Left out mix2
#define       I2S_WM8987_R36          0x24      //Right out Mix1
#define       I2S_WM8987_R37          0x25      //Right out Mix2
#define       I2S_WM8987_R38          0x26      //Mono out mix1
#define       I2S_WM8987_R39          0x27      //Mono out mix2
#define       I2S_WM8987_R40          0x28      //Lout2 volume
#define       I2S_WM8987_R41          0x29      //Rout2 volume
#define       I2S_WM8987_R42          0x2a      //Monoout volume

//WM8987_R7
#define   I2S_WM_MASTER_MODE 0x40
#define   I2S_WM_SLAVE_MODE 0x00
#define  I2S_WM_16BIT_MODE (0x0<<2)
#define  I2S_WM_24BIT_MODE (0x2<<2)
#define  I2S_WM_DSP_MODE  0x03
#define     I2S_WM_I2S_MODE  0x02

//WM8987_R8
#define   I2S_WM_BCKdiv0               (0<<7)
#define   I2S_WM_BCKdiv4               (1<<7)
#define   I2S_WM_BCKdiv8               (2<<7)
#define   I2S_WM_CLKDIV2  0x40

#define  I2S_FREQ96kHz    (0x0e<<1)
#define  I2S_FREQ48kHz    (0x00<<1)
#define  I2S_FREQ441kHz   (0x11<<1)
#define  I2S_FREQ32kHz   (0x0c<<1)
#define  I2S_FREQ24kHz   (0x1c<<1)
#define  I2S_FREQ2205kHz  (0x1b<<1)
#define  I2S_FREQ16kHz   (0x0a<<1)
#define  I2S_FREQ12kHz   (0x08<<1)
#define  I2S_FREQ11025kHz  (0x19<<1)
//#define  FREQ9k6Hz   0x09
#define  I2S_FREQ8kHz   (0x06<<1)
#define  I2S_WM_USB_MODE  0x01|WM_BCKdiv8

#define   I2S_WM_VOL_MUTE  0x00
#define   I2S_WM_VOL_0DB  0xff
#define   I2S_WM_UPDATE_VOL 0x100

//PM1 WM8987_R25
#define   I2S_WM_VMID50k  0x080
#define   I2S_WM_VREF   0x40
#define  I2S_WM_AINL   0x20
#define  I2S_WM_AINR   0x10
#define I2S_WM_ADCL   0x08
#define I2S_WM_ADCR   0x04
#define I2S_WM_MICB   0x02
#define   I2S_WM_DIGENB   0x01

//PM2  WM8987_R26
#define   I2S_WM_DACL   0x100
#define   I2S_WM_DACR   0x80
#define  I2S_WM_LOUT1   0x40
#define  I2S_WM_ROUT1   0x20
#define  I2S_WM_LOUT2   0x10
#define  I2S_WM_ROUT2   0x08
#define I2S_WM_MONO   0x04
#define   I2S_WM_OUT3   0x02

// WM8987_R34  Left out mix1
#define  I2S_LD2LO    0x100
#define  I2S_LI2LO    0x80
#define  I2S_LO0DB    0x20

//WM8987_R36  Right out Mix1
#define  I2S_RD2RO    0x100
#define  I2S_RI2RO    0x80
#define  I2S_RO0DB    0x20

#define  I2S_Bass_control  WM8987_R12
#define  I2S_Treble_control  WM8987_R13
#define  I2S_EQ3D_enhance  WM8987_R16



#define  I2S_Bass_Linear   0x00
#define  I2S_Bass_Adaptive  0x80

#define I2S_Bass_Cut130  0x00
#define I2S_Bass_Cut200  0x40

#define   I2S_Treble_Cut4k 0x40
#define   I2S_Treble_Cut8k 0x00

#define I2S_EQ3D_enable 0x01
#define I2S_EQ3D_disable  0x00
#define I2S_EQ3D_depth   0x01
/*******************************************************************************
** Name: I2sDevShellBspGetFs
** Input:uint8 * pstr
** Return: I2sFS_en_t
** Owner:hj
** Date: 2014.12.8
** Time: 14:01:19
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN I2sFS_en_t I2sDevShellBspGetFs(uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_8000Hz;
        rk_print_string("\r\n 8K start 0\n");          //8k
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_11025Hz;
        rk_print_string("\r\n 11.025K start 0\n");     //11.025k
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_12KHz;
        rk_print_string("\r\n 12K start 0\n");         //12K
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_16KHz;
        rk_print_string("\r\n 16K start 0\n");         //16K
    }
    else if (StrCmpA(pstr, "4", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_22050Hz;
        rk_print_string("\r\n 22.025K start 0\n");     //22.025K
    }
    //---------------------------------------------------------------//
    else if (StrCmpA(pstr, "5", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_24KHz;
        rk_print_string("\r\n 24K start 0\n");         //24K
    }
    else if (StrCmpA(pstr, "6", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_32KHz;
        rk_print_string("\r\n 32K   start 0\n");       //32K
    }
    else if (StrCmpA(pstr, "7", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_44100Hz;
        rk_print_string("\r\n 44.1K start 0\n");       //44.1K
    }
    else if (StrCmpA(pstr, "8", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_48KHz;
        rk_print_string("\r\n 48K start 0\n");         //48K
    }
    else if (StrCmpA(pstr, "9", 1) == 0)
    {
        I2S_TEST_FS = I2S_FS_192KHz;
        rk_print_string("\r\n 192K  start 0\n");        //192K
    }
    return I2S_TEST_FS;
}
/*******************************************************************************
** Name: I2S_Isr_test
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.8
** Time: 11:50:27
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2S_Isr_test_RX(void)
{
    uint32 IntEvent;

    IntEvent = I2SGetIntType(I2S_DEV_TEST);

    if (IntEvent & RX_interrupt_active)
    {
#ifdef i2sTest
        I2S_PIO_Write(I2S_DEV_TEST,(uint32*)outptr1,length1);
#endif
        return;
    }
}
/*******************************************************************************
** Name: I2S_Isr_test
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.8
** Time: 11:50:27
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2S_Isr_test(void)
{
    uint32 IntEvent;

    IntEvent = I2SGetIntType(I2S_DEV_TEST);

    if (IntEvent & TX_interrupt_active)
    {
#ifdef i2sTest
        I2S_PIO_Write(I2S_DEV_TEST,(uint32*)outptr1,length1);
#endif
        return;
    }
}
#if 0
/*******************************************************************************
** Name: I2S_RW_DmaIntIsr
** Input:void
** Return: void
** Owner:HJ
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_DATA_ pFunc I2S_RWDmaIsrCallBack[DMA_CHN_MAX];
_DRIVER_I2S_I2SDEVICE_SHELL_DATA_ HDC i2c_test;
_DRIVER_I2S_I2SDEVICE_SHELL_DATA_ UINT32 I2S_RW_AudioBuf[2][1024] = {0,};
_DRIVER_I2S_I2SDEVICE_SHELL_DATA_ uint32 I2S_RW_BuferIndex = 0;

_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2S_RW_DmaIntIsr(void)
{
    uint32 channel;
    uint32 rawStatus;
    pFunc CallBack;

    rawStatus = DmaGetIntType();

    for (channel = 0; channel < DMA_CHN_MAX; channel++)
    {
        if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
        {
            CallBack = I2S_RWDmaIsrCallBack[channel];
            if (CallBack)
            {
                I2S_RWDmaIsrCallBack[channel] = 0;
                CallBack();
            }

            //DmaDisableInt(channel);
        }
    }

}
/*******************************************************************************
** Name: I2sRW_DmaIsr_W
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sRW_DmaIsr_W(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_DEV_TEST);
    DmaDisableInt(1);

    DmaEnableInt(1);
    I2S_RWDmaIsrCallBack[1] = I2sRW_DmaIsr_W;

    DmaConfig(1,(uint32)I2S_RW_AudioBuf[1 - I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0ControlDmaCfg_TX, NULL);
    //DmaConfig(1, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S0ControlDmaCfg_TX, NULL);

}
/*******************************************************************************
** Name: I2sRW_DmaIsr_R
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sRW_DmaIsr_R(void)
{
    uint32 RX_FIFO_ADDR;
    RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_DEV_TEST);
    DmaDisableInt(0);

    DmaEnableInt(0);
    I2S_RW_BuferIndex = 1 - I2S_RW_BuferIndex;
    I2S_RWDmaIsrCallBack[0] = I2sRW_DmaIsr_R;

    DmaConfig(0,RX_FIFO_ADDR,(uint32)I2S_RW_AudioBuf[I2S_RW_BuferIndex],1024, &I2S0ControlDmaCfg_RX, NULL);
}
/*******************************************************************************
** Name: I2sDmaIsr_test
** Input:void
** Return: void
** Owner:Aaron
** Date: 2014.2.17
** Time: 11:43:35
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDmaIsr_test(void)
{
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_DEV_TEST);
    DmaDisableInt(2);

    if (I2S_DEV1 == I2S_DEV_TEST)
    {
        DmaEnableInt(2);
#ifdef i2sTest
        DmaConfig(2, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S0ControlDmaCfg_TX, NULL);
#endif
    }
    else
    {
        //DmaConfig(0, (UINT32)I2S_TestBuf, (uint32)(&(I2s_Reg->I2S_TXDR)),1024, &I2S1ControlDmaCfg, NULL);
    }
}



/*******************************************************************************
** Name: I2sDevShell5633_SendCmd
** Input:UINT32 RegAddr, UINT16 uData
** Return: void
** Owner:hj
** Date: 2014.12.4
** Time: 15:57:18
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell5633_SendCmd(UINT16 RegAddr, UINT16 uData)
{
    UINT8 SlaveAddress = (0x1c<<1);
    eI2C_RW_mode_t RWmode;
    uint8* DataBuf;
    uint32 size;

    uint8 cmd,reg_cmd,data[2];
    I2C_CMD_ARG stArg;

    //data = (uint8)(uData & 0xff);
    data[0] = (0xff00 & uData) >> 8;
    data[1] = (0xff & uData);
#ifdef __DRIVER_I2C_I2CDEVICE_C__
    i2c_test = RKDev_Open(DEV_CLASS_I2C, 0, NOT_CARE);

    if (i2c_test != NULL)
    {
        stArg.SlaveAddress = SlaveAddress;
        stArg.RegAddr = RegAddr;
        stArg.RWmode = NormalMode;
        stArg.speed = 50;
        stArg.addr_reg_fmt = I2C_7BIT_ADDRESS_8BIT_REG;

        if (RK_SUCCESS != I2cDev_SendData(i2c_test,&data[0],2,&stArg))
        {
            //printf("I2cDev_SendData IS ERR\n");
        }
    }
#endif
}


_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell5633_SetVolumet(uint32 Volume)
{

    UINT32 Vol_L,Vol_R ;

    if (Volume > MAX_VOLUME)
        Volume = MAX_VOLUME;

    Vol_L = (Volume << 8);
    Vol_R = (Volume << 0);

    I2sDevShell5633_SendCmd(RT5633_HP_OUT_VOL, Vol_L |Vol_R|HP_L_VOL_SEL_HPMIX_L|HP_R_VOL_SEL_HPMIX_R);    // set output 1 volume

}

/*
--------------------------------------------------------------------------------
  Function name : void I2sDevShell5633_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   : ????Codec2?????

  Input         : CodecFS?o2???????

  Return        : ?T

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ????:
--------------------------------------------------------------------------------
*/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell5633_SetSampleRate(CodecFS_en_t CodecFS)
{
    switch (CodecFS)
    {
        case I2S_FS_8000Hz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x0040);      //12MHZ,8KHZ
            printf("8KHZ\n");
            break;
        case I2S_FS_11025Hz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x123f);      //12MHZ,11.025K
            printf("11.025KHZ\n");
            break;
        case I2S_FS_12KHz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x0020);      //12MHZ,12K
            printf("12KHZ\n");
            break;
        case I2S_FS_16KHz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x276d);      //12MHZ,16K
            printf("16KHZ\n");
            break;
        case I2S_FS_22050Hz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x2e4f);      //12MHZ,22.05K
            printf("22.05KHZ\n");
            break;
        case I2S_FS_24KHz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x294c);      //12MHZ,24K
            printf("24KHZ\n");
            break;
        case I2S_FS_32KHz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x456b);      //12MHZ,32K
            printf("32KHZ\n");
            break;
        case I2S_FS_48KHz:
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x291c);      //12MHZ,48K
            printf("48KHZ\n");
            break;
        case I2S_FS_44100Hz:
        default:
            printf("44.1KHZ\n");
            I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x4d4c);      //12MHZ,44.1KHZ
            break;
    }
}
/*******************************************************************************
** Name: I2sDevShell5633_Init
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.4
** Time: 15:57:18
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell5633_Init(void)
{
    UINT8 uData[16] = {0,};
    UINT16 i;
    I2sDevShell5633_SendCmd(RT5633_RESET, 0x00);   //reset

    ///////////////////////////////////////////////////////////
    //power
    I2sDevShell5633_SendCmd(RT5633_PWR_MANAG_ADD3,PWR_VREF | PWR_MAIN_BIAS | PWR_HP | PWR_HP_AMP);   //power on

    I2sDevShell5633_SendCmd(RT5633_PWR_MANAG_ADD1,(PWR_MAIN_I2S_EN |
                            //PWR_ADC_L_CLK | PWR_ADC_R_CLK |
                            PWR_DAC_L_CLK | PWR_DAC_R_CLK | PWR_DAC_REF |
                            PWR_DAC_L_TO_MIXER | PWR_DAC_R_TO_MIXER));                       //power I2S and DAC

    I2sDevShell5633_SendCmd(RT5633_PWR_MANAG_ADD2,(PWR_PLL |
                            PWR_HPMIXER_L | PWR_HPMIXER_R));
    // PWR_RECMIXER_L|PWR_RECMIXER_R |
    //PWR_LINEIN1_L |PWR_LINEIN1_R));                         //POWER PLL

    I2sDevShell5633_SendCmd(RT5633_PWR_MANAG_ADD4, (PWR_HP_L_VOL | PWR_HP_R_VOL));  //power hp vol

    // input set
    I2sDevShell5633_SendCmd(RT5633_REC_MIXER_CTRL, 0xf7f7);                 //Line 1 Input
    I2sDevShell5633_SendCmd(RT5633_ADC_CTRL,0x0000);                        //ADC unmute
    I2sDevShell5633_SendCmd(RT5633_LINE_IN_1_VOL,0x0000);                   // Line 1 Vol 0db
    I2sDevShell5633_SendCmd(RT5633_ADC_DIG_VOL, 0x0000);                    // ADC vol 0db

    // output set
    I2sDevShell5633_SendCmd(RT5633_DAC_CTRL,0x0000);                         // DAC unmute
    I2sDevShell5633_SendCmd(RT5633_DAC_DIG_VOL,0x0000);                      // 0db
    I2sDevShell5633_SendCmd(RT5633_HPMIXER_CTRL,0xfefe);                    // HpMix DAC output
    //Codec5633CommandSet(dev,RT5633_HPMIXER_CTRL,0xdfdf);                    // HpMix RecMix output
    I2sDevShell5633_SendCmd(RT5633_SPK_HP_MIXER_CTRL,0x0020);               // hp vol to hpout
    I2sDevShell5633_SendCmd(RT5633_HP_OUT_VOL, 0x4040);                     // HPMIX output

    //setup digital audio  interface
#ifdef BIT_TEST
    I2sDevShell5633_SendCmd(RT5633_SDP_CTRL,  0x0008);
    // IIS interface WM_MASTER_MODE | WM_I2S_MODE);   //Audio inteface Data length and Data formate,24Bit
    I2sDevShell5633_SendCmd(RT5633_STEREO_AD_DA_CLK_CTRL,  0x0000);         // IIS 16bit BCLK = LRCK * 32
#else
    I2sDevShell5633_SendCmd(RT5633_SDP_CTRL,  (0x0000 | SDP_MODE_SEL_SLAVE));
    //I2sDevShell5633_SendCmd(RT5633_SDP_CTRL,  (0x0000));

    // IIS interface WM_MASTER_MODE | WM_I2S_MODE);   //Audio inteface Data length and Data formate,16Bit
    I2sDevShell5633_SendCmd(RT5633_STEREO_AD_DA_CLK_CTRL,  0x1000);         // IIS 16bit BCLK = LRCK * 32
#endif

    I2sDevShell5633_SendCmd(RT5633_GBL_CLK_CTRL,SYSCLK_IS_PLL);             // sysclk is pll
    I2sDevShell5633_SendCmd(RT5633_PLL_CTRL,0x4d4c);                        //44.1kHz

    // HP det
    I2sDevShell5633_SendCmd(RT5633_DEPOP_CTRL_1,PW_SOFT_GEN|EN_DEPOP_2|EN_SOFT_FOR_S_M_DEPOP|EN_HP_R_M_UM_DEPOP|EN_HP_L_M_UM_DEPOP);
    I2sDevShell5633_SendCmd(RT5633_DEPOP_CTRL_2, 0x8000);

}
/*******************************************************************************
** Name: I2sDevShell8987_SendCmd
** Input:UINT32 RegAddr, UINT16 uData
** Return: void
** Owner:hj
** Date: 2014.12.4
** Time: 15:57:18
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_SendCmd(UINT32 RegAddr, UINT16 uData)
{
    UINT8 SlaveAddress = (0x1a << 1);
    eI2C_RW_mode_t RWmode;
    uint8* DataBuf;
    rk_size_t size;

    uint8 cmd,reg_cmd;
    I2C_CMD_ARG stArg;

    cmd = (UINT8)uData;
    reg_cmd = (UINT8)(((RegAddr << 9) | uData) >> 8);
#ifdef __DRIVER_I2C_I2CDEVICE_C__
    i2c_test = RKDev_Open(DEV_CLASS_I2C, 0, NOT_CARE);
    //printf("reg_cmd = %x\n",reg_cmd);
    //printf("cmd = %x\n",cmd);

    if (i2c_test != NULL)
    {
        stArg.SlaveAddress = SlaveAddress;
        stArg.RegAddr = reg_cmd;
        stArg.RWmode = NormalMode;
        stArg.speed = 100;
        stArg.addr_reg_fmt = I2C_7BIT_ADDRESS_8BIT_REG;

        size = I2cDev_SendData(i2c_test,&cmd,1,&stArg);
        printf("size = %d\n",size);
        while (size == 1)
        {
            size = I2cDev_SendData(i2c_test,&cmd,1,&stArg);
            DelayMs(5);
            printf("send data\n");
        }
    }
#endif
}
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_SetVolumet(uint32 Volume)
{

    UINT32 Vol ;

    if (Volume > MAX_VOLUME)
        Volume = MAX_VOLUME;

    Vol = 41 + (Volume << 1);

    if (Volume == 0)
    {
        Vol = 0;
    }

    if (Volume <= 7)  Vol = 47 + Volume;

    I2sDevShell8987_SendCmd(I2S_WM8987_R40, Vol | 0x0080);    // set output 1 volume
    I2sDevShell8987_SendCmd(I2S_WM8987_R41, Vol | 0x0180);

}
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_DACUnMute(void)
{
    I2sDevShell8987_SendCmd(I2S_WM8987_R5,   0x0000); // DAC soft unmute
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sDevShell8987_SetMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   : ??Codec????

  Input         : Codecmode:????

  Return        : ?

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ??:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_SetMode(CodecMode_en_t Codecmode)
{
    switch (Codecmode)
    {
        case Codec_DACoutHP:
            {
                if (I2S_MASTER_MODE == I2S_TEST_MODE)
                {
                    I2sDevShell8987_SendCmd(WM8987_R7,  WM_SLAVE_MODE | WM_I2S_MODE );
                }

                I2sDevShell8987_SendCmd(I2S_WM8987_R8,  I2S_FREQ441kHz | I2S_WM_USB_MODE);

                I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LD2LO | I2S_LO0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RD2RO | I2S_RO0DB);

                I2sDevShell8987_SendCmd(I2S_WM8987_R10, I2S_WM_VOL_0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R11, I2S_WM_VOL_0DB | I2S_WM_UPDATE_VOL);

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF);
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, I2S_WM_DACL | I2S_WM_DACR | I2S_WM_LOUT2 | I2S_WM_ROUT2 | I2S_WM_OUT3);
            }
            break;

        case Codec_FMin:
            {
                I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LI2LO | I2S_LO0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RI2RO | I2S_RO0DB);

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF);
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, I2S_WM_LOUT2 | I2S_WM_ROUT2 | I2S_WM_OUT3);
            }
            break;

        case Codec_FMADC:
            {
                I2sDevShell8987_SendCmd(I2S_WM8987_R0,  0x003f);    //Left channel PGA  0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R1,  0x013f);    //Left channel PGA   0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R21, 0x00c3);    //Left digital ADC 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R31, 0x0000);

                I2sDevShell8987_SendCmd(I2S_WM8987_R32, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R33, 0x0000);    //Left ADC select = Linput3 MICBoost = 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LI2LO | I2S_LO0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R36,I2S_LO0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RI2RO);

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF | I2S_WM_ADCL | I2S_WM_ADCR);
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, I2S_WM_LOUT2 | I2S_WM_ROUT2 | I2S_WM_OUT3);
            }
            break;

        case Codec_Line2ADC:
            {
                I2sDevShell8987_SendCmd(I2S_WM8987_R0,  0x0057);    //Left channel PGA  0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R1,  0x0157);    //Left channel PGA   0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R21, 0x00c3);    //Left digital ADC 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R31, 0x0000);    //ADC Input mode select

                I2sDevShell8987_SendCmd(I2S_WM8987_R32, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R33, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LI2LO | I2S_LO0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RI2RO | I2S_RO0DB);

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF | I2S_WM_ADCL | I2S_WM_ADCR);
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, 0x0000);
            }
            break;

        case Codec_MicStero:
            {
                I2sDevShell8987_SendCmd(I2S_WM8987_R0,  0x0117);    //Left channel PGA  0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R1,  0x0117);    //Left channel PGA   0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R21, 0x01c3);    //Left digital ADC 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R22, 0x01c3);    //Right  digital ADC 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R10, 0x01ff);    //Left digital DAC 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R11, 0x01ff);    //Right  digital DAC 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R31, 0x0040);    // adc use mono mix, select left adc

                I2sDevShell8987_SendCmd(I2S_WM8987_R32, 0x00a0);    //Left ADC select = Linput3 MICBoost = 20dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R33, 0x0080);    //Left ADC select = Linput3 MICBoost = 00dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF | I2S_WM_AINL | I2S_WM_ADCL | I2S_WM_MICB);   //Left ADC and Left PGA power on,Right ADC and Right PGA power off
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, 0x00);
            }
            break;

        case Codec_Standby:
            {
                I2sDevShell8987_SendCmd(I2S_WM8987_R0,  0x0057);    //Left channel PGA  0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R1,  0x0157);    //Left channel PGA   0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R21, 0x00ff);    //Left digital ADC 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R22, 0x01ff);    //Right  digital ADC 0dB

                I2sDevShell8987_SendCmd(I2S_WM8987_R31, 0x0000);    //ADC Input mode select

                I2sDevShell8987_SendCmd(I2S_WM8987_R32, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB
                I2sDevShell8987_SendCmd(I2S_WM8987_R33, 0x0000);    //Left ADC select = Linput0 MICBoost = 0dB

                if (I2S_MASTER_MODE == I2S_TEST_MODE)
                {
                    I2sDevShell8987_SendCmd(WM8987_R7,  WM_SLAVE_MODE | WM_I2S_MODE );
                }

                I2sDevShell8987_SendCmd(I2S_WM8987_R8,  I2S_FREQ441kHz | I2S_WM_USB_MODE);

                I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LD2LO);
                I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RD2RO);

                I2sDevShell8987_SendCmd(I2S_WM8987_R10, I2S_WM_VOL_0DB);
                I2sDevShell8987_SendCmd(I2S_WM8987_R11, I2S_WM_VOL_0DB | I2S_WM_UPDATE_VOL);

                I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF | I2S_WM_ADCL | I2S_WM_ADCR);
                I2sDevShell8987_SendCmd(I2S_WM8987_R26, I2S_WM_DACL | I2S_WM_DACR | I2S_WM_LOUT2 | I2S_WM_ROUT2 | I2S_WM_OUT3);
            }
            break;

    }

}

/*
--------------------------------------------------------------------------------
  Function name : void I2sDevShell8987_ExitMode(CodecMode_en_t Codecmode)
  Author        : yangwenjie
  Description   : ??Codec????

  Input         : Codecmode:????

  Return        : ?

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ??:      if exit from application, like FM or MIC , please set codec to standby mode
--------------------------------------------------------------------------------
*/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_ExitMode(void)
{

}

/*
--------------------------------------------------------------------------------
  Function name : void I2sDevShell8987_SetSampleRate(CodecFS_en_t CodecFS)
  Author        : yangwenjie
  Description   : ??Codec???

  Input         : CodecFS:????

  Return        : ?

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  ??:
--------------------------------------------------------------------------------
*/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_SetSampleRate(CodecFS_en_t CodecFS)
{
    switch (CodecFS)
    {
        case I2S_FS_8000Hz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,    I2S_FREQ8kHz | I2S_WM_USB_MODE);    //12MHZ,8KHZ
            break;
        case I2S_FS_11025Hz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,    I2S_FREQ11025kHz | I2S_WM_USB_MODE);    //12MHZ,11.025K
            break;
        case I2S_FS_12KHz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,    I2S_FREQ12kHz | I2S_WM_USB_MODE);    //12MHZ,12K
            break;
        case I2S_FS_16KHz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,    I2S_FREQ16kHz | I2S_WM_USB_MODE);    //12MHZ,16K
            break;
        case I2S_FS_22050Hz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,     I2S_FREQ2205kHz | I2S_WM_USB_MODE);    //12MHZ,22.05K
            break;
        case I2S_FS_24KHz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,     I2S_FREQ24kHz | I2S_WM_USB_MODE);    //12MHZ,24K
            break;
        case I2S_FS_32KHz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,     I2S_FREQ32kHz | I2S_WM_USB_MODE);    //12MHZ,32K
            break;
        case I2S_FS_48KHz:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,     I2S_FREQ48kHz | I2S_WM_USB_MODE);    //12MHZ,48K
            break;
        case I2S_FS_44100Hz:
        default:
            I2sDevShell8987_SendCmd(I2S_WM8987_R8,     I2S_FREQ441kHz | I2S_WM_USB_MODE);    //12MHZ,44.1KHZ
            break;
    }
}
/*******************************************************************************
** Name: I2sDevShell8987_Init
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.4
** Time: 15:57:18
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN void I2sDevShell8987_Init(void)
{
    //open I2C clk
    ScuClockGateCtr(PCLK_I2C0_GATE, 1);
    //SetUartFreq(SYS_DEFAULT_FREQ);

    //open rst I2C ip
    ScuSoftResetCtr(I2C0_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(I2C0_SRST, 0);

    //I2CDevHwInit(I2C_DEV0);

    I2sDevShell8987_SendCmd(I2S_WM8987_R15, 0x012f);                        //software reset wm8750

    I2sDevShell8987_SendCmd(I2S_WM8987_R27, 0x0040);                        // set VREF to output 40K

    //setup output and DAC mute
    I2sDevShell8987_SendCmd(I2S_WM8987_R10, I2S_WM_VOL_MUTE);                   // left DAC mute//????????

    I2sDevShell8987_SendCmd(I2S_WM8987_R11, I2S_WM_VOL_MUTE | I2S_WM_UPDATE_VOL);   // right DAC mute

    I2sDevShell8987_SendCmd(I2S_WM8987_R5,  0x0008);                        //Mute


    DelayMs(10);
    I2sDevShell8987_SendCmd(I2S_WM8987_R25, I2S_WM_VMID50k | I2S_WM_VREF);          // setup Vmid and Vref, Vmid =

    DelayMs(10);
    I2sDevShell8987_SendCmd(I2S_WM8987_R26, 0x0000);

    //setup digital audio  interface
    I2sDevShell8987_SendCmd(I2S_WM8987_R7,  I2S_WM_MASTER_MODE|I2S_WM_I2S_MODE);     //WM_MASTER_MODE | WM_I2S_MODE);   //Audio inteface Data length and Data formate,16Bit

    I2sDevShell8987_SendCmd(I2S_WM8987_R8,  I2S_FREQ441kHz | I2S_WM_USB_MODE);      //ADC and DAC sample frequence 44.118Khz,USB mode

    //setup left and right MIXER
    I2sDevShell8987_SendCmd(I2S_WM8987_R34, I2S_LD2LO | I2S_LO0DB);

    I2sDevShell8987_SendCmd(I2S_WM8987_R37, I2S_RD2RO | I2S_RO0DB);

    //setup no used register
    I2sDevShell8987_SendCmd(I2S_WM8987_R35, 0x0070);                        // Right DAC to left MIXER: mute

    I2sDevShell8987_SendCmd(I2S_WM8987_R36, 0x0070);                        //

    I2sDevShell8987_SendCmd(I2S_WM8987_R38, 0x0070);                        //Mono out Mix Default: Mono Mixer no input, Lmixsel volume=0

    I2sDevShell8987_SendCmd(I2S_WM8987_R39, 0x0070);                        //Mono out Mix Default: Mono Mixer no input, Rmixsel volume=0

    I2sDevShell8987_SendCmd(I2S_WM8987_R2,  0x0100);                        // set R  output 1 mute

    I2sDevShell8987_SendCmd(I2S_WM8987_R40, 0x0080);                        // set output 2 mute

    I2sDevShell8987_SendCmd(I2S_WM8987_R41, 0x0180);                        // set output 2 mute

    I2sDevShell8987_SendCmd(I2S_WM8987_R24, 0x0004);                        // set output 3 ref

    I2sDevShell8987_SendCmd(I2S_EQ3D_enhance, I2S_EQ3D_disable);

}
/*******************************************************************************
** Name: I2sDevShellSampleRateTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.1
** Time: 16:11:32
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellSampleRateTest(HDC dev, uint8 * pstr)
{
    rk_print_string("test ALC5633 SampleRate\r\n");
    I2sDevShell5633_Init();
    I2sDevShell5633_SetVolumet(30);

    //rk_print_string("test SampleRate\r\n");
    //I2sDevShell8987_Init();
    //I2sDevShell8987_SetMode(Codec_DACoutHP);
    //I2sDevShell8987_SetSampleRate(I2S_FS_16KHz);
    //I2SInit(I2S_DEV0,I2S_SLAVE_MODE,I2S_EXT,I2S_FS_8000Hz,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    //I2SStart(I2S_DEV0,I2S_START_PIO_TX);
}

/*******************************************************************************
** Name: I2sDevShell5633_Test
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.1
** Time: 16:11:32
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShell5633_Test(HDC dev, uint8 * pstr)
{
    rk_print_string("test I2sDevShell5633\r\n");
    I2SDevHwInit(I2S_DEV1, I2S_SEL_PAD);
    I2sDevShell5633_Init();
    I2sDevShell5633_SetVolumet(30);
    I2sDevShell5633_SetSampleRate(I2S_FS_44100Hz);
    I2SInit(I2S_DEV1,I2S_SLAVE_MODE,I2S_EXT,I2S_FS_44100Hz,I2S_FORMAT,I2S_DATA_WIDTH24,I2S_NORMAL_MODE);
    I2SStart(I2S_DEV1,I2S_START_PIO_TX);
    //I2sDevShell8987_SetMode(Codec_DACoutHP);
    //I2sDevShell8987_SetSampleRate(I2S_FS_16KHz);
    //I2SInit(I2S_DEV0,I2S_SLAVE_MODE,I2S_EXT,I2S_FS_8000Hz,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    //I2SStart(I2S_DEV0,I2S_START_PIO_TX);
}


/*******************************************************************************
** Name: I2sDevShellBspPIO_Read
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:11:58
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspPIO_Read(HDC dev, uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;

    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    printf("I2sDevShellBspPIO_Read %d\n",I2S_DEV_TEST);

    IntRegister(INT_ID_I2S0 ,I2S_Isr_test_RX);
    IntPendingClear(INT_ID_I2S0);
    IntEnable(INT_ID_I2S0);
    while (1);
}
/*******************************************************************************
** Name: I2sDevShellBspPIO_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:11:58
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspPIO_Write(HDC dev, uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    printf("I2sDevShellBspPIO_Wtite %d\n",I2S_DEV_TEST);

    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    if (I2S_DEV0 == I2S_DEV_TEST)
    {
        IntRegister(INT_ID_I2S0 ,I2S_Isr_test);
        IntPendingClear(INT_ID_I2S0);
        IntEnable(INT_ID_I2S0);
    }
    else
    {
        IntRegister(INT_ID_I2S1 ,I2S_Isr_test);
        IntPendingClear(INT_ID_I2S1);
        IntEnable(INT_ID_I2S1);
    }

    //I2S init
    if (I2S_MASTER_MODE == I2S_TEST_MODE)
    {
        printf("I2S_MASTER_MODE\n");

        //I2sDevShell8987_SetMode(Codec_DACoutHP);
        ////I2sDevShell8987_SetSampleRate(I2S_TEST_FS);
        //I2sDevShell8987_DACUnMute();
        //I2sDevShell8987_SetVolumet(30);

        //I2sDevShell8987_DACUnMute();
        I2sDevShell5633_SetVolumet(30);

        I2SInit(I2S_DEV_TEST,I2S_MASTER_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }
    else
    {
        printf("I2S_SLAVE_MODE\n");

        I2sDevShell8987_SetMode(Codec_DACoutHP);
        I2sDevShell8987_SetSampleRate(I2S_TEST_FS);
        I2sDevShell8987_DACUnMute();
        I2sDevShell8987_SetVolumet(30);
        I2SInit(I2S_DEV_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }
#ifdef i2sTest
    I2S_PIO_Write(I2S_DEV_TEST,(uint32*)outptr1,length1);
#endif
    I2SIntEnable(I2S_DEV_TEST,I2S_INT_TX);
    I2SStart(I2S_DEV_TEST,I2S_START_PIO_TX);

    while (1);
}

/*******************************************************************************
** Name: I2sDevShellBspDMA_RW
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspDMA_RW(HDC dev, uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR,RX_FIFO_ADDR;
    pFunc pCallBack_TX,pCallBack_RX;

    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_DEV_TEST);
    RX_FIFO_ADDR = I2sGetRxFIFOaddr(I2S_DEV_TEST);
    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    printf("I2sDevShellBspDMA_ReadWrite %d\n",I2S_DEV_TEST);

    //open uart clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst uart ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA ,I2S_RW_DmaIntIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(0);
    DmaEnableInt(1);

    I2sDevShell8987_SetMode(Codec_Standby); //ADC DAC
    I2sDevShell8987_SetSampleRate(I2S_FS_44100Hz);
    I2sDevShell8987_DACUnMute();
    I2sDevShell8987_SetVolumet(30);

    if (I2S_MASTER_MODE == I2S_TEST_MODE)
    {
        printf("I2S_MASTER_MODE\n");
        I2SInit(I2S_DEV_TEST,I2S_MASTER_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }
    else
    {
        printf("I2S_SLAVE_MODE\n");
        I2SInit(I2S_DEV_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }

    pCallBack_RX = I2sRW_DmaIsr_R;
    pCallBack_TX = I2sRW_DmaIsr_W;

    I2S_RWDmaIsrCallBack[0] = pCallBack_RX;
    I2S_RWDmaIsrCallBack[1] = pCallBack_TX;

    DmaConfig(0,RX_FIFO_ADDR,(uint32)I2S_RW_AudioBuf[I2S_RW_BuferIndex],1024, &I2S0ControlDmaCfg_RX, NULL);
    DelayMs(5);
    DmaConfig(1,(uint32)I2S_RW_AudioBuf[1 - I2S_RW_BuferIndex], TX_FIFO_ADDR,1024, &I2S0ControlDmaCfg_TX, NULL);
    //DmaConfig(1, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S0ControlDmaCfg_TX, NULL);

    I2SDMAEnable(I2S_DEV_TEST,I2S_START_DMA_RTX);
    I2SStart(I2S_DEV_TEST,I2S_START_DMA_RTX);

    while (1);
}
/*******************************************************************************
** Name: I2sDevShellBspDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:10:53
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspDMA_Write(HDC dev, uint8 * pstr)
{
    I2sFS_en_t I2S_TEST_FS;
    uint32 i = 0;
    uint32 TX_FIFO_ADDR;
    TX_FIFO_ADDR = I2sGetTxFIFOaddr(I2S_DEV_TEST);
    printf("I2sDevShellBspDMA_Wtite %d\n",I2S_DEV_TEST);


    I2S_TEST_FS = I2sDevShellBspGetFs(pstr);

    //open dma clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst dma ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA ,I2sDmaIsr_test);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(2);

    I2sDevShell8987_SetMode(Codec_DACoutHP);
    I2sDevShell8987_SetSampleRate(I2S_TEST_FS);
    I2sDevShell8987_DACUnMute();
    I2sDevShell8987_SetVolumet(30);
    //I2S init
    if (I2S_MASTER_MODE == I2S_TEST_MODE)
    {
        printf("I2S_MASTER_MODE\n");
        I2SInit(I2S_DEV_TEST,I2S_MASTER_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }
    else
    {
        printf("I2S_SLAVE_MODE\n");
        I2SInit(I2S_DEV_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }

    if (I2S_DEV1 == I2S_DEV_TEST)
    {
#ifdef i2sTest
        DmaConfig(2, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S0ControlDmaCfg_TX, NULL);
#endif
    }
    else
    {
#ifdef i2sTest
        DmaConfig(0, (uint32)outptr1, TX_FIFO_ADDR,length1, &I2S1ControlDmaCfg_TX, NULL);
#endif
    }
    I2SDMAEnable(I2S_DEV_TEST,I2S_START_DMA_TX);
    I2SStart(I2S_DEV_TEST,I2S_START_DMA_TX);
    while (1);
}

/*******************************************************************************
** Name: I2sDevShellBspDeinit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:05:28
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspDeinit(HDC dev, uint8 * pstr)
{

}

/*******************************************************************************
** Name: I2sDevShellBspInit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:04:22
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspInit(HDC dev, uint8 * pstr)
{

    if (StrCmpA(pstr, "0", 1) == 0)
    {
        I2S_TEST_MODE = I2S_SLAVE_MODE;
        rk_print_string("\r\n I2S_SLAVE_MODE\n");          //slave
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        I2S_TEST_MODE = I2S_MASTER_MODE;
        rk_print_string("\r\n I2S_MASTER_MODE\n");     //MASTER
    }
    I2S_DEV_TEST = I2S_DEV1;
    //ScuClockGateCtr(CLOCK_GATE_I2C, 1);
    //ScuClockGateCtr(CLK_I2S_GATE, 1);
    //ScuClockGateCtr(CLOCK_GATE_GRF, 1);
    //I2s_Iomux_Set();
    //I2c_Iomux_Set();

    //codec init
    I2sDevShell8987_Init();
    rk_print_string("\r\ni2s init over");


}
#endif
#ifdef SHELL_HELP
/*******************************************************************************
** Name: I2sDevShellBspHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 14:56:46
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBspHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if (StrLenA(pstr) != 0)
        return RK_ERROR;

    rk_print_string("I2s.Bsp?????????????I2S??????????\r\n");
    rk_print_string("??????:\r\n");
    rk_print_string("init      ??? I2S                                    \r\n");
    rk_print_string("deinit    ???? I2S                                  \r\n");
    rk_print_string("test SampleRate      ??I2S master  ???             \r\n");
    rk_print_string("start     ??I2S                                       \r\n");
    rk_print_string("stop      ??I2S                                       \r\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: I2SShellHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:chad.ma
** Date: 2014.11.3
** Time: 10:57:25
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2SShellHelp(HDC dev, uint8 * pstr)
{
    pstr--;

    if ( StrLenA( pstr) != 0)
        return RK_ERROR;

    rk_print_string("usage: I2s [.pcb] [.create] [.delete] [.help]   \r\n\n");

    rk_print_string("The most commonly used I2s commands are:\r\n");

    rk_print_string("pcb        I2s pcb info\r\n");
    rk_print_string("create     create I2s device\r\n");
    rk_print_string("delete     delete I2s device\r\n");
    rk_print_string("help       I2s the most commonly used I2s commands list\r\n");

    //rk_print_string("test      ??i2s             \r\n");
    //rk_print_string("close     ??i2s             \r\n");
    //rk_print_string("suspend   suspend i2s         \r\n");
    //rk_print_string("resume    resume i2s          \r\n");
    //rk_print_string("read      read                \r\n");
    //rk_print_string("write     write               \r\n");
    //rk_print_string("control   control i2s         \r\n");

    return RK_SUCCESS;

}
#endif
#ifdef SHELL_BSP

/*******************************************************************************
** Name: I2sDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 11:32:09
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2sDevShellBsp(HDC dev, uint8 * pstr)
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

    ret = ShellCheckCmd(ShellI2sBspName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                                 //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:  //bsp help
#ifdef SHELL_HELP
            ret = I2sDevShellBspHelp(dev,pItem);
#endif
            break;

        case 0x01:  //init
            ret = I2sDevShellBspInit(dev,pItem);
            //ret = I2sDevShellSampleRateTest(dev,pItem);
            break;

        case 0x02:  //deinit
            ret = I2sDevShellBspDeinit(dev,pItem);
            break;
        case 0x03:  //DMA_Wtite
            ret = I2sDevShellBspDMA_Write(dev,pItem);
            break;
        case 0x04:  //DMA_Read
            ret = I2sDevShellBspDMA_RW(dev,pItem);
            break;
        case 0x05:  //PIO_Wtite
            ret = I2sDevShellBspPIO_Write(dev,pItem);
            break;
        case 0x06:  //PIO_Read
            ret = I2sDevShellBspPIO_Read(dev,pItem);
            break;
        case 0x07:  //SampleRateTest
            ret = I2sDevShellSampleRateTest(dev,pItem);
            break;
        //case 0x08:  //5633test
        //    ret = I2sDevShell5633_Test(dev,pItem);
        //    break;
        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}
#endif
/*******************************************************************************
** Name: I2SShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
rk_err_t I2SShellPcb(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    I2S_DEVICE_CLASS * pstI2sDev;
#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2s.pcb : pcb info\r\n");
            return RK_SUCCESS;
        }
    }
#endif

    DevID = String2Num(pstr);

    if(DevID >= I2S_DEV_MAX)
    {
        return RK_ERROR;
    }

    pstI2sDev = gpstI2sDevISRHandler[DevID];

    if(pstI2sDev == NULL)
    {
        rk_printf("i2sDev%d in not exist", DevID);
        return RK_SUCCESS;
    }
    if(gpstI2sDevISRHandler[DevID] != NULL)
    {
        rk_printf_no_time(".gpstI2sDevISRHandler[%d]", DevID);
        rk_printf_no_time("    .stI2SDevice");
        rk_printf_no_time("        .next = %08x",pstI2sDev->stI2SDevice.next);
        rk_printf_no_time("        .UseCnt = %d",pstI2sDev->stI2SDevice.UseCnt);
        rk_printf_no_time("        .SuspendCnt = %d",pstI2sDev->stI2SDevice.SuspendCnt);
        rk_printf_no_time("        .DevClassID = %d",pstI2sDev->stI2SDevice.DevClassID);
        rk_printf_no_time("        .DevID = %d",pstI2sDev->stI2SDevice.DevID);
        rk_printf_no_time("        .suspend = %08x",pstI2sDev->stI2SDevice.suspend);
        rk_printf_no_time("        .resume = %08x",pstI2sDev->stI2SDevice.resume);
        rk_printf_no_time("    .osI2SOperReqSem = %08x",pstI2sDev->osI2SOperReqSem);
        rk_printf_no_time("    .osI2SReadOperSem = %08x",pstI2sDev->osI2SReadOperSem);
        rk_printf_no_time("    .osI2SWriteOperSem = %08x",pstI2sDev->osI2SWriteOperSem);
        rk_printf_no_time("    .Channel = %08x",pstI2sDev->Channel);
        rk_printf_no_time("    .txRxMode = %08x",pstI2sDev->txRxMode);
        rk_printf_no_time("    .dwTxNowNum = %d",pstI2sDev->dwTxNowNum);
        rk_printf_no_time("    .dwRxNowNum = %d",pstI2sDev->dwRxNowNum);
        rk_printf_no_time("    .I2S_Status = %d",pstI2sDev->I2S_Status);
        rk_printf_no_time("    .I2sItemID = %d",pstI2sDev->I2sItemID);
        rk_printf_no_time("    .stTxItem[0]");
        rk_printf_no_time("        .buffer = %08x",pstI2sDev->stTxItem[0].buffer);
        rk_printf_no_time("        .size = %d",pstI2sDev->stTxItem[0].size);
        rk_printf_no_time("    .stTxItem[1]");
        rk_printf_no_time("        .buffer = %08x",pstI2sDev->stTxItem[1].buffer);
        rk_printf_no_time("        .size = %d",pstI2sDev->stTxItem[1].size);
        rk_printf_no_time("    .Cnt = %d",pstI2sDev->Cnt);
        rk_printf_no_time("    .DmaCh = %d",pstI2sDev->DmaCh);
        rk_printf_no_time("    .WriteBusy = %d",pstI2sDev->WriteBusy);
        rk_printf_no_time("    .ReadBusy = %d",pstI2sDev->ReadBusy);
        rk_printf_no_time("    .WriteDataFlag = %d",pstI2sDev->WriteDataFlag);
        rk_printf_no_time("    .ReadDataFlag = %d",pstI2sDev->ReadDataFlag);
        rk_printf_no_time("    .hDma = %d",pstI2sDev->hDma);
        rk_printf_no_time("    .dwTxStatus = %d",pstI2sDev->dwTxStatus);
        rk_printf_no_time("    .dwRxStatus = %d",pstI2sDev->dwRxStatus);
        rk_printf_no_time("    .I2S_mode = %d",pstI2sDev->I2S_mode);
        rk_printf_no_time("    .i2sCS = %d",pstI2sDev->i2sCS);
        rk_printf_no_time("    .I2S_FS = %d",pstI2sDev->I2S_FS);
        rk_printf_no_time("    .BUS_FORMAT = %d",pstI2sDev->BUS_FORMAT);
        rk_printf_no_time("    .Data_width = %d",pstI2sDev->Data_width);
        rk_printf_no_time("    .I2S_Bus_mode = %d",pstI2sDev->I2S_Bus_mode);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("I2SDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;
}


_DRIVER_I2S_I2SDEVICE_SHELL_
rk_err_t I2sDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hI2S;
    I2S_DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 i,j;
    HDC hExDev;
    I2S_DEVICE_CONFIG_REQ_ARG stArg;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2s.test : test i2s    \r\n");
            return RK_SUCCESS;
        }
    }
#endif

#if 1

    stArg.i2smode = I2S_SLAVE_MODE;
    stArg.i2sCS = I2S_EXT;
    stArg.I2S_FS = I2S_FS_44100Hz;
    stArg.BUS_FORMAT = I2S_FORMAT;
    stArg.Data_width = I2S_DATA_WIDTH16;
    stArg.I2S_Bus_mode = I2S_NORMAL_MODE;

    hI2S = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);

    if ((hI2S == NULL) || (hI2S == (HDC)RK_ERROR) || (hI2S == (HDC)RK_PARA_ERR))
    {
        UartDev_Write(dev,"I2sDevShellTest I2S open failure",34,SYNC_MODE,NULL);
    }

    pDev = (I2S_DEVICE_CLASS *)hI2S;
/*
    //gpstI2sDevISRHandler[pDev->stI2SDevice.DevID] = pDev;
    I2sDevShell8987_Init();
    I2sDevShell8987_SetMode(Codec_DACoutHP);
    I2sDevShell8987_SetSampleRate(I2S_FS_44100Hz);
    I2sDevShell8987_DACUnMute();
    I2sDevShell8987_SetVolumet(30);
    ret = I2sDev_Control(hI2S, I2S_DEVICE_INIT_CMD, &stArg);
*/
    while (1)
    {
#ifdef i2sTest
        ret = I2sDev_Write(hI2S, (uint32*)outptr1,length1, ASYNC_MODE);
#endif
    }

#endif

    return ret;
}
/*******************************************************************************
** Name: I2SShellOpen
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 10:43:17
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
rk_err_t I2SShellOpen(HDC dev,  uint8 * pstr)
{
    HDC hI2S;
    I2S_DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 i,j;
    HDC hExDev;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("i2s.open : open i2s    \r\n");
            return RK_SUCCESS;
        }
    }
#endif

    pDev = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);
    if(pDev != NULL)
    {
        printf("I2SDev open fail\n");
    }
    return RK_SUCCESS;;
}

/*******************************************************************************
** Name: I2SDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.15
** Time: 8:24:52
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2SDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    I2S_DEV_ARG stI2Sarg;
    //Get I2SDev ID...
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

    if (RKDev_Delete(DEV_CLASS_I2S, DevID, &stI2Sarg) != RK_SUCCESS)
    {
        printf("I2SDev delete failure DevID %d",DevID);
    }

    if (stI2Sarg.hDma != NULL)
    {
        if (RKDev_Close(stI2Sarg.hDma) != RK_SUCCESS)
        {
            printf("i2s hDma close failure\n");
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: I2SDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aron.chen
** Date: 2015.6.13
** Time: 16:34:49
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL FUN rk_err_t I2SDevShellCreate(HDC dev, uint8 * pstr)
{
    I2S_DEV_ARG stI2Sarg;
    rk_err_t ret;
    uint32 DevID;

#ifdef SHELL_HELP
    pstr--;
    if (pstr[0] == '.')
    {
        //list have sub cmd
        pstr++;
        if (StrCmpA(pstr, "help", 4) == 0)
        {
            rk_print_string("usbotg.mc : usbotgdev mc cmd.\r\n");
            return RK_SUCCESS;
        }
    }
#endif

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

    //Init I2SgDev arg..
#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
    stI2Sarg.hDma = RKDev_Open(DEV_CLASS_DMA, DMA_CHN0, NOT_CARE);
    if (stI2Sarg.hDma == NULL)
    {
        printf("DMA RKDev_Open failure\n");
        return RK_ERROR;
    }
    printf("DMA RKDev_Open success\n");

    stI2Sarg.Channel = I2S_SEL_ACODEC;

    stI2Sarg.req_arg.i2smode = I2S_SLAVE_MODE;
    stI2Sarg.req_arg.i2sCS = I2S_IN;
    stI2Sarg.req_arg.I2S_FS = I2S_FS_44100Hz;
    stI2Sarg.req_arg.BUS_FORMAT = I2S_FORMAT;
    stI2Sarg.req_arg.Data_width = I2S_DATA_WIDTH24;
    stI2Sarg.req_arg.I2S_Bus_mode = I2S_NORMAL_MODE;

    //Create I2SgDev...
    ret=RKDev_Create(DEV_CLASS_I2S, I2S_DEV0, &stI2Sarg);
    if (ret != RK_SUCCESS)
    {
        printf("I2S RKDev_Create failure\n");
        return RK_ERROR;
    }
    printf("\nI2S RKDev_Create success\n");
#endif
    return RK_SUCCESS;
}


//#else
_DRIVER_I2S_I2SDEVICE_SHELL_
SHELL API rk_err_t I2SDev_Shell(HDC dev,  uint8 * pstr)
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

    ret = ShellCheckCmd(ShellI2SName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                                 //remove '.',the point is the useful item

    switch (i)
    {
        case 0x00:
            //ret = I2SDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = I2SDevShellCreate(dev,pItem);
            break;

        case 0x02:
            ret = I2SDevShellDel(dev,pItem);
            break;

        case 0x03:
            //ret = I2SDevShellTest(dev,pItem);
            break;

        case 0x04:
            //ret = I2SDevShellHelp(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;
}
#else
rk_err_t I2SDev_Shell(HDC dev,  uint8 * pstr)
{

}
#endif

#endif





