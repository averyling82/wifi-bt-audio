/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                  Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*File    : device.C
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/
#include "BspConfig.h"

#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
rk_err_t  RKDeviceRegister(DEVICE_CLASS *pDev);
rk_err_t  RKDeviceUnRegister(DEVICE_CLASS* RKDev);


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define DEV_ORIGINAL_ADDRESS  0x0fffffff

__attribute__((section("DevInfo_Table")))
DEV_INFO DevInfo_Table[DEV_CLASS_MAX_NUM] =
{
    //add new device info here
    //......
#ifdef __DRIVER_FM_FMDEVICE_C__
    SEGMENT_ID_FM_INIT,FmDev_Create,FmDev_Delete,"fm",
#endif
#ifdef __DRIVER_PWM_PWMDEVICE_C__
    SEGMENT_ID_PWM_INIT,PwmDev_Create,PwmDev_Delete,"pwm",
#endif
#ifdef __DRIVER_DISPLAY_DISPLAYDEVICE_C__
    SEGMENT_ID_DISPLAY_INIT,DisplayDev_Create,DisplayDev_Delete,"display",
#endif
#ifdef __DRIVER_KEY_KEYDEVICE_C__
    SEGMENT_ID_KEY_INIT,KeyDev_Create,KeyDev_Delete,"key",
#endif
#ifdef __DRIVER_AD_ADCDEVICE_C__
    SEGMENT_ID_ADC_INIT,ADCDev_Create,ADCDev_Delete,"adc",
#endif
#ifdef __DRIVER_TIMER_TIMERDEVICE_C__
    SEGMENT_ID_TIMER_INIT,TimerDev_Create,TimerDev_Delete,"timer",
#endif
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__
    NULL,SpiFlashDev_Create,SpiFlashDev_Delete,"spiflash",
#endif
#ifdef __DRIVER_BCORE_BCOREDEVICE_C__
    SEGMENT_ID_BCORE_INIT,BcoreDev_Create,BcoreDev_Delete,"bcore",
#endif
#ifdef __DRIVER_MSG_MSGDEVICE_C__
    SEGMENT_ID_MSG_INIT,MsgDev_Create,MsgDev_Delete,"msg",
#endif
#ifdef __DRIVER_FILE_DIRDEVICE_C__
    SEGMENT_ID_DIR_INIT,DirDev_Create,DirDev_Delete,"dir",
#endif
#ifdef __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__
    SEGMENT_ID_ROCKCODEC_INIT,RockCodecDev_Create,RockCodecDev_Delete,"acodec",
#endif
#ifdef __DRIVER_I2S_I2SDEVICE_C__
    SEGMENT_ID_I2S_INIT,I2SDev_Create,I2SDev_Delete,"i2s",
#endif
#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
    SEGMENT_ID_AUDIO_INIT,AudioDev_Create,AudioDev_Delete,"audio",
#endif
#ifdef __DRIVER_USBMSC_USBMSCDEVICE_C__
    SEGMENT_ID_USBMSC_INIT,USBMSCDev_Create,USBMSCDev_Delete,"usbmsc",
#endif
#ifdef __DRIVER_MAILBOX_MAILBOXDEVICE_C__
    SEGMENT_ID_MAILBOX_INIT,MailBoxDev_Create,MailBoxDev_Delete,"mailbox",
#endif
#ifdef __DRIVER_WATCHDOG_WATCHDOGDEVICE_C__
    SEGMENT_ID_WDOG_INIT,WDTDev_Create,NULL,"wdog",
#endif

#ifdef __DRIVER_WM8987_WM8987DEVICE_C__
    NULL,WM8987Dev_Create,NULL,"wm8987",
#endif
#ifdef __DRIVER_RK61X_RK618DEVICE_C__
    SEGMENT_ID_RK618_INIT,RK618Dev_Create,RK618Dev_Delete,"rk618",
#endif
#ifdef __DRIVER_SD_SDDEVICE_C__
    SEGMENT_ID_SD_INIT,SdDev_Create,SdDev_Delete,"sd",
#endif
#ifdef __DRIVER_I2C_I2CDEVICE_C__
    SEGMENT_ID_I2C_INIT,I2CDev_Create,I2CDev_Delete,"i2c",
#endif
#ifdef __DRIVER_USB_USBOTGDEV_C__
    SEGMENT_ID_USBOTG_INIT,UsbOtgDev_Create,UsbOtgDev_Delete,"usbotg",
#endif
#ifdef __DRIVER_WLC_WLCCD_C__
    NULL,WLCCCDev_Create,NULL,"wlccc",
#endif
#ifdef __DRIVER_RK903_RK903DEVICE_C__
    SEGMENT_ID_RK903_INIT,Rk903Dev_Create,NULL,"rk903",
#endif
#ifdef  __DRIVER_WLC_WLCDEVICE_C__
    SEGMENT_ID_WLC_INIT,WlcDev_Create,NULL,"wlc",
#endif
#ifdef __DRIVER_SDIO_SDIODEVICE_C__
    SEGMENT_ID_SDIO_INIT,SdioDev_Create,SdioDevDelete,"sdio",
#endif
#ifdef __DRIVER_SPINOR_SPINORDEVICE_C__
    NULL,SpiNorDev_Create,NULL,"spinor",
#endif
#ifdef __DRIVER_SPI_SPIDEVICE_C__
    NULL,SpiDev_Create,NULL,"spi",
#endif
#ifdef __DRIVER_FIFO_FIFODEVICE_C__
    SEGMENT_ID_FIFO_INIT,fifoDev_Create,fifoDev_Delete,"fifo",
#endif
#ifdef __DRIVER_EXAMPLE_DEVICEFORMAT_C__
    NULL,TestDev_Create,NULL,"test",
#endif
#ifdef __DRIVER_UART_UARTDEVICE_C__
    SEGMENT_ID_UART_INIT,UartDev_Create,UartDev_Delete,"uart",
#endif
#ifdef __DRIVER_DMA_DMADEVICE_C__
    NULL,DmaDev_Create,DmaDevDelete,"dma",
#endif

#ifdef _SPI_BOOT_
#ifdef __DRIVER_SDMMC_SDMMCDEVICE_C__
    SEGMENT_ID_SDC_INIT,SdcDev_Create,SdcDevDelete,"sdc",
#endif
#endif

#ifdef _EMMC_BOOT_
#ifdef __DRIVER_SDMMC_SDMMCDEVICE_C__
    NULL,SdcDev_Create,SdcDevDelete,"sdc",
#endif
#endif

#ifdef __DRIVER_EMMC_EMMCDEVICE_C__
    NULL,EmmcDev_Create,EmmcDevDelete,"emmc",
#endif
#ifdef __DRIVER_LUN_LUNDEVICE_C__
    SEGMENT_ID_LUN_INIT,LunDev_Create,LunDev_Delete,"lun",
#endif
#ifdef __DRIVER_PARTION_PARTIONDEVICE_C__
    SEGMENT_ID_PAR_INIT,ParDev_Create,ParDev_Delete,"par",
#endif
#ifdef __DRIVER_VOLUME_FATDEVICE_C__
    SEGMENT_ID_FAT_INIT,FatDev_Create,FatDev_Delete,"fat",
#endif
#ifdef __DRIVER_FILE_FILEDEVICE_C__
    SEGMENT_ID_FILE_INIT,FileDev_Create,FileDevDelete,"file",
#endif
#ifdef __DRIVER_ALC5633_ALC5633DEVICE_C__
    NULL,ALC5633Dev_Create,NULL,"alc5633",
#endif
#ifdef __DRIVER_VOP_VOPDEVICE_C__
    SEGMENT_ID_VOP_INIT,VopDev_Create,VopDev_Delete,"vop",
#endif
#ifdef __DRIVER_LCD_LCDDEVICE_C__
    SEGMENT_ID_LCD_INIT,LcdDev_Create,LcdDev_Delete,"lcd",
#endif
};

