/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   scu.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _CRU_IN_

#include "BspConfig.h"
#include <typedef.h>
#include <DriverInclude.h>

#define POSTDIV1(n)         (((APLL_POSTDIV1_MASK << APLL_POSTDIV1_SHIFT)<<16) | ((n)<<APLL_POSTDIV1_SHIFT))
#define FBDIV(n)            (((APLL_FBDIV_MASK << APLL_FBDIV_SHIFT)<<16) | ((n)<<APLL_FBDIV_SHIFT))
#define DSMPD(n)            (((APLL_DSMPD_MASK<<APLL_DSMPD_SHIFT)<<16) | ((n)<<APLL_DSMPD_SHIFT))
#define POSTDIV2(n)         (((APLL_POSTDIV2_MASK<<APLL_POSTDIV2_SHIFT)<<16) | ((n)<<APLL_POSTDIV2_SHIFT))
#define REFDIV(n)           (((APLL_REFDIV_MASK<<APLL_REFDIV_SHIFT)<<16) | ((n)<<APLL_REFDIV_SHIFT))
#define LOCK(n)             (((n)>>APLL_LOCK_SHIFT)&0x1)

/*
DSMPD = 1:
FOUTVCO = FREF / REFDIV * FBDIV
FOUTVCO / FREF = FBDIV / REFDIV
FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2

DSMPD = 1:
FOUTVCO = FREF / FEFDIV(1~63) * (FBDIV+FRAC/16777216)
FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2

FREF/REFDIV: 1~40MHz
FOUTVCO: 400MHz ~ 1.6GHz
FOUTPOSTDIV: 8MHz~1.6GHz

FBDIV(16~1600):Integer mode
FBDIV(19~160):Fraction Mode
*/
uint32 Get_PLL_Post_Div(uint32 nHz, PLL_ARG_t *pPLL)
{
    uint32 i;
    uint32 PostDiv1 = 1;
    uint32 PostDiv2 = 1;
    uint32 FreqVCO;

    for (PostDiv1 = 1; PostDiv1 <= 7; PostDiv1++)
    {
        for (PostDiv2 = 1; PostDiv2 <= 7; PostDiv2++)
        {
            FreqVCO = nHz * PostDiv1 * PostDiv2;
            if ((400000000 <= FreqVCO) && (FreqVCO <= 1600000000))
            {
                pPLL->PostDiv1 = PostDiv1;
                pPLL->PostDiv2 = PostDiv2;
                pPLL->VCO      = FreqVCO;
                return OK;
            }
        }
    }
    DEBUG("ERROR");
    return ERROR;
}

//FOUTVCO = FREF / REFDIV * (FBDIV+FRAC/16777216)
//FOUTVCO / FREF = FBDIV / REFDIV +  (FRAC/16777216) / REFDIV
uint32 Get_PLL_Pre_Div(uint32 VCO, PLL_ARG_t *pPLL)
{
    uint32 i;
    uint32 RevDivBuffer[5] = {6,4,3,2,1};
    uint32 mode   = 0;
    uint32 Div = 0;
    uint32 FBDiv  = 0;
    uint32 RefDiv = 0;
    uint32 Frac = 0;
    uint64 temp;

    for (i = 0; i < 5; i++)
    {
        RefDiv = RevDivBuffer[i];

        FBDiv = VCO / (24000000 / RefDiv);
        mode  = VCO % (24000000 / RefDiv);

        if ((20 <= FBDiv) && (FBDiv <= 1600))
        {
            pPLL->FBDiv   = FBDiv;
            pPLL->RefDiv  = RefDiv;
            break;
        }
    }
    if (i >= 5)
    {
        DEBUG("ERROR");
        return ERROR;
    }

    pPLL->Frac  = 0;
    pPLL->Dsmpd = 1;
    if (mode != 0)
    {
        //Frac = mode / (24000000 / RefDiv) * 16777216;
        temp = (uint64)mode;
        temp = temp << 24;
        Frac = (uint32)(temp / (24000000 / RefDiv));
        pPLL->Frac = Frac;
        pPLL->Dsmpd = 0;
    }

    return OK;
}

