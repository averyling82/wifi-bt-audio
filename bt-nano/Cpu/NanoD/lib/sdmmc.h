/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Cpu\NanoC\lib\sdmmc.h
* Owner: Aaron.sun
* Date: 2014.2.20
* Time: 10:11:15
* Desc: sd mmc control bsp driver
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.2.20     10:11:15   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_SDMMC_H__
#define __CPU_NANOC_LIB_SDMMC_H__

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
#define FIFO_DEPTH        (0x20)       //FIFO depth = 32 word

#if 0
#define FOD_FREQ              (400)    //卡识别阶段使用的频率,单位KHz,协议规定最大400KHz
//卡正常工作的最低频率为FREQ_HCLK_MAX/8
#define SD_FPP_FREQ           (24000)  //标准SD卡正常工作频率，单位KHz，协议规定最大25MHz
#define SDHC_FPP_FREQ         (48000)  //SDHC卡在高速模式下的工作频率，单位KHz，协议规定最大50MHz
#define MMC_FPP_FREQ          (18000)  //标准MMC卡正常工作频率，单位KHz，协议规定最大20MHz
#define MMCHS_26_FPP_FREQ     (25000)  //高速模式只支持最大26M的HS-MMC卡，在高速模式下的工作频率，单位KHz，协议规定最大26MHz
#define MMCHS_52_FPP_FREQ     (50000)  //高速模式能支持最大52M的HS-MMC卡，在高速模式下的工作频率，单位KHz，协议规定最大52MHz
#define MAX_CLKIN_FREQ        (50000)
#else
//考虑到系统变频对控制器的影响，这种影响在OS上越来越明显，考虑到系统对控制器的速度要求不是很高，故将控制器固定到24M
//相应的频率下调
#define FOD_FREQ              (400)    //卡识别阶段使用的频率,单位KHz,协议规定最大400KHz
//卡正常工作的最低频率为FREQ_HCLK_MAX/8
#define SD_FPP_FREQ           (24000)  //标准SD卡正常工作频率，单位KHz，协议规定最大25MHz
#define SDHC_FPP_FREQ         (24000)  //SDHC卡在高速模式下的工作频率，单位KHz，协议规定最大50MHz
#define MMC_FPP_FREQ          (18000)  //标准MMC卡正常工作频率，单位KHz，协议规定最大20MHz
#define MMCHS_26_FPP_FREQ     (24000)  //高速模式只支持最大26M的HS-MMC卡，在高速模式下的工作频率，单位KHz，协议规定最大26MHz
#define MMCHS_52_FPP_FREQ     (24000)  //高速模式能支持最大52M的HS-MMC卡，在高速模式下的工作频率，单位KHz，协议规定最大52MHz
#define MAX_CLKIN_FREQ        (24000)

#endif

/* Command Register */
#define START_CMD           (0x1U << 31) //start command
#define USE_HOLD_REG        (1 << 29)
#define VOLT_SWITCH         (1 << 28)
#define BOOT_MODE           (1 << 27)
#define DISABLE_BOOT        (1 << 26)
#define EXPECT_BOOT_ACK     (1 << 25)
#define ENABLE_BOOT         (1 << 24)
#define CCS_EXPECTED        (1 << 23)
#define READ_CEATA          (1 << 22)

#define UPDATE_CLOCK        (1 << 21)    //update clock register only
#define SEND_INIT           (1 << 15)    //send initialization sequence
#define STOP_CMD            (1 << 14)    //stop abort command
#define NO_WAIT_PREV        (0 << 13)    //not wait previous data transfer complete, send command at once
#define WAIT_PREV           (1 << 13)    //wait previous data transfer complete
#define AUTO_STOP           (1 << 12)    //send auto stop command at end of data transfer
#define BLOCK_TRANS         (0 << 11)    //block data transfer command
#define STREAM_TRANS        (1 << 11)    //stream data transfer command
#define READ_CARD           (0 << 10)    //read from card
#define WRITE_CARD          (1 << 10)    //write to card
#define NOCARE_RW           (0 << 10)    //not care read or write
#define NO_DATA_EXPECT      (0 << 9)     //no data transfer expected
#define DATA_EXPECT         (1 << 9)     //data transfer expected
#define NO_CHECK_R_CRC      (0 << 8)     //do not check response crc
#define CHECK_R_CRC         (1 << 8)     //check response crc
#define NOCARE_R_CRC        CHECK_R_CRC  //not care response crc
#define SHORT_R             (0 << 7)     //short response expected from card
#define LONG_R              (1 << 7)     //long response expected from card
#define NOCARE_R            SHORT_R      //not care response length
#define NO_R_EXPECT         (0 << 6)     //no response expected from card
#define R_EXPECT            (1 << 6)     //response expected from card

//sdc event
#define SDC_EVENT_SDC_ERR       (0x1 << 0)
#define SDC_EVENT_RX_DATA_REQ   (0x01 << 1)
#define SDC_EVENT_TX_DATA_REQ   (0x01 << 2)
#define SDC_EVENT_CMD_OK        (0x01 << 3)
#define SDC_EVENT_DATA_TRANS_OK (0x01 << 4)
#define SDC_EVENT_SDIO          (0X01 << 5)
#define SDC_EVENT_RESP_TIMEOUT  (0X01 << 6)
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
extern void SDC_DisableDMA(SDMMC_PORT SDCPort);
extern void SDC_EnableDMA(SDMMC_PORT SDCPort);
extern uint32 SDC_GetFIFOAddr(SDMMC_PORT SDCPort);
extern rk_err_t SDC_CheckDataBusy(SDMMC_PORT SDCPort);
extern void SDC_ClearIRQ(SDMMC_PORT SDCPort, uint32 intmask);
extern rk_err_t SDC_SetBusWidth(SDMMC_PORT SDCPort, BUS_WIDTH width);
extern uint32 SDC_GetResponse(SDMMC_PORT SDCPort, uint32 ResponseNum);
extern rk_err_t SDC_SetByteCnt(SDMMC_PORT SDCPort, uint32 ByteCnt);
extern rk_err_t SDC_SetCmdArg(SDMMC_PORT SDCPort, uint32 CmdArg);
extern rk_err_t SDC_SetBlockSize(SDMMC_PORT SDCPort, uint32 BlockSize);
extern rk_err_t SDC_SetDmaMode(SDMMC_PORT SDCPort, uint32 OpenDma);
extern uint32 SDC_GetIRQ(SDMMC_PORT SDCPort, uint32 * Int);
extern rk_size_t SDC_ReadData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize);
extern rk_size_t SDC_WriteData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize);
extern rk_err_t SDC_ResetFIFO(SDMMC_PORT SDCPort);
extern rk_err_t SDC_Init(SDMMC_PORT SDCPort, uint32 FreqKHz);
extern rk_err_t SDC_UpdateFreq(SDMMC_PORT SDCPort, uint32 FreqKHz);



#endif

