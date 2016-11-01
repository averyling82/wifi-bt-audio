/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   Scu.c
*
* Description:  each module open,close and reset and some related usb PHY registers setting.
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "typedef.h"
#include "DriverInclude.h"

#define  _IN_GRF_

//IOMUX portion  implement in Gpio.c

/*
--------------------------------------------------------------------------------
  Function name : Grf_GpioMuxSet()
  Author        : anzhiguo
  Description   : set GPIO work mode.

  Input         : null

  Return        : null

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Grf_GpioMuxSet(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinIOMux_t data)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioPort >= GPIO_CH_MAX)
        return;

    Group = GPIOPinNum / 8;
    pin   = GPIOPinNum % 8;

    if(gpioPort == GPIO_CH0)
    {
        Grf->GPIO_IO0MUX[Group] = ((0x00030000ul + data) << (pin << 0x01ul));
    }
    else if(gpioPort == GPIO_CH1)
    {
        Grf->GPIO_IO1MUX[Group] = ((0x00030000ul + data) << (pin << 0x01ul));
    }
    else if(gpioPort == GPIO_CH2)
    {
        Grf->GPIO_IO2MUX[Group] = ((0x00030000ul + data) << (pin << 0x01ul));
    }
}

/*
--------------------------------------------------------------------------------
  Function name : Grf_GpioMuxGet()
  Author        : anzhiguo
  Description   : get gpio current work mode

  Input         : GPIOPinNum -- GPIO

  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 Grf_GpioMuxGet(eGPIO_CHANNEL gpioPort, eGPIOPinNum_t GPIOPinNum)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;
    int32  val;

    if(gpioPort >= GPIO_CH_MAX)
        return 0;

    Group  = GPIOPinNum / 8;
    pin    = GPIOPinNum % 8;

    if(gpioPort == GPIO_CH0)
    {
        return (((Grf->GPIO_IO0MUX[Group]) >> (pin << 0x01ul)) & 0x03);
    }
    else if(gpioPort == GPIO_CH1)
    {
        return (((Grf->GPIO_IO1MUX[Group]) >> (pin << 0x01ul)) & 0x03);
    }
    else if(gpioPort == GPIO_CH2)
    {
        return (((Grf->GPIO_IO2MUX[Group]) >> (pin << 0x01ul)) & 0x03);
    }
}




void Grf_GPIO_SetPinPull(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinPull_t pull)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioPort >= GPIO_CH_MAX)
        return;

    Group = GPIOPinNum / 8 ;
    pin   = GPIOPinNum % 8;

    if(gpioPort == GPIO_CH0)
    {
        Grf->GPIO_IO0PULL[Group] = ((0x00010000ul + pull) << pin);
    }
    else if(gpioPort == GPIO_CH1)
    {
        Grf->GPIO_IO1PULL[Group] = ((0x00010000ul + pull) << pin);
    }
    else if(gpioPort == GPIO_CH2)
    {
        Grf->GPIO_IO2PULL[Group] = ((0x00010000ul + pull) << pin);
    }

}


uint8 Grf_GPIO_GetPinPull(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;
    int32  val;

    Group = GPIOPinNum / 8 ;
    pin   = GPIOPinNum % 8;
    grf_reg = (uint32)Grf + gpioPort * 0x20 + 0x10 + Group * 4;
    val = *(uint32* volatile)grf_reg;

    return (val & (1 << pin) >> pin);
}

void Grf_GPIO_PortMux_Set(eGPIO_CHANNEL gpioChn, eGPIO_PORT gpioPort, uint16 muxdata)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioChn == GPIO_CH0)
    {
        Grf->GPIO_IO0MUX[gpioPort] = 0xffff0000 | muxdata;
    }
    else if(gpioChn == GPIO_CH1)
    {
        Grf->GPIO_IO1MUX[gpioPort] = 0xffff0000 | muxdata;
    }
    else if(gpioChn == GPIO_CH2)
    {
        Grf->GPIO_IO2MUX[gpioPort] = 0xffff0000 | muxdata;
    }
}

void Grf_GPIO_PortPull_Set(eGPIO_CHANNEL gpioChn, eGPIO_PORT gpioPort, uint8 pull)
{
    UINT8  Group;
    UINT8  pin;
    uint32 grf_reg;

    if(gpioChn == GPIO_CH0)
    {
        Grf->GPIO_IO0PULL[gpioPort] = 0x00ff0000 | pull;
    }
    else if(gpioChn == GPIO_CH1)
    {
        Grf->GPIO_IO1PULL[gpioPort] = 0x00ff0000 | pull;
    }
    else if(gpioChn == GPIO_CH2)
    {
        Grf->GPIO_IO2PULL[gpioPort] = 0x00ff0000 | pull;
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void Grf_PVTM_OSC_Enable(BOOL Enable)
  Author           :
  Description     :
  Input             :
  Return           :

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void Grf_PVTM_OSC_Enable(BOOL Enable)
{
    if(Enable)
        Grf->GRF_PVTM_CON0 = PVTM_FUNC_OSC_EN <<  16 | PVTM_FUNC_OSC_EN;
    else
        Grf->GRF_PVTM_CON0 = PVTM_FUNC_OSC_EN <<  16 | PVTM_FUNC_OSC_DIS ;
}


void Grf_PVTM_Fun_Start(BOOL bStart)
{
    if(bStart)
        Grf->GRF_PVTM_CON0 |= PVTM_FUNC_START << 16 | PVTM_FUNC_START;
    else
        Grf->GRF_PVTM_CON0 |= PVTM_FUNC_START << 16 | (~PVTM_FUNC_START & 0x0001) ;
}


void Grf_PVTM_Set_Divider(int clk_src,int clk)
{
    int div;
    div = clk_src / clk - 1;
    Grf->GRF_PVTM_CON0 |= PVTM_DIV_CON << 16 | div;
}


void Grf_PVTM_Set_Cal_Cnt(int cnt)
{
    Grf->GRF_PVTM_CON1 = PVTM_FUNC_CAL_CNT & cnt;
}

int Grf_PVTM_Get_Cal_Cnt(void)
{
    int cnt;
    cnt = Grf->GRF_PVTM_CON1;
    return cnt;
}

void Grf_PVTM_Set_24M_Divider(int clk)
{
    int div;
    div = 24*1000*1000 / clk - 1;
    Grf->GRF_PVTM_CON2 = CLK_24M_DIV_CON << 16 | div;
}


bool Grf_PVTM_Is_FreqCal_Done(void)
{
    int status;
    status = Grf->GRF_PVTM_STATUS0;

    return (status & PVTM_FUNC_FREQ_DONE) ? TRUE : FALSE;
}

int Grf_PVTM_Get_Freq_Cnt(void)
{
    int cnt;
    cnt = Grf->GRF_PVTM_STATUS1 & PVTM_FUNC_FREQ_CNT;
    return  cnt;
}


void Grf_I2C0_Sel(eI2C_SELECT sel)
{
    if( I2C_SEL_A == sel)
        Grf->GRF_IOMUX_CON |= I2C0_SEL_MASK << 16 | I2C0_SEL_A;
    else if( I2C_SEL_B == sel)
        Grf->GRF_IOMUX_CON |= I2C0_SEL_MASK << 16 | I2C0_SEL_B;
    else if( I2C_SEL_C == sel)
        Grf->GRF_IOMUX_CON |= I2C0_SEL_MASK << 16 | I2C0_SEL_C;
    else
        return;
}

void Grf_I2C1_Sel(eI2C_SELECT sel)
{
    if( I2C_SEL_A == sel)
        Grf->GRF_IOMUX_CON |= I2C1_SEL_MASK << 16 | I2C1_SEL_A;
    else if( I2C_SEL_B == sel)
        Grf->GRF_IOMUX_CON |= I2C1_SEL_MASK << 16 | I2C1_SEL_B;
    else if( I2C_SEL_C == sel)
        Grf->GRF_IOMUX_CON |= I2C1_SEL_MASK << 16 | I2C1_SEL_C;
    else
        return;
}

void Grf_I2C2_Sel(eI2C_SELECT sel)
{
    if( I2C_SEL_A == sel)
        Grf->GRF_IOMUX_CON |= I2C2_SEL_MASK << 16 | I2C2_SEL_A;
    else if( I2C_SEL_B == sel)
        Grf->GRF_IOMUX_CON |= I2C2_SEL_MASK << 16 | I2C2_SEL_B;
    else if( I2C_SEL_C == sel)
        Grf->GRF_IOMUX_CON |= I2C2_SEL_MASK << 16 | I2C2_SEL_C;
    else
        return;
}


void Grf_SPI0_Sel(eSPI_SELECT sel)
{
    if( SPI_SEL_A == sel )
        Grf->GRF_IOMUX_CON = SPI0_SEL_MASK << 16 | SPI0_SEL_A;
    else
        Grf->GRF_IOMUX_CON = SPI0_SEL_MASK << 16 | SPI0_SEL_B;
}

void Grf_SPI1_Sel(eSPI_SELECT sel)
{
    if( SPI_SEL_A == sel)
        Grf->GRF_IOMUX_CON = SPI1_SEL_MASK << 16 | SPI1_SEL_A;
    else
        Grf->GRF_IOMUX_CON = SPI1_SEL_MASK << 16 | SPI1_SEL_B;
}


void Grf_UART0_Sel(eUART_SELECT sel)
{
    if(UART_SEL_B == sel)
    {
        Grf->GRF_UOC_CON0 = USBOTGCON_SEL_MASK << 16
                            | OTG_DISABLE0_BYPASS
                            | BYPASSSDMEN0_BYPASS
                            | BYPASSSEL0_BYPASS;
    }
    else if(UART_SEL_A == sel )
    {
        Grf->GRF_UOC_CON0 = USBOTGCON_SEL_MASK << 16
                            | OTG_DISABLE0_NOBYPASS
                            | BYPASSSDMEN0_NOBYPASS
                            | BYPASSSEL0_NOBYPASS;
    }

}

void Grf_UART1_Sel(eUART_SELECT sel)
{
    if(UART_SEL_A == sel)
        Grf->GRF_IOMUX_CON = UART1_SEL_MASK << 16 | UART1_SEL_A;
    else if( UART_SEL_B== sel )
        Grf->GRF_IOMUX_CON = UART1_SEL_MASK << 16 | UART1_SEL_B;
    else
        return;
}

void Grf_UART2_Sel(eUART_SELECT sel)
{
    if( UART_SEL_A == sel )
        Grf->GRF_IOMUX_CON = UART2_SEL_MASK << 16 | UART2_SEL_A;
    else if( UART_SEL_B== sel )
        Grf->GRF_IOMUX_CON = UART2_SEL_MASK << 16 | UART2_SEL_B;
    else if( UART_SEL_C== sel )
        Grf->GRF_IOMUX_CON = UART2_SEL_MASK << 16 | UART2_SEL_C;
    else
        return;
}

void Grf_I2S0_Sel(eI2S_SELECT sel)
{
    if( I2S_SEL_PAD == sel)
        Grf->GRF_IOMUX_CON = I2S0_SEL_MASK << 16 | I2S0_SEL_PAD;
    else
        Grf->GRF_IOMUX_CON = I2S0_SEL_MASK << 16 | I2S0_SEL_ACODEC;
}


void Grf_otgphy_suspend(BOOL Enable)
{
    if (Enable)
    {
        Grf->GRF_UOC_CON1 = (USB_CTRL_SEL_MASK << 16) | USB_CTRL_SEL;   //USB control enable
        Grf->GRF_UOC_CON1 = ((UTMI_SUSPEND_N_MASK | UTMI_OPMODE_MASK) << 16) | UTMI_SUSPEND_N_SUSPEND | UTMI_OPMODE_2; //0x000f0005
        Grf->GRF_UOC_CON2 = (USBPHY_COMMONON_MASK << 16) | USBPHY_COMMONON_PLL_SUSPEND;//0x00010001;
        Grf->GRF_USBPHY_CON6 = (TURN_OFF_DIFF_RECEIVER_MASK << 16) | TURN_OFF_DIFF_RECEIVER;//0x00040000;
        //Grf->GRF_USBPHY_CON7 = (VBUS_VOL_DET_MASK << 16) | VBUS_VOL_DET_OFF;//0x80008000;
    }
    else
    {
        Grf->GRF_UOC_CON2 = (USBPHY_COMMONON_MASK << 16) | USBPHY_COMMONON_PLL_RESUME;
        Grf->GRF_UOC_CON1 = ((UTMI_SUSPEND_N_MASK | UTMI_OPMODE_MASK) << 16) | UTMI_SUSPEND_N_NORMAL | UTMI_OPMODE_1;
        Grf->GRF_USBPHY_CON6 = (TURN_OFF_DIFF_RECEIVER_MASK << 16) | TURN_ON_DIFF_RECEIVER;
        //Grf->GRF_USBPHY_CON7 = (VBUS_VOL_DET_MASK << 16) | VBUS_VOL_DET_ON;
        Grf->GRF_UOC_CON1 = (USB_CTRL_SEL_MASK << 16) | USB_CTRL_NON_SEL;   //USB control enable
    }
    Delay100cyc(5);     //zyz: need delay
}


void Grf_Otgphy_Driver_Confige(void)
{
    Grf->GRF_USBPHY_CON2 = (AUTO_COMPENSATION_BYPASS_MASK << 16) | AUTO_COMPENSATION_BYPASS;//((0x03 << 16) | 0x03) << 10;
    Grf->GRF_USBPHY_CON1 = (((HS_EYE_HEIGHT_HI5BIT_MASK  << 16) | 0x01) |  //bit [0] [15:13]
                            ((HS_EYE_HEIGHT_LOW3BIT_MASK << 16) | 0x04));  //      1   100
    Grf->GRF_USBPHY_CON3 = (BIAS_CURRENT_REF_MASK << 16) | BIAS_CURRENT_REF_400MV;
    Grf->GRF_USBPHY_CON0 = HS_EYE_DIAG_ADJUST_MASK << 16 | HS_EYE_DIAG_ADJUST_IN_NON_CHIRP;//((0x07 << 16) | 0x04) << 0;
}

void Grf_Memory_Resp_Enable(eMemResp memory, BOOL enable)
{
    if (enable)
    {
        Grf->GRF_INTER_CON0 = (memory << 16) | memory;
    }
    else
    {
        Grf->GRF_INTER_CON0 = (memory << 16) | 0;
    }
}

void Grf_NOC_Remap_Sel(eNOC_REMAP_SELECT sel)
{
    if(NOC_BOOT_ROM == sel)
        Grf->GRF_INTER_CON0 = NOC_REMAP_MASK << 16 | NOC_REMAP_BOOT_ROM;
    else
        Grf->GRF_INTER_CON0 = NOC_REMAP_MASK << 16 | NOC_REMAP_PMU_SRAM;
}


void Grf_DMA2UART_Sel(eDMA2UART_SELECT sel)
{
    if(sel == DMA_UART0)
        Grf->GRF_INTER_CON0 = DMA_UART_MASK << 16 | DMA_UART0_SEL;
    else
        Grf->GRF_INTER_CON0 = DMA_UART_MASK << 16 | DMA_UART2_SEL;
}

void Grf_Force_Jtag_Set (bool enable)
{
    if(enable)
        Grf->GRF_INTER_CON0 = FORCE_JTAG_MASK << 16 | FORCE_JTAG_EN;
    else
        Grf->GRF_INTER_CON0 = FORCE_JTAG_MASK << 16 | FORCE_JTAG_DIS ;
}

void Grf_Verf_Vbg_Sel (eVERF_TRIM_SEL sel)
{
    Grf->GRF_VREF_TRIM_CON = (GRF_VREG_VBG_SEL_MASK << 16) | (sel << 4);
}

void Grf_Verf_Trim_Sel (eVERF_TRIM_SEL sel)
{
    Grf->GRF_VREF_TRIM_CON = (GRF_VREF_TRIM_MASK << 16) | (sel << 1);
}

void Grf_Set_ADC_Vref_PowerOn(eVERF_PD_SEL control)
{
    Grf->GRF_VREF_TRIM_CON = GRF_VREF_PWD_MASK << 16 | control;
}

BOOL Grf_SysCore_Is_Sleeping()
{
    int sleep_sta;
    sleep_sta = Grf->GRF_SOC_STATUS0;
    return ( sleep_sta & SYSCORE_SLEEPING)? TRUE : FALSE;
}

/*1 = indicates to the system that Cortex-M3 clock can be stopped.*/
BOOL Grf_SysCore_Get_SleepDeep()
{
    int sleep_sta;
    sleep_sta = Grf->GRF_SOC_STATUS0;
    return ( sleep_sta & SYSCORE_SLEEPDEEP)? TRUE : FALSE;
}

