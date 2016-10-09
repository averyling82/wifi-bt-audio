/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\hifi.c
* Owner: WJR
* Date: 2014.11.28
* Time: 15:41:41
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.28     15:41:41   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HIFI_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "DriverInclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
int HIFI_Dma_TO_Finish = 0;
int HIFI_Dma_FROM_Finish = 0;
int HIFI_DMA_TO_CH = 1;
int HIFI_DMA_FROM_CH = 0;
DMA_LLP HIFIllpListn[DMA_CHN_MAX][2];

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
** Name: Hifi_Clear_FUN_DONE_FLAG
** Input:UINT32 HifiId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 20:10:41
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Clear_FUN_DONE_FLAG(UINT32 HifiId)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->ACC_INTCR &= 0xFFFF7FFF;
}
/*******************************************************************************
** Name: Hifi_Enable_FUN_DONE_FLAG
** Input:UINT32 HifiId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 20:07:41
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Enable_FUN_DONE_FLAG(UINT32 HifiId)
{
     HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->ACC_INTCR|= Function_DONE_INT_ENABLE;

}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFITranData(int32 *pSrc,int32 *pDst,int count)
{    int i;
    for(i=0;i<(count);i++)
    {
      pDst[i] = pSrc[i];
    }
}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFITranData_fifo(int32 *pSrc_t,int32 *pDst_t,int count_t,int32 *pSrc_r,int32 *pDst_r,int *count_r)
{    int i;
     int sum_num = count_t;
     while(count_t>0)
    {
      while((Hifi_Get_ACC_Intsr(0,TX_Empty_Interrupt_Active)==TX_Empty_Interrupt_Active))
      {     for(i=0;i<8;i++)
          {
            *pDst_t= *pSrc_t;
            pSrc_t++;
            count_t--;
						if((count_t ==0 ))
						{
							break;
						}
          }

			while((Hifi_Get_ACC_Intsr(0,RX_Full_Interrupt_Active)==RX_Full_Interrupt_Active))
			{
			   for(i=0;i<8;i++)
               {
                *pDst_r = *pSrc_r;
                pDst_r++;
                *count_r =*count_r+1 ;
									if(*count_r ==sum_num)
									{
										break;
									}
			   }
			}
			if((count_t ==0 ))
			{
				break;
			}
      }

    }
		 while((Hifi_Get_ACC_Intsr(0,RX_Full_Interrupt_Active)==RX_Full_Interrupt_Active)||(*count_r !=sum_num) )
		{
		       while(*count_r !=sum_num)
		        {
                    *pDst_r = *pSrc_r;
                    pDst_r++;
                    *count_r =*count_r+1 ;
		        }

				if(*count_r ==sum_num)
				{
					break;
				}
		}

}

_CPU_NANOD_LIB_HIFI_COMMON_
void HIFI_DmaToIsr()
{
     uint32 rawStatus;
    rawStatus = (uint32)(DmaReg2->StatusTfr);
    DmaReg2->ClearTfr = rawStatus;
     HIFI_Dma_TO_Finish = 1;
     DmaDisableInt2(HIFI_DMA_TO_CH);
}
pFunc HIFI_DmaIsrCallBack[DMA_CHN_MAX];

_CPU_NANOD_LIB_HIFI_COMMON_
void HIFI_DmaFromIsr()
{
     DmaGetIntType2();
     HIFI_Dma_FROM_Finish = 1;
     //DmaDisableInt2(HIFI_DMA_FROM_CH);
}


_CPU_NANOD_LIB_HIFI_COMMON_
void HIFI_DmaIsr()
{
    uint32 channel;
    uint32 rawStatus;
    pFunc CallBack;

    do
    {
        rawStatus = (uint32)(DmaReg2->StatusTfr);
        DmaReg2->ClearTfr = rawStatus;

        channel = 0;
        do
        {
            for(; channel < DMA_CHN_MAX; channel++)
            {
                if (((uint32)(rawStatus)) & ((uint32)(0x01) << channel))
                {
                    rawStatus &= ~(0x01u << channel);
                    break;
                }
            }

            CallBack = HIFI_DmaIsrCallBack[channel];
            if (CallBack)
            {
                HIFI_DmaIsrCallBack[channel] = 0;
                CallBack();
            }

        }while (rawStatus & 0x07);

    }while((uint32)(DmaReg->StatusTfr) & 0x07);

}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_M2M_register()
{
        ScuClockGateCtr2(HCLK_HIFI_GATE, 1);     //open uart clk
        ScuClockGateCtr2(HCLK_HIGH_DMA_GATE, 1);     //open uart clk

        ScuSoftResetCtr2(DMA2_SRST, 1); //open rst uart ip
        ScuSoftResetCtr2(HIFI_SRST, 1);
        DelayMs2(1);
        ScuSoftResetCtr2(DMA2_SRST, 0);
        ScuSoftResetCtr2(HIFI_SRST, 0);

        IntRegister2(INT_ID_DMA2 ,HIFI_DmaToIsr);
        IntPendingClear2(INT_ID_DMA2);
        IntEnable2(INT_ID_DMA2);
}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_M2M_Unregister()
{
        IntDisable2(INT_ID_DMA2);
        IntPendingClear2(INT_ID_DMA2);
        IntUnregister2(INT_ID_DMA2 );

        ScuClockGateCtr2(HCLK_HIFI_GATE, 0);
        ScuClockGateCtr2(HCLK_HIGH_DMA_GATE, 0);
}

/*******************************************************************************
** Name: HIFI_DMA_TO_ACC
** Input:int *pSrc,int *pDst,int count
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 17:38:58
*******************************************************************************/

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_MEM_ACC(int *pSrc,int *pDst,int count)
{
    int i ,j,total_size;

    DMA_CFGX DmaCfg = {DMA_CTLL_M2M_WORD, DMA_CFGL_M2M_WORD, DMA_CFGH_M2M_WORD,0};

    {
        {
            HIFI_Dma_TO_Finish= 0;
            DmaEnableInt2(HIFI_DMA_TO_CH);
            DmaConfig2(HIFI_DMA_TO_CH, (uint32)pSrc, (uint32)pDst,count, &DmaCfg, NULL);
            while(!HIFI_Dma_TO_Finish)
			{
				__WFI2();

			};
            DmaDisableInt2(HIFI_DMA_TO_CH);
        }
     }

}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_TO_register()
{

        ScuClockGateCtr2(HCLK_HIFI_GATE, 1);     //open uart clk
        ScuClockGateCtr2(HCLK_HIGH_DMA_GATE, 1);     //open uart clk

        ScuSoftResetCtr2(DMA2_SRST, 1); //open rst uart ip
        ScuSoftResetCtr2(HIFI_SRST, 1);
        DelayMs2(1);
        ScuSoftResetCtr2(DMA2_SRST, 0);
        ScuSoftResetCtr2(HIFI_SRST, 0);

        IntRegister2(INT_ID_DMA2 ,HIFI_DmaFromIsr);
        IntPendingClear2(INT_ID_DMA2);
        IntEnable2(INT_ID_DMA2);
}

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_TO_Unregister()
{
        IntDisable2(INT_ID_DMA2);
        IntPendingClear2(INT_ID_DMA2);
        IntUnregister2(INT_ID_DMA2);

        ScuClockGateCtr2(HCLK_HIFI_GATE, 0);
        ScuClockGateCtr2(HCLK_HIGH_DMA_GATE, 0);
}

