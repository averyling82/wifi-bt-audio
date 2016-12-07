/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Bsp\EVK_V2.0\BSP.c
* Owner: aaron.sun
* Date: 2015.5.14
* Time: 15:23:47
* Version: 1.0
* Desc: EVK_V2.0 Bsp Init
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.5.14     15:23:47   1.0
********************************************************************************************
*/

#define __BSP_EVK_V2_0_BSP_C__
#include "BspConfig.h"
#ifdef __BSP_EVK_V2_0_BSP_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include <stdarg.h>
#include "FwUpdate.h"
#include "wdt.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define DEBUG_TIME_LEN  8

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static uint32  SysTickCntSave;
static uint32  SysTickSave;
uint32 VbusStatus;
int    VbusTimeCount=0;

#ifdef _LOG_DEBUG_
uint32 BcoreRequestDebug;
uint32 BcoreDebug;
uint8 * pBBDebugBuf;
#endif

#ifndef _USE_GUI_
#ifdef NOSCREEN_USE_LED //led display
int8   redLed_state = 0;
int8   greenled_state = 0;
#endif
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
volatile uint32 SysTickCounter;                            //在SYSTICK中进行增加
uint32 SysTickCounterSave;
uint32 SysTickCheck;
uint32 MspSave;
void * UartHDC;
void * FileSysHDC;
void * hLcd;
void * hKey;
void * hWdt;
void (*pIdleFun)(void);
void (*pIdleFunSave)(void);

//USB LUN
#ifdef _USB_
uint8 gLun[USBMSC_LUN] = {2,0,0,0,0,0,0,0,0,0}; //gLun[0] 默认等于2，若要隐藏需要改掉默认
#endif


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
void CpuIdle(void);
void GpioInit(void);
void CPUInit(void);
void SysTickInit(void);
void SysVariableInit(void);
void SysInterruptInit(void);
void fputc_hook(char ch);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: PnPSever
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.4.27
** Time: 18:14:15
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void PnPSever(void)
{
	//rk_printf("\nenter PnPSever...\n");
#ifndef _USE_GUI_
#ifndef _ENABLE_WIFI_BLUETOOTH
    #ifdef _BLUETOOTH_
    {
        #ifdef NOSCREEN_USE_LED //led display
        {
            if ((gSysConfig.BtOpened == 1)&&(gSysConfig.BtControl == 1))
            {
                MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
            }
            if ((gSysConfig.BtControl == 1)&&(gSysConfig.BtOpened != 1))
            {
                redLed_state++;
                if ((redLed_state == 1)||(redLed_state == 2))
                {
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }
                if ((redLed_state == 3)||(redLed_state == 4))
                {
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                    if (redLed_state == 4)
                    {
                        redLed_state = 0;
                    }
                }
            }
        }
        #endif //NOSCREEN_USE_LED end
    }
    #endif //_BLUETOOTH_ end

    #ifdef _WIFI_
    {
        #ifdef NOSCREEN_USE_LED //led display
        {
            if (MainTask_GetStatus(MAINTASK_WIFICONFIG)==1)
            {
                greenled_state++;
                if ((greenled_state == 1)||(greenled_state == 2))
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);

                }
                if ((greenled_state == 3)||(greenled_state == 4))
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                    if (greenled_state == 4)
                    {
                        greenled_state = 0;
                    }
                }
            }
            else
            {
                if ((MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER) != 1)
                    &&(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER) != 1)
                    &&(MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER) != 1))
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }
                if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER_START) == 1)
                {
                    greenled_state++;
                    if ((greenled_state == 1)||(greenled_state == 2))
                    {
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);

                    }
                    if ((greenled_state == 3)||(greenled_state == 4))
                    {
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                        if (greenled_state == 4)
                        {
                            greenled_state = 0;
                        }
                    }
                }
                if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER) == 1)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                }

                if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER_START) == 1)
                {
                    greenled_state++;
                    if ((greenled_state == 1)||(greenled_state == 2))
                    {
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);

                    }
                    if ((greenled_state == 3)||(greenled_state == 4))
                    {
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                        if (greenled_state == 4)
                        {
                            greenled_state = 0;
                        }
                    }
                }
                if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER) == 1)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                }

                if (MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER) == 1)
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }
            }
        }
        #endif //NOSCREEN_USE_LED end
    }
    #endif //_WIFI_ end
#else//#ifndef _ENABLE_WIFI_BLUETOOTH
   {
        #ifdef NOSCREEN_USE_LED //led display
        {
            if(MainTask_GetStatus(MAINTASK_SYS_UPDATE_FW)==1)//updating fw----LED1:FLASH; LED2:OFF
            {
				MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);//led off
					
				redLed_state++;
				if ((redLed_state == 1)||(redLed_state == 3))//led flash
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
				}
				if ((redLed_state == 2)||(redLed_state == 4))
				{
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
					if (redLed_state == 4)
					{
						redLed_state = 0;
					}
				}
			}
            else if(MainTask_GetStatus(MAINTASK_WIFICONFIG)==1)
            {	
            	//rk_printf(">>>>>>>1111111\n");
                greenled_state++;
                if ((greenled_state == 1)||(greenled_state == 3))
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);

                }
                if ((greenled_state == 2)||(greenled_state == 4))
                {
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                    if (greenled_state == 4)
                    {
                        greenled_state = 0;
                    }
                }
            }
            else
            {	//rk_printf(">>>>>>>222222222\n");
				if ((gSysConfig.BtOpened == 1)&&(gSysConfig.BtControl == 1) ||
					(MainTask_GetStatus(MAINTASK_APP_BT_PLAYER) == 1))//BT MODE----LED1:OFF; LED2:ON
				{//rk_printf(">>>>>>>3333333344444\n");
					MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
					MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
				}
				else if ((MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER) != 1)
                    &&(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER) != 1)
                    &&(MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER) != 1)
                    &&(MainTask_GetStatus(MAINTASK_APP_BT_PLAYER) != 1))//NO PLAYER or switching Player----LED1:ON; LED2:FLASH
                {
                	//rk_printf(">>>>>>>444444444444\n");
                    greenled_state++;
                    if ((greenled_state == 1)||(greenled_state == 3))
                    {
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);

                    }
                    if ((greenled_state == 2)||(greenled_state == 4))
                    {	
                        MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                        MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                        if (greenled_state == 4)
                        {
                            greenled_state = 0;
                        }
                    }
                }
                else if (MainTask_GetStatus(MAINTASK_APP_DLNA_PLAYER) == 1 ||
					(MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER) == 1)/**/)//DLNA/AIRPLAY MODE----LED1:ON; LED2:ON
                {	//rk_printf(">>>>>>>5555555555\n");
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_ON);
                }
                /*else if (MainTask_GetStatus(MAINTASK_APP_XXX_PLAYER) == 1)//AIRPLAY MODE----LED1:OFF; LED2:OFF
                {	//rk_printf(">>>>>>>5555555555\n");
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_OFF);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }*/
				else if (MainTask_GetStatus(MAINTASK_APP_LOCAL_PLAYER) == 1)//MP3/本地播放 MODE----LED1:ON; LED2:OFF
                {	//rk_printf(">>>>>>>6666666666\n");
                    MainTask_SetLED (MAINTASK_LED1,MAINTASK_LED_ON);
                    MainTask_SetLED (MAINTASK_LED2,MAINTASK_LED_OFF);
                }

				/*LOW POWER and CHARGING led state implemented in Battery_GetLevel*/
            }//if (MainTask_GetStatus(MAINTASK_WIFICONFIG)==1)
        }
        #endif //NOSCREEN_USE_LED end
    }
#endif//_ENABLE_WIFI_BLUETOOTH
#endif //No _USE_GUI_ end

    if(VbusStatus == 0)
    {
        if (Grf_CheckVbus()) //USB VBUS插拔缓冲1s
        {
            VbusTimeCount++;
            if (VbusTimeCount>2)
                VbusTimeCount = 2;
        }
        else
        {
            VbusTimeCount = 0;
        }


        if(Grf_CheckVbus()&&(VbusTimeCount>=2))
        {
            printf ("\n-----PnPSever connect -------\n");
            if(RKTaskFind(TASK_ID_MAIN, 0))
            {
                MainTask_SysEventCallBack(MAINTASK_USBCREATE,NULL);
            }
            VbusStatus = 1;
            ClearSytemIdle();
            #ifndef _USB_
            ChargeEnable();
            #endif
        }
    }
    else if(VbusStatus == 1)
    {

        if(Grf_CheckVbus() == 0) //USB VBUS插拔缓冲2s
        {
            VbusTimeCount++;
            if (VbusTimeCount>2)
                VbusTimeCount = 2;
        }
        else
        {
            VbusTimeCount = 0;
        }

        if((Grf_CheckVbus() == 0)&&(VbusTimeCount>=2))
        {
            printf ("\n-----PnPSever connect duankai-------\n");
            if(MainTask_GetStatus(MAINTASK_APP_USB_OK) == 1)
            {
                MainTask_SysEventCallBack(MAINTASK_USBDELETE,NULL);
            }
            else
            {
                ChargeDisable();
            }

            VbusStatus = 0;
            ClearSytemIdle();
        }
    }
}

#ifdef _RADIO_
/*******************************************************************************
** Name: DeleteDeviceListFm
** Input:uint32 * list, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 11:13:24
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t DeleteDeviceListFm(uint32 * list, void *arg)
{
    FM_DEV_ARG stFmDevArg;

    if(RKDev_Delete(DEV_CLASS_FM, 0, &stFmDevArg) != RK_SUCCESS)
    {
        printf("FMDev delete failureDevID=%d\n");
        return RK_ERROR;
    }
    if (stFmDevArg.hControlBus != NULL)
    {
        if (RKDev_Close(stFmDevArg.hControlBus) != RK_SUCCESS)
        {
            rk_printf("hControlBus close failure\n");
            return RK_ERROR;
        }
    }
    if (RKDev_Delete(DEV_CLASS_I2C, I2C_DEV1, NULL) != RK_SUCCESS)
    {
        printf("i2c delete failure\n");
        return RK_ERROR;
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: rk_err_t CreateDeviceListFm
** Input:uint32 * list, void *arg
** Return: rk_err_t
** Owner:cjh
** Date: 2016.3.25
** Time: 9:44:39
*******************************************************************************/
_APP_FM_FMCONTROLTASK_COMMON_
COMMON API rk_err_t CreateDeviceListFm(uint32 * list, void *arg)
{
    I2C_DEVICE_ARG stI2cArg;
    //I2S_DEV_ARG stI2sArg;
    FM_DEV_ARG stFmDevArg;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    AUDIO_DEV_ARG stAudioArg;
    HDC hCodec;
    rk_err_t ret;
    //Create BcoreDev...

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    stI2cArg.Channel = I2C_DEV1_PA;
    stI2cArg.speed = 100;
    ret = RKDev_Create(DEV_CLASS_I2C, I2C_DEV1,&stI2cArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\n I2C device create failure");
        goto err;
    }
    if(gSysConfig.RadioConfig.FmArea > RADIO_USA)
    {
        stFmDevArg.FmArea = RADIO_CHINA;
    }
    else
    {
        //stFmDevArg.FmArea = gSysConfig.RadioConfig.FmArea;
        stFmDevArg.FmArea = RADIO_CHINA;
    }
    stFmDevArg.hControlBus = RKDev_Open(DEV_CLASS_I2C, I2C_DEV1, NOT_CARE);
    if(stFmDevArg.hControlBus == NULL)
    {
        rk_print_string("\n I2C open error");
        goto err;
    }
    extern RK_FM_DRIVER fm5807Driver;
    stFmDevArg.pFMDriver = &fm5807Driver;
    stFmDevArg.pfmStateCallBcak = NULL; //for APP callback
    ret = RKDev_Create(DEV_CLASS_FM, 0, &stFmDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("FmDev create failure");
        goto err;
    }
    rk_print_string("FmDevList create Success...\n");
    return RK_SUCCESS;

    err:
    rk_print_string("FmDevList create error...\n");
    return RK_ERROR;

}
#endif

#ifdef _USB_
/*******************************************************************************
** Name: DeleteDeviceListUsbHostMsc
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 17:16:57
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t DeleteDeviceListUsbHostMsc(uint32 * list)
{

}

/*******************************************************************************
** Name: CreateDeviceListUsbHostMsc
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 17:16:10
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t CreateDeviceListUsbHostMsc(uint32 * list)
{

    USBMSC_DEV_ARG stUSBMSCDevArg;
    rk_err_t ret;
    uint32 DevID = 0;
    HDC hUSBMSCDev;
    HDC hUsbOtgDev;
    HDC hLun[USBMSC_LUN];
    int i = 0;
    uint8 valid_lun = 0;

#if 1
    hUsbOtgDev = RKDev_Open(DEV_CLASS_USBOTG, 0, NOT_CARE);
    if ((hUsbOtgDev == NULL) || (hUsbOtgDev == (HDC)RK_ERROR) || (hUsbOtgDev == (HDC)RK_PARA_ERR))
    {
        printf("\nUsbOtgDev open failure for Init USBMSC");
        return RK_ERROR;
    }

    for (i=0; i<USBMSC_LUN; i++)
    {
        if (gLun[i] != 0)
        {
            hLun[i] = RKDev_Open(DEV_CLASS_LUN, gLun[i], NOT_CARE);
            if (hLun[i] == NULL)
            {
                printf("\n11Lun%d device open failure(i=%d)",gLun[i],i);
                return RK_SUCCESS;
            }
        }
        else
        {
            printf ("\nvalid lun have %d",i);
            break;
        }
    }
    //有效Lun个数
    valid_lun = i;

    //Init USBMSCDev arg...
    stUSBMSCDevArg.hUsbOtgDev = hUsbOtgDev;
    for (i=0; i<valid_lun; i++)
    {
        stUSBMSCDevArg.hLun[i] = hLun[i];
    }
    stUSBMSCDevArg.valid_lun = valid_lun;

#endif

    //Create USBMSCDev...
    ret = RKDev_Create(DEV_CLASS_USBMSC, DevID, &stUSBMSCDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev create failure");
    }

    return RK_SUCCESS;

}
#endif

/*******************************************************************************
** Name: GetBtAudioDeviceName
** Input:uint8 * name
** Return: void
** Owner:void
** Date: 2016.3.14
** Time: 14:57:24
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void GetBtAudioDeviceName(uint8 * name)
{
#ifdef ENABLE_NFC
    strcpy(name,"nft_test");
#else
    strcpy(name,"RK-BT");
#endif
}

/*******************************************************************************
** Name: GetAirplayDeviceName
** Input:uint8  * name
** Return: void
** Owner:void
** Date: 2016.3.14
** Time: 14:56:46
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void GetAirplayDeviceName(uint8  * name)
{
    strcpy(name, "RK-AIRPLAY");
}
/*******************************************************************************
** Name: GetDlnaDeviceName
** Input:uint8 * name
** Return: void
** Owner:void
** Date: 2016.3.14
** Time: 14:54:14
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void GetDlnaDeviceName(uint8 * name)
{
    strcpy(name, "RK-DLNA");
}

#ifdef _USB_
#ifdef _FS_
/*******************************************************************************
** Name: DeviceListLoadFs
** Input:uint32 devicelist,uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.31
** Time: 11:54:48
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t DeviceListLoadFs(uint32 devicelist,uint32 * list)
{
    HDC hLun, hPar;
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    switch(devicelist)
    {
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS1:
        #endif
        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS1:
        #endif
        {
            hLun = RKDev_Open(DEV_CLASS_LUN, 2, NOT_CARE);

            if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
            {
                rk_print_string("lun2 open failure");
                return RK_ERROR;
            }

            stCreateParArg.hLun = hLun;

            DeviceResume(DEV_CLASS_LUN, 2);

            ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("\nlun2 get par failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateParArg);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("par0 create failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 0;
            i++;

            hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
            if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
            {
                rk_print_string("par0 open failure");
                return RK_ERROR;
            }

            stCreateFatArg.hPar = hPar;

            ret = RKDev_Create(DEV_CLASS_FAT, 0, &stCreateFatArg);

            if (ret != RK_SUCCESS)
            {
                if (ret != RK_SUCCESS)
                {
                    rk_print_string("fat0 create failure");
                    RKDev_Close(hPar);
                    return RK_ERROR;
                }
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 0;
            i++;

            stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);

            stVolumeInf.VolumeID = 'C';
            stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

            ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
            if (ret != RK_SUCCESS)
            {
                rk_print_string("add volume C failure");
                RKDev_Close(stVolumeInf.hVolume);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
            i++;

            {
                uint8 name[13];
                ret = FileDev_ReadVolumeName(FileSysHDC, 'C', name);
                if(ret == RK_SUCCESS)
                {
                    if(StrCmpA(name, "RKOSC", 6))
                    {
                        FileDev_WriteVolumeName(FileSysHDC, 'C', "RKOSC");
                    }
                }
            }
        }
        break;


        #ifdef _HIDDEN_DISK1_
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS2:
        #endif
        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS2:
        #endif
        {
            hLun = RKDev_Open(DEV_CLASS_LUN, 3, NOT_CARE);

            if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
            {
                rk_print_string("lun3 open failure");
                return RK_ERROR;
            }

            stCreateParArg.hLun = hLun;

            DeviceResume(DEV_CLASS_LUN, 3);

            ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("\nlun3 get par failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            ret = RKDev_Create(DEV_CLASS_PAR, 1, &stCreateParArg);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("par1 create failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 1;
            i++;

            hPar = RKDev_Open(DEV_CLASS_PAR, 1, NOT_CARE);
            if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
            {
                rk_print_string("par1 open failure");
                return RK_ERROR;
            }

            stCreateFatArg.hPar = hPar;

            ret = RKDev_Create(DEV_CLASS_FAT, 1, &stCreateFatArg);

            if (ret != RK_SUCCESS)
            {
                if (ret != RK_SUCCESS)
                {
                    rk_print_string("fat1 create failure");
                    RKDev_Close(hPar);
                    return RK_ERROR;
                }
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 1;
            i++;

            stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 1, NOT_CARE);

            stVolumeInf.VolumeID = 'A';
            stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

            ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
            if (ret != RK_SUCCESS)
            {
                rk_print_string("add volume A failure");
                RKDev_Close(stVolumeInf.hVolume);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
            i++;

            {
                uint8 name[13];
                ret = FileDev_ReadVolumeName(FileSysHDC, 'A', name);
                if(ret == RK_SUCCESS)
                {
                    if(StrCmpA(name, "RKOSA", 6))
                    {
                        FileDev_WriteVolumeName(FileSysHDC, 'A', "RKOSA");
                    }
                }
            }
        }
        break;
        #endif

        #ifdef _HIDDEN_DISK2_
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS3:
        #endif
        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS3:
        #endif
        {
            hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);

            if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
            {
                rk_print_string("lun4 open failure");
                return RK_ERROR;
            }

            stCreateParArg.hLun = hLun;

            DeviceResume(DEV_CLASS_LUN, 4);

            ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("\nlun4 get par failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            ret = RKDev_Create(DEV_CLASS_PAR, 2, &stCreateParArg);

            if (ret != RK_SUCCESS)
            {
                rk_print_string("par2 create failure");
                RKDev_Close(hLun);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 2;
            i++;

            hPar = RKDev_Open(DEV_CLASS_PAR, 2, NOT_CARE);
            if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
            {
                rk_print_string("par2 open failure");
                return RK_ERROR;
            }

            stCreateFatArg.hPar = hPar;

            ret = RKDev_Create(DEV_CLASS_FAT, 2, &stCreateFatArg);

            if (ret != RK_SUCCESS)
            {
                if (ret != RK_SUCCESS)
                {
                    rk_print_string("fat2 create failure");
                    RKDev_Close(hPar);
                    return RK_ERROR;
                }
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 2;
            i++;

            stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 2, NOT_CARE);

            stVolumeInf.VolumeID = 'B';
            stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

            ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
            if (ret != RK_SUCCESS)
            {
                rk_print_string("add volume B failure");
                RKDev_Close(stVolumeInf.hVolume);
                return RK_ERROR;
            }

            list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
            i++;

            {
                uint8 name[13];
                ret = FileDev_ReadVolumeName(FileSysHDC, 'B', name);
                if(ret == RK_SUCCESS)
                {
                    if(StrCmpA(name, "RKOSB", 6))
                    {
                        FileDev_WriteVolumeName(FileSysHDC, 'B', "RKOSB");
                    }
                }
            }
        }
        break;
        #endif

    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DeviceListRemoveFs
** Input:uint32 devicelist, uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.31
** Time: 11:54:03
*******************************************************************************/
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t DeviceListRemoveFs(uint32 devicelist, uint32 * list)
{
#if 0
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateArg;
    uint32 i, j, k;

    k = 0;

    for(i = 0; i < (DEVICE_LIST_NUM + MAX_D_DEV_LIST); i++)
    {
        for(j = 0; j < (MAX_DEV_PER_LIST + 1); j++)
        {
            //rk_printf("DeviceList[%d][%d] = %x, %x",  i, j, DeviceList, DeviceList[j]);
            if(DeviceList[j] == 0)
            {
                if(k == 4)
                {
                    stVolumeInf.VolumeID = NULL;
                    stVolumeInf.hVolume = RKDev_Open(((DeviceList[j - 2] >> 16) & 0x0000ffff), ((DeviceList[j - 2]) & 0x0000ffff));
                    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
                    if(ret != RK_SUCCESS)
                    {
                        rk_print_string("remove volume failure");
                        RKDev_Close(stVolumeInf.hVolume);
                        return RK_ERROR;
                    }

                    RKDev_Close(stVolumeInf.hVolume);

                    DeviceList[j - 1] = 0;

                    k = 3;
                    j--;
                }

                if(k == 3)
                {
                    ret = RKDev_Delete(((DeviceList[j - 1] >> 16) & 0x0000ffff), ((DeviceList[j - 1]) & 0x0000ffff), &stCreateFatArg);
                    if(ret != RK_SUCCESS)
                    {
                        rk_print_string("volume delete failure");
                        return RK_ERROR;
                    }

                    RKDev_Close(stCreateFatArg.hPar);
                    DeviceList[j - 1] = 0;
                    k = 2;
                    j--;
                }

                if(k == 2)
                {
                    ret = RKDev_Delete(((DeviceList[j - 1] >> 16) & 0x0000ffff), ((DeviceList[j - 1]) & 0x0000ffff), &stCreateArg);
                    if(ret != RK_SUCCESS)
                    {
                        rk_print_string("par delete failure");
                        return RK_ERROR;
                    }

                    RKDev_Close(stCreateArg.hLun);
                    DeviceList[j - 1] = 0;
                    k = 1;
                    j--;
                }
                break;
            }
            else if((((DeviceList[j] >> 16) & 0x0000ffff)  == DEV_CLASS_LUN)
                    && (((DeviceList[j]) & 0x0000ffff)  == ObjectId) && (k == 0))
            {
                k = 1;
            }
            else if((((DeviceList[j] >> 16) & 0x0000ffff)  == DEV_CLASS_PAR)
                    && (k == 1))
            {
                k = 2;
            }
            else if((((DeviceList[j] >> 16) & 0x0000ffff)  == DEV_CLASS_FAT)
                    && (k == 2))
            {
                k = 3;
            }
            else if((((DeviceList[j] >> 16) & 0x0000ffff)  == DEV_CLASS_FILE)
                    && (k == 3))
            {
                k = 4;
            }
            else
            {
                if(k)
                {
                    k = 0;
                    break;
                }
            }
        }

        DeviceList += (MAX_DEV_PER_LIST + 1);
    }

    return RK_SUCCESS;
#else
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateArg;
    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    switch(devicelist)
    {
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS1:
        #endif

        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS1:
        #endif
        {
            stVolumeInf.VolumeID = 'C';

            ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("remove volume c failure");
                return RK_ERROR;
            }

            RKDev_Close(stVolumeInf.hVolume);
            i--;
            list[i] = 0;


            if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
            {
                ret = RKDev_Delete(DEV_CLASS_FAT, 0, &stCreateFatArg);
                if(ret != RK_SUCCESS)
                {
                    rk_print_string("fat0 delete failure");
                    return RK_ERROR;
                }

                RKDev_Close(stCreateFatArg.hPar);
                i--;
                list[i] = 0;


            }

            ret = RKDev_Delete(DEV_CLASS_PAR, 0, &stCreateArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("par0 delete failure");
                return RK_ERROR;
            }

            ret = RKDev_Close(stCreateArg.hLun);
            i--;
            list[i] = 0;

            break;
        }

        #ifdef _HIDDEN_DISK1_
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS2:
        #endif

        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS2:
        #endif
        {
            stVolumeInf.VolumeID = 'A';

            ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("remove volume a failure");
                return RK_ERROR;
            }

            RKDev_Close(stVolumeInf.hVolume);
            i--;
            list[i] = 0;


            if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
            {
                ret = RKDev_Delete(DEV_CLASS_FAT, 1, &stCreateFatArg);
                if(ret != RK_SUCCESS)
                {
                    rk_print_string("fat 1 delete failure");
                    return RK_ERROR;
                }

                RKDev_Close(stCreateFatArg.hPar);
                i--;
                list[i] = 0;
            }

            ret = RKDev_Delete(DEV_CLASS_PAR, 1, &stCreateArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("par 1 delete failure");
                return RK_ERROR;
            }

            ret = RKDev_Close(stCreateArg.hLun);
            i--;
            list[i] = 0;
            break;
        }
        #endif

        #ifdef _HIDDEN_DISK2_
        #ifdef _SPI_BOOT_
        case DEVICE_LIST_SPI_FS3:
        #endif

        #ifdef _EMMC_BOOT_
        case DEVICE_LIST_EMMC_FS3:
        #endif
        {
            stVolumeInf.VolumeID = 'B';

            ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("remove volume B failure");
                return RK_ERROR;
            }

            RKDev_Close(stVolumeInf.hVolume);
            i--;
            list[i] = 0;

            if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
            {
                ret = RKDev_Delete(DEV_CLASS_FAT, 2, &stCreateFatArg);
                if(ret != RK_SUCCESS)
                {
                    rk_print_string("FAT 2 delete failure");
                    return RK_ERROR;
                }

                RKDev_Close(stCreateFatArg.hPar);
                i--;
                list[i] = 0;
            }

            ret = RKDev_Delete(DEV_CLASS_PAR, 2, &stCreateArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("par 2 delete failure");
                return RK_ERROR;
            }

            ret = RKDev_Close(stCreateArg.hLun);

            i--;
            list[i] = 0;

            break;
        }
        #endif

    }

    return RK_SUCCESS;
