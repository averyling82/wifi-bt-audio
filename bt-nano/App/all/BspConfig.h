/*
**************************************************************************************
*
*     Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* File Name  BspConfig.h
*
* Description:
*
* History:      <author>          <time>        <version>
*    desc:    ORG.
**************************************************************************************
*/

#ifndef __BSP_CONFIG_H__
#define __BSP_CONFIG_H__

/*************************************************************************************
 *                                   Debug config
 *************************************************************************************/
//#define _NOT_USE_SEGMENT_OVERLAY_              /* Enalbe this macor will not use overlay,
                                                 /*if code size match hard memory,can use jtag*/
#define _LOG_DEBUG_                              /* Enable debug log output          */
#ifdef _LOG_DEBUG_                               /*-------Log Debug config-----------*/
    //#define USB_SERIAL_DEBUG                   /* Reserved                         */
    //#define _FILE_DEBUG_                       /* Reserved                         */
    #define _UART_DEBUG_                         /* Enable UART Debug                */
    #ifdef _UART_DEBUG_                          /*-----UART Debug config------------*/
        #define _USE_SHELL_                      /* Enable shell command             */
        //#define _RKOS_DEBUG_                     /* Use COM1 Debug RKOS              */
        //#define _IDLE_DEBUG_                     /* Not Shut off Uart                */

        #ifdef _RKOS_DEBUG_
        #define DEBUG_UART_PORT  UART_DEV1       /* Enable Uart0 Debug               */
        #define DEBUG_UART_ADDR  0x400b0000      /* Set UART0 address                */
        #else
        //#define _BROAD_LINE_OUT_               /* Enable LINE OUT for audio        */
        #ifndef _BROAD_LINE_OUT_                 /*-LINE OUT & UART Debug Mixconfig--*/
        #define DEBUG_UART_PORT  UART_DEV2       /* Enable Uart0 Debug          UART_DEV0     */
        #define DEBUG_UART_ADDR  0x400c0000      /* Set UART0 address           0x400a0000     */
        #else
        #define DEBUG_UART_PORT  UART_DEV2       /* Enable UART2 Debug               */
        #define DEBUG_UART_ADDR  0x400c0000      /* Set UART2 address                */
        #endif
        #endif

    #endif

    #define rk_print_stringA    rk_print_string  /* A core debug, only print string  */
    #define rk_print_stringB                     /* Reserved                         */
    #define rk_print_stringC                     /* Reserved                         */
    #define rk_print_stringD                     /* Reserved                         */

    #define rk_printfA          rk_printf        /* A core debug                     */
    #define rk_printfB                           /* Reserved                         */
    #define rk_printfC                           /* Reserved                         */
    #define rk_printfD                           /* Reserved                         */

    #define rk_printf_no_timeA  rk_printf_no_time/* A core debug,without current time*/
    #define rk_printf_no_timeB                   /* Reserved                         */
    #define rk_printf_no_timeC                   /* Reserved                         */
    #define rk_printf_no_timeD                   /* Reserved                         */

    #define printfA             printf           /* A core debug                     */
    #define printfB                              /* Reserved                         */
    #define printfC                              /* Reserved                         */
    #define printfD                              /* Reserved                         */

    #define DEBUGA(format,...)  rk_printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define DEBUGB(format,...)
    #define DEBUGC(format,...)
    #define DEBUGD(format,...)

    #define DEBUG(format,...)    rk_printf("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define DEBUG2(format,...)   rk_printf2("FILE: %s, LINE: %d: "format, __FILE__, __LINE__, ##__VA_ARGS__)
    #define USBDEBUG(format,...) printf("\nUSB:"format, ##__VA_ARGS__)
    #define bb_printf1           rk_printf2      /* B core debug                     */

