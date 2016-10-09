/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name：   Pwm.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             anzhiguo      2008-11-20         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"

#ifdef __DRIVER_PWM_PWMDEVICE_C__
#include "typedef.h"
#include "DriverInclude.h"

//#define _IN_PWM_

const uint32 PWM_GROUP[2] =
{
    PWM0_BASE,
    PWM1_BASE,
};

#define PWMPORT(n)          ((PWM_REG*)(PWM_GROUP[n]))

/*
--------------------------------------------------------------------------------
  Function name : void PWM_Start(int num)
  Author        : LinChongSen
  Description   : 启动PWM 输出

  Input         : num -- PWM号(0~4)

  Return        : *rate -- 当前PWM的占空比
                  *PWM_freq  -- 当前PWM的频率
--------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
void PWM_Start(ePWM_CHN ch)
{
    if (ch < PWM_CHN4)
    {
        ScuClockGateCtr(PCLK_PWM0_GATE, 1);
    }
    else
    {
        ScuClockGateCtr(PCLK_PWM1_GATE, 1);
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_Stop(int num)
  Author        : LinChongSen
  Description   : 关闭PWM 输出

  Input         : num -- PWM号(0~4)

  Return        :
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
void PWM_Stop(ePWM_CHN ch)
{
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
    {
        pReg = PWMPORT(1);
        ch -= 4;
    }

    uReg = (uint32)&(pReg->PWM0_CTRL);
    uReg = uReg + 16 * ch;

    *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

    if (ch < PWM_CHN4)
    {
        ScuClockGateCtr(PCLK_PWM0_GATE, 1);
    }
    else
    {
        ScuClockGateCtr(PCLK_PWM1_GATE, 1);
    }
}

//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : void PwmRegReset(int num)
  Author        : LinChongSen
  Description   : 复位PWM

  Input         : num -- PWM号(0~4)

  Return        :
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void PwmRegReset(ePWM_CHN ch)
{
    PWM_REG* pReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    memset(pReg,0, sizeof(PWM_REG));
}

/*
--------------------------------------------------------------------------------
  Function name : void PwmPrescalefFctorSet(int num,int mpwmPrescalefFctor)
  Author        : LinChongSen
  Description   : PWM 分频率设置

  Input         : num -- PWM号(0~4)
                  mpwmPrescalefFctor -- 0000: 1/2             0001: 1/4  (0~15)
                                        0000: 1/8             0001: 1/16
                                        0100: 1/32            0101: 1/64
                                        0110: 1/128           0111: 1/256
                                        1000: 1/512           1001: 1/1024
                                        1010: 1/2048          1011: 1/4096
                                        1100: 1/8192          1101: 1/16384
                                        1110: 1/32768         1111: 1/65536

  Return        :
  desc          : 不调用，默认为1/2分频
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
void PwmPrescalefFctorSet(ePWM_CHN ch, ePWM_PRESCALE mpwmPrescalefFctor)
{
    PWM_REG* pReg;
    pReg = PWMPORT(ch);
    pReg->PWM0_CTRL |= (mpwmPrescalefFctor << 9);
}

/*
--------------------------------------------------------------------------------
  Function name : int PwmPrescalefFctorGet(int num)
  Author        : LinChongSen
  Description   : PWM 分频率读取

  Input         : num -- PWM号(0~4)
  Output        : mpwmPrescalefFctor  -- 返回2的次方数 如:2，4，8，16....65536
                                         可以参考PwmPrescalefFctorSet中的mpwmPrescalefFctor注释
  Return        :
--------------------------------------------------------------------------------
*/
//_ATTR_DRIVERLIB_CODE_
uint32 PwmPrescalefFctorGet(ePWM_CHN ch)
{
    PWM_REG* pReg;
    pReg = PWMPORT(ch);
    return (pReg->PWM0_CTRL & 0x00001E00) >> 9;
}


