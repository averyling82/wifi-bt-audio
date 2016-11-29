/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Key\KeyDevice.c
* Owner: aaron.sun
* Date: 2015.8.20
* Time: 14:10:42
* Version: 1.0
* Desc: Key Device
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.8.20     14:10:42   1.0
********************************************************************************************
*/
#include <BspConfig.h>
#ifdef __DRIVER_KEY_KEYDEVICE_C__
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
#define KEY_TIME_DOWN               2    //按键按下
#define KEY_TIME_PRESS_START        20   //开始判定为长按
#define KEY_TIME_PRESS_STEP         4    //长按一步的时间范围

typedef struct _KEY_DEVICE_CLASS
{
    DEVICE_CLASS KeyDevice;            //device father class

    pSemaphore KeyReadSem;
    HDC        ADCHandler;
    pTimer     TimerHandler;
    uint8      StartFlag;
    uint32*    buf;

    uint16           ADCVal;
    uint32           KeyCnt;
    uint32           KeyBackup;
    void             (*KeyCallBack)(uint32);
}KEY_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static KEY_DEVICE_CLASS* KeyISRDevHandle;


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
rk_err_t KeyDevCheckHandle(HDC dev);
void KeyDevIsr(void);
rk_err_t KeyDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t KeyDevShellDelete(HDC dev, uint8 * pstr);
void KeyScan(HDC dev);
void KeyScanCallBack(uint32 KeyValue);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: KeyDev_UnRegister
** Input:void
** Return: rk_err_t
** Owner:cjh
** Date: 2015.9.12
** Time: 11:35:19
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON API rk_err_t KeyDev_UnRegister(HDC dev)
{
    KEY_DEVICE_CLASS * KeyDevHandle;
    KeyDevHandle = (KEY_DEVICE_CLASS *)dev;
    //portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if (dev != NULL)
    {
        KeyDevHandle->StartFlag = 0;
        KeyDevHandle->KeyCallBack = NULL;
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;

    }
}

