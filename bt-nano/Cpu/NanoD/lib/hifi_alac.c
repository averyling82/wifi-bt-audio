/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_alac.c
* Owner: WJR
* Date: 2014.11.28
* Time: 16:39:16
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     16:39:16   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_ALAC_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"

#ifdef _HIFI_SHELL_
#include "hifi_test_data_alac.h"
#endif

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HIFI_ALAC_READ_  __attribute__((section("cpu_nanod_lib_hifi_alac_read")))
#define _CPU_NANOD_LIB_HIFI_ALAC_WRITE_ __attribute__((section("cpu_nanod_lib_hifi_alac_write")))
#define _CPU_NANOD_LIB_HIFI_ALAC_INIT_  __attribute__((section("cpu_nanod_lib_hifi_alac_init")))
#define _CPU_NANOD_LIB_HIFI_ALAC_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_alac_shell")))

#define DMA_TRAN_ALAC
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
** Input:UINT32 HifiId,int16_t *lpc_coefs,int lpc_order, int lpc_quant,int bps
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 16:40:51
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_ALAC_READ_
COMMON API rk_err_t Alac_Set_CFG(UINT32 HifiId,int lpc_order, int lpc_quant,int bps)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->ALAC_CFG = (lpc_order <<16) |(lpc_quant<<8)|(bps << 24);
       //  printf("ALAC_CFG 0x%x \n",pHifi->ALAC_CFG);

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
** Name: SHELL_ALAC_TEST
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.4
** Time: 18:25:06
*******************************************************************************/
#ifdef _HIFI_SHELL_
extern unsigned long SysTickCounter;
_CPU_NANOD_LIB_HIFI_ALAC_SHELL_
SHELL API rk_err_t hifi_alac_shell(void)
{


    int i,k;
    int count_rc=0;
    int  SysTick,Tick,Time;
    #ifdef  alac_test1
    int lpc_order = 0x20;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 128;
    #endif
    #ifdef  alac_test2
    int lpc_order = 0x8;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 128;
    #endif
    #ifdef  alac_test3
    int lpc_order = 0x8;
    int lpc_quant = 0x9;
    int lpc_bps  = 0x11;
    int frm_size = 4096;
    #endif
    int count = frm_size;
    int circle = frm_size/count;
    rk_printf_no_time("-------------ALAC START-------------");
    SysTick = SysTickCounter;

    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_ALAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    HIFI_DMA_TO_register();
    for(k =0;k<96;k++)
    {
    Hifi_Set_ACC_XFER_Disable(0,frm_size,HIfi_ACC_TYPE_ALAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Enable_FUN_DONE_FLAG(0);
    Alac_Set_CFG(0,lpc_order,lpc_quant,lpc_bps);
    HIFITranData((UINT32*)alac_coef,(uint32*)ALAC_COEF_ADD,lpc_order);
    Hifi_Set_ACC_XFER_Start(0,frm_size,HIfi_ACC_TYPE_ALAC);//可以开始往fifo传数据，并且可以取数据
    for(i=0;i<circle;i++)
    {
#ifdef DMA_TRAN_ALAC
        HIFI_DMA_TO_ACC(&test_data_alac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_alac[i*count]);
#else
         count_rc=0;
        HIFITranData_fifo(&test_data_alac[i*count],(uint32*)TX_FIFO,count,(uint32*)RX_FIFO,&test_output_alac[i*count],&count_rc);
#endif
     }
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    }
    Tick = SysTickCounter -  SysTick;
    Time = Tick *10;
    rk_printf_no_time("\nDMA alac %d num is %d(ms)",frm_size,Time);
    HIFI_DMA_TO_Unregister();
    {

        for(i=0;i<count;i++)
        {
            if( test_output_alac[i] != output_alac[i])
            {
              break;
            }
        }
        if(i == count )
        {
            rk_printf_no_time("%d  %d num alac test over\n",lpc_order,frm_size);
            return  RK_SUCCESS;
        }
        else
        {
            rk_printf_no_time("alac test error from %d \n ",i);
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