/*
--------------------------------------------------------------------------------
  Function name : Pll_SetFreq()
  Author        : anzhiguo
  Description   :

  Input         :  nKHz: pll out clock (arm clock, ahb clock)

  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:         ORG
  note:    CLK_OUT=24*M/(N*OD*2);M>=2;N>=2,OD>=1; 100MHz<+CLK_OUTxOD<=300MHz

--------------------------------------------------------------------------------
*/
uint32 SetPllFreq(uint32 nHz, PLL_ARG_t *pPllArg)
{
    uint32 delay;

    uint32  Refdiv         = pPllArg->RefDiv;
    uint32  Fbdiv          = pPllArg->FBDiv;
    uint32  Frac           = pPllArg->Frac;
    uint32  Dsmpd          = pPllArg->Dsmpd;
    uint32  Postdiv1       = pPllArg->PostDiv1;
    uint32  Postdiv2       = pPllArg->PostDiv2;
    uint32  sys_core_div   = pPllArg->sys_core_div;
    uint32  sys_stclk_div  = pPllArg->sys_stclk_div;
    uint32  pclk_logic_div = pPllArg->pclk_logic_div;
    uint32  cal_core_div   = pPllArg->cal_core_div;
    uint32  cal_stclk_div  = pPllArg->cal_stclk_div;
    uint32  div_con_24m    = pPllArg->div_con_24m;

    if (nHz<= 24000000)
    {
        CRU->CRU_MODE_CON = ((DIV_CON_24M_MASK << 16) | 0) << DIV_CON_24M_SHIFT;
        CRU->CRU_MODE_CON = ((APLL_WORK_MODE_MASK << 16) | APLL_WORK_MODE_SLOW) << APLL_WORK_MODE_SHIFT;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_FFFF << 16) | ((cal_core_div - 1) << 12) | ((cal_stclk_div - 1) << 8) | ((sys_stclk_div - 1) << 4) | ((sys_core_div - 1) << 0));       //sys_core & cal_core div
        CRU->CRU_CLKSEL_CON[1] = ((CLK_SEL_MASK_3 << 16) | (pclk_logic_div)) << 8;
        CRU->CRU_MODE_CON = (DIV_CON_24M_MASK << 16 | (div_con_24m - 1)) << DIV_CON_24M_SHIFT;

        //foutvco power up and DA power up
        CRU->CRU_APLL_CON2 |= APLL_FOUTVCO_PD | APLL_DAC_PD | APLL_FOUTVCO_PD | APLL_FOUT4PHASE_PD;
        //pll powr_up
        CRU->CRU_APLL_CON1 = ((APLL_SOFTWARE_PD_MASK << 16) | APLL_SOFTWARE_PD) << APLL_SOFTWARE_PD_SHIFT;

        Grf_Memory_Resp_Enable(PMU_RAM | LOW_IRAM | LOW_DRAM, 0);
        Grf_Memory_Resp_Enable(HIGH_IRAM | HIGH_DRAM, 0);
    }
    else
    {
        //switch 24MHz, clear Div

         //switch 24MHz, clear Div
        {
            if (CRU->CRU_MODE_CON & 0x01)
            {
                //Slow mode, clock from external 24MHz
                CRU->CRU_MODE_CON = ((APLL_WORK_MODE_MASK << 16) | APLL_WORK_MODE_SLOW) << APLL_WORK_MODE_SHIFT;

                //Clear Div
                CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_FFFF << 16) | 0x0000);       //sys_core & cal_core div
                CRU->CRU_CLKSEL_CON[1] = ((CLK_SEL_MASK_3 << 16) | 0) << 8;
            }

        }

        //configer PLL , wait stable
        {
            //pll software reset
            CRU->CRU_APLL_CON1 = ((APLL_RESET_MASK << 16) | APLL_SOFTWARE_RESET) << APLL_RESET_SHIFT;

            //pll powerdown
            CRU->CRU_APLL_CON1 = ((APLL_SOFTWARE_PD_MASK << 16) | APLL_SOFTWARE_PD) << APLL_SOFTWARE_PD_SHIFT;

            //foutvco power up and DA power up
            CRU->CRU_APLL_CON2 = 0x08 << 24;


            CRU->CRU_APLL_CON0 = POSTDIV1(Postdiv1)|FBDIV(Fbdiv)
                    |(((APLL_BYPASS_MASK << 16) | ~(APLL_BYPASS)) << APLL_BYPASS_SHIFT);//no pll bypass
            CRU->CRU_APLL_CON1 = DSMPD(Dsmpd)|POSTDIV2(Postdiv2)|REFDIV(Refdiv);
            CRU->CRU_APLL_CON2 = (CRU->CRU_APLL_CON2  &  (0x000f << 24)) | Frac;

             //pll powerdown
            CRU->CRU_APLL_CON1 = ((APLL_SOFTWARE_PD_MASK << 16) | APLL_SOFTWARE_PU) << APLL_SOFTWARE_PD_SHIFT;

            IntMasterDisable();
            delay = 240000;
            while(delay > 0)
            {
                delay--;
                Delay10cyc(1);
                if (LOCK(CRU->CRU_APLL_CON1))
                    break;
            }
            IntMasterEnable();
        }

        //set Div, switch to PLL
        {
            CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_FFFF << 16) | ((cal_core_div - 1) << 12) | ((cal_stclk_div - 1) << 8) | ((sys_stclk_div - 1) << 4) | ((sys_core_div - 1) << 0));       //sys_core & cal_core div
            Delay10cyc(1);
            CRU->CRU_CLKSEL_CON[1] = ((CLK_SEL_MASK_3 << 16) | (pclk_logic_div)) << 8;
            Delay10cyc(1);
            CRU->CRU_MODE_CON = ((APLL_WORK_MODE_MASK << 16) | APLL_WORK_MODE_NORMAL) << APLL_WORK_MODE_SHIFT;
        }

        {// here is hook for freq change start

            if ((chip_freq.hclk_sys_core < 100000000) && (chip_freq.hclk_cal_core < 200000000))
            {
                VDD_VoltageAdjust_Set(SCU_DCOUT_100);
            }
            else
            {
                VDD_VoltageAdjust_Set(SCU_DCOUT_120);
            }

            if (chip_freq.hclk_sys_core >= 180000000)
            {
                Grf_Memory_Resp_Enable(PMU_RAM | LOW_IRAM | LOW_DRAM, 1);
            }
            else
            {
                Grf_Memory_Resp_Enable(PMU_RAM | LOW_IRAM | LOW_DRAM, 0);
            }

            if (chip_freq.hclk_cal_core >= 300000000)
            {
                Grf_Memory_Resp_Enable(HIGH_IRAM | HIGH_DRAM, 1);
            }
            else
            {
                Grf_Memory_Resp_Enable(HIGH_IRAM | HIGH_DRAM, 0);
            }
        }// here is hook for freq change end!!!
    }
}


