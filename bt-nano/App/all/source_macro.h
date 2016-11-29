/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Bsp\EVK_V1.0\source_macro.h
* Owner: Aaron
* Date: 2014.2.17
* Time: 14:09:36
* Desc: this file define some macro using control source code whether be builied
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron     2014.2.17     14:09:36   1.0
********************************************************************************************
*/

#ifndef __BSP_EVK_V1_0_SOURCE_MACRO_H__
#define __BSP_EVK_V1_0_SOURCE_MACRO_H__

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

//#define __WEB_CHANNELS_RK_FOCHANNEL_C__
#ifdef __WEB_CHANNELS_RK_FOCHANNEL_C__
#define __RK_BROAD_TASK_C__
#define __RK_TCPSTREAM_TASK_C__
#define __RK_TCPCONTROL_TASK_C__
#define __RK_TCPCHANNEL_TASK_C__
#define __RK_TCPHEART_TASK_C__
#endif

#ifdef _BROWSER_
#ifdef _USE_GUI_
#define __APP_BROWSER_BROWSERUITASK_C__
#endif
#endif

#ifdef _SYSSET_
#ifdef _USE_GUI_
#define __APP_SYSTEMSET_SYSTEMSETTASK_C__
#define __APP_SYSTEMSET_SYSTEMSETWIFISELECTTASK_C__
#endif
#endif

#ifdef _MEDIA_MODULE_
#define __APP_MEDIA_UPDATE_C__
#define __APP_MEDIA_LIBRARY_C__
#define __APP_MEDIA_BROWSER_C__
#endif

#define __APP_MAIN_TASK_MAIN_TASK_C__

#ifdef USE_LWIP
#define __WICE_HTTP_C__
//#define __WICE_HTTP_SERVER__
#define __WIFI_DLNA_C__
#define __WIFI_XXX_C__
#define __WIFI_AIRPLAY_C__
//#define __SHELL_SWITCH_PLAYER_C__
//jjjhhh
//#define __SSL_MbedTLS__

#ifdef __WIFI_DLNA_C__
#define __DLNA_PLAYER_C__
#define __WIFI_UPNP_C__
#define __DLNA_RKOS_TASK__
#endif
#endif

#ifdef _WICE_
#define _DRIVER_WIFI__
#ifdef _DRIVER_WIFI__
#define __WICE_WIFITHREAD_C__
#define __WICE_WIFI_TRANS_C__
#define __WICE_TCPIP_C__
#define __WICE_NETWORKING_C__
#define __WICE_HARD_TO_WOKR_C__
#endif
#endif

#ifdef _REALTEK_
#define CONFIG_WLAN 1
#define __WICE_TCPIP_C__
#endif

//#define __DRIVER_MAILBOX_MAILBOXDEVICE_C__
//#define __DRIVER_RK61X_RK618DEVICE_C__
#define __DRIVER_SD_SDDEVICE_C__
#define __DRIVER_PWM_PWMDEVICE_C__

#ifdef _USB_
#define __SYSTEM_USBSERVER_USBSERVICE_C__
#define __DRIVER_USB_USBOTGDEV_C__
#define __DRIVER_USBMSC_USBMSCDEVICE_C__
#endif

#define ___COMMON_SYSTEM_SYSSERVICE_SYSRESUME_C__
#define __DRIVER_SDIO_SDIODEVICE_C__
//#define __DRIVER_SPINOR_SPINORDEVICE_C__
#define __DRIVER_SPI_SPIDEVICE_C__
#define __DRIVER_FIFO_FIFODEVICE_C__

#ifdef _BLUETOOTH_
#define __APP_MEDIA_MEDIATASK_C__
#endif

//#define __APP_FILESTREAM_FILESTREAMTASK_C__
//#define __SYSTEM_EXAMPLE_MODULEFORMAT_C__
//#define __APP_EXAMPLE_TASKFORMAT_C__
//#define __DRIVER_EXAMPLE_DEVICEFORMAT_C__
#define __DRIVER_UART_UARTDEVICE_C__

