/*
********************************************************************************************
*
*          Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                                 All rights reserved.
*
* FileName: Driver\Sdio\SdioDevice.h
* Owner: Aaron.sun
* Date: 2014.6.14
* Time: 9:10:44
* Desc: sdio device class
* History:
*    <author>      <date>        <time>      <version>        <Desc>
*    Aaron.sun     2014.6.14     9:10:44   1.0
********************************************************************************************
*/

#ifndef __DRIVER_SDIO_SDIODEVICE_H__
#define __DRIVER_SDIO_SDIODEVICE_H__

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
#define _DRIVER_SDIO_SDIODEVICE_COMMON_  __attribute__((section("driver_sdio_sdiodevice_common")))
#define _DRIVER_SDIO_SDIODEVICE_INIT_  __attribute__((section("driver_sdio_sdiodevice_init")))
#define _DRIVER_SDIO_SDIODEVICE_SHELL_  __attribute__((section("driver_sdio_sdiodevice_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_SDIO_SDIODEVICE_SHELL_DATA_      _DRIVER_SDIO_SDIODEVICE_SHELL_
#define _DRIVER_SDIO_SDIODEVICE_COMMON_DATA_      _DRIVER_SDIO_SDIODEVICE_COMMON_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_SDIO_SDIODEVICE_SHELL_DATA_      __attribute__((section("driver_sdio_sdiodevice_shell_data")))
#define _DRIVER_SDIO_SDIODEVICE_COMMON_DATA_      __attribute__((section("driver_sdio_sdiodevice_common_data")))
#else
#error Unknown compiling tools.
#endif


typedef void (sdio_irq_handler_t)(void *);

#define SDIO_DEV_MAX_CNT 1

typedef  struct _SDIO_DEV_ARG
{
    HDC hSdc;

}SDIO_DEV_ARG;

#define SDIO_FUN_0          0
#define SDIO_FUN_1          1
#define SDIO_FUN_2          2

/*
 * Card Common Control Registers (CCCR)
 */

#define SDIO_CCCR_CCCR          0x00

#define  SDIO_CCCR_REV_1_00     0     /* CCCR/FBR Version 1.00 */
#define  SDIO_CCCR_REV_1_10     1     /* CCCR/FBR Version 1.10 */
#define  SDIO_CCCR_REV_1_20     2     /* CCCR/FBR Version 1.20 */

#define  SDIO_SDIO_REV_1_00     0     /* SDIO Spec Version 1.00 */
#define  SDIO_SDIO_REV_1_10     1     /* SDIO Spec Version 1.10 */
#define  SDIO_SDIO_REV_1_20     2     /* SDIO Spec Version 1.20 */
#define  SDIO_SDIO_REV_2_00     3     /* SDIO Spec Version 2.00 */

#define SDIO_CCCR_SD          0x01

#define  SDIO_SD_REV_1_01     0     /* SD Physical Spec Version 1.01 */
#define  SDIO_SD_REV_1_10     1     /* SD Physical Spec Version 1.10 */
#define  SDIO_SD_REV_2_00     2     /* SD Physical Spec Version 2.00 */

#define SDIO_CCCR_IOEx          0x02
/* io_en */
#define SDIO_FUNC_ENABLE_1     0x02     /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2     0x04     /* function 2 I/O enable */

#define SDIO_CCCR_IORx          0x03
/* io_rdys */
#define SDIO_FUNC_READY_1     0x02     /* function 1 I/O ready */
#define SDIO_FUNC_READY_2     0x04     /* function 2 I/O ready */

#define SDIO_CCCR_IENx          0x04     /* Function/Master Interrupt Enable */
#define SDIO_CCCR_INTx          0x05     /* Function Interrupt Pending */

#define SDIO_CCCR_ABORT          0x06     /* function abort/card reset */

#define SDIO_CCCR_IF          0x07     /* bus interface controls */


#define _DRIVER_SDIO_SDIODEVICE_COMMON_  __attribute__((section("driver_sdio_sdiodevice_read")))
#define _DRIVER_SDIO_SDIODEVICE_COMMON_ __attribute__((section("driver_sdio_sdiodevice_write")))
#define _DRIVER_SDIO_SDIODEVICE_INIT_  __attribute__((section("driver_sdio_sdiodevice_init")))
#define _DRIVER_SDIO_SDIODEVICE_SHELL_  __attribute__((section("driver_sdio_sdiodevice_shell")))


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
extern rk_err_t SdioDev_Memcpy_FromIo(HDC hSdioFun, void *dst, uint32 addr, uint32 count);
extern rk_err_t SdioDev_Memcpy_ToIo(HDC hSdioFun, uint32 addr, void *src, uint32 count);
extern rk_err_t SdioDevDelete(uint32 DevID, void * arg);
extern rk_err_t SdioDev_DisalbeInt(HDC dev, uint32 FuncNum);
extern rk_err_t SdioDev_EnableInt(HDC dev, uint32 FunNum);
extern HDC SdioDev_GetFuncHandle(HDC dev, uint32 FuncNum);
extern rk_err_t SdioDev_Writew(HDC hSdioFun, uint32 b, uint32 addr);
extern rk_err_t SdioDev_Readw(HDC hSdioFunc, uint32 addr);
extern rk_err_t SdioDev_Writel(HDC hSdioFun, uint32 b, uint32 addr);
extern rk_err_t SdioDev_Readl(HDC hSdioFun, uint32 addr);
extern rk_err_t SdioDev_SetBlockSize(HDC hSdioFun, uint32 BlockSize);
extern rk_err_t SdioDev_WriteSb(HDC hSdioFun,uint32 addr, void *src, uint32 count);
extern rk_err_t SdioDev_ReadSb(HDC hSdioFun, void *dst, uint32 addr, uint32 count);
extern rk_err_t SdioDev_Readb(HDC hSdioFunc, uint32 addr);
extern rk_err_t SdioDev_Writeb(HDC hSdioFunc, uint8 b, uint32 addr);
extern rk_err_t SdioDev_DisableFunc(HDC hSdioFunc);
extern rk_err_t SdioDev_EnalbeFunc(HDC hSdioFunc);
extern rk_err_t SdioDev_Shell(HDC dev, uint8 * pstr);
extern HDC SdioDev_Create(uint32 DevID, void * arg);
extern void SdioIrqTask(void);
extern rk_err_t SdioIntIrqInit(void *pvParameters);
extern rk_err_t SdioIntIrqDeInit(void *pvParameters);
extern int SdioDev_Claim_irq(void *_func, sdio_irq_handler_t * handler);
extern int sdio_release_irq(void *func);



#endif