/*
--------------------------------------------------------------------------------
  Function name : SetHclkCalCoreFreq()
  Author        :
  Description   :

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.hclk_cal_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetHclkCalCoreFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | CAL_CORE_SRC_SEL_PLL) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 12;
        chip_freq.hclk_cal_core = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == USB480M)
    {
        Source_clk = 480*1000*1000;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | CAL_CORE_SRC_SEL_480M) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 12;
        chip_freq.hclk_cal_core = Source_clk/(div_num + 1);
    }


    return(chip_freq.hclk_cal_core);
}
/*
--------------------------------------------------------------------------------
  Function name : SetStclkCalCoreFreq()
  Author        :
  Description   :

  Input         : UINT32 TargetFreqHz

  Return        : chip_freq.stclk_cal_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetStclkCalCoreFreq(UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    Source_clk = chip_freq.hclk_cal_core;

    div_num = Source_clk/TargetFreqHz - 1;

    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 8;

    chip_freq.stclk_cal_core = Source_clk/(div_num + 1);

    return(chip_freq.stclk_cal_core);
}
/*
--------------------------------------------------------------------------------
  Function name : SetHclkSysCoreFreq()
  Author        :
  Description   :

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.hclk_sys_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetHclkSysCoreFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | SYS_CORE_SRC_SEL_PLL) << 3;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 0;
        chip_freq.hclk_sys_core = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == USB480M)
    {
        Source_clk = 480*1000*1000;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_1 << 16) | SYS_CORE_SRC_SEL_480M) << 3;
        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 0;
        chip_freq.hclk_sys_core = Source_clk/(div_num + 1);
    }

    return(chip_freq.hclk_sys_core);
}
/*
--------------------------------------------------------------------------------
  Function name : SetStclkSysCoreFreq()
  Author        :
  Description   :

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.stclk_sys_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetStclkSysCoreFreq(UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    Source_clk = chip_freq.hclk_sys_core;

    div_num = Source_clk/TargetFreqHz - 1;

    CRU->CRU_CLKSEL_CON[0] = ((CLK_SEL_MASK_7 << 16) | div_num) << 4;

    chip_freq.stclk_sys_core = Source_clk/(div_num + 1);

    return(chip_freq.stclk_sys_core);
}
/*
--------------------------------------------------------------------------------
  Function name : SetPclkLogicPreFreq()
  Author        :
  Description   :

  Input         : div_num : 1;2;4;8

  Return        : chip_freq.pclk_logic_pre

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetPclkLogicPreFreq(UINT32 TargetFreqHz)
{
    UINT32 Source_clk,div_num;

    Source_clk =  chip_freq.hclk_sys_core;
    div_num = Source_clk/TargetFreqHz - 1;

    CRU->CRU_CLKSEL_CON[1] = ((CLK_SEL_MASK_3 << 16) | div_num) << 8;

    chip_freq.pclk_logic_pre = Source_clk/(div_num + 1);

    return(chip_freq.pclk_logic_pre);
}
/*
--------------------------------------------------------------------------------
  Function name : SetHclkLogicPreFreq()
  Author        :
  Description   : void

  Input         :

  Return        : chip_freq.hclk_logic_pre

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetHclkLogicPreFreq(void)
{
    chip_freq.hclk_logic_pre = chip_freq.hclk_sys_core;
    return(chip_freq.hclk_logic_pre);
}
/*
--------------------------------------------------------------------------------
  Function name : SetPclkPmuPreFreq()
  Author        :
  Description   : void

  Input         :

  Return        : chip_freq.pclk_pmu_pre

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetPclkPmuPreFreq(void)
{
    chip_freq.pclk_pmu_pre = chip_freq.hclk_sys_core;

    return(chip_freq.pclk_pmu_pre);
}
/*
--------------------------------------------------------------------------------
  Function name : SetHclkPmuPreFreq()
  Author        :
  Description   : void

  Input         :

  Return        : chip_freq.hclk_pmu_pre

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetHclkPmuPreFreq(void)
{
    chip_freq.hclk_pmu_pre = chip_freq.hclk_sys_core;

    return(chip_freq.hclk_pmu_pre);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetClkFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:        get apb clock
--------------------------------------------------------------------------------
*/
uint32 GetHclkCalCoreFreq(void)
{
    return(chip_freq.hclk_cal_core);
}