#else //_LOG_DEBUG_

    #define rk_print_stringA(format,...)        /* Disable debug                     */
    #define rk_print_stringB(format,...)        /* Disable debug                     */
    #define rk_print_stringC(format,...)        /* Disable debug                     */
    #define rk_print_stringD(format,...)        /* Disable debug                     */

    #define rk_printfA(format,...)              /* Disable debug                     */
    #define rk_printfB(format,...)              /* Disable debug                     */
    #define rk_printfC(format,...)              /* Disable debug                     */
    #define rk_printfD(format,...)              /* Disable debug                     */

    #define rk_printf_no_timeA(format,...)      /* Disable debug                     */
    #define rk_printf_no_timeB(format,...)      /* Disable debug                     */
    #define rk_printf_no_timeC(format,...)      /* Disable debug                     */
    #define rk_printf_no_timeD(format,...)      /* Disable debug                     */

    #define printfA(format,...)                 /* Disable debug                     */
    #define printfB(format,...)                 /* Disable debug                     */
    #define printfC(format,...)                 /* Disable debug                     */
    #define printfD(format,...)                 /* Disable debug                     */

    #define DEBUGA(format,...)                  /* Disable debug                     */
    #define DEBUGB(format,...)                  /* Disable debug                     */
    #define DEBUGC(format,...)                  /* Disable debug                     */
    #define DEBUGD(format,...)                  /* Disable debug                     */

    #define DEBUG(format,...)                   /* Disable debug                     */
    #define DEBUG2(format,...)                  /* Disable debug                     */
    #define USBDEBUG(format,...)                /* Disable debug                     */
    #define printf(format,...)                  /* Disable debug                     */
    #define bb_printf1(format,...)              /* Disable debug                     */
    #define rk_print_string(format,...)         /* Disable debug                     */
    #define rk_printf(format,...)               /* Disable debug                     */
    #define rk_printf_no_time(format,...)       /* Disable debug                     */
    #define rk_printf2(format,...)              /* Disable debug                     */
    #define rk_printf_no_time2(format,...)      /* Disable debug                     */
    #define rk_print_string2(format,...)        /* Disable debug                     */

#endif

/*************************************************************************************
 *                        External memory and boot config
 *************************************************************************************/
//#define DISK_VOLUME                           /* Enable disk volume                */
#define _HIDDEN_DISK1_                          /* Enable hidden disk(A:)            */
#define _HIDDEN_DISK2_                          /* Enable hidden disk(B:)            */
//#define ENABLE_MBR                            /* Enable MBR                        */

//#define _EMMC_BOOT_                             /* Enable eMMC Flash                 */
#ifdef _EMMC_BOOT_                              /* -----eMMC Flash config------------*/
#define EMMC_SDC_DEV_ID     0                   /* SDC controller: 0 eMMC, 1 SDIO    */
#define EMMC_SDC_CHANNEL    1                   /* SDC iomux: 0 SD card, 1 Emmc      */
#define EMMC_LUN0_SIZE      40                  /* EMMC Firmware area: 40MB          */
#define EMMC_LUN1_SIZE      160                 /* EMMC Database area: 160MB         */
#define EMMC_PAGE_SIZE      8                   /* Page size: 8 sector               */
#define EMMC_BLOCK_SIZE     2                   /* Block size: 2 MB                  */

#ifdef _HIDDEN_DISK1_
#define EMMC_LUN3_SIZE      200                 /* Hidden file system area: 200MB    */
#else
#define EMMC_LUN3_SIZE      0                   /* Hidden file system area: 200MB    */
#endif
#ifdef _HIDDEN_DISK2_
#define EMMC_LUN4_SIZE      200                 /* Hidden file system area: 200MB    */
#else
#define EMMC_LUN4_SIZE      0                   /* Hidden file system area: 200MB    */
#endif

#endif

#define _SPI_BOOT_                            /* Enable SPI Flash                  */
#ifdef _SPI_BOOT_                               /* -----SPI Flash config-------------*/
#define _OTA_UPDATEFW_SPI
#define SPI_LUN0_SIZE       6                  /* SPI Firmware area: 6MB=x*512 */
#define SPI_LUN1_SIZE       2                   /* SPI Database area: 2MB            */
#define SPI_PAGE_SIZE       1                   /* Page size: 1 sector               */
#define SPI_BLOCK_SIZE      64                  /* Block size: 64 KB                 */
//#define USE_SPI_STORAGE                       /* LUN2 Use SpiFlash                 */
#ifdef _HIDDEN_DISK1_
#define SPI_LUN3_SIZE      200                 /* Hidden file system area: 200MB    */
#else
#define SPI_LUN3_SIZE      0                   /* Hidden file system area: 200MB    */
#endif
#ifdef _HIDDEN_DISK1_
#define SPI_LUN4_SIZE      200                 /* Hidden file system area: 200MB    */
#else
#define SPI_LUN4_SIZE      0                   /* Hidden file system area: 200MB    */
#endif
#endif


/*************************************************************************************
 *                               Function config
 *************************************************************************************/
//#define _USE_GUI_                               /* Enable GUI                        */
#ifndef _USE_GUI_
    #define NOSCREEN_USE_LED                    /* Enable LED in no screen mode      */
	#define NOSCREEN_OPEN                       /* open noscreen operation           */
    #define __ENABLE_POWERMANAGER         /*Enable PowerManager*/
#endif


#define _ENABLE_WIFI_BLUETOOTH
#define _DISABLE_ENTER_IDLEMODE

#define _WIFI_                                  /* Enable WIFI                       */
#if defined(_WIFI_) || defined(_ENABLE_WIFI_BLUETOOTH)                                   /* -----WIFI config------------------*/

#define WLAN_MODE_AP 1
#define WLAN_MODE_STA 2