/*******************************************************************************
** Name: HIFI_DMA_TO_ACC
** Input:int *pSrc,int *pDst,int count
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 17:38:58
*******************************************************************************/

_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t HIFI_DMA_TO_ACC(int *pSrc_t,int *pDst_t,int count,int *pSrc_r,int *pDst_r)
{
    int i ,j,total_size;

    DMA_CFGX DmaCfg = {DMA_CTLL_HIFI_WR, DMA_CFGL_HIFI_WR, DMA_CFGH_HIFI_WR,0};
    DMA_CFGX DmaCfg2 = {DMA_CTLL_HIFI_RD, DMA_CFGL_HIFI_RD, DMA_CFGH_HIFI_RD,0};

    {
        {
            HIFI_Dma_FROM_Finish= 0;
            HIFI_Dma_TO_Finish = 0;
            DmaEnableInt2(HIFI_DMA_FROM_CH);
            //DmaEnableInt2(HIFI_DMA_TO_CH);
          //  DmaDisableInt2(HIFI_DMA_TO_CH);
            DmaConfig2(HIFI_DMA_TO_CH, (uint32)pSrc_t, (uint32)pDst_t,count, &DmaCfg, HIFIllpListn[HIFI_DMA_TO_CH]);
            DmaConfig2(HIFI_DMA_FROM_CH, (uint32)pSrc_r, (uint32)pDst_r,count, &DmaCfg2, HIFIllpListn[HIFI_DMA_FROM_CH]);

            while(!HIFI_Dma_FROM_Finish)
            {
                __WFI2();
            }

            //DmaDisableInt2(HIFI_DMA_TO_CH);
          //  DmaDisableInt2(HIFI_DMA_FROM_CH);
        }
    }
}


