/*
********************************************************************************
*                   Copyright (c) 2008,anzhgiuo
*                         All rights reserved.
*
* File Name£º   Pwm.h
*
* Description:
*
* History:      <author>          <time>        <version>
*              anzhiguo          2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _PWM_H_
#define _PWM_H_

#undef  EXT
#ifdef  _IN_PWM_
#define EXT
#else
#define EXT extern
#endif
/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
typedef enum _PWM_CHN
{
    PWM_CHN0 = (uint32)(0),
    PWM_CHN1,
    PWM_CHN2,
    PWM_CHN3,
    PWM_CHN4,
    PWM_CHN_MAX

}ePWM_CHN;

/*every pwm channel has 4 output pwm*/
typedef enum _PWM_CHN_NUM
{
    PWM_CHN_NUM0 = (uint32)(0),
    PWM_CHN_NUM1,
    PWM_CHN_NUM2,
    PWM_CHN_NUM3,

}ePWM_CHN_NUM;


typedef enum _PWM_PRESCALE
{
    PWM_PRE_FACTOR0 = (uint32)(0),
    PWM_PRE_FACTOR1,
    PWM_PRE_FACTOR2,
    PWM_PRE_FACTOR3,
    PWM_PRE_FACTOR4,
    PWM_PRE_FACTOR5,
    PWM_PRE_FACTOR6,
    PWM_PRE_FACTOR7,
    PWM_PRE_FACTOR8,
    PWM_PRE_FACTOR9,
    PWM_PRE_FACTOR10,
    PWM_PRE_FACTOR11,
    PWM_PRE_FACTOR12,
    PWM_PRE_FACTOR13,
    PWM_PRE_FACTOR14,
    PWM_PRE_FACTOR15,

    PWM_PRE_MAX

}ePWM_PRESCALE;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
//#ifdef DRIVERLIB_IRAM

extern void PwmRegReset(ePWM_CHN ch);
extern void PwmPrescalefFctorSet(ePWM_CHN ch, ePWM_PRESCALE mpwmPrescalefFctor);
extern uint32 PwmPrescalefFctorGet(ePWM_CHN ch);

//#else
//
//typedef void   (*pPwmRegReset)(ePWM_CHN ch);
//typedef void   (*pPwmPrescalefFctorSet)(ePWM_CHN ch, ePWM_PRESCALE mpwmPrescalefFctor);
//typedef uint32 (*pPwmPrescalefFctorGet)(ePWM_CHN ch);
//
//#define PwmRegReset(ch)                              (((pPwmRegReset         )(Addr_PwmRegReset         ))(ch))
//#define PwmPrescalefFctorSet(ch, mpwmPrescalefFctor) (((pPwmPrescalefFctorSet)(Addr_PwmPrescalefFctorSet))(ch, mpwmPrescalefFctor))
//#define PwmPrescalefFctorGet(ch)                     (((pPwmPrescalefFctorGet)(Addr_PwmPrescalefFctorGet))(ch))
//
//#endif

extern void PWM_Start(ePWM_CHN ch);
extern void PWM_Stop(ePWM_CHN ch);
extern int32 PwmRateSet(ePWM_CHN ch,UINT32 rate, UINT32 PWM_freq);

extern void PWM_SetIntEnable(ePWM_CHN ch);
extern void PWM_SetIntDisable(ePWM_CHN ch);
extern int PWM_GetIntStatus(ePWM_CHN ch);
extern void PWM_ClrInt(ePWM_CHN ch);


/*
********************************************************************************
*
*                         End of pwm.h
*
********************************************************************************
*/
#endif