/*
--------------------------------------------------------------------------------
  Function name : BOOL PwmRateSet(int num,UINT32 rate, UINT32 PWM_freq)
  Author        : LinChongSen
  Description   : PWM 占空比设置

  Input         : num -- PWM号(0~4)
                  rate -- 高与低的比值(0~100)
                  PWM_freq  -- 当前PWM的频率(<10k)

  Return        :
  desc          : data1 过小,则无法正常设置
--------------------------------------------------------------------
*/
//_ATTR_DRIVER_CODE_
int32 PwmRateSet(ePWM_CHN ch,UINT32 rate, UINT32 PWM_freq)
{
    UINT32 data0,data1;
    UINT32 pwmclk;
    UINT32 pwmPrescalefFctor;
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
    {
        pReg = PWMPORT(1);
        ch -= 4;
    }


    if(rate > 100)
        return 0;

    if (rate == 100)
    {
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

        return 0;
    }
    else if (rate == 0)
    {
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg &= ~(PWM_ENABLE);

        return 0;
    }

    if((rate > 0) && (rate < 100))
    {
        uReg = (uint32)&(pReg->PWM0_DUTY_LPR);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = rate;

        uReg = (uint32)&(pReg->PWM0_CNT);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = 0;

        uReg = (uint32)&(pReg->PWM0_PERIOD_HPR);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = 100;

        //repeat counter[7:0]<<24 | scale factor[7:0]<<16 |
        //prescale factor[2:0]<<12 | clk_sel<<9 | lp_en<<8 |
        //output mode<<5 | inactive pol<<4 | duty pol<<3 |
        //pwm mode[1:0]<<1 | pwm enable<<0

/*continuous mode*/
        uReg = (uint32)&(pReg->PWM0_CTRL);
        uReg = uReg + 16 * ch;
        *(uint32 volatile *)uReg = PWM_RPT_CNT(0x0) \
                        |PWM_SCALE_FACTOR(2) \
                        |PWM_PRESCALE_FACTOR(1) \
                        |PWM_CLK_SEL(NON_SCALED_CLOCK) \
                        |PWM_LP_DISABLE \
                        |PWM_OUTPUT_MODE(OUTPUT_MODE_LEFT) \
                        |PWM_INACTIVE_POLARITY(POLARITY_NEG) \
                        |PWM_DUTY_POLARITY(POLARITY_NEG) \
                        |PWM_CON_MODE(PWM_MODE_CONTINUOUS) \
                        |PWM_ENABLE;


/*one shot mode*/
//        uReg = (uint32)&(pReg->PWM0_CTRL);
//        uReg = uReg + 16 * ch;
//        *(uint32 volatile *)uReg = PWM_RPT_CNT(10) \
//                                |PWM_SCALE_FACTOR(2) \
//                                |PWM_PRESCALE_FACTOR(1) \
//                                |PWM_CLK_SEL(NON_SCALED_CLOCK) \
//                                |PWM_LP_DISABLE \
//                                |PWM_OUTPUT_MODE(OUTPUT_MODE_LEFT) \
//                                |PWM_INACTIVE_POLARITY(POLARITY_NEG) \
//                                |PWM_DUTY_POLARITY(POLARITY_NEG) \
//                                |PWM_CON_MODE(PWM_MODE_ONESHOT) \
//                                |PWM_ENABLE;


        return 0;
    }

    return -1;
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_SetIntEnable(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM使能中断

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_SetIntEnable(ePWM_CHN ch)
{
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

   uReg = (uint32)pReg;

    *(uint32 volatile *)(uReg + PWM_INT_EN) = 0xf;
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_SetIntDisable(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM中断除能

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_SetIntDisable(ePWM_CHN ch)
{
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    uReg = (uint32)pReg;

    *(uint32 volatile *)(uReg + PWM_INT_EN) = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetIntStatus(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM获取中断状态值

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
int PWM_GetIntStatus(ePWM_CHN ch)
{
    int intStatus;
    PWM_REG* pReg;
    uint32 uReg;

    if( ch < PWM_CHN4)
        pReg = PWMPORT(0);
    else
        pReg = PWMPORT(1);

    uReg = (uint32)pReg;

    intStatus =  *(uint32 volatile *)(uReg + PWM_INTSTS);

    if(intStatus & PWM_CH0INT_STAS == PWM_CH0INT_STAS)
    {
        //clear interrupt bit
        *(uint32 volatile *)(uReg+PWM_INTSTS) = 0x1;
    }
    else if(intStatus & PWM_CH1INT_STAS == PWM_CH1INT_STAS)
    {
        //clear interrupt bit
        *(uint32 volatile *)(uReg+PWM_INTSTS) = 0x2;
    }
    else if(intStatus & PWM_CH2INT_STAS == PWM_CH2INT_STAS)
    {
        //clear interrupt bit
        *(uint32 volatile *)(uReg+PWM_INTSTS) = 0x4;
    }
    else if(intStatus & PWM_CH3INT_STAS == PWM_CH3INT_STAS)
    {
        //clear interrupt bit
        *(uint32 volatile *)(uReg + PWM_INTSTS) = 0x8;
    }

    return intStatus;
}


/*
--------------------------------------------------------------------------------
  Function name : void PWM_GetIntStatus(ePWM_CHN ch)
  Author        : mlc
  Description   : PWM获取中断状态值

  Input         : ch -- PWM号(0~4)

  Return        :
  desc          :
--------------------------------------------------------------------
*/
void PWM_ClrInt(ePWM_CHN ch)
{
    int intStatus;

    PWM_GetIntStatus(ch);
}

#endif
/*
********************************************************************************
*
*                         End of pwm.c
*
********************************************************************************
*/