static DEVICE_CLASS *DevListHead = NULL;
static pSemaphore osDeviceReqSem;
uint32 DevTotalCnt;
uint32 DevTotalSuspendCnt;



/*
*********************************************************************************************************
*                                RKDeviceHandler RKDeviceFind(const char *pDeviceName)
*
* Description:  任务通过设备名称找到设备
*
* Argument(s) : const char *pDeviceName
*
* Return(s)   : RKDeviceHandler
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t RKDeviceFind(uint32 DevClassID,uint32 DevID)
{
    DEVICE_CLASS *pDev = DevListHead;

    while (pDev != NULL)
    {
        if ((pDev->DevClassID == DevClassID)&&(pDev->DevID == DevID))
            return (rk_err_t)pDev;
        pDev = pDev->next;

    }

    return RK_ERROR;
}

HDC RKDev_GetFirstHandler(uint32 DevClassID)
{
    if(DevClassID == 0xffffffff)
    {
        return DevListHead;
    }
}


HDC RKDev_GetNexttHandler(HDC dev, uint32 DevClassID)
{
    DEVICE_CLASS* pDev = (DEVICE_CLASS*)dev;
    if(DevClassID == 0xffffffff)
    {
        return pDev->next;
    }
}


/*
*********************************************************************************************************
*                                 rk_err_t RKDeviceRegister(uint DevNO,char *pDeviceName,RKDeviceHandler (*DevCreate)(void))
*
* Description :  向操作系统注册设备.
*
* Argument(s) : uint DevNO,char *pDeviceName,RKDeviceHandler (*DevCreate)(void)
*
* Return(s)   : rk_err_t
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t RKDeviceRegister(DEVICE_CLASS *pDev)
{
    DEVICE_CLASS *pDevTemp;

    if(DevListHead == NULL)
    {
        DevListHead = pDev;
    }
    else
    {
        pDevTemp = DevListHead;
        while(pDevTemp->next != NULL)
        {
            pDevTemp = pDevTemp->next;
        }
        pDevTemp->next = pDev;
    }
    DevTotalCnt++;
    return RK_SUCCESS;
}

/*
*********************************************************************************************************
*                                   rk_err_t RKDeviceUnRegister(RKDeviceHandler RKDev)
*
* Description:  将设备从链表中删除.
*
* Argument(s) : none
*
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/
rk_err_t RKDeviceUnRegister(DEVICE_CLASS* pDev)
{
    DEVICE_CLASS *pDevTemp = DevListHead;
    DEVICE_CLASS *pLastDevTemp = NULL;

    while (pDevTemp != NULL)
    {
        if (pDevTemp == pDev)
        {
            if(pLastDevTemp == NULL)
            {
                DevListHead = NULL;
            }
            else
            {
                pLastDevTemp->next = pDev->next;
            }

            DevTotalCnt--;
            return RK_SUCCESS;
        }
        pLastDevTemp = pDevTemp;
        pDevTemp = pDevTemp->next;
    }

    return RK_ERROR;
}

/*
*********************************************************************************************************
*                                   rk_err_t RKDeviceUnRegister(RKDeviceHandler RKDev)
*
* Description:  将设备从链表中删除.
*
* Argument(s) : none
*
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/
COMMON API rk_err_t RKDev_Create(uint32 DevClassID, uint32 DevID, void* arg)
{
    DEVICE_CLASS* pDev;
    rk_err_t ret;

    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);

    ret = RKDeviceFind(DevClassID, DevID);

    if (ret == RK_ERROR)
    {
        if((DevClassID != DEV_CLASS_LUN) || (DevID != 0))
        {
            if(DevInfo_Table[DevClassID].SegmentID != 0)
            {
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_LoadSegment(DevInfo_Table[DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                #endif
            }
        }

        pDev = DevInfo_Table[DevClassID].pfCreate(DevID, arg);


        if((DevClassID != DEV_CLASS_LUN) || (DevID != 0))
        {
            if(DevInfo_Table[DevClassID].SegmentID != 0)
            {
                #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                FW_RemoveSegment(DevInfo_Table[DevClassID].SegmentID);
                #endif
            }
        }

        if((rk_err_t)pDev <= 0)
        {
            rkos_semaphore_give(osDeviceReqSem);
            return RK_ERROR;
        }

        pDev->UseCnt = 0;
        pDev->DevClassID = DevClassID;
        pDev->DevID = DevID;
        pDev->next = NULL;
        RKDeviceRegister(pDev);
        rkos_semaphore_give(osDeviceReqSem);
        return RK_SUCCESS;
    }
    else
    {
        rk_printf("already exist");
        rkos_semaphore_give(osDeviceReqSem);
        return RK_ERROR;
    }
}


COMMON API HDC RKDev_Open(uint8 DevClassID, uint8 DevID, uint32 DevMode)
{
    DEVICE_CLASS* pDev;
    rk_err_t ret;

    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);
    ret = RKDeviceFind(DevClassID,DevID);

    if (ret == RK_ERROR)
    {
        rkos_semaphore_give(osDeviceReqSem);
        return NULL;
    }
    else
    {
        pDev = (DEVICE_CLASS *)ret;
        pDev->UseCnt++;
        pDev = RK_DevToCompetence(pDev, DevMode);
    }
    rkos_semaphore_give(osDeviceReqSem);

    if(pDev->State !=  DEV_STATE_WORKING)
    {
        DeviceTask_ResumeDevice(pDev->DevClassID, pDev->DevID, NULL, SYNC_MODE);
    }
    return pDev;

}

COMMON API rk_err_t RKDev_Close(HDC dev)
{
    DEVICE_CLASS * pDev = (DEVICE_CLASS *)((uint32)dev & DEV_ORIGINAL_ADDRESS);

    if(pDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);
    if(pDev->UseCnt > 0)
        pDev->UseCnt--;
    rkos_semaphore_give(osDeviceReqSem);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name:
** Input:
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.2.24
** Time: 10:47:31
*******************************************************************************/
COMMON API HDC RK_CompetenceToDev(HDC dev, uint32 Mode)
{
    HDC ExclusiveDev = NULL;

    if((dev != NULL) && (((uint32)dev & Mode) || (((uint32)dev & 0x07000000) == 0)))
    {
        //ExclusiveDev = (HDC)((uint32)dev &= DEV_ORIGINAL_ADDRESS); //[GCC]:error: lvalue required as left operand of assignment
        dev = (HDC)((uint32)dev & DEV_ORIGINAL_ADDRESS);
        ExclusiveDev = dev;
        return ExclusiveDev;
    }
    else
    {
        return NULL;
    }

}

