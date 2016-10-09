/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_fft.c
* Owner: WJR
* Date: 2014.11.28
* Time: 16:50:28
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     16:50:28   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_FFT_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "hifi_test_data_fft.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HIFI_FFT_READ_  __attribute__((section("cpu_nanod_lib_hifi_fft_read")))
#define _CPU_NANOD_LIB_HIFI_FFT_WRITE_ __attribute__((section("cpu_nanod_lib_hifi_fft_write")))
#define _CPU_NANOD_LIB_HIFI_FFT_INIT_  __attribute__((section("cpu_nanod_lib_hifi_fft_init")))
#define _CPU_NANOD_LIB_HIFI_FFT_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_fft_shell")))
#define  ABS(x) ((x) > 0 ? (x) : -(x))

#define DMA_TRAN_FFT
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------'------------------------------------------------------------------------------------------
*/



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
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FFT_Set_CFG
** Input:UINT32 HifiId,int N_point
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 16:50:39
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_FFT_READ_
COMMON API rk_err_t FFT_Set_CFG(UINT32 HifiId,int num,int type)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->FFT_CFG = type;
    if(num==64)
    {
       pHifi->FFT_CFG |=FFT_MODE_64;
    }
    else if(num ==128)
    {
       pHifi->FFT_CFG |=FFT_MODE_128;
    }
     else if(num ==256)
    {
       pHifi->FFT_CFG |=FFT_MODE_256;
    }
      else if(num ==512)
    {
       pHifi->FFT_CFG |=FFT_MODE_512;
    }
       else if(num ==1024)
    {
       pHifi->FFT_CFG |=FFT_MODE_1024;
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
/*******************************************************************************
** Name: SHELL_FFT_TEST
** Input:void
**注意:
******FFT必须在xfer disable时才能传数据，并且必须使用fun_done标志，才能取输出数据。
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 16:47:18
*******************************************************************************/
#ifdef _HIFI_SHELL_
_CPU_NANOD_LIB_HIFI_FFT_SHELL_
extern unsigned long SysTickCounter;
SHELL API rk_err_t FFT_TEST(int fft_point,int* test_data_fft,int* output_fft_R ,int* output_fft_I)
{

    int i,k;
	  int count = fft_point;
    int times = 1024/count;
    int  SysTick,Tick,Time;
    int circle = 192*2;//*1024/count;
    SysTick = SysTickCounter;
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);
    HIFI_DMA_M2M_register();
    for(k =0;k<circle;k++)
    {
    /****************fft********************/
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    FFT_Set_CFG(0,count,FFT_FLAG);

     for(i=0;i<times;i++)
    {
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC(&test_data_fft[i*count],(uint32*)FFT_DATR_ADD,count);
#else
    HIFITranData(&test_data_fft[i*count],(uint32*)FFT_DATR_ADD,count);
#endif
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_FFT);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active);
    Hifi_Clear_FUN_DONE_FLAG(0);//清中断
    Hifi_Set_ACC_XFER_Disable(0,0,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATR_ADD,&test_output_fft_R[i*count],count);
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATI_ADD,&test_output_fft_I[i*count],count);
#else
    HIFITranData((uint32*)FFT_DATR_ADD,&test_output_fft_R[i*count],count);
    HIFITranData((uint32*)FFT_DATI_ADD,&test_output_fft_I[i*count],count);
#endif
   }
#if 1
    /****************Ifft********************/
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    FFT_Set_CFG(0,count,IFFT_FLAG);
    for(i=0;i<times;i++)
    {
#ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC(&output_fft_I[i*count],(uint32*)FFT_DATI_ADD,count);
    HIFI_DMA_MEM_ACC(&output_fft_R[i*count],(uint32*)FFT_DATR_ADD,count);
#else
    HIFITranData(&output_fft_I[i*count],(uint32*)FFT_DATI_ADD,count);
    HIFITranData(&output_fft_R[i*count],(uint32*)FFT_DATR_ADD,count);
#endif
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_FFT);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);//清中断
    Hifi_Set_ACC_XFER_Disable(0,0,HIfi_ACC_TYPE_FFT);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
 #ifdef DMA_TRAN_FFT
    HIFI_DMA_MEM_ACC((uint32*)FFT_DATR_ADD,&test_output_ifft_R[i*count],count);
#else
    HIFITranData((uint32*)FFT_DATR_ADD,&test_output_ifft_R[i*count],count);
#endif
   }
    #endif
   }
    Tick = SysTickCounter -  SysTick;
    Time = Tick *10;
    rk_printf_no_time("\nDMA FFT IFFT %d num is %d(ms) --- %d",count,Time, Tick);
    HIFI_DMA_M2M_Unregister();
    {
        for(i=0;i<count;i++)
        {
           int diff_r = test_output_fft_R[i] - output_fft_R[i];
					 int diff_i = test_output_fft_I[i] - output_fft_I[i];

            if( (ABS(diff_r) )||(ABS(diff_i)))
            {
              break;
            }
        }
        if(i == count )
        {
            rk_printf_no_time("%d fft test over",count);
        }
        else
        {
            rk_printf_no_time("%d fft test error %d",count,i);

        }
    }
    {
        for(i=0;i<count;i++)
        {
           int diff = test_output_ifft_R[i] - test_data_fft[i];

            if( ABS(diff) > 5)
            {
              break;
            }
        }
        if(i == count )
        {
            rk_printf_no_time("%d ifft test over\n",count);
            return  RK_SUCCESS;
        }
        else
        {
            rk_printf_no_time("%d ifft test error %d \n ",count,i);
            return RK_ERROR  ;
        }
    }

}

SHELL API rk_err_t hifi_fft_shell(void )
{
  rk_printf_no_time("-------------FFT START-------------");

   FFT_TEST(64,test_data_fft_64,output_fft_R_64,output_fft_I_64);
   FFT_TEST(128,test_data_fft_128,output_fft_R_128,output_fft_I_128);
   FFT_TEST(256,test_data_fft_256,output_fft_R_256,output_fft_I_256);
   FFT_TEST(512,test_data_fft_512,output_fft_R_512,output_fft_I_512);
   FFT_TEST(1024,test_data_fft_1024,output_fft_R_1024,output_fft_I_1024);


}

#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