/*******************************************************************************
** Name: Hifi_Set_ACC_XFER_Start
** FUN_CFG;   //Transfer Start/Function Register
** 此时RX_Transfer_Start标志可以让内部加速器开始工作
** 此时TX_Transfer_Start标志可以让配置开始工作
** Input:UINT32 HidiId,int count,int fun_type
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 16:02:18
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Set_ACC_XFER_Start(UINT32 HidiId,int count,int fun_type)
{
    HIFIACC * pHifi = HifiPort(HidiId);
    pHifi->FUN_CFG=fun_type|(count << 15)|TX_Transfer_Start|RX_Transfer_Start;
    // printf("xfer start 0x%x \n",pHifi->FUN_CFG);
}
/*******************************************************************************
** Name: Hifi_Set_ACC_XFER_Disable
** FUN_CFG;   //Transfer Start/Function Register
** 让总线访问RAM，此时可以向RAM 传数据。配置数据和 fft及MAC数据都是直接向ram传数据
** Input:UINT32 HidiId,int count,int fun_type
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 16:02:18
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Set_ACC_XFER_Disable(UINT32 HidiId,int count,int fun_type)
{
    HIFIACC * pHifi = HifiPort(HidiId);
    pHifi->FUN_CFG =fun_type|(count << 15)|TX_Transfer_Stop|RX_Transfer_Stop;
    //printf("\nxfer dis 0x%x \n",pHifi->FUN_CFG);

}
/*******************************************************************************
** Name: Hifi_Get_ACC_Intsr
** ACC_INTSR; //interrupt status register (read only)
** Input:UINT32 HifiId
** Return: RK_ERR_T
** Owner:WJR
** Date: 2014.12.3
** Time: 15:56:47
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API RK_ERR_T Hifi_Get_ACC_Intsr(UINT32 HifiId,int mode)
{
    int res;
    HIFIACC * pHifi = HifiPort(HifiId);
    res = pHifi->ACC_INTSR & mode;
    return res;
}
/*******************************************************************************
** Name: Hifi_Set_ACC_Intcr
** ACC_INTCR; //interrupt control register
** Input:UINT32 HifiId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 15:48:20
*************************************************************000******************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Set_ACC_Intcr(UINT32 HifiId)
{
    HIFIACC * pHifi = HifiPort(HifiId);

    pHifi->ACC_INTCR= Receive_FIFO_Threshold_8\
                     |RX_Overrun_Interrupt_Enable|Transmit_FIFO_Threshold_8\
                     |TX_Underrun_Interrupt_Enable\
                     |TX_Empty_Interrupt_Active\
                     |RX_FULL_Interrupt_Enable;

}
/*******************************************************************************
** Name: Hifi_Set_ACC_Dmacr
** ACC_DMACR; //DMA control register
** Input:UINT32 HifiId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.3
** Time: 15:34:28
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Set_ACC_Dmacr(UINT32 HifiId)
{
    HIFIACC * pHifi = HifiPort(HifiId);

    pHifi->ACC_DMACR= Receive_DMA_Enable|Receive_Data_Level_8\
                     |Transmit_DMA_Enable|Transmit_Data_Level_8;
     //printf("DMACR 0x%x \n",pHifi->ACC_DMACR);

}
/*******************************************************************************
** Name: Hifi_Set_ACC_clear
** Input:UINT32 HifiId,UINT32 mode
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.28
** Time: 15:42:28
*******************************************************************************/
_CPU_NANOD_LIB_HIFI_COMMON_
COMMON API rk_err_t Hifi_Set_ACC_clear(UINT32 HifiId)
{
    HIFIACC * pHifi = HifiPort(HifiId);
    pHifi->ACC_CLR = (TX_LOGIC_CLEAR|RX_LOGIC_CLEAR);
    //printf("CLR 0x%x \n",pHifi->ACC_CLR);
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
** Name: HIFI_SHELL
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.22
** Time: 9:41:20
*******************************************************************************/
#ifdef _HIFI_SHELL_
_CPU_NANOD_LIB_HIFI_SHELL_
SHELL API rk_err_t HIFI_SHELL(void)
{
   hifi_ape_shell();
   hifi_fft_shell();
   hifi_alac_shell();
   hifi_flac_shell();
   hifi_mac_shell();
   rk_printf_no_time("hifi test over\n");
   return  RK_SUCCESS;
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

