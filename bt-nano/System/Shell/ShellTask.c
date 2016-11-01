/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: System\Shell\ShellTask.c
* Owner: aaron.sun
* Date: 2015.10.20
* Time: 14:14:04
* Version: 1.0
* Desc: shell task
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.10.20     14:14:04   1.0
********************************************************************************************
*/
#include "BspConfig.h"
#ifdef __SYSTEM_SHELL_SHELLTASK_C__
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "platform.h"
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "AppInclude.h"
#include "ModuleFormat.h"
#include "LCDDriver.h"
#include "FwAnalysis.h"
#include "GUITask.h"
#include "music.h"
#include "ap6181wifi.h"
#include "ShellDevCmd.h"
#include "ShellSysCmd.h"
#include "ShellTaskCmd.h"
#include "ShellCustomCmd.h"
#include "record.h"
#include "FwAnalysis.h"
#include "airplay.h"
#include "lwipopts.h"
#include "shell_switch_player.h"//jjjhhh 20161016


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define SHELL_RX_BUF_MAX_SIZE                128
#define SHELL_CMD_MAX_ITEM                   1

typedef struct _SHELL_CMD_ITEM
{
    uint32 useflag;
    UINT32 SegmentID;
    uint8 ShellCmdName[12];
    SHELL_PARASE_FUN ShellCmdParaseFun;
    uint8 * CmdDes;

}SHELL_CMD_ITEM;


typedef struct SHELL_TASK_DATA_BLOCK
{
    uint8 ShellRxBuffer[10][SHELL_RX_BUF_MAX_SIZE];
    uint32 Index;
    uint32 OldIndex;
    uint32 ShellRxStart;
    HDC hUart;
    SHELL_CMD_ITEM CmdHeadItem[SHELL_CMD_MAX_ITEM];

}SHELL_TASK_DATA_BLOCK;

typedef struct _SHELL_CMD_INFO
{
    uint8 ShellCmdName[12];
    uint32 SegmentID;
    SHELL_PARASE_FUN ShellCmdParaseFun;
    uint8 * CmdSampleDes;
    uint8 * CmdDes;

}SHELL_CMD_INFO;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t ShellTaskCheckIdle(HTC hTask);
rk_err_t ShellTaskResume(HTC hTask);
rk_err_t ShellTaskSuspend(HTC hTask, uint32 Level);
uint32 ShellCmdGetCnt(SHELL_CMD * cmd);
rk_err_t ShellPcb(HDC dev, uint8 * pstr);
rk_err_t ShellHelp(HDC dev, uint8 * pstr);
rk_err_t ShellCmdRegister(HDC dev, uint8 * pItemName);
rk_err_t ShellCmdRemove(HDC dev, uint8 * pItemName);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_SYSTEM_SHELL_SHELLTASK_COMMON_
static SHELL_CMD ShellRootName[] =
{
    "connect",ShellCmdRegister,"load shell cmd","NULL",
    "remove",ShellCmdRemove,"reload shell cmd","NULL",
    "pcb",ShellPcb,"display shell task pcb information","pcb format as follow:\r\n pcb",
    "help",ShellHelp,"help cmd","NULL",
    "\b",NULL,"NULL","NULL",                          // the end
};

