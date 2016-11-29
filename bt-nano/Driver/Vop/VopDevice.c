/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\Vop\VopDevice.c
* Owner: Benjo.lei
* Date: 2015.10.9
* Time: 13:51:36
* Desc: Vop Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei     2015.10.9    13:51:36      1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_VOP_VOPDEVICE_C__

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
#include "vop.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef  struct _VOP_DEVICE_CLASS
{
    DEVICE_CLASS stDev;
    pSemaphore osVopWriteReqSem;
    pSemaphore osVopWriteSem;
    HDC hDma;
    uint32 DmaCh;

}VOP_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t VopCheckHandler(HDC dev);
rk_err_t VopDevResume(HDC dev);
rk_err_t VopDevSuspend(HDC dev, uint32 Level);
void DMATranferCallBack(uint32 ch);
void VopDevInit(VOP_DEVICE_CLASS * pstVopDev);
rk_err_t VopDevShellMc(HDC dev, uint8 * pstr);
rk_err_t VopDevShellTest(HDC dev, uint8 * pstr);
rk_err_t VopDevShellDel(HDC dev, uint8 * pstr);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static VOP_DEVICE_CLASS * gpstVopDevISR[VOP_DEV_MAX];

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
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: VopDevSetWidthHeight
** Input:HDC dev, uint32 Width, uint32 Height
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.9
** Time: 11:00:28
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_COMMON_
COMMON API rk_err_t VopDevSetWidthHeight(HDC dev, uint32 Width, uint32 Height)
{
    VOP_DEVICE_CLASS * pstVopDev =  (VOP_DEVICE_CLASS *)dev ;
    rkos_semaphore_take(pstVopDev->osVopWriteReqSem, MAX_DELAY);
    VopSetWidthHeight(pstVopDev->stDev.DevID,Width,Height);
    rkos_semaphore_give(pstVopDev->osVopWriteReqSem);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: VopDev_Write
** Input:HDC dev,const void* buffer, uint32 size, uint32 data
** Return:
** Owner:Benjo.lei
** Date: 2015.10.09
** Time: 13:46:51
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_COMMON_
COMMON API rk_size_t VopDev_Write(HDC dev, uint8 * buffer, uint32 size, uint32 data)
{
    uint32 i;
    uint8 *p8;
    uint32 len;
    rk_err_t ret;
    DMA_CFGX DmaCfg;

    VOP_DEVICE_CLASS * pstVopDev = (VOP_DEVICE_CLASS *)dev;

    if(pstVopDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstVopDev->osVopWriteReqSem, MAX_DELAY);

    retry:

    if((size > 128) && (data == 1))
    {
        while(1)
        {
            ret = DmaDev_GetChannel(pstVopDev->hDma);
            if (ret < 0)
            {
                rk_printf("read get channel fail");
                //return (rk_size_t)RK_ERROR;
                rkos_sleep(2);
            }
            else
            {
                break;
            }
        }
        pstVopDev->DmaCh = (uint32)ret;

        DmaCfg.CTLL = DMA_CTLL_VOP_WR;
        DmaCfg.CFGL = DMA_CFGL_VOP_WR;
        DmaCfg.CFGH = DMA_CFGH_VOP_WR;
        DmaCfg.pLLP = 0;

        if(size > (LLP_MAX_NUM * LLP_BLOCK_SIZE))
        {
            len = (LLP_MAX_NUM * LLP_BLOCK_SIZE);
        }
        else
        {
            len = size;
        }

        ret = DmaDev_DmaStart(pstVopDev->hDma, pstVopDev->DmaCh, (uint32)buffer, VopGetDmaReg(pstVopDev->stDev.DevID),
                               len, &DmaCfg,  DMATranferCallBack);

        rkos_semaphore_take(pstVopDev->osVopWriteSem, MAX_DELAY);

        DmaDev_RealseChannel(pstVopDev->hDma, pstVopDev->DmaCh);

        if(size > len)
        {
            buffer += len;
            size -= len;
            goto retry;
        }

    }
    else
    {
        len = size;
        if(len)
        {
            VopSetSplit(pstVopDev->stDev.DevID, VOP_CON_SPLIT_ONE);

            p8 = (uint8 *)buffer;

            if(data)
            {
                for (i=0;i<len;i++)
                {
                    VopSendData(pstVopDev->stDev.DevID,*p8++);
                }
            }
            else
            {
                for (i=0;i<len;i++)
                {
                    VopSendCmd(pstVopDev->stDev.DevID,*p8++);
                }
            }
        }
    }
    rkos_semaphore_give(pstVopDev->osVopWriteReqSem);
    return size;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: VopCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.1
** Time: 9:33:38
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_COMMON_
COMMON FUN rk_err_t VopCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < VOP_DEV_MAX; i++)
    {
        if(gpstVopDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}


/*******************************************************************************
** Name: DMATranferCallBack
** Input:uint32 ch
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.1
** Time: 9:29:15
*******************************************************************************/
void DMATranferCallBack(uint32 ch)
{
    rkos_semaphore_give_fromisr(gpstVopDevISR[0]->osVopWriteSem);
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: VopDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.1
** Time: 9:30:23
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT FUN rk_err_t VopDevResume(HDC dev)
{
    VOP_DEVICE_CLASS * pstVopDev = (VOP_DEVICE_CLASS *)dev;
    if(VopCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    VopDevHwInit(pstVopDev->stDev.DevID,0);
    VopDevInit(pstVopDev);
    RKDev_Resume(pstVopDev->hDma);

    pstVopDev->stDev.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: VopDevSuspend
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.1
** Time: 9:29:15
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT FUN rk_err_t VopDevSuspend(HDC dev, uint32 Level)
{
    VOP_DEVICE_CLASS * pstVopDev = (VOP_DEVICE_CLASS *)dev;
    if(VopCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstVopDev->stDev.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstVopDev->stDev.State = DEV_SATE_IDLE2;
    }

    VopDevHwDeInit(pstVopDev->stDev.DevID,0);

    RKDev_Suspend(pstVopDev->hDma);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: VopDevInit
** Input:VOP_DEVICE_CLASS * pstVopDev
** Return: void
** Owner:aaron.sun
** Date: 2016.3.17
** Time: 13:53:18
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT FUN void VopDevInit(VOP_DEVICE_CLASS * pstVopDev)
{
    VopSetStart(pstVopDev->stDev.DevID, 0);
    VopSetSplit(pstVopDev->stDev.DevID, VOP_CON_SPLIT_ONE);
    VopSetTiming(pstVopDev->stDev.DevID,5,5,5);
    VopSetMode(pstVopDev->stDev.DevID, (0x0<<12) | (0x1<<11) | (0x0<<8) | (0x1<<3) | (0x0<<1));
}

/*******************************************************************************
** Name: VopDev_Create
** Input:uint32 DevID, void *arg
** Return: HDC
** Owner:Benjo.lei
** Date: 2015.10.09
** Time: 14:54:56
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT API HDC VopDev_Create(uint32 DevID, void *arg)
{
    DEVICE_CLASS * pstDev;
    VOP_DEVICE_CLASS * pstVopDev;
    VOP_DEV_ARG * pstVopArg = (VOP_DEV_ARG *)arg;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    pstVopDev = rkos_memory_malloc(sizeof(VOP_DEVICE_CLASS));
    if (pstVopDev == NULL)
    {
        return pstVopDev;
    }

    pstVopDev->osVopWriteReqSem = rkos_semaphore_create(1,1);
    pstVopDev->osVopWriteSem = rkos_semaphore_create(1,0);

    if((pstVopDev->osVopWriteReqSem == 0) || (pstVopDev->osVopWriteSem == 0))
    {
        rkos_semaphore_delete(pstVopDev->osVopWriteReqSem);
        rkos_semaphore_delete(pstVopDev->osVopWriteSem);
        rkos_memory_free(pstVopDev);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstVopDev;

    pstDev->suspend = VopDevSuspend;
    pstDev->resume  = VopDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    pstVopDev->hDma = pstVopArg->hDma;

    pstDev->DevID = DevID;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_VOP_DEV, SEGMENT_OVERLAY_CODE);
#endif
    VopDevHwInit(DevID,0);
    VopDevInit(pstVopDev);
    gpstVopDevISR[DevID] = pstVopDev;
    return pstDev;
}


/*******************************************************************************
** Name: UartDevDelete
** Input:DEVICE_CLASS * dev
** Return: rk_err_t
** Owner:Aaron
** Date: 2014.2.17
** Time: 13:41:18
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT API rk_err_t VopDev_Delete(uint32 DevID, void * arg)
{
    //Check VopDev is not exist...
    if (gpstVopDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    VopDevHwDeInit(DevID,0);

    if(arg != NULL)
    {
        ((VOP_DEV_ARG *)arg)->hDma = gpstVopDevISR[DevID]->hDma;
    }

    rkos_semaphore_delete( gpstVopDevISR[DevID]->osVopWriteReqSem);
    rkos_semaphore_delete( gpstVopDevISR[DevID]->osVopWriteSem);
    rkos_memory_free(gpstVopDevISR[DevID]);

    gpstVopDevISR[DevID] = NULL;



#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_VOP_DEV);
#endif

    return RK_SUCCESS;
}



#ifdef _VOP_DEV_SHELL_
_DRIVER_VOP_VOPDEVICE_SHELL_DATA_
static SHELL_CMD ShellVopName[] =
{
    "create",VopDevShellMc,"create vop device","vop.create",
    "delete",VopDevShellDel,"delete vop device","vop.delete",
    "test",VopDevShellTest,"test vop device","vop.test",
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
** Name: UsbOtgDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_SHELL_
SHELL API rk_err_t VopDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellVopName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellVopName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                      //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellVopName[i].CmdDes, pItem);
    if(ShellVopName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellVopName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: VopDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.9
** Time: 14:37:09
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_SHELL_
SHELL FUN rk_err_t VopDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hVopDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    hVopDev = RKDev_Open(DEV_CLASS_VOP, 0, NOT_CARE);
    if ((hVopDev == NULL) || (hVopDev == (HDC)RK_ERROR) || (hVopDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("VopDev open failure");
        return RK_SUCCESS;
    }

    RKDev_Close(hVopDev);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: VopDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.9
** Time: 14:37:09
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_SHELL_
SHELL FUN rk_err_t VopDevShellMc(HDC dev, uint8 * pstr)
{
    VOP_DEV_ARG stVopDevArg;
    rk_err_t ret;

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
** Name: VopDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.10.8
** Time: 14:37:09
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_SHELL_
SHELL FUN rk_err_t VopDevShellDel(HDC dev, uint8 * pstr)
{
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }
    if (RKDev_Delete(DEV_CLASS_VOP, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("VopDev delete failure");
    }


    return RK_SUCCESS;
}

#endif

#endif