#define __DRIVER_SDMMC_SDMMCDEVICE_C__
#define __DRIVER_EMMC_EMMCDEVICE_C__
#define __DRIVER_KEY_KEYDEVICE_C__
#define __DRIVER_LUN_LUNDEVICE_C__

#ifdef _FS_
#define __DRIVER_VOLUME_FATDEVICE_C__
#define __DRIVER_PARTION_PARTIONDEVICE_C__
#define __DRIVER_FILE_FILEDEVICE_C__
#define __DRIVER_FILE_DIRDEVICE_C__
#endif

#define __DRIVER_I2C_I2CDEVICE_C__
#define __DRIVER_I2S_I2SDEVICE_C__
#define __DRIVER_ROCKCODEC_ROCKCODECDEVICE_C__
#define __OS_PLUGIN_DEVICEPLUGIN_C__
//#define __DRIVER_ALC5633_ALC5633DEVICE_C__
#define __DRIVER_DMA_DMADEVICE_C__
#define __DRIVER_AUDIO_AUDIODEVICE_C__
#define __DRIVER_AD_ADCDEVICE_C__
#define __DRIVER_TIMER_TIMERDEVICE_C__
//#define __DRIVER_WM8987_WM8987DEVICE_C__
#define __DRIVER_WATCHDOG_WATCHDOGDEVICE_C__
#define __DRIVER_SPIFLASH_SPIFLASHDEV_C__
#define __DRIVER_BCORE_BCOREDEVICE_C__


#define _USE_CODEPAGE_CP874_
#define _USE_CODEPAGE_CP932_
#define _USE_CODEPAGE_CP936_
#define _USE_CODEPAGE_CP949_
#define _USE_CODEPAGE_CP950_
#define _USE_CODEPAGE_CP1250_
#define _USE_CODEPAGE_CP1251_
#define _USE_CODEPAGE_CP1252_
#define _USE_CODEPAGE_CP1253_
#define _USE_CODEPAGE_CP1254_
#define _USE_CODEPAGE_CP1255_
#define _USE_CODEPAGE_CP1256_
#define _USE_CODEPAGE_CP1257_
#define _USE_CODEPAGE_CP1258_

#ifdef _USE_GUI_
#define __APP_USB_CHARGETASK_C__
#define __GUI_LCDDRIVER_C__
#define __DRIVER_LCD_LCDDEVICE_C__
#define __DRIVER_DISPLAY_DISPLAYDEVICE_C__
#define __GUI_GUITASK_C__
#define __GUI_GUIMANAGER_C__
#define __GUI_GUIICON_C__
#define __GUI_GUIIMAGE_C__
#define __GUI_GUITEXT_C__
#define __GUI_GUISELECT_C__
#define __GUI_GUIMSGBOX_C__
#define __GUI_GUICHAIN_C__
#define __GUI_GUIPROGRESSBAR_C__
#define __GUI_GUISPECTRUM_C__
#define __GUI_ST7735SDRIVER_C__
#define __CPU_NANOD_LIB_VOP_C__
#define __DRIVER_VOP_VOPDEVICE_C__
//#define __CPU_NANOD_LIB_HW_YUV2RGB_C__
#endif

#define __OS_FWANALYSIS_FWANALYSIS_C__
#define __OS_FWANALYSIS_FWUPDATE_C__
#define __SYSTEM_MODULEOVERLAY_SYSINFOSAVE_C__
#define __CPU_NANOD_LIB_MAILBOX_C__
#define __CPU_NANOD_LIB_GPIO_C__
#define __DRIVER_MSG_MSGDEVICE_C__
#define __SYSTEM_SYSSEVER_POWERMANAGER_C__


#ifdef B_CORE_DECODE
#define __CPU_NANOD_LIB_HIFI_C__
#define __CPU_NANOD_LIB_HIFI_APE_C__
#define __CPU_NANOD_LIB_HIFI_ALAC_C__
#define __CPU_NANOD_LIB_HIFI_FLAC_C__
//#define __CPU_NANOD_LIB_HIFI_MAC_C__
//#define __CPU_NANOD_LIB_HIFI_FFT_C__
//#define __CPU_NANOD_LIB_HW_MP3_IMDCT_C__
//#define __CPU_NANOD_LIB_HW_MP3_SYN_C__
#endif


