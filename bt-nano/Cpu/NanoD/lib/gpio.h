/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name：  Gpio.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             anzhiguo      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _GPIO_H_
#define _GPIO_H_

#undef  EXT
#ifdef  _IN_GPIO_
#define EXT
#else
#define EXT extern
#endif

/*
--------------------------------------------------------------------------------

                ENUMERATIONS AND STRUCTURES

--------------------------------------------------------------------------------
*/
typedef enum eGPIOPinLevel
{
    GPIO_LOW,
    GPIO_HIGH

}eGPIOPinLevel_t;

typedef enum eGPIOPinDirection
{
    GPIO_IN,
    GPIO_OUT

}eGPIOPinDirection_t;

typedef enum eGPIOPinNum
{
    GPIOPortA_Pin0,
    GPIOPortA_Pin1,
    GPIOPortA_Pin2,
    GPIOPortA_Pin3,
    GPIOPortA_Pin4,
    GPIOPortA_Pin5,
    GPIOPortA_Pin6,
    GPIOPortA_Pin7,
    GPIOPortB_Pin0,
    GPIOPortB_Pin1,
    GPIOPortB_Pin2,
    GPIOPortB_Pin3,
    GPIOPortB_Pin4,
    GPIOPortB_Pin5,
    GPIOPortB_Pin6,
    GPIOPortB_Pin7,
    GPIOPortC_Pin0,
    GPIOPortC_Pin1,
    GPIOPortC_Pin2,
    GPIOPortC_Pin3,
    GPIOPortC_Pin4,
    GPIOPortC_Pin5,
    GPIOPortC_Pin6,
    GPIOPortC_Pin7,
    GPIOPortD_Pin0,
    GPIOPortD_Pin1,
    GPIOPortD_Pin2,
    GPIOPortD_Pin3,
    GPIOPortD_Pin4,
    GPIOPortD_Pin5,
    GPIOPortD_Pin6,
    GPIOPortD_Pin7

}eGPIOPinNum_t;

// Constants for gpio interrupt type definition
typedef enum GPIOIntrType
{
    IntrTypeRisingEdge,      // Rising edge triggered
    IntrTypeFallingEdge,      // Falling edge triggered
    IntrTypeHighLevel,         // Level high triggered
    IntrTypeLowLevel         // Level low triggered

} GPIOIntrType_c;

typedef enum eGPIOPinIOMux
{
    Type_Gpio = 0x0ul,
    Type_Mux1 = 0x1ul,
    Type_Mux2 = 0x2ul,
    Type_Mux3 = 0x3ul,

} eGPIOPinIOMux_t;

typedef enum eGPIOPinPull
{
    ENABLE = ((uint32)(0)),
    DISABLE,

} eGPIOPinPull_t;


typedef enum _EGPIO_PORT
{
    GPIO_PORTA,   //0
    GPIO_PORTB,
    GPIO_PORTC,
    GPIO_PORTD,

    GPIO_PORT_MAX
}eGPIO_PORT;


typedef enum _EGPIO_CHANNEL
{
    GPIO_CH0,   //0
    GPIO_CH1,
    GPIO_CH2,

    GPIO_CH_MAX
}eGPIO_CHANNEL;


/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
extern void   Gpio_SetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinDirection_t direction);
extern UINT8  Gpio_GetPinDirection(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void   Gpio_SetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinLevel_t level);
extern UINT8  Gpio_GetPinLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void   Gpio_SetIntMode(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, GPIOIntrType_c type);
extern UINT32 Gpio_GetIntMode(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void   Gpio_ClearInt_Level(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);   //电平触发中断状态清除
extern void   Gpio_EnableInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void   Gpio_DisableInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void Gpio_MaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void Gpio_UnMaskInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void Gpio_ClearEdgeInt(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern UINT8 Gpio_GetExtPortLevel(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void Gpio_SetLevelSens_Sync(eGPIO_CHANNEL gpioPort,bool bSync );
extern void Gpio_EnableDebounce(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);

extern int32 GpioIsrRegister(eGPIO_CHANNEL gpioChn,uint32 pin, pFunc CallBack);
extern int32 GPIOIsrUnRegister(eGPIO_CHANNEL gpioChn,uint32 pin);
extern void  GpioInt(eGPIO_CHANNEL gpioPort);
extern void GpioInt0(void);
extern void GpioInt1(void);
extern void GpioInt2(void);

extern rk_err_t GPIOShell(void * dev, uint8 * pstr);
/*
********************************************************************************
*
*                         End of Gpio.h
*
********************************************************************************
*/
#endif

