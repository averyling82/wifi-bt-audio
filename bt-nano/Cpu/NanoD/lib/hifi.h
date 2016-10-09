/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi.h
* Owner: WJR
* Date: 2014.11.28
* Time: 14:09:07
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    WJR     2014.11.28     14:09:07   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOD_LIB_HIFI_H__
#define __CPU_NANOD_LIB_HIFI_H__

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
#define _CPU_NANOD_LIB_HIFI_COMMON_ __attribute__((section("cpu_nanod_lib_hifi_common")))
#define _CPU_NANOD_LIB_HIFI_INIT_  __attribute__((section("cpu_nanod_lib_hifi_init")))
#define _CPU_NANOD_LIB_HIFI_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_shell")))

/****  ACC_DMACR ******/
#define Transmit_Data_Level_1              (uint32)(0x0 << 0)//WATERMARK
#define Transmit_Data_Level_4              (uint32)(0x4 << 0)
#define Transmit_Data_Level_8              (uint32)(0x8 << 0)
#define Transmit_Data_Level_16             (uint32)(0x10 << 0)
#define Transmit_Data_Level_32             (uint32)(0xlf << 0)
#define Transmit_DMA_Enable                (uint32)(0x1 << 8)
#define Transmit_DMA_Disable               (uint32)(0x0 << 8)

#define Receive_Data_Level_1               (uint32)(0x0 << 16)//WATERMARK
#define Receive_Data_Level_4               (uint32)(0x4 << 16)//WATERMARK
#define Receive_Data_Level_8               (uint32)(0x8 << 16)//WATERMARK
#define Receive_Data_Level_16              (uint32)(0x10 << 16)//WATERMARK
#define Receive_Data_Level_32              (uint32)(0x1f << 16)//WATERMARK
#define Receive_DMA_Enable               (uint32)(0x01 << 24)
#define Receive_DMA_Disable              (uint32)(0x00 << 24)
/****  ACC_INTCR ******/
#define TX_Empty_Interrupt_Enable        (uint32)(0x01 << 0)
#define TX_Empty_Interrupt_Disable       (uint32)(0x00 << 0)
#define TX_Underrun_Interrupt_Enable     (uint32)(0x01 << 1)
#define TX_Underrun_Interrupt_Disable    (uint32)(0x00 << 1)
#define TX_Empty_Interrupt_clear         (uint32)(0x01 << 2)
#define Transmit_FIFO_Threshold_1        (uint32)(0x0 << 4)
#define Transmit_FIFO_Threshold_4        (uint32)(0x4 << 4)
#define Transmit_FIFO_Threshold_8        (uint32)(0x8 << 4)
#define Transmit_FIFO_Threshold_16       (uint32)(0x10 << 4)
#define Transmit_FIFO_Threshold_32       (uint32)(0x1F << 4)
#define Function_DONE_CLEAR              (uint32)(0x00 << 15)
#define Function_DONE_INT_ENABLE         (uint32)(0x01 << 15)
#define RX_FULL_Interrupt_Enable         (uint32)(0x01 << 16)
#define RX_Overrun_Interrupt_Enable      (uint32)(0x01 << 17)
#define RX_Overrun_Interrupt_Disable     (uint32)(0x00 << 17)
#define RX_Overrun_Interrupt_clear       (uint32)(0x01 << 18)
#define Receive_FIFO_Threshold_1         (uint32)(0x0 << 20)
#define Receive_FIFO_Threshold_4         (uint32)(0x4 << 20)
#define Receive_FIFO_Threshold_8         (uint32)(0x8 << 20)
#define Receive_FIFO_Threshold_16        (uint32)(0x10 << 20)
#define Receive_FIFO_Threshold_32         (uint32)(0x1F << 20)

/****  ACC_INTSR ******/
#define TX_Empty_Interrupt_Active        (uint32)(0x1 << 0)
#define TX_Empty_Interrupt_Inacttive     (uint32)(0x0 << 0)
#define TX_Underrun_Interrupt_Active     (uint32)(0x1 << 1)
#define TX_Underrun_Interrupt_Inacttive  (uint32)(0x0 << 1)
#define Function_done_interrupt_active   (uint32)(0x1 << 15)
#define RX_Full_Interrupt_Active         (uint32)(0x1 << 16)
#define RX_Full_Interrupt_Inacttive      (uint32)(0x0 << 16)
#define RX_Overrun_Interrupt_Active      (uint32)(0x1 << 17)
#define RX_Overrun_Interrupt_Inacttive   (uint32)(0x0 << 17)

/****  ACC_XFER ******/
#define TX_Transfer_Start                (uint32)(0x01 << 0)
#define TX_Transfer_Stop                 (uint32)(0x00 << 0)
#define RX_Transfer_Start                (uint32)(0x01 << 1)
#define RX_Transfer_Stop                 (uint32)(0x00 << 1)
#define Data_count                       (uint32)(0x1ff << 20)
#define HIfi_ACC_TYPE_APE_L              (uint32)(0x0 << 29)
#define HIfi_ACC_TYPE_APE_R              (uint32)(0x1 << 29)
#define HIfi_ACC_TYPE_FFT                (uint32)(0x2 << 29)
#define HIfi_ACC_TYPE_ALAC               (uint32)(0x3 << 29)
#define HIfi_ACC_TYPE_MAC                (uint32)(0x4 << 29)
#define HIfi_ACC_TYPE_FLAC               (uint32)(0x5 << 29)

/****  ACC_CLR ********/
#define TX_LOGIC_CLEAR                   (uint32)(0x1 << 0)
#define RX_LOGIC_CLEAR                   (uint32)(0x1 << 1)


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
extern rk_err_t HIFI_SHELL(void);
extern rk_err_t Hifi_Clear_FUN_DONE_FLAG(UINT32 HifiId);
extern rk_err_t Hifi_Enable_FUN_DONE_FLAG(UINT32 HifiId);
extern rk_err_t HIFITranData(int32 *pSrc,int32 *pDst,int count);
extern rk_err_t HIFITranData_fifo(int32 *pSrc_t,int32 *pDst_t,int count_t,int32 *pSrc_r,int32 *pDst_r,int *count_r);
extern rk_err_t HIFI_DMA_M2M_register();
extern rk_err_t HIFI_DMA_M2M_Unregister();
extern rk_err_t HIFI_DMA_MEM_ACC(int *pSrc,int *pDst,int count);
extern rk_err_t HIFI_DMA_TO_register();
extern rk_err_t HIFI_DMA_TO_Unregister();
extern  rk_err_t HIFI_DMA_TO_ACC(int *pSrc_t,int *pDst_t,int count,int *pSrc_r,int *pDst_r);
extern rk_err_t Hifi_Set_ACC_XFER_Start(UINT32 HidiId,int count,int fun_type);
extern rk_err_t Hifi_Set_ACC_XFER_Disable(UINT32 HidiId,int count,int fun_type);
extern RK_ERR_T Hifi_Get_ACC_Intsr(UINT32 HifiId,int mode);
extern rk_err_t Hifi_Set_ACC_Intcr(UINT32 HifiId);
extern rk_err_t Hifi_Set_ACC_Dmacr(UINT32 HifiId);
extern rk_err_t Hifi_Set_ACC_clear(UINT32 HifiId);
;



#endif

