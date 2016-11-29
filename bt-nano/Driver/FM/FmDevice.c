/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\FM\FmDevice.c
* Owner: cjh
* Date: 2016.3.15
* Time: 11:20:56
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    cjh     2016.3.15     11:20:56   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_FM_FMDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "Hw_fm5807.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define FM_DEV_NUM 2

typedef  struct _FM_DEVICE_CLASS
{
    DEVICE_CLASS stFmDevice;
    pSemaphore osFmOperReqSem;
    pSemaphore osFmOperSem;
    HDC hControlBus;     //i2c
    RK_FM_DRIVER *pFMDriver;
    UINT16 FmArea;

    UINT16  FmFreqMaxVal;   //max frequency.(japan fm station)
    UINT16  FmFreqMinVal;
    UINT16  gbFmTuneRegion;
    //uint32 Channel;

}FM_DEVICE_CLASS;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static FM_DEVICE_CLASS * gpstFmDevISR[FM_DEV_NUM] = {(FM_DEVICE_CLASS *)NULL,(FM_DEVICE_CLASS *)NULL};



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
rk_err_t FmDevSetFrequency(HDC dev, UINT32 FmFreq);
rk_err_t FmDevShellByPassModChange(HDC dev, uint8 * pstr);
rk_err_t FmDevShellNextStation(HDC dev, uint8 * pstr);
rk_err_t FmDevShellPreStation(HDC dev, uint8 * pstr);
rk_err_t FmDevShellSemiAutoSearch(HDC dev, uint8 * pstr);
rk_err_t FmDevShellSetVol(HDC dev, uint8 * pstr);
rk_err_t FmDevShellSetMute(HDC dev, uint8 * pstr);
rk_err_t FmDevShellSwitchStereo(HDC dev, uint8 * pstr);
rk_err_t FmDevShellAutoSearch(HDC dev, uint8 * pstr);
rk_err_t FmDevShellSetArea(HDC dev, uint8 * pstr);
rk_err_t FmDevShellManualSearch(HDC dev, uint8 * pstr);
rk_err_t FmDevShellTest(HDC dev, uint8 * pstr);
rk_err_t FmDevShellDel(HDC dev, uint8 * pstr);
rk_err_t FmDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t FmDevShellPcb(HDC dev, uint8 * pstr);
void FmDevIntIsr1(void);
void FmDevIntIsr0(void);
void FmDevIntIsr(uint32 DevID);
rk_err_t FmDevDeInit(FM_DEVICE_CLASS * pstFmDev);
rk_err_t FmDevInit(FM_DEVICE_CLASS * pstFmDev);
rk_err_t FmDevResume(HDC dev);
rk_err_t FmDevSuspend(HDC dev);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FmDevGetMinOrMaxFreq
** Input:HDC dev, uint32 minMax
** Return: UINT32
** Owner:cjh
** Date: 2016.4.18
** Time: 9:56:30
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API UINT32 FmDevGetMinOrMaxFreq(HDC dev, uint32 minMax)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        printf("MinOrMaxFreq hFmDev ERROR NULL \n");
        return RK_ERROR;
    }

    if(minMax)
    {
        return pstFmDev->FmFreqMaxVal;
    }
    else
    {
        return pstFmDev->FmFreqMinVal;
    }
}
/*******************************************************************************
** Name: FMDevPowerOffDeinit
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.15
** Time: 20:21:59
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FMDevPowerOffDeinit(HDC dev)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        printf("PowerOff hFmDev ERROR NULL \n");
        return RK_ERROR;
    }
    FmDevMuteControl(dev, 1);
    if(pstFmDev->pFMDriver->Tuner_PowerDown != NULL)
    {
        pstFmDev->pFMDriver->Tuner_PowerDown(pstFmDev->hControlBus);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FMDevStepChangeFreq
** Input:UINT16 updownflag, UINT32 FmFreq, UINT16 step
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:40:22
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FMDevStepChangeFreq(HDC dev, int16 updownflag, UINT32 *pFmFreq, UINT16 step)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        printf("ChangeFreq hFmDev ERROR NULL \n");
        return RK_ERROR;
    }

    if((*pFmFreq > pstFmDev->FmFreqMaxVal) || (*pFmFreq < pstFmDev->FmFreqMinVal))
    {
        return FM_InvalidStation;
    }

    if(updownflag == 1)
    {
        *pFmFreq += step;
    }
    else if(updownflag == -1)
    {
        *pFmFreq -= step;
    }

    if(*pFmFreq > pstFmDev->FmFreqMaxVal)
    {
        *pFmFreq = pstFmDev->FmFreqMinVal;
    }

    if(*pFmFreq < pstFmDev->FmFreqMinVal)
    {
        *pFmFreq = pstFmDev->FmFreqMaxVal;
    }
    //rk_printf(".......cur FmFreq = %d.....updownflag:%d...\n",*pFmFreq ,updownflag);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevGetStereoStatus
** Input:HDC dev
** Return: UINT8
** Owner:cjh
** Date: 2016.3.23
** Time: 15:36:22
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API BOOLEAN FmDevGetStereoStatus(HDC dev)
{
    BOOLEAN ret;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);
    if(pstFmDev->pFMDriver->GetStereoStatus != NULL)
    {
        rk_printf("pstFmDev->hControlBus=0x%x\n",pstFmDev->hControlBus);
        ret = pstFmDev->pFMDriver->GetStereoStatus(pstFmDev->hControlBus);
    }

    rkos_semaphore_give(pstFmDev->osFmOperReqSem);
    return ret;
}
/*******************************************************************************
** Name: FmDevVolSet
** Input:UINT8 gain
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:34:29
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevVolSet(HDC dev, UINT8 gain)
{
    rk_err_t ret = RK_ERROR;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);

    if(pstFmDev->pFMDriver->Tuner_SetVolume != NULL)
    {
        //rk_printf("\n..Tuner_SetVolume..!!\n");
        ret = pstFmDev->pFMDriver->Tuner_SetVolume(pstFmDev->hControlBus, gain);
    }

    rkos_semaphore_give(pstFmDev->osFmOperReqSem);
    return ret;
}
/*******************************************************************************
** Name: FmDevMuteControl
** Input:HDC dev, UINT8 active
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:33:22
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevMuteControl(HDC dev, UINT8 active)
{
    rk_err_t ret = RK_ERROR;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);

    if(pstFmDev->pFMDriver->Tuner_MuteControl != NULL)
    {
        ret = pstFmDev->pFMDriver->Tuner_MuteControl(pstFmDev->hControlBus, active);
    }

    rkos_semaphore_give(pstFmDev->osFmOperReqSem);
    return ret;
}
/*******************************************************************************
** Name: FmDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:32:15
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevStart(HDC dev, UINT32 FmFreq, BOOLEAN FmStereo, UINT32 FmArea)
{
    rk_err_t ret = RK_ERROR;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    //rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);

    //pstFmDev->pFMDriver->Tuner_SetFrequency(pstFmDev->hControlBus, FmFreq, 1, FmStereo, FmArea);
    ret = FM_SearchByHand(0);
    //ret = FM_SearchByHand(-1);
    if(pstFmDev->pFMDriver->Tuner_SetFrequency != NULL)
    {
        pstFmDev->pFMDriver->Tuner_SetFrequency(pstFmDev->hControlBus, FmFreq, 1, FmStereo, FmArea);
    }
    //rkos_semaphore_give(pstFmDev->osFmOperReqSem);

    FmDevMuteControl(dev, 0);
    return ret;
}
/*******************************************************************************
** Name: FmDevPause
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:31:29
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevPause(HDC dev)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    FmDevMuteControl(dev, 1);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevStop
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:31:00
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevStop(HDC dev)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);
    if(pstFmDev->pFMDriver->Tuner_PowerDown != NULL)
    {
        pstFmDev->pFMDriver->Tuner_PowerDown(pstFmDev->hControlBus);
    }

    rkos_semaphore_give(pstFmDev->osFmOperReqSem);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevStart
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:30:08
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevPlay(HDC dev)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    FmDevMuteControl(dev, 0);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FmDevSearchByHand
** Input:HDC dev, UINT16 direct, UINT32 *FmFreq
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:26:44
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API UINT16 FmDevSearchByHand(HDC dev, UINT32 FmFreq)
{
    UINT16 ret;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);
    if(pstFmDev->pFMDriver->Tuner_SearchByHand != NULL)
    {

        ret = pstFmDev->pFMDriver->Tuner_SearchByHand(pstFmDev->hControlBus, FmFreq);
    }
    else
    {
        rk_printf("Tuner_SearchByHand == NULL\n");
        ret = RK_ERROR;
    }
/*
    if(pstFmDev->pFMDriver->Tuner_SetFrequency != NULL)
    {

        ret = pstFmDev->pFMDriver->Tuner_SetFrequency(pstFmDev->hControlBus, FmFreq, 0, 0, pstFmDev->FmArea);
    }
    else
    {
        rk_printf("Tuner_SetFrequency == NULL\n");
        ret = RK_ERROR;
    }
*/
    rkos_semaphore_give(pstFmDev->osFmOperReqSem);
    return ret;
}
/*******************************************************************************
** Name: FmDevSetStereo
** Input:HDC dev, UINT8 bStereo
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:25:50
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevSetStereo(HDC dev, UINT8 bStereo)
{
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);

    if(pstFmDev->pFMDriver->Tuner_SetStereo != NULL)
    {
        pstFmDev->pFMDriver->Tuner_SetStereo(pstFmDev->hControlBus, bStereo);
    }

    rkos_semaphore_give(pstFmDev->osFmOperReqSem);
}
/*******************************************************************************
** Name: FmDevSetInitArea
** Input:HDC dev, UINT8 area
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.23
** Time: 15:24:48
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDevSetInitArea(HDC dev, UINT8 area)
{
    rk_err_t ret = RK_ERROR;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    //rkos_semaphore_take(pstFmDev->osFmOperReqSem, MAX_DELAY);

    if(pstFmDev->pFMDriver->Tuner_SetInitArea != NULL)
    {
        ret = pstFmDev->pFMDriver->Tuner_SetInitArea(pstFmDev->hControlBus, area);
        pstFmDev->FmArea = area;
    }

    if(ret == RK_SUCCESS)
    {
        if(pstFmDev->FmArea == RADIO_JAPAN)//SID_RadioListAreaJapan)
        {
            pstFmDev->FmFreqMinVal = 7600;
            pstFmDev->FmFreqMaxVal = 9000;
            pstFmDev->gbFmTuneRegion = 1;
        }
        else    //(gpstFMControlData->FmArea == 2)//SID_RadioListAreaCamp)
        {
            rk_printf("area china 8750...\n");
            pstFmDev->FmFreqMinVal = 8750;//page
            pstFmDev->FmFreqMaxVal = 10800;
            pstFmDev->gbFmTuneRegion = 0;
        }
    }
    //rkos_semaphore_give(pstFmDev->osFmOperReqSem);
    return ret;
}
/*******************************************************************************
** Name: FmDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDev_Write(HDC dev)
{
    //FmDev Write...
    FM_DEVICE_CLASS * pstFmDev =  (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: FmDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON API rk_err_t FmDev_Read(HDC dev)
{
    //FmDev Read...
    FM_DEVICE_CLASS * pstFmDev =  (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        return RK_ERROR;
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
** Name: FmDevSetFrequency
** Input:HDC dev, UINT32 FmFreq
** Return: rk_err_t
** Owner:cjh
** Date: 2016.5.16
** Time: 14:36:16
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN rk_err_t FmDevSetFrequency(HDC dev, UINT32 FmFreq)
{
    UINT16 ret;
    FM_DEVICE_CLASS * pstFmDev = (FM_DEVICE_CLASS *)dev;
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }
    if(pstFmDev->pFMDriver->Tuner_SetFrequency != NULL)
    {
        ret = pstFmDev->pFMDriver->Tuner_SetFrequency(pstFmDev->hControlBus, FmFreq, 0, 0, pstFmDev->FmArea);
    }
    else
    {
        rk_printf("Tuner_SetFrequency == NULL\n");
        ret = RK_ERROR;
    }
    return ret;
}

/*******************************************************************************
** Name: FmDevIntIsr1
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN void FmDevIntIsr1(void)
{
    //Call total int16 service...
    FmDevIntIsr(1);

}
/*******************************************************************************
** Name: FmDevIntIsr0
** Input:void
** Return: void
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN void FmDevIntIsr0(void)
{
    //Call total int16 service...
    FmDevIntIsr(0);

}
/*******************************************************************************
** Name: FmDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN void FmDevIntIsr(uint32 DevID)
{
    uint32 FmDevIntType;


    //Get FmDev Int type...
    //FmDevIntType = GetIntType();
    if (gpstFmDevISR[DevID] != NULL)
    {
        //if (FmDevIntType & INT_TYPE_MAP)
        {
            //write serice code...
        }

        //wirte other int16 service...
    }

}
/*******************************************************************************
** Name: FmDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN rk_err_t FmDevResume(HDC dev)
{
    //FmDev Resume...
    FM_DEVICE_CLASS * pstFmDev =  (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: FmDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:16
*******************************************************************************/
_DRIVER_FM_FMDEVICE_COMMON_
COMMON FUN rk_err_t FmDevSuspend(HDC dev)
{
    //FmDev Suspend...
    FM_DEVICE_CLASS * pstFmDev =  (FM_DEVICE_CLASS *)dev;
    if(pstFmDev == NULL)
    {
        return RK_ERROR;
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
** Name: FmDev_Delete
** Input:uint32 DevID, void * arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_INIT_
INIT API rk_err_t FmDev_Delete(uint32 DevID, void * arg)
{
    FM_DEV_ARG *stFmArg = arg;
    //Check FmDev is not exist...
    if(gpstFmDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }
    stFmArg->hControlBus = gpstFmDevISR[DevID]->hControlBus;
    //FmDev deinit...
    FmDevDeInit(gpstFmDevISR[DevID]);

    //Free FmDev memory...
    rkos_semaphore_delete(gpstFmDevISR[DevID]->osFmOperReqSem);
    rkos_semaphore_delete(gpstFmDevISR[DevID]->osFmOperSem);
    rkos_memory_free(gpstFmDevISR[DevID]);

    //Delete FmDev...
    gpstFmDevISR[DevID] = NULL;

    //Delete FmDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_FM_DEV);
#endif

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:16
*******************************************************************************/
_DRIVER_FM_FMDEVICE_INIT_
INIT API HDC FmDev_Create(uint32 DevID, void * arg)
{
    FM_DEV_ARG * pstFmDevArg;
    DEVICE_CLASS* pstDev;
    FM_DEVICE_CLASS * pstFmDev;
    rk_printf("fmdec_create\n");
    if(arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }
    rk_printf("fmdec_create\n");
    // Create handler...
    pstFmDevArg = (FM_DEV_ARG *)arg;
    pstFmDev =  rkos_memory_malloc(sizeof(FM_DEVICE_CLASS));
    memset(pstFmDev, 0, sizeof(FM_DEVICE_CLASS));
    if (pstFmDev == NULL)
    {
        return NULL;
    }

    //init handler...
    pstFmDev->osFmOperReqSem  = rkos_semaphore_create(1,1);
    pstFmDev->osFmOperSem  = rkos_semaphore_create(1,0);

    if(((pstFmDev->osFmOperReqSem) == 0)
        || ((pstFmDev->osFmOperSem) == 0))
    {
        rkos_semaphore_delete(pstFmDev->osFmOperReqSem);
        rkos_semaphore_delete(pstFmDev->osFmOperSem);
        rkos_memory_free(pstFmDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstFmDev;
    pstDev->suspend = FmDevSuspend;
    pstDev->resume  = FmDevResume;

    //init arg...
    //pstFmDev->hControlBus = pstFmDevArg->Channel;
    pstFmDev->pFMDriver = pstFmDevArg->pFMDriver;
    pstFmDev->FmArea = pstFmDevArg->FmArea;
    pstFmDev->hControlBus = pstFmDevArg->hControlBus;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_FM_DEV, SEGMENT_OVERLAY_ALL);
#endif
    //device init...
    //FmHwInit(DevID, pstFmDev->Channel);
    if(FmDevInit(pstFmDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstFmDev->osFmOperReqSem);
        rkos_semaphore_delete(pstFmDev->osFmOperSem);
        rkos_memory_free(pstFmDev);
        gpstFmDevISR[DevID] = NULL;
        return (HDC) RK_ERROR;
    }
    gpstFmDevISR[DevID] = pstFmDev;
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
** Name: FmDevDeInit
** Input:FM_DEVICE_CLASS * pstFmDev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_INIT_
INIT FUN rk_err_t FmDevDeInit(FM_DEVICE_CLASS * pstFmDev)
{


    return RK_ERROR;
}
/*******************************************************************************
** Name: FmDevInit
** Input:FM_DEVICE_CLASS * pstFmDev
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_INIT_
INIT FUN rk_err_t FmDevInit(FM_DEVICE_CLASS * pstFmDev)
{
    if (pstFmDev == NULL)
    {
        return RK_ERROR;
    }

    printf("FmDevSetInitArea ...\n");
    FmDevSetInitArea(pstFmDev, pstFmDev->FmArea);
    printf("\n... FmDevSetInitArea over\n");

    //FmDevPause(pstFmDev);
    //rk_printf("... FmDevPause\n");
    return RK_SUCCESS;
}


#ifdef _FM_DEV_SHELL_
_DRIVER_FM_FMDEVICE_SHELL_
static SHELL_CMD ShellFmName[] =
{
    "pcb",FmDevShellPcb,"show fm pcb info","fm.pcb[ device number] --- display device pcb info",
    "create",FmDevShellCreate,"create fm device","fm.create --- create the fm device list",
    "delete",FmDevShellDel,"delete fm device","fm.delete --- delete the fm device list ",
    "test",FmDevShellTest,"fm test","fm.test --- open a radio station",
    "cc",FmDevShellManualSearch,"change FM channel","cc.pre:previous channel; cc.next:next channel",
    "setvol",FmDevShellSetVol,"set FM vol","setvol.1:set fm output vol 1; setvol.5:set fm output vol 5; setvol.10:set fm output vol 10; setvol.15:set fm output vol 15",
    "setmute",FmDevShellSetMute,"set FM mute or unmute","setmute 1:set mute; setmute 0:set unmute",
    "setstereo",FmDevShellSwitchStereo,"set FM stereo or mono","setstereo.0:setstereo; setstereo.1:mono ",
    "setarea",FmDevShellSetArea,"set FM area","setarea.japan:Frequency range7600~9000KHZ(china/europe/usa); setarea.china:8750~10800KHZ(china/europe/usa) ",
    "autosearch",FmDevShellAutoSearch,"Auto Search","NULL",
    "semiauto",FmDevShellSemiAutoSearch,"Stop search then play when find the station","NULL",
    "prestation",FmDevShellPreStation,"previous Station in the preset","NULL",
    "nextstation",FmDevShellNextStation,"next Station in the preset","NULL",
    "mod",FmDevShellByPassModChange,"change mode between bypass and fm-line","mod.bypass:fm bypass mode; mod.rec:fm recording mod",
    "\b",NULL,"NULL","NULL",
};

_DRIVER_FM_FMDEVICE_SHELL_
UINT32  FmDevShellCurFmFreq;
_DRIVER_FM_FMDEVICE_SHELL_
uint8  FmDevShellCurStationNum;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FmDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL API rk_err_t FmDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellFmName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellFmName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellFmName[i].CmdDes, pItem);
    if(ShellFmName[i].ShellCmdParaseFun != NULL)
    {
        ShellFmName[i].ShellCmdParaseFun(dev, pItem);
    }

    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FmDevShellByPassModChange
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.20
** Time: 15:30:31
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellByPassModChange(HDC dev, uint8 * pstr)
{
    HDC hRockCodec;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    hRockCodec = RKDev_Open(DEV_CLASS_ROCKCODEC, 0, NOT_CARE);
    if((hRockCodec == NULL) || (hRockCodec == (HDC)RK_ERROR) || (hRockCodec == (HDC)RK_PARA_ERR))
    {
        rk_print_string("hRockCodec open failure");
         return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "bypass", 6) == 0)
    {
        Codec_ExitMode(Codec_Line1ADC);
        RockcodecDev_SetAdcMode(hRockCodec, Codec_Line1in);
    }
    else if(StrCmpA(pstr, "rec", 3) == 0)
    {
        Codec_ExitMode(Codec_Line1in);
        RockcodecDev_SetAdcMode(hRockCodec, Codec_Line1ADC);
    }

    RKDev_Close(hRockCodec);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellNextStation
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.15
** Time: 13:51:00
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellNextStation(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(gSysConfig.RadioConfig.FmSaveNum < 1)
    {
        rk_printf("Without saving the Station\n");
        return RK_SUCCESS;
    }

    if(FmDevShellCurStationNum >= gSysConfig.RadioConfig.FmSaveNum)
    {
        rk_printf("The last Station\n");
        return RK_SUCCESS;
    }

    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }
    FmDevShellCurStationNum++;
    FmDevShellCurFmFreq = gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum - 1];
    FmDevSearchByHand(hFmDev, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum - 1]);
    FmDevStart(hFmDev, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum - 1], 1 ,RADIO_CHINA);
    FmDevMuteControl(hFmDev, 0);
    rk_printf("next CurFmFreq[%d] =%d\n",FmDevShellCurStationNum, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum - 1]);
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellPreStation
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.15
** Time: 13:50:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellPreStation(HDC dev, uint8 * pstr)
{
    HDC hFmDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    if(gSysConfig.RadioConfig.FmSaveNum < 1)
    {
        rk_printf("Without saving the Station\n");
        return RK_SUCCESS;
    }

    if(FmDevShellCurStationNum == 0)
    {
        rk_printf("The first Station\n");
        return RK_SUCCESS;
    }

    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }
    FmDevShellCurStationNum--;

    FmDevShellCurFmFreq = gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum];
    FmDevSearchByHand(hFmDev, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum]);
    FmDevStart(hFmDev, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum], 1 ,RADIO_CHINA);
    FmDevMuteControl(hFmDev, 0);
    rk_printf("pre CurFmFreq[%d] =%d\n",FmDevShellCurStationNum, gSysConfig.RadioConfig.FmFreqArray[FmDevShellCurStationNum]);
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellSemiAutoSearch
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:23:07
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellSemiAutoSearch(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 DevID;
    uint32 FmFreq;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }
    rk_printf("FmFreqMaxVal =%d\n",gpstFmDevISR[0]->FmFreqMaxVal);
    while(1)
    {
        FMDevStepChangeFreq(hFmDev, 1, &FmDevShellCurFmFreq, 20);
        ret = FmDevSearchByHand(hFmDev, FmDevShellCurFmFreq);
        if(FM_FoundStation == ret)
        {
            rk_printf("HAND_AUTOSEARCH  FM_FoundStation\n");
            FmDevStart(hFmDev, FmDevShellCurFmFreq, 1 ,RADIO_CHINA);
            FmDevMuteControl(hFmDev, 0);
            break;
        }
        if(FmDevShellCurFmFreq > gpstFmDevISR[0]->FmFreqMaxVal)
        {
            rk_printf("search end InvalidStation\n");
            break;
        }
    }

    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellSetVol
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:22:19
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellSetVol(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 vol;
    uint32 FmFreq;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    //Get FmDev ID...
    if(StrCmpA(pstr, "5", 1) == 0)
    {
        vol = 5;
    }
    else if(StrCmpA(pstr, "10", 2) == 0)
    {
        vol = 10;
    }
    else if(StrCmpA(pstr, "15", 2) == 0)
    {
        vol = 15;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        vol = 1;
    }
    else
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }
    ret = FmDevVolSet(hFmDev, vol);
    if(ret == RK_SUCCESS)
    {
        rk_printf("Set vol=%d\n",vol);
    }

    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellSetMute
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:21:35
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellSetMute(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 mute;
    uint32 FmFreq;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    //Get FmDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        mute = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        mute = 1;
    }
    else
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }

    ret = FmDevMuteControl(hFmDev, mute);
    if(ret == RK_SUCCESS)
    {
        if(mute)
        {
            rk_print_string("Set mute\n");
        }
        else
        {
            rk_print_string("Set unmute\n");
        }
    }

    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellSwitchStereo
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:20:41
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellSwitchStereo(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 StereoState;
    uint32 FmFreq;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }
    //Get FmDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        StereoState = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        StereoState = 1;
    }
    else
    {
        return RK_SUCCESS;
    }
    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }
    ret = FmDevSetStereo(hFmDev, StereoState);
    if(ret == RK_SUCCESS)
    {
        if(StereoState)
        {
            rk_print_string("Set mono success\n");
        }
        else
        {
            rk_print_string("Set Stereo success\n");
        }
    }
    //close FmDev...
    RKDev_Close(hFmDev);

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellAutoSearch
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:12:00
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellAutoSearch(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 FmFreq;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
        return RK_SUCCESS;
    }
    if(gpstFmDevISR[0]->FmArea == RADIO_JAPAN)//SID_RadioListAreaJapan)
    {
        FmFreq = 7600;
    }
    else    //(gpstFMControlData->FmArea == 2)//SID_RadioListAreaCamp)
    {
        rk_printf("area china 8750...\n");
        FmFreq = 8750;//page
    }
    while(1)
    {
        FMDevStepChangeFreq(hFmDev, 1, &FmFreq, 10);
        ret = FmDevSearchByHand(hFmDev, FmFreq);
        if(FM_FoundStation == ret)
        {
            //call back app
            UINT32 freqpre, freqcur;

            if(gSysConfig.RadioConfig.FmSaveNum >= FREQMAXNUMBLE)
            {
                return RK_ERROR;
            }

            if(gSysConfig.RadioConfig.FmSaveNum >0 )
            {
                freqpre = gSysConfig.RadioConfig.FmFreqArray[gSysConfig.RadioConfig.FmSaveNum-1] / 10;
            }
            else
            {
                freqpre = 0;
            }
            freqcur = (FmFreq) / 10;
            if (freqpre != freqcur)
            {
                gSysConfig.RadioConfig.FmFreqArray[gSysConfig.RadioConfig.FmSaveNum++] = FmFreq;
            }
            printf("\n\n ++++++++++gpstFMControlData->FmSaveNum = %d\n\n",gSysConfig.RadioConfig.FmSaveNum);
        }
        if(FmFreq >= gpstFmDevISR[0]->FmFreqMaxVal)
        {
            rk_printf("AutoSearch end");
            break;
        }
    }
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellSetArea
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:09:13
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellSetArea(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 area;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Get FmDev ID...
    if(StrCmpA(pstr, "japan", 6) == 0)
    {
        area = RADIO_JAPAN;
    }
    else if(StrCmpA(pstr, "china", 6) == 0)
    {
        area = RADIO_CHINA;
    }
    else
    {
        return RK_ERROR;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }

    ret = FmDevSetInitArea(hFmDev, area);
    if(ret == RK_SUCCESS)
    {
        if(area == RADIO_CHINA)
        {
            rk_printf("Set Area RADIO_CHINA");
        }
        else if(area == RADIO_EUROPE)
        {
            rk_printf("Set Area RADIO_EUROPE");
        }
        else if(area == RADIO_JAPAN)
        {
            rk_printf("Set Area RADIO_JAPAN");
        }
        else
        {
            rk_printf("Set Area RADIO_USA");
        }
    }
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: FmDevShellManualSearch
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.4.12
** Time: 17:07:31
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellManualSearch(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 dir;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Get FmDev ID...
    if(StrCmpA(pstr, "pre", 4) == 0)
    {
        dir = 0;
    }
    else if(StrCmpA(pstr, "next", 5) == 0)
    {
        dir = 1;
    }
    else
    {
        return RK_ERROR;
    }
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }

    FMDevStepChangeFreq(hFmDev, dir, &FmDevShellCurFmFreq, 20);
    FmDevSearchByHand(hFmDev, FmDevShellCurFmFreq);
    FmDevStart(hFmDev, FmDevShellCurFmFreq, 1 ,RADIO_CHINA);

    FmDevMuteControl(hFmDev, 0);

    rk_printf("CurFmFreq =%d\n",FmDevShellCurFmFreq);
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: FmDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 FmFreq;
    uint32 i;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }

    //do test....

    FmFreq = 9120;

    FmDevStart(hFmDev, FmFreq, 1 ,RADIO_CHINA);
    FmDevMuteControl(hFmDev, 0);

    rkos_delay(1000);
    for(i=0; i<30; i++ )
    {
        FmFreq += 10;
        rk_printf("next.. FmFreq =%d", FmFreq);
        FmDevSearchByHand(gpstFmDevISR[0], FmFreq);
        FmDevStart(hFmDev, FmDevShellCurFmFreq, 1 ,RADIO_CHINA);
        FmDevMuteControl(hFmDev, 0);
        rkos_delay(2000);
    }
    //close FmDev...
    RKDev_Close(hFmDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: FmDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellDel(HDC dev, uint8 * pstr)
{
    FM_DEV_ARG stFMarg;

    HDC hFmDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Open FmDev...
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);
    if((hFmDev == NULL) || (hFmDev == (HDC)RK_ERROR) || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("FmDev open failure");
         return RK_SUCCESS;
    }

    AudioDevExitType(Codec_Line1in);
    #ifndef _BROAD_LINE_OUT_
    AudioDevExitType(Codec_DACoutHP);
    #else
    AudioDevExitType(Codec_DACoutLINE);
    #endif
    printf("AudioDevExitType...\n");
    #ifdef CODEC_24BIT //24bit
    //RockcodecDev_SetDacDataWidth(hFmDev,ACodec_I2S_DATA_WIDTH24);
    #else
    RockcodecDev_SetDacDataWidth(hFmDev,ACodec_I2S_DATA_WIDTH16);
    #endif
    //RockcodecDev_SetDacRate(hFmDev, I2S_FS_44100Hz);

    printf("RockcodecDev_SetDacRate...\n");

    #ifndef _BROAD_LINE_OUT_
    //RockcodecDev_SetDacMode(hFmDev, Codec_DACoutHP);
    #else
    RockcodecDev_SetDacMode(hFmDev, Codec_DACoutLINE);
    #endif

    FMDevPowerOffDeinit(hFmDev);
    RKDev_Close(hFmDev);
    if(DeviceTask_DeleteDeviceList(DEVICE_LIST_FM, NULL, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("FM device list create fail\n");
         while(1);
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellCreate(HDC dev, uint8 * pstr)
{
    FM_DEV_ARG stFmDevArg;
    rk_err_t ret;
    HDC     hFmDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(DeviceTask_CreateDeviceList(DEVICE_LIST_FM, NULL, SYNC_MODE) != RK_SUCCESS)
    {
         rk_printf("FM device list create fail\n");
         while(1);
    }
    rk_printf("0 DEVICE_LIST_FM over\n");
    hFmDev = RKDev_Open(DEV_CLASS_FM, 0, NOT_CARE);

    if((hFmDev == NULL)
    || (hFmDev == (HDC)RK_ERROR)
    || (hFmDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("hFmDev device open failure");
        rk_printf("0 hFmDev -1\n");
        RKDev_Close(hFmDev);
        while(1);
    }

#if 1
    HDC hI2S;
    I2S_DEVICE_CONFIG_REQ_ARG stI2sDevArg;

    hI2S = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);

    stI2sDevArg.i2smode = I2S_SLAVE_MODE;//;I2S_SLAVE_MODE
    stI2sDevArg.i2sCS = I2S_IN; //I2S_EXT
    stI2sDevArg.BUS_FORMAT = I2S_FORMAT;
    stI2sDevArg.I2S_Bus_mode = I2S_NORMAL_MODE;
    stI2sDevArg.Data_width = I2S_DATA_WIDTH24;

    #ifdef _RECORD_
    gSysConfig.RecordConfig.RecordQuality = 1;
    if (RECORD_QUALITY_HIGH == gSysConfig.RecordConfig.RecordQuality)  //quality record.
    {
        DEBUG("FM_FS = FS_192KHz;");
        stI2sDevArg.I2S_FS = I2S_FS_192KHz;
        stI2sDevArg.Rx_Data_width = I2S_DATA_WIDTH24;
    }
    else
    {
        DEBUG("FM_FS = FS_48KHz;");
        stI2sDevArg.I2S_FS = I2S_FS_48KHz;
        stI2sDevArg.Rx_Data_width = I2S_DATA_WIDTH16;
    }
    #endif

    stI2sDevArg.RX_BUS_FORMAT = I2S_FORMAT;
    stI2sDevArg.Rx_I2S_Bus_mode = I2S_NORMAL_MODE;

    ret = I2sDev_Control(hI2S, I2S_DEVICE_INIT_CMD, &stI2sDevArg);
    if (RK_ERROR == ret)
    {
        rk_printf("I2s set error\n");
    }
    RKDev_Close(hI2S);

    HDC hRockCodec;
    ROCKCODEC_DEV_CONFIG_ARG stRockCodecDevArg;

    hRockCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

    #ifdef _RECORD_
    if (RECORD_QUALITY_HIGH == gSysConfig.RecordConfig.RecordQuality)  //quality record.
    {
        rk_print_string("codec = FS_192KHz;");
        stRockCodecDevArg.DacFs = I2S_FS_192KHz;
        stRockCodecDevArg.DacDataWidth = VDW_RX_WIDTH_16BIT;//VDW_RX_WIDTH_24BIT;
    }
    else
    {
        rk_print_string("codec = FS_48KHz;");
        stRockCodecDevArg.DacFs = I2S_FS_48KHz;
        stRockCodecDevArg.DacDataWidth = VDW_RX_WIDTH_16BIT;
    }
    #endif

#ifndef _BROAD_LINE_OUT_
    stRockCodecDevArg.DacMode  = Codec_DACoutHP;
#else
    stRockCodecDevArg.DacMode  = Codec_DACoutLINE;
#endif
    stRockCodecDevArg.AdcMode = Codec_Line1in;//Codec_Line1in : bypass mode
    rk_printf(" DacMode =%d \n", stRockCodecDevArg.DacMode);

    RockcodecDev_SetDacDataWidth(hRockCodec,stRockCodecDevArg.DacDataWidth);
    RockcodecDev_SetAdcDataWidth(hRockCodec,stRockCodecDevArg.DacDataWidth);
    RockcodecDev_SetDacRate(hRockCodec, stRockCodecDevArg.DacFs);
    RockcodecDev_SetAdcMode(hRockCodec, stRockCodecDevArg.AdcMode);
    RockcodecDev_SetDacMode(hRockCodec, stRockCodecDevArg.DacMode);

    RKDev_Close(hRockCodec);

#endif
    printf("FMControlTask_  FM_Start\n");
    RKDev_Close(hFmDev);

    if(gpstFmDevISR[0]->FmArea == RADIO_JAPAN)//SID_RadioListAreaJapan)
    {
        FmDevShellCurFmFreq = 7600;
    }
    else    //(gpstFMControlData->FmArea == 2)//SID_RadioListAreaCamp)
    {
        rk_printf("are china 8750...\n");
        FmDevShellCurFmFreq = 8750;//page
    }
    rk_printf("CurFmFreq =%d\n",FmDevShellCurFmFreq);
    gSysConfig.RadioConfig.FmSaveNum = 0;
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FmDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.15
** Time: 11:21:17
*******************************************************************************/
_DRIVER_FM_FMDEVICE_SHELL_
SHELL FUN rk_err_t FmDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hFmDev;
    uint32 DevID;
    FM_DEVICE_CLASS * pstFmDev;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    DevID = String2Num(pstr);

    if(DevID > FM_DEV_NUM)
    {
        return RK_ERROR;
    }

    if(RKDeviceFind(DEV_CLASS_FM, DevID) == RK_ERROR)
    {
        rk_printf("FmDev%d in not exist", DevID);
        return RK_SUCCESS;
    }

    pstFmDev = gpstFmDevISR[DevID];

    //Display pcb...
    if(gpstFmDevISR[DevID] != NULL)
    {
        rk_printf_no_time(".gpstFmDevISR[%d]", DevID);
        rk_printf_no_time("    .stFmDevice");
        rk_printf_no_time("        .next = %08x",pstFmDev->stFmDevice.next);
        rk_printf_no_time("        .UseCnt = %d",pstFmDev->stFmDevice.UseCnt);
        rk_printf_no_time("        .SuspendCnt = %d",pstFmDev->stFmDevice.SuspendCnt);
        rk_printf_no_time("        .DevClassID = %d",pstFmDev->stFmDevice.DevClassID);
        rk_printf_no_time("        .DevID = %d",pstFmDev->stFmDevice.DevID);
        rk_printf_no_time("        .suspend = %08x",pstFmDev->stFmDevice.suspend);
        rk_printf_no_time("        .resume = %08x",pstFmDev->stFmDevice.resume);
        rk_printf_no_time("    .osFmOperReqSem = %08x",pstFmDev->osFmOperReqSem);
        rk_printf_no_time("    .osFmOperSem = %08x",pstFmDev->osFmOperSem);
        rk_printf_no_time("    .hControlBus = %08x",pstFmDev->hControlBus);
        rk_printf_no_time("    .pFMDriver");
        rk_printf_no_time("        .Tuner_SetInitArea = %08x",pstFmDev->pFMDriver->Tuner_SetInitArea);
        rk_printf_no_time("        .Tuner_SetFrequency = %d",pstFmDev->pFMDriver->Tuner_SetFrequency);
        rk_printf_no_time("        .Tuner_SetStereo = %08x",pstFmDev->pFMDriver->Tuner_SetStereo);
        rk_printf_no_time("        .Tuner_SetVolume = %d",pstFmDev->pFMDriver->Tuner_SetVolume);
        rk_printf_no_time("        .Tuner_SearchByHand = %08x",pstFmDev->pFMDriver->Tuner_SearchByHand);
        rk_printf_no_time("        .Tuner_PowerDown = %d",pstFmDev->pFMDriver->Tuner_PowerDown);
        rk_printf_no_time("        .Tuner_MuteControl = %08x",pstFmDev->pFMDriver->Tuner_MuteControl);
        rk_printf_no_time("        .GetStereoStatus = %d",pstFmDev->pFMDriver->GetStereoStatus);
        rk_printf_no_time("        .next = %08x",pstFmDev->pFMDriver->next);
        rk_printf_no_time("    .fmArea = %d",pstFmDev->FmArea);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("FmDev ID = %d not exit", DevID);
    }
    return RK_SUCCESS;
}


#endif

#endif