void Grf_SysCore_Set_SleepDeep()
{
    Grf->GRF_SOC_STATUS0 = SYSCORE_SLEEPDEEP;
}

/*Reads as one if the core is running (not halted) and a lockup condition is
    present.
*/
BOOL Grf_SysCore_Get_Lockup()
{
    int status;
    status = Grf->GRF_SOC_STATUS0;
    return ( status & SYSCORE_LOCKUP)? TRUE : FALSE;
}

BOOL Grf_SysCore_Is_Halted()
{
    int status;
    status = Grf->GRF_SOC_STATUS0;
    return ( status & SYSCORE_HALTED)? TRUE : FALSE;
}


BOOL Grf_CalCore_Is_Sleeping()
{
    int sleep_sta;
    sleep_sta = Grf->GRF_SOC_STATUS1;
    return ( sleep_sta & CALCORE_SLEEPING) ? TRUE:FALSE;
}

/*1 = indicates to the system that Cortex-M3 clock can be stopped.*/
BOOL Grf_CalCore_Get_SleepDeep()
{
    int sleep_sta;
    sleep_sta = Grf->GRF_SOC_STATUS1;
    return ( sleep_sta & SYSCORE_SLEEPDEEP)? TRUE : FALSE;
}

void Grf_CalCore_Set_SleepDeep()
{
    Grf->GRF_SOC_STATUS1 = CALCORE_SLEEPDEEP;
}