#endif
}
#endif
#endif

/*******************************************************************************
** Name: DeleteDeviceListSdio
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.2
** Time: 14:01:06
*******************************************************************************/
_ATTR_SYS_CODE_
COMMON API rk_err_t DeleteDeviceListSdio(uint32 * list)
{
    SDIO_DEV_ARG pstSdioArg;
    SDC_DEV_ARG stSdcArg;
    if (RKDev_Delete(DEV_CLASS_SDIO, 0, &pstSdioArg) != RK_SUCCESS)
    {
        printf("SDDev delete failureDevID=%d\n");
    }
    else
    {
        RKDev_Close(pstSdioArg.hSdc);
    }


    if (RKDev_Delete(DEV_CLASS_SDC, 1, &stSdcArg) != RK_SUCCESS)
    {
        printf("sdc1 delete failure\n");
    }
    else
    {
        RKDev_Close(stSdcArg.hDma);
    }

    rk_printf("Delete Sdio Device list");

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DeleteDeviceListFIFO
** Input:uint32 * list, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.30
** Time: 11:31:15
*******************************************************************************/
_ATTR_SYS_CODE_
COMMON API rk_err_t DeleteDeviceListFIFO(uint32 * list, void *arg)
{
    FIFO_DEV_ARG * pstFifoArg = (FIFO_DEV_ARG *)arg;
    RK_ERR_T ret;

    ret = RKDev_Delete(DEV_CLASS_FIFO,  pstFifoArg->ObjectId, pstFifoArg);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("fifo dev create failure");
        goto err;
    }
    return RK_SUCCESS;

    err:
    return RK_ERROR;
}

/*******************************************************************************
** Name: CreateDeviceListFIFO
** Input:uint32 * list, void *arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.30
** Time: 11:29:15
*******************************************************************************/
_ATTR_SYS_CODE_
COMMON API rk_err_t CreateDeviceListFIFO(uint32 * list, void *arg)
{
    FIFO_DEV_ARG * pstFifoArg = (FIFO_DEV_ARG *)arg;
    RK_ERR_T ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    ret = RKDev_Create(DEV_CLASS_FIFO, pstFifoArg->ObjectId, pstFifoArg);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("fifo dev create failure");
        goto err;
    }
    rk_print_string("fifo dev create success");

    return RK_SUCCESS;

    err:
    return RK_ERROR;
}


