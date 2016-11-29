/*
********************************************************************************************
*
*  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                     All rights reserved.
*
* FileName: ..\Driver\DMA\DmaDevice.c
* Owner: Aaron.sun
* Date: 2014.4.24
* Time: 10:56:35
* Desc: Dma Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.4.24     10:56:35   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_DMA_DMADEVICE_C__

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
typedef  struct _DMA_DEVICE_CLASS
{
    DEVICE_CLASS stDmaDevice;

    pSemaphore osDmaOperReqSem;
    pDmaFunc DmaIsrCallBack[DMA_CHN_MAX];
    uint32 DmaTransSizeBack[DMA_CHN_MAX];
    DMA_LLP g_llpListn[DMA_CHN_MAX][LLP_MAX_NUM];
    uint32 DmaChStatus;
    uint32 DmaIntStatus;

}DMA_DEVICE_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static DMA_DEVICE_CLASS * gpstDmaDevInf;



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
rk_err_t DmaDevCheckHandler(HDC dev);
rk_err_t ShellDmaBsp(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestChp(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestMulDma(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestLlp(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestInt(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestStartStop(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestBlock(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspTestDma(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspStop(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspStart(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetChP(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetTargetInc(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetSrcInc(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetBlock(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetTarget(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspSetSrc(HDC dev, uint8 * pstr);
rk_err_t ShellDmaBspHelp(HDC dev, uint8 * pstr);
rk_err_t DmaShellHelp(HDC dev, uint8 * pstr);
rk_err_t DmaShellCreate(HDC dev, uint8 * pstr);
rk_err_t DmaShellPcb(HDC dev,  uint8 * pstr);
rk_err_t DmaDevResume(HDC dev);
rk_err_t DmaDevSuspend(HDC dev, uint32 Level);
rk_err_t DmaDevDeInit(DMA_DEVICE_CLASS * pstDmaDev);
rk_err_t DmaDevInit(DMA_DEVICE_CLASS * pstDmaDev);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DmaSoftIntIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.12.30
** Time: 10:57:16
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API void DmaSoftIntIsr(void)
{
    uint32 channel;
    uint32 rawStatus;
    pDmaFunc CallBack;

    rawStatus = gpstDmaDevInf->DmaIntStatus;
    if ((rawStatus & 0x3f) == 0)
    {
        return;
    }

    for (channel = 0; channel < (DMA_CHN_MAX - 1); channel++)
    {
        if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
        {
            CallBack = gpstDmaDevInf->DmaIsrCallBack[channel];
            gpstDmaDevInf->DmaIntStatus &= ~((uint32)(0x01) << channel);
            if (CallBack)
            {
                gpstDmaDevInf->DmaIsrCallBack[channel] = 0;
                CallBack(channel);
            }
        }
    }

}

/*******************************************************************************
** Name: DmaIntIsr
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API void DmaIntIsr(void)
{
    uint32 rawStatus;
    uint32 channel;
    pDmaFunc CallBack;

    while(1)
    {
        rawStatus = DmaGetIntType();
        if((rawStatus & 0x3f) == 0)
        {
            break;
        }

        #if 1
        if (((uint32)(rawStatus)) & ((uint32)(0x01) << (DMA_CHN_MAX - 1)))
        {
            CallBack = gpstDmaDevInf->DmaIsrCallBack[DMA_CHN_MAX - 1];
            DmaDisableInt(DMA_CHN_MAX - 1);
            if (CallBack)
            {
                gpstDmaDevInf->DmaIsrCallBack[DMA_CHN_MAX - 1] = 0;
                CallBack(DMA_CHN_MAX - 1);
            }
        }

        if(((uint32)(rawStatus)) & ~((uint32)(0x01) << (DMA_CHN_MAX - 1)))
        {
            for (channel = 0; channel < (DMA_CHN_MAX - 1); channel++)
            {
                if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
                {
                    DmaDisableInt(channel);
                    gpstDmaDevInf->DmaIntStatus |= ((uint32)(0x01) << channel);
                }
            }
            IntPendingSet(INT_ID_REV0);
        }
        #else
        for (channel = 0; channel < DMA_CHN_MAX; channel++)
        {
            if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
            {
                CallBack = gpstDmaDevInf->DmaIsrCallBack[channel];
                DmaDisableInt(channel);
                if (CallBack)
                {
                    gpstDmaDevInf->DmaIsrCallBack[channel] = 0;
                    CallBack();
                }
            }
        }
        #endif

    }

}


/*******************************************************************************
** Name: DmaDev_RealseChannel
** Input:HDC dev ,uint32 ch
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 15:16:19
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API rk_err_t DmaDev_RealseChannel(HDC dev ,uint32 ch)
{
    uint32 n=0;
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;

    if (pstDmaDev == NULL)
    {
        return RK_ERROR;
    }

    DmaStop(ch);

    if(DmaGetState(ch) == DMA_BUSY)
    {
        printf("\ndma busy");
    }

    rkos_enter_critical();
    pstDmaDev->DmaChStatus &= ~((uint32)0x01 << ch);
    rkos_exit_critical();

}

/*******************************************************************************
** Name: DmaDev_GetChannel
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 14:48:54
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API rk_err_t DmaDev_GetChannel(HDC dev)
{
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;

    uint32 ch;

    if (pstDmaDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstDmaDev->osDmaOperReqSem, MAX_DELAY);

    for (ch = 0; ch < DMA_CHN_MAX - 2; ch++)
    {
        if((pstDmaDev->DmaChStatus & ((uint32)0x01 << ch)) == 0)
        {
            if (DmaGetState(ch) == DMA_IDLE)
            {
                rkos_enter_critical();
                pstDmaDev->DmaChStatus |= ((uint32)0x01 << ch);
                rkos_exit_critical();

                rkos_semaphore_give(pstDmaDev->osDmaOperReqSem);
                return (rk_err_t)ch;
            }
            else
            {
                rk_printf("dma channel error");
            }
        }
    }

    //rk_printf("pstDmaDev->DmaChStatus = %x", pstDmaDev->DmaChStatus);

    rkos_semaphore_give(pstDmaDev->osDmaOperReqSem);

    return RK_ERROR;

}

/*******************************************************************************
** Name: DmaDev_DmaReStart
** Input:HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 13:41:23
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API rk_err_t DmaDev_DmaReStart(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack)
{
    //dma channel configer

    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;

    if (pstDmaDev == NULL)
    {
        return RK_ERROR;
    }

    //rkos_enter_critical();

    if (pstDmaDev->DmaTransSizeBack[ch] == size)
    {
        pstDmaDev->DmaIsrCallBack[ch] = (pDmaFunc)CallBack;
        DmaReConfig(ch, srcAddr, dstAddr, size, pstDmaDev->g_llpListn[ch]);
    }
    else
    {
        pstDmaDev->DmaTransSizeBack[ch] = size;
        pstDmaDev->DmaIsrCallBack[ch] = CallBack;

        DmaConfig(ch, srcAddr, dstAddr, size, g_dmaPar, pstDmaDev->g_llpListn[ch]);
    }

    //rkos_exit_critical();

}

/*******************************************************************************
** Name: DmaDev_DmaStartIIS
** Input:uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack
** Return: rk_err_t
** Owner:cjh
** Date: 2015.10.30
** Time: 11:39:48
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API rk_err_t DmaDev_DmaStartIIS(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack)
{
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;

    uint32   llpNum;
    rk_err_t ret = RK_ERROR;

    /*
    if (pstDmaDev == NULL)
    {
        return RK_ERROR;
    }
    */

    //rkos_enter_critical();

    /*
    if (DmaGetState(ch) == DMA_BUSY)
    {
        ret = RK_ERROR;
        goto out;
    }

    if (size > DMA_MAX_BLOCK_SIZE)
    {
        llpNum  = (size + (LLP_BLOCK_SIZE - 1))/LLP_BLOCK_SIZE;

        if (llpNum > LLP_MAX_NUM)
        {
            ret = RK_ERROR;
            goto out;
        }
    }
    */

    pstDmaDev->DmaTransSizeBack[ch] = size;
    pstDmaDev->DmaIsrCallBack[ch] = CallBack;
    DmaEnableInt(ch);
    DmaConfig(ch, srcAddr, dstAddr, size, g_dmaPar, pstDmaDev->g_llpListn[ch]);
    ret = RK_SUCCESS;