/*******************************************************************************
** Name: KeyDev_Register
** Input: HDC dev, pKeyDevRx_indicate RxCallBack
** Return: rk_err_t
** Owner:cjh
** Date: 2015.9.12
** Time: 10:54:28
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON API rk_err_t KeyDev_Register(HDC dev, pKeyDevRx_indicate RxCallBack)
{
    KEY_DEVICE_CLASS * KeyDevHandle;
    KeyDevHandle = (KEY_DEVICE_CLASS *)dev;

    if (dev != NULL)
    {
        KeyDevHandle->StartFlag = 1;
        KeyDevHandle->KeyCallBack = RxCallBack;
        return RK_SUCCESS;
    }
    else
    {
        return RK_ERROR;

    }
}


/*
*********************************************************************************************************
*                                     KeyDevRead  (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_COMMON_
rk_size_t KeyDevRead(HDC dev)//,uint32* buffer
{
    KEY_DEVICE_CLASS * KeyDevHandle;

    KeyDevHandle = (KEY_DEVICE_CLASS *)dev;
    rkos_semaphore_take(KeyDevHandle->KeyReadSem, MAX_DELAY);//KeyScanCallBack GUI give

    if(KeyDevHandle->KeyDevice.State != DEV_STATE_WORKING)
    {
        KeyScan(dev);
        rkos_semaphore_give(KeyDevHandle->KeyReadSem);
        return RK_SUCCESS;
    }

    ADCDev_Read(KeyDevHandle->ADCHandler,ADC_CHANEL_KEY, 1, 1);//,KeyScanCallBack
    ADCDev_GetAdcBufData(KeyDevHandle->ADCHandler,(uint16 *)&KeyDevHandle->ADCVal, 1, ADC_CHANEL_KEY);
    KeyScan(dev);
    rkos_semaphore_give(KeyDevHandle->KeyReadSem);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: TimerDev_Stop
** Input:HDC dev
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.5.21
** Time: 11:23:17
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
rk_err_t KeyDevStop(HDC dev)
{
    KEY_DEVICE_CLASS * KeyDevHandle;
    KeyDevHandle = (KEY_DEVICE_CLASS *)dev;

    KeyDevHandle->StartFlag = NULL;
    rkos_semaphore_give(KeyDevHandle->KeyReadSem);
    return RK_SUCCESS;
}


/*
*********************************************************************************************************
*                                     KeyDevControl (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
rk_err_t KeyDevSuspend(HDC dev, uint32 Level)
{
    KEY_DEVICE_CLASS * pstKeyDev = (KEY_DEVICE_CLASS *)dev;
    if(KeyDevCheckHandle(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstKeyDev->KeyDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstKeyDev->KeyDevice.State = DEV_SATE_IDLE2;
    }
    rkos_semaphore_take(pstKeyDev->KeyReadSem, MAX_DELAY);
    RKDev_Suspend(pstKeyDev->ADCHandler);
    rkos_semaphore_give(pstKeyDev->KeyReadSem);

    return RK_SUCCESS;
}

/*
*********************************************************************************************************
*                                     KeyDevControl (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
rk_err_t KeyDevResume(HDC dev)
{
    KEY_DEVICE_CLASS * pstKeyDev = (KEY_DEVICE_CLASS *)dev;
    if(KeyDevCheckHandle(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    RKDev_Resume(pstKeyDev->ADCHandler);
    pstKeyDev->KeyDevice.State = DEV_STATE_WORKING;
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
** Name: KeyDevCheckHandle
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 16:58:37
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON FUN rk_err_t KeyDevCheckHandle(HDC dev)
{
    if(KeyISRDevHandle == dev)
    {
        return RK_SUCCESS;
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: KeyDevIsr
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.8.20
** Time: 14:17:17
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON FUN void KeyDevIsr(void)
{
    rkos_start_timer(KeyISRDevHandle->TimerHandler);
    KeyDevRead(KeyISRDevHandle);//GUIKeyCallBack
    //printf("keyVal =%d \n",KeyISRDevHandle->ADCVal);
}

/*
--------------------------------------------------------------------------------
  Function name : uint32 Get_PlayKey_State(void)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
_DRIVER_KEY_KEYDEVICE_COMMON_
uint32 Get_PlayKey_State(void)
{
    uint8 temp;

    temp = Gpio_GetPinLevel(GPIO_CH2,GPIOPortA_Pin5);//PLAYON KEY jjjhhh

    return (temp);
}

/*******************************************************************************
** Name: KeySendMsgToGUI
** Input:void
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 8:46:38
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON API void KeySendMsgToGUI(uint32 KeyValue)
{
#ifdef _USE_GUI_
    GUITask_SendMsg(GUI_EVENT_KEY, KeyValue, NULL, ASYNC_MODE);
#endif
}


/*
*********************************************************************************************************
*                                    rk_err_t GetKey(UINT32 *pKeyVal)
*
* Description:  This function is the to get the key val.
*
* Argument(s) : UINT32 *pKeyVal
*
* Return(s)   : rk_err_t
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_COMMON_
rk_err_t ADCToKey(uint32 ADCVal,uint32 *pKeyVal)
{
    UINT32 KeyTemp = ADCVal;
    UINT32 PlayKeyTemp = 0;
    UINT32 RetVal = -1;

    if (Get_PlayKey_State() != 0)              //PLAY单独检测
    {
        //printf("paly GPIOA2_5 press\n");
        PlayKeyTemp = KEY_VAL_PLAY;
        RetVal  = RK_SUCCESS;
    }

    {
        if ((ADKEY2_MIN <= KeyTemp ) && (KeyTemp < ADKEY2_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY2;
            //printf("adc to keyval = MENU\n");
        }
        else if ((ADKEY3_MIN <= KeyTemp ) && (KeyTemp < ADKEY3_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY3;
            //printf("adc to keyval = UP\n");
        }
        else if ((ADKEY4_MIN <= KeyTemp ) && (KeyTemp < ADKEY4_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY4;
            //printf("adc to keyval = FFD\n");
        }
        else if ((ADKEY5_MIN <= KeyTemp ) && (KeyTemp < ADKEY5_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY5;
            //printf("adc to keyval = FFW\n");
        }
        else if ((ADKEY6_MIN <= KeyTemp ) && (KeyTemp < ADKEY6_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY6;
            //printf("adc to keyval = DOWN\n");
        }
        else if ((ADKEY7_MIN <= KeyTemp ) && (KeyTemp < ADKEY7_MAX))
        {
            *pKeyVal |= KEY_VAL_ADKEY7;
            //printf("adc to keyval = ESC\n");
        }
        RetVal = RK_SUCCESS;
    }

    *pKeyVal |= PlayKeyTemp;

    return RetVal;
}

/*******************************************************************************
** Name: KeySendMsg
** Input:void
** Return: void
** Owner:wrm
** Date: 2016.3.22
** Time: 16:46:38
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_COMMON_
COMMON API void KeySendMsg(uint32 KeyValue)
{
#ifndef _USE_GUI_
#ifdef __APP_MAIN_TASK_MAIN_TASK_C__
    MainTask_KeyCallBack(KeyValue);
#endif
#endif
}

/*
*********************************************************************************************************
*                                      void KeyScanService(void)
*
* Description:  This function is to scan the key.
*
* Argument(s) : void
*
* Return(s)   : void
*
* Note(s)     : none.
*********************************************************************************************************
*/
//static RKDeviceHandler KeyDev;                       //因为Key设备不会被删除，因此可以被一次性打开，减少获取设备句柄的次数
_DRIVER_KEY_KEYDEVICE_COMMON_
void KeyScan(HDC dev)
{
    //uint32 ADCVal = 0;
    uint32 KeyVal = 0;
    uint32 KeyValBack = 0;
    KEY_DEVICE_CLASS* KeyDevHandle = (KEY_DEVICE_CLASS *)KeyISRDevHandle;
    //portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    //will write the adc val
    if (KeyDevHandle != NULL)
    {
        if (KeyDevHandle->StartFlag != NULL)
        {
            ADCToKey(KeyISRDevHandle->ADCVal,&KeyVal);

            KeyValBack = KeyVal;
            //rk_printf("!!KeyVal=0x%x KeyBackup=0x%x adc=%d\n",KeyVal,KeyDevHandle->KeyBackup,KeyISRDevHandle->ADCVal);
            if ((KeyVal != KEY_VAL_NONE) && (KeyVal ==  KeyDevHandle->KeyBackup))  //防止抖动
            {
                KeyDevHandle->KeyCnt++;

                if (KeyDevHandle->KeyCnt == 0)
                {
                    KeyDevHandle->KeyCnt = KEY_TIME_PRESS_START + KEY_TIME_PRESS_STEP;
                }
                else if (KeyDevHandle->KeyCnt == KEY_TIME_DOWN)
                {
                    //key down
                    KeyVal |= KEY_STATUS_DOWN;
                }
                else if (KeyDevHandle->KeyCnt == KEY_TIME_PRESS_START)
                {
                    //press start
                    KeyVal |= KEY_STATUS_PRESS_START;
                    goto BACK;
                }
                else if (KeyDevHandle->KeyCnt > KEY_TIME_PRESS_START)
                {
                    //press
                    if (((KeyDevHandle->KeyCnt - KEY_TIME_PRESS_START) % KEY_TIME_PRESS_STEP) == 0)
                    {
                        KeyVal |= KEY_STATUS_PRESS;
                        goto BACK;
                    }
                }
                //ADCDev_Read(KeyDevHandle->ADCHandler,ADC_CHANEL_KEY,1,1*1000*1000, (uint16 *)(&KeyDevHandle->ADCVal),KeyScanCallBack);
            }
            else
            {
                if (KeyDevHandle->KeyCnt >= KEY_TIME_PRESS_START)
                {
                    KeyVal = (KeyDevHandle->KeyBackup | KEY_STATUS_LONG_UP);
                    KeyDevHandle->KeyCnt = 0;
                    goto BACK;
                }
                else if (KeyDevHandle->KeyCnt >= KEY_TIME_DOWN)
                {
                    KeyVal = (KeyDevHandle->KeyBackup | KEY_STATUS_SHORT_UP);
                    KeyDevHandle->KeyCnt = 0;
                    goto BACK;
                }
                else
                {
                    KeyDevHandle->KeyCnt = 0;
                    //printf("$$$ disturb gan rao$$$$\n");
                    //ADCDev_Read(KeyDevHandle->ADCHandler,ADC_CHANEL_KEY,1,1, (uint16 *)(&KeyDevHandle->ADCVal),KeyScanCallBack);
                }
            }

            KeyDevHandle->KeyBackup = KeyValBack;

        }
    }
    return;

BACK:

    if(CheckSystemIdle() == RK_SUCCESS)
    {
        ClearSytemIdle();
    }
    else
    {
        ClearSytemIdle();
        if(KeyDevHandle->KeyCallBack != NULL)
        {
            //printf("&^*%$#@@2KeyCallBack KEY_STATUS_SHORT_UP KeyVal = %d\n",KeyVal);
            KeyDevHandle->KeyCallBack(KeyVal);
        }
    }

}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*
*********************************************************************************************************
*                                     KeyDevInit  (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
rk_err_t KeyDevInit(HDC dev)
{
    rk_err_t ret;
    KEY_DEVICE_CLASS* KeyDevHandler = (KEY_DEVICE_CLASS*)dev;

    if (NULL == KeyDevHandler )
        return RK_PARA_ERR;

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin5,GPIO_IN);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin5,ENABLE);

    KeyDevHandler->TimerHandler = rkos_create_timer(2, 2, NULL, KeyDevIsr);
    rkos_start_timer(KeyDevHandler->TimerHandler);

    return RK_SUCCESS;
}

/*
*********************************************************************************************************
*                                     KeyDevDeInit (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
rk_err_t KeyDevDeInit(HDC dev)
{
    KEY_DEVICE_CLASS* KeyDevHandler = (KEY_DEVICE_CLASS*)dev;
    rkos_delete_timer(KeyDevHandler->TimerHandler);
    rk_printf("key device deinit...");
    return RK_SUCCESS;
}

/*
*********************************************************************************************************
*                                     KeyCreate (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
HDC KeyDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS*        KeyDevPublicHandler = NULL;
    KEY_DEVICE_CLASS*      KeyDevHandler;
    uint32 i = 0;

    KeyDevHandler = (KEY_DEVICE_CLASS *)rkos_memory_malloc(sizeof(KEY_DEVICE_CLASS));
    if (KeyDevHandler == NULL)
    {
        goto exit;
    }
    memset(KeyDevHandler, 0, sizeof(KEY_DEVICE_CLASS));

    KeyDevHandler->KeyReadSem = rkos_semaphore_create(1,1);
    if (KeyDevHandler->KeyReadSem == NULL)
    {
        goto KeyReadSem_err;
    }

    if(arg != NULL)
    {
        KeyDevHandler->ADCHandler =((KEY_DEV_ARG *)arg)->ADCHandler;
        //rk_printf("ADCHandler =%x\n",KeyDevHandler->ADCHandler);

    }
    else
    {
        goto KeyArg_err;
    }

    KeyDevPublicHandler = (DEVICE_CLASS*)KeyDevHandler;
    KeyDevPublicHandler->SuspendMode = ENABLE_MODE;
    KeyDevPublicHandler->suspend = KeyDevSuspend;
    KeyDevPublicHandler->resume = KeyDevResume;
    KeyDevPublicHandler->Idle1EventTime = 10 * PM_TIME;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_KEY_DEV, SEGMENT_OVERLAY_CODE);
#endif

    KeyISRDevHandle = KeyDevHandler;

    if (KeyDevInit(KeyDevPublicHandler) != RK_SUCCESS)
    {
        rkos_semaphore_delete(KeyDevHandler->KeyReadSem);

        rkos_memory_free(KeyDevHandler);

        printf("init TimmerDev fail");
        return (HDC) RK_ERROR;
    }
    return KeyDevPublicHandler;


KeyArg_err:
    rkos_semaphore_delete(KeyDevHandler->KeyReadSem );
KeyReadSem_err:
    rkos_memory_free(KeyDevHandler);
exit:
    rkos_exit_critical();

    return (DEVICE_CLASS*)RK_ERROR;
}


/*
*********************************************************************************************************
*                                     KeyDevDeInit (void *p_arg)
*
* Description:  This function is the Task to Start.
*
* Argument(s) : void *p_arg
*
* Return(s)   : none
*
* Note(s)     : none.
*********************************************************************************************************
*/
_DRIVER_KEY_KEYDEVICE_INIT_
rk_err_t KeyDev_Delete(uint32 DevID, void * arg)
{
    KEY_DEV_ARG * pstKeyArg = (KEY_DEV_ARG *)arg;
    //KEY_DEVICE_CLASS * KeyDevHandler;
    KeyDevDeInit(KeyISRDevHandle);
    KeyDev_UnRegister(KeyISRDevHandle);
    ADCDev_ClearMainClk(KeyISRDevHandle->ADCHandler);
    rkos_semaphore_delete(KeyISRDevHandle->KeyReadSem );

    if(KeyISRDevHandle->ADCHandler != NULL)
    {
        pstKeyArg->ADCHandler = KeyISRDevHandle->ADCHandler;
    }

    rkos_memory_free(KeyISRDevHandle);
    KeyISRDevHandle = NULL;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_KEY_DEV);
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


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#ifdef _KEY_DEV_SHELL_
_DRIVER_KEY_KEYDEVICE_SHELL_DATA_
static SHELL_CMD ShellKeyName[] =
{
    "pcb",NULL,"list key device pcb inf","key.pcb [key device object id]",
    "create",KeyDevShellCreate,"create a key device","key.create",
    "delete",KeyDevShellDelete,"delete a key device","key.delete",
    "\b",NULL,"NULL","NULL",
};
/*******************************************************************************
** Name: KeyDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.24
** Time: 15:07:41
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_SHELL_
SHELL API rk_err_t KeyDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellKeyName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellKeyName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellKeyName[i].CmdDes, pItem);
    if(ShellKeyName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellKeyName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}


/*******************************************************************************
** Name: KeyScanCallBack
** Input:void
** Return: void
** Owner:cjh
** Date: 2015.6.24
** Time: 8:46:38
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_SHELL_
SHELL API void KeyScanCallBack(uint32 KeyValue)
{
    switch(KeyValue)
    {
        case KEY_VAL_HOLD_PRESS_START:
            rk_printf("!MENU+PLAY LONG PRESS START\n");
            break;
        case KEY_VAL_HOLD_SHORT_UP:
            rk_printf("!MENU+PLAY SHORT UP\n");
            break;
        case KEY_VAL_HOLD_LONG_UP:
            rk_printf("!MENU+PLAY LONG UP\n");
            break;
        case KEY_VAL_PLAY_LONG_UP:
            rk_printf("!PLAY LONG UP\n");
            break;
        case KEY_VAL_PLAY_SHORT_UP:
            rk_printf("!PLAY SHORT UP\n");
            break;
        case KEY_VAL_UP_SHORT_UP:
            rk_printf("!UP SHORT UP\n");
            break;
        case KEY_VAL_DOWN_SHORT_UP:
            rk_printf("!DOWN SHORT UP\n");
            break;
        case KEY_VAL_ESC_SHORT_UP:
            rk_printf("!ESC SHORT UP\n");
            break;
        case KEY_VAL_MENU_SHORT_UP:
            rk_printf("!MENU SHORT UP\n");
            break;
        case KEY_VAL_FFW_SHORT_UP:
            rk_printf("!FFW SHORT UP\n");
            break;
        case KEY_VAL_FFD_SHORT_UP:
            rk_printf("!FFD SHORT UP\n");
            break;
        case KEY_VAL_UP_LONG_UP:
            rk_printf("!UP LONG UP\n");
            break;
        case KEY_VAL_DOWN_LONG_UP:
            rk_printf("!DOWN LONG UP\n");
            break;
        case KEY_VAL_ESC_LONG_UP:
            rk_printf("!ESC LONG UP\n");
            break;
        case KEY_VAL_MENU_LONG_UP:
            rk_printf("!MENU LONG UP\n");
            break;
        case KEY_VAL_FFW_LONG_UP:
            rk_printf("!FFW LONG UP\n");
            break;
        case KEY_VAL_FFD_LONG_UP:
            rk_printf("!FFD LONG UP\n");
            break;
        case KEY_VAL_PLAY_PRESS_START:
            rk_printf("!PLAY LONG PRESS START\n");
            break;
        case KEY_VAL_UP_PRESS_START:
            rk_printf("!UP LONG PRESS START\n");
            break;
        case KEY_VAL_DOWN_PRESS_START:
            rk_printf("!DOWN LONG PRESS START\n");
            break;
        case KEY_VAL_ESC_PRESS_START:
            rk_printf("!ESC LONG PRESS START\n");
            break;
        case KEY_VAL_MENU_PRESS_START:
            rk_printf("!MENU LONG PRESS START\n");
            break;
        case KEY_VAL_FFW_PRESS_START:
            rk_printf("!FFW LONG PRESS START\n");
            break;
        case KEY_VAL_FFD_PRESS_START:
            rk_printf("!FFD LONG PRESS START\n");
            break;

            // other key val will be writen later
        default :
            break;
    }
}

/*******************************************************************************
** Name: KeyDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.24
** Time: 15:07:41
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_SHELL_
SHELL FUN rk_err_t KeyDevShellCreate(HDC dev, uint8 * pstr)
{
    HDC hKey;
    KEY_DEV_ARG stKeyArg;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    #ifdef __DRIVER_AD_ADCDEVICE_C__
    stKeyArg.ADCHandler = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
    if (stKeyArg.ADCHandler == NULL)
    {
        printf("ADC OPEN FRIL\n");
        return RK_SUCCESS;
    }
    else
    {
        printf("ADC OPEN SUCCESS\n");
    }
    #endif

    //stKeyArg->ADCHandler = gpstTimerDevISRHandler[0]->ADCHandle

    ret = RKDev_Create(DEV_CLASS_KEY, 0, &stKeyArg);//&stKeyArg
    if (ret != RK_SUCCESS)
    {
        printf("KEY CREATE FRIL\n");
        return RK_SUCCESS;
    }
    else
    {
        printf("KEY CREATE SUCCESS\n");
    }

    hKey = RKDev_Open(DEV_CLASS_KEY, 0, NOT_CARE);
    if (hKey == NULL)
    {
        printf("key OPEN FRIL\n");
        return RK_SUCCESS;
    }
    else
    {
        printf("key OPEN SUCCESS\n");
    }

    {
        pstr++;
        if (StrCmpA((uint8 *) pstr, "/key/test", 9) == 0)
        {
            ret = KeyDev_Register(hKey, KeyScanCallBack);
            if (ret != RK_SUCCESS)
            {
                printf("GUIKEY Register FRIL\n");
                return RK_SUCCESS;

            }
            else
            {
                printf("GUIKEY Register SUCCESS\n");
            }
        }
        else if (StrCmpA((uint8 *) pstr, "/key/gui", 8) == 0)
        {
            //ret = KeyDev_Register(hKey, GUITask_SendMsg);
            ret = KeyDev_Register(hKey, KeySendMsgToGUI);
            if (ret != RK_SUCCESS)
            {
                printf("GUIKEY Register FRIL\n");
                return RK_SUCCESS;
            }
            else
            {
                printf("GUIKEY Register SUCCESS\n");
            }
        }

    }
    RKDev_Close(stKeyArg.ADCHandler);
    RKDev_Close(hKey);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: KeyDevShellDelete
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.24
** Time: 15:07:41
*******************************************************************************/
_DRIVER_KEY_KEYDEVICE_SHELL_
SHELL FUN rk_err_t KeyDevShellDelete(HDC dev, uint8 * pstr)
{
    HDC hKey;
    KEY_DEV_ARG stKeyArg;
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*(pstr - 1) == '.')
    {
        return RK_ERROR;
    }

    stKeyArg.ADCHandler = NULL;
    if (RKDev_Delete(DEV_CLASS_KEY, 0, &stKeyArg) != RK_SUCCESS)
    {
        rk_printf("key RKDev_Delete ng\n");
        return RK_SUCCESS;
    }

    if (stKeyArg.ADCHandler != NULL)
    {
        printf("stKeyArg.ADCHandler != NULL\n");
        if (RKDev_Close(stKeyArg.ADCHandler) != RK_SUCCESS)
        {
            rk_printf("adc close failure\n");
            return RK_SUCCESS;
        }
    }

    printf("adc RKDev_Delete over\n");

    return RK_SUCCESS;
}

#endif
#endif