/*******************************************************************************
** Name: CreateCreateDeviceListSdio
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 15:08:26
*******************************************************************************/
_ATTR_SYS_CODE_
COMMON API rk_err_t CreateDeviceListSdio(uint32 * list)
{
#ifdef _WIFI_
    SDC_DEV_ARG stSdcArg;
    HDC hSdc;
    SDIO_DEV_ARG stSdioArg;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    stSdcArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    stSdcArg.Channel = 0;
    if (stSdcArg.hDma <= 0)
    {
        rk_print_string("\ndma device open failure");
        goto err;
    }
    ret = RKDev_Create(DEV_CLASS_SDC, SDC1, &stSdcArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsdc device device failure");
        goto err;
    }

    hSdc = RKDev_Open(DEV_CLASS_SDC, SDC1, NOT_CARE);

    if ((hSdc == NULL) || (hSdc == (HDC)RK_ERROR) || (hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nsdc device open failure");
        goto err;
    }
    stSdioArg.hSdc = hSdc;

    ret = RKDev_Create(DEV_CLASS_SDIO, 0, &stSdioArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nSDIO0 device create failure");
        goto err;
    }
#endif
    return RK_SUCCESS;

    err:

    return RK_ERROR;
}

#ifdef _USB_
/*******************************************************************************
** Name: DeleteDeviceListUsbDeviceMsc
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.20
** Time: 9:44:37
*******************************************************************************/
//_BSP_EVK_V20_BSP_COMMON_
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t DeleteDeviceListUsbDeviceMsc(uint32 * list)
{
    uint32 DevID = 0;
    int i = 0;

    USBMSC_DEV_ARG pstUSBMSCDevArg;
    //Delete USBMSC
    uint32 j;

    for(j = 0 ; j < (MAX_DEV_PER_LIST + 1); j++)
    {
        if(list[j] == 0)
        {
            break;
        }
    }

    rk_printf("delete usb device list");

    if (RKDev_Delete(DEV_CLASS_USBMSC, DevID, (void*)&pstUSBMSCDevArg) != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev delete failure");
    }
    else
    {
        for (i=0; i<pstUSBMSCDevArg.valid_lun; i++)
        {
            RKDev_Close(pstUSBMSCDevArg.hLun[i]);
            j--;
            list[j] = 0;
        }

        RKDev_Close(pstUSBMSCDevArg.hUsbOtgDev);
        j--;
        list[j] = 0;
    }

    //Delete USBOTG
    if (RKDev_Delete(DEV_CLASS_USBOTG, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("UsbOtgDev delete failure");
    }

    j--;
    list[j] = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: CreateDeviceListUsbDeviceMsc
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:49:38
*******************************************************************************/
//_BSP_EVK_V20_BSP_COMMON_
_SYSTEM_USBSERVER_USBSERVICE_COMMON_
COMMON API rk_err_t CreateDeviceListUsbDeviceMsc(uint32 * list)
{
    USBOTG_DEV_ARG stUsbOtgDevArg;
    USBMSC_DEV_ARG stUSBMSCDevArg;
    USBMSC_DEV_ARG pstUSBMSCDevArg;
    HDC hUSBMSCDev;
    HDC hUsbOtgDev;
    HDC hLun[USBMSC_LUN];

    uint32 j;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    j = 0;


    rk_err_t ret;
    uint32 DevID = 0;
    int i = 0;
    uint8 valid_lun = 0;

    //Init UsbOtgDev arg...
    stUsbOtgDevArg.usbmode = USBOTG_MODE_DEVICE;
    stUsbOtgDevArg.usbspeed = 10;

    //Create UsbOtgDev...
    ret = RKDev_Create(DEV_CLASS_USBOTG, DevID, &stUsbOtgDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("UsbOtgDev create failure");
        return RK_ERROR;
    }


    hUsbOtgDev = RKDev_Open(DEV_CLASS_USBOTG, 0, NOT_CARE);
    if ((hUsbOtgDev == NULL) || (hUsbOtgDev == (HDC)RK_ERROR) || (hUsbOtgDev == (HDC)RK_PARA_ERR))
    {
        printf("\nUsbOtgDev open failure for Init USBMSC");
        goto USB_ERROR1;
    }


    for (i=0; i<USBMSC_LUN; i++)
    {
        if (gLun[i] != 0)
        {
            hLun[i] = RKDev_Open(DEV_CLASS_LUN, gLun[i], NOT_CARE);
            if (hLun[i] == NULL)
            {
                printf("\n22Lun%d device open failure(i=%d)",gLun[i],i);
                goto USB_ERROR2;
            }
        }
        else
        {
            printf ("\nvalid lun have %d",i);
            break;
        }
    }
    //有效Lun个数
    valid_lun = i;

    //Init USBMSCDev arg...
    stUSBMSCDevArg.hUsbOtgDev = hUsbOtgDev;
    for (i=0; i<valid_lun; i++)
    {
        stUSBMSCDevArg.hLun[i] = hLun[i];
    }
    stUSBMSCDevArg.valid_lun = valid_lun;


    //Create USBMSCDev...
    ret = RKDev_Create(DEV_CLASS_USBMSC, DevID, &stUSBMSCDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev create failure");
        i = valid_lun;
        goto USB_ERROR2;
    }

    list[j] = ((j + 1 + valid_lun) << 24) | ((j + 1) << 16) | (DEV_CLASS_USBOTG << 8) | 0;
    j++;

    for(i = 0; i < valid_lun; i++)
    {
        list[j] = ((j + valid_lun - i) << 24) | ((j + 1) << 16) | (DEV_CLASS_LUN << 8) | gLun[i];
        j++;
    }

    list[j] = ((j + 1) << 24) | (DEV_CLASS_USBMSC<< 8) | 0;
    j++;

    //Open USBMSCDev...
    hUSBMSCDev = RKDev_Open(DEV_CLASS_USBMSC, 0, NOT_CARE);
    if ((hUSBMSCDev == NULL) || (hUSBMSCDev == (HDC)RK_ERROR) || (hUSBMSCDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("USBMSCDev open failure");
        goto USB_ERROR3;
    }

    //do connect....
    UsbMscDev_Connect(hUSBMSCDev, 0);
    //close USBMSCDev...
    RKDev_Close(hUSBMSCDev);


    return RK_SUCCESS;

USB_ERROR3:
    if (RKDev_Delete(DEV_CLASS_USBMSC, DevID, (void*)&pstUSBMSCDevArg) != RK_SUCCESS)
    {
        rk_print_string("USBMSCDev delete failure");
    }
    else
    {
        rk_print_string("USBMSCDev delete success");
    }


USB_ERROR2:
    for (j=0; j<i; j++)
    {
        RKDev_Close(hLun[j]);
    }

    RKDev_Close(hUsbOtgDev);

USB_ERROR1:
    //Delete USBOTG
    if (RKDev_Delete(DEV_CLASS_USBOTG, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("UsbOtgDev delete failure");
    }
    else
    {
        rk_print_string("UsbOtgDev delete success");
    }

    return RK_ERROR;

}
#endif
/*******************************************************************************
** Name: CreateDeviceListSD
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:41:22
*******************************************************************************/
_ATTR_SYS_CODE_
COMMON API rk_err_t CreateDeviceListSD(uint32 * list)
{

}


/*******************************************************************************
** Name: Num2String
** Input:uint8 * pstr, uint32 num
** Return: uint32
** Owner:aaron.sun
** Date: 2015.11.8
** Time: 19:23:46
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 Num2String(uint8 * pstr, uint32 num)
{
    uint32 i;
    uint8 pstr1[16];

    i = 0;

    do
    {
        pstr1[i++] = num % 10 + 0x30;
        num = num / 10;

    }
    while(num != 0);

    num = i;

    for(i = 0; i < num; i++)
        pstr[num - i - 1] = pstr1[i];

    pstr[num] = 0;

    return num;
}

#ifdef _LOG_DEBUG_
#ifdef __DRIVER_BCORE_BCOREDEVICE_C__
/*******************************************************************************
** Name: BBReqDebug
** Input:uint8 * buf
** Return: void
** Owner:aaron.sun
** Date: 2015.6.13
** Time: 16:04:06
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void BBReqDebug(uint8 * buf)
{
    pBBDebugBuf = buf;
    BcoreDebug = 1;
}

/*******************************************************************************
** Name: BBDebug
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.13
** Time: 16:03:13
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void BBDebug(void)
{
    uint32 tmpcnt,i,j;
    uint8 dgbBuffer[DEBUG_TIME_LEN];

    //printf("BcoreDebug: %d", BcoreDebug);
    if (BcoreDebug)
    {
        #if 1
        tmpcnt = SysTickCounter;
        for (i = 0; i < DEBUG_TIME_LEN; i++)
        {
            dgbBuffer[i] = tmpcnt % 10;
            tmpcnt = tmpcnt / 10;
        }

        pBBDebugBuf[0] = '\r';
        pBBDebugBuf[1] = '\n';
        pBBDebugBuf[2] = '[';

        pBBDebugBuf[3] = 'B';

        pBBDebugBuf[4] = ']';

        pBBDebugBuf[5] = '[';

        for (i = 0 ,j = 0; i < DEBUG_TIME_LEN; i++)
        {
            pBBDebugBuf[j + 6] = dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30;
            if (DEBUG_TIME_LEN - 1 - i == 2)
            {
                j++;
                pBBDebugBuf[j + 6] = '.';
            }
            j++;
        }

        pBBDebugBuf[j + 6] = ']';
        #endif

        UartDev_Write(UartHDC, pBBDebugBuf, StrLenA(pBBDebugBuf), SYNC_MODE, NULL);

        BcoreDebug = 0;
        MailBoxWriteA2BCmd(MSGBOX_CMD_SYSTEM_PRINT_LOG_OK, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
        MailBoxWriteA2BData(0, MAILBOX_ID_0, MAILBOX_CHANNEL_0);
    }

}
#endif
#endif

/*******************************************************************************
** Name: vApplicationIdleHook
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:17:37
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void vApplicationIdleHook(void)
{
    SysTickCounterSave = 0;
    if(SysTickCounter - SysTickCheck > 1000)
    {
        SysTickCheck = SysTickCounter;
        //printf("t");
    }

    if(pIdleFun != NULL)
    {
        pIdleFun();
    }

    if(hWdt != NULL)
    {
        WDTDev_Feed(hWdt);
    }
}

/*******************************************************************************
** Name: Unicode2Ascii
** Input:uint8 * pbAscii, uint16 * pwUnicode, uint32 len
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:22:40
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 Unicode2Ascii(uint8 * pbAscii, uint16 * pwUnicode, uint32 len)
{
    uint32 i;
    uint8 * pbUnicode = (uint8 *)pwUnicode;

    len = len / 2;

    for (i = 0; i < len;)
    {
        if ((*pbUnicode == 0) && (*(pbUnicode + 1) == 0))
        {
            pbAscii[i] = 0;
            break;
        }

        if (*pbUnicode != 0)
        {
            pbAscii[i] = *pbUnicode;
            i++;
        }

        if (*(pbUnicode + 1) != 0)
        {
            pbAscii[i] = *(pbUnicode + 1);
            i++;
        }
        pbUnicode += 2;
    }

    return i;
}
/*******************************************************************************
** Name: Ascii2Unicode
** Input:uint8 * pbAscii, uint16 * pwUnicode, uint32 len
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:21:35
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 Ascii2Unicode(uint8 * pbAscii, uint16 * pwUnicode, uint32 len)
{
    uint32 i;

    for (i = 0; i < len; i++)
    {
        pwUnicode[i] = (uint16)pbAscii[i];
    }
    return i * 2;
}

/*******************************************************************************
** Name: String2Num
** Input:uint8 * pstr
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:08:07
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 String2Num(uint8 * pstr)
{
    uint32 num = 0;
    int32 i;

    /*ptr save the fisrt char of pstr*/
    const uint8* ptr = pstr;

    /*if pstr start with '-' or '+' sign,move to the next charactor.*/
    if (*pstr == '-' || *pstr == '+')
        pstr++;

    while (*pstr != 0)
    {
        /*if pstr is none digal number ,break and return*/
        if (*pstr < '0' || *pstr > '9')
            break;
        /*current is digal ,calculate the value*/
        num = num * 10 + (*pstr - '0');
        pstr++;
    }

    if ( *ptr == '-')
        num = -num;

    return num;
}
/*******************************************************************************
** Name: MemCpy
** Input:uint8 * trg, uint8 * src, uint32 len
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:07:14
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void MemCpy(uint8 * trg, uint8 * src, uint32 len)
{
    uint32 i;

    for (i = 0; i < len; i++)
    {
        *trg++ = *src++;
    }
}
/*******************************************************************************
** Name: StrCmpA
** Input:uint8 * pstr1, uint8 * pstr2, uint32 len
** Return: int32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:06:04
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API int32 StrCmpA(uint8 * pstr1, uint8 * pstr2, uint32 len)
{
    uint32 len1, len2;

    if (len == 0)
    {
        len1 = StrLenA(pstr1);
        len2 = StrLenA(pstr2);

        if (len1 > len2)
        {
            return 1;
        }
        else if (len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }


    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}
/*******************************************************************************
** Name: StrCmpW
** Input:uint16 * pstr1, uint16 * pstr2, uint32 len
** Return: int32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:04:53
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API int32 StrCmpW(uint16 * pstr1, uint16 * pstr2, uint32 len)//include '0'
{
    uint32 len1, len2;

    if (len == 0)
    {
        len1 = StrLenW(pstr1);
        len2 = StrLenW(pstr2);

        if (len1 > len2)
        {
            return 1;
        }
        else if (len1 < len2)
        {
            return -1;
        }
        else
        {
            len = len1;
        }
    }

    while (len--)
    {
        if ((*pstr1 == 0) && (*pstr2 == 0))
        {
            return 0;
        }
        else if (*pstr1 == 0)
        {
            return -1;
        }
        else if (*pstr2 == 0)
        {
            return 1;
        }
        if (*pstr1 > * pstr2)
        {
            return 1;
        }
        else if (*pstr1 < * pstr2)
        {
            return -1;
        }
        pstr1++;
        pstr2++;
    }

    return 0;

}
/*******************************************************************************
** Name: StrLenA
** Input:uint8 * pstr
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:03:38
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 StrLenA(uint8 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}
/*******************************************************************************
** Name: StrLenW
** Input:uint16 * pstr
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:02:57
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 StrLenW(uint16 * pstr)
{
    uint32 i = 0;
    while (*pstr++ != 0)
        i++;

    return i;
}
/*******************************************************************************
** Name: rk_count_clk_end
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:59:48
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API uint32 rk_count_clk_end(void)
{
    uint32 SysTick, SysTickCnt;
    uint32 clk;
    uint32 ms;

    SysTickCnt = SysTickCounter;
    SysTick = nvic->SysTick.Value;

    if (SysTickCntSave == SysTickCnt)
    {
        clk = SysTickSave - SysTick;
    }
    else
    {
        clk = SysTickSave + (SysTickCnt - SysTickCntSave - 1) * nvic->SysTick.Reload
              + (nvic->SysTick.Reload - SysTick);
    }

    ms = ((long long)clk) * 10/nvic->SysTick.Reload;
    printf("\ntotal clk = %u, %u, %u", clk, SysTickCntSave, SysTickCnt);
    return ms;
   // printf("\ntotal us = %lld\n", ((long long)clk)*10000/nvic->SysTick.Reload);
}

/*******************************************************************************
** Name: rk_count_clk_start
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:58:24
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void rk_count_clk_start(void)
{
    SysTickCntSave = SysTickCounter;
    SysTickSave = nvic->SysTick.Value;
}


#ifdef _LOG_DEBUG_
/*******************************************************************************
** Name: rk_print_string
** Input:const uint8 * pstr
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:57:38
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void rk_print_string(const uint8 * pstr)
{
    if(UartHDC == NULL)
        return;

    if (pstr[0] == '\n')
    {
        UartDev_Write(UartHDC, "\r\n[A]", 5, SYNC_MODE, NULL);
        UartDev_Write(UartHDC, pstr + 1, strlen(pstr) - 1, SYNC_MODE, NULL);

    }
    else
    {

        UartDev_Write(UartHDC, pstr, strlen(pstr), SYNC_MODE, NULL);
    }


}
/*******************************************************************************
** Name: rk_printf_no_time
** Input:const char * fmt,...
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:56:37
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void rk_printf_no_time(const char * fmt,...)
{
    uint8 * buf;
    uint8 size;

    if(UartHDC == NULL)
        return;

    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    buf = rkos_memory_malloc(1024);

    UartDev_Write(UartHDC, "\r\n[A]", 5, SYNC_MODE, NULL);

    size =  vsnprintf((char*)buf, 255, (const char *)fmt, arg_ptr);
    UartDev_Write(UartHDC, buf, size,  SYNC_MODE, NULL);

    rkos_memory_free(buf);


}


/*******************************************************************************
** Name: rk_printf
** Input:const char * fmt,...
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:45:24
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void rk_printf(const char * fmt,...)
{

#if (DEBUG_UART_ADDR == 0x400a0000)//uart0 //jjjhhh 20161115
    uint8 * buf;
    uint8 size;
    uint32 tmpcnt,i,j;
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    va_list arg_ptr;

    if(UartHDC == NULL)
        return;

    buf = rkos_memory_malloc(1024);

    tmpcnt = SysTickCounter;
    for (i = 0; i < DEBUG_TIME_LEN; i++)
    {
        dgbBuffer[i] = tmpcnt % 10;
        tmpcnt = tmpcnt / 10;
    }

    buf[0] = '\r';
    buf[1] = '\n';
    buf[2] = '[';

    buf[3] = 'A';

    buf[4] = ']';

    buf[5] = '[';

    for (i = 0 ,j = 0; i < DEBUG_TIME_LEN; i++)
    {
        buf[j + 6] = dgbBuffer[DEBUG_TIME_LEN - 1 - i] + 0x30;
        if (DEBUG_TIME_LEN - 1 - i == 2)
        {
            j++;
            buf[j + 6] = '.';
        }
        j++;
    }

    buf[j + 6] = ']';

    va_start(arg_ptr, fmt);

    size =  vsnprintf((char*)(buf + j + 7), 900, (const char *)fmt, arg_ptr);
    UartDev_Write(UartHDC, buf, size + 16,  SYNC_MODE, NULL);
    rkos_memory_free(buf);
#endif

}
#endif

/*******************************************************************************
** Name: debug_hex
** Input:char *buf, int BufSize, int radix
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:51:57
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API void debug_hex(char *buf, int BufSize, int radix)
{

    int i;

    for (i=0; i<BufSize; i++)
    {
        if ((i>0)&&((i % 16) == 0))
            printf("\n");

        if (radix == 10)
            printf(" %d", buf[i]);
        else
            printf(" %02x", buf[i]);
    }
    printf("\n");

}

/*******************************************************************************
** Name: fputc
** Input:int ch, FILE *f
** Return: int
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:39:24
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON API int fputc(int ch, FILE *f)
{
    uint8 dgbBuffer[DEBUG_TIME_LEN];
    uint32 tmpcnt, i;

    #ifndef _RKOS_DEBUG_
        if(UartHDC == NULL)
            return RK_SUCCESS;
    #endif

#if 1
    if (ch == '\n')
    {
        tmpcnt = SysTickCounter;

        for (i = 0; i < DEBUG_TIME_LEN; i++)
        {
            dgbBuffer[i] = tmpcnt % 10;
            tmpcnt = tmpcnt / 10;
        }

        fputc_hook('\r');
        fputc_hook('\n');
        fputc_hook('[');
        for (i = 0; i < DEBUG_TIME_LEN; i++)
        {
            fputc_hook(dgbBuffer[DEBUG_TIME_LEN - 1 -i]+0x30);
            if (DEBUG_TIME_LEN - 1 -i == 2)
            {
                fputc_hook('.');
            }
        }
        fputc_hook(']');

        return RK_SUCCESS;
    }
#else
    if(ch == '\n')
    {
        fputc_hook('\r');
        fputc_hook('\n');
    }

#endif

    fputc_hook(ch);

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
** Name: CpuIdle
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:18:49
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON FUN void CpuIdle(void)
{

    if (SysState == LEVEL_INIT)
    {
        SysWakeCnt = SysTickCounter;
        PrevSysWakeCnt = SysWakeCnt;
        PrevSysTickCounter = SysTickCounter;
        SysState = LEVEL_0;
    }
    else if (SysState != LEVEL_0)
    {
        SysResume(SysState);

        //printf("r\n");
        SysState = LEVEL_0;

        SysWakeCnt = SysTickCounter;
        PrevSysWakeCnt = SysWakeCnt;
        PrevSysTickCounter = SysTickCounter;
    }
    else
    {
        __WFI();

        SysWakeCnt++;

        if ((SysWakeCnt - PrevSysWakeCnt) == (SysTickCounter - PrevSysTickCounter))
        {
            if ((SysWakeCnt - PrevSysWakeCnt) >= 300) //wait 3s
            {
                //SysState = LEVEL_1;
                //if(FW_LoadSegment(SEGMENT_ID_RESUME, SEGMENT_OVERLAY_CODE) != RK_SUCCESS)
                {

                }
                //SysTickDisable();
                //printf("s");
            }
        }
        else
        {
            //printf("%d\n", SysTickCounter - PrevSysTickCounter);
            SysWakeCnt = SysTickCounter;
            PrevSysWakeCnt = SysWakeCnt;
            PrevSysTickCounter = SysTickCounter;
        }
    }


}
/*******************************************************************************
** Name: fputc_hook
** Input:char ch
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:43:49
*******************************************************************************/
_BSP_EVK_V20_BSP_COMMON_
COMMON FUN void fputc_hook(char ch)
{
#ifdef DEBUG_UART_PORT//jjjhhh 20161109
    while (UARTWriteByte(DEBUG_UART_PORT,(uint8*)&ch, 1) == 0);
#endif
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SystemDeInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.4.27
** Time: 16:27:58
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API void SystemDeInit(void)
{

    FREQ_EnterModule(FREQ_INIT);

    PowerManagerEnd();

    if(RKTaskDelete(TASK_ID_SHELL,0,SYNC_MODE) != RK_SUCCESS)
    {
        rk_printf("shell task delete fail");
    }

    if(RKTaskDelete(TASK_ID_MAIN,0,SYNC_MODE) != RK_SUCCESS)
    {
        rk_printf("task manager delete fail");
    }

    #ifdef _FS_
    if(DeleteDeviceList(DEVICE_LIST_DIR, NULL) != RK_SUCCESS)
    {
        rk_printf("dir device list delete fail");
    }
    #endif

    if(DeleteDeviceList(DEVICE_LIST_ADUIO_PLAY, NULL) != RK_SUCCESS)
    {
        rk_printf("audio play device list delete fail");
    }

    #ifdef __DRIVER_BCORE_BCOREDEVICE_C__
    if(RKDev_Delete(DEV_CLASS_BCORE, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("\nbcore delete fail");
    }
    #endif

    SaveSysInformation(1);

    #ifdef _USE_GUI_
    if(RKTaskDelete(TASK_ID_GUI,0,SYNC_MODE) != RK_SUCCESS)
    {
        rk_printf("gui task delete fail");
    }
    if(DeleteDeviceList(DEVICE_LIST_DISPLAY, NULL) != RK_SUCCESS)
    {
        rk_printf("display device list delete fail");
    }
    #endif

    RKDev_Close(hKey);

    FW_CodePageDeInit();

    if(DeleteDeviceList(DEVICE_LIST_ADC_KEY, NULL) != RK_SUCCESS)
    {
        rk_printf("adc key device list delete fail");
    }

    #ifdef _FS_
    #ifdef _EMMC_BOOT_
    if(DeleteDeviceList(DEVICE_LIST_EMMC_FS1, NULL) != RK_SUCCESS)
    {
        rk_printf("emmc fs1 device list delete fail");
    }

    #ifdef _HIDDEN_DISK1_
    if(DeleteDeviceList(DEVICE_LIST_EMMC_FS2, NULL) != RK_SUCCESS)
    {
        rk_printf("emmc fs2 device list delete fail");
    }
    #endif

    #ifdef _HIDDEN_DISK2_
    if(DeleteDeviceList(DEVICE_LIST_EMMC_FS3, NULL) != RK_SUCCESS)
    {
        rk_printf("emmc fs3 device list delete fail");
    }
    #endif
    #endif

    #ifdef _SPI_BOOT_
    #ifdef _HIDDEN_DISK1_
    if(DeleteDeviceList(DEVICE_LIST_SPI_FS2, NULL) != RK_SUCCESS)
    {
        rk_printf("spi fs2 device list delete fail");
    }
    #endif

    #ifdef _HIDDEN_DISK2_
    if(DeleteDeviceList(DEVICE_LIST_SPI_FS3, NULL) != RK_SUCCESS)
    {
        rk_printf("spi fs3 device list delete fail");
    }
    #endif

    if(DeleteDeviceList(DEVICE_LIST_SPI_FS1, NULL) != RK_SUCCESS)
    {
        rk_printf("spi fs1 device list delete fail");
    }
    #endif

    if(RKDev_Delete(DEV_CLASS_MSG, 0, NULL) != RK_SUCCESS)
    {
        rk_printf("msg device list delete fail");
    }

    RKDev_Close(FileSysHDC);

    if(RKDev_Delete(DEV_CLASS_FILE, 0, NULL) != RK_SUCCESS)
    {
        rk_printf("file device delete fail");
    }
    #endif

    RKDev_Close(hWdt);

    if(RKDev_Delete(DEV_CLASS_WDT, 0, NULL) != RK_SUCCESS)
    {
        rk_printf("watchdog device delete fail");
    }

    //RKDev_Close(UartHDC);

    //if(RKDev_Delete(DEV_CLASS_UART, DEBUG_UART_PORT, NULL) != RK_SUCCESS)
    {
        //rk_printf("uart device list delete fail");
    }

    FW_Resource_DeInit();

    #ifdef _EMMC_BOOT_
    if(DeleteDeviceList(DEVICE_LIST_EMMC_DATABASE, NULL) != RK_SUCCESS)
    {
        rk_printf("emmc database device list delete fail");
    }

    if(DeleteDeviceList(DEVICE_LIST_EMMC_BOOT, NULL) != RK_SUCCESS)
    {
        rk_printf("emmc boot device list delete fail");
    }
    #endif

    #ifdef _SPI_BOOT_
    if(DeleteDeviceList(DEVICE_LIST_SPI_DATABASE, NULL) != RK_SUCCESS)
    {

    }


    if(DeleteDeviceList(DEVICE_LIST_SPI_BOOT, NULL) != RK_SUCCESS)
    {

    }
    #endif

}

/*******************************************************************************
** Name: SystemInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2016.4.27
** Time: 15:10:29
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API void SystemInit(void)
{
    UART_DEV_ARG stUartArg;
    rk_err_t ret;

    PowerManagerInit();

    FREQ_EnterModule(FREQ_INIT);

#ifdef _EMMC_BOOT_
    if (CreateDeviceList(DEVICE_LIST_EMMC_BOOT, NULL) != RK_SUCCESS)
    {
        rk_printf("Emmc Code List Create Fail");
        while (1);
    }
#endif

#ifdef _SPI_BOOT_
    if (CreateDeviceList(DEVICE_LIST_SPI_BOOT, NULL) != RK_SUCCESS)
    {

        rk_printf("Spi Code List Create Fail");
        while (1);
    }
#endif

    FwCheck();

    if((FW1Valid == 0) || (FW2Valid == 0))
    {
        FwRecovery();//recovery the fw which was broken
    }

    PmuSetSysRegister(3, 0);

    FW_Resource_Init();

#ifdef _EMMC_BOOT_
    CreateDeviceList(DEVICE_LIST_EMMC_DATABASE, NULL);
#endif


#ifdef _SPI_BOOT_
    CreateDeviceList(DEVICE_LIST_SPI_DATABASE, NULL);
#endif

    FW_DBInit();
#ifdef DEBUG_UART_PORT//jjjhhh 20161109
    stUartArg.dwBitWidth = UART_DATA_8B;
    stUartArg.dwBitRate = UART_BR_115200;
#ifndef _BROAD_LINE_OUT_
    stUartArg.Channel = UART_CHA;
#else
     stUartArg.Channel = UART_CHB;
#endif

    stUartArg.stopbit = UART_ONE_STOPBIT;
    stUartArg.parity = UART_PARITY_DISABLE;


    if(RKDev_Create(DEV_CLASS_UART, DEBUG_UART_PORT, &stUartArg) == RK_SUCCESS)
    {
         UartHDC = RKDev_Open(DEV_CLASS_UART, DEBUG_UART_PORT, NOT_CARE);
    }
    else
#endif
    {
         UartHDC = NULL;
    }

    #ifdef _FS_
    if(RKDev_Create(DEV_CLASS_FILE, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("\nfile device create failure");
        FileSysHDC = NULL;
    }
    else
    {
        FileSysHDC = RKDev_Open(DEV_CLASS_FILE, 0, NOT_CARE);
    }

    #ifdef _EMMC_BOOT_
    CreateDeviceList(DEVICE_LIST_EMMC_FS1, NULL);
    #ifdef _HIDDEN_DISK1_
    CreateDeviceList(DEVICE_LIST_EMMC_FS2, NULL);
    #endif
    #ifdef _HIDDEN_DISK2_
    CreateDeviceList(DEVICE_LIST_EMMC_FS3, NULL);
    #endif
    #endif


    #ifdef _SPI_BOOT_
    CreateDeviceList(DEVICE_LIST_SPI_FS1, NULL);
    #ifdef _HIDDEN_DISK1_
    CreateDeviceList(DEVICE_LIST_SPI_FS2, NULL);
    #endif
    #ifdef _HIDDEN_DISK2_
    CreateDeviceList(DEVICE_LIST_SPI_FS3, NULL);
    #endif
    #endif

    {
        uint32 i;
        i = 0;
        FILE_ATTR stFileAttr;
        HDC hKeyFile;

        gLun[0] = 2;
        i++;

        #ifdef _HIDDEN_DISK1_
        if(RKDeviceFind(DEV_CLASS_LUN, 3) != RK_ERROR)
        {
            stFileAttr.FileName = NULL;
            stFileAttr.Path = L"C:\\RKOSA.KEY";
            hKeyFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
            if((int)hKeyFile > 0)
            {
                gLun[i] = 3;
                i++;
                FileDev_CloseFile(hKeyFile);
            }
        }
        #endif

        #ifdef _HIDDEN_DISK2_
        if(RKDeviceFind(DEV_CLASS_LUN, 4) != RK_ERROR)
        {
            stFileAttr.FileName = NULL;
            stFileAttr.Path = L"C:\\RKOSB.KEY";
            hKeyFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
            if((int)hKeyFile > 0)
            {
                gLun[i] = 4;
                i++;
                FileDev_CloseFile(hKeyFile);
            }
        }
        #endif

        gLun[i] = 0;

    }
    #endif


    if(RKDev_Create(DEV_CLASS_MSG, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("\nmsg device create failure");
    }

    #if 1
    {
        WDT_DEV_ARG stWDTDevArg;

        stWDTDevArg.timeout = PERIOD_RANGE_0X7FFFFFFF;

        ret = RKDev_Create(DEV_CLASS_WDT, 0, &stWDTDevArg);
        if(ret != RK_SUCCESS)
        {
            rk_print_string("WDTDev create failure");
            hWdt = NULL;
        }
        else
        {
            hWdt = RKDev_Open(DEV_CLASS_WDT, 0, NOT_CARE);
        }
    }
    #endif


    CreateDeviceList(DEVICE_LIST_ADC_KEY, NULL);

    hKey = RKDev_Open(DEV_CLASS_KEY, 0, NOT_CARE);

    FW_CodePageInit();

#ifdef _USE_GUI_
    CreateDeviceList(DEVICE_LIST_DISPLAY, NULL);
    RKTaskCreate(TASK_ID_GUI,0, NULL, SYNC_MODE);
#else
    PmuSetSysRegister(0, 0x55aa55aa);
#endif

    if(Grf_CheckVbus())
    {	
        if(PmuGetSysRegister(0) == 0x55aa55aa)
        {	
            #ifdef _USE_GUI_
            {
                RKGUI_ICON_ARG pGcArg;
                HGC pGc;
                pGcArg.resource= IMG_ID_RKOS;
                pGcArg.x= 0;
                pGcArg.y= 0;
                pGcArg.display = 1;
                pGcArg.level = 0;
                pGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
                GuiTask_DeleteWidget(pGc);
            }
            #endif

            LoadSysInformation();

            CreateDeviceList(DEVICE_LIST_ADUIO_PLAY, NULL);

            #ifdef __DRIVER_BCORE_BCOREDEVICE_C__
            //Create BcoreDev...
            ret = RKDev_Create(DEV_CLASS_BCORE, 0, NULL);
            if (ret != RK_SUCCESS)
            {
                rk_print_string("\nBcoreDev create failure");
            }
            #endif

            #ifdef _FS_
            CreateDeviceList(DEVICE_LIST_DIR, NULL);
            #endif

            RKTaskCreate(TASK_ID_MAIN,0, NULL, SYNC_MODE);

#ifdef _USE_SHELL_
            RKTaskCreate(TASK_ID_SHELL,0, NULL, SYNC_MODE);
#endif
            PowerManagerStart();

        }
        else
        {
            rk_printf("charge...");
            PmuSetSysRegister(0, 0x55aa55aa);
            RKTaskCreate(TASK_ID_CHARGE, 0, NULL, SYNC_MODE);
        }

    }
    else
    {	
         #ifdef _USE_GUI_
         {
            RKGUI_ICON_ARG pGcArg;
            HGC pGc;
            pGcArg.resource= IMG_ID_RKOS;
            pGcArg.x= 0;
            pGcArg.y= 0;
            pGcArg.display = 1;
            pGcArg.level = 0;
            pGc= GUITask_CreateWidget(GUI_CLASS_ICON, &pGcArg);
            GuiTask_DeleteWidget(pGc);
         }
         #endif

        LoadSysInformation();

        CreateDeviceList(DEVICE_LIST_ADUIO_PLAY, NULL);

        #ifdef __DRIVER_BCORE_BCOREDEVICE_C__
        //Create BcoreDev...
        ret = RKDev_Create(DEV_CLASS_BCORE, 0, NULL);
        if (ret != RK_SUCCESS)
        {
            rk_print_string("\nBcoreDev create failure");
        }
        #endif

        #ifdef _FS_
        CreateDeviceList(DEVICE_LIST_DIR, NULL);
        #endif

        RKTaskCreate(TASK_ID_MAIN,0, NULL, SYNC_MODE);

        #ifdef _USE_SHELL_
        RKTaskCreate(TASK_ID_SHELL,0, NULL, SYNC_MODE);
        #endif
#ifdef __ENABLE_POWERMANAGER
        PowerManagerStart();
#endif
    }
}


/*******************************************************************************
** Name: DeleteDeviceListKey
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:11:51
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListKey(uint32 * list)
{
    KEY_DEV_ARG stKeyArg;
    rk_err_t ret;

    ret = RKDev_Delete(DEV_CLASS_KEY, 0, &stKeyArg);//&stKeyArg
    if (ret != RK_SUCCESS)
    {
        printf("KEY Delete FRIL\n");
    }
    else
    {
        RKDev_Close(stKeyArg.ADCHandler);
    }

    ret = RKDev_Delete(DEV_CLASS_ADC,0, NULL);
    if (ret != RK_SUCCESS)
    {
        printf("ADC Delete fail\n");
        return RK_ERROR;
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DeleteDeviceListAudioPlay
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:09:30
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListAudioPlay(uint32 * list)
{
    I2S_DEV_ARG stI2Sarg;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    AUDIO_DEV_ARG stAudioArg;
    rk_err_t ret = RK_SUCCESS;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    if(RKDev_Delete(DEV_CLASS_AUDIO, 0, &stAudioArg) != RK_SUCCESS)
    {
        rk_print_string("\naudio delete fail");
        ret = RK_ERROR;
    }
    else
    {
        RKDev_Close(stAudioArg.hCodec);
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_ROCKCODEC, 0, &stRockCodecDevArg) != RK_SUCCESS)
    {
        rk_print_string("\nacodec  delete fail");
        ret = RK_ERROR;
    }
    else
    {
        RKDev_Close(stRockCodecDevArg.hI2s);
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_I2S, I2S_DEV0, &stI2Sarg) != RK_SUCCESS)
    {
        rk_print_string("\n i2s device  delete fail");
        ret = RK_ERROR;
    }
    else
    {
        RKDev_Close(stI2Sarg.hDma);
        i--;
        list[i] = 0;
    }

    rk_printf("delet audio play device tree");
    return ret;

}

/*******************************************************************************
** Name: CreateDeviceListAudioPlay
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:38:41
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListAudioPlay(uint32 * list)
{

    I2S_DEV_ARG stI2Sarg;
    ROCKCODEC_DEV_ARG stRockCodecDevArg;
    AUDIO_DEV_ARG stAudioArg;
    HDC hCodec;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    stI2Sarg.hDma = RKDev_Open(DEV_CLASS_DMA, DMA_CHN0, NOT_CARE);
    if (stI2Sarg.hDma == NULL)
    {
        rk_print_string("\nDMA RKDev_Open failure");
        goto err;
    }

    stI2Sarg.Channel = I2S_SEL_ACODEC;

    stI2Sarg.req_arg.i2smode = I2S_SLAVE_MODE;
    stI2Sarg.req_arg.i2sCS = I2S_IN;
    stI2Sarg.req_arg.I2S_FS = I2S_FS_44100Hz;
    stI2Sarg.req_arg.BUS_FORMAT = I2S_FORMAT;
    stI2Sarg.req_arg.Data_width = I2S_DATA_WIDTH24;
    stI2Sarg.req_arg.I2S_Bus_mode = I2S_NORMAL_MODE;

    stI2Sarg.req_arg.RX_BUS_FORMAT = I2S_FORMAT;
    stI2Sarg.req_arg.Rx_Data_width = I2S_DATA_WIDTH16;
    stI2Sarg.req_arg.Rx_I2S_Bus_mode = I2S_RIGHT_MODE;

    //Create I2SgDev...
    ret=RKDev_Create(DEV_CLASS_I2S, I2S_DEV0, &stI2Sarg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nI2S RKDev_Create failure");
        RKDev_Close(stI2Sarg.hDma);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_I2S << 8) | 0;
    i++;


    stRockCodecDevArg.hI2s = RKDev_Open(DEV_CLASS_I2S, I2S_DEV0, NOT_CARE);
    stRockCodecDevArg.arg.DacFs = I2S_FS_44100Hz;
#ifndef _BROAD_LINE_OUT_
    stRockCodecDevArg.arg.DacMode  = Codec_DACoutHP;
#else
    stRockCodecDevArg.arg.DacMode  = Codec_DACoutLINE;
#endif
    stRockCodecDevArg.arg.AdcMode = Codec_Standby;
#ifdef CODEC_24BIT
    stRockCodecDevArg.arg.DacDataWidth = VDW_RX_WIDTH_24BIT;
#else
    stRockCodecDevArg.arg.DacDataWidth = VDW_RX_WIDTH_16BIT;
#endif
    ret = RKDev_Create(DEV_CLASS_ROCKCODEC, 0,&stRockCodecDevArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nopen failure");
        RKDev_Close(stRockCodecDevArg.hI2s);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_ROCKCODEC << 8) | 0;
    i++;

    //SetI2SFreq(I2S_DEV0, I2S_XIN12M, NULL);//12M

    hCodec = RKDev_Open(DEV_CLASS_ROCKCODEC,0,NOT_CARE);

    //hCodec = RKDev_Open(DEV_CLASS_WM8987, 0, NOT_CARE);

    if ((hCodec == NULL) || (hCodec == (HDC)RK_ERROR) || (hCodec == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\ncodec device open failure");

        goto err;
    }

    stAudioArg.Bit = I2S_DATA_WIDTH24;
    stAudioArg.hCodec = hCodec;
    stAudioArg.SampleRate = CodecFS_48KHz;//CodecFS_44100Hz;//CodecFS_16KHz;
    stAudioArg.Vol = 25;
#ifdef _RK_EQ_
    stAudioArg.EQMode = EQ_NOR;
#endif
    ret = RKDev_Create(DEV_CLASS_AUDIO, 0, &stAudioArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\naudio device create failure");
        RKDev_Close(hCodec);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_AUDIO << 8) | 0;
    i++;

    return RK_SUCCESS;

    err:
    return RK_ERROR;

}

#ifdef _FS_
/*******************************************************************************
** Name: DeleteDeviceListDir
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 17:09:46
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListDir(uint32 * list)
{
    rk_err_t ret;

    ret = RKDev_Delete(DEV_CLASS_DIR, 0, NULL);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("delete dir device failure");
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: CreateDeviceListDir
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 17:08:42
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListDir(uint32 * list)
{
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    ret = RKDev_Create(DEV_CLASS_DIR, 0, NULL);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("create dir device failure");
    }
    return RK_SUCCESS;

}
#endif

/*******************************************************************************
** Name: CreateDeviceListKey
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:40:12
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
COMMON API rk_err_t CreateDeviceListKey(uint32 * list)
{
    //ADC - KEY
    ADC_DEV_ARG stADCArg;
    KEY_DEV_ARG stKeyArg;
    HDC hKey;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    stADCArg.channel = ADC_CHANEL_KEY;
    stADCArg.size = 1;

    ret = RKDev_Create(DEV_CLASS_ADC,0, &stADCArg);
    if (ret != RK_SUCCESS)
    {
        printf("ADC CREATE fail\n");
        return RK_ERROR;
    }

    stKeyArg.ADCHandler = RKDev_Open(DEV_CLASS_ADC, 0, NOT_CARE);
    if (stKeyArg.ADCHandler == NULL)
    {
        printf("ADC OPEN FRIL\n");
        return RK_ERROR;
    }

    ret = RKDev_Create(DEV_CLASS_KEY, 0, &stKeyArg);//&stKeyArg
    if (ret != RK_SUCCESS)
    {
        printf("KEY CREATE FRIL\n");
        return RK_ERROR;
    }

    return RK_SUCCESS;

}

#ifdef _USE_GUI_
/*******************************************************************************
** Name: DeleteDeviceListDisplay
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.16
** Time: 17:22:24
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
COMMON API rk_err_t DeleteDeviceListDisplay(uint32 * list)
{
    DISPLAY_DEV_ARG stDisplayDevArg;
    LCD_DEV_ARG stLcdDevArg;
    VOP_DEV_ARG stVopDevArg;
    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    if(RKDev_Delete(DEV_CLASS_DISPLAY, 0, &stDisplayDevArg) != RK_SUCCESS)
    {
         rk_print_string("\nDelete Display0 failure");
    }
    else
    {
        RKDev_Close(stDisplayDevArg.hLcd);
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_DISPLAY, 1, &stDisplayDevArg) !=  RK_SUCCESS)
    {
        rk_print_string("\nDelete Display1 failure");
    }
    else
    {
        RKDev_Close(stDisplayDevArg.hLcd);
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_LCD, 0, &stLcdDevArg) != RK_SUCCESS)
    {
        rk_print_string("\nDelete lcd failure");
    }
    else
    {
        RKDev_Close(stLcdDevArg.hBus);
        RKDev_Close(stLcdDevArg.hPwm);
        i--;
        list[i] = 0;
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_VOP, 0, &stVopDevArg) != RK_SUCCESS)
    {
        rk_print_string("\nDelete vop failure");
    }
    else
    {
        RKDev_Close(stVopDevArg.hDma);
        i--;
        list[i] = 0;
    }

    if(RKDev_Delete(DEV_CLASS_PWM, 0, NULL) != RK_SUCCESS)
    {
         rk_print_string("\nDelete pwm failure");
    }

    rk_printf("delete display device list");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: CreateDeviceListDisplay
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:39:20
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListDisplay(uint32 * list)
{
    DISPLAY_DEV_ARG stDisplayDevArg;
    VOP_DEV_ARG stVopDevArg;
    LCD_DEV_ARG stLcdDevArg;
    PWM_DEV_ARG PwmDev;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    stVopDevArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    if (stVopDevArg.hDma <= 0)
    {
        rk_printf("\nhDma open failure");
        goto err;
    }

    ret= RKDev_Create(DEV_CLASS_VOP, 0, &stVopDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nCreate DEV_CLASS_VOP failure");
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 2) << 24) | ((i + 1) << 16) | (DEV_CLASS_VOP << 8) | 0;
    i++;

    // Test code. Release version should be created dynamically
    extern RK_LCD_DRIVER ST7735SDriver;
    stLcdDevArg.pLCDriver= &ST7735SDriver;

    stLcdDevArg.hBus = RKDev_Open(DEV_CLASS_VOP, 0, NULL);
    if (stLcdDevArg.hBus == NULL)
    {
        rk_print_string("\nOpen DEV_CLASS_VOP failure");
        goto err;
    }

    //Create PWMDev...
    PwmDev.channel = 0;
    ret = RKDev_Create(DEV_CLASS_PWM, 0, &PwmDev);
    if (ret != RK_SUCCESS)
    {
        RKDev_Close(stLcdDevArg.hBus);
        rk_print_string("\pwm 0 create failure");
        goto err;
    }

    stLcdDevArg.hPwm = RKDev_Open(DEV_CLASS_PWM, 0, NOT_CARE);
    if (stLcdDevArg.hPwm == NULL)
    {
        rk_printf("Open PWM 0 fail");
        goto err;
    }

    ret= RKDev_Create(DEV_CLASS_LCD, 0, &stLcdDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nCreate DEV_CLASS_LCD failure");
        RKDev_Close(stLcdDevArg.hBus);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PWM << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LCD << 8) | 0;
    i++;

    stDisplayDevArg.hLcd = RKDev_Open(DEV_CLASS_LCD, 0, NULL);
    if (stDisplayDevArg.hLcd == NULL)
    {
        rk_print_string("\nOpen DEV_CLASS_LCD failure");
        goto err;
    }

    ret= RKDev_Create(DEV_CLASS_DISPLAY, 0, &stDisplayDevArg);
    if (ret!= RK_SUCCESS)
    {
        rk_print_string("\nDisplay device0 create failure");
        RKDev_Close(stDisplayDevArg.hLcd);
        goto err;
    }

    list[i] = (DEV_CLASS_DISPLAY << 8) | 0;
    i++;

    stDisplayDevArg.hLcd = RKDev_Open(DEV_CLASS_LCD, 0, NULL);
    if(stDisplayDevArg.hLcd == NULL)
    {
        rk_print_string("\nOpen DEV_CLASS_LCD failure");
        goto err;
    }

    ret= RKDev_Create(DEV_CLASS_DISPLAY, 1, &stDisplayDevArg);
    if (ret!= RK_SUCCESS)
    {
        rk_print_string("\nDisplay device1 create failure");
        RKDev_Close(stDisplayDevArg.hLcd);
        goto err;
    }

    list[i] = (DEV_CLASS_DISPLAY << 8) | 1;
    i++;

    return RK_SUCCESS;

    err:
    return RK_ERROR;

}
#endif

#ifdef _SPI_BOOT_
/*******************************************************************************
** Name: DeleteDeviceListSpiBoot
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:16:53
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListSpiBoot(uint32 * list)
{
    HDC hSdc,hStorage;
    SPIFLASH_DEV_ARG stSpiArg;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_LUN, 0, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun0 device delete failure");
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_SPIFLASH, 0, &stSpiArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nspi device delete failure");
    }

    RKDev_Close(stSpiArg.hDma);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\ndma device delete failure");
    }

    rk_printf("delete spi boot list");

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DeleteDeviceListSpiDataBase
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:15:27
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListSpiDataBase(uint32 * list)
{
    HDC hStorage;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    ret = RKDev_Delete(DEV_CLASS_LUN, 1, &stLunArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN1 Delete Failure");
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;


    rk_printf("delete emmc database device list");

    return RK_SUCCESS;

}

#ifdef _FS_
/*******************************************************************************
** Name: DeleteDeviceListSpiFs3
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:26:01
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListSpiFs3(uint32 * list)
{
#ifdef _HIDDEN_DISK2_
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;
    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    stVolumeInf.VolumeID = 'B';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("remove B volume failure");
        return RK_ERROR;
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);
        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 2, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("FAT 2 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 2, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("par 2 delete failure");
        return RK_ERROR;
    }
    RKDev_Close(stCreateParArg.hLun);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_LUN, 4, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("lun 4 delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    i--;
    list[i] = 0;

    i--;
    list[i] = 0;

    rk_printf("delete spi fs3 device list");
    return RK_SUCCESS;
#else
    return RK_ERROR;
#endif
}
/*******************************************************************************
** Name: DeleteDeviceListSpiFs2
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:25:36
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListSpiFs2(uint32 * list)
{
#ifdef _HIDDEN_DISK1_
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;
    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    stVolumeInf.VolumeID = 'A';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("remove A volume failure");
        return RK_ERROR;
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);
        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 1, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("FAT 1 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 1, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("par 1 delete failure");
        return RK_ERROR;
    }
    RKDev_Close(stCreateParArg.hLun);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_LUN, 3, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("lun 3 delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    i--;
    list[i] = 0;

    i--;
    list[i] = 0;

    rk_printf("delete spi fs2 device list");
    return RK_SUCCESS;
#else
    return RK_ERROR;
#endif
}

/*******************************************************************************
** Name: DeleteDeviceListSpiFs1
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.12.31
** Time: 10:26:55
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListSpiFs1(uint32 * list)
{
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;

    SDC_DEV_ARG stSdcArg;
    SD_DEV_ARG stSdDevArg;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    stVolumeInf.VolumeID = 'C';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("remove C volume failure");
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);
        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 0, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("FAT 0 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 0, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("par0 delete failure");
    }
    else
    {
        RKDev_Close(stCreateParArg.hLun);
        i--;
        list[i] = 0;
    }

    ret = RKDev_Delete(DEV_CLASS_LUN, 2, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("lun2 delete failure");
    }
    else
    {
        RKDev_Close(stLunArg.hStorage);
        i--;
        list[i] = 0;
    }

    ret = RKDev_Delete(DEV_CLASS_SD, 0, &stSdDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nSD device delete failure");
    }
    else
    {
        RKDev_Close(stSdDevArg.hSdc);
        i--;
        list[i] = 0;
    }
    ret = RKDev_Delete(DEV_CLASS_SDC, 0, &stSdcArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsdc device delete failure");
    }
    else
    {
        RKDev_Close(stSdcArg.hDma);
        i--;
        list[i] = 0;
    }
    rk_printf("delete spi fs1 device list");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: CreateDeviceListSpiFs3
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:35:00
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListSpiFs3(uint32 * list)
{
#ifndef USE_SPI_STORAGE
#ifdef _HIDDEN_DISK1_
#ifdef __DRIVER_SD_SDDEVICE_C__
    HDC hStorage, hLun, hPar, hSpiFlash,hDma;
    SPIFLASH_DEV_ARG stSpiArg;
    LUN_DEV_ARG stLunArg;
    SDC_DEV_ARG stSdcArg;
    SD_DEV_ARG stSdDevArg;

    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    hStorage = RKDev_Open(DEV_CLASS_SD,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nsd open failure");
        goto err;
    }

    ret = SdDev_GetSize(hStorage, &stLunArg.dwEndLBA);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsd get size failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = stLunArg.dwEndLBA - (SPI_LUN4_SIZE << 11);
    stLunArg.dwEndLBA = stLunArg.dwEndLBA - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = SdDev_Read;
    stLunArg.pfStorgeWrite = SdDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN4 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SDC << 8) | 0;
    i++;


    list[i] = ((i + 1) << 24) | (DEV_CLASS_SD << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 4;
    i++;


    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("lun3 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun4 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 2, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("par2 create failure");
        RKDev_Close(hLun);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 2;
    i++;

    hPar = RKDev_Open(DEV_CLASS_PAR, 2, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("par2 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 2, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        if (ret != RK_SUCCESS)
        {
            rk_print_string("fat2 create failure");
            RKDev_Close(hPar);
            goto err;
        }
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 2;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 2, NOT_CARE);

    stVolumeInf.VolumeID = 'B';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'B', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSB", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'B', "RKOSB");
            }
        }
    }

    return RK_SUCCESS;
err:
    return RK_ERROR;

#else
    return RK_ERROR;
#endif
#else
    return RK_ERROR;
#endif
#endif
}

/*******************************************************************************
** Name: CreateDeviceListSpiFs2
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:24:36
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListSpiFs2(uint32 * list)
{
#ifndef USE_SPI_STORAGE
#ifdef _HIDDEN_DISK1_
#ifdef __DRIVER_SD_SDDEVICE_C__
    HDC hStorage, hLun, hPar, hSpiFlash,hDma;
    SPIFLASH_DEV_ARG stSpiArg;
    LUN_DEV_ARG stLunArg;
    SDC_DEV_ARG stSdcArg;
    SD_DEV_ARG stSdDevArg;

    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    hStorage = RKDev_Open(DEV_CLASS_SD,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nsd open failure");
        goto err;
    }

    ret = SdDev_GetSize(hStorage, &stLunArg.dwEndLBA);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsd get size failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = stLunArg.dwEndLBA - ((SPI_LUN3_SIZE + SPI_LUN4_SIZE) << 11);
    stLunArg.dwEndLBA = stLunArg.dwEndLBA - (SPI_LUN4_SIZE << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = SdDev_Read;
    stLunArg.pfStorgeWrite = SdDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 3, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN3 Create Failure");
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SDC << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SD << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 3;
    i++;

    hLun = RKDev_Open(DEV_CLASS_LUN, 3, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("lun3 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun3 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 1, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("par1 create failure");
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 1;
    i++;

    hPar = RKDev_Open(DEV_CLASS_PAR, 1, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("par1 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 1, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("fat1 create failure");
        RKDev_Close(hPar);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 1;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 1, NOT_CARE);

    stVolumeInf.VolumeID = 'A';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'A', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSA", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'A', "RKOSA");
            }
        }
    }


    return RK_SUCCESS;
err:
    return RK_ERROR;

#else
    return RK_ERROR;
#endif
#else
    return RK_ERROR;
#endif
#endif
}
/*******************************************************************************
** Name: CreateDeviceListSpiFs1
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:33:48
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListSpiFs1(uint32 * list)
{
#ifndef USE_SPI_STORAGE
#ifdef __DRIVER_SD_SDDEVICE_C__
    HDC hStorage, hLun, hPar, hSpiFlash,hDma;
    SPIFLASH_DEV_ARG stSpiArg;
    LUN_DEV_ARG stLunArg;
    SDC_DEV_ARG stSdcArg;
    SD_DEV_ARG stSdDevArg;

    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    stSdcArg.Channel = 0;
    stSdcArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    if (stSdcArg.hDma <= 0)
    {
        rk_printf("\nhDma open failure");
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_SDC, 0, &stSdcArg);
    if (ret != RK_SUCCESS)
    {
        rk_printf("\nsdc0 open failure");
        RKDev_Close(stSdcArg.hDma);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SDC << 8) | 0;
    i++;

    stSdDevArg.hSdc = RKDev_Open(DEV_CLASS_SDC, 0, NOT_CARE);
    if ((stSdDevArg.hSdc == NULL) || (stSdDevArg.hSdc == (HDC)RK_ERROR) || (stSdDevArg.hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nSDC0 open failure");
        goto err;
    }

    stSdDevArg.BusWidth = BUS_WIDTH_4_BIT;

    //Create SdDev...
    ret = RKDev_Create(DEV_CLASS_SD, 0, &stSdDevArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nSdDev create failure");
        RKDev_Close(stSdDevArg.hSdc);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SD << 8) | 0;
    i++;

    hStorage = RKDev_Open(DEV_CLASS_SD,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nsd open failure");
        goto err;
    }

    ret = SdDev_GetSize(hStorage, &stLunArg.dwEndLBA);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsd get size failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = 0;
    stLunArg.dwEndLBA = stLunArg.dwEndLBA - ((SPI_LUN3_SIZE + SPI_LUN4_SIZE) << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = SdDev_Read;
    stLunArg.pfStorgeWrite = SdDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 2, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN2 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 2;
    i++;

    hLun = RKDev_Open(DEV_CLASS_LUN, 2, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("lun2 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun2 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("par0 create failure");
        RKDev_Close(hLun);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 0;
    i++;


    hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("par0 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 0, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("fat0 create failure");
        RKDev_Close(hPar);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 0;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);

    stVolumeInf.VolumeID = 'C';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'C', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSC", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'C', "RKOSC");
            }
        }
    }

#else
    return RK_ERROR;
#endif
#else
    HDC hStorage, hLun, hPar, hSpiFlash,hDma;
    LUN_DEV_ARG stLunArg;

    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    hSpiFlash = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);

    if ((hSpiFlash == NULL) || (hSpiFlash == (HDC)RK_ERROR) || (hSpiFlash == (HDC)RK_PARA_ERR))
    {
        goto err;
    }

    ret = SpiFlashDev_GetSize(hSpiFlash, &StorageSise);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nspi get size failure");
        RKDev_Close(hSpiFlash);
        goto err;
    }

    stLunArg.dwStartLBA = (SPI_LUN0_SIZE + SPI_LUN1_SIZE) << 11;
    stLunArg.dwEndLBA = StorageSise - SPI_LUN3_SIZE - SPI_LUN4_SIZE - 1;
    stLunArg.hStorage = hSpiFlash;
    stLunArg.pfStorgeRead = SpiFlashDev_Read;
    stLunArg.pfStorgeWrite = SpiFlashDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 2, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN2 Create Failure");
        RKDev_Close(hSpiFlash);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SPIFLASH << 8) | 0;
    i++;
    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 2;
    i++;

    hLun = RKDev_Open(DEV_CLASS_LUN, 2, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("lun2 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun2 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("par0 create failure");
        RKDev_Close(hLun);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 0;
    i++;

    hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("par0 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 0, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("fat0 create failure");
        RKDev_Close(hPar);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 0;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);

    stVolumeInf.VolumeID = 'C';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'C', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSC", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'C', "RKOSC");
            }
        }
    }
#endif

    return RK_SUCCESS;
err:
    return RK_ERROR;

}

#endif


/*******************************************************************************
** Name: CreateDeviceListSpiDataBase
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:33:24
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListSpiDataBase(uint32 * list)
{
    HDC hStorage;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    hStorage = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nspiflash open failure");
        goto err;
    }

    stLunArg.dwStartLBA = (SPI_LUN0_SIZE) << 11;

    stLunArg.dwEndLBA = ((SPI_LUN0_SIZE + SPI_LUN1_SIZE) << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = SpiFlashDev_Read;
    stLunArg.pfStorgeWrite = SpiFlashDev_Write;
    stLunArg.dwSupportPar = 0;

    ret = RKDev_Create(DEV_CLASS_LUN, 1, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN1 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SPIFLASH << 8) | 0;
    i++;
    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 1;
    i++;

    return RK_SUCCESS;

    err:

    return RK_ERROR;

}

/*******************************************************************************
** Name: CreateDeviceListSpiBoot
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:32:32
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListSpiBoot(uint32 * list)
{
#if 0
    SPI_DEV_ARG stSpiArg;
    HDC hSpi,hStorage;
    SPINOR_DEV_ARG stSpiNorArg;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    rk_print_string("\r\ndma device init...");

    ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        goto err;
    }



    stSpiArg.Ch = 0;
    stSpiArg.CtrlMode = SPI_CTL_MODE;
    stSpiArg.SpiRate = SPI_BUS_CLK;


    rk_print_string("\r\nspi device init...");

    ret = RKDev_Create(DEV_CLASS_SPI, 0, &stSpiArg);
    if (ret != RK_SUCCESS)
    {
        goto err;
    }

    rk_print_string("\r\nspinor device init...");

    hSpi = RKDev_Open(DEV_CLASS_SPI, 0, NOT_CARE);

    if ((hSpi == NULL) || (hSpi == (HDC)RK_ERROR) || (hSpi == (HDC)RK_PARA_ERR))
    {
        goto err;
    }

    stSpiNorArg.hSpi = hSpi;
    stSpiNorArg.SpiChannel = 0;

    ret = RKDev_Create(DEV_CLASS_SPINOR, 0, &stSpiNorArg);

    if (ret != RK_SUCCESS)
    {
        goto err;
    }


    rk_print_string("\r\nlun0 device init...");


    hStorage = RKDev_Open(DEV_CLASS_SPINOR,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        goto err;
    }

    ret = SpiNorDev_GetSize(hStorage, &stLunArg.dwEndLBA);

    if (ret != RK_SUCCESS)
    {
        goto err;
    }

    stLunArg.dwStartLBA = 0;
    stLunArg.dwEndLBA--;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = SpiNorDev_Read;
    stLunArg.pfStorgeWrite = SpiNorDev_Write;

    ret = RKDev_Create(DEV_CLASS_LUN, 0, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        goto err;
    }
#endif
    SPIFLASH_DEV_ARG stSpiArg;
    HDC hSpiFlash;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;
    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;


    ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\ndma device create failure");
        goto err;
    }

    stSpiArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    stSpiArg.spirate = SPI_BUS_CLK;
    if (stSpiArg.hDma <= 0)
    {
        rk_print_string("\ndma device open failure");
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_SPIFLASH, 0, &stSpiArg);
    if (ret != RK_SUCCESS)
    {
        goto err;
    }


    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_SPIFLASH << 8) | 0;
    i++;

    hSpiFlash = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);

    if ((hSpiFlash == NULL) || (hSpiFlash == (HDC)RK_ERROR) || (hSpiFlash == (HDC)RK_PARA_ERR))
    {
        goto err;
    }

    stLunArg.dwStartLBA = 0;
    stLunArg.dwEndLBA = ((SPI_LUN0_SIZE) << 11) - 1;
    stLunArg.hStorage = hSpiFlash;
    stLunArg.pfStorgeRead = SpiFlashDev_Read;
    stLunArg.pfStorgeWrite = SpiFlashDev_Write;
    stLunArg.dwSupportPar = 0;

    ret = RKDev_Create(DEV_CLASS_LUN, 0, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        RKDev_Close(hSpiFlash);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 0;
    i++;

    return RK_SUCCESS;

    err:
    return RK_ERROR;

}
#endif

#ifdef _EMMC_BOOT_
/*******************************************************************************
** Name: DeleteDeviceListEmmcBoot
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:19:49
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListEmmcBoot(uint32 * list)
{
    SDC_DEV_ARG stSdcArg;
    HDC hSdc,hStorage;
    EMMC_DEV_ARG stEmmcArg;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    ret = RKDev_Delete(DEV_CLASS_LUN, 0, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun0 device delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_EMMC, 0, &stEmmcArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc device delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stEmmcArg.hSdc);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_SDC, EMMC_SDC_DEV_ID, &stSdcArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsdc device delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stSdcArg.hDma);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\ndma device delete failure");
    }

    rk_printf("delete emmc boot list");

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DeleteDeviceListEmmcDataBase
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:18:50
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListEmmcDataBase(uint32 * list)
{
    HDC hStorage;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_LUN, 1, &stLunArg);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN1 Delete Failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);

    i--;
    list[i] = 0;

    rk_printf("delete emmc database device list");

    return RK_SUCCESS;

}

#ifdef _FS_
/*******************************************************************************
** Name: DeleteDeviceListEmmcFs3
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:29:36
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListEmmcFs3(uint32 * list)
{
#ifdef _HIDDEN_DISK1_
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    stVolumeInf.VolumeID = 'B';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("remove B volume failure");
        return RK_ERROR;
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);
        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 2, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("FAT 2 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 2, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("par 2  delete failure");
        return RK_ERROR;
    }
    RKDev_Close(stCreateParArg.hLun);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_LUN, 4, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("lun4 delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    rk_printf("delete emmc fs3 device list");
    return RK_SUCCESS;
#else
    return RK_ERROR;
#endif
}
/*******************************************************************************
** Name: DeleteDeviceListEmmcFs2
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:29:01
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListEmmcFs2(uint32 * list)
{
#ifdef _HIDDEN_DISK1_
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;

    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }


    stVolumeInf.VolumeID = 'A';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("\nremove A volume failure");
        return RK_ERROR;
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);
        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 1, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("\nFAT 1 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 1, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("\npar1 delete failure");
        return RK_ERROR;
    }
    RKDev_Close(stCreateParArg.hLun);
    i--;
    list[i] = 0;

    ret = RKDev_Delete(DEV_CLASS_LUN, 3, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("\nlun3 delete failure");
        return RK_ERROR;
    }

    RKDev_Close(stLunArg.hStorage);
    i--;
    list[i] = 0;

    rk_printf("\ndelete emmc fs2 device list");
    return RK_SUCCESS;
#else
    return RK_ERROR;
#endif
}

/*******************************************************************************
** Name: DeleteDeviceListEmmcFs1
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.11
** Time: 18:17:58
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t DeleteDeviceListEmmcFs1(uint32 * list)
{
    VOLUME_INF stVolumeInf;
    rk_err_t ret;
    FAT_DEVICE_ARG stCreateFatArg;
    PAR_DEVICE_ARG stCreateParArg;
    LUN_DEV_ARG stLunArg;
    uint32 i;

    for(i = 0 ; i < (MAX_DEV_PER_LIST + 1); i++)
    {
        if(list[i] == 0)
        {
            break;
        }
    }

    stVolumeInf.VolumeID = 'C';
    ret = FileDev_DeleteVolume(FileSysHDC, &stVolumeInf);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("remove C volume failure");
    }
    else
    {
        RKDev_Close(stVolumeInf.hVolume);

        i--;
        list[i] = 0;

        if(stVolumeInf.VolumeType == VOLUME_TYPE_FAT)
        {
            ret = RKDev_Delete(DEV_CLASS_FAT, 0, &stCreateFatArg);
            if(ret != RK_SUCCESS)
            {
                rk_print_string("FAT 0 delete failure");
                return RK_ERROR;
            }

            RKDev_Close(stCreateFatArg.hPar);
            i--;
            list[i] = 0;
        }
    }

    ret = RKDev_Delete(DEV_CLASS_PAR, 0, &stCreateParArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("par0 delete failure");
    }
    else
    {
        RKDev_Close(stCreateParArg.hLun);
        i--;
        list[i] = 0;
    }
    ret = RKDev_Delete(DEV_CLASS_LUN, 2, &stLunArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("lun2 delete failure");
    }
    else
    {
        RKDev_Close(stLunArg.hStorage);
        i--;
        list[i] = 0;
    }
    rk_printf("delete emmc fs1 device list");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: CreateDeviceListEmmcFs3
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:31:33
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListEmmcFs3(uint32 * list)
{

#ifdef _HIDDEN_DISK2_
    HDC hStorage, hLun, hPar;
    LUN_DEV_ARG stLunArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;
    uint32 i;
    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    StorageSise = 0;

    hStorage = RKDev_Open(DEV_CLASS_EMMC,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nemmc open failure");
        goto err;
    }


    ret = EmmcDev_GetAreaSize(hStorage, EMMC_AREA_USER, &StorageSise);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc get user area size failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = StorageSise - (EMMC_LUN4_SIZE << 11);
    stLunArg.dwEndLBA = StorageSise - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = EmmcDev_Read;
    stLunArg.pfStorgeWrite = EmmcDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 4, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN4 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }


    list[i] = ((i + 1) << 24) | (DEV_CLASS_EMMC << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 4;
    i++;


    hLun = RKDev_Open(DEV_CLASS_LUN, 4, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nlun4 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun4 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 2, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\npar2 create failure");
        RKDev_Close(hLun);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 2;
    i++;


    hPar = RKDev_Open(DEV_CLASS_PAR, 2, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\npar2 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 2, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        if (ret != RK_SUCCESS)
        {
            rk_print_string("\nfat2 create failure");
            RKDev_Close(hPar);
            goto err;
        }
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 2;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 2, NOT_CARE);

    stVolumeInf.VolumeID = 'B';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nadd volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'B', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSB", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'B', "RKOSB");
            }
        }
    }

    return RK_SUCCESS;

    err:
    return RK_ERROR;
#else
    return RK_ERROR;
#endif

}

/*******************************************************************************
** Name: CreateDeviceListEmmcFs2
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:30:59
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListEmmcFs2(uint32 * list)
{

#ifdef _HIDDEN_DISK1_
    HDC hStorage, hLun, hPar;
    LUN_DEV_ARG stLunArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;
    uint32 i;
    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    StorageSise = 0;

    hStorage = RKDev_Open(DEV_CLASS_EMMC,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nemmc open failure");
        goto err;
    }


    ret = EmmcDev_GetAreaSize(hStorage, EMMC_AREA_USER, &StorageSise);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc get user area size failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = StorageSise - ((EMMC_LUN3_SIZE + EMMC_LUN4_SIZE) << 11);
    stLunArg.dwEndLBA = StorageSise - (EMMC_LUN4_SIZE << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = EmmcDev_Read;
    stLunArg.pfStorgeWrite = EmmcDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 3, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN3 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_EMMC << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 3;
    i++;


    hLun = RKDev_Open(DEV_CLASS_LUN, 3, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nlun3 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun3 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 1, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\npar1 create failure");
        RKDev_Close(hLun);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 1;
    i++;

    hPar = RKDev_Open(DEV_CLASS_PAR, 1, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\npar1 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 1, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nfat1 create failure");
        RKDev_Close(hPar);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 1;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 1, NOT_CARE);

    stVolumeInf.VolumeID = 'A';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nadd volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE<< 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'A', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSA", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'A', "RKOSA");
            }
        }
    }

    return RK_SUCCESS;

    err:
    return RK_ERROR;
#else
    return RK_ERROR;
#endif

}

/*******************************************************************************
** Name: CreateDeviceListEmmcFs1
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:29:54
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListEmmcFs1(uint32 * list)
{

    HDC hStorage, hLun, hPar;
    LUN_DEV_ARG stLunArg;
    DEVICE_CLASS * pDev;
    rk_err_t ret;
    uint32 StorageSise;
    PAR_DEVICE_ARG stCreateParArg;
    FAT_DEVICE_ARG stCreateFatArg;
    VOLUME_INF stVolumeInf;
    uint32 i;
    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);
    i = 0;

    StorageSise = 0;

    hStorage = RKDev_Open(DEV_CLASS_EMMC,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nemmc open failure");
        goto err;
    }


    ret = EmmcDev_GetAreaSize(hStorage, EMMC_AREA_USER, &StorageSise);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc get user area size failure");
        RKDev_Close(hStorage);
        goto err;
    }


    stLunArg.dwStartLBA = (EMMC_LUN0_SIZE + EMMC_LUN1_SIZE) << 11;

    stLunArg.dwEndLBA = StorageSise - ((EMMC_LUN3_SIZE + EMMC_LUN4_SIZE) << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = EmmcDev_Read;
    stLunArg.pfStorgeWrite = EmmcDev_Write;
    stLunArg.dwSupportPar = 1;

    ret = RKDev_Create(DEV_CLASS_LUN, 2, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN2 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_EMMC << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 2;
    i++;


    hLun = RKDev_Open(DEV_CLASS_LUN, 2, NOT_CARE);

    if ((hLun == NULL) || (hLun == (HDC)RK_ERROR) || (hLun == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nlun2 open failure");
        goto err;
    }

    stCreateParArg.hLun = hLun;

    ret = LunDev_GetPar(hLun, 0,&stCreateParArg.VolumeType, &stCreateParArg.ParStartLBA,&stCreateParArg.ParTotalSec);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun2 get par failure");
        RKDev_Close(hLun);
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_PAR, 0, &stCreateParArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\npar0 create failure");
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_PAR << 8) | 0;
    i++;

    hPar = RKDev_Open(DEV_CLASS_PAR, 0, NOT_CARE);
    if ((hPar == NULL) || (hPar == (HDC)RK_ERROR) || (hPar == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\npar0 open failure");
        goto err;
    }

    stCreateFatArg.hPar = hPar;

    ret = RKDev_Create(DEV_CLASS_FAT, 0, &stCreateFatArg);

    if (ret != RK_SUCCESS)
    {
        if (ret != RK_SUCCESS)
        {
            rk_print_string("fat0 create failure");
            RKDev_Close(hPar);
            return RK_ERROR;
        }
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FAT << 8) | 0;
    i++;

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);

    stVolumeInf.VolumeID = 'C';
    stVolumeInf.VolumeType = VOLUME_TYPE_FAT;

    ret = FileDev_AddVolume(FileSysHDC, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        RKDev_Close(stVolumeInf.hVolume);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_FILE << 8) | 0;
    i++;

    {
        uint8 name[13];
        ret = FileDev_ReadVolumeName(FileSysHDC, 'C', name);
        if(ret == RK_SUCCESS)
        {
            if(StrCmpA(name, "RKOSC", 6))
            {
                FileDev_WriteVolumeName(FileSysHDC, 'C', "RKOSC");
            }
        }
    }

    return RK_SUCCESS;

    err:
    return RK_ERROR;

}
#endif

/*******************************************************************************
** Name: CreateDeviceListEmmcDataBase
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:28:50
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListEmmcDataBase(uint32 * list)
{
    HDC hStorage;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;

    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);

    i = 0;


    hStorage = RKDev_Open(DEV_CLASS_EMMC,0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nemmc open failure");
        goto err;
    }

    stLunArg.dwStartLBA = (EMMC_LUN0_SIZE) << 11;


    stLunArg.dwEndLBA = ((EMMC_LUN0_SIZE + EMMC_LUN1_SIZE) << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = EmmcDev_Read;
    stLunArg.pfStorgeWrite = EmmcDev_Write;
    stLunArg.dwSupportPar = 0;

    ret = RKDev_Create(DEV_CLASS_LUN, 1, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nLUN1 Create Failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_EMMC << 8) | 0;
    i++;

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 1;
    i++;

    return RK_SUCCESS;

    err:

    return RK_ERROR;

}
/*******************************************************************************
** Name: CreateDeviceListEmmcBoot
** Input:uint32 * list
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.19
** Time: 11:18:16
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API rk_err_t CreateDeviceListEmmcBoot(uint32 * list)
{
    SDC_DEV_ARG stSdcArg;
    HDC hSdc,hStorage;
    EMMC_DEV_ARG stEmmcArg;
    LUN_DEV_ARG stLunArg;
    rk_err_t ret;
    uint32 i;

    memset((uint8 *)list, 0, (MAX_DEV_PER_LIST + 1) * 4);

    i = 0;

    rk_print_string("\ndma device init...");

    ret = RKDev_Create(DEV_CLASS_DMA, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\ndma device create failure");
        goto err;
    }

    rk_print_string("\nsdmmc device init...");

    stSdcArg.hDma = RKDev_Open(DEV_CLASS_DMA, 0, NOT_CARE);
    stSdcArg.Channel = EMMC_SDC_CHANNEL;
    if (stSdcArg.hDma <= 0)
    {
        rk_print_string("\ndma device open failure");
        goto err;
    }

    ret = RKDev_Create(DEV_CLASS_SDC, EMMC_SDC_DEV_ID, &stSdcArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nsdc device device failure");
        RKDev_Close(stSdcArg.hDma);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_DMA << 8) | 0;
    i++;
    list[i] = ((i + 1) << 24) | (DEV_CLASS_SDC << 8) | 0;
    i++;

    rk_print_string("\nemmc device init...");

    hSdc = RKDev_Open(DEV_CLASS_SDC, 0, NOT_CARE);

    if ((hSdc == NULL) || (hSdc == (HDC)RK_ERROR) || (hSdc == (HDC)RK_PARA_ERR))
    {
        rk_print_string("\nsdc device open failure");
        goto err;
    }


    stEmmcArg.BusWidth = EMMC_DEV0_BUS_WIDTH;
    stEmmcArg.hSdc = hSdc;


    ret = RKDev_Create(DEV_CLASS_EMMC, 0, &stEmmcArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc device create failure");
        RKDev_Close(hSdc);
        goto err;
    }

    list[i] = (DEV_CLASS_EMMC << 8) | 0;
    i++;


    rk_print_string("\nlun0 device init...");


    hStorage = RKDev_Open(DEV_CLASS_EMMC, 0, NOT_CARE);
    if ((hStorage == NULL) || (hStorage == (HDC)RK_ERROR) || (hStorage == (HDC)RK_PARA_ERR))
    {

        rk_print_string("\nemmc device open failure");
        goto err;
    }

    ret = EmmcDev_SetArea(hStorage, EMMC_AREA_USER);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nemmc device set user failure");
        RKDev_Close(hStorage);
        goto err;
    }

    stLunArg.dwStartLBA = 0;
    stLunArg.dwEndLBA = ((EMMC_LUN0_SIZE) << 11) - 1;
    stLunArg.hStorage = hStorage;
    stLunArg.pfStorgeRead = EmmcDev_Read;
    stLunArg.pfStorgeWrite = EmmcDev_Write;
    stLunArg.dwSupportPar = 0;

    ret = RKDev_Create(DEV_CLASS_LUN, 0, &stLunArg);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("\nlun0 device create failure");
        RKDev_Close(hStorage);
        goto err;
    }

    list[i] = ((i + 1) << 24) | (DEV_CLASS_LUN << 8) | 0;
    i++;

    return RK_SUCCESS;

    err:

    return RK_ERROR;

}
#endif

#ifdef __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__
/*******************************************************************************
** Name: RockCodecDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.12.30
** Time: 10:50:20
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
INIT API void RockCodecDevHwInit(uint32 DevID, uint32 Channel)
{
    //ACODEC CLOCK gate open
    //G0_14 and G9_16
    //printf("Codec_PowerOnInitial\n");
    ScuClockGateCtr(CLK_ACODEC_GATE, 1);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 1);     //PCLK ACODEC gating open
    ScuSoftResetCtr(ACODEC_SRST0, 0);
    SetAcodecFreq();

    #ifdef HP_DET_CONFIG
    Grf_GpioMuxSet(GPIO_CH2,HP_DET,IOMUX_GPIO2B3_IO);
    Gpio_SetPinDirection(GPIO_CH2,HP_DET,GPIO_IN);
    Grf_GPIO_SetPinPull(GPIO_CH2,HP_DET,DISABLE);
    #endif
}

/*******************************************************************************
** Name: RockCodecDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.12.30
** Time: 10:50:20
*******************************************************************************/
_DRIVER_ROCKCODEC_ROCKCODECDEVICE_INIT_
INIT API void RockCodecDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //ACODEC CLOCK gate open
    //G0_14 and G9_16
    //printf("Codec_PowerOnInitial\n");
    ScuSoftResetCtr(ACODEC_SRST0, 1);
    ScuClockGateCtr(CLK_ACODEC_GATE, 0);      //ACODEC gating open
    ScuClockGateCtr(PCLK_ACODEC_GATE, 0);     //PCLK ACODEC gating open

    #ifdef HP_DET_CONFIG
    Grf_GpioMuxSet(GPIO_CH2,HP_DET,IOMUX_GPIO2B3_IO);
    Gpio_SetPinDirection(GPIO_CH2,HP_DET,GPIO_IN);
    Grf_GPIO_SetPinPull(GPIO_CH2,HP_DET,DISABLE);
    #endif
}


#endif


#ifdef __DRIVER_DMA_DMADEVICE_C__
/*******************************************************************************
** Name: DmaDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.12.30
** Time: 10:50:20
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT API void DmaDevHwInit(uint32 DevID, uint32 Channel)
{
    //open dma1 clk
    ScuClockGateCtr(HCLK_DMA_GATE, 1);
    //open rst dma1 ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(SYSDMA_SRST, 0);

    IntRegister(INT_ID_REV0,DmaSoftIntIsr);
    IntPendingClear(INT_ID_REV0);
    IntEnable(INT_ID_REV0);

    IntRegister(INT_ID_DMA ,DmaIntIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
}

/*******************************************************************************
** Name: DmaDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2016.4.13
** Time: 8:56:55
*******************************************************************************/
_DRIVER_DMA_DMADEVICE_INIT_
INIT API void DmaDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //open dma1 clk
    ScuClockGateCtr(HCLK_DMA_GATE, 0);
    //open rst dma1 ip
    ScuSoftResetCtr(SYSDMA_SRST, 1);

    IntRegister(INT_ID_REV0,DmaSoftIntIsr);
    IntPendingClear(INT_ID_REV0);
    IntEnable(INT_ID_REV0);

    IntRegister(INT_ID_DMA ,DmaIntIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
}

#endif



//#ifdef __DRIVER_LCD_LCDDEVICE_C__
#ifdef __DRIVER_VOP_VOPDEVICE_C__
/*******************************************************************************
** Name: VopDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2015.6.26
** Time: 13:38:16
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT API void VopDevHwInit(uint32 DevID, uint32 Channel)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin0,IOMUX_GPIO0C0_VOP_D0);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin1,IOMUX_GPIO0C0_VOP_D1);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin2,IOMUX_GPIO0C0_VOP_D2);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin3,IOMUX_GPIO0C0_VOP_D3);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C0_VOP_D4);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C0_VOP_D5);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C0_VOP_D6);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C0_VOP_D7);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0C0_VOP_WRN);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0C0_VOP_RS);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0C0_VOP_CSN);
    ScuClockGateCtr(HCLK_VOP_GATE, 1);
    ScuSoftResetCtr(VOP_SRST, 1);
    DelayMs(1);
    ScuSoftResetCtr(VOP_SRST, 0);
}

/*******************************************************************************
** Name: VopDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2015.6.26
** Time: 13:38:16
*******************************************************************************/
_DRIVER_VOP_VOPDEVICE_INIT_
INIT API void VopDevHwDeInit(uint32 DevID, uint32 Channel)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin0,IOMUX_GPIO0C0_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin1,IOMUX_GPIO0C1_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin2,IOMUX_GPIO0C2_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin3,IOMUX_GPIO0C3_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0B7_IO);
    ScuClockGateCtr(HCLK_VOP_GATE, 0);
    ScuSoftResetCtr(VOP_SRST, 1);
}
#endif

#ifdef __DRIVER_TIMER_TIMERDEVICE_C__
/*******************************************************************************
** Name: TimerDevHwInit
** Input:HDC dev
** Return: rk_err_t
** Owner:cjh
** Date: 2015.6.2
** Time: 15:32:43
*******************************************************************************/
_DRIVER_TIMER_TIMERDEVICE_INIT_
INIT API void TimerDevHwInit(uint32 DevID, uint32 Channel)
{
    //rk_printf ("Timer%d HardWare Init\n",DevID);
    if (DevID == 0)
    {
        //Timer0
        //rk_printf ("Timer%d reset\n",DevID);
        ScuClockGateCtr( CLK_TIMER0_GATE, 1);
        ScuClockGateCtr( PCLK_TIMER_GATE, 1);

        ScuSoftResetCtr(TIMER0_SRST, 0);
        SetTimerFreq(TIMER0,XIN24M,24*1000*1000);
    }
    else if (DevID == 1)
    {
        //Timer1
        ScuClockGateCtr( CLK_TIMER0_GATE, 1);
        ScuClockGateCtr( PCLK_TIMER_GATE, 1);

        ScuSoftResetCtr(TIMER1_SRST, 0);
        SetTimerFreq(TIMER1,XIN24M,24*1000*1000);
    }

    if (DevID == 0)
    {
        //printf("timer0 inttrupt init\n");
        IntUnregister(INT_ID_TIMER0);
        IntPendingClear(INT_ID_TIMER0);
        IntDisable(INT_ID_TIMER0);

        IntRegister(INT_ID_TIMER0, TimerIntIsr0);
        IntPendingClear(INT_ID_TIMER0);
        IntEnable(INT_ID_TIMER0);

        TimerStop(TIMER0);
        TimerInit(TIMER0,TIMER_FREE_RUNNING);
        TimerSetCount(TIMER0,(UINT64)24*1000*1000);
        TimerIntUnmask(TIMER0);
    }
    else if (DevID == 1)
    {
        IntUnregister(INT_ID_TIMER1);
        IntPendingClear(INT_ID_TIMER1);
        IntDisable(INT_ID_TIMER1);

        IntRegister(INT_ID_TIMER1, TimerIntIsr1);
        IntPendingClear(INT_ID_TIMER1);
        IntEnable(INT_ID_TIMER1);

        TimerStop(TIMER1);
        TimerInit(TIMER1,TIMER_FREE_RUNNING);
        TimerSetCount(TIMER1,(UINT64)24*1000*1000);
        TimerIntUnmask(TIMER1);
    }
}

#endif


#ifdef __DRIVER_BCORE_BCOREDEVICE_C__
/*******************************************************************************
** Name: BcoreDeHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:27:18
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API void BcoreHwDeInit(uint32 DevID, uint32 Channel)
{
    IntDisable(INT_ID_MAILBOX0);

    MailBoxDisableB2AInt(MAILBOX_ID_0, MAILBOX_INT_0);

    IntPendingClear(MAILBOX_ID_0);

    IntUnregister(INT_ID_MAILBOX0);

    ScuClockGateCtr(PCLK_MAILBOX_GATE,0);
    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,0);
    ScuClockGateCtr(HDRAM0_GATE,0);
    ScuClockGateCtr(HDRAM1_GATE,0);
    ScuClockGateCtr(HDRAM2_GATE,0);
    ScuClockGateCtr(HDRAM3_GATE,0);
    ScuClockGateCtr(HDRAM4_GATE,0);
    ScuClockGateCtr(HDRAM5_GATE,0);
    ScuClockGateCtr(HDRAM6_GATE,0);
    ScuClockGateCtr(HDRAM7_GATE,0);
    ScuClockGateCtr(HDRAM_INTERFACE_GATE,0);
    ScuClockGateCtr(HIRAM0_GATE,0);
    ScuClockGateCtr(HIRAM1_GATE,0);
    ScuClockGateCtr(HIRAM2_GATE,0);
    ScuClockGateCtr(HIRAM3_GATE,0);
    ScuClockGateCtr(HIRAM_INTERFACE_GATE,0);
    ScuClockGateCtr(HCLK_HIGH_GATE,0);
    ScuClockGateCtr(HCLK_HIGH_MATRIX_GATE,0);
    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,0);

    ScuSoftResetCtr(MAILBOX_SRST, 1);
    ScuSoftResetCtr(ASYNC_BRG_SRST, 1);

    ScuSoftResetCtr(HIGH_MATRIX_SRST, 1);
    ScuSoftResetCtr(HIGHRAM1_SRST, 1);
    ScuSoftResetCtr(HIGHRAM0_SRST, 1);

    PmuPdLogicPowerDown(1);

}

/*******************************************************************************
** Name: BcoreDeHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.6.5
** Time: 10:27:18
*******************************************************************************/
_DRIVER_BCORE_BCOREDEVICE_INIT_
INIT API void BcoreHwInit(uint32 DevID, uint32 Channel)
{
    PmuPdLogicPowerDown(0);

    ScuClockGateCtr(PCLK_MAILBOX_GATE,1);
    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,1);
    ScuClockGateCtr(HDRAM0_GATE,1);
    ScuClockGateCtr(HDRAM1_GATE,1);
    ScuClockGateCtr(HDRAM2_GATE,1);
    ScuClockGateCtr(HDRAM3_GATE,1);
    ScuClockGateCtr(HDRAM4_GATE,1);
    ScuClockGateCtr(HDRAM5_GATE,1);
    ScuClockGateCtr(HDRAM6_GATE,1);
    ScuClockGateCtr(HDRAM7_GATE,1);
    ScuClockGateCtr(HDRAM_INTERFACE_GATE,1);
    ScuClockGateCtr(HIRAM0_GATE,1);
    ScuClockGateCtr(HIRAM1_GATE,1);
    ScuClockGateCtr(HIRAM2_GATE,1);
    ScuClockGateCtr(HIRAM3_GATE,1);
    ScuClockGateCtr(HIRAM_INTERFACE_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_GATE,1);
    ScuClockGateCtr(HCLK_HIGH_MATRIX_GATE,1);
    ScuClockGateCtr(HCLK_ASYNC_BRG_GATE,1);

    ScuSoftResetCtr(MAILBOX_SRST, 0);
    ScuSoftResetCtr(ASYNC_BRG_SRST, 0);

    ScuSoftResetCtr(HIGH_MATRIX_SRST, 0);
    ScuSoftResetCtr(HIGHRAM1_SRST, 0);
    ScuSoftResetCtr(HIGHRAM0_SRST, 0);


    MailBoxEnableB2AInt(MAILBOX_ID_0, MAILBOX_INT_0);

    IntRegister(INT_ID_MAILBOX0, (void*)BcoreDevIntIsr0);

    IntEnable(INT_ID_MAILBOX0);


}
#endif


#ifdef __DRIVER_SD_SDDEVICE_C__
/*******************************************************************************
** Name: SdDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.6.9
** Time: 16:40:42
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT API void SdDevHwInit(uint32 DevID, uint32 Channel)
{

}

/*******************************************************************************
** Name: SdDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.6.9
** Time: 16:40:42
*******************************************************************************/
_DRIVER_SD_SDDEVICE_INIT_
INIT API void SdDevHwDeInit(uint32 DevID, uint32 Channel)
{

}
#endif


#ifdef __DRIVER_EMMC_EMMCDEVICE_C__
/*******************************************************************************
** Name: EmmcDevDeHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2016.4.12
** Time: 19:35:21
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
INIT API void EmmcDevDeHwInit(uint32 DevID, uint32 Channel)
{
    Gpio_SetPinLevel(GPIO_CH0, GPIOPortA_Pin0, GPIO_LOW);
    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin0, GPIO_IN);
}

/*******************************************************************************
** Name: EmmcDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:API
** Date: 2015.5.20
** Time: 19:16:20
*******************************************************************************/
_DRIVER_EMMC_EMMCDEVICE_INIT_
INIT API void EmmcDevHwInit(uint32 DevID, uint32 Channel)
{
    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin0, GPIO_OUT);
    Gpio_SetPinLevel(GPIO_CH0, GPIOPortA_Pin0, GPIO_HIGH);
}
#endif


#ifdef __DRIVER_I2S_I2SDEVICE_C__
/******************************************************************************
** Name: I2SDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2015.5.19
** Time: 17:36:58
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
INIT API void I2SDevHwInit(uint32 DevID, uint32 Channel)
{
    switch (DevID)
    {
        case I2S_DEV0:
            switch (Channel)
            {
                case I2S_SEL_ACODEC:
                    //rk_printf("select I2S DevID 0, I2S_SEL_ACODEC\n");
                    Grf_I2S0_Sel(I2S_SEL_ACODEC);//Select NAND interior codec
                    break;
                case I2S_SEL_PAD:
                    //rk_printf("select I2S DevID 0, I2S_SEL_PAD\n");
                    Grf_I2S0_Sel(I2S_SEL_PAD);//Select external codec
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin7, IOMUX_GPIO0B7_I2S0_CLK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin5, IOMUX_GPIO0B5_I2S0_SCLK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin3, IOMUX_GPIO0B3_I2S0_SDI);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin6, IOMUX_GPIO0B6_I2S0_LRCK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin4, IOMUX_GPIO0B4_I2S0_SDO);
                    break;
                default:
                    break;
            }
            break;

        case I2S_DEV1://I2S_DEV1 only I2S mode
            switch (Channel)
            {
                case I2S_SEL_DEV1_PA:
                    //rk_printf("select I2S DevID 1\n");
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_I2S1A_CLK);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin2, IOMUX_GPIO1A2_I2S1A_SCLK);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin4, IOMUX_GPIO1A4_I2S1A_SDI);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin1, IOMUX_GPIO1A1_I2S1A_LRCK);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin3, IOMUX_GPIO1A3_I2S1A_SDO);
                    break;
                case I2S_SEL_DEV1_PB:
                    //rk_printf("select I2S DevID 1_B\n");
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin0, IOMUX_GPIO0A0_I2S1B_CLK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin2, IOMUX_GPIO0A2_I2S1B_SCLK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin4, IOMUX_GPIO0A4_I2S1B_SDI);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin1, IOMUX_GPIO0A1_I2S1B_LRCK);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin3, IOMUX_GPIO0A3_I2S1B_SDO);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
    //open I2S clk
    ScuClockGateCtr(CLK_I2S_GATE, 1);//CLOCK_GATE_I2S
    switch (DevID)
    {
        case I2S_DEV0:
            //open rst I2S ip
            ScuClockGateCtr(PCLK_I2S0_GATE, 1);
            ScuClockGateCtr(CLK_I2S0_SRC_GATE, 1);
            ScuSoftResetCtr(I2S0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2S0_SRST, 0);

            IntRegister(INT_ID_I2S0 ,I2s0IntIsr);
            IntPendingClear(INT_ID_I2S0);
            IntEnable(INT_ID_I2S0);
            //rk_printf("\n enable I2S_DEV0\n");

            break;
        case I2S_DEV1:
            //open rst I2S ip
            ScuClockGateCtr(PCLK_I2S1_GATE, 1);
            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 1);
            ScuSoftResetCtr(I2S1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2S1_SRST, 0);

            IntRegister(INT_ID_I2S1 ,I2s1IntIsr);
            IntPendingClear(INT_ID_I2S1);
            IntEnable(INT_ID_I2S1);
            break;
        default:
            break;
    }

}

/*******************************************************************************
** Name: I2sDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2016.4.11
** Time: 16:39:05
*******************************************************************************/
_DRIVER_I2S_I2SDEVICE_INIT_
INIT API void I2sDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //close I2S clk
    switch (DevID)
    {
        case I2S_DEV0:
            //open rst I2S ip
            IntDisable(INT_ID_I2S0);
            IntPendingClear(INT_ID_I2S0);
            IntUnregister(INT_ID_I2S0);

            ScuClockGateCtr(CLK_I2S0_SRC_GATE, 0);
            ScuClockGateCtr(PCLK_I2S0_GATE, 0);
            ScuSoftResetCtr(I2S0_SRST, 1);
            break;

        case I2S_DEV1:
            //open rst I2S ip
            IntDisable(INT_ID_I2S1);
            IntPendingClear(INT_ID_I2S1);
            IntUnregister(INT_ID_I2S1);

            ScuClockGateCtr(CLK_I2S1_SRC_GATE, 0);
            ScuClockGateCtr(PCLK_I2S1_GATE, 0);
            ScuSoftResetCtr(I2S1_SRST, 1);
            break;
        default:
            break;
    }
    ScuClockGateCtr(CLK_I2S_GATE, 0);//CLOCK_GATE_I2S

    switch (DevID)
    {
        case I2S_DEV0:
            switch (Channel)
            {
                case I2S_SEL_ACODEC:
                    //rk_printf("select I2S DevID 0, I2S_SEL_PAD\n");
                    Grf_I2S0_Sel(I2S_SEL_PAD);
                    break;
                case I2S_SEL_PAD:
                    Grf_I2S0_Sel(I2S_SEL_PAD);//Select external codec
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin7, IOMUX_GPIO0B7_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin7, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin5, IOMUX_GPIO0B5_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin5, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin3, IOMUX_GPIO0B3_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin3, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin6, IOMUX_GPIO0B6_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin6, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortB_Pin4, IOMUX_GPIO0B4_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortB_Pin4, GPIO_IN);
                    break;
                default:
                    break;
            }
            break;

        case I2S_DEV1://I2S_DEV1 only I2S mode
            switch (Channel)
            {
                case I2S_SEL_DEV1_PA:
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin0, IOMUX_GPIO1A0_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin0, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin2, IOMUX_GPIO1A2_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin2, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin4, IOMUX_GPIO1A4_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin4, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin1, IOMUX_GPIO1A1_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin3, IOMUX_GPIO1A3_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin3, GPIO_IN);
                    break;
                case I2S_SEL_DEV1_PB:
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin0, IOMUX_GPIO0A0_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin0, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin2, IOMUX_GPIO0A2_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin2, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin4, IOMUX_GPIO0A4_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin4, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin1, IOMUX_GPIO0A1_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0, GPIOPortA_Pin3, IOMUX_GPIO0A3_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin3, GPIO_IN);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
}

