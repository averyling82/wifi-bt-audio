/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name£º   Hw_pwm.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_PWM_H_
#define _HW_PWM_H_

#undef  EXT
#ifdef  _IN_PWM_
#define EXT
#else
#define EXT extern
#endif


/*
--------------------------------------------------------------------------------

            external memory control memap register define

--------------------------------------------------------------------------------
*/
typedef volatile struct {
    //pwm0
    UINT32 PWM0_CNT;         //Main counter register
    UINT32 PWM0_PERIOD_HPR; //PWM HIGH Reference/Capture register
    UINT32 PWM0_DUTY_LPR;   //PWM LOW Reference/Capture register
    UINT32 PWM0_CTRL;       //Current value register
    //pwm1
    UINT32 PWM1_CNT;         //Main counter register
    UINT32 PWM1_PERIOD_HPR; //PWM HIGH Reference/Capture register
    UINT32 PWM1_DUTY_LPR;   //PWM LOW Reference/Capture register
    UINT32 PWM1_CTRL;       //Current value register
    //pwm2
    UINT32 PWM2_CNT;         //Main counter register
    UINT32 PWM2_PERIOD_HPR; //PWM HIGH Reference/Capture register
    UINT32 PWM2_DUTY_LPR;   //PWM LOW Reference/Capture register
    UINT32 PWM2_CTRL;       //Current value register
    //pwm3
    UINT32 PWM3_CNT;         //Main counter register
    UINT32 PWM3_PERIOD_HPR; //PWM HIGH Reference/Capture register
    UINT32 PWM3_DUTY_LPR;   //PWM LOW Reference/Capture register
    UINT32 PWM3_CTRL;       //Current value register

} PWM_REG,*gPWM_REG;

//typedef volatile struct
//{
//    PWM_t CHANNEL[5];
//
//}PWM_REGs,*gPWM_REGs;
//
//#define Pwm     ((PWM_REGs *) PWM0_BASE)

/*
*-------------------------------------------------------------------------------
*
*  The following define the bit fields in the PWMControlReg .
*
*-------------------------------------------------------------------------------
*/
#define PWM_ENABLE              ((UINT32)1<<0)  //pwm channel Ê¹ÄÜ 0:Disable 1:Enable
#define PWM_CON_MODE(mod)	    ((mod) << 1)    //PWM Operation Mode
#define PWM_MODE_ONESHOT		0x00
#define PWM_MODE_CONTINUOUS		0x01
#define PWM_MODE_CAPTURE		0x02
#define PWM_MODE_RESERVE        0x03

#define POLARITY_NEG              0  //negative
#define POLARITY_POS              1  //positive
#define OUTPUT_MODE_LEFT          0  //left aligned mode
#define OUTPUT_MODE_CENTER        1  //center aligned mode
#define PWM_DUTY_POLARITY(pol)      ((UINT32)(pol)<<3)  //Duty Cycle Output Polarity 0: negative 1: positive
#define PWM_INACTIVE_POLARITY(pol)  ((UINT32)(pol)<<4)  //Inactive State Output Polarity  0: negative 1: positive
#define PWM_OUTPUT_MODE(mod)        ((UINT32)(mod)<<5)  //PWM Output mode 0: left aligned mode 1: center aligned mode
#define PWM_LP_ENABLE               ((UINT32)1<<8)  //Low Power Mode Enable 0: disabled 1: enabled
#define PWM_LP_DISABLE              ((UINT32)0<<8)  //Low Power Mode Enable 0: disabled 1: enabled

//Clock Source Select
//0: non-scaled clock is selected as PWM clock source. It means that the prescale clock is directly used as the PWM clock source
//1: scaled clock is selected as PWM clock source
#define NON_SCALED_CLOCK          0
#define SCALED_CLOCK              1
#define PWM_CLK_SEL(mod)             ((UINT32)(mod)<<9)

//Prescale Factor .This field defines the prescale factor applied to input clock.
//The value N means that the input clock is divided by 2^N.
#define PWM_PRESCALE_FACTOR(mod)     ((UINT32)(mod)<<12)


//This fields defines the scale factor applied to prescaled clock.
//The value N means the clock is divided by 2*N.
//If N is 0, it means that the clock is divided by 512(2*256).
#define PWM_SCALE_FACTOR(mod)     ((UINT32)(mod)<<16)

//Repeat Counter
//This field defines the repeated effective periods of output waveform in one-shot
//mode. The value N means N+1 repeated effective periods.
#define PWM_RPT_CNT(cnt)          ((UINT32)(cnt)<<24)




#define      PWM_INTSTS          0x0040  //Interrupt Status Registe
#define      PWM_INT_EN          0x0044  //Interrupt Enable Register

//pwm interrupt status register
#define      PWM_CH0INT_STAS          ((UINT32)1<<0) // 1: Channel 0 Interrupt generated 0 : NOT
#define      PWM_CH1INT_STAS          ((UINT32)1<<1) // 1: Channel 1 Interrupt generated 0 : NOT
#define      PWM_CH2INT_STAS          ((UINT32)1<<2) // 1: Channel 2 Interrupt generated 0 : NOT
#define      PWM_CH3INT_STAS          ((UINT32)1<<3) // 1: Channel 3 Interrupt generated 0 : NOT

/*  This bit is used in capture mode in order to identify the transition of
    the input waveform when interrupt is generated.

    When bit is 1, please refer to PWMx_PERIOD_HPR to know the
    effective high cycle of Channel x input waveform.

    Otherwise, please refer to PWMx_PERIOD_HPR to know the
    effective low cycle of Channel x input waveform.
    Write 1 to CH0_IntSts will clear this bit.
*/
#define      PWM_CH0_POL              ((UINT32)1<<8) //Channel 0 Interrupt Polarity Flag
#define      PWM_CH1_POL              ((UINT32)1<<9) //Channel 1 Interrupt Polarity Flag
#define      PWM_CH2_POL              ((UINT32)1<<10) //Channel 2 Interrupt Polarity Flag
#define      PWM_CH3_POL              ((UINT32)1<<11) //Channel 3 Interrupt Polarity Flag


//pwm interrupt enable register
#define     PWM_CHN0_INT_EN           ((UINT32)1<<0) //0: Channel 0 Interrupt disabled 1: Channel 0 Interrupt enabled
#define     PWM_CHN1_INT_EN           ((UINT32)1<<1) //0: Channel 1 Interrupt disabled 1: Channel 1 Interrupt enabled
#define     PWM_CHN2_INT_EN           ((UINT32)1<<2) //0: Channel 2 Interrupt disabled 1: Channel 2 Interrupt enabled
#define     PWM_CHN3_INT_EN           ((UINT32)1<<3) //0: Channel 3 Interrupt disabled 1: Channel 3 Interrupt enabled

/*
********************************************************************************
*
*                         End of hw_pwm.h
*
********************************************************************************
*/
#endif