uint32 GetStclkCalCoreFreq(void)
{
    return(chip_freq.stclk_cal_core);
}

uint32 GetHclkSysCoreFreq(void)
{
    return(chip_freq.hclk_sys_core);
}

uint32 GetStclkSysCoreFreq(void)
{
    return(chip_freq.stclk_sys_core);
}

uint32 GetPclkLogicPreFreq(void)
{
    return(chip_freq.pclk_logic_pre);
}

uint32 GetHclkLogicPreFreq(void)
{
    return(chip_freq.hclk_logic_pre);
}

uint32 GetPclkPmuPreFreq(void)
{
    return(chip_freq.pclk_pmu_pre);
}

uint32 GetHclkPmuPreFreq(void)
{
    return(chip_freq.hclk_pmu_pre);
}

uint32 GetHclkHighPreFreq(void)
{
    return(chip_freq.hclk_high_pre);
}

/*
--------------------------------------------------------------------------------
  Function name : SetTimerFreq()
  Author        :
  Description   : set apb clock para

  Input         : TimerID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.timer_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetTimerFreq(UINT32 TimerID, Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(TimerID == TIMER0)
    {
        if(Clk_Source == PCLK_LOGIC)
        {
            Source_clk = chip_freq.pclk_logic_pre;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1 << 16) | TIMER0_PLL_SEL_PCLK) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1F << 16) | div_num) << 0;
            chip_freq.timer_clk[TIMER0] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1 << 16) | TIMER0_PLL_SEL_24M) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1F << 16) | div_num) << 0;
            chip_freq.timer_clk[TIMER0] = Source_clk/(div_num + 1);
        }

        return(chip_freq.timer_clk[TIMER0]);
    }
    else if(TimerID == TIMER1)
    {
        if(Clk_Source == PCLK_LOGIC)
        {
            Source_clk = chip_freq.pclk_logic_pre;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1 << 16) | TIMER1_PLL_SEL_PCLK) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
            chip_freq.timer_clk[TIMER1] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1 << 16) | TIMER1_PLL_SEL_24M) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[2] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
            chip_freq.timer_clk[TIMER1] = Source_clk/(div_num + 1);
        }

        return(chip_freq.timer_clk[TIMER1]);
    }

}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetAdcClkFreq(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetTimerFreq(UINT32 TimerID)
{
    if(TimerID == TIMER0)
    {
        return(chip_freq.timer_clk[TIMER0]);
    }
    else if(TimerID == TIMER1)
    {
        return(chip_freq.timer_clk[TIMER1]);
    }
}
/*
--------------------------------------------------------------------------------
  Function name : SetI2SClkOutFreq()
  Author        :
  Description   :

  Input         : I2S_output_clk_Sel Clk_Source

  Return        : chip_freq.i2s_clk_out

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetI2SClkOutFreq(I2S_output_clk_Sel Clk_Source)
{
    if(Clk_Source == XIN12M)
    {
        CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_1 << 16) | I2S1_OUT_SEL_CLK_12M) << 12;
        chip_freq.i2s_clk_out = 12 * 1000 * 1000;
    }
    else if(Clk_Source == I2S1_CLK)
    {
        CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_1 << 16) | I2S1_OUT_SEL_CLK_I2S1) << 12;
        chip_freq.i2s_clk_out = chip_freq.i2s_clk[I2S_DEV1];
    }

    return(chip_freq.i2s_clk_out);
}

/*
--------------------------------------------------------------------------------
  Function name : GetI2SClkOutFreq()
  Author        :
  Description   : get apb clock para

  Input         : void

  Return        : chip_freq.i2s_clk_out

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
UINT32 GetI2SClkOutFreq(void)
{
    return(chip_freq.i2s_clk_out);
}


/*
--------------------------------------------------------------------------------
  Function name : SetI2SFreq()
  Author        :
  Description   : set apb clock para

  Input         : UINT32 I2sId,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.i2s_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetI2SFreq(UINT32 I2sId,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
   UINT32 RealFreq, Source_clk,div_num,ret;

    if(I2sId == I2S_DEV1)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_1 << 16) | I2S1_PLL_SEL_SOC) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_7F << 16) | div_num) << 0;
            chip_freq.i2s_clk[I2S_DEV1] = Source_clk/(div_num + 1);

            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_3 << 16) | I2S1_CLK_SEL_PLL_CLOCK) << 8;

        }
        else if(Clk_Source == CLK_ACODEC_PLL)
        {
            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_1 << 16) | I2S1_PLL_SEL_ACODEC) << 15;

            //TargetFreqHz = TargetFreqHz/20;

            //div_num = Source_clk/TargetFreqHz - 1;
            //5分频，固定值，因为考虑到ACODEC PLL使用与I2S0与I2S1，所以I2S1外置的MCLK CLOCK分频保持跟ACODEC内部一致5分频
            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_7F << 16) | 19) << 0;

            //CRU->CRU_CLK_FRACDIV_CON1 = 0x0bb8ea60; // 3000/60000 = 1/20 20 DIV

            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_3 << 16) | I2S1_CLK_SEL_PLL_CLOCK) << 8;


        }
        else if(Clk_Source == I2S_XIN12M)
        {
            Source_clk = 12*1000*1000;
            CRU->CRU_CLKSEL_CON[3] = ((CLK_SEL_MASK_3 << 16) | I2S1_CLK_SEL_12M) << 8;

            chip_freq.i2s_clk[I2S_DEV1] = Source_clk;
        }

        return(chip_freq.i2s_clk[I2S_DEV1]);
    }
    else if(I2sId == I2S_DEV0)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_1 << 16) | I2S0_PLL_SEL_SOC) << 15;

            //div_num = Source_clk/TargetFreqHz - 1;
             CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_7F << 16) | 0) << 0;

            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_3 << 16) | I2S0_CLK_SEL_PLL_CLOCK) << 8;

            chip_freq.i2s_clk[I2S_DEV0] = Source_clk/(div_num + 1);
        }
        else if(Clk_Source == CLK_ACODEC_PLL)
        {
            Source_clk = chip_freq.acodec_clk;
            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_1 << 16) | I2S0_PLL_SEL_ACODEC) << 15;

            //TargetFreqHz = TargetFreqHz/20;

            //div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_7F << 16) | 0) << 0;

            //CRU->CRU_CLK_FRACDIV_CON0 = 0x0bb8ea60; // 3000/60000 = 1/20 20 DIV

            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_3 << 16) | I2S0_CLK_SEL_PLL_CLOCK) << 8;

            chip_freq.i2s_clk[I2S_DEV0] = (Source_clk/(div_num + 1))/20;

        }
        else if(Clk_Source == I2S_XIN12M)
        {
            Source_clk = 12*1000*1000;
            CRU->CRU_CLKSEL_CON[4] = ((CLK_SEL_MASK_3 << 16) | I2S0_CLK_SEL_12M) << 8;

            chip_freq.i2s_clk[I2S_DEV0] = Source_clk;
        }

        return(chip_freq.i2s_clk[I2S_DEV0]);
    }

}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetI2SFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetI2SFreq(UINT32 I2sId)
{
    return(chip_freq.i2s_clk[I2sId]);
}


/*
--------------------------------------------------------------------------------
  Function name : SetEbcFreq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.ebc_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetEbcFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[5] = ((CLK_SEL_MASK_1 << 16) | EBC_DCLK_SEL_PLL) << 8;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[5] = ((CLK_SEL_MASK_FF << 16) | div_num) << 0;
        chip_freq.ebc_clk = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == USB480M)
    {
        Source_clk = 480*1000*1000;
        CRU->CRU_CLKSEL_CON[5] = ((CLK_SEL_MASK_1 << 16) | EBC_DCLK_SEL_480M) << 8;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[5] = ((CLK_SEL_MASK_FF << 16) | div_num) << 0;
        chip_freq.ebc_clk = Source_clk/(div_num + 1);
    }

    return(chip_freq.ebc_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetEbcFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetEbcFreq(void)
{
    return(chip_freq.ebc_clk);
}
/*
--------------------------------------------------------------------------------
  Function name : SetSarAdcFreq()
  Author        :
  Description   : set apb clock para

  Input         : UINT32 TargetFreqHz

  Return        : chip_freq.saradc_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetSarAdcFreq(UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    Source_clk = 24 * 1000 * 1000;
    div_num = (Source_clk / TargetFreqHz) - 1;

    CRU->CRU_CLKSEL_CON[6] = ((CLK_SEL_MASK_FF << 16) | div_num) << 0;
    chip_freq.saradc_clk = Source_clk/(div_num + 1);

    return(chip_freq.saradc_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetSarAdcFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetSarAdcFreq(void)
{
    return(chip_freq.saradc_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : SetSdmmc1Freq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.sdmmc1_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetSdmmc1Freq(Clock_Source_Sel Clk_Source, UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1 << 16) | SDMMC_CLK_PLL_SEL_PLL) << 7;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1F << 16) | div_num) << 0;
        chip_freq.sdmmc_clk[1] = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == XIN24M)
    {
        Source_clk = 24*1000*1000;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1 << 16) | SDMMC_CLK_PLL_SEL_24M) << 7;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1F << 16) | div_num) << 0;
        chip_freq.sdmmc_clk[1] = Source_clk/(div_num + 1);
    }

    return(chip_freq.sdmmc_clk[1]);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetSdmmc1Freq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetSdmmc1Freq(void)
{
    return(chip_freq.sdmmc_clk[1]);
}


/*
--------------------------------------------------------------------------------
  Function name : SetHclkCalCoreFreq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.hclk_cal_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetSdmmc0Freq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1 << 16) | EMMC_PLL_SEL_PLL) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
        chip_freq.sdmmc_clk[0] = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == XIN24M)
    {
        Source_clk = 24*1000*1000;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1 << 16) | EMMC_PLL_SEL_24M) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[7] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
        chip_freq.sdmmc_clk[0] = Source_clk/(div_num + 1);
    }

    return(chip_freq.sdmmc_clk[0]);

}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetSdmmc0Freq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetSdmmc0Freq(void)
{
    return(chip_freq.sdmmc_clk[0]);
}

/*
--------------------------------------------------------------------------------
  Function name : SetSPIFreq()
  Author        :
  Description   : set apb clock para

  Input         : SpiID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.spi_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetSPIFreq(uint32 SpiID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(SpiID == SPI_CH0)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_1 << 16) | SPI0_CLK_PLL_SEL_PLL) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.spi_clk[SPI_CH0] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_1 << 16) | SPI0_CLK_PLL_SEL_24M) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.spi_clk[SPI_CH0] = Source_clk/(div_num + 1);
        }

        return(chip_freq.spi_clk[SPI_CH0]);
    }
    else if(SpiID == SPI_CH1)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_1 << 16) | SPI1_CLK_PLL_SEL_PLL) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.spi_clk[SPI_CH1] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_1 << 16) | SPI1_CLK_PLL_SEL_24M) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[8] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.spi_clk[SPI_CH1] = Source_clk/(div_num + 1);
        }

        return(chip_freq.spi_clk[SPI_CH1]);
    }

}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetSPIFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetSPIFreq(uint32 SpiID)
{
    if(SpiID == SPI_CH0)
    {
        return(chip_freq.spi_clk[SPI_CH0]);
    }
    else if(SpiID == SPI_CH1)
    {
        return(chip_freq.spi_clk[SPI_CH1]);
    }
}


/*
--------------------------------------------------------------------------------
  Function name : SetSFCFreq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.sfc_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetSFCFreq(Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(Clk_Source == PLL_MUX_CLK)
    {
        Source_clk = chip_freq.pll;
        CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1 << 16) | SFC_PLL_SEL_PLL) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
        chip_freq.sfc_clk = Source_clk/(div_num + 1);

    }
    else if(Clk_Source == XIN24M)
    {
        Source_clk = 24*1000*1000;
        CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1 << 16) | SFC_PLL_SEL_24M) << 15;

        div_num = Source_clk/TargetFreqHz - 1;
        CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1F << 16) | div_num) << 8;
        chip_freq.sfc_clk = Source_clk/(div_num + 1);
    }

    return(chip_freq.sfc_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetSFCFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetSFCFreq(void)
{
    return(chip_freq.sfc_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : SetUartFreq()
  Author        :
  Description   : set apb clock para

  Input         : UartID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.uart_clk

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetUartFreq(UINT32 UartID,Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz)
{
    UINT32 RealFreq, Source_clk,div_num,ret;

    if(UartID == UART_DEV0)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_1 << 16) | UART0_PLL_SEL_PLL) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV0] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_1 << 16) | UART0_PLL_SEL_24M) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV0] = Source_clk/(div_num + 1);
        }

        return(chip_freq.uart_clk[UART_DEV0]);
    }
    else if(UartID == UART_DEV1)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3 << 16) | UART1_PLL_SEL_PLL) << 6;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV1] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3 << 16) | UART1_PLL_SEL_24M) << 6;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV1] = Source_clk/(div_num + 1);
        }
        else if(Clk_Source == USB480M)
        {
            Source_clk = 480*1000*1000;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3 << 16) | UART1_PLL_SEL_480M) << 6;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[9] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV1] = Source_clk/(div_num + 1);
        }
        return(chip_freq.uart_clk[UART_DEV1]);
    }
    else if(UartID == UART_DEV2)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1 << 16) | UART2_PLL_SEL_PLL) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV2] = Source_clk/(div_num + 1);

        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_1 << 16) | UART2_PLL_SEL_24M) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[10] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV2] = Source_clk/(div_num + 1);
        }
        return(chip_freq.uart_clk[UART_DEV2]);
    }
    else if(UartID == UART_DEV3)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_1 << 16) | UART3_PLL_SEL_PLL) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV3] = Source_clk/(div_num + 1);
        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_1 << 16) | UART3_PLL_SEL_24M) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV3] = Source_clk/(div_num + 1);
        }
        return(chip_freq.uart_clk[UART_DEV3]);
    }
    else if(UartID == UART_DEV4)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_1 << 16) | UART4_PLL_SEL_PLL) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV4] = Source_clk/(div_num + 1);
        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_1 << 16) | UART4_PLL_SEL_24M) << 7;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_3F << 16) | div_num) << 0;
            chip_freq.uart_clk[UART_DEV4] = Source_clk/(div_num + 1);
        }
        return(chip_freq.uart_clk[UART_DEV4]);
    }
    else if(UartID == UART_DEV5)
    {
        if(Clk_Source == PLL_MUX_CLK)
        {
            Source_clk = chip_freq.pll;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_1 << 16) | UART5_PLL_SEL_PLL) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV5] = Source_clk/(div_num + 1);
        }
        else if(Clk_Source == XIN24M)
        {
            Source_clk = 24*1000*1000;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_1 << 16) | UART5_PLL_SEL_24M) << 15;

            div_num = Source_clk/TargetFreqHz - 1;
            CRU->CRU_CLKSEL_CON[12] = ((CLK_SEL_MASK_3F << 16) | div_num) << 8;
            chip_freq.uart_clk[UART_DEV5] = Source_clk/(div_num + 1);
        }
        return(chip_freq.uart_clk[UART_DEV5]);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetUartFreq(void)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetUartFreq(UINT32 UartID)
{
   return(chip_freq.uart_clk[UartID]);
}


uint32 GetI2CFreq()
{
    return(chip_freq.pclk_logic_pre);
}

uint32 GetPWMFreq()
{
    return(chip_freq.pclk_logic_pre);
}


/*
--------------------------------------------------------------------------------
  Function name : SetPMUFreq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.hclk_cal_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetPMUFreq(UINT32 nHz)
{
    UINT32 div_num;

    return(chip_freq.pmu_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetAdcClkFreq(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetPMUFreq(void)
{
    return(chip_freq.pmu_clk);
}


/*
--------------------------------------------------------------------------------
  Function name : ScuClockSet()
  Author        :
  Description   : selet open/close module by its id.

  Input         : clk_id module id
                  Enable :open flag: 1--open this id module.0--close this id module

  Return        : null

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void ScuClockGateCtr(eCLOCK_GATE clk_id, BOOL Enable)
{
    UINT32  group;
    UINT32  num;
    UINT32  Value;

    group  = clk_id / 16 ;
    num    = clk_id % 16;
    //When HIGH, disable clock
    if (Enable)
    {
        #if 1
        if(group == 7)
        {
            Value = CRU->CRU_CLKGATE_CON[group];
            Value &= ~(0x00000001 << (num));
            Value |= 0xffff0000;
            CRU->CRU_CLKGATE_CON[group] = Value;

        }
        else
        #endif
        {
            CRU->CRU_CLKGATE_CON[group] = (uint32)(0x00010000) << (num);
        }
    }
    else
    {
        #if 1
        if(group == 7)
        {
            Value = CRU->CRU_CLKGATE_CON[group];
            Value |= (0x00000001 << (num));
            Value |= 0xffff0000;
            CRU->CRU_CLKGATE_CON[group] = Value;


        }
        else
        #endif
        {
           CRU->CRU_CLKGATE_CON[group] = (uint32)(0x00010001) << (num);

        }
    }
    Delay10cyc(5);
}

/*
--------------------------------------------------------------------------------
  Function name : ScuModuleReset()
  Author        :
  Description   : reset module by id.

  Input         : clk_id module id
                  Reset:   1:  reset
                           0:  release reset
  Return        : null

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void ScuSoftResetCtr(eSOFT_RST Module_id, BOOL Reset)
{
    UINT32  group;
    UINT32  num;

    group  = Module_id / 16 ;
    num    = Module_id % 16;

    if (Reset)
    {
        CRU->CRU_SOFTRST[group] = (uint32)(0x00010001) << (num);
    }
    else
    {
        CRU->CRU_SOFTRST[group] = (uint32)(0x00010000) << (num);
    }
    DelayUs(5);
}
/*
--------------------------------------------------------------------------------
  Function name : SetHclkCalCoreFreq()
  Author        :
  Description   : set apb clock para

  Input         : Clock_Source_Sel Clk_Source,UINT32 TargetFreqHz

  Return        : chip_freq.hclk_cal_core

  desc:         : TargetFreqHz unit Hz
--------------------------------------------------------------------------------
*/
uint32 SetAcodecFreq(void)
{
    chip_freq.acodec_clk = 24 * 1000 * 1000;
    return(chip_freq.acodec_clk);
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 GetAdcClkFreq(void)
  Author        : anzhiguo
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
               anzhiguo     2009-3-24         Ver1.0
  desc:
--------------------------------------------------------------------------------
*/
UINT32 GetAcodecFreq(void)
{
    return(chip_freq.acodec_clk);
}
uint32 Acodec_pll_Source_sel(Clock_Source_Sel Clk_Source)
{
    uint32 ret;

    if(Clk_Source == XIN24M)
    {
        CRU->CRU_MODE_CON = ((ACODEC_PLL_SEL_MASK << 16) | ACODEC_PLL_SEL_24M) << 5;
        ret = XIN24M;
    }
    else if(Clk_Source == CLK_ACODEC_PLL)
    {
        CRU->CRU_MODE_CON = ((ACODEC_PLL_SEL_MASK << 16) | ACODEC_PLL_SEL_PLL) << 5;
        ret = CLK_ACODEC_PLL;
    }
    else
    {
        ret = Clock_Source_ERR;
    }

    return ret;
}

uint32 Usb480m_Source_sel(Clock_Source_Sel Clk_Source)
{
    uint32 ret;

    if(Clk_Source == XIN24M)
    {
        CRU->CRU_MODE_CON = ((USB480M_SEL_MASK << 16) | USB480M_SEL_24M) << 4;
        ret = XIN24M;
    }
    else if(Clk_Source == USBPHY_480_ORI)
    {
        CRU->CRU_MODE_CON = ((USB480M_SEL_MASK << 16) | USB480M_SEL_480M) << 4;
        ret = USBPHY_480_ORI;
    }
    else
    {
        ret = Clock_Source_ERR;
    }

    return ret;

}

uint32 SysM3_core_stick_clk_set(UINT32 mstick)
{
     CRU->CRU_STCLK_CON0 = mstick;
}

uint32 CalM3_core_stick_clk_set(UINT32 mstick)
{
     CRU->CRU_STCLK_CON1 = mstick;
}

uint32 Global_software_reset_Set(UINT32 value)
{
     CRU->CRU_GLB_SRST_FST_VALUE = value;
}

uint32 Global_reset_wait_cnt_th(UINT32 value)
{
     CRU->CRU_GLB_CNT_TH = value;
}
uint32 OBS_output_Source_sel(OBS_Source_Sel Clk_Source)
{
    uint32 ret = 1;
    if(Clk_Source > obs_clk_i2s1)
    {
        ret = 0;
    }
    else
    {
        CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_7 << 16) | Clk_Source) << 5;
        CRU->CRU_CLKSEL_CON[11] = ((CLK_SEL_MASK_1F << 16) | 0) << 0;// div = 0
    }
    return ret;
}
/*
********************************************************************************
*
*                         End of Pll.c
*
********************************************************************************
*/


