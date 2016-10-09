/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name¡êo   Pll.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _CRU_H
#define _CRU_H

#undef EXT
#ifdef _CRU_IN_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                         Macro Define
*
*-------------------------------------------------------------------------------
*/

/*
*-------------------------------------------------------------------------------
*
*                         struct Define
*
*-------------------------------------------------------------------------------
*/

typedef  struct chip_freq_st
{
    uint32  pll;

    uint32  stclk_sys_core;
    uint32  hclk_sys_core;
    uint32  fclk_sys_core;

    uint32  stclk_cal_core;
    uint32  hclk_cal_core;
    uint32  fclk_cal_core;

    uint32  pclk_logic_pre;

    uint32  hclk_logic_pre;

    uint32  pclk_pmu_pre;
    uint32  hclk_pmu_pre;

    uint32  hclk_high_pre;

    uint32  usbphy_clk;

    uint32  i2s_clk_out;
    uint32  i2s_clk[2];
    uint32  ebc_clk;
    uint32  sdmmc_clk[2];
    uint32  acodec_clk;
    uint32  spi_clk[2];
    uint32  sfc_clk;
    uint32  saradc_clk;
    uint32  timer_clk[2];
    uint32  pmu_clk;
    uint32  uart_clk[5];


}chip_freq_t;

EXT chip_freq_t chip_freq;

typedef struct PLL_ARG
{
    uint32  VCO;

    //PLL Div
    uint32  RefDiv;
    uint32  FBDiv;
    uint32  Frac;
    uint32  Dsmpd;
    uint32  PostDiv1;
    uint32  PostDiv2;

    //24MHz div
    uint32  div_con_24m;

    //syscore Div
    uint32  sys_core_div;
    uint32  sys_stclk_div;
    uint32  pclk_logic_div;

    //Cal core Div
    uint32  cal_core_div;
    uint32  cal_stclk_div;

    uint32  FreqPostOut;
}PLL_ARG_t;

PLL_ARG_t;

/*
*-------------------------------------------------------------------------------
*
*                         function declaration
*
*-------------------------------------------------------------------------------
*/
extern void ScuClockGateCtr(eCLOCK_GATE clk_id, BOOL Enable);
extern void ScuSoftResetCtr(eSOFT_RST Module_id, BOOL Reset);
extern uint32 SetSarAdcFreq(UINT32 TargetFreqHz);
extern uint32 GetSarAdcFreq(void);

extern uint32 SetI2SFreq(uint32 I2sId, Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetI2SFreq(uint32 I2sId);

extern uint32 SetEbcFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetEbcFreq(void);

extern uint32 SetSdmmc1Freq(Clock_Source_Sel Clk_Source, UINT32 TargetFreqHz);
extern uint32 GetSdmmc1Freq(void);

extern uint32 SetSdmmc0Freq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetSdmmc0Freq(void);


extern uint32 SetAcodecFreq(void);
extern uint32 GetAcodecFreq(void);

extern uint32 SetSPIFreq(uint32 SpiID, Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetSPIFreq(uint32 SpiID);

extern uint32 SetSFCFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetSFCFreq(void);

extern uint32 SetUartFreq(uint32 UartID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetUartFreq(uint32 UartID);

extern uint32 GetI2CFreq(void);
extern uint32 GetPWMFreq(void);

extern uint32 SetTimerFreq(uint32 TimerID, Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz);
extern uint32 GetTimerFreq(uint32 TimerID);

extern uint32 SetPMUFreq(uint32 nHz);
extern uint32 GetPMUFreq(void);


extern uint32 Acodec_pll_Source_sel(Clock_Source_Sel Clk_Source);
extern uint32 Usb480m_Source_sel(Clock_Source_Sel Clk_Source);

extern uint32 SysM3_core_stick_clk_set(UINT32 mstick);
extern uint32 CalM3_core_stick_clk_set(UINT32 mstick);

extern uint32 Global_software_reset_Set(UINT32 value);
extern uint32 Global_reset_wait_cnt_th(UINT32 value);
extern uint32 OBS_output_Source_sel(OBS_Source_Sel Clk_Source);

/*
********************************************************************************
*
*                         End of Pll.h
*
********************************************************************************
*/
#endif

