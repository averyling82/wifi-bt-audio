/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                        All rights reserved.
*
* FileName: Driver\SPI\SpiDevice.c
* Owner: Aaron.sun
* Date: 2014.5.30
* Time: 9:07:32
* Desc: SPI DEVICE CLASS
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    Aaron.sun     2014.5.30     9:07:32   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_SPI_SPIDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "DeviceInclude.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum _SPI_DEV_STATUS
{
    SPI_DEV_IDLE,
    SPI_DEV_LOCKED,
    SPI_DEV_NUM

}SPI_DEV_STATUS;

typedef  struct _RX_TX_ITEM
{
    uint8* buffer;
    uint32 size;
    uint32 NeedTransLen;

}RX_TX_ITEM;

typedef struct _SPI_CONFIG
{
    uint32 SpiRate;
    uint32 CtrlMode;

}SPI_CONFIG;


typedef  struct _SPI_DEVICE_CLASS
{
    DEVICE_CLASS stSpiDevice;
    pSemaphore osSpiOperReqSem;
    pSemaphore osSpiOperSem;
    pSemaphore lock;
    uint32 CurCh;
    SPI_CONFIG stConfig[SPI_MAX_CH];

    RX_TX_ITEM stRxItem;
    RX_TX_ITEM stTxItem;

    uint32 dwRxTxStatus;
    uint32 mode;
    pSpiRxTx_complete SpiRxTx_complete;

}SPI_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SPI_DEVICE_CLASS * gpstSpiDevISR[SPI_DEVICE_MAX];



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
void SpiDMAIntIsr_Test(void);
void SpiIntIsr_Test_RX(void);
void SpiIntIsr_Test_TX(void);
rk_err_t SpiDevShellBspDMA_ReadWrite(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspDMA_Write(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspPIO_ReadWrite(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspPIO_Write(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspDeinit(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspInit(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBspHelp(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellBsp(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellHelp(HDC dev,  uint8 * pstr);
void SpiDevIntIsr0(void);
void SpiDevIntIsr1(void);
rk_err_t SpiDevShellTest(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellDel(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellMc(HDC dev, uint8 * pstr);
rk_err_t SpiDevShellPcb(HDC dev, uint8 * pstr);
void SpiDevIntIsr(uint32 DevID);
rk_err_t SpiDevDelete(HDC dev);
rk_err_t SpiDevDeInit(SPI_DEVICE_CLASS * pstSpiDev);
rk_err_t SpiDevInit(SPI_DEVICE_CLASS * pstSpiDev);
rk_err_t SpiDevResume(HDC dev);
rk_err_t SpiDevSuspend(HDC dev);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiDev_Realse_Channel
** Input:HDC dev,  uint32 Ch
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 17:09:17
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON API rk_err_t SpiDev_Realse_Channel(HDC dev,  uint32 Ch)
{
    SPI_DEVICE_CLASS * pstSpiDev = (SPI_DEVICE_CLASS *)dev;

    rkos_semaphore_give(pstSpiDev->osSpiOperReqSem);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiDev_SetChannel
** Input:HDC dev, uint32 Ch, uint32 SpiRate, uint32 CtrlMode
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 17:00:58
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON API rk_err_t SpiDev_SetChannel(HDC dev, uint32 Ch, uint32 SpiRate, uint32 CtrlMode)
{
     SPI_DEVICE_CLASS * pstSpiDev = (SPI_DEVICE_CLASS *)dev;

     rkos_semaphore_take(pstSpiDev->osSpiOperReqSem, MAX_DELAY);

     if(pstSpiDev->CurCh != Ch)
     {
         pstSpiDev->CurCh = Ch;
         pstSpiDev->stConfig[Ch].CtrlMode = CtrlMode;
         pstSpiDev->stConfig[Ch].SpiRate = SpiRate;
         //SPIInit(Ch, SpiRate, CtrlMode);
         SpiDevHwInit(((DEVICE_CLASS *)pstSpiDev)->DevID, Ch);
     }

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiDev_Read_Write
** Input:HDC dev, uint8 * ReadBuf, uint8 * WriteBuf, uint32 Size, uint32 Mode
** Return: rk_size_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 16:55:05
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON API rk_size_t SpiDev_Read_Write(HDC dev, uint8 * ReadBuf, uint8 * WriteBuf, uint32 Size, uint32 Mode, pSpiRxTx_complete pfRxTxComplete)
{

}


/*******************************************************************************
** Name: SpiDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON API rk_err_t SpiDev_Read(HDC dev, uint8 * pBuf, uint32 size, uint32 DummySize, uint32 Mode, pSpiRxTx_complete pfRxComplete)
{
    rk_size_t realsize;

    SPI_DEVICE_CLASS * pstSpiDev = (SPI_DEVICE_CLASS *)dev;

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if(pstSpiDev->CurCh > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

    if (Mode == SYNC_MODE)
    {

        rkos_semaphore_take(pstSpiDev->lock, MAX_DELAY);
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_PIO_READ;

        pstSpiDev->stRxItem.buffer = (uint8*) pBuf;
        pstSpiDev->stRxItem.size   = size;
        pstSpiDev->stRxItem.NeedTransLen = size;
        SpiEnableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        SPIPioRead(((DEVICE_CLASS*)pstSpiDev)->DevID, pstSpiDev->stRxItem.NeedTransLen);

        realsize = SPIReadFIFO(((DEVICE_CLASS*)pstSpiDev)->DevID,pstSpiDev->stRxItem.buffer, pstSpiDev->stRxItem.NeedTransLen);

        if(realsize == pstSpiDev->stRxItem.NeedTransLen)
        {
             pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
             SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);
             SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
             rkos_semaphore_give(pstSpiDev->lock);
             return size;
        }
        else
        {
             pstSpiDev->stRxItem.buffer += realsize;
             pstSpiDev->stRxItem.NeedTransLen -= realsize;
        }

        SpiGetInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
        SpiEnableRxInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
        rkos_semaphore_take(pstSpiDev->osSpiOperSem, MAX_DELAY);
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
        SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);

        SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        rkos_semaphore_give(pstSpiDev->lock);
        return size;

    }
    else if (Mode == ASYNC_MODE)
    {

    }

    return NULL;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiDevIntIsr0
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 14:27:27
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON FUN void SpiDevIntIsr0(void)
{
    SpiDevIntIsr(0);
}

/*******************************************************************************
** Name: SpiDevIntIsr0
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 14:27:27
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON FUN void SpiDevIntIsr1(void)
{
    SpiDevIntIsr(1);
}

/*******************************************************************************
** Name: SpiDevIntIsr
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON FUN void SpiDevIntIsr(uint32 DevID)
{
    uint32 spiIntType, realsize;

    //portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    spiIntType = SpiGetInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);

    if (gpstSpiDevISR[DevID] != NULL)
    {

			if (spiIntType & SPI_RX_FIFO_FULL)
        {

            if ( (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_READ)
                || (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_READ))
            {

                if (gpstSpiDevISR[DevID]->stRxItem.NeedTransLen)
                {

                    realsize = SPIReadFIFO(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID,
                        gpstSpiDevISR[DevID]->stRxItem.buffer,
                        gpstSpiDevISR[DevID]->stRxItem.NeedTransLen);

                    gpstSpiDevISR[DevID]->stRxItem.buffer += realsize;
                    gpstSpiDevISR[DevID]->stRxItem.NeedTransLen -= realsize;


                    if (gpstSpiDevISR[DevID]->stRxItem.NeedTransLen == 0)
                    {
                        if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_READ)
                        {
//                            rkos_semaphore_give_fromisr(gpstSpiDevISR[DevID]->osSpiOperSem, &xHigherPriorityTaskWoken);
							  rkos_semaphore_give_fromisr(gpstSpiDevISR[DevID]->osSpiOperSem);
                        }
                        else if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_READ)
                        {

                        }

                        SpiDisalbeRxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);

                    }
                }
            }
        }

    }


    if (spiIntType & SPI_TX_FIFO_EMPTY)
    {

        if ((gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                || (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE))
        {

            if (gpstSpiDevISR[DevID]->stTxItem.NeedTransLen)
            {

                realsize = SPIWriteFIFO(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID,gpstSpiDevISR[DevID]->stTxItem.buffer, gpstSpiDevISR[DevID]->stTxItem.NeedTransLen);

                gpstSpiDevISR[DevID]->stTxItem.buffer += realsize;
                gpstSpiDevISR[DevID]->stTxItem.NeedTransLen -= realsize;


                if(gpstSpiDevISR[DevID]->stTxItem.NeedTransLen == 0)
                {
                    if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
                    {
                        gpstSpiDevISR[DevID]->dwRxTxStatus = DEVICE_STATUS_IDLE;

                    }
                    else if (gpstSpiDevISR[DevID]->dwRxTxStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
                    {
                        //rkos_semaphore_give_fromisr(gpstSpiDevISR[DevID]->osSpiOperSem, &xHigherPriorityTaskWoken);
						rkos_semaphore_give_fromisr(gpstSpiDevISR[DevID]->osSpiOperSem);
                    }

                    SpiDisalbeTxInt(((DEVICE_CLASS*)gpstSpiDevISR[DevID])->DevID);
                }

            }

        }

    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_COMMON_
COMMON API rk_size_t SpiDev_Write(HDC dev, uint8 * pBuf, uint32 size, uint32 Mode, pSpiRxTx_complete pfTxComplete)
{
    rk_size_t realsize;

    SPI_DEVICE_CLASS * pstSpiDev = (SPI_DEVICE_CLASS *)dev;

    if (pstSpiDev == NULL)
    {
        return RK_ERROR;
    }

    if(pstSpiDev->CurCh > SPI_MAX_CH)
    {
        return RK_ERROR;
    }

    if (Mode == SYNC_MODE)
    {
        rkos_semaphore_take(pstSpiDev->lock, MAX_DELAY);
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_SYNC_PIO_WRITE;

        pstSpiDev->stTxItem.buffer = (uint8*) pBuf;
        pstSpiDev->stTxItem.size   = size;
        pstSpiDev->stTxItem.NeedTransLen = size;


        SpiEnableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        SPIPioWrite(((DEVICE_CLASS*)pstSpiDev)->DevID);
        //Delay100cyc(100);
        realsize = SPIWriteFIFO(((DEVICE_CLASS*)pstSpiDev)->DevID,pstSpiDev->stTxItem.buffer, pstSpiDev->stTxItem.NeedTransLen);

        if(realsize == pstSpiDev->stTxItem.NeedTransLen)
        {
             pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
             SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);
             SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
             rkos_semaphore_give(pstSpiDev->lock);
             return size;
        }
        else
        {
             pstSpiDev->stTxItem.buffer += realsize;
             pstSpiDev->stTxItem.NeedTransLen -= realsize;
        }

        SpiGetInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
        SpiEnableTxInt(((DEVICE_CLASS*)pstSpiDev)->DevID);
        rkos_semaphore_take(pstSpiDev->osSpiOperSem, MAX_DELAY);
        pstSpiDev->dwRxTxStatus = DEVICE_STATUS_IDLE;
        SpiWaitIdle(((DEVICE_CLASS*)pstSpiDev)->DevID);

        SpiDisableChannel(((DEVICE_CLASS*)pstSpiDev)->DevID, 0);
        rkos_semaphore_give(pstSpiDev->lock);
        return size;

    }
    else if (Mode == ASYNC_MODE)
    {

    }

    return NULL;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
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
/*******************************************************************************
** Name: SpiDev_Create
** Input:void * arg
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT API HDC SpiDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS * pstDev;
    SPI_DEVICE_CLASS * pstSpiDev;
    SPI_DEV_ARG * pstSpiArg = (SPI_DEV_ARG *)arg;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    pstSpiDev =  rkos_memory_malloc(sizeof(SPI_DEVICE_CLASS));
    if (pstSpiDev == NULL)
    {
        return pstSpiDev;
    }

    pstSpiDev->osSpiOperReqSem        = rkos_semaphore_create(1,1);
    pstSpiDev->osSpiOperSem       = rkos_semaphore_create(1,0);
    pstSpiDev->lock = rkos_semaphore_create(1,1);

    if ((pstSpiDev->osSpiOperReqSem && pstSpiDev->osSpiOperSem) == 0)
    {

        rkos_semaphore_delete(pstSpiDev->osSpiOperReqSem);
        rkos_semaphore_delete(pstSpiDev->osSpiOperSem);

        rkos_memory_free(pstSpiDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstSpiDev;

    pstDev->suspend = SpiDevSuspend;
    pstDev->resume  = SpiDevResume;
    pstDev->DevID = DevID;
    pstSpiDev->CurCh = pstSpiArg->Ch;
    pstSpiDev->stConfig[pstSpiDev->CurCh].SpiRate = pstSpiArg->SpiRate;
    pstSpiDev->stConfig[pstSpiDev->CurCh].CtrlMode = pstSpiArg->CtrlMode;

    //pstSpiDev->CurCh = -1;

    gpstSpiDevISR[DevID] = pstSpiDev;

    SpiDevInit(pstSpiDev);

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
** Name: SpiDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT FUN rk_err_t SpiDevDelete(HDC dev)
{

}
/*******************************************************************************
** Name: SpiDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT FUN rk_err_t SpiDevDeInit(SPI_DEVICE_CLASS * pstSpiDev)
{
    if(((DEVICE_CLASS*)pstSpiDev)->DevID == 0)
    {
        IntDisable(INT_ID_SPI0);

        IntPendingClear(INT_ID_SPI0);

        IntUnregister(INT_ID_SPI0);

        ScuClockGateCtr(PCLK_SPI0_GATE,0);
    }
    else
    {
        IntDisable(INT_ID_SPI1);

        IntPendingClear(INT_ID_SPI1);

        IntUnregister(INT_ID_SPI1);

        ScuClockGateCtr(PCLK_SPI1_GATE,0);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiDevInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT FUN rk_err_t SpiDevInit(SPI_DEVICE_CLASS * pstSpiDev)
{

	 //open uart clk
	if(((DEVICE_CLASS*)pstSpiDev)->DevID == 0)
	{
        ScuClockGateCtr(CLK_SPI0_GATE, 0);
        ScuClockGateCtr(PCLK_SPI0_GATE, 0);
        DelayMs(1);
        ScuClockGateCtr(CLK_SPI0_GATE, 1);
        ScuClockGateCtr(PCLK_SPI0_GATE, 1);
        ScuSoftResetCtr(SPI0_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(SPI0_SRST, 0);
        //SetSPIFreq(0,PLL_MUX_CLK,96000000);
        SetSPIFreq(0,XIN24M,24000000);
        //open rst uart ip

	}
    else if(((DEVICE_CLASS*)pstSpiDev)->DevID == 1)
    {
        ScuClockGateCtr(CLK_SPI1_GATE, 1);
        ScuClockGateCtr(PCLK_SPI1_GATE, 1);
        SetSPIFreq(1,XIN24M,24000000);

        //open rst uart ip
        ScuSoftResetCtr(SPI1_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(SPI1_SRST, 0);
    }
    SpiDevHwInit(((DEVICE_CLASS *)pstSpiDev)->DevID, pstSpiDev->CurCh);
    SPIInit(((DEVICE_CLASS *)pstSpiDev)->DevID,0, pstSpiDev->stConfig[pstSpiDev->CurCh].SpiRate,
            pstSpiDev->stConfig[pstSpiDev->CurCh].CtrlMode);

    if(((DEVICE_CLASS *)pstSpiDev)->DevID == 0)
    {
        IntRegister(INT_ID_SPI0,SpiDevIntIsr0);
        IntPendingClear(INT_ID_SPI0);
        IntEnable(INT_ID_SPI0);
    }
    else if(((DEVICE_CLASS*)pstSpiDev)->DevID == 1)
    {
        IntRegister(INT_ID_SPI1,SpiDevIntIsr1);
        IntPendingClear(INT_ID_SPI1);
        IntEnable(INT_ID_SPI1);
    }


    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT FUN rk_err_t SpiDevResume(HDC dev)
{

}
/*******************************************************************************
** Name: SpiDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT FUN rk_err_t SpiDevSuspend(HDC dev)
{

}


#ifdef _SPI_DEV_SHELL_
_DRIVER_SPI_SPIDEVICE_SHELL_
static SHELL_CMD ShellSpiName[] =
{
    "pcb",SpiDevShellPcb,"list spidev pcb inf","spi.pcb [object id]",
    "create",SpiDevShellMc,"create spidevice","spi.create",
    "delete",SpiDevShellDel,"delete spidevice","spi.delete",
    "test",SpiDevShellTest,"test spidevice","spi.test",
#ifdef SHELL_BSP
    "bsp",SpiDevShellBsp,"NULL","NULL",
#endif
    "\b",NULL,"NULL","NULL",
};
#ifdef SHELL_BSP
static SHELL_CMD ShellSpiBspName[] =
{
    "init",SpiDevShellBspInit,"NULL","NULL",
    "deinit",SpiDevShellBspDeinit,"NULL","NULL",
    "dmaw",SpiDevShellBspDMA_Write,"NULL","NULL",
    "dmawr",SpiDevShellBspDMA_ReadWrite,"NULL","NULL",
    "piow",SpiDevShellBspPIO_Write,"NULL","NULL",
    "piowr",SpiDevShellBspPIO_ReadWrite,"NULL","NULL",
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
#define SPI_CTL_TXRX_MASTER_TEST ( SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | BIG_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW \
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_CTL_TXRX_SLAVE_TEST ( SPI_SLAVE_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | BIG_ENDIAN_MODE | CS_2_SCLK_OUT_1_2_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_LOW \
                            | SERIAL_CLOCK_PHASE_MIDDLE | DATA_FRAME_8BIT)
_DRIVER_SPI_SPIDEVICE_SHELL_ uint8 Spi_TestBuffer[1024];
_DRIVER_SPI_SPIDEVICE_SHELL_ uint8 Spi_TestBuffer_Read[1024] = {0,};

_DRIVER_SPI_SPIDEVICE_SHELL_ uint32 Spi_NeedTransLen;

_DRIVER_SPI_SPIDEVICE_SHELL_ uint32 Spi_NeedTransLen_RX;
_DRIVER_SPI_SPIDEVICE_SHELL_ uint32 Spi_Testflag;

_DRIVER_SPI_SPIDEVICE_SHELL_ eSPI_TRANSFER_MODE_t SPI_TEST_MODE;
_DRIVER_SPI_SPIDEVICE_SHELL_ eSPI_ch_t SPI_CH_TEST;
_DRIVER_SPI_SPIDEVICE_SHELL_ DMA_CFGX SPI0ControlDmaCfg_TX  = {DMA_CTLL_SPI0_8_MTX, DMA_CFGL_SPI0_8_MTX, DMA_CFGH_SPI0_8_MTX,0};
_DRIVER_SPI_SPIDEVICE_SHELL_ DMA_CFGX SPI1ControlDmaCfg_TX  = {DMA_CTLL_SPI1_8_MTX, DMA_CFGL_SPI1_8_MTX, DMA_CFGH_SPI1_8_MTX,0};

_DRIVER_SPI_SPIDEVICE_SHELL_ DMA_CFGX SPI0ControlDmaCfg_RX  = {DMA_CTLL_SPI0_8_MRX, DMA_CFGL_SPI0_8_MRX, DMA_CFGH_SPI0_8_MRX,0};
_DRIVER_SPI_SPIDEVICE_SHELL_ DMA_CFGX SPI1ControlDmaCfg_RX  = {DMA_CTLL_SPI1_8_MRX, DMA_CFGL_SPI1_8_MRX, DMA_CFGH_SPI1_8_MRX,0};
/*******************************************************************************
** Name: SpiDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL API rk_err_t SpiDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSpiName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSpiName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellSpiName[i].CmdDes, pItem);
    if(ShellSpiName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSpiName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: SpiDMAIntIsr_Test
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN void SpiDMAIntIsr_Test(void)
{
    uint32 spiIntType, realsize,TX_FIFO_ADDR;

    if(SPI_TRANSMIT_RECEIVE == SPI_TEST_MODE)
    {
        DmaDisableInt(2);
        return;
    }

    TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH_TEST);
    DmaDisableInt(2);
    if(SPI_CH0 == SPI_CH_TEST)
    {
        DmaEnableInt(2);
        DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)TX_FIFO_ADDR,1024, &SPI0ControlDmaCfg_TX, NULL);
    }
    else
    {
        DmaEnableInt(2);
        DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)TX_FIFO_ADDR,1024, &SPI1ControlDmaCfg_TX, NULL);
    }

}
/*******************************************************************************
** Name: SpiIntIsr_Test_RX
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN void SpiIntIsr_Test_RX(void)
{
    uint32 spiIntType, realsize;
}
/*******************************************************************************
** Name: SpiIntIsr_Test_TX
** Input:void
** Return: void
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN void SpiIntIsr_Test_TX(void)
{
    uint32 spiIntType, realsize;

    spiIntType = SpiGetInt(SPI_CH_TEST);

    if (spiIntType & SPI_TX_FIFO_EMPTY)
    {
        if (Spi_NeedTransLen)
        {
            realsize = SPIWriteFIFO(SPI_CH_TEST,&Spi_TestBuffer[Spi_NeedTransLen],Spi_NeedTransLen);
            Spi_NeedTransLen -= realsize;
            if(Spi_NeedTransLen == 0)
            {
                Spi_Testflag = 0;
                SpiDisalbeTxInt(SPI_CH_TEST);
            }
        }
        else
        {
            Spi_Testflag = 0;
            SpiDisalbeTxInt(SPI_CH_TEST);
        }
    }
}
#ifdef SHELL_BSP
/*******************************************************************************
** Name: SpiDevShellBspDMA_ReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:39:07
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspDMA_ReadWrite(HDC dev, uint8 * pstr)
{
    rk_size_t realsize,_RXrealsize;
    pSPI_REG Spi_Reg0,Spi_Reg1;
    uint8 Spi_clockBuffer[5] = {0,};
    eSPI_ch_t spiChMaster,spiChSlave;
    uint32 i = 0;
    uint32 SPI0_RX_FIFO_ADDR, SPI0_TX_FIFO_ADDR,SPI1_RX_FIFO_ADDR,SPI1_TX_FIFO_ADDR;

    Spi_Reg0 = SpiGetCH(SPI_CH0);
    Spi_Reg1 = SpiGetCH(SPI_CH1);

    SPI0_RX_FIFO_ADDR = SpiGetRxFIFOaddr(SPI_CH0);
    SPI0_TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH0);

    SPI1_RX_FIFO_ADDR = SpiGetRxFIFOaddr(SPI_CH1);
    SPI1_TX_FIFO_ADDR = SpiGetTxFIFOaddr(SPI_CH1);


    if(StrCmpA(pstr, "0", 1) == 0)
    {
        spiChMaster = SPI_CH0;
        spiChSlave = SPI_CH1;
        DEBUG("Master-SPI0;Slave-SPI1\n");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        spiChMaster = SPI_CH1;
        spiChSlave = SPI_CH0;
        DEBUG("Master-SPI1;Slave-SPI0\n");
    }

    Spi_NeedTransLen = 0;
    Spi_NeedTransLen_RX = 5;
    Spi_Testflag = 1;
    for(i=0;i<10;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }

     //open uart clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst uart ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA ,SpiDMAIntIsr_Test);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(2);

    SPIInit(spiChMaster,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST);

    SPIInit(spiChSlave,0, 12*1000*1000, SPI_CTL_TXRX_SLAVE_TEST);

    SPIDmaRead(spiChSlave,5);

    SpiEanbleChannel(spiChMaster,0);
    SPIDmaWrite(spiChMaster);

    //写入0x55 0xaa 0x55 0xaa 0x55 发送到slave
    if(spiChMaster == SPI_CH0)
    {
        DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)SPI0_TX_FIFO_ADDR,5, &SPI0ControlDmaCfg_TX, NULL);
    }
    else
    {
        DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)SPI1_TX_FIFO_ADDR,5, &SPI1ControlDmaCfg_TX, NULL);
    }

    SpiGetInt(spiChMaster);
    //SpiEnableTxInt(SPI_CH_TEST);
    SpiWaitIdle(spiChMaster);

    //从Slave读出0x55 0xaa 0x55 0xaa 0x55
    DmaEnableInt(2);

    if(spiChMaster == SPI_CH0)
    {
        DmaConfig(2, (uint32)SPI1_RX_FIFO_ADDR,(uint32)Spi_TestBuffer_Read, 5, &SPI1ControlDmaCfg_RX, NULL);
    }
    else
    {
        DmaConfig(2, (uint32)SPI0_RX_FIFO_ADDR,(uint32)Spi_TestBuffer_Read, 5, &SPI0ControlDmaCfg_RX, NULL);
    }

    //SPIReadFIFO(spiChSlave,Spi_TestBuffer_Read,5);

    //读出master Read FIFO里面的无效数据
    SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);

    //check
    if((Spi_TestBuffer_Read[0] == 0x55)
       && (Spi_TestBuffer_Read[1] == 0xaa)
       && (Spi_TestBuffer_Read[2] == 0x55)
       && (Spi_TestBuffer_Read[3] == 0xaa)
       && (Spi_TestBuffer_Read[4] == 0x55))
    {
       //写入0x55 0xaa 0x55 0xaa 0x55 发送到master
       //SPIDmaWrite(spiChSlave);

       DmaEnableInt(2);
       //写入数据，需要MASTER提供clock
       if(spiChMaster == SPI_CH0)
       {
           DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)SPI1_TX_FIFO_ADDR,5, &SPI1ControlDmaCfg_TX, NULL);
       }
       else
       {
           DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)SPI0_TX_FIFO_ADDR,5, &SPI0ControlDmaCfg_TX, NULL);
       }
       //SPIWriteFIFO(spiChSlave,Spi_TestBuffer,5);

       SPIWriteFIFO(spiChMaster,Spi_clockBuffer,5); //master 发送无效数据0x00，只提供clock

       SpiGetInt(spiChMaster);
       //SpiEnableTxInt(SPI_CH_TEST);
       SpiWaitIdle(spiChMaster);

       DelayMs(10);
       //读出0x55 0xaa 0x55 0xaa 0x55
       //SPIDmaRead(spiChMaster,5);
       DmaEnableInt(2);

       if(spiChMaster == SPI_CH0)
       {
           DmaConfig(2, (uint32)SPI0_RX_FIFO_ADDR,(uint32)&Spi_TestBuffer_Read[10], 5, &SPI0ControlDmaCfg_RX, NULL);
       }
       else
       {
           DmaConfig(2,(uint32)SPI1_RX_FIFO_ADDR,(uint32)&Spi_TestBuffer_Read[10],5, &SPI1ControlDmaCfg_RX, NULL);
       }

       //SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);
       if((Spi_TestBuffer_Read[10] == 0x55)
       && (Spi_TestBuffer_Read[11] == 0xaa)
       && (Spi_TestBuffer_Read[12] == 0x55)
       && (Spi_TestBuffer_Read[13] == 0xaa)
       && (Spi_TestBuffer_Read[14] == 0x55))
       {
          Spi_Testflag = 0;
       }
    }

    while(Spi_Testflag);
    DEBUG("SPI_TRANSMIT_RECEIVE is ok\n");
}
/*******************************************************************************
** Name: SpiDevShellBspDMA_Write
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:38:37
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspDMA_Write(HDC dev, uint8 * pstr)
{
    rk_size_t realsize;
    pSPI_REG Spi_Reg;

    uint32 i = 0;
    DEBUG("SpiDevShellBspDMA_Write %d\n",SPI_CH_TEST);
    Spi_Reg = SpiGetCH(SPI_CH_TEST);
    Spi_NeedTransLen = 0;
    for(i=0;i<1024;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }

    //open uart clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);

    //open rst uart ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_DMA ,SpiDMAIntIsr_Test);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DmaEnableInt(2);

    SPIInit(SPI_CH_TEST,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST | TRANSMIT_ONLY);
    SpiEanbleChannel(SPI_CH_TEST,0);
    if(SPI_TRANSMIT_ONLY == SPI_TEST_MODE)
    {
        DEBUG("SPI_TRANSMIT_ONLY\n");

        Spi_Testflag = 1;
        SPIDmaWrite(SPI_CH_TEST);
        if(SPI_CH0 == SPI_CH_TEST)
        {
            DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)(&(Spi_Reg->SPI_TXDR)),1024, &SPI0ControlDmaCfg_TX, NULL);
        }
        else
        {
            DmaConfig(2, (uint32)Spi_TestBuffer, (uint32)(&(Spi_Reg->SPI_TXDR)),1024, &SPI1ControlDmaCfg_TX, NULL);
        }
    }
    else
    {
       DEBUG("SPI_RECEIVE_ONLY\n");

       //I2SInit(I2S_DEV_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }


    while(1);
}
/*******************************************************************************
** Name: SpiDevShellBspPIO_ReadWrite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:38:05
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspPIO_ReadWrite(HDC dev, uint8 * pstr)
{
    rk_size_t realsize,_RXrealsize;
    pSPI_REG Spi_Reg0,Spi_Reg1;
    uint8 Spi_clockBuffer[5] = {0,};
    eSPI_ch_t spiChMaster,spiChSlave;
    uint32 i = 0;

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        spiChMaster = SPI_CH0;
        spiChSlave = SPI_CH1;
        DEBUG("Master-SPI0;Slave-SPI1\n");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        spiChMaster = SPI_CH1;
        spiChSlave = SPI_CH0;
        DEBUG("Master-SPI1;Slave-SPI0\n");
    }

    Spi_NeedTransLen = 0;
    Spi_NeedTransLen_RX = 5;

    for(i=0;i<10;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }


    SPIInit(spiChMaster,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST);

    SPIInit(spiChSlave,0, 12*1000*1000, SPI_CTL_TXRX_SLAVE_TEST);

    SPIPioRead(spiChSlave,5);

    SpiEanbleChannel(spiChMaster,0);
    SPIPioWrite(spiChMaster);



    Spi_Testflag = 1;
    //写入0x55 0xaa 0x55 0xaa 0x55 发送到slave
    SPIWriteFIFO(spiChMaster,&Spi_TestBuffer[0],5);

    SpiGetInt(spiChMaster);
    //SpiEnableTxInt(SPI_CH_TEST);
    SpiWaitIdle(spiChMaster);

    //读出0x55 0xaa 0x55 0xaa 0x55
    SPIReadFIFO(spiChSlave,Spi_TestBuffer_Read,5);

    //读出master Read FIFO里面的无效数据
    SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);

    //check
    if((Spi_TestBuffer_Read[0] == 0x55)
       && (Spi_TestBuffer_Read[1] == 0xaa)
       && (Spi_TestBuffer_Read[2] == 0x55)
       && (Spi_TestBuffer_Read[3] == 0xaa)
       && (Spi_TestBuffer_Read[4] == 0x55))
    {
       //写入0x55 0xaa 0x55 0xaa 0x55 发送到master
       SPIWriteFIFO(spiChSlave,Spi_TestBuffer,5); //写入数据，需要MASTER提供clock

       SPIWriteFIFO(spiChMaster,Spi_clockBuffer,5); //master 发送无效数据0x00，只提供clock

       SpiGetInt(spiChMaster);
       //SpiEnableTxInt(SPI_CH_TEST);
       SpiWaitIdle(spiChMaster);

       DelayMs(10);
       //读出0x55 0xaa 0x55 0xaa 0x55
       SPIReadFIFO(spiChMaster,&Spi_TestBuffer_Read[10],5);
       if((Spi_TestBuffer_Read[10] == 0x55)
       && (Spi_TestBuffer_Read[11] == 0xaa)
       && (Spi_TestBuffer_Read[12] == 0x55)
       && (Spi_TestBuffer_Read[13] == 0xaa)
       && (Spi_TestBuffer_Read[14] == 0x55))
       {
          Spi_Testflag = 0;
       }
    }

    while(Spi_Testflag);
    DEBUG("SPI_TRANSMIT_RECEIVE is ok\n");

}
/*******************************************************************************
** Name: SpiDevShellBspPIO_Wtite
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.12.9
** Time: 20:37:26
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspPIO_Write(HDC dev, uint8 * pstr)
{
    rk_size_t realsize;

    uint32 i = 0;
    DEBUG("SpiDevShellBspPIO_Write %d\n",SPI_CH_TEST);
    Spi_NeedTransLen = 0;

    for(i=0;i<1024;i++)
    {
        Spi_TestBuffer[i++] = 0x55;
        Spi_TestBuffer[i] = 0xaa;
    }
    SPIInit(SPI_CH_TEST,0, 12*1000*1000, SPI_CTL_TXRX_MASTER_TEST | TRANSMIT_ONLY);

    if(SPI_CH0 == SPI_CH_TEST)
    {
        IntRegister(INT_ID_SPI0 ,SpiIntIsr_Test_TX);
        IntPendingClear(INT_ID_SPI0);
        IntEnable(INT_ID_SPI0);
    }
    else
    {
        IntRegister(INT_ID_SPI1 ,SpiIntIsr_Test_TX);
        IntPendingClear(INT_ID_SPI1);
        IntEnable(INT_ID_SPI1);
    }

    if(SPI_TRANSMIT_ONLY == SPI_TEST_MODE)
    {
        SpiEanbleChannel(SPI_CH_TEST,0);
        SPIPioWrite(SPI_CH_TEST);
        DEBUG("SPI_TRANSMIT_ONLY\n");
        while(1)
        {
            Spi_Testflag = 1;
            realsize = SPIWriteFIFO(SPI_CH_TEST,Spi_TestBuffer,1024);
            if(realsize == 1024)
            {
				SpiWaitIdle(SPI_CH_TEST);
            }
            else
            {
                Spi_NeedTransLen += realsize;
            }

            SpiGetInt(SPI_CH_TEST);
            SpiEnableTxInt(SPI_CH_TEST);
            SpiWaitIdle(SPI_CH_TEST);

            while(Spi_Testflag);
         }
    }
    else
    {
       DEBUG("SPI_RECEIVE_ONLY\n");

       //I2SInit(I2S_DEV_TEST,I2S_SLAVE_MODE,I2S_EXT,I2S_TEST_FS,I2S_FORMAT,I2S_DATA_WIDTH16,I2S_NORMAL_MODE);
    }

    while(1);
}
/*******************************************************************************
** Name: SpiDevShellBspDeinit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:46:22
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspDeinit(HDC dev, uint8 * pstr)
{

}

/*******************************************************************************
** Name: SpiDevShellBspInit
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:45:43
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBspInit(HDC dev, uint8 * pstr)
{

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
        SPI_CH_TEST = SPI_CH0;
        DEBUG("SPI_TRANSMIT_ONLY 0");
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_ONLY;
        SPI_CH_TEST = SPI_CH1;
        DEBUG("SPI_TRANSMIT_ONLY 1");
    }
    else if(StrCmpA(pstr, "2", 1) == 0)
    {
        SPI_TEST_MODE = SPI_TRANSMIT_RECEIVE;
        DEBUG("SPI_Read_Write");
    }
    //ScuClockGateCtr(CLOCK_GATE_I2C, 1);
    //ScuClockGateCtr(CLOCK_GATE_I2S, 1);
    //ScuClockGateCtr(CLOCK_GATE_GRF, 1);
    //I2s_Iomux_Set();
    //I2c_Iomux_Set();

    rk_print_string("\r\nspi init over");
}
#endif

#ifdef SHELL_BSP

/*******************************************************************************
** Name: SpiDevShellBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:hj
** Date: 2014.11.11
** Time: 15:31:05
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;

    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSpiBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellSpiBspName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellSpiBspName[i].CmdDes, pItem);
    if(ShellSpiBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSpiBspName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;
}
#endif

/*******************************************************************************
** Name: SpiDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellTest(HDC dev, uint8 * pstr)
{
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

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: SpiDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellDel(HDC dev, uint8 * pstr)
{
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

	return RK_SUCCESS;
}
/*******************************************************************************
** Name: SpiDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellMc(HDC dev, uint8 * pstr)
{
     SPI_DEV_ARG stSpiArg;
     rk_err_t ret;

     if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
     {
         return RK_SUCCESS;
     }

     if(*(pstr - 1) == '.')
     {
        return RK_ERROR;
     }

     stSpiArg.Ch = 0;
     stSpiArg.CtrlMode = SPI_CTL_MODE;
     stSpiArg.SpiRate = SPI_BUS_CLK;

     ret = RKDev_Create(DEV_CLASS_SPI, 0, &stSpiArg);
     if(ret != RK_SUCCESS)
     {
        rk_print_string("Spi Device Create failure");
     }

     return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.30
** Time: 9:18:00
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_SHELL_
SHELL FUN rk_err_t SpiDevShellPcb(HDC dev, uint8 * pstr)
{
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

    return RK_SUCCESS;
}



#endif
#endif