/*Reads as one if the core is running (not halted) and a lockup condition is
    present.
*/
BOOL Grf_CalCore_Get_Lockup()
{
    int lockup;
    lockup = Grf->GRF_SOC_STATUS1;
    return ( lockup & CALCORE_LOCKUP)? TRUE : FALSE;
}

BOOL Grf_CalCore_Is_Halted()
{
    int status;
    status = Grf->GRF_SOC_STATUS1;
    return ( status & CALCORE_HALTED)? TRUE : FALSE;
}



uint32 Grf_Get_PRJ_ID(void)
{
    uint32 prjID;
    prjID = Grf->GRF_PRJ_ID;
   return prjID;
}


/*
--------------------------------------------------------------------------------
  Function name : Grf_Get_Cpu_ID(void)
  Author        : aaron.sun
  Description   : get cpu id.
  Input         :

  Return        : 0: main cpu
                  1: calculator cpu

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 Grf_Get_Cpu_ID(void)
{
    return Grf->GRF_CPU_ID;
}



/*
--------------------------------------------------------------------------------
  Function name : Grf_CheckVbus(void)
  Author        : aaron.sun
  Description   : check vbus status
  Input         :

  Return        : 0: vbus removed
                  1: vbus insert

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
uint32 Grf_CheckVbus(void)
{
#ifndef __ENABLE_POWERMANAGER
    return 0;//JJJHHH 20161013 don't enter charge...
#endif
#ifndef _BROAD_LINE_OUT_
    //return Grf->GRF_SOC_USB_STATUS;
    return ((Grf->GRF_SOC_USB_STATUS & 0x2)==0x2)? TRUE : FALSE;
#else
    return 0;
#endif
}

/*
********************************************************************************
*
*                         End of Scu.c
*
********************************************************************************
*/