/*******************************************************************************
** Name:
** Input:
** Return: rk_err_t
** Owner:zhuzhe
** Date: 2014.2.24
** Time: 10:47:31
*******************************************************************************/
COMMON API HDC RK_DevToCompetence(HDC dev,uint32 Mode)
{
    HDC ExclusiveDev = NULL;
    if (dev != NULL)
    {
        //ExclusiveDev = (HDC)((uint32)dev |= Mode); //[GCC]:error: lvalue required as left operand of assignment
        dev = (HDC)((uint32)dev | Mode);
        ExclusiveDev = dev;
        return ExclusiveDev;
    }
    else
    {
        return dev;
    }
}

COMMON API rk_err_t  RKDev_Suspend(HDC* dev)
{
    DEVICE_CLASS* pDev = (DEVICE_CLASS *)((uint32)dev & DEV_ORIGINAL_ADDRESS);

    rk_err_t ret;
    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);
    if(pDev->SuspendCnt < pDev->UseCnt)
    {
        pDev->SuspendCnt++;
        pDev->IdleTick = 0;
    }
    rkos_semaphore_give(osDeviceReqSem);
    return ret;
}


COMMON API rk_err_t  RKDev_Resume(HDC* dev)
{
    DEVICE_CLASS* pDev = (DEVICE_CLASS *)((uint32)dev & DEV_ORIGINAL_ADDRESS);
    rk_err_t ret;

    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);
    if(pDev->SuspendCnt > 0)
    {
         pDev->SuspendCnt--;
    }
    rkos_semaphore_give(osDeviceReqSem);
    return ret;
}