#endif

#ifdef __DRIVER_USB_USBOTGDEV_C__
/*******************************************************************************
** Name: UsbOtgDevHwInit
** Input:uitn32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.18
** Time: 11:29:13
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
INIT API void UsbOtgDevHwInit(uint32 DevID, uint32 Channel)
{
    //CPU init
    ScuClockGateCtr(CLK_USBPHY_GATE, 1);
    ScuClockGateCtr(HCLK_USBC_GATE, 1);
    ScuSoftResetCtr(USBPHY_SRST, 0);
    ScuSoftResetCtr(USBGLB_SRST, 0);
    ScuSoftResetCtr(USBOTG_SRST, 0);
    DelayUs(5);
    Grf_otgphy_suspend(0);
    DelayUs(200);

    //interrupt init
    IntRegister(INT_ID_USBC, (void*)UsbOtgDevIntIsr0);

}


/*******************************************************************************
** Name: UsbOtgDevHwInit
** Input:uitn32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.18
** Time: 11:29:13
*******************************************************************************/
_DRIVER_USB_USBOTGDEV_INIT_
INIT API void UsbOtgDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //CPU init
    ScuSoftResetCtr(USBPHY_SRST, 1);
    ScuSoftResetCtr(USBGLB_SRST, 1);
    ScuSoftResetCtr(USBOTG_SRST, 1);
    ScuClockGateCtr(CLK_USBPHY_GATE, 0);
    ScuClockGateCtr(HCLK_USBC_GATE, 0);
    Grf_otgphy_suspend(1);
    //interrupt init
    IntUnregister(INT_ID_USBC);

}
#endif

