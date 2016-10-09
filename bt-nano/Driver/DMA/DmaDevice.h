/*
********************************************************************************************
*
*  Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\DMA\DmaDevice.h
* Owner: Aaron.sun
* Date: 2014.4.24
* Time: 10:57:36
* Desc: Dma Device Class
* History:
*	 <author>	 <date> 	  <time>	 <version>	   <Desc>
* Aaron.sun     2014.4.24     10:57:36   1.0
********************************************************************************************
*/

#ifndef __DRIVER_DMA_DMADEVICE_H__
#define __DRIVER_DMA_DMADEVICE_H__

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
#define _DRIVER_DMA_DMADEVICE_COMMON_  __attribute__((section("driver_dma_dmadevice_common")))
#define _DRIVER_DMA_DMADEVICE_INIT_  __attribute__((section("driver_dma_dmadevice_init")))
#define _DRIVER_DMA_DMADEVICE_SHELL_  __attribute__((section("driver_dma_dmadevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_DMA_DMADEVICE_DATA_      _DRIVER_DMA_DMADEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_DMA_DMADEVICE_DATA_      __attribute__((section("driver_dma_dmadevice_data")))
#else
#error Unknown compiling tools.
#endif

typedef     void (*pDmaFunc)(uint32 ch);

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
extern void DmaSoftIntIsr(void);
extern void DmaIntIsr(void);
extern rk_err_t DmaDev_RealseChannel(HDC dev ,uint32 ch);
extern rk_err_t DmaDev_GetChannel(HDC dev);
extern rk_err_t DmaDev_DmaStartIIS(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pFunc CallBack);
extern rk_err_t DmaDev_DmaReStart(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack);
extern rk_err_t DmaDev_DmaStart(HDC dev, uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDmaFunc CallBack);
extern rk_err_t DmaDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t DmaDevDelete(uint32 DevID, void * arg);
extern HDC DmaDev_Create(uint32 DevID, void * arg);



#endif