rk_err_t i6000_wifi_dev_shell(HDC dev, uint8 * pstr);
__attribute__((used, section("ShellRegisterName")))
const static SHELL_CMD_INFO ShellRegisterName1[] =
{
    //add new shell cmd here
    //...
#ifdef __DRIVER_FM_FMDEVICE_C__
#ifdef _FM_DEV_SHELL_
    "fm",SEGMENT_ID_FM_SHELL,FmDev_Shell,"fm device cmd package","FMDevice Functional test entry",
#endif
#endif
#ifdef __DRIVER_I6000_WIFI_C__
#ifdef _I6000_WIFI_SHELL_
    "i6000",SEGMENT_ID_I6000_WIFI_SHELL,i6000_wifi_dev_shell,"i6000 cmd package","NULL",
#endif
#endif
#ifdef USE_LWIP
#ifdef _IP_SHELL_
    "ip",SEGMENT_ID_IPCONFIG_SHELL,IP_config_shell,"ip config cmd package","NULL",
#endif
#endif
#ifdef _BLUETOOTH_
#ifdef _BLUETOOTH_SHELL_
    "bt",SEGMENT_ID_BLUETOOTH_SHELL,BlueTooth_Shell,"blueetooh cmd package","NULL",
#endif
#endif
#ifdef __SYSTEM_SHELL_SHELLCUSTOMCMD_C__
    "custom",SEGMENT_ID_CUSTOM_CMD,ShellCustomParsing,"user custom cmd package","NULL",
#endif
#ifdef __SYSTEM_SHELL_SHELLDEVCMD_C__
    "dev",SEGMENT_ID_DEV_CMD,ShellDeviceParsing,"rkos device manager cmd packaage","NULL",
#endif
#ifdef __SYSTEM_SHELL_SHELLTASKCMD_C__
    "task",SEGMENT_ID_TASK_CMD,ShellTaskParsing,"rkos thread manager cmd package","NULL",
#endif
#ifdef __SYSTEM_SHELL_SHELLSYSCMD_C__
    "system",SEGMENT_ID_SYS_CMD,ShellSystemParsing,"rkos system cmd package","NULL",
#endif
#ifdef __DRIVER_PWM_PWMDEVICE_C__
#ifdef _PWM_DEV_SHELL_
    "pwm",SEGMENT_ID_PWM_SHELL,PwmDev_Shell,"pwm device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_VOP_VOPDEVICE_C__
#ifdef _VOP_DEV_SHELL_
    "vop",SEGMENT_ID_VOP_SHELL,VopDev_Shell,"vop device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_DISPLAY_DISPLAYDEVICE_C__
#ifdef _DISPLAY_DEV_SHELL_
    "display",SEGMENT_ID_DISPLAY_SHELL,DisplayDev_Shell,"display device cmd package","NULL",
#endif
#endif
#ifdef __GUI_GUITASK_C__
#ifdef _GUI_SHELL_
    "gui",SEGMENT_ID_GUI_SHELL,GuiShell,"gui manager cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_KEY_KEYDEVICE_C__
#ifdef _KEY_DEV_SHELL_
    "key",SEGMENT_ID_KEY_SHELL,KeyDev_Shell,"key device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_AD_ADCDEVICE_C__
#ifdef _ADC_DEV_SHELL_
    "adc",SEGMENT_ID_ADC_SHELL,ADCDev_Shell,"adc device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_TIMER_TIMERDEVICE_C__
#ifdef _TIMER_DEV_SHELL_
    "timer",SEGMENT_ID_TIMER_SHELL,TimerDev_Shell,"timer device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__
#ifdef _SPIFLASH_DEV_SHELL_
    "spiflash",SEGMENT_ID_SPIFLASH_SHELL,SpiFlashDev_Shell,"spiflash device cmd package","NULL",
#endif
#endif

#ifdef __DRIVER_BCORE_BCOREDEVICE_C__
#ifdef _BCORE_DEV_SHELL_
    "bcore",SEGMENT_ID_BCORE_SHELL,BcoreDev_Shell,"rknanod b core device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_MSG_MSGDEVICE_C__
#ifdef _MSG_DEV_SHELL_
    "msg",SEGMENT_ID_MSG_SHELL,MsgDev_Shell,"message device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_FILE_DIRDEVICE_C__
#ifdef _DIR_DEV_SHELL_
    "dir",SEGMENT_ID_DIR_SHELL,DirDev_Shell,"dir for system device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__
#ifdef _RK_ACODE_SHELL_
    "rockcodec",SEGMENT_ID_ROCKCODEC_SHELL,RockCodecDev_Shell,"rknanod acode device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_I2S_I2SDEVICE_C__
#ifdef _I2S_DEV_SHELL_
    "i2s",SEGMENT_ID_I2S_SHELL,I2SDev_Shell,"i2s device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_AUDIO_AUDIODEVICE_C__
#ifdef _AUDIO_SHELL_
    "audio",SEGMENT_ID_AUDIO_SHELL,AudioDev_Shell,"audio device for play, record and fm cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_USBMSC_USBMSCDEVICE_C__
#ifdef _USBMSC_DEV_SHELL_
    "usbmsc",SEGMENT_ID_USBMSC_SHELL,USBMSCDev_Shell,"usb massstorage device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_MAILBOX_MAILBOXDEVICE_C__
#ifdef _MAILBOX_DEV_SHELL_
    "mailbox",SEGMENT_ID_MAILBOX_SHELL,MailBoxDev_Shell,"mail box device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_WATCHDOG_WATCHDOGDEVICE_C__
#ifdef _WDT_DEV_SHELL_
    "wdt", SEGMENT_ID_WDOG_SHELL, WDTDev_Shell,"watch dog device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_WM8987_WM8987DEVICE_C__
#ifdef _WM8987_DEV_SHELL_
    "8987",NULL,WM8987Dev_Shell,"8987 codec device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_RK61X_RK618DEVICE_C__
#ifdef _RK618_DEV_SHELL_
    "rk618",SEGMENT_ID_RK618_SHELL,RK618Dev_Shell,"rk618 device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SD_SDDEVICE_C__
#ifdef _SD_DEV_SHELL_
    "sd",SEGMENT_ID_SD_SHELL,SdDev_Shell,"sd card device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_I2C_I2CDEVICE_C__
#ifdef _I2C_DEV_SHELL_
    "i2c",SEGMENT_ID_I2C_SHELL,I2CDev_Shell,"i2c controller device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_USB_USBOTGDEV_C__
#ifdef _USBOTG_DEV_SHELL_
    "usbotg",SEGMENT_ID_USBOTG_SHELL,UsbOtgDev_Shell,"usb otg device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SDIO_SDIODEVICE_C__
#ifdef _SDIO_DEV_SHELL_
    "sdio",SEGMENT_ID_SDIO_SHELL,SdioDev_Shell,"sdio device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SPINOR_SPINORDEVICE_C__
#ifdef _SPINOR_DEV_SHELL_
    "spinor",NULL,SpiNorDev_Shell,"spi nor flash device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SPI_SPIDEVICE_C__
#ifdef _SPI_DEV_SHELL_
     "spi",NULL,SpiDev_Shell,"spi controller device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_FIFO_FIFODEVICE_C__
#ifdef _FIFO_DEV_SHELL_
    "fifo",SEGMENT_ID_FIFO_SHELL,fifoDev_Shell,"rkos fifo device for network stream transport cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_SDMMC_SDMMCDEVICE_C__
#ifdef _SDEMMC_SHELL_
    "sdmmc",SEGMENT_ID_SDC_SHELL,SdcDev_Shell,"sd/mmsc controller device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_EMMC_EMMCDEVICE_C__
#ifdef _EMMC_DEV_SHELL_
    "emmc", SEGMENT_ID_EMMC_SHELL, EmmcDev_Shell,"emmc card device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_LUN_LUNDEVICE_C__
#ifdef _LUN_DEV_SHELL_
    "lun", SEGMENT_ID_LUN_SHELL, LunDev_Shell,"lun device for storage cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_PARTION_PARTIONDEVICE_C__
#ifdef _PAR_DEV_SHELL_
    "par", SEGMENT_ID_PAR_SHELL, ParDev_Shell,"partion device for file system cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_VOLUME_FATDEVICE_C__
#ifdef _FAT_SHELL_
    "fat", SEGMENT_ID_FAT_SHELL, FatDev_Shell,"fat file system cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_FILE_FILEDEVICE_C__
#ifdef _FILE_SHELL_
    "file", SEGMENT_ID_FILE_SHELL, FileDev_Shell,"file manager device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_ALC5633_ALC5633DEVICE_C__
#ifdef _ALC5633_DEV_SHELL_
    "5633", NULL,  ALC5633Shell,"alc 5633 device cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_DMA_DMADEVICE_C__
#ifdef _DMA_SHELL_
    "dma", SEGMENT_ID_DMA_SHELL, DmaDev_Shell,"dma device cmd package","NULL",
#endif
#endif
#ifdef __GUI_LCDDRIVER_C__
#ifdef _LCD_SHELL_
    "lcd", SEGMENT_ID_LCD_SHELL, LcdShell,"lcd device cmd package","NULL",
#endif
#endif
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
#ifdef _FW_ANLYSIS_SHELL_
    "fw", SEGMENT_ID_FW_SHELL, FWShell,"firmeware dirver cmd package","NULL",
#endif
#endif
#ifdef __DRIVER_UART_UARTDEVICE_C__
#ifdef _AUDIO_SHELL_
    "uart", NULL, UartDev_Shell,"uart device cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HW_MP3_IMDCT_C__
#ifdef _IMDCT_SHELL_
    "imdct", NULL, hw_imdct_shell,"imdct for mp3 cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HW_MP3_SYN_C__
#ifdef _SYN_SHELL_
    "syn", NULL, hw_syn_shell,"hw synthe for mp3 cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HW_YUV2RGB_C__
#ifdef _YUV2RGB_SHELL_
    "yuv", NULL, hw_yuv2rgb_shell,"yuv2rgb for display cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_APE_C__
#ifdef _HIFI_SHELL_
    "ape", NULL,  hifi_ape_shell,"hifi ape hard decode cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_MAC_C__
#ifdef _HIFI_SHELL_
    "mac", NULL, hifi_mac_shell,"mac machine cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_C__
#ifdef _HIFI_SHELL_
    "hifi", NULL, HIFI_SHELL,"hifi test cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_FFT_C__
#ifdef _HIFI_SHELL_
    "fft", NULL,  hifi_fft_shell,"fft cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_ALAC_C__
#ifdef _HIFI_SHELL_
    "alac", NULL,  hifi_alac_shell,"hifi alac hard decode cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_HIFI_FLAC_C__
#ifdef _HIFI_SHELL_
    "flac", NULL,  hifi_flac_shell,"hifi flac hard decode cmd package","NULL",
#endif
#endif
#ifdef __CPU_NANOD_LIB_GPIO_C__
#ifdef _GPIO_SHELL_
    "gpio", SEGMENT_ID_GPIO_SHELL, GPIOShell,"gpio bsp driver cmd package","NULL",
#endif
#endif
#ifdef __APP_RECORD_RECORD_C__  //chad.ma add
    "record",SEGMENT_ID_RECORD_SHELL,record_shell,"record contoller cmd package","NULL",
#endif
#ifdef __APP_AUDIO_MUSIC_C__
    "music", SEGMENT_ID_MUSIC_SHELL, music_shell,"audio contoller cmd package","NULL",
#endif
#ifdef _DRIVER_WIFI__
    "wifi", SEGMENT_ID_AP6181_SHELL, wifi_shell,"wifi contoller cmd package","NULL",
#endif
#ifdef __WIFI_DLNA_C__
    "dlna", SEGMENT_ID_DLNA_SHELL, dlna_shell,"dlna player cmd package","NULL",
#endif
#ifdef __WIFI_XXX_C__
    "XXX", SEGMENT_ID_XXX_SHELL, XXX_shell,"XXX player cmd package","NULL",
#endif
#ifdef __SHELL_SWITCH_PLAYER_C__//JJJHHH 20161016
	"switch", SEGMENT_ID_SWITCHPLAYER_SHELL, SwitchPlayer_shell,"switch player cmd package","NULL",
#endif

    "\b", NULL, NULL, NULL
};

