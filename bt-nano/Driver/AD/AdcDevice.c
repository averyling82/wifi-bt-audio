/*
********************************************************************************************
*
*      Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\AD\AdcDevice.c
* Owner: zhuzhe
* Date: 2014.5.26
* Time: 10:42:35
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    zhuzhe     2014.5.26     10:42:35   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_AD_ADCDEVICE_C__
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_AD_ADCDEVICE_DATA_      _DRIVER_AD_ADCDEVICE_SHELL_
#define _DRIVER_AD_ADCDEVICE_HLP_DATA_ _DRIVER_AD_ADCDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_AD_ADCDEVICE_DATA_      __attribute__((section("driver_ad_adcdevice_data")))
#define _DRIVER_AD_ADCDEVICE_HLP_DATA_      __attribute__((section("driver_ad_adcdevice_hlp_data")))
#else
#error Unknown compiling tools.
#endif

#define ADCQueueLenth 16
#define ADCQueueSize  2
#define ADCMAXCHANNELNUM 5

typedef struct ADCReadItem
{
    //uint16 channel;
    uint16* buf;
    uint32 size;
    uint32 CurSize;
    uint16 clk;
    uint16 multiple;
    //pADCDevRx_indicate RxCallBack;

}ADCReadItem;


typedef struct _ADC_DEVICE_CLASS
{
    DEVICE_CLASS ADCDevice;            //device father class
    HDC          pTimerDevice;

    pSemaphore   ADCReadSem;
    pSemaphore   ADCOperSem;
    pSemaphore   ADCMainClkSem;
    uint16       MainChannel;
    uint16       MainClk;
    ADCReadItem  ChannelItem[ADCMAXCHANNELNUM];
}ADC_DEVICE_CLASS;

static ADC_DEVICE_CLASS *  ADCDevISRHandler;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
rk_err_t ADCDevCheckHandler(HDC dev);
rk_err_t ADCDev_Stop(ADC_DEVICE_CLASS* ADCDevHandler);
rk_err_t ADCDev_Start(ADC_DEVICE_CLASS* ADCDevHandler);
rk_err_t ADCDevShellHelp(HDC dev,  uint8 * pstr);
rk_err_t ADCDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t ADCDevShellTest(HDC dev, uint8 * pstr);
rk_err_t ADCDevShellDel(HDC dev, uint8 * pstr);
rk_err_t ADCDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t ADCDev_Resume(HDC dev);
rk_err_t ADCDev_Suspend(HDC dev, uint32 Level);
rk_err_t ADCDev_Init(ADC_DEVICE_CLASS* ADCDevHandler);
rk_err_t ADCDev_DeInit(ADC_DEVICE_CLASS* ADCDevHandler);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: ADCDev_SetCurSize
** Input:eADC_CHN Channel
** Return: void
** Owner:cjh
** Date: 2015.6.17
** Time: 8:59:48
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON API rk_err_t ADCDev_GetAdcBufData(HDC dev, uint16* buf, uint16 size, uint16 channel)//, uint32 CurSize
{
    ADC_DEVICE_CLASS * ADCDevHandler;
    ADCDevHandler =  (ADC_DEVICE_CLASS *)dev;
    //*buf = ADCDevHandler->ChannelItem[channel].buf;

    if(size > ADCDevHandler->ChannelItem[channel].CurSize)
    {
        size = ADCDevHandler->ChannelItem[channel].CurSize;
    }

    ADCDevHandler->ChannelItem[channel].CurSize -= size;
    //rk_printf("CurSize = 0x%x size=%d\n",ADCDevHandler->ChannelItem[channel].CurSize,size);

    if(size>0)
    {
        memcpy(buf, ADCDevHandler->ChannelItem[channel].buf, size*2);
    }
    if(ADCDevHandler->ChannelItem[channel].CurSize > 0)
    {
        memcpy(ADCDevHandler->ChannelItem[channel].buf, &ADCDevHandler->ChannelItem[channel].buf[size], ADCDevHandler->ChannelItem[channel].CurSize*2);
        memset(&ADCDevHandler->ChannelItem[channel].buf[size], 0, (256-ADCDevHandler->ChannelItem[channel].CurSize)*2);
    }
    //rk_printf("@!!ADCVal=%d copy ADCVal=%d \n",ADCDevHandler->ChannelItem[channel].buf[0],buf[0]);

    return  RK_SUCCESS;
}


/*******************************************************************************
** Name: ADCDev_GetMainClk
** Input:DEVICE_CLASS *dev
** Return: uint16
** Owner:zhuzhe
** Date: 2014.5.29
** Time: 17:12:44
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON API rk_err_t ADCDev_GetMainClk(HDC dev)
{
    ADC_DEVICE_CLASS * ADCDevHandler;
    ADCDevHandler =  (ADC_DEVICE_CLASS *)dev;

    return  (ADCDevHandler->MainClk);
}
/*******************************************************************************
** Name: ADCDev_ClearMainClk
** Input:DEVICE_CLASS *dev,uint16 clk
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.29
** Time: 17:04:40
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON API rk_err_t ADCDev_ClearMainClk(HDC dev)
{
    ADC_DEVICE_CLASS * ADCDevHandler;
    ADCDevHandler =  (ADC_DEVICE_CLASS *)dev;

    rkos_semaphore_give(ADCDevHandler->ADCMainClkSem);

    ADCDevHandler->MainClk = 0;

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ADCDev_SetMainClk
** Input:uint16 clk
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.29
** Time: 16:21:48
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON API rk_err_t ADCDev_SetMainClk(HDC dev,uint16 clk)
{
    ADC_DEVICE_CLASS * ADCDevHandler;
    ADCDevHandler =  (ADC_DEVICE_CLASS *)dev;
    rkos_semaphore_take(ADCDevHandler->ADCMainClkSem,MAX_DELAY);
    if (clk  != NULL)
    {
        ADCDevHandler->MainClk = clk;
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;
    }

}

/*******************************************************************************
** Name: ADCDev_Read
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 16:51:02
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON API rk_err_t ADCDev_Read(HDC dev,uint16 channel,uint16 size, uint16 clk)//,uint16** buf
{
    ADC_DEVICE_CLASS * ADCDevHandler;
    ADCDevHandler =  (ADC_DEVICE_CLASS *)dev;
    ADCReadItem tempReadItem;
    uint32 temp;//i,
    rkos_semaphore_take(ADCDevHandler->ADCOperSem, MAX_DELAY);
    //ADCDev_SetMainClk((DEVICE_CLASS *)ADCDevHandler,clk);
    ADCDevHandler->ChannelItem[channel].size = size;
    ADCDevHandler->ChannelItem[channel].clk = clk;
    ADCDevHandler->MainChannel = channel;

    ADCDev_Start(ADCDevHandler);
    rkos_semaphore_take(ADCDevHandler->ADCReadSem, MAX_DELAY);

    if(ADCDevHandler->ChannelItem[channel].size > 256)
    {
        ADCDevHandler->ChannelItem[channel].size = 256;
    }

    if(ADCDevHandler->ChannelItem[channel].CurSize < ADCDevHandler->ChannelItem[channel].size)
    {
        //(uint32)ADCDevHandler->ChannelItem[channel].buf[ADCDevHandler->ChannelItem[channel].CurSize] = GetAdcData(channel); //GetAdcData(ADCDevHandler->MainChannel);
        ADCDevHandler->ChannelItem[channel].buf[ADCDevHandler->ChannelItem[channel].CurSize] = (uint16)GetAdcData(channel); //GetAdcData(ADCDevHandler->MainChannel);
        //rk_printf("@!!ADCVal=%d ^^^^^^ \n",ADCDevHandler->ChannelItem[channel].buf[ADCDevHandler->ChannelItem[channel].CurSize]);
        ADCDevHandler->ChannelItem[channel].CurSize++;
    }
    else
    {
        temp = GetAdcData(channel);
        ADCDevHandler->ChannelItem[channel].CurSize = ADCDevHandler->ChannelItem[channel].size;
    }
    //ADCDev_Stop((DEVICE_CLASS *)ADCDevHandler);
    rkos_semaphore_give(ADCDevHandler->ADCOperSem);
    return ADCDevHandler->ChannelItem[channel].CurSize;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ADCDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.17
** Time: 14:44:14
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON FUN rk_err_t ADCDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < 1; i++)
    {
        if(ADCDevISRHandler == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: ADCDev_Stop
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 11:38:22
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON FUN rk_err_t ADCDev_Stop(ADC_DEVICE_CLASS * ADCDevHandler)
{
    AdcPowerDown();
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: ADCDev_Start
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 11:37:45
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON FUN rk_err_t ADCDev_Start(ADC_DEVICE_CLASS * ADCDevHandler)
{
    //AdcPowerDown();
    AdcStart(ADCDevHandler->MainChannel);//This bit will reset to 0 by hardware when ADC conversion has started

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ADCDevISR
** Input:void
** Return: void
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 17:12:44
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_COMMON_
COMMON FUN void ADCDevISR(void)
{
    ADC_DEVICE_CLASS* ADCDevHandler = (ADC_DEVICE_CLASS*) ADCDevISRHandler;
    ADCReadItem* pReadItem;
    pFunc CallBack;

    AdcIntClr();

    if (ADCDevHandler != NULL)
    {
        pReadItem = &(ADCDevHandler->ChannelItem[ADCDevHandler->MainChannel]);
//        CallBack = (pFunc) pReadItem->RxCallBack;
//        if (CallBack)
//        {
//            CallBack();
//        }
        rkos_semaphore_give_fromisr(ADCDevHandler->ADCReadSem);
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
** Name: ADCDev_Create
** Input:void
** Return: HDC
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 15:38:01
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT API HDC ADCDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* ADCDevPublicHandler;
    ADC_DEVICE_CLASS* ADCDevHandler;
    ADC_DEV_ARG* pstADCarg = (ADC_DEV_ARG *)arg;
    int channel = 0;
    uint i;

    ADCDevHandler  = (ADC_DEVICE_CLASS *)rkos_memory_malloc(sizeof(ADC_DEVICE_CLASS));
    memset(ADCDevHandler, 0, sizeof(ADC_DEVICE_CLASS));
    memset((void *)(ADCDevHandler->ChannelItem),NULL,sizeof(ADCReadItem)*ADCMAXCHANNELNUM);
    if (ADCDevHandler == NULL)
    {
        goto exit;
    }

    if (pstADCarg != NULL)
    {
        channel = pstADCarg->channel;
        if(pstADCarg->hTimmer != NULL)
        {
            ADCDevHandler->pTimerDevice = pstADCarg->hTimmer;
        }
    }
    ADCDevHandler->ADCMainClkSem = rkos_semaphore_create(1,1);

    ADCDevHandler->ADCReadSem = rkos_semaphore_create(1,0);
    ADCDevHandler->ADCOperSem = rkos_semaphore_create(1,1);
    if (ADCDevHandler->ADCReadSem == NULL)
    {
        rkos_semaphore_delete(ADCDevHandler->ADCReadSem);
        rk_printf("!!ERROR::semaphore_delete\n");
        goto ADCReadSemError;
    }

    for(i=0; i < ADCMAXCHANNELNUM ;i++)
    {
        ADCDevHandler->ChannelItem[i].buf= rkos_memory_malloc(512);
        if(ADCDevHandler->ChannelItem[i].buf == NULL)
        {
            //rk_printf("ChannelItem %d buf NULL",i);
        }
        else
        {
            //rk_printf("ChannelItem %d buf OK",i);
            memset(ADCDevHandler->ChannelItem[i].buf, 0, 512);
        }
    }

    ADCDevPublicHandler = &(ADCDevHandler->ADCDevice);
    ADCDevPublicHandler->DevID = DevID;
    ADCDevPublicHandler->suspend = ADCDev_Suspend;
    ADCDevPublicHandler->resume = ADCDev_Resume;
    ADCDevPublicHandler->SuspendMode = ENABLE_MODE;

    //ADCDevHandler->ChannelItem[channel].size = pstADCarg->size;
    ADCDevHandler->MainChannel = channel;
    AdcDevHwInit (ADCDevPublicHandler->DevID,channel);

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_ADC_DEV, SEGMENT_OVERLAY_CODE);
#endif
    if (ADCDev_Init(ADCDevHandler) != RK_SUCCESS)
    {
        rkos_semaphore_delete(ADCDevHandler->ADCReadSem);

        rkos_memory_free(ADCDevHandler);
        for(i=0; i < ADCMAXCHANNELNUM ;i++)
        {
            rkos_memory_free(ADCDevHandler->ChannelItem[i].buf);
        }
        return (HDC) RK_ERROR;
    }
    //rk_printf("create MainChannel=%d clk=%d\n",ADCDevHandler->MainChannel,ADCDevHandler->ChannelItem[channel].clk);
    ADCDevISRHandler =  ADCDevHandler;
    return ADCDevHandler;

ADCReadSemError:
    rkos_memory_free(ADCDevHandler);
exit:
    return (DEVICE_CLASS*) RK_ERROR;
}


/*******************************************************************************
** Name: ADCDev_Delete
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 17:14:30
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT API rk_err_t ADCDev_Delete(uint32 DevID, void * arg)
{
    uint i;
    ADC_DEV_ARG * pstADCArg = (ADC_DEV_ARG *)arg;
    for(i=0; i < ADCMAXCHANNELNUM ;i++)
    {
        rkos_memory_free(ADCDevISRHandler->ChannelItem[i].buf);
    }
    AdcDevHwDeInit(DevID, ADCDevISRHandler->MainChannel);

    ADCDev_DeInit(ADCDevISRHandler);

    rkos_semaphore_delete(ADCDevISRHandler->ADCReadSem);
    rkos_semaphore_delete(ADCDevISRHandler->ADCMainClkSem);

    rkos_memory_free(ADCDevISRHandler);
    ADCDevISRHandler  = NULL;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_ADC_DEV);
#endif


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
** Name: ADCDevInit
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 17:29:17
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT FUN rk_err_t ADCDev_Init(ADC_DEVICE_CLASS* ADCDevHandler)
{
    ADCDev_SetMainClk(ADCDevHandler,1);//1M
    //ADCDevHandler->MainClk = 1;//1M
    AdcIntEnable();
    AdcPowerUp();//ADC power up and reset

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ADCDev_Resu me
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 16:53:27
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT FUN rk_err_t ADCDev_Resume(HDC dev)
{
     ADC_DEVICE_CLASS * pstADCDev = (ADC_DEVICE_CLASS *)dev;
     if(ADCDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     AdcDevHwInit (pstADCDev->ADCDevice.DevID,pstADCDev->MainChannel);
     ADCDev_Init(pstADCDev);

     pstADCDev->ADCDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}
/*******************************************************************************
** Name: ADCDev_Suspend
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 16:52:13
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT FUN rk_err_t ADCDev_Suspend(HDC dev, uint32 Level)
{
    ADC_DEVICE_CLASS * pstADCDev = (ADC_DEVICE_CLASS *)dev;
    if(ADCDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstADCDev->ADCDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstADCDev->ADCDevice.State = DEV_SATE_IDLE2;
    }

    AdcDevHwDeInit(pstADCDev->ADCDevice.DevID, pstADCDev->MainChannel);
    ADCDev_DeInit(pstADCDev);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: ADCDevDeInit
** Input:void
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.26
** Time: 17:38:05
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
INIT API rk_err_t ADCDev_DeInit(ADC_DEVICE_CLASS * ADCDevHandler)
{
    ADCDev_Stop(ADCDevHandler);
    ADCDev_ClearMainClk(ADCDevHandler);
    AdcIntDisable();

    return RK_SUCCESS;
}


#ifdef _ADC_DEV_SHELL_

_DRIVER_AD_ADCDEVICE_DATA_
static SHELL_CMD ShellADCName[] =
{
    "create",ADCDevShellCreate,"create a adc device","adc.create </key | /mic0 | /fm1 | /fm0 | /bat>",
    "delete",ADCDevShellDel,"delete a adc device","adc.delete",
    "test",ADCDevShellTest,"test adc","adc.test",
    "pcb",ADCDevShellPcb,"list adc device pcb","ad.pcb [device id]",
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
** Name: AdcDev_Shell
** Input:HDC dev
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.18
** Time: 16:50:17
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_SHELL_
rk_err_t ADCDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellADCName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellADCName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }
    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellADCName[i].CmdDes, pItem);
    if(ShellADCName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellADCName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: WRMDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.13
** Time: 14:03:40
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_SHELL_
SHELL FUN rk_err_t ADCDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hADCDevHandle;
    uint32 adcChannel = ADC_CHANEL_KEY;
    uint32 adcVal = 0;
    uint32 temp = 0x01;

    ADC_DEV_ARG stADCArg;
    uint8 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    hADCDevHandle = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
    if (hADCDevHandle == NULL)
    {
        rk_printf("ADC OPEN FRIL");
    }
    else
    {
        rk_printf("ADC%d OPEN SUCCESS",ADC_CHANEL_KEY);
    }

    for(temp=0; temp<200; temp++)
    {
        //Adc->ADC_CTRL = adcChannel    | ADC_START | ADC_POWERUP | ADC_INT_ENBALE;//ADCDevISR
        ADCDev_Read(hADCDevHandle,adcChannel,1,1);//((DEVICE_CLASS *)ADCDevHandle)
        ADCDev_GetAdcBufData(hADCDevHandle,(uint16 *)(&adcVal), 1, ADC_CHANEL_KEY);
       /* while (1)
        {
            DelayMs(10);
            if ((CheckAdcState(adcChannel) & 0x01) == 0)
            {
                break;
            }

            temp = Adc->ADC_STAS;
            if ((temp & 0x01) == 0)
            {
                break;
            }

        } */
        //ADCDev_Stop(pstADCPublicHandler);
        //adcVal = (UINT32)(Adc->ADC_DATA);
        rk_printf("Adc channel %d read value = %d", adcChannel, adcVal);
    }
    RKDev_Close(hADCDevHandle);
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: WRMDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.13
** Time: 14:03:40
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_SHELL_
SHELL FUN rk_err_t ADCDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    if (RKDev_Delete(DEV_CLASS_ADC, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("ADCDev delete failure");
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: WRMDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.13
** Time: 14:03:40
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_SHELL_
SHELL FUN rk_err_t ADCDevShellCreate(HDC dev, uint8 * pstr)
{
    ADC_DEV_ARG stADCArg;
    //TIMER_DEV_ARG stTimerArg;
    //HDC PhTimmer;
    //HDC hADC;
    rk_err_t ret;
    uint8 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    {
        pstr++;
        if (StrCmpA((uint8 *) pstr, "/key", 4) == 0)
        {
            DevID = 0;
            stADCArg.channel = ADC_CHANEL_KEY;
            stADCArg.size = 1;
        }
        else if (StrCmpA((uint8 *) pstr, "/mic0", 5) == 0)
        {
            DevID = 0;
            stADCArg.channel= ADC_MIC;
            stADCArg.size = 512;
#ifdef __DRIVER_TIMER_TIMERDEVICE_C__
            stADCArg.hTimmer = RKDev_Open(DEV_CLASS_TIMER,TIMER0,NOT_CARE);

            if (stADCArg.hTimmer == NULL)
            {
                rk_printf("TIMER0 OPEN fail");
            }
            else
            {
                rk_printf("TIMER0 OPEN SUCCESS");
            }
#endif
        }
        else if (StrCmpA((uint8 *) pstr, "/fm0", 4) == 0)
        {
            DevID = 0;
            stADCArg.channel = ADC_CHANEL_FM0;
            stADCArg.size = 512;
        }
        else if (StrCmpA((uint8 *) pstr, "/fm1", 4) == 0)
        {
            DevID = 0;
            stADCArg.channel = ADC_CHANEL_FM1;
            stADCArg.size = 512;
        }
        else if (StrCmpA((uint8 *) pstr, "/bat", 4) == 0)
        {
            DevID = 0;
            stADCArg.channel = ADC_CHANEL_BATTERY;
            stADCArg.size = 512;
        }
        else
        {
            return RK_ERROR;
        }
    }

    ret = RKDev_Create(DEV_CLASS_ADC,DevID, &stADCArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("ADC CREATE fail");
    }
    else
    {
        rk_printf("ADC CREATE SUCCESS");
    }
#if 0
    ret = ADCDev_Register(ADCDevISRHandler,stADCArg.channel, stADCArg.size, 1, ADCISRCallBack);
    if (ret != RK_SUCCESS)
    {
        printf("Register FRIL\n");
    }
    else
    {
        printf("Register SUCCESS\n");
    }
#endif
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: WRMDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wrm
** Date: 2015.5.13
** Time: 14:03:40
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_SHELL_
SHELL FUN rk_err_t ADCDevShellPcb(HDC dev, uint8 * pstr)
{
    HDC hWRMDev;
    uint32 DevID;


    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    //Get WRMDev ID...
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
#if 0
    if (ADCDevISRHandler[DevID] != NULL)
    {
        rk_print_string("rn");
        rk_printf_no_time("WRMDev ID = %d Pcb list as follow:", DevID);
        rk_printf_no_time("    &stWRMDevice---0x%08x", &gpstWRMDevISR[DevID]->stWRMDevice);
        rk_printf_no_time("    osWRMOperReqSem---0x%08x", gpstWRMDevISR[DevID]->osWRMOperReqSem);
        rk_printf_no_time("    osWRMOperSem---0x%08x", gpstWRMDevISR[DevID]->osWRMOperSem);
        rk_printf_no_time("    osWRMReadReqSem---0x%08x", gpstWRMDevISR[DevID]->osWRMReadReqSem);
        rk_printf_no_time("    osWRMReadSem---0x%08x", gpstWRMDevISR[DevID]->osWRMReadSem);
        rk_printf_no_time("    osWRMWriteReqSem---0x%08x", gpstWRMDevISR[DevID]->osWRMWriteReqSem);
        rk_printf_no_time("    osWRMWriteSem---0x%08x", gpstWRMDevISR[DevID]->osWRMWriteSem);
        //rk_printf_no_time("    usbmode---0x%08x", gpstUsbOtgDevISR[DevID]->usbmode);
        //rk_printf_no_time("    usbspeed---0x%08x", gpstUsbOtgDevISR[DevID]->usbspeed);
    }
    else
    {
        rk_print_string("rn");
        rk_printf_no_time("WRMDev ID = %d not exit", DevID);
    }
#endif
    return RK_SUCCESS;
}
#endif

#endif