#ifdef __DRIVER_I2C_I2CDEVICE_C__
/*******************************************************************************
** Name: I2cDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2016.4.11
** Time: 17:06:16
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_INIT_
INIT API void I2cDevHwDeInit(uint32 DevID, uint32 Channel)
{
    switch (DevID)
    {
        case I2C_DEV0:
            IntDisable(INT_ID_I2C0);
            IntPendingClear(INT_ID_I2C0);
            IntUnregister(INT_ID_I2C0);//I2CDevIntIsr0
            //close I2C0 clk
            ScuClockGateCtr(PCLK_I2C0_GATE, 0);
            rk_printf("I2CDevInit I2C_DEV0\n");
            ScuSoftResetCtr(I2C0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C0_SRST, 0);
            break;

        case I2C_DEV1:
            rk_printf("I2CDevInit I2C_DEV1\n");
            IntDisable(INT_ID_I2C1);
            IntPendingClear(INT_ID_I2C1);
            IntUnregister(INT_ID_I2C1);
            //close I2C1 clk
            ScuClockGateCtr(PCLK_I2C1_GATE, 0);
            ScuSoftResetCtr(I2C1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C1_SRST, 0);
            break;

        case I2C_DEV2:
            //close I2C2 clk
            IntDisable(INT_ID_I2C2);
            IntPendingClear(INT_ID_I2C2);
            IntUnregister(INT_ID_I2C2);

            ScuClockGateCtr(PCLK_I2C2_GATE, 0);
            ScuSoftResetCtr(I2C2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C2_SRST, 0);
            break;

        default:
            break;
    }

    switch (DevID)
    {
        case I2C_DEV0:
            Grf_I2C0_Sel(I2C_SEL_A);//I2C0_SEL Grf_I2C1_Sel(I2C_SEL_A);
            switch (Channel)
            {
                case I2C_DEV0_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin2, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin3, GPIO_IN);
                    //rk_printf("I2C0_IO RTC_HYM8563\n");
                    break;

                case I2C_DEV0_PB:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortC_Pin0, GPIO_IN);
                    break;

                case I2C_DEV0_PC:
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin6, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortA_Pin5, GPIO_IN);
                    break;

                default:
                    break;
            }
            break;
        case I2C_DEV1:
            Grf_I2C0_Sel(I2C_SEL_B);//I2C1_SEL//FM5807
            switch (Channel)
            {
                case I2C_DEV1_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin0, GPIO_IN);
                    break;

                case I2C_DEV1_PB:
                    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_IO);
                    Gpio_SetPinDirection(GPIO_CH1, GPIOPortB_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_IO);
                    Gpio_SetPinDirection(GPIO_CH1, GPIOPortB_Pin2, GPIO_IN);
                    break;

                case I2C_DEV1_PC:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin5, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortB_Pin4, GPIO_IN);
                    break;

                default:
                    break;
            }
            break;
        case I2C_DEV2:
            Grf_I2C0_Sel(I2C_SEL_C);//I2C2_SEL
            switch (Channel)
            {
                case I2C_DEV2_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin1, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin0, GPIO_IN);
                    break;

                case I2C_DEV2_PB:
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortD_Pin0, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_IO);
                    Gpio_SetPinDirection(GPIO_CH0, GPIOPortD_Pin1, GPIO_IN);
                    break;

                case I2C_DEV2_PC:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin5, GPIO_IN);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
                    Gpio_SetPinDirection(GPIO_CH2, GPIOPortA_Pin6, GPIO_IN);
                    break;

                default:
                    break;
            }
            break;
    }
}

/*******************************************************************************
** Name: I2cDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:31:27
*******************************************************************************/
_DRIVER_I2C_I2CDEVICE_INIT_
INIT API void I2cDevHwInit(uint32 DevID, uint32 Channel)
{
    switch (DevID)
    {
        case I2C_DEV0:
            Grf_I2C0_Sel(I2C_SEL_A);//I2C0_SEL Grf_I2C1_Sel(I2C_SEL_A);
            switch (Channel)
            {
                case I2C_DEV0_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_I2C0A_SCL);//I2C0  -- U6001 RTC_HYM8563
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_I2C0A_SDA);
                    //rk_printf("I2C0_IO RTC_HYM8563\n");
                    break;

                case I2C_DEV0_PB:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_I2C0B_SCL);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_I2C0B_SDA);
                    break;

                case I2C_DEV0_PC:
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_I2C0C_SCL);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_I2C0C_SDA);
                    break;

                default:
                    break;
            }
            break;
        case I2C_DEV1:
            Grf_I2C0_Sel(I2C_SEL_B);//I2C1_SEL//FM5807
            switch (Channel)
            {
                case I2C_DEV1_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_I2C1A_SCL);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_I2C1A_SDA);
                    break;

                case I2C_DEV1_PB:
                    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_I2C1B_SCL);
                    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_I2C1B_SDA);
                    break;

                case I2C_DEV1_PC:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_I2C1C_SCL);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_I2C1C_SDA);
                    break;

                default:
                    break;
            }
            break;
        case I2C_DEV2:
            Grf_I2C0_Sel(I2C_SEL_C);//I2C2_SEL
            switch (Channel)
            {
                case I2C_DEV2_PA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_I2C2A_SCL);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_I2C2A_SDA);
                    break;

                case I2C_DEV2_PB:
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_I2C2B_SCL);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_I2C2B_SDA);
                    break;

                case I2C_DEV2_PC:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_I2C2C_SCL);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_I2C2C_SDA);
                    break;

                default:
                    break;
            }
            break;
    }
    switch (DevID)
    {
        case I2C_DEV0:
            //open I2C0 clk
            ScuClockGateCtr(PCLK_I2C0_GATE, 1);
            rk_printf("I2CDevInit I2C_DEV0\n");
            ScuSoftResetCtr(I2C0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C0_SRST, 0);

            IntRegister(INT_ID_I2C0 ,I2CDevIntIsr0);//I2CDevIntIsr0
            IntPendingClear(INT_ID_I2C0);
            IntEnable(INT_ID_I2C0);
            break;

        case I2C_DEV1:
            rk_printf("I2CDevInit I2C_DEV1\n");
            //open I2C1 clk
            ScuClockGateCtr(PCLK_I2C1_GATE, 1);
            ScuSoftResetCtr(I2C1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C1_SRST, 0);

            IntRegister(INT_ID_I2C1 ,I2CDevIntIsr1);
            IntPendingClear(INT_ID_I2C1);
            IntEnable(INT_ID_I2C1);
            break;

        case I2C_DEV2:
            //open I2C2 clk
            ScuClockGateCtr(PCLK_I2C2_GATE, 1);
            ScuSoftResetCtr(I2C2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(I2C2_SRST, 0);

            IntRegister(INT_ID_I2C2 ,I2CDevIntIsr2);
            IntPendingClear(INT_ID_I2C2);
            IntEnable(INT_ID_I2C2);
            break;

        default:
            break;
    }
}
#endif