COMMON API rk_err_t  RKDev_Delete(uint32 DevClassID, uint32 DevID, void * arg)
{
    DEVICE_CLASS* pDev;
    rk_err_t ret;

    rkos_semaphore_take(osDeviceReqSem, MAX_DELAY);
    ret = RKDeviceFind(DevClassID, DevID);

    if (ret != RK_ERROR)
    {
        pDev = (DEVICE_CLASS *)ret;

        if(pDev->UseCnt != 0)
        {
            rk_printf("pDev->UseCnt = %d", pDev->UseCnt);
            rkos_semaphore_give(osDeviceReqSem);
            return RK_ERROR;
        }

        if(DevInfo_Table[DevClassID].SegmentID != 0)
        {
            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
            FW_LoadSegment(DevInfo_Table[DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
            #endif
        }

        RKDeviceUnRegister(pDev);

        ret = DevInfo_Table[DevClassID].pfDelete(DevID, arg);

        if(DevInfo_Table[DevClassID].SegmentID != 0)
        {
            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
            FW_RemoveSegment(DevInfo_Table[DevClassID].SegmentID);
            #endif
        }

        if(ret != RK_SUCCESS)
        {
            rk_printf("device delete fail  Classid = %d, objectid = %d", DevClassID, DevID);
            RKDeviceRegister(pDev);
            rkos_semaphore_give(osDeviceReqSem);
            return RK_ERROR;
        }

        rkos_semaphore_give(osDeviceReqSem);
        return RK_SUCCESS;
    }
    else
    {
        rk_printf("no find device classid = %d, objectid = %d", DevClassID, DevID);
        rkos_semaphore_give(osDeviceReqSem);
        return RK_ERROR;
    }
}


COMMON API rk_err_t  RKDev_Init(void)
{
    DevListHead = NULL;
    osDeviceReqSem  = rkos_semaphore_create(1,1);

    DevTotalCnt = 0;
    DevTotalSuspendCnt = 0;

    if(osDeviceReqSem == NULL)
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;
}


COMMON API void  RKDev_IdleCheck(void)
{
    DEVICE_CLASS *pDev = DevListHead;
    uint32 es;

    es = 1;

    while (pDev != NULL)
    {
        pDev->IdleTick += PM_TIME;

        if(es)
        {
            if((pDev->IdleTick >= pDev->Idle1EventTime)
                &&(pDev->SuspendCnt == pDev->UseCnt)
                && (pDev->State == DEV_STATE_WORKING)
                && (pDev->SuspendMode != DISABLE_MODE))
            {
                //suspent this device
                if(pDev->suspend != NULL)
                {
                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                            #endif
                        }
                    }

                    if(pDev->suspend(pDev, DEV_STATE_IDLE1) != RK_SUCCESS)
                    {
                        rk_printf("enable suspend fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                    else
                    {
                        rk_printf("enable %s[%d] suspend ok",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                        DevTotalSuspendCnt++;
                        es = 0;
                    }

                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                            #endif
                        }
                    }
                }

                es = 0;
            }
            else if((pDev->IdleTick >= pDev->Idle1EventTime)
                && (pDev->SuspendMode == FORCE_MODE)
                && (pDev->State == DEV_STATE_WORKING))
            {
                 //suspent this device
                if(pDev->suspend != NULL)
                {
                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                            #endif
                        }
                    }

                    if(pDev->suspend(pDev, DEV_STATE_IDLE1) != RK_SUCCESS)
                    {
                        rk_printf("force suspend fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                    else
                    {
                        rk_printf("force %s[%d] suspend ok",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                        DevTotalSuspendCnt++;
                        es = 0;
                    }

                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                            #endif
                        }
                    }
                }

            }
            #if 0
            else if((pDev->IdleTick >= pDev->Idle2EventTime)
                && (pDev->State == DEV_STATE_IDLE1))
            {
                 //suspent this device
                if(pDev->suspend != NULL)
                {
                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_LoadSegment(DevInfo_Table[pDev->DevClassID].SegmentID, SEGMENT_OVERLAY_CODE);
                            #endif
                        }
                    }

                    if(pDev->suspend(pDev, DEV_SATE_IDLE2) != RK_SUCCESS)
                    {
                        rk_printf("idle3 suspend fail = %s[%d]",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }
                    else
                    {
                        rk_printf("idle3 %s[%d] suspend ok",DevInfo_Table[pDev->DevClassID].DeviceName, pDev->DevID);
                    }

                    if((pDev->DevClassID != DEV_CLASS_LUN) || (pDev->DevID != 0))
                    {
                        if(DevInfo_Table[pDev->DevClassID].SegmentID != 0)
                        {
                            #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                            FW_RemoveSegment(DevInfo_Table[pDev->DevClassID].SegmentID);
                            #endif
                        }
                    }
                }
                es = 0;
            }
            #endif

        }

        pDev = pDev->next;
    }

    return;
}