#ifdef _MUSIC_
#define __APP_AUDIO_AUDIOCONTROLTASK_C__
#define __APP_FILESTREAM_STREAMCONTROLTASK_C__
#define __APP_MUSIC_PLAY_MENU_TASK_C__
#ifdef _FS_
#define __APP_AUDIO_MUSIC_C__
#endif
#endif

#ifdef _RECORD_
#define __APP_RECORD_RECORDCONTROL_C__
#define __APP_RECORD_RECORD_C__
#ifdef _USE_GUI_
#define __APP_RECORD_RECOD_TASK_C__
#endif
#endif

#ifdef _RADIO_
#define __DRIVER_FM_FMDEVICE_C__
#ifdef _USE_GUI_
#define __APP_FM_FMUITASK_C__
#endif
#define __APP_FM_FMCONTROLTASK_C__
#define __DRIVER_FM_HW_FM5807_C__
#endif

#ifdef _LINEIN_
#define __APP_LINE_IN_LINEINCONTROLTASK_C__
#ifdef _USE_GUI_
#define __APP_LINE_IN_LINEINTASK_C__
#endif
#endif

#ifdef _USE_SHELL_
#define _FM_DEV_SHELL_
#define _I6000_WIFI_SHELL_
#define _IP_SHELL_
#define _BLUETOOTH_SHELL_
#define _VOP_DEV_SHELL_
#define _LCD_SHELL_
#define _DISPLAY_DEV_SHELL_
#define _GUI_SHELL_
#define _ADC_DEV_SHELL_
#define _KEY_DEV_SHELL_
#define _TIMER_DEV_SHELL_
#define _BCORE_DEV_SHELL_
#define _MSG_DEV_SHELL_
#define _DIR_DEV_SHELL_

#define _USBMSC_DEV_SHELL_
#define _MAILBOX_DEV_SHELL_
#define _WDT_DEV_SHELL_
#define _WM8987_DEV_SHELL_
#define _ALC5633_DEV_SHELL_
//    #define _RK618_DEV_SHELL_
#define _SD_DEV_SHELL_
#define _I2C_DEV_SHELL_
#define _I2S_DEV_SHELL_
#define _PWM_DEV_SHELL_
#define _RK_ACODE_SHELL_
#define _USBOTG_DEV_SHELL_
#define _SDIO_DEV_SHELL_
#define _SPINOR_DEV_SHELL_
#define _SPI_DEV_SHELL_
#define _FIFO_DEV_SHELL_
#define _MEDIA_TASK_SHELL_
#define _FILESTREAM_TASK_SHELL_
#define _TEST_SHELL_
#define _TEST_TASK_SHELL_
#define _FAT_SHELL_
#define _FILE_SHELL_
#define _SDEMMC_SHELL_
#define _EMMC_DEV_SHELL_
#define _LUN_DEV_SHELL_
#define _PAR_DEV_SHELL_
#define _DMA_SHELL_
#define _AUDIO_SHELL_
#define _UART_DEV_SHELL_
#define _FW_ANLYSIS_SHELL_
#define _SYS_BOOT_SHELL_
#define _SPIFLASH_DEV_SHELL_
#define _IMDCT_SHELL_
#define _SYN_SHELL_
#define _YUV2RGB_SHELL_
#define _GPIO_SHELL_
//#define SHELL_BSP
#define __SYSTEM_SHELL_SHELLSYSCMD_C__
#define __SYSTEM_SHELL_SHELLDEVCMD_C__
#define __SYSTEM_SHELL_SHELLTASKCMD_C__
#define __SYSTEM_SHELL_SHELLCUSTOMCMD_C__
#define __SYSTEM_SHELL_SHELLTASK_C__
//#define _HIFI_SHELL_
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