#ifdef __DRIVER_UART_UARTDEVICE_C__
/*******************************************************************************
** Name: UartDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:30:01
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
INIT API rk_err_t UartDevHwInit(uint32 DevID, uint32 Channel, uint32 Freq)
{
    //gpio iomux
    switch (DevID)
    {
        case UART_DEV0:
        {

            switch (Channel)
            {
                case UART_CHA:
                    Grf_UART0_Sel(UART_SEL_A);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_UART0A_TX);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_UART0A_RX);

                    break;
            }
            break;
        }


        case UART_DEV1:
        {
            switch (Channel)
            {
                case UART_CHA:
                    Grf_UART1_Sel(UART_SEL_A);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_UART1A_TX);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_UART1A_RX);

                    // rts/cts
                    Grf_Force_Jtag_Set(0);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_UART1A_RTS);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_UART1A_CTS);
                    break;
            }
            break;
        }

        case UART_DEV2:
        {
           switch (Channel)
            {
                case UART_CHB:
                    Grf_UART2_Sel(UART_CHB);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_UART2B_TX);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_UART2B_RX);
                    // rts/cts
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_UART2B_RTS);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_UART2B_CTS);
                    break;
            }
            break;
        }

        case UART_DEV3:
            break;

        case UART_DEV4:
            break;

        case UART_DEV5:
            break;
    }

    //clock gate & rst
    switch (DevID)
    {
        case UART_DEV0:
            ScuSoftResetCtr(UART0_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART0_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 1);
            ScuClockGateCtr(PCLK_UART0_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV0,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }
            break;

        case UART_DEV1:
            ScuSoftResetCtr(UART1_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART1_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 1);
            ScuClockGateCtr(PCLK_UART1_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV1,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }

            break;

        case UART_DEV2:
            ScuSoftResetCtr(UART2_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART2_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 1);
            ScuClockGateCtr(PCLK_UART2_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV2,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }
            break;

        case UART_DEV3:
            ScuSoftResetCtr(UART3_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART3_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 1);
            ScuClockGateCtr(PCLK_UART3_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV3,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }
            break;

        case UART_DEV4:
            ScuSoftResetCtr(UART4_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART4_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 1);
            ScuClockGateCtr(PCLK_UART4_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV4,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }
            break;

        case UART_DEV5:
            ScuSoftResetCtr(UART5_SRST, 1);
            DelayMs(1);
            ScuSoftResetCtr(UART5_SRST, 0);

            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 1);
            ScuClockGateCtr(PCLK_UART5_GATE, 1);
            if(Freq == 24*1000*1000)
            {
                SetUartFreq(UART_DEV5,XIN24M,24*1000*1000);
            }
            else
            {
                return RK_ERROR;
            }
            break;
    }


    switch (DevID)
    {
        case UART_DEV0:
            IntRegister(INT_ID_UART0 ,UartIntIsr0);
            IntPendingClear(INT_ID_UART0);
            IntEnable(INT_ID_UART0);
            break;

        case UART_DEV1:
            IntRegister(INT_ID_UART1 ,UartIntIsr1);
            IntPendingClear(INT_ID_UART1);
            IntEnable(INT_ID_UART1);
            break;

        case UART_DEV2:
            IntRegister(INT_ID_UART2 ,UartIntIsr2);
            IntPendingClear(INT_ID_UART2);
            IntEnable(INT_ID_UART2);
            break;

        case UART_DEV3:
            IntRegister(INT_ID_UART3 ,UartIntIsr3);
            IntPendingClear(INT_ID_UART3);
            IntEnable(INT_ID_UART3);
            break;

        case UART_DEV4:
            IntRegister(INT_ID_UART4 ,UartIntIsr4);
            IntPendingClear(INT_ID_UART4);
            IntEnable(INT_ID_UART4);
            break;

        case UART_DEV5:
            IntRegister(INT_ID_UART5 ,UartIntIsr5);
            IntPendingClear(INT_ID_UART5);
            IntEnable(INT_ID_UART5);
            break;

        default:
            break;
    }


    return RK_SUCCESS;

}


/*******************************************************************************
** Name: UartDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:30:01
*******************************************************************************/
_DRIVER_UART_UARTDEVICE_INIT_
INIT API rk_err_t UartDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //gpio iomux
    switch (DevID)
    {
        case UART_DEV0:
        {

            switch (Channel)
            {
                case UART_CHA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_IO);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_IO);

                    break;
            }
            break;
        }


        case UART_DEV1:
        {
            switch (Channel)
            {
                case UART_CHA:
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_IO);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_IO);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_IO);
                    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_IO);
                    break;
            }
            break;
        }

        case UART_DEV2:
        {
           switch (Channel)
            {
                case UART_CHB:
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_IO);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_IO);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_IO);
                    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_IO);
                    break;
            }
            break;
        }

        case UART_DEV3:
            break;

        case UART_DEV4:
            break;

        case UART_DEV5:
            break;
    }

    //clock gate & rst
    switch (DevID)
    {
        case UART_DEV0:
            ScuSoftResetCtr(UART0_SRST, 1);

            //open uart clk
            ScuClockGateCtr(CLK_UART0_GATE, 0);
            ScuClockGateCtr(PCLK_UART0_GATE, 0);
            break;

        case UART_DEV1:
            ScuSoftResetCtr(UART1_SRST, 1);

            //open uart clk
            ScuClockGateCtr(CLK_UART1_GATE, 0);
            ScuClockGateCtr(PCLK_UART1_GATE, 0);
            break;

        case UART_DEV2:
            ScuSoftResetCtr(UART2_SRST, 1);
            //open uart clk
            ScuClockGateCtr(CLK_UART2_GATE, 0);
            ScuClockGateCtr(PCLK_UART2_GATE, 0);
            break;

        case UART_DEV3:
            ScuSoftResetCtr(UART3_SRST, 1);

            //open uart clk
            ScuClockGateCtr(CLK_UART3_GATE, 0);
            ScuClockGateCtr(PCLK_UART3_GATE, 0);
            break;

        case UART_DEV4:
            ScuSoftResetCtr(UART4_SRST, 1);
            //open uart clk
            ScuClockGateCtr(CLK_UART4_GATE, 0);
            ScuClockGateCtr(PCLK_UART4_GATE, 0);
            break;

        case UART_DEV5:
            ScuSoftResetCtr(UART5_SRST, 1);
            //open uart clk
            ScuClockGateCtr(CLK_UART5_GATE, 0);
            ScuClockGateCtr(PCLK_UART5_GATE, 0);
            break;
    }


    switch (DevID)
    {
        case UART_DEV0:
            IntUnregister(INT_ID_UART0);
            IntPendingClear(INT_ID_UART0);
            IntDisable(INT_ID_UART0);
            break;

        case UART_DEV1:
            IntUnregister(INT_ID_UART1);
            IntPendingClear(INT_ID_UART1);
            IntDisable(INT_ID_UART1);
            break;

        case UART_DEV2:
            IntUnregister(INT_ID_UART2);
            IntPendingClear(INT_ID_UART2);
            IntDisable(INT_ID_UART2);
            break;

        case UART_DEV3:
            IntUnregister(INT_ID_UART3);
            IntPendingClear(INT_ID_UART3);
            IntDisable(INT_ID_UART3);
            break;

        case UART_DEV4:
            IntUnregister(INT_ID_UART4);
            IntPendingClear(INT_ID_UART4);
            IntDisable(INT_ID_UART4);
            break;

        case UART_DEV5:
            IntUnregister(INT_ID_UART5);
            IntPendingClear(INT_ID_UART5);
            IntDisable(INT_ID_UART5);
            break;

        default:
            break;
    }


    return RK_SUCCESS;

}
#endif

