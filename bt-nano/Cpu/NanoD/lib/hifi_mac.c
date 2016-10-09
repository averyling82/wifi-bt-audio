/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi_mac .c
* Owner: WJR
* Date: 2014.11.28
* Time: 17:47:34
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     17:47:34   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_MAC_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HIFI_MAC_READ_  __attribute__((section("cpu_nanod_lib_hifi_mac _read")))
#define _CPU_NANOD_LIB_HIFI_MAC_WRITE_ __attribute__((section("cpu_nanod_lib_hifi_mac _write")))
#define _CPU_NANOD_LIB_HIFI_MAC_INIT_  __attribute__((section("cpu_nanod_lib_hifi_mac _init")))
#define _CPU_NANOD_LIB_HIFI_MAC_SHELL_  __attribute__((section("cpu_nanod_lib_hifi_mac _shell")))

#define DMA_TRAN_MAC
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
/*******************************************************************************
** Name: MAC_Get_result
** Input:UINT32 HifiId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 18:39:26
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_MAC_READ_
COMMON API long long MAC_Get_result(UINT32 HifiId)
{
    long long res;
    int test_L,test_R;
    HIFIACC * pHifi = HifiPort(HifiId);
    HIFITranData(&pHifi->MAC_RSH,&test_R,1);
    HIFITranData(&pHifi->MAC_RSL,&test_L,1);
    res = (test_R <<32)|(test_L);
  // printf("\n H:0x%x L:0x%x\n",test_R,test_L);
    return res;
}

/*******************************************************************************
** Name: MAC_Set_CFG
** Input:UINT32 HifiId,int order
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 17:47:41
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_MAC_READ_
COMMON API rk_err_t MAC_Set_CFG(UINT32 HifiId,int order)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->MAC_CFG |= (order<<18);
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
#ifdef _HIFI_SHELL_
_CPU_NANOD_LIB_HIFI_MAC_SHELL_
__align(4) int test_data_mac[] = {0x94777fff,0xfeff8000,0x88856807,0xfe23456};
__align(4) int test_coef_mac[] = {0x9acbd,0xeffeadc,0xff7000ab,0x3456789a};
int res_l= 0xe56e1bac;
int res_r = 0x036f6cf9;

__align(4) int a[4];
__align(4) int b[4];

extern unsigned long SysTickCounter;
SHELL API rk_err_t hifi_mac_shell(void)
{
    int count = 4;
    long long res=0;
    int  SysTick,Tick,Time;
      int  circle = 1024/4;
      int i;
    SysTick = SysTickCounter;
    for(i=0;i<circle;i++)
    {
    Hifi_Set_ACC_XFER_Disable(0,count,HIfi_ACC_TYPE_MAC);//开始传输配置数据和初始化系数(不往fifo送)
    Hifi_Set_ACC_clear(0);//fpga 内部已实现
    Hifi_Set_ACC_Dmacr(0);
    Hifi_Set_ACC_Intcr(0);
    Hifi_Enable_FUN_DONE_FLAG(0);
    MAC_Set_CFG(0,count);
    HIFI_DMA_M2M_register();
#ifdef DMA_TRAN_MAC
    HIFI_DMA_MEM_ACC(test_data_mac,(uint32*)FFT_DATR_ADD,count);
    HIFI_DMA_MEM_ACC(test_coef_mac,(uint32*)FFT_DATI_ADD,count);

#else
       HIFITranData(test_coef_mac,(uint32*)FFT_DATR_ADD,count);
    HIFITranData(test_data_mac,(uint32*)FFT_DATI_ADD,count);

#endif
    Hifi_Set_ACC_XFER_Start(0,count,HIfi_ACC_TYPE_MAC);//可以开始往fifo传数据，并且可以取数据
    while(Hifi_Get_ACC_Intsr(0,Function_done_interrupt_active) != Function_done_interrupt_active) ;
    Hifi_Clear_FUN_DONE_FLAG(0);
    res =MAC_Get_result(0);
    }
    Tick = SysTickCounter -  SysTick;
    Time = Tick *10;
    rk_printf_no_time("CPU mac %d num is %d(ms) --- %d\r\n",circle*count,Time, Tick);
    HIFI_DMA_M2M_Unregister();
    if(res == ((res_r <<32)|res_l))
    {
        rk_print_string("\r\n mac test over\n");
        return  RK_SUCCESS;
    }
    else
    {
        rk_print_string("\r\n mac test error\n ");
        return RK_ERROR  ;
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