out:
    //rkos_exit_critical();

    return ret;

}

/*******************************************************************************
** Name: DmaDev_DmaStart
** Input:uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 11:39:48
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON API rk_err_t DmaDev_DmaStart(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack)
{
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;

    uint32   llpNum;
    uint32   temp_ch;
    rk_err_t ret = RK_ERROR;

    if (pstDmaDev == NULL)
    {
        return RK_ERROR;
    }

    //rkos_enter_critical();

    temp_ch = ch;
    if (temp_ch >= DMA_CHN_MAX)
    {
        rk_printf("Dma Channel error: ch = %d!\n", ch);
        ret = RK_ERROR;
        goto out;
    }

    if (DmaGetState(ch) == DMA_BUSY)
    {
        ret = RK_ERROR;
        rk_printf("dma channel busy....");
        goto out;
    }

    if (size > DMA_MAX_BLOCK_SIZE)
    {
        llpNum  = (size + (LLP_BLOCK_SIZE - 1))/LLP_BLOCK_SIZE;

        if (llpNum > LLP_MAX_NUM)
        {
            rk_printf("Dma Ch%d size so large!\n", temp_ch);
            ret = RK_ERROR;
            goto out;
        }
    }

    pstDmaDev->DmaTransSizeBack[temp_ch] = size;
    pstDmaDev->DmaIsrCallBack[temp_ch] = CallBack;
    DmaEnableInt(temp_ch);
    DmaConfig(temp_ch, srcAddr, dstAddr, size, g_dmaPar, pstDmaDev->g_llpListn[temp_ch]);

    ret = RK_SUCCESS;

out:
    //rkos_exit_critical();

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
** Name: DmaDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.18
** Time: 11:01:19
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_COMMON_
COMMON FUN rk_err_t DmaDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < 1; i++)
    {
        if(gpstDmaDevInf == dev)
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
** Name: DmaDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT API rk_err_t DmaDevDelete(uint32 DevID, void * arg)
{
    DMA_DEVICE_CLASS * pstDmaDev = gpstDmaDevInf;


    DmaDevDeInit(gpstDmaDevInf);
    DmaDevHwDeInit(DevID, 0);



    rkos_semaphore_delete(pstDmaDev->osDmaOperReqSem);

    rkos_memory_free(pstDmaDev);

    gpstDmaDevInf = NULL;

    rk_printf("delete dma device");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DmaDevCreate
** Input:void * Args
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT API HDC DmaDev_Create(uint32 DevID, void * arg)
{

    DEVICE_CLASS* pstDev;
    DMA_DEVICE_CLASS * pstDmaDev;

    pstDmaDev =  rkos_memory_malloc(sizeof(DMA_DEVICE_CLASS));
    if (pstDmaDev == NULL)
    {
        return pstDmaDev;
    }

    memset((uint8 *)pstDmaDev, 0, sizeof(DMA_DEVICE_CLASS));

    pstDmaDev->osDmaOperReqSem     = rkos_semaphore_create(1, 1);

    if ((pstDmaDev->osDmaOperReqSem) == 0)
    {

        rkos_semaphore_delete(pstDmaDev->osDmaOperReqSem);

        rkos_memory_free(pstDmaDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstDmaDev;

    pstDev->suspend = DmaDevSuspend;
    pstDev->resume  = DmaDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    DmaDevHwInit(DevID, 0);
    DmaDevInit(pstDmaDev);

    gpstDmaDevInf = pstDmaDev;

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
** Name: DmaDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT FUN rk_err_t DmaDevResume(HDC dev)
{
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;
    if(DmaDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    DmaDevHwInit(pstDmaDev->stDmaDevice.DevID, 0);
    DmaDevInit(pstDmaDev);

    pstDmaDev->stDmaDevice.State = DEV_STATE_WORKING;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DmaDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT FUN rk_err_t DmaDevSuspend(HDC dev, uint32 Level)
{
    DMA_DEVICE_CLASS * pstDmaDev = (DMA_DEVICE_CLASS *)dev;
    if(DmaDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }


    if(Level == DEV_STATE_IDLE1)
    {
        pstDmaDev->stDmaDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstDmaDev->stDmaDevice.State = DEV_SATE_IDLE2;
    }

    DmaDevDeInit(pstDmaDev);
    DmaDevHwDeInit(pstDmaDev->stDmaDevice.DevID, 0);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: DmaDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT FUN rk_err_t DmaDevInit(DMA_DEVICE_CLASS * pstDmaDev)
{
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: DmaDevDeInit
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT FUN rk_err_t DmaDevDeInit(DMA_DEVICE_CLASS * pstDmaDev)
{
    return RK_SUCCESS;
}


#ifdef _DMA_SHELL_
_DRIVER_DMA_DMADEVICE_DATA_
static SHELL_CMD ShellDmaName[] =
{
    "pcb",DmaShellPcb,"list dma device info","dma.pcb [objectid]\n",
    "create",DmaShellCreate,"create a dma device ","dma.create",
#ifdef SHELL_BSP
    "bsp",ShellDmaBsp,"NULL","NULL",
#endif
    "\b",NULL,"NULL","NULL",
};

#ifdef SHELL_BSP
_DRIVER_DMA_DMADEVICE_SHELL_
static SHELL_CMD ShellDmaBspName[] =
{
    "setsrc",ShellDmaBspSetSrc,"NULL","NULL",
    "settarget",ShellDmaBspSetTarget,"NULL","NULL",
    "setblock",ShellDmaBspSetBlock,"NULL","NULL",
    "setsrcinc",ShellDmaBspSetSrcInc,"NULL","NULL",
    "settargetinc",ShellDmaBspSetTargetInc,"NULL","NULL",
    "setchp",ShellDmaBspSetChP,"NULL","NULL",
    "start",ShellDmaBspStart,"NULL","NULL",
    "stop",ShellDmaBspStop,"NULL","NULL",
    "testdma",ShellDmaBspTestDma,"NULL","NULL",
    "testblock",ShellDmaBspTestBlock,"NULL","NULL",,
    "teststartstop",ShellDmaBspTestStartStop,"NULL","NULL",
    "testint",ShellDmaBspTestInt,"NULL","NULL",
    "testllp",ShellDmaBspTestLlp,"NULL","NULL",
    "testmuldma",ShellDmaBspTestMulDma,"NULL","NULL",
    "testchp",ShellDmaBspTestChp,"NULL","NULL",
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
** Name: DmaShell
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL API rk_err_t DmaDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellDmaName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDmaName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                 //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellDmaName[i].CmdDes, pItem);
    if(ShellDmaName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellDmaName[i].ShellCmdParaseFun(dev, pItem);
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
#ifdef SHELL_BSP
/*******************************************************************************
** Name: ShellDmaBsp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 14:05:25
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBsp(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellDmaBspName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDmaBspName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                 //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellDmaBspName[i].CmdDes, pItem);
    if(ShellDmaBspName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellDmaBspName[i].ShellCmdParaseFun(dev, pItem);
    }

    return ret;
}

/*******************************************************************************
** Name: ShellDmaBspTestChp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:59:48
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestChp(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspTestMulDma
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:59:10
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestMulDma(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspTestLlp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:58:30
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestLlp(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspTestInt
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:58:10
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestInt(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspTestStartStop
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:57:03
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestStartStop(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspTestBlock
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:56:35
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestBlock(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}

_DRIVER_DMA_DMADEVICE_SHELL_ uint32 DmaFinish;
_DRIVER_DMA_DMADEVICE_SHELL_ DMA_LLP llpListn[DMA_CHN_MAX][LLP_MAX_NUM];
_DRIVER_DMA_DMADEVICE_SHELL_    eDMA_CHN dma_test_channel;

_DRIVER_DMA_DMADEVICE_SHELL_
void DmaIsr()
{
    DmaFinish = 1;
    DmaDisableInt(dma_test_channel);
}

/*******************************************************************************
** Name: ShellDmaBspTestDma
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:56:02
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspTestDma(HDC dev, uint8 * pstr)
{
    if (StrCmpA(pstr, "0", 1) == 0)
    {
        dma_test_channel = 0;
        rk_print_string("\r\nShellDmaBspTestDma start 0\n");
    }
    else if (StrCmpA(pstr, "1", 1) == 0)
    {
        dma_test_channel = 1;
        rk_print_string("\r\nShellDmaBspTestDma start 1\n");
    }
    else if (StrCmpA(pstr, "2", 1) == 0)
    {
        dma_test_channel = 2;
        rk_print_string("\r\nShellDmaBspTestDma start 2\n");
    }
    else if (StrCmpA(pstr, "3", 1) == 0)
    {
        dma_test_channel = 3;
        rk_print_string("\r\nShellDmaBspTestDma start 3\n");
    }
    else if (StrCmpA(pstr, "4", 1) == 0)
    {
        dma_test_channel = 4;
        rk_print_string("\r\nShellDmaBspTestDma start 4\n");
    }
    else if (StrCmpA(pstr, "5", 1) == 0)
    {
        dma_test_channel = 5;
        rk_print_string("\r\nShellDmaBspTestDma start 5\n");
    }
    //open uart clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 1);

    //open rst uart ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,DmaIsr);

    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);

    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD, 0};

    // test signle
    DmaEnableInt(dma_test_channel);
    DmaFinish = 0;

    *(uint32 *)0x01040000 = 0x55aaaa55;

    *(uint32 *)0x01043FF8 = 0xaa5555aa;

    *(uint32 *)0x03040000 = 0;

    *(uint32 *)0x03043FF8 = 0;


    DmaConfig(dma_test_channel, 0x01040000, 0x03040000, 0xFFF, &DmaCfg, NULL);
    while (!DmaFinish);

    DmaDisableInt(dma_test_channel);

    if ((*(uint32 *)0x03040000 != 0x55aaaa55) || (*(uint32 *)0x03043FF8!= 0xaa5555aa))
    {
        return RK_SUCCESS;
    }
    rk_print_string("\r\ntest signle over\n");


    //test auto llp
    DmaEnableInt(dma_test_channel);
    DmaFinish = 0;

    *(uint32 *)0x01040000 = 0x55aaaa55;

    *(uint32 *)0x01047ffc = 0xaa5555aa;

    *(uint32 *)0x03040000 = 0;

    *(uint32 *)0x03047ffc = 0;



    DmaConfig(dma_test_channel, 0x01040000, 0x03040000, 0x2000, &DmaCfg, llpListn[dma_test_channel]);
    while (!DmaFinish);

    DmaDisableInt(dma_test_channel);

    if ((*(uint32 *)0x03040000 != 0x55aaaa55) || (*(uint32 *)0x03047ffc != 0xaa5555aa))
    {
        return RK_SUCCESS;
    }
    rk_print_string("\r\ntest auto llp over\n");


    //test define llp
    DmaEnableInt(dma_test_channel);
    DmaFinish = 0;

    *(uint32 *)0x01040000 = 0x55aaaa55;
    *(uint32 *)0x010407fc = 0xaa5555aa;

    *(uint32 *)0x03040000 = 0;
    *(uint32 *)0x030407fc = 0;

    *(uint32 *)0x01043000 = 0x55aaaa55;
    *(uint32 *)0x010437fc = 0xaa5555aa;

    *(uint32 *)0x03043000 = 0;
    *(uint32 *)0x030437fc = 0;

    llpListn[0][0].SAR = 0x01040000;
    llpListn[0][0].DAR = 0x03040000;
    llpListn[0][0].SIZE = 0x200;

    llpListn[0][1].SAR = 0x01043000;
    llpListn[0][1].DAR = 0x03043000;
    llpListn[0][1].SIZE = 0x200;

    DmaConfig_for_LLP(dma_test_channel, 0x200, 2,&DmaCfg, llpListn[0]);
    while (!DmaFinish);

    DmaDisableInt(dma_test_channel);

    if ((*(uint32 *)0x03040000 != 0x55aaaa55) || (*(uint32 *)0x030407fc != 0xaa5555aa) ||
            (*(uint32 *)0x03043000 != 0x55aaaa55) || (*(uint32 *)0x030437fc != 0xaa5555aa) )
    {
        return RK_SUCCESS;
    }

    rk_print_string("\r\ntest define llp\n");


    //close dma clk
    ScuClockGateCtr(CLOCK_GATE_DMA, 0);

    //rst dma ip
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntUnregister(INT_ID_DMA);


    rk_print_string("dma test over");


    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspStop
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:55:19
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspStop(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspStart
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:54:57
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspStart(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetChP
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:54:20
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetChP(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetTargetInc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:53:51
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetTargetInc(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetSrcInc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:53:20
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetSrcInc(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetBlock
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:52:57
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetBlock(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetTarget
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:52:34
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetTarget(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ShellDmaBspSetSrc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.11
** Time: 11:51:58
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t ShellDmaBspSetSrc(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}
#endif

/*******************************************************************************
** Name: DmaShellOpen
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t DmaShellCreate(HDC dev,  uint8 * pstr)
{
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("dma device create failure");
    }

    return ret;

}

/*******************************************************************************
** Name: DmaShellPcb
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.24
** Time: 10:58:36
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_SHELL_
SHELL FUN rk_err_t DmaShellPcb(HDC dev,  uint8 * pstr)
{
    uint32 DevID;
    DMA_DEVICE_CLASS * pstDmaDev;
    uint32 i,j;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    DevID = String2Num(pstr);

    pstDmaDev = gpstDmaDevInf;

    if(pstDmaDev == NULL)
    {
        rk_printf("dmadev%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstDmaDevInf[%d]", DevID);
    rk_printf_no_time("    .stDmaDevice");
    rk_printf_no_time("        .next = %08x",pstDmaDev->stDmaDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstDmaDev->stDmaDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstDmaDev->stDmaDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstDmaDev->stDmaDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstDmaDev->stDmaDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstDmaDev->stDmaDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstDmaDev->stDmaDevice.resume);
    rk_printf_no_time("    .osDmaOperReqSem = %08x",pstDmaDev->osDmaOperReqSem);

    for(i = 0; i < DMA_CHN_MAX; i++)
    {
        rk_printf_no_time("    .DmaIsrCallBack[%d] = %08x",i, pstDmaDev->DmaIsrCallBack[i]);
    }

    for(i = 0; i < DMA_CHN_MAX; i++)
    {
        rk_printf_no_time("    .DmaTransSizeBack[%d] = %d",i, pstDmaDev->DmaTransSizeBack[i]);
    }

    for(i = 0; i < DMA_CHN_MAX; i++)
    {
        for(j = 0; j < LLP_MAX_NUM; j++)
        {
            rk_printf_no_time("    .g_llpListn[%d][%d]",i,j);
            rk_printf_no_time("        .SAR = %08x",pstDmaDev->g_llpListn[i][j].SAR);
            rk_printf_no_time("        .DAR = %08x",pstDmaDev->g_llpListn[i][j].DAR);
            rk_printf_no_time("        .LLP = %08x",pstDmaDev->g_llpListn[i][j].LLP);
            rk_printf_no_time("        .CTLL = %08x",pstDmaDev->g_llpListn[i][j].CTLL);
            rk_printf_no_time("        .SIZE = %d",pstDmaDev->g_llpListn[i][j].SIZE);
            rk_printf_no_time("        .DSTAT = %08x",pstDmaDev->g_llpListn[i][j].DSTAT);
        }
    }

    rk_printf_no_time("    .DmaChStatus = %08x",pstDmaDev->DmaChStatus);
    rk_printf_no_time("    .DmaIntStatus = %08x",pstDmaDev->DmaIntStatus);

    return RK_SUCCESS;

}



#endif
#endif