#ifdef __DRIVER_SDIO_SDIODEVICE_C__
/*******************************************************************************
** Name: SdioDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:28:17
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API void SdioDevHwInit(uint32 DevID, uint32 Channel)
{
    if (DevID == 0)
    {
        #ifdef _WICE_
        Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
        Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
        Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_HIGH);
        rkos_sleep(100);
        #endif
    }
}
#endif


/*******************************************************************************
** Name: SdioDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.12.5
** Time: 12:02:00
*******************************************************************************/
_DRIVER_SDIO_SDIODEVICE_INIT_
INIT API void SdioDevHwDeInit(uint32 DevID, uint32 Channel)
{
    if (DevID == 0)
    {
        #ifdef _WICE_
        printf("sdio de init");
        Grf_GpioMuxSet(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, Type_Gpio);
        Gpio_SetPinDirection(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_OUT);
        Gpio_SetPinLevel(AP6181_POWER_ON_GPIO_CH, AP6181_POWER_ON_GPIO_PIN, GPIO_LOW);
        #endif
    }
}


#ifdef __DRIVER_SDMMC_SDMMCDEVICE_C__
/*******************************************************************************
** Name: SdMmcDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:26:37
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
INIT API void SdMmcDevHwInit(uint32 DevID, uint32 Channel)
{
    //io mux

    if (DevID == SDC0)
    {

        if (Channel == 0)
        {
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_EMMC_D3);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin6,ENABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_EMMC_D2);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5,ENABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_EMMC_D1);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin4,ENABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_EMMC_D0);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin3,ENABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_EMMC_CMD);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin2,ENABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_EMMC_CLK);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin1,ENABLE);
        }
        else if (Channel == 1)
        {
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_EMMC_D4);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_EMMC_D3);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_EMMC_D2);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_EMMC_D1);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_EMMC_D0);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_EMMC_CMD);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_EMMC_CLK);

            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_EMMC_D7);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_EMMC_D6);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_EMMC_D5);
        }
    }
    else if (DevID == SDC1)
    {
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_SDMMC_CMD);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin5,ENABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_SDMMC_CLK);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin6,ENABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_SDMMC_D0);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin7,ENABLE);

        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_SDMMC_D1);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin0,ENABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_SDMMC_D2);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin1,ENABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_SDMMC_D3);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin2,ENABLE);
    }

    //clock gate
    if (DevID == SDC0)
    {
        SetSdmmc0Freq(XIN24M,24*1000*1000);

        ScuClockGateCtr(HCLK_EMMC_GATE, 1);
        ScuClockGateCtr(CLK_EMMC_GATE, 1);

        ScuSoftResetCtr(SDMMC0_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(SDMMC0_SRST, 0);

    }
    else if (DevID == SDC1)
    {
        SetSdmmc1Freq(XIN24M,24*1000*1000);

        ScuClockGateCtr(HCLK_SDMMC_GATE, 1);
        ScuClockGateCtr(CLK_SDMMC_GATE, 1);

        //open rst sdmmc ip
        ScuSoftResetCtr(SDMMC1_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(SDMMC1_SRST, 0);

    }

    // inter isr
    if (DevID == SDC0)
    {
        IntUnregister(INT_ID_EMMC);
        IntPendingClear(INT_ID_EMMC);
        IntDisable(INT_ID_EMMC);
        IntRegister(INT_ID_EMMC ,SdcIntIRQ0);
        IntPendingClear(INT_ID_EMMC);
        IntEnable(INT_ID_EMMC);
    }
    else if (DevID == SDC1)
    {
        IntUnregister(INT_ID_SDMMC);
        IntPendingClear(INT_ID_SDMMC);
        IntDisable(INT_ID_SDMMC);
        IntRegister(INT_ID_SDMMC ,SdcIntIRQ1);
        IntPendingClear(INT_ID_SDMMC);
        IntEnable(INT_ID_SDMMC);
    }

}



/*******************************************************************************
** Name: SdMmcDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:26:37
*******************************************************************************/
_DRIVER_SDMMC_SDMMCDEVICE_INIT_
INIT API void SdMmcDevHwDeInit(uint32 DevID, uint32 Channel)
{
    //io mux

    if (DevID == SDC0)
    {

        if (Channel == 0)
        {
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin6,DISABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5,DISABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin4,DISABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin3,DISABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin2,DISABLE);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_IO);
            Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin1,DISABLE);
        }
        else if (Channel == 1)
        {
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_IO);

            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_IO);
            Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_IO);
        }
    }
    else if (DevID == SDC1)
    {
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin5,DISABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin6,DISABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin7,DISABLE);

        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin0,DISABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin1,DISABLE);
        Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_IO);
        Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin2,DISABLE);
    }

    //clock gate
    if (DevID == SDC0)
    {
        ScuClockGateCtr(HCLK_EMMC_GATE, 0);
        ScuClockGateCtr(CLK_EMMC_GATE, 0);

        ScuSoftResetCtr(SDMMC0_SRST, 1);
    }
    else if (DevID == SDC1)
    {
        ScuClockGateCtr(HCLK_SDMMC_GATE, 0);
        ScuClockGateCtr(CLK_SDMMC_GATE, 0);
        //open rst sdmmc ip
        ScuSoftResetCtr(SDMMC1_SRST, 1);
    }

    // inter isr
    if (DevID == SDC0)
    {
        IntUnregister(INT_ID_EMMC);
        IntPendingClear(INT_ID_EMMC);
        IntDisable(INT_ID_EMMC);

    }
    else if (DevID == SDC1)
    {
        IntUnregister(INT_ID_SDMMC);
        IntPendingClear(INT_ID_SDMMC);
        IntDisable(INT_ID_SDMMC);
    }

}
#endif

#ifdef __DRIVER_SPI_SPIDEVICE_C__
/*******************************************************************************
** Name: SpiDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:23:08
*******************************************************************************/
_DRIVER_SPI_SPIDEVICE_INIT_
INIT API void SpiDevHwInit(uint32 DevID, uint32 Channel)
{
    if (DevID == 0)
    {
        if (Channel == 0)
        {
            Grf_SPI0_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin0, IOMUX_GPIO0C0_SPI0A_TX);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin2, IOMUX_GPIO0C2_SPI0A_CLK);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin3, IOMUX_GPIO0C3_SPI0A_CS);
            Grf_GpioMuxSet(GPIO_CH0, GPIOPortC_Pin1, IOMUX_GPIO0C1_SPI0A_RX);

        }
        else if (Channel == 1)
        {
            Grf_SPI0_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin2, IOMUX_GPIO2B2_SPI0B_TX);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin1, IOMUX_GPIO2B1_SPI0B_CLK);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin0, IOMUX_GPIO2B0_SPI0B_CS);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin3, IOMUX_GPIO2B3_SPI0B_RX);
        }
    }
    else if (DevID == 1)
    {
        if (Channel == 0)
        {
            Grf_SPI1_Sel(SPI_SEL_A);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortB_Pin0, IOMUX_GPIO1B0_SPI1A_TX);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin6, IOMUX_GPIO1A6_SPI1A_CLK);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin5, IOMUX_GPIO1A5_SPI1A_CS);
            Grf_GpioMuxSet(GPIO_CH1, GPIOPortA_Pin7, IOMUX_GPIO1A7_SPI1A_RX);

        }
        else if (Channel == 1)
        {
            Grf_SPI1_Sel(SPI_SEL_B);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin1, IOMUX_GPIO2C1_SPI1B_TX);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortC_Pin0, IOMUX_GPIO2C0_SPI1B_CLK);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin6, IOMUX_GPIO2B6_SPI1B_CS);
            Grf_GpioMuxSet(GPIO_CH2, GPIOPortB_Pin7, IOMUX_GPIO2B7_SPI1B_RX);
        }
    }
}
#endif

#ifdef __DRIVER_AD_ADCDEVICE_C__
/*******************************************************************************
** Name: AdcDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:wrm
** Date: 2015.5.18
** Time: 15:43:37
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
void AdcDevHwInit (uint32 DevID, uint32 Channel)
{
    ScuClockGateCtr(CLK_SARADC_GATE, 1);
    ScuClockGateCtr(PCLK_SARADC_GATE, 1);

    ScuSoftResetCtr(SARADC_SRST, 0);

    SetSarAdcFreq(1000000);        // 1M

    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWON);
    Grf_Verf_Trim_Sel(GRF_VREF_TRIM_2600);
    Grf_Verf_Vbg_Sel(GRF_VREF_TRIM_2425);

    IntUnregister(INT_ID_SRADC);
    IntPendingClear(INT_ID_SRADC);
    IntDisable(INT_ID_SRADC);

    IntRegister(INT_ID_SRADC, ADCDevISR);
    IntPendingClear(INT_ID_SRADC);
    IntEnable(INT_ID_SRADC);
}


/*******************************************************************************
** Name: AdcDevHwDeInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:wrm
** Date: 2015.5.18
** Time: 15:43:37
*******************************************************************************/
_DRIVER_AD_ADCDEVICE_INIT_
void AdcDevHwDeInit (uint32 DevID, uint32 Channel)
{
    ScuClockGateCtr(CLK_SARADC_GATE, 0);
    ScuClockGateCtr(PCLK_SARADC_GATE, 0);

    ScuSoftResetCtr(SARADC_SRST, 1);

    Grf_Set_ADC_Vref_PowerOn(GRF_VREF_PWOFF);

    IntUnregister(INT_ID_SRADC);
    IntPendingClear(INT_ID_SRADC);
    IntDisable(INT_ID_SRADC);
}
#endif

#ifdef __DRIVER_PWM_PWMDEVICE_C__
/*******************************************************************************
** Name: PwmDevHwDeInit
** Input:uint32 DevID, uint32 channel
** Return: void
** Owner:aaron.sun
** Date: 2016.3.17
** Time: 9:03:01
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
INIT API void PwmDevHwDeInit(uint32 DevID, uint32 channel)
{
    switch ( channel )
    {
        case PWM_CHN0:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4, GPIO_IN);
            break;

        case PWM_CHN1:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3, GPIO_IN);
            break;

        case PWM_CHN2:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2, GPIO_IN);
            break;

        case PWM_CHN3:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1, GPIO_IN);
            break;

        case PWM_CHN4:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0, GPIO_IN);
            break;

        default:
            break;
    }

    if (channel < PWM_CHN4)
    {
        ScuSoftResetCtr(PWM0_SRST, 1);
        ScuClockGateCtr(PCLK_PWM0_GATE, 0);
    }
    else
    {
        ScuSoftResetCtr(PWM1_SRST, 1);
        ScuClockGateCtr(PCLK_PWM1_GATE, 0);
    }
}

/*******************************************************************************
** Name: PwmDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 9:17:37
*******************************************************************************/
_DRIVER_PWM_PWMDEVICE_INIT_
void PwmDevHwInit (uint32 DevID, uint32 channel)
{
    switch ( channel )
    {
        case PWM_CHN0:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_PWM0_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4, GPIO_OUT);
            break;

        case PWM_CHN1:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_PWM1_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3, GPIO_OUT);
            break;

        case PWM_CHN2:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_PWM2_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2, GPIO_OUT);
            break;

        case PWM_CHN3:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_PWM3_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1, GPIO_OUT);
            break;

        case PWM_CHN4:
            Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_PWM4_OUT);
            Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0, GPIO_OUT);
            break;

        default:
            break;
    }

    if (channel < PWM_CHN4)
    {
        ScuSoftResetCtr(PWM0_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(PWM0_SRST, 0);

        ScuClockGateCtr(PCLK_PWM0_GATE, 1);
    }
    else
    {
        ScuSoftResetCtr(PWM1_SRST, 1);
        DelayMs(1);
        ScuSoftResetCtr(PWM1_SRST, 0);

        ScuClockGateCtr(PCLK_PWM1_GATE, 1);
    }
}
#endif


#ifdef __DRIVER_WATCHDOG_WATCHDOGDEVICE_C__
/*******************************************************************************
** Name: WdtDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 10:34:37
*******************************************************************************/
void WdtDevHwInit (uint32 DevID, uint32 channel)
{

    rk_printf ("Init WDT Hardware");

    ScuClockGateCtr(PCLK_WDT_GATE, 1);

    ScuSoftResetCtr(WDT_SRST, 0);

    IntUnregister(INT_ID_WDT);
    IntPendingClear(INT_ID_WDT);
    IntDisable(INT_ID_WDT);

    IntRegister(INT_ID_WDT, WDTDevIntIsr0);
    IntPendingClear(INT_ID_WDT);
    IntEnable(INT_ID_WDT);
}

/*******************************************************************************
** Name: WdtDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:wrm
** Date: 2015.5.19
** Time: 10:34:37
*******************************************************************************/
void WdtDevHwDeInit (uint32 DevID, uint32 channel)
{

    rk_printf ("DeInit WDT Hardware");

    ScuSoftResetCtr(WDT_SRST, 1);

    rkos_sleep(10);

    ScuClockGateCtr(PCLK_WDT_GATE, 0);

    IntUnregister(INT_ID_WDT);
    IntPendingClear(INT_ID_WDT);
    IntDisable(INT_ID_WDT);
}

#endif