static SHELL_TASK_DATA_BLOCK*   gpstShellTaskDataBlock;
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
** Name: ShellHelpDesDisplay
** Input:HDC dev,  uint8 * CmdDes,  uint8 * pstr
** Return: void
** Owner:aaron.sun
** Date: 2016.4.14
** Time: 17:18:05
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API rk_err_t ShellHelpDesDisplay(HDC dev,  uint8 * CmdDes,  uint8 * pstr)
{
    uint8 CmdSampleDes[512];
    SEGMENT_INFO_T  Segment;

    pstr--;

    if(pstr[0] == '.')
    {
        pstr++;
        if(StrCmpA(pstr, "help", 4) != 0)
        {
            return RK_ERROR;
        }
    }
    else
    {
        return RK_ERROR;
    }

    if(CmdDes == NULL)
    {
        return RK_SUCCESS;
    }

    CmdSampleDes[511] = 0;

    FW_GetSegmentInfo(SEGMENT_ID_SHELL_CMD_NAME, &Segment);

    FW_ReadFirmwaveByByte(Segment.CodeLoadBase + (uint32)CmdDes - Segment.CodeImageBase , CmdSampleDes, 511);

    rk_printf_no_time("%s", CmdSampleDes);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: ShellHelpSampleDesDisplay
** Input:HDC dev,  SHELL_CMD * cmd,  uint8 * pstr
** Return: void
** Owner:aaron.sun
** Date: 2016.4.14
** Time: 17:03:20
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API rk_err_t ShellHelpSampleDesDisplay(HDC dev,  SHELL_CMD * cmd,  uint8 * pstr)
{
    uint32 i;
    uint8 CmdSampleDes[512];
    SEGMENT_INFO_T  Segment;

    pstr--;
    if(pstr[0] == '.')
    {
        pstr++;
        if(StrCmpA(pstr, "help", 4) != 0)
        {
            return RK_ERROR;
        }
    }
    else
    {
        return RK_ERROR;
    }
    if(cmd == NULL)
    {
        return RK_SUCCESS;
    }

    CmdSampleDes[511] = 0;

    FW_GetSegmentInfo(SEGMENT_ID_SHELL_CMD_NAME, &Segment);

    i = 0;
    while(cmd[i].CmdName[0] != '\b')
    {
        uint8 buf[12];
        memset(buf, 0x20, 12);
        buf[11] = 0;
        memcpy(buf, cmd[i].CmdName, strlen(cmd[i].CmdName));

        FW_ReadFirmwaveByByte(Segment.CodeLoadBase + (uint32)cmd[i].CmdSampleDes - Segment.CodeImageBase, CmdSampleDes, 511);

        rk_printf_no_time("%s\t\t%s",buf, CmdSampleDes);
        i++;
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: ShellItemExtract
** Input:uint8 * pstr, uint8 ** pItem, uint8 * Space
** Return: uint32
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:51:49
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API uint32 ShellItemExtract(uint8 * pstr, uint8 ** pItem, uint8 * Space)
{
    uint32 i = 0;
    uint16 ItemSize = 0;

    (*pItem) = (uint8 *)(pstr + i);

    while ((*(pstr + i) != ' ') && (*(pstr + i) != '.') && (*(pstr + i) != NULL))
    {
        ItemSize++;
        i++;
    }

    *Space = *(pstr + i);

    return ItemSize;

}
/*******************************************************************************
** Name: ShellCheckCmd
** Input:uint8 * CmdStr,  uint8 * Cmd,  uint32 len
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.14
** Time: 16:36:37
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API rk_err_t ShellCheckCmd(SHELL_CMD CmdStr[],  uint8 * Cmd,  uint32 len)
{

    int32 i = 0;
    while (1)   //compare the name
    {
        if (len != StrLenA(CmdStr[i].CmdName))
        {
            i++;
        }
        else
        {
            if (StrCmpA(CmdStr[i].CmdName, Cmd, len) != 0)
            {
                i++;
            }
            else
            {
                break;
            }
        }

        if(CmdStr[i].CmdName[0] == '\b')
            return RK_ERROR;

    }

    return i;

}
/*******************************************************************************
** Name: ShellTask
** Input:void * arg
** Return: void
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 16:43:00
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API void ShellTask(void * arg)
{
    uint32 i,j;
    uint8* pstr;
    HDC pstTimerDevHandler;
    uint32 CmdCnt;
    uint32 RecordCmd;
    HTC hSelf;
    hSelf = RKTaskGetRunHandle();

    pstr = gpstShellTaskDataBlock->ShellRxBuffer[gpstShellTaskDataBlock->Index];

    rk_print_string("\nWelcome to Use RKOS V1.1");
    rk_print_string("\nRKOS V1.1 - Copyright (C) Fuzhou Rockchips Electronics CO.,Ltd");
    rk_print_string("\n***************************************************************************");
    rk_print_string("\n *    If you do not how to use RKOS, please input help cmd.              *");
    rk_print_string("\n *    also send message to aaron.sun@rock-chips.com                      *");
    rk_print_string("\n *    Thank you for using RKOS, and thank you for your support!          *");
    rk_print_string("\n***************************************************************************");

    if (Grf_Get_Cpu_ID())
    {
        rk_print_string("\ncur system is BB system");
    }
    else
    {
        rk_print_string("\ncur system is AP system");
    }

    pstr[0] = 'r';
    pstr[1] = 'k';
    pstr[2] = 'o';
    pstr[3] = 's';
    pstr[4] = ':';
    pstr[5] = '/';
    pstr[6] = '/';
    CmdCnt = 0;
    RecordCmd = 1;

    gpstShellTaskDataBlock->ShellRxStart = 7;

    while (1)
    {
        rk_print_string("\r\n");
        rk_print_string(pstr);

        i = gpstShellTaskDataBlock->ShellRxStart;

        while (i <= SHELL_RX_BUF_MAX_SIZE)
        {

            UartDev_Read(gpstShellTaskDataBlock->hUart, pstr + i, 1, MAX_DELAY);

            if(ShellTaskCheckIdle(hSelf) != RK_SUCCESS)
            {
                continue;
            }

            if(pstr[i] == 0x09)
            {
                if(CmdCnt == 0)
                {
                    continue;
                }

                for(i = 0; i < 80; i++)
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,"\b",1,SYNC_MODE,NULL);
                }

                for(i = 0; i < 80; i++)
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart," ",1,SYNC_MODE,NULL);
                }

                for(i = 0; i < 80; i++)
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart,"\b",1,SYNC_MODE,NULL);
                }

                if(gpstShellTaskDataBlock->OldIndex == 0)
                {
                    if(CmdCnt < 10)
                    {
                        gpstShellTaskDataBlock->OldIndex = gpstShellTaskDataBlock->Index - 1;
                    }
                    else
                    {
                        gpstShellTaskDataBlock->OldIndex = 9;
                    }
                }
                else
                {
                    gpstShellTaskDataBlock->OldIndex--;
                }

                memcpy(pstr,gpstShellTaskDataBlock->ShellRxBuffer[gpstShellTaskDataBlock->OldIndex], SHELL_RX_BUF_MAX_SIZE);
                UartDev_Write(gpstShellTaskDataBlock->hUart,pstr,strlen(pstr),SYNC_MODE,NULL);
                i = strlen(pstr);
                RecordCmd = 0;
                continue;

            }
            else if (pstr[i] == 0x0d)
            {
                UartDev_Write(gpstShellTaskDataBlock->hUart,"\r\n",2,SYNC_MODE,NULL);
                gpstShellTaskDataBlock->OldIndex = gpstShellTaskDataBlock->Index;
                break;
            }
            else if ((pstr[i] < 32) && (pstr[i] != '\b'))
            {
                continue;
            }
            else if ((pstr[i] >= 127))
            {
                continue;
            }

            RecordCmd = 1;

            UartDev_Write(gpstShellTaskDataBlock->hUart,pstr+i,1,SYNC_MODE,NULL);

            if (pstr[i] == '\b')
            {
                if (i == gpstShellTaskDataBlock->ShellRxStart)
                {
                    if (gpstShellTaskDataBlock->ShellRxStart == 7)
                    {
                        UartDev_Write(gpstShellTaskDataBlock->hUart,"/",1,SYNC_MODE,NULL);
                    }
                    else
                    {
                        UartDev_Write(gpstShellTaskDataBlock->hUart,".",1,SYNC_MODE,NULL);
                    }
                }
                else
                {
                    UartDev_Write(gpstShellTaskDataBlock->hUart," \b", 2, SYNC_MODE, NULL);
                    i--;
                }
            }
            else
            {
                i++;
            }
        }

        if (pstr[i] == 0x0d)
        {
            if (i == gpstShellTaskDataBlock->ShellRxStart)
            {
                //RKwrite(gpstShellTaskDataBlock->hUart,0,"rkos://",7,SYNC_MODE,NULL);
                goto NextCmd;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == 0x0d))
            {
                pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
            }
            else if ((pstr[gpstShellTaskDataBlock->ShellRxStart] == 'c')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 1] == 'd')
                     && (pstr[gpstShellTaskDataBlock->ShellRxStart + 2] == ' '))
            {
                if (pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == 0x0d)
                {
                    pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
                }
                else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                         && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == '.')
                         &&(pstr[gpstShellTaskDataBlock->ShellRxStart + 5] == 0x0d))
                {
                    if (gpstShellTaskDataBlock->ShellRxStart > 8)
                    {
                        for (j = gpstShellTaskDataBlock->ShellRxStart - 2; j >= 7; j--)
                        {
                            if (pstr[j] == '.')
                            {
                                break;
                            }
                        }
                        gpstShellTaskDataBlock->ShellRxStart = j + 1;
                        CmdCnt = 0;
                    }
                    pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
                }
                else if ((pstr[gpstShellTaskDataBlock->ShellRxStart + 3] == '.')
                         && (pstr[gpstShellTaskDataBlock->ShellRxStart + 4] == 0x0d))
                {
                    pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
                }
                else if (memcmp(pstr + gpstShellTaskDataBlock->ShellRxStart + 3, "rkos://", 7) == 0)
                {
                    pstr[i] = 0;
                    for (j = 0; j < strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3); j++)
                    {
                        pstr[j] = pstr[gpstShellTaskDataBlock->ShellRxStart + 3 + j];
                    }

                    if (j > 7)
                    {
                        pstr[j++] = '.';
                        gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                    }
                    else
                    {
                        gpstShellTaskDataBlock->ShellRxStart = strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3);
                    }

                    CmdCnt = 0;

                    pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
                }
                else
                {
                    pstr[i] = 0;
                    for (j = gpstShellTaskDataBlock->ShellRxStart; j <(gpstShellTaskDataBlock->ShellRxStart + strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3)); j++)
                    {
                        pstr[j] = pstr[3 + j];
                    }
                    pstr[j++] = '.';

                    gpstShellTaskDataBlock->ShellRxStart += strlen(pstr + gpstShellTaskDataBlock->ShellRxStart + 3) + 1;
                    CmdCnt = 0;
                    pstr[gpstShellTaskDataBlock->ShellRxStart] = 0;
                }
            }
            else
            {
                pstr[i] = 0;
                if(ShellRootParsing(gpstShellTaskDataBlock->hUart, pstr + 7) != RK_SUCCESS)
                    goto NextCmd;
            }
        }

        if(RecordCmd == 1)
        {
            gpstShellTaskDataBlock->Index++;
            gpstShellTaskDataBlock->Index = gpstShellTaskDataBlock->Index % 10;
            gpstShellTaskDataBlock->OldIndex = gpstShellTaskDataBlock->Index;
            memcpy(gpstShellTaskDataBlock->ShellRxBuffer[gpstShellTaskDataBlock->Index],pstr, gpstShellTaskDataBlock->ShellRxStart);
            pstr = gpstShellTaskDataBlock->ShellRxBuffer[gpstShellTaskDataBlock->Index];
            CmdCnt++;
        }

NextCmd:
        memset(pstr + gpstShellTaskDataBlock->ShellRxStart,0,(SHELL_RX_BUF_MAX_SIZE - gpstShellTaskDataBlock->ShellRxStart));


    }
}

/*******************************************************************************
** Name: ShellRootParsing
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 16:33:41
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON API rk_err_t ShellRootParsing(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 temp;
    SHELL_CMD_ITEM * pTempCmdItem = NULL;

    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);

    if (StrCnt == 0)
    {
        rk_print_string("error cmd\r\n");
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellRootName, pItem, StrCnt);

    if (ret < 0)
    {
        pTempCmdItem = gpstShellTaskDataBlock->CmdHeadItem;

        temp = pstr[StrCnt];
        pstr[StrCnt] = 0;

        for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
        {
            if (pTempCmdItem->useflag == 1)
            {
                if (StrCmpA(pTempCmdItem->ShellCmdName, pstr, 0) == 0)
                {
                    if (pTempCmdItem->ShellCmdParaseFun != NULL)
                    {
                        pItem += StrCnt;
                        pItem++;

                        pstr[StrCnt] = temp;
                        ShellHelpDesDisplay(dev, pTempCmdItem->CmdDes, pItem);

                        if(pTempCmdItem->ShellCmdParaseFun != NULL)
                            ret = pTempCmdItem->ShellCmdParaseFun(dev,pItem);

                        if (ret == RK_ERROR)
                        {
                            rk_print_string("error cmd\r\n");
                        }

                        return ret;
                    }
                }
            }
            pTempCmdItem++;
        }

        pstr[StrCnt] = temp;

        rk_print_string("error cmd\r\n");
        return RK_ERROR;

    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellRootName[i].CmdDes, pItem);

    if(ShellRootName[i].ShellCmdParaseFun != NULL)
        ret = ShellRootName[i].ShellCmdParaseFun(dev, pItem);

    if (ret == RK_ERROR)
    {
        rk_print_string("error cmd\r\n");
    }

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
** Name: ShellTaskCheckIdle
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 19:36:56
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellTaskCheckIdle(HTC hTask)
{
    RK_TASK_CLASS*   pTask = (RK_TASK_CLASS*)hTask;

    if(pTask->State != TASK_STATE_WORKING)
    {
        if(ShellTaskResume(hTask) != RK_SUCCESS)
        {
            return RK_ERROR;
        }
    }

    pTask->IdleTick = 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellTaskResume
** Input:HTC hTask
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 17:38:46
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellTaskResume(HTC hTask)
{
   RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;
   gpstShellTaskDataBlock->hUart = RKDev_Open(DEV_CLASS_UART, DEBUG_UART_PORT, NOT_CARE);
   pShellTask->State = TASK_STATE_WORKING;
   return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellTaskSuspend
** Input:HTC hTask, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.4
** Time: 17:36:03
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellTaskSuspend(HTC hTask, uint32 Level)
{
    RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)hTask;

    if(Level == TASK_STATE_IDLE1)
    {
        pShellTask->State = TASK_STATE_IDLE1;
    }
    else if(Level == TASK_STATE_IDLE2)
    {
        pShellTask->State = TASK_STATE_IDLE2;
    }
    RKDev_Close(gpstShellTaskDataBlock->hUart);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellCmdGetCnt
** Input:SHELL_CMD * cmd
** Return: uint32
** Owner:aaron.sun
** Date: 2016.4.14
** Time: 18:06:00
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN uint32 ShellCmdGetCnt(SHELL_CMD * cmd)
{
    uint32 i;

    i = 0;

    while(cmd[i].CmdName[0] != '\b')
        i++;

    return i;
}

/*******************************************************************************
** Name: ShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.14
** Time: 16:41:41
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellPcb(HDC dev, uint8 * pstr)
{
    uint32 i;
    if(gpstShellTaskDataBlock == NULL)
    {
        rk_printf("shell task not exist");
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstShellTaskDataBlock");

    for(i = 0; i < SHELL_CMD_MAX_ITEM; i++)
    {
        rk_printf_no_time("    .CmdHeadItem[%d]", i);
        rk_printf_no_time("        .useflag = %d",gpstShellTaskDataBlock->CmdHeadItem[i].useflag);
        rk_printf_no_time("        .CmdDes = %d",gpstShellTaskDataBlock->CmdHeadItem[i].CmdDes);
        rk_printf_no_time("        .SegmentID = %d",gpstShellTaskDataBlock->CmdHeadItem[i].SegmentID);
        rk_printf_no_time("        .ShellCmdName = %08x",gpstShellTaskDataBlock->CmdHeadItem[i].ShellCmdName);
        rk_printf_no_time("        .ShellCmdParaseFun = %08x",gpstShellTaskDataBlock->CmdHeadItem[i].ShellCmdParaseFun);
    }

    return RK_SUCCESS;

}


/*******************************************************************************
** Name: ShellHelp
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 16:31:47
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellHelp(HDC dev, uint8 * pstr)
{
    uint32 i;

    SHELL_CMD_INFO  ShellCmdName[sizeof(ShellRegisterName1) / sizeof(SHELL_CMD_INFO)];
    uint8  CmdSampleDes[512];
    SEGMENT_INFO_T  Segment;
    uint32 DesStart, DesSize;

    pstr--;

    if (StrLenA(pstr) != 0)
        return RK_ERROR;

    rk_print_string("\nrkos shell support mulitple level cmd£¬between two level cmd use '.', for example usbotg.mc");
    rk_print_string("\n max level cmd as follow:");

    FW_GetSegmentInfo(SEGMENT_ID_SHELL_CMD_NAME, &Segment);

    FW_ReadFirmwaveByByte(Segment.CodeLoadBase, (uint8 *)ShellCmdName, sizeof(ShellRegisterName1));

    CmdSampleDes[511] = 0;

    i = 0;

    while(ShellRootName[i].CmdName[0] != '\b')
    {
        uint8 buf[12];
        memset(buf, 0x20, 12);
        buf[11] = 0;
        memcpy(buf, ShellRootName[i].CmdName, strlen(ShellRootName[i].CmdName));

        FW_ReadFirmwaveByByte(Segment.CodeLoadBase + (uint32)ShellRootName[i].CmdSampleDes - Segment.CodeImageBase, CmdSampleDes, 511);
        rk_printf_no_time("%s\t\t%s",buf, CmdSampleDes);
        i++;
    }


    i = 0;
    while(ShellCmdName[i].ShellCmdName[0] != '\b')
    {
        uint8 buf[12];
        memset(buf, 0x20, 12);
        buf[11] = 0;
        memcpy(buf, ShellCmdName[i].ShellCmdName, strlen(ShellCmdName[i].ShellCmdName));

        FW_ReadFirmwaveByByte(Segment.CodeLoadBase + (uint32)ShellCmdName[i].CmdSampleDes - Segment.CodeImageBase, CmdSampleDes, 511);
        rk_printf_no_time("%s\t\t%s",buf, CmdSampleDes);
        i++;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellCmdReister
** Input:HDC dev, uint8 * pItemName
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 14:27:57
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellCmdRegister(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;
    SEGMENT_INFO_T  Segment;
    SHELL_CMD_INFO  ShellRegisterName[sizeof(ShellRegisterName1) / sizeof(SHELL_CMD_INFO)];

    pItemName--;

    if(*pItemName != ' ')
    {
        return RK_ERROR;
    }
    pItemName++;

    uint32 i = 0, j = 0;

    pCmdItem = gpstShellTaskDataBlock->CmdHeadItem;

    for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
    {
        if (pCmdItem->useflag == 1)
        {
            if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
            {
                // already register
                rk_print_string("\r\ndevice already connected");
                return RK_SUCCESS;
            }
        }
        pCmdItem++;
    }

    FW_GetSegmentInfo(SEGMENT_ID_SHELL_CMD_NAME, &Segment);

    FW_ReadFirmwaveByByte(Segment.CodeLoadBase, (uint8*)ShellRegisterName, sizeof(ShellRegisterName1));

    while (StrCmpA((uint8*)ShellRegisterName[i].ShellCmdName,pItemName, 0) != 0)
    {
        i++;
        if (*((uint8*)ShellRegisterName[i].ShellCmdName) == '\b')
        {
            rk_print_string("\r\ndevice not exist");
            return RK_SUCCESS;
        }
    }

    pCmdItem = gpstShellTaskDataBlock->CmdHeadItem;
    for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
    {
        if (pCmdItem->useflag == 0)
        {
            if (ShellRegisterName[i].SegmentID != 0)
            {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                if (FW_LoadSegment(ShellRegisterName[i].SegmentID, SEGMENT_OVERLAY_CODE) != RK_SUCCESS)
                {
                    rk_print_string("\r\ndevice connect err");
                    return RK_SUCCESS;
                }
#endif
            }
            pCmdItem->ShellCmdParaseFun  = ShellRegisterName[i].ShellCmdParaseFun;
            memcpy(pCmdItem->ShellCmdName, ShellRegisterName[i].ShellCmdName, 12);
            pCmdItem->useflag = 1;
            pCmdItem->CmdDes = ShellRegisterName[i].CmdDes;
            pCmdItem->SegmentID = ShellRegisterName[i].SegmentID;

            rk_print_string("\r\ndevice connect success");

            return RK_SUCCESS;
        }
        pCmdItem++;
    }

    rk_print_string("\r\ncmd full");

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellCmdRemove
** Input:HDC dev, uint8 * pItemName
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 14:27:18
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_COMMON_
COMMON FUN rk_err_t ShellCmdRemove(HDC dev, uint8 * pItemName)
{
    SHELL_CMD_ITEM * pCmdItem;

    uint32 i = 0, j = 0;

    pCmdItem = gpstShellTaskDataBlock->CmdHeadItem;
    for (j = 0; j < SHELL_CMD_MAX_ITEM; j++)
    {
        if (pCmdItem->useflag == 1)
        {
            if (StrCmpA((uint8*)pCmdItem->ShellCmdName, pItemName, 0) == 0)
            {
                // already register
                pCmdItem->useflag = 0;
                rk_print_string("device remove success");

                if (pCmdItem->SegmentID)
                {
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
                    FW_RemoveSegment(pCmdItem->SegmentID);
#endif
                }

                return RK_SUCCESS;
            }
        }
        pCmdItem++;
    }

    rk_print_string("not find cmd");

    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: ShellTaskDeInit
** Input:void *pvParameters
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 16:41:06
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_INIT_
INIT API rk_err_t ShellTaskDeInit(void *pvParameters)
{
    RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)pvParameters;
    RKDev_Close(gpstShellTaskDataBlock->hUart);
    rkos_memory_free(gpstShellTaskDataBlock);
    gpstShellTaskDataBlock = NULL;
    rk_printf("shell task deinit ok");
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: ShellTaskInit
** Input:void *pvParameters, void * arg
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.20
** Time: 16:35:33
*******************************************************************************/
_SYSTEM_SHELL_SHELLTASK_INIT_
INIT API rk_err_t ShellTaskInit(void *pvParameters, void * arg)
{
    RK_TASK_CLASS*   pShellTask = (RK_TASK_CLASS*)pvParameters;
    SHELL_TASK_DATA_BLOCK*  pShellTaskDataBlock;
    SHELL_CMD_ITEM * pShellCmdItem;

    uint32 i;

    if (pShellTask == NULL)
        return RK_PARA_ERR;

    pShellTask->Idle1EventTime = 10 * PM_TIME;
    pShellTask->TaskSuspendFun = ShellTaskSuspend;
    pShellTask->TaskResumeFun = ShellTaskResume;

    pShellTaskDataBlock = rkos_memory_malloc(sizeof(SHELL_TASK_DATA_BLOCK));
    memset(pShellTaskDataBlock,NULL,sizeof(SHELL_TASK_DATA_BLOCK));

    pShellCmdItem = (void *)pShellTaskDataBlock->CmdHeadItem;
    for (i = 0; i < SHELL_CMD_MAX_ITEM; i++)
    {
        pShellCmdItem->useflag = 0;
        pShellCmdItem++;
    }

    pShellTaskDataBlock->hUart = RKDev_Open(DEV_CLASS_UART, DEBUG_UART_PORT, NOT_CARE);
    if (pShellTaskDataBlock->hUart ==  (HDC)RK_ERROR)
        goto exit;

    gpstShellTaskDataBlock = pShellTaskDataBlock;

    return RK_SUCCESS;

exit:

    return RK_ERROR;
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



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#endif
