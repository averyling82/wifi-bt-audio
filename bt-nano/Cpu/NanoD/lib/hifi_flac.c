/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_flac .c
* Owner: WJR
* Date: 2014.11.28
* Time: 16:31:16
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     16:31:16   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_FLAC_C__
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "hifi_test_data_flac.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HIFI_FLAC_READ_  __attribute__((section("cpu_nanod_lib_hifi_flac _read")))
#define _CPU_NANOD_LIB_HIFI_FLAC_WRITE_ __attribute__((section("cpu_nanod_lib_hifi_flac _write")))
#define _CPU_NANOD_LIB_HIFI_FLAC_INIT_  __attribute__((section("cpu_nanod_lib_hifi_flac _init")))
#define _CPU_NANOD_LIB_HIFI_FLAC_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_flac _shell")))
#define DMA_TRAN_FLAC

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
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
** Name: Aalc_Set_CFG
** Input:int16_t *lpc_coefs,int lpc_order, int lpc_quant
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 16:31:30
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_FLAC_READ_
COMMON API rk_err_t Flac_Set_CFG(UINT32 HifiId,int lpc_order, int lpc_quant)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->FLAC_CFG = (lpc_order <<16) |(lpc_quant<<8);
    //printf("FLAC_CFG 0x%x \n",pHifi->FLAC_CFG);
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
** Name: SHELL_FLAC_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 18:23:23
*******************************************************************************/
#ifdef _HIFI_SHELL_
_CPU_NANOD_LIB_HIFI_FLAC_READ_
extern unsigned long SysTickCounter;
SHELL API rk_err_t hifi_flac_shell(void)
{


    int i,k;
    int count_rc=0;
    int  SysTick,Tick,Time;
    #ifdef flac_test1
    int lpc_order = 0x20;
    int lpc_quant = 0xd;
    int frm_size = 128;
    #endif
    #ifdef flac_test2
    int lpc_order = 0xc;
    int lpc_quant = 0xd;
    int frm_size = 128;
    #endif
    #ifdef flac_test3
    int lpc_order = 0x8;
    int lpc_quant = 0xe;
    int frm_size = 4096;
    #endif
    int count = frm_size;
    int circle =frm_size/count;
    rk_printf_no_time("-------------FLAC START-------------");
    SysTick = SysTickCounter;

    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_FLAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    HIFI_DMA_TO_register();

    for(k =0;k<96;k++)
    {
	Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_FLAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
    Flac_Set_CFG(0,lpc_order,lpc_quant);
    HIFITranData((uint32*)flac_coef,(uint32*)FLAC_COEF_ADD,lpc_order);
    Hifi_Set_ACC_XFER_Start(0,frm_size,HIfi_ACC_TYPE_FLAC);//可以开始往fifo传数据，并且可以取数据
    for(i=0;i<circle;i++)
    {

#ifdef DMA_TRAN_FLAC
        HIFI_DMA_TO_ACC(&test_data_flac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_flac[i*count]);
#else
		count_rc=0;
        HIFITranData_fifo(&test_data_flac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_flac[i*count],&count_rc);
  #endif
    }
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    }
    Tick = SysTickCounter -  SysTick;
    Time = Tick *10;
    rk_printf_no_time("\nDMA flac %d num is %d(ms)",frm_size,Time);
    HIFI_DMA_TO_Unregister();

    {

        for(i=0;i<count;i++)
        {
            if( test_output_flac[i] != output_flac[i])
            {
              break;
            }
        }
        if(i == count )
        {
            rk_printf_no_time("%d  %d num flac test over\n",lpc_order,frm_size);
            return  RK_SUCCESS;
        }
        else
        {
            rk_printf_no_time("flac test error from %d \n ",i);
            return RK_ERROR  ;
        }
    }

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

