/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoC\lib\Hw_hifi.h
* Owner: WJR
* Date: 2014.11.28
* Time: 14:10:58
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    WJR     2014.11.28     14:10:58   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_HW_HIFI_H__
#define __CPU_NANOC_LIB_HW_HIFI_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#include "hw_memap.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _HIFIACC
{

    uint32 ACC_DMACR;
    uint32 ACC_INTCR;
    uint32 ACC_INTSR;
    uint32 FUN_CFG;
    uint32 ACC_CLR;
    uint32 reserved0; //14
    uint32 reserved1; //18
    uint32 reserved2; // 1c
    uint32 APE_CFG;
    uint32 FFT_CFG;
    uint32 ALAC_CFG;
    uint32 FLAC_CFG;
    uint32 MAC_CFG;
    uint32 MAC_RSL;   // 34 MAC data LOW 32 bit out [31:0]
    uint32 MAC_RSH;   // 38 MAC data high 32 bit out [31:0]

}HIFIACC;

#define HifiReg              ((HIFIACC *)HIFIACC_BASE)

#define HifiPort(n)          ( HifiReg + n )

#define   TX_FIFO                 (HIFIACC_BASE + 0x40)   //Transimt FIFO Data Register
#define   RX_FIFO                 (HIFIACC_BASE + 0xA0)   //Receive FIFO Data Register
#define   FFT_DATI_ADD            (HIFIACC_BASE + 0x1000)   //fft im data address, max=1024
#define   FFT_DATR_ADD            (HIFIACC_BASE + 0x2000)   //fft real data address , max=1024
#define   APEL_COEF_ADD           (HIFIACC_BASE + 0x3000)   //Ape left channel coef data max=320+256+64
#define   APER_COEF_ADD           (HIFIACC_BASE + 0x4000)   //Ape right channel coef data max=320+256+64
#define   APEL_ACOEF_ADD          (HIFIACC_BASE + 0x5000)   //Ape left channel adaptcoef max=320+256+64
#define   APER_ACOEF_ADD          (HIFIACC_BASE + 0x6000)   //Ape right channel adaptcoef max=320+256+64
#define   APEL_DATA_ADD           (HIFIACC_BASE + 0x7000)   //Ape left channel data max=320+256+64
#define   APER_DATA_ADD           (HIFIACC_BASE + 0x8000)   //Ape right channel data max=320+256+64
#define   ALAC_COEF_ADD           (HIFIACC_BASE + 0x9000)   //Alac initial coef address max=64x32bit
#define   ALAC_DATA_ADD           (HIFIACC_BASE + 0xA000)   //alac initial data address max=64x32bit
#define   FLAC_COEF_ADD           (HIFIACC_BASE + 0xB000)   //flac initial coef address max=64x32bit
#define   FLAC_DATA_ADD           (HIFIACC_BASE + 0xC000)   //flac initial coef address max=64x32bit

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