#ifdef __DRIVER_MAILBOX_MAILBOXDEVICE_C__
/*******************************************************************************
** Name: MailBoxDevHwInit
** Input:uint32 DevID, uint32 Channel
** Return: void
** Owner:cjh
** Date: 2015.5.20
** Time: 19:28:55
*******************************************************************************/
_DRIVER_MAILBOX_MAILBOXDEVICE_INIT_
INIT API void MailBoxDevHwInit(uint32 DevID, uint32 IntTypeA2BB2A)
{
    switch (IntTypeA2BB2A)
    {
        case MAILBOX_A2B_TYPE:
            switch (DevID)
            {
                case MAILBOX_CHANNEL_0:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0, MailBoxDevB2AIsr0);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
                    MailBoxEnableA2BInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_0));
                    break;

                case MAILBOX_CHANNEL_1:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1, MailBoxDevB2AIsr1);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
                    MailBoxEnableA2BInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_1));
                    break;

                case MAILBOX_CHANNEL_2:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2, MailBoxDevB2AIsr2);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
                    MailBoxEnableA2BInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_2));
                    break;

                case MAILBOX_CHANNEL_3:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3, MailBoxDevB2AIsr3);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
                    MailBoxEnableA2BInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_3));
                    break;

                default:
                    break;
            }
            ScuSoftResetCtr(CAL_CORE_SRST, TRUE);
            DelayMs(1);
            ScuSoftResetCtr(CAL_CORE_SRST, FALSE);
            break;
        case MAILBOX_B2A_TYPE:
            switch (DevID)
            {
                case MAILBOX_CHANNEL_0:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0, MailBoxDevA2BIsr0);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_0);
                    MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_0));
                    break;

                case MAILBOX_CHANNEL_1:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1, MailBoxDevA2BIsr1);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_1);
                    MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_1));
                    break;

                case MAILBOX_CHANNEL_2:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2, MailBoxDevA2BIsr2);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_2);
                    MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_2));
                    break;

                case MAILBOX_CHANNEL_3:
                    IntRegister(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3, MailBoxDevA2BIsr3);
                    IntPendingClear(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
                    IntEnable(INT_ID_MAILBOX0 + MAILBOX_CHANNEL_3);
                    MailBoxEnableB2AInt(MAILBOX_ID_0, (int32)(1 << MAILBOX_CHANNEL_3));
                    break;

                default:
                    break;
            }
            ScuSoftResetCtr(CAL_CORE_SRST, TRUE);
            DelayMs(1);
            ScuSoftResetCtr(CAL_CORE_SRST, FALSE);
            break;
        default:
            break;
    }

}
#endif

#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API void SpiFlashDevHwInit(void)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_SFC_D3);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_SFC_D2);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_SFC_D1);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_SFC_D0);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_SFC_CLK);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_SFC_CS);

    SetSFCFreq(XIN24M,24000000);

    ScuClockGateCtr(CLK_SFC_GATE,1);
    ScuClockGateCtr(HCLK_SFC_GATE,1);
    ScuSoftResetCtr(SFC_SRST, 0);

    IntRegister(INT_ID_SFC,SpiFlashDevIntIsr0);
    IntPendingClear(INT_ID_SFC);
    IntDisable(INT_ID_SFC);
}

_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API void SpiFlashDevHwDeInit(void)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_IO);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_IO);

    ScuClockGateCtr(CLK_SFC_GATE,0);
    ScuClockGateCtr(HCLK_SFC_GATE,0);
    ScuSoftResetCtr(SFC_SRST, 1);

    IntUnregister(INT_ID_SFC);
    IntPendingClear(INT_ID_SFC);
    IntDisable(INT_ID_SFC);
}
#endif

/*******************************************************************************
** Name: BSP_Init
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 17:15:36
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT API void bsp_init(void)
{
    SysVariableInit();

    #ifndef _RKOS_DEBUG_
    CPUInit();
    GpioInit();
    #endif

#ifdef BB_SYS_JTAG
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_JTG1_TMS);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_JTG1_TCK);
#endif

    SysTickInit();
    SysInterruptInit();

    System_Power_Init();

    System_Power_On(1);
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*
--------------------------------------------------------------------------------
  Function name : void CpuClockGateInit(void)
  Author        : ZHengYongzhi
  Description   : clock gating init.

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2015/03/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
_BSP_EVK_V20_BSP_INIT_
void CpuClockGateInit(void)
{
    CRU->CRU_CLKGATE_CON[0] = (0x00010000 << (CLK_SYS_CORE_GATE%16))  |
                              (0x00010000 << (HCLK_SYS_CORE_GATE%16)) |
                              (0x00010001 << (HCLK_CAL_CORE_GATE%16)) |
                              (0x00010000 << (HCLK_LOGIC_GATE%16))    |
                              (0x00010000 << (PCLK_LOGIC_GATE%16))    |
                              (0x00010001 << (CLK_RESERVED0%16))      |
                              (0x00010000 << (HCLK_PMU_GATE%16))      |
                              (0x00010001 << (CLK_USBPHY_GATE%16))    |
                              (0x00010001 << (CLK_SFC_GATE%16))       |
                              (0x00010001 << (CLK_RESERVED1%16))      |
                              (0x00010001 << (CLK_SARADC_GATE%16))    |
                              (0x00010001 << (CLK_TIMER0_GATE%16))    |
                              (0x00010001 << (CLK_TIMER1_GATE%16))    |
                              (0x00010000 << (PCLK_PMU_GATE%16))      |
                              (0x00010001 << (CLK_ACODEC_GATE%16))    ;

    CRU->CRU_CLKGATE_CON[1] = (0x00010001 << (CLK_I2S1_SRC_GATE%16))      |
                              (0x00010001 << (CLK_I2S1_FRAC_SRC_GATE%16)) |
                              (0x00010001 << (CLK_I2S1_OUT_GATE%16))      |
                              (0x00010001 << (CLK_I2S1_GATE%16))          |
                              (0x00010001 << (CLK_I2S0_SRC_GATE%16))      |
                              (0x00010001 << (CLK_I2S_FRAC_SRC_GATE%16))  |
                              (0x00010001 << (CLK_I2S_GATE%16))           |
                              (0x00010001 << (DCLK_EBC_SRC_GATE%16))      |
                              (0x00010001 << (CLK_SDMMC_GATE%16))         |
                              (0x00010001 << (CLK_EMMC_GATE%16))          |
                              (0x00010001 << (CLK_SPI0_GATE%16))          |
                              (0x00010001 << (CLK_SPI1_GATE%16))          |
                              (0x00010001 << (CLK_UART0_GATE%16))         |
                              (0x00010001 << (CLK_UART1_GATE%16))         |
                              (0x00010001 << (CLK_UART2_GATE%16))         |
                              (0x00010001 << (CLK_TEST_GATE%16))          ;

    CRU->CRU_CLKGATE_CON[2] = (0x00010001 << (CLK_UART3_GATE%16)) |
                              (0x00010001 << (CLK_UART4_GATE%16)) |
                              (0x00010001 << (CLK_UART5_GATE%16)) ;

    CRU->CRU_CLKGATE_CON[3] = (0x00010001 << (HDRAM0_GATE%16))          |
                              (0x00010001 << (HDRAM1_GATE%16))          |
                              (0x00010001 << (HDRAM2_GATE%16))          |
                              (0x00010001 << (HDRAM3_GATE%16))          |
                              (0x00010001 << (HDRAM4_GATE%16))          |
                              (0x00010001 << (HDRAM5_GATE%16))          |
                              (0x00010001 << (HDRAM6_GATE%16))          |
                              (0x00010001 << (HDRAM7_GATE%16))          |
                              (0x00010001 << (HDRAM_INTERFACE_GATE%16)) |
                              (0x00010001 << (HCLK_HIFI_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[4] = (0x00010001 << (HIRAM0_GATE%16))          |
                              (0x00010001 << (HIRAM1_GATE%16))          |
                              (0x00010001 << (HIRAM2_GATE%16))          |
                              (0x00010001 << (HIRAM3_GATE%16))          |
                              (0x00010001 << (Reserved4_GATE%16))       |
                              (0x00010001 << (Reserved5_GATE%16))       |
                              (0x00010001 << (Reserved6_GATE%16))       |
                              (0x00010001 << (Reserved7_GATE%16))       |
                              (0x00010001 << (HIRAM_INTERFACE_GATE%16)) |
                              (0x00010001 << (HCLK_HIGH_GATE%16))       |
                              (0x00010001 << (HCLK_HIGH_MATRIX_GATE%16))|
                              (0x00010001 << (HCLK_HIGH_DMA_GATE%16))   |
                              (0x00010001 << (HCLK_ASYNC_BRG_GATE%16))  ;


    CRU->CRU_CLKGATE_CON[5] = (0x00010000 << (PCLK_PMU_BUS_GATE%16))      |
                              (0x00010001 << (CLK_RESERVED2%16))          |
                              (0x00010001 << (CLK_RESERVED3%16))          |
                              (0x00010000 << (PCLK_GRF_GATE%16))          |
                              (0x00010000 << (PCLK_GPIO2_GATE%16))        |
                              (0x00010000 << (CLK24M_GATE%16))            |
                              (0x00010000 << (PMU_RAM_INTERFACE_GATE%16)) |
                              (0x00010000 << (PMU_RAM0_GATE%16))          |
                              (0x00010000 << (PMU_RAM1_GATE%16))          |
                              (0x00010001 << (CLK_RESERVED4%16))          |
                              (0x00010001 << (PVTM_CLK_GATE%16))          ;

    CRU->CRU_CLKGATE_CON[6] = (0x00010000 << (DRAM0_GATE%16))          |
                              (0x00010000 << (DRAM1_GATE%16))          |
                              (0x00010000 << (DRAM2_GATE%16))          |
                              (0x00010000 << (DRAM3_GATE%16))          |
                              (0x00010000 << (DRAM4_GATE%16))          |
                              (0x00010000 << (DRAM5_GATE%16))          |
                              (0x00010000 << (DRAM6_GATE%16))          |
                              (0x00010000 << (DRAM7_GATE%16))          |
                              (0x00010000 << (DRAM_INTERFACE_GATE%16)) |
                              (0x00010000 << (HCLK_APB_BRG_GATE%16))   |
                              (0x00010001 << (HCLK_USBC_GATE%16))      |
                              (0x00010001 << (HCLK_SDMMC_GATE%16))     |
                              (0x00010001 << (HCLK_EMMC_GATE%16))      |
                              (0x00010001 << (HCLK_EBC_GATE%16))       |
                              (0x00010001 << (HCLK_SYNTH_GATE%16))     |
                              (0x00010001 << (HCLK_SFC_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[7] = (0x00010000 << (IRAM0_GATE%16))          |
                              (0x00010000 << (IRAM1_GATE%16))          |
                              (0x00010000 << (IRAM2_GATE%16))          |
                              (0x00010000 << (IRAM3_GATE%16))          |
                              (0x00010000 << (IRAM4_GATE%16))          |
                              (0x00010000 << (IRAM5_GATE%16))          |
                              (0x00010000 << (IRAM6_GATE%16))          |
                              (0x00010000 << (IRAM7_GATE%16))          |
                              (0x00010000 << (IRAM8_GATE%16))|
                              (0x00010000 << (IRAM9_GATE%16))   |
                              (0x00010000 << (IRAM_INTERFACE_GATE%16)) |
                              (0x00010001 << (HCLK_BOOTROM_GATE%16))   |
                              (0x00010000 << (HCLK_1TOM_BRG_GATE%16))  |
                              (0x00010000 << (HCLK_DMA_GATE%16))       |
                              (0x00010001 << (HCLK_IMDCT_GATE%16))     |
                              (0x00010001 << (HCLK_VOP_GATE%16))       ;

    CRU->CRU_CLKGATE_CON[8] = (0x00010000 << (HCLK_PMU_APB_BRG_GATE%16))|
                              (0x00010001 << (PCLK_I2S0_GATE%16))       |
                              (0x00010001 << (PCLK_I2S1_GATE%16))       |
                              (0x00010001 << (PCLK_I2C0_GATE%16))       |
                              (0x00010001 << (PCLK_I2C1_GATE%16))       |
                              (0x00010001 << (PCLK_I2C2_GATE%16))       |
                              (0x00010001 << (PCLK_SPI0_GATE%16))       |
                              (0x00010001 << (PCLK_SPI1_GATE%16))       |
                              (0x00010001 << (PCLK_UART0_GATE%16))      |
                              (0x00010001 << (PCLK_UART1_GATE%16))      |
                              (0x00010001 << (PCLK_UART2_GATE%16))      |
                              (0x00010001 << (PCLK_UART3_GATE%16))      |
                              (0x00010001 << (PCLK_UART4_GATE%16))      |
                              (0x00010001 << (PCLK_UART5_GATE%16))      |
                              (0x00010001 << (PCLK_TIMER_GATE%16))      |
                              (0x00010001 << (PCLK_WDT_GATE%16))        ;

    CRU->CRU_CLKGATE_CON[9] = (0x00010001 << (PCLK_PWM0_GATE%16))      |
                              (0x00010001 << (PCLK_PWM1_GATE%16))      |
                              (0x00010001 << (PCLK_MAILBOX_GATE%16))   |
                              (0x00010001 << (PCLK_SARADC_GATE%16))    |
                              (0x00010000 << (PCLK_GPIO0_GATE%16))     |
                              (0x00010000 << (PCLK_GPIO1_GATE%16))     |
                              (0x00010001 << (PCLK_ACODEC_GATE%16))    |
                              (0x00010000 << (PCLK_ASYNC_BRG_GATE%16)) |
                              (0x00010000 << (PCLK_LGC_MATRIX_GATE%16));
}

_BSP_EVK_V20_BSP_INIT_
void CpuSoftResetInit(void)
{
    CRU->CRU_SOFTRST[0] = (0x00010001 << (USBOTG_SRST%16))  |
                          (0x00010001 << (USBPHY_SRST%16))  |
                          (0x00010001 << (USBGLB_SRST%16))  |
                          (0x00010001 << (I2S0_SRST%16))    |
                          (0x00010001 << (I2S1_SRST%16))    |
                          (0x00010001 << (SPI0_SRST%16))    |
                          (0x00010001 << (SPI1_SRST%16))    |
                          (0x00010001 << (UART0_SRST%16))   |
                          (0x00010001 << (UART1_SRST%16))   |
                          (0x00010001 << (UART2_SRST%16))   |
                          (0x00010001 << (BOOTROM_SRST%16)) |
                          (0x00010000 << (SYSRAM1_SRST%16)) |
                          (0x00010000 << (SYSRAM0_SRST%16)) |
                          (0x00010001 << (VOP_SRST%16))    |
                          (0x00010001 << (IMDCT_SRST%16))   |
                          (0x00010001 << (SYNTH_SRST%16))   ;

    CRU->CRU_SOFTRST[1] = (0x00010000 << (SYSDMA_SRST%16))   |
                          (0x00010001 << (SDMMC0_SRST%16))    |
                          (0x00010001 << (SDMMC1_SRST%16))     |
                          (0x00010000 << (SYSMATRIX_SRST%16))|
                          (0x00010001 << (TIMER0_SRST%16))   |
                          (0x00010001 << (TIMER1_SRST%16))   |
                          (0x00010001 << (SARADC_SRST%16))   |
                          (0x00010001 << (PWM0_SRST%16))     |
                          (0x00010001 << (PWM1_SRST%16))     |
                          (0x00010001 << (SFC_SRST%16))      |
                          (0x00010001 << (I2C0_SRST%16))     |
                          (0x00010001 << (I2C1_SRST%16))     |
                          (0x00010001 << (I2C2_SRST%16))     |
                          (0x00010001 << (EBC_SRST%16))      |
                          (0x00010001 << (MAILBOX_SRST%16))  |
                          (0x00010001 << (WDT_SRST%16))      ;

    CRU->CRU_SOFTRST[2] = (0x00010001 << (HIGH_MATRIX_SRST%16)) |
                          (0x00010001 << (HIGHRAM1_SRST%16))    |
                          (0x00010001 << (HIGHRAM0_SRST%16))    |
                          (0x00010001 << (HIFI_SRST%16))        |
                          (0x00010001 << (RESERVED0%16))        |
                          (0x00010001 << (UART3_SRST%16))       |
                          (0x00010001 << (UART4_SRST%16))       |
                          (0x00010001 << (UART5_SRST%16))       |
                          (0x00010000 << (GPIO0_SRST%16))       |
                          (0x00010000 << (GPIO1_SRST%16))       ;

    CRU->CRU_SOFTRST[3] = (0x00010001 << (ACODEC_SRST0%16))    |
                          (0x00010000 << (SYS_CORE_SRST%16))  |
                          (0x00010001 << (CAL_CORE_SRST%16))  |
                          (0x00010000 << (BRG_TO_PMU_SRST%16))|
                          (0x00010001 << (RESERVED1%16))      |
                          (0x00010000 << (PMU_SRST%16))       |
                          (0x00010001 << (ASYNC_BRG_SRST%16)) |
                          (0x00010001 << (DMA2_SRST%16))      ;

}
/*******************************************************************************
** Name: GpioInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:56:53
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT FUN void GpioInit(void)
{
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin0,IOMUX_GPIO0A0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin1,IOMUX_GPIO0A1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin2,IOMUX_GPIO0A2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin3,IOMUX_GPIO0A3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin4,IOMUX_GPIO0A4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin5,IOMUX_GPIO0A5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin6,IOMUX_GPIO0A6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortA_Pin7,IOMUX_GPIO0A7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortA_Pin7,DISABLE);

    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin0,IOMUX_GPIO0B0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin1,IOMUX_GPIO0B1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin2,IOMUX_GPIO0B2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin3,IOMUX_GPIO0B3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin4,IOMUX_GPIO0B4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin5,IOMUX_GPIO0B5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin6,IOMUX_GPIO0B6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortB_Pin7,IOMUX_GPIO0B7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortB_Pin7,DISABLE);


    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin0,IOMUX_GPIO0C0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin1,IOMUX_GPIO0C1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin2,IOMUX_GPIO0C2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin3,IOMUX_GPIO0C3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin4,IOMUX_GPIO0C4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin5,IOMUX_GPIO0C5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin6,IOMUX_GPIO0C6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortC_Pin7,IOMUX_GPIO0C7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortC_Pin7,DISABLE);

    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin0,IOMUX_GPIO0D0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH0,GPIOPortD_Pin1,IOMUX_GPIO0D1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH0,GPIOPortD_Pin1,DISABLE);

    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin0,IOMUX_GPIO1A0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin1,IOMUX_GPIO1A1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin2,IOMUX_GPIO1A2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin3,IOMUX_GPIO1A3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin4,IOMUX_GPIO1A4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin5,IOMUX_GPIO1A5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin6,IOMUX_GPIO1A6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortA_Pin7,IOMUX_GPIO1A7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortA_Pin7,DISABLE);

    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin0,IOMUX_GPIO1B0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin1,IOMUX_GPIO1B1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin2,IOMUX_GPIO1B2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH1,GPIOPortB_Pin3,IOMUX_GPIO1B3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH1,GPIOPortB_Pin3,DISABLE);


    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin0,IOMUX_GPIO2A0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin1,IOMUX_GPIO2A1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin2,IOMUX_GPIO2A2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin4,IOMUX_GPIO2A4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin5,IOMUX_GPIO2A5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin6,IOMUX_GPIO2A6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin7,IOMUX_GPIO2A7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortA_Pin7,DISABLE);

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin0,IOMUX_GPIO2B0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin1,IOMUX_GPIO2B1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin1,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin2,IOMUX_GPIO2B2_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin2,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin3,IOMUX_GPIO2B3_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin3,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin4,IOMUX_GPIO2B4_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin4,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin5,IOMUX_GPIO2B5_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin5,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin6,IOMUX_GPIO2B6_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin6,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortB_Pin7,IOMUX_GPIO2B7_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortB_Pin7,DISABLE);

    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin0,IOMUX_GPIO2C0_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin0,DISABLE);
    Grf_GpioMuxSet(GPIO_CH2,GPIOPortC_Pin1,IOMUX_GPIO2C1_IO);
    Grf_GPIO_SetPinPull(GPIO_CH2,GPIOPortC_Pin1,DISABLE);

    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortA_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortB_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortC_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH0,GPIOPortD_Pin1,GPIO_IN);

#ifdef _SUPPORT_PA_EN
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin0,GPIO_OUT);//jjjhhh---PA_EN
	Gpio_SetPinLevel(GPIO_CH1, GPIOPortA_Pin0, GPIO_LOW);
#else
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin0,GPIO_IN);
#endif
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortA_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH1,GPIOPortB_Pin2,GPIO_IN);


    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortA_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin1,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin2,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin3,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin4,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin5,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin6,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortB_Pin7,GPIO_IN);

    Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin0,GPIO_IN);
    Gpio_SetPinDirection(GPIO_CH2,GPIOPortC_Pin1,GPIO_IN);

}
/*******************************************************************************
** Name: CPUInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:55:46
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT FUN void CPUInit(void)
{
    //Scu_OOL_Power_Set(1);    //power on

    //ScuLDOEnable();
    //ScuDCoutEnable();

    //ScuDCoutSet(SCU_DCOUT_120);
    //ScuLDOSet(SCU_LDO_30);

    //Scu_Force_PWM_Mode_Set(0);
    //Scu_Threshold_Set(PFM_26MA);
    //Scu_Battery_Sns_Set(1);

    //FREQ_EnterModule(FREQ_BLON);

    //ScuIpCoreInit();
    //ScuClockGateInit();
    CpuSoftResetInit();
    CpuClockGateInit();


    //Scu_Memory_Set_High_Speed(LDRAM_RESP_CYCLE);

}
/*******************************************************************************
** Name: SysTickInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:55:04
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT FUN void SysTickInit(void)
{
    SysTickDisable();

    SysTickClkSourceSet(NVIC_SYSTICKCTRL_CLKIN);
    SysTickPeriodSet(10);

    SysTickEnable();
}
/*******************************************************************************
** Name: SysVariableInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:54:12
*******************************************************************************/
_BSP_EVK_V20_BSP_INIT_
INIT FUN void SysVariableInit(void)
{
    SysTickCounter   = 0;
    SysTickCounterSave =0;

    chip_freq.pll = 0;

    UartHDC = NULL;
    pIdleFun = CpuIdle;
    pIdleFunSave = CpuIdle;

    SysState = LEVEL_INIT;
    SysWakeCnt = 0;
}
/*******************************************************************************
** Name: SysInterruptInit
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 15:29:42
*******************************************************************************/
extern void xPortPendSVHandler( void );
extern void xPortSysTickHandler( void );
extern void vPortSVCHandler( void );

_BSP_EVK_V20_BSP_INIT_
INIT FUN void SysInterruptInit(void)
{
    IntMasterEnable();

    //set priority.
    IrqPriorityInit();
    //......

    //register interrupt functions.
    IntRegister(FAULT_ID11_SVCALL, (void*)vPortSVCHandler);
    IntRegister(FAULT_ID15_SYSTICK, (void*)xPortSysTickHandler);
    IntRegister(FAULT_ID14_PENDSV, (void*)xPortPendSVHandler);;

    //interrupt enable.
    //IntEnable(FAULT_ID15_SYSTICK);
    //IntEnable(FAULT_ID14_PENDSV);
    //IntEnable(FAULT_ID11_SVCALL);

    IntDisable(FAULT_ID15_SYSTICK);

}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif
