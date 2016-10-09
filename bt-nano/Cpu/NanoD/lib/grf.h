/*
********************************************************************************
*                   Copyright (c) 2008,anzhiguo
*                         All rights reserved.
*
* File Name£º  Grf.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24          1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _GRF_H
#define _GRF_H

#undef EXT
#ifdef _IN_GRF_
#define EXT
#else
#define EXT extern
#endif

#ifndef SUPPORT_ECO_CHIP
#define SUPPORT_ECO_CHIP
#endif

typedef enum
{
    I2C_SEL_A,
    I2C_SEL_B,
    I2C_SEL_C,

}eI2C_SELECT;

typedef enum
{
    SPI_SEL_A,
    SPI_SEL_B,

}eSPI_SELECT;

typedef enum
{
    UART_SEL_A,
    UART_SEL_B,
    UART_SEL_C,

}eUART_SELECT;

typedef enum
{
    I2S_SEL_PAD,
    I2S_SEL_ACODEC,
    I2S_SEL_DEV1_PA,
    I2S_SEL_DEV1_PB,

}eI2S_SELECT;

typedef enum
{
    NOC_BOOT_ROM,
    NOC_PMU_SRAM,

}eNOC_REMAP_SELECT;

typedef enum
{
    DMA_UART0,
    DMA_UART2,

}eDMA2UART_SELECT;


typedef enum
{
    //GRF_VREF_TRIM_2400,
    GRF_VREF_TRIM_2425,
    GRF_VREF_TRIM_2450,
    GRF_VREF_TRIM_2475,
    GRF_VREF_TRIM_2500,
    GRF_VREF_TRIM_2525,
    GRF_VREF_TRIM_2550,
    GRF_VREF_TRIM_2575,
    GRF_VREF_TRIM_2600,

}eVERF_TRIM_SEL;

typedef enum
{
#ifndef SUPPORT_ECO_CHIP
    GRF_VREF_PWON,
    GRF_VREF_PWOFF,
#else
    GRF_VREF_PWOFF,
    GRF_VREF_PWON,
#endif
//    GRF_VREF_PWON,
//    GRF_VREF_PWOFF,
}eVERF_PD_SEL;

typedef enum eDataPortIOMux
{
    IO_SDMMC ,
    IO_LCD ,
    IO_EMMC,
    IO_FLASH,
    IO_REV ,

} eDataPortIOMux_t;


typedef enum eMemoryResp
{
    HIGH_DRAM = ((uint32)(0x01) << 15),
    HIGH_IRAM = ((uint32)(0x01) << 14),
    LOW_DRAM  = ((uint32)(0x01) << 13),
    LOW_IRAM  = ((uint32)(0x01) << 12),
    PMU_RAM   = ((uint32)(0x01) << 6),

}eMemResp;

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
extern void Grf_GpioMuxSet(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinIOMux_t data);
extern int32 Grf_GpioMuxGet(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum);
extern void Grf_GPIO_SetPinPull(eGPIO_CHANNEL gpioPort,eGPIOPinNum_t GPIOPinNum, eGPIOPinPull_t pull);
extern void Grf_PVTM_OSC_Enable(BOOL Enable);
extern void Grf_PVTM_Fun_Start(BOOL bStart);
extern void Grf_PVTM_Set_Divider(int clk_src,int clk);
extern void Grf_PVTM_Set_Cal_Cnt(int cnt);
extern int Grf_PVTM_Get_Cal_Cnt(void);
extern void Grf_PVTM_Set_24M_Divider(int clk);
extern bool Grf_PVTM_Is_FreqCal_Done(void);
extern int Grf_PVTM_Get_Freq_Cnt(void);
extern void Grf_I2C0_Sel(eI2C_SELECT sel);
extern void Grf_I2C1_Sel(eI2C_SELECT sel);
extern void Grf_I2C2_Sel(eI2C_SELECT sel);
extern void Grf_SPI0_Sel(eSPI_SELECT sel);
extern void Grf_SPI1_Sel(eSPI_SELECT sel);
extern void Grf_UART1_Sel(eUART_SELECT sel);
extern void Grf_UART2_Sel(eUART_SELECT sel);
extern void Grf_I2S0_Sel(eI2S_SELECT sel);
extern void Grf_otgphy_suspend(BOOL Enable);
extern void Grf_Memory_Set_High_Speed(uint32 memory);
extern void Grf_Memory_Set_Low_Speed(uint32 memory);

extern void Grf_NOC_Remap_Sel(eNOC_REMAP_SELECT sel);
extern void Grf_DMA2UART_Sel(eDMA2UART_SELECT sel);
extern void Grf_Force_Jtag_Set (bool enable);
extern void Grf_Verf_Trim_Sel (eVERF_TRIM_SEL sel);
extern void Grf_Set_ADC_Vref_PWD(void);
extern void Grf_Set_ADC_Vref_PwOn(void);
extern BOOL Grf_SysCore_Is_Sleeping(void);
extern BOOL Grf_SysCore_Get_SleepDeep(void);
extern void Grf_SysCore_Set_SleepDeep(void);
extern BOOL Grf_SysCore_Get_Lockup(void);
extern BOOL Grf_SysCore_Is_Halted(void);
extern BOOL Grf_CalCore_Is_Sleeping(void);
extern BOOL Grf_CalCore_Get_SleepDeep(void);
extern void Grf_CalCore_Set_SleepDeep(void);
extern BOOL Grf_CalCore_Get_Lockup(void);
extern BOOL Grf_CalCore_Is_Halted(void);
extern uint32 Grf_Get_Cpu_ID(void);
extern uint32 Grf_CheckVbus(void);

/*
********************************************************************************
*
*                         End of Scu.h
*
********************************************************************************
*/

#endif

