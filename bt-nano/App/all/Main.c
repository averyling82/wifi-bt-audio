/*
*********************************************************************************************************
*                                       NANO_OS The Real-Time Kernel
*                                         FUNCTIONS File for V0.X
*
*                                    (c) Copyright 2013, RockChip.Ltd
*                                          All Rights Reserved
*File    : APP.C
* By      : Zhu Zhe
*Version : V0.x
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            DESCRIPTION
*  RK_NANO_OS is a system designed specifically for real-time embedded SOC operating system ,before using
*RK_NANO_OS sure you read the user's manual
*  The TASK NAME TABLE:
*
*
*  The DEVICE NAME TABLE:
*  "UartDevice",              Uart Serial communication devices
*  "ADCDevice",               The analog signal is converted to a digital signal device
*  "KeyDevice",               Key driver device
*
*
*
*
*
*
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/
#include "BspConfig.h"
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "hw_pmu.h"

/*
*********************************************************************************************************
*                                        Macro Define
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        Variable Define
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                              Main(void)
*
* Description:  This Function is the first function.
*
* Argument(s) : none
*
* Return(s)   : int
*
* Note(s)     : none.
*********************************************************************************************************
*/
int Main(void)
{
    UART_DEV_ARG stUartArg;

    bsp_init();

    rkos_init();

    RKDev_Init();

    //ScuClockGateCtr(CLK_TEST_GATE, 1);
    //Grf_GpioMuxSet(GPIO_CH2,GPIOPortA_Pin3,IOMUX_GPIO2A3_CLK_OBS);
    //OBS_output_Source_sel(obs_clk_cal_core);

    rkos_start();

    return 0;
}

__attribute__ ((section ("link_entry")))
int link_main(void)
{
    Main();
}


