/*
********************************************************************************
*                Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* File Name£º  Adc.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2015-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _ADC_H_
#define _ADC_H_

#undef  EXT
#ifdef  _IN_ADC_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                           Macro define
*
*-------------------------------------------------------------------------------
*/
//define adc sample channel
#define ADC_MIC                         0       //ADC channel 0
#define ADC_CHANEL_FM0                  0       //ADC channel 0
#define ADC_CHANEL_FM1                  1       //ADC channel 1
#define ADC_CHANEL_KEY                  1       //ADC channel 1
#define ADC_CHANEL_BATTERY              0       //ADC channel 0

#define ADC_CH0_FINISH                  ((uint32)1 << 0)
#define ADC_CH1_FINISH                  ((uint32)1 << 1)
#define ADC_CH2_FINISH                  ((uint32)1 << 2)
#define ADC_CH3_FINISH                  ((uint32)1 << 3)
#define ADC_CH4_FINISH                  ((uint32)1 << 4)

#define ADC_CH0_PEND                    ((uint32)1 << 8)    //channel suspend flag
#define ADC_CH1_PEND                    ((uint32)1 << 9)    //channel suspend flag
#define ADC_CH2_PEND                    ((uint32)1 << 10)    //channel suspend flag
#define ADC_CH3_PEND                    ((uint32)1 << 11)    //channel suspend flag
#define ADC_CH4_PEND                    ((uint32)1 << 12)    //channel suspend flag

#define ADC_FINISH                      ((uint32)1 << 16)    //sampling finish flag

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum _ADC_CHN
{
    ADC_CH0 = (uint32)(0),
    ADC_CH1,
    ADC_CH2,
    ADC_CH3,
    ADC_CH4,
    ADC_CH5,
    ADC_CH6,
    ADC_CH7,
    ADC_CHN_MAX

}eADC_CHN;

/*
*-------------------------------------------------------------------------------
*
*                           Variable define
*
*-------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
extern void AdcIntEnable(void);
extern void AdcIntDisable(void);
extern void AdcIntClr(void);
extern void AdcStart(uint8 StartCh);
extern uint32 CheckAdcState(uint8 ch);
extern uint32 GetAdcData(uint8 ch);
void AdcInit(void);
/*
********************************************************************************
*
*                         End of Adc.h
*
********************************************************************************
*/
#endif

