/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name£º  Hw_Adc.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo      2009-3-23          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_ADC_H_
#define _HW_ADC_H_


/*
--------------------------------------------------------------------------------

      external memory control memap register define

--------------------------------------------------------------------------------
*/
typedef volatile struct {

    UINT32 ADC_DATA; //transform register.
    UINT32 ADC_STAS; //status register
    UINT32 ADC_CTRL; //control register.

} ADC_t,*gADC_t;

#define Adc                         ((ADC_t *) ADC_BASE)

#define ADC_CH_MASK                 ((UINT32)7 << 0)
#define ADC_POWERUP                 ((UINT32)1<<3)      //ADC reset
#define ADC_START                   ((UINT32)1<<4)      //ADC start transform
#define ADC_INT_ENBALE              ((UINT32)1<<5)      //ADC interrupt enable
#define ADC_INT_CLEAR               ((UINT32)1<<6)      //ADC clear interrupt flag

/*
********************************************************************************
*
*                         End of Hw_Adc.h
*
********************************************************************************
*/
#endif