#define _QPLAY_ENABLE

#define _WICE_
#ifdef _WICE_
#define WIFI_AP_COUNT 3   //(545)
#define _WIFI_OB                                /* Enable WIFI IO interrupt          */
#define _WIFI_FOR_SYSTEM_                       /* Enable save WIFI information      */
//#define _WIFI_5G_AP6234                       /* Enable WIFI 5G Module             */
//#define _WIFI_AP6181
#define _WIFI_AP6212
//#define _WIFI_5G_AP6255
#endif//#ifdef _WICE_ end
//#define _REALTEK_
#endif//#if defined(_WIFI_) || defined(_ENABLE_WIFI_BLUETOOTH)  end

#define USE_LWIP

#if !defined(_WIFI_) || defined(_ENABLE_WIFI_BLUETOOTH)
	#define _BLUETOOTH_                             /* Enable Bluetooth                  */
#endif

#define _USB_                                   /* Enable USB                        */
#ifdef _USB_
//#define USB_HOST
#define    USBMSC_LUN  10
#endif
#define _FS_

#define _MUSIC_                                 /* Enable Music                      */
#ifdef _MUSIC_                                  /* -----MUSIC config-----------------*/
    //#define PA_CS8508L                        /* Enable power amplifier            */
    #define CODEC_24BIT                         /* Enable 24 Bit codec               */
    #define _RK_EQ_                             /* Enable EQ                         */

    #define _MP3_DECODE_                        /* Enable MP3 decode                 */
    #define _SBC_DECODE_                        /* Enable SBC decode                 */
    #define _WAV_DECODE_                        /* Enable WAV decode                 */
#ifdef CODEC_24BIT
    #define _HIFI_ALAC_DEC                      /* Enable HIFI ALAC decode           */
#endif

#ifdef _SPI_BOOT_
#if(SPI_LUN0_SIZE > 3)
    #define _AMR_DECODE_                        /* Enable AMR decode                 */
//    #define _XXX_DECODE_                        /* Enable XXX decode                 */
    #define _OGG_DECODE_                        /* Enable OGG decode                 */
    #define _APE_DECODE_                        /* Enable APE decode                 */
    #define _AAC_DECODE_                        /* Enable AAC decode                 */
    #define _FLAC_DECODE_                       /* Enable FLAC decode                */

    #ifdef CODEC_24BIT
    #define _HIFI_APE_DEC                       /* Enable HIFI APE decode            */
    #define _HIFI_FLAC_DEC                      /* Enable HIFI FLAC decode           */
    #endif
#endif
#endif

#ifdef _EMMC_BOOT_
    #define _AMR_DECODE_                        /* Enable AMR decode                 */
    //#define _XXX_DECODE_                        /* Enable XXX decode                 */
    #define _OGG_DECODE_                        /* Enable OGG decode                 */
    #define _APE_DECODE_                        /* Enable APE decode                 */
    #define _AAC_DECODE_                        /* Enable AAC decode                 */
    #define _FLAC_DECODE_                       /* Enable FLAC decode                */

    #ifdef CODEC_24BIT
    #define _HIFI_APE_DEC                       /* Enable HIFI APE decode            */
    #define _HIFI_FLAC_DEC                      /* Enable HIFI FLAC decode           */
    #endif

#endif



    #define _RK_SPECTRUM_                       /* Enable Spectrum                   */
    #define _FADE_PROCESS_                      /* Enable fade operator              */
    #define _RK_ID3_                            /* Enable ID3                        */
    //#define _RK_CUE_                          /* Enable CUE                        */
    #define AUDIOHOLDONPLAY                     /* Enable breakpoint play            */
    #define SCROLL_LRC                          /* Reserved                          */
    //#define _MP3_LOG_                         /* Reserved                          */
#endif  //#ifdef _MUSIC_ end


#define _RECORD_                                /* Enable Record                     */
#ifdef _RECORD_                                 /* -----Record config----------------*/
    #define _WAV_ENCODE_                        /* Enable WAV encode                 */

#ifdef SPI_BOOT
#if(SPI_LUN0_SIZE > 3)
    #define _AMR_ENCODE_                        /* Enable WAV encode                 */
    //#define _XXX_ENCODE_                        /* Enable XXX encode                 */
#endif
#endif

#ifdef _EMMC_BOOT_
    #define _AMR_ENCODE_                        /* Enable WAV encode                 */
    //#define _XXX_ENCODE_                        /* Enable XXX encode                 */
#endif

#endif

//#define _LINEIN_//jjjhhh

#define _RADIO_                                 /* Enable Radio                      */
#ifdef _RADIO_                                  /* -----Radio config-----------------*/
    #ifdef _RECORD_                             /* -----Record config----------------*/
        //#define _FM_RECORD_                   /* Enable FM Record                  */
    #endif
