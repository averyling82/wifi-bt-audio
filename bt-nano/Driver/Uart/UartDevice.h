/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\Uart\UartDevice.h
* Owner: Aaron.sun
* Date: 2014.2.14
* Time: 13:53:19
* Desc: Uart Device Class
* History:
*     <author>     <date>       <time>     <version>       <Desc>
* Aaron.sun     2014.2.14     13:53:19   1.0
********************************************************************************************
*/

#ifndef __DRIVER_UART_UARTDEVICE_H__
#define __DRIVER_UART_UARTDEVICE_H__

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
#define _DRIVER_UART_UARTDEVICE_COMMON_  __attribute__((section("driver_uart_uartdevie_common")))
#define _DRIVER_UART_UARTDEVICE_INIT_  __attribute__((section("driver_uart_uartdevie_init")))
#define _DRIVER_UART_UARTDEVICE_SHELL_  __attribute__((section("driver_uart_uartdevie_shell")))
#if defined(__arm__) && (__ARMCC_VERSION)
#define _DRIVER_UART_UARTDEVICE_SHELL_DATA_      _DRIVER_UART_UARTDEVICE_SHELL_
#elif defined(__arm__) && (__GNUC__)
#define _DRIVER_UART_UARTDEVICE_SHELL_DATA_      __attribute__((section("driver_uart_uartdevice_shell_data")))
#else
#error Unknown compiling tools.
#endif


typedef rk_err_t (* pUartTx_complete)(void* buffer);      //tx callback funciton

typedef struct _UART_DEV_ARG
{
    uint32 dwBitRate;
    uint32 dwBitWidth;
    uint32 Channel;
    uint32 stopbit;
    uint32 parity;

}UART_DEV_ARG;


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
extern rk_err_t UartDev_SetFlowControl(HDC dev);
extern rk_err_t UartDev_SetBaudRate(HDC dev, uint32 BaudRate);
extern void UartIntIsr5(void);
extern void UartIntIsr4(void);
extern void UartIntIsr3(void);
extern void UartIntIsr2(void);
extern void UartIntIsr1(void);
extern void UartIntIsr0(void);
extern rk_err_t UartDev_Shell(HDC dev, uint8 * pstr);
extern rk_err_t UartDev_Delete(uint32 DevID, void *arg);
extern HDC UartDev_Create(uint32 DevID, void *arg);
extern rk_size_t UartDev_Read(HDC dev, void* buffer, uint32 size ,uint32 timeout);
extern rk_size_t UartDev_Write(HDC dev,const void* buffer, uint32 size, uint8 mode, pUartTx_complete Tx_complete);
#endif