#endif


//#define _VIDEO_                               /* Enable Video                      */
#ifdef _VIDEO_                                  /* -----Video config-----------------*/
    #define VIDEO_MP2_DECODE                    /* Enable Video mp2 decode           */
    #define VIDEO_AVI                           /* Enable Video AVI decode           */
    #define VIDEO_HOLDON_PLAY                   /* Enable Video breakpoint play      */
#endif


//#define _PICTURE_                             /* Enable Picture                    */
#ifdef _PICTURE_                                /* -----Picture config---------------*/
    #define _JPG_DECODE_                        /* Enable JPG decode                 */
    #define _BMP_DECODE_                        /* Enable BMP decode                 */
    //#define PIC_MEDIA                         /* Enable picture media              */
#endif

#ifndef _SPI_BOOT_
    #ifdef _USE_GUI_
        #define _MEDIA_MODULE_                        /* Enable Media Module               */
    #endif
#endif
#ifdef _MEDIA_MODULE_                           /* -----Media Module config----------*/
#define BROSUB                                  /* Enable browse directory           */
#define MEDIA_UPDATE                            /* Enable media update               */
#define FAVOSUB                                 /* Enable favorate directory         */
#endif


//#define _EBOOK_                               /* Enable EBOOK                      */

#define _BROWSER_                               /* Enable browser                    */

#define _SYSSET_                                /* Enable system setting             */


/*************************************************************************************
 *                               Misc config
 *************************************************************************************/
#define SYS_MIN_FREQ        1                  /* System minimum frequency 1 MHZ     */
#define SYS_DEFAULT_FREQ    24                 /* System default frequency 24 MHZ    */
#define SYS_MAX_FREQ        200                /* System Maximum frequency 300 MHZ   */


//#define _MEMORY_LEAK_CHECH_                    /* Enable Memory check                */
#ifdef _MEMORY_LEAK_CHECH_                     /* -----Memory check config-----------*/
//#define _SUPPORT_SORT_BY_ADDR_               /* Reserved                           */
#define _SUPPORT_MEMORY_COUNT_                 /* Enable memory count for every thread*/
#endif


//#define _DUAL_CORE_                          /* Reserved                           */
//#define _TEST_MUSIC_                         /* Reserved                           */
//#define DC_FILTER                            /* Reserved                           */

#define _SUPPORT_BB_SYSTEM_                    /* Enable B Core                      */
#ifdef _SUPPORT_BB_SYSTEM_                     /* -----B Core config-----------------*/
    //#define BB_SYS_JTAG                      /* Reserved                           */
#endif  //_SUPPORT_BB_SYSTEM_ end

#ifdef B_CORE_DECODE                           /* -----B Core decode config----------*/
#define A_CORE_DECODE                          /* Enable A Core decode               */
#endif

//#define _BEEP_                               /* Enable beep                        */
#ifdef _BEEP_                                  /* -----Beep config-------------------*/
#define _BEEP_NOT_MIXED_                       /* Enable beep and no mixed           */
#endif

#define _SUPPORT_PA_EN
#define HP_DET_CONFIG                        /* Reserved                           */
#ifdef HP_DET_CONFIG
	#define HP_DET GPIOPortB_Pin3
#endif
#define _Vol_Tab_General                       /* Enable volume table                */

//#define SUPPORT_YUV                          /* Enable YUV                         */
#ifdef SUPPORT_YUV                             /* -----YUV config--------------------*/
#define USE_LLP                                /* Enable LIP                         */
#endif

#define _FRAME_BUFFER_                         /* Enable framebuffer                 */
#define FILE_ERROR_DIALOG                      /* Enable Dialog Display              */



/*************************************************************************************
 *                               Board config
 *************************************************************************************/
//#define BOARD_VERSION_RKNANOD_SDK_MAIN_V10_20150126HJH   /* Enable Board Version1.0*/
#ifdef BOARD_VERSION_RKNANOD_SDK_MAIN_V10_20150126HJH
#include "board_main_v10_20150126_config.h"
#endif

//#define BOARD_VERSION_RKNANOD_SDK_MAIN_V21_20150515HJH   /* Enable Board Version2.1*/
#ifdef BOARD_VERSION_RKNANOD_SDK_MAIN_V21_20150515HJH
#include "board_main_v21_20150515_config.h"
#endif

#define BOARD_VERSION_RKNANOD_SDK_MAIN_V22_20150727HJH     /* Enable Board Version2.2*/
#ifdef BOARD_VERSION_RKNANOD_SDK_MAIN_V22_20150727HJH
#include "board_main_v22_20150727_config.h"
#endif

#include "source_macro.h"



/*
**************************************************************************************
*
*                                 End of BspConfig.h
*
**************************************************************************************
*/
#endif

