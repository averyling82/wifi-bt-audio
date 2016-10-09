/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name¡êo   Hw_CRU.h
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo          2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#ifndef _HW_CRU_H
#define _HW_CRU_H

typedef volatile struct CRUReg_s
{
    UINT32 CRU_APLL_CON0;
    UINT32 CRU_APLL_CON1;
    UINT32 CRU_APLL_CON2;
    UINT32 reserved1[1];
    UINT32 CRU_MODE_CON;
    UINT32 CRU_CLKSEL_CON[13];
    UINT32 reserved2[2];
    UINT32 CRU_CLK_FRACDIV_CON0;
    UINT32 CRU_CLK_FRACDIV_CON1;
    UINT32 reserved3[10];
    UINT32 CRU_CLKGATE_CON[10];
    UINT32 reserved4[6];
    UINT32 CRU_SOFTRST[4];
    UINT32 reserved5[4];
    UINT32 CRU_STCLK_CON0;
    UINT32 CRU_STCLK_CON1;
    UINT32 reserved6[3];
    UINT32 CRU_GLB_SRST_FST_VALUE;
    UINT32 CRU_GLB_CNT_TH;

}CRUReg_t, *pCRUReg_t;

#define CRU  ((CRUReg_t *) CRU_BASE)

typedef enum CLOCK_GATE
{
    CLK_SYS_CORE_GATE = 0, //CRU_CLKGATE0_CON[0]
    HCLK_SYS_CORE_GATE,
    HCLK_CAL_CORE_GATE,
    HCLK_LOGIC_GATE,
    PCLK_LOGIC_GATE,
    CLK_RESERVED0,
    HCLK_PMU_GATE,
    CLK_USBPHY_GATE,
    CLK_SFC_GATE,
    CLK_RESERVED1,
    CLK_SARADC_GATE,
    CLK_TIMER0_GATE,
    CLK_TIMER1_GATE,
    PCLK_PMU_GATE,
    CLK_ACODEC_GATE,

    CLK_I2S1_SRC_GATE = 16,//CRU_CLKGATE0_CON[1]
    CLK_I2S1_FRAC_SRC_GATE,
    CLK_I2S1_OUT_GATE,
    CLK_I2S1_GATE,
    CLK_I2S0_SRC_GATE,
    CLK_I2S_FRAC_SRC_GATE,
    CLK_I2S_GATE,
    DCLK_EBC_SRC_GATE,
    CLK_SDMMC_GATE,
    CLK_EMMC_GATE,
    CLK_SPI0_GATE,
    CLK_SPI1_GATE,
    CLK_UART0_GATE,
    CLK_UART1_GATE,
    CLK_UART2_GATE,
    CLK_TEST_GATE,

    CLK_UART3_GATE = 32,//CRU_CLKGATE0_CON[2]
    CLK_UART4_GATE,
    CLK_UART5_GATE,

    HDRAM0_GATE = 48,//CRU_CLKGATE0_CON[3]
    HDRAM1_GATE,
    HDRAM2_GATE,
    HDRAM3_GATE,
    HDRAM4_GATE,
    HDRAM5_GATE,
    HDRAM6_GATE,
    HDRAM7_GATE,
    HDRAM_INTERFACE_GATE,
    HCLK_HIFI_GATE,

    HIRAM0_GATE = 64,//CRU_CLKGATE0_CON[4]
    HIRAM1_GATE,
    HIRAM2_GATE,
    HIRAM3_GATE,
    Reserved4_GATE,
    Reserved5_GATE,
    Reserved6_GATE,
    Reserved7_GATE,
    HIRAM_INTERFACE_GATE,
    HCLK_HIGH_GATE,
    HCLK_HIGH_MATRIX_GATE,
    HCLK_HIGH_DMA_GATE,
    HCLK_ASYNC_BRG_GATE,

    PCLK_PMU_BUS_GATE = 80,//CRU_CLKGATE0_CON[5]
    CLK_RESERVED2,
    CLK_RESERVED3,
    PCLK_GRF_GATE,
    PCLK_GPIO2_GATE,
    CLK24M_GATE,
    PMU_RAM_INTERFACE_GATE,
    PMU_RAM0_GATE,
    PMU_RAM1_GATE,
    CLK_RESERVED4,
    PVTM_CLK_GATE,

    DRAM0_GATE = 96,//CRU_CLKGATE0_CON[6]
    DRAM1_GATE,
    DRAM2_GATE,
    DRAM3_GATE,
    DRAM4_GATE,
    DRAM5_GATE,
    DRAM6_GATE,
    DRAM7_GATE,
    DRAM_INTERFACE_GATE,
    HCLK_APB_BRG_GATE,
    HCLK_USBC_GATE,
    HCLK_SDMMC_GATE,
    HCLK_EMMC_GATE,
    HCLK_EBC_GATE,
    HCLK_SYNTH_GATE,
    HCLK_SFC_GATE,

    IRAM0_GATE = 112, //CRU_CLKGATE0_CON[7]
    IRAM1_GATE,
    IRAM2_GATE,
    IRAM3_GATE,
    IRAM4_GATE,
    IRAM5_GATE,
    IRAM6_GATE,
    IRAM7_GATE,
    IRAM8_GATE,
    IRAM9_GATE,
    IRAM_INTERFACE_GATE,
    HCLK_BOOTROM_GATE,
    HCLK_1TOM_BRG_GATE,
    HCLK_DMA_GATE,
    HCLK_IMDCT_GATE,
    HCLK_VOP_GATE,

    HCLK_PMU_APB_BRG_GATE = 128,//CRU_CLKGATE0_CON[8]
    PCLK_I2S0_GATE,
    PCLK_I2S1_GATE,
    PCLK_I2C0_GATE,
    PCLK_I2C1_GATE,
    PCLK_I2C2_GATE,
    PCLK_SPI0_GATE,
    PCLK_SPI1_GATE,
    PCLK_UART0_GATE,
    PCLK_UART1_GATE,
    PCLK_UART2_GATE,
    PCLK_UART3_GATE,
    PCLK_UART4_GATE,
    PCLK_UART5_GATE,
    PCLK_TIMER_GATE,
    PCLK_WDT_GATE,

    PCLK_PWM0_GATE = 144, //CRU_CLKGATE0_CON[9]
    PCLK_PWM1_GATE,
    PCLK_MAILBOX_GATE,
    PCLK_SARADC_GATE,
    PCLK_GPIO0_GATE,
    PCLK_GPIO1_GATE,
    PCLK_ACODEC_GATE,
    PCLK_ASYNC_BRG_GATE,
    PCLK_LGC_MATRIX_GATE,

}eCLOCK_GATE;

typedef enum SOFT_RST
{
    USBOTG_SRST = 0,    //CRU_SOFTRST0_CON[0]
    USBPHY_SRST,
    USBGLB_SRST,
    I2S0_SRST,
    I2S1_SRST,
    SPI0_SRST,
    SPI1_SRST,
    UART0_SRST,
    UART1_SRST,
    UART2_SRST,
    BOOTROM_SRST,
    SYSRAM1_SRST,
    SYSRAM0_SRST,
    VOP_SRST,
    IMDCT_SRST,
    SYNTH_SRST,


    SYSDMA_SRST = 16,   //CRU_SOFTRST0_CON[1]
    SDMMC1_SRST,
    SDMMC0_SRST,
    SYSMATRIX_SRST,
    TIMER0_SRST,
    TIMER1_SRST,
    SARADC_SRST,
    PWM0_SRST,
    PWM1_SRST,
    SFC_SRST,
    I2C0_SRST,
    I2C1_SRST,
    I2C2_SRST,
    EBC_SRST,
    MAILBOX_SRST,
    WDT_SRST,

    HIGH_MATRIX_SRST = 32,//CRU_SOFTRST0_CON[2]
    HIGHRAM1_SRST,
    HIGHRAM0_SRST,
    HIFI_SRST,
    RESERVED0,
    UART3_SRST,
    UART4_SRST,
    UART5_SRST,
    GPIO0_SRST,
    GPIO1_SRST,

    ACODEC_SRST0 = 48,    //CRU_SOFTRST0_CON[3]
    SYS_CORE_SRST,
    CAL_CORE_SRST,
    BRG_TO_PMU_SRST,
    RESERVED1,
    PMU_SRST,
    ASYNC_BRG_SRST,
    DMA2_SRST,

}eSOFT_RST;

typedef enum
{
    I2S1_CLK,
    XIN12M,
}I2S_output_clk_Sel;

typedef enum
{
    obs_clk_acodec,
    obs_clk_pvtm,
    obs_clk_sys_core,
    obs_clk_cal_core,
    obs_clk_sdmmc,
    obs_pclk_logic ,
    obs_clk_i2s0,
    obs_clk_i2s1,
}OBS_Source_Sel;
typedef enum
{
    PLL_MUX_CLK,    //for all app
    XIN24M,         //for all app
    CLK_ACODEC_PLL, //for i2s only
    I2S_XIN12M,     //for i2s only
    USB480M,        //cal_core;sys_core;ebc;uart0
    PCLK_LOGIC,     //for TIME0-1 only
    USBPHY_480_ORI, //for usbphy480m
    Clock_Source_ERR,
}Clock_Source_Sel;

//CRU_APLL_CON0
#define APLL_BYPASS         ((UINT32)(1))
#define APLL_NO_BYPASS         ((UINT32)(0))

#define APLL_BYPASS_MASK    ((UINT32)(0x01))
#define APLL_BYPASS_SHIFT    ((UINT32)(15))

#define APLL_POSTDIV1_SHIFT ((UINT32)(12) << 0)
#define APLL_POSTDIV1_MASK  ((UINT32)(0x07))
#define APLL_FBDIV_SHIFT    ((UINT32)(0) << 0)
#define APLL_FBDIV_MASK     ((UINT32)(0xFFF))

//CRU_APLL_CON1
#define APLL_INTERNAL_RESET          ((UINT32)(0))
#define APLL_SOFTWARE_RESET          ((UINT32)(1))
#define APLL_RESET_MASK              ((UINT32)(0x01))
#define APLL_RESET_SHIFT             ((UINT32)(15))

#define APLL_SOFTWARE_PD             ((UINT32)(1))
#define APLL_SOFTWARE_PU             ((UINT32)(0))

#define APLL_SOFTWARE_PD_MASK        ((UINT32)(0x01))
#define APLL_SOFTWARE_PD_SHIFT       ((UINT32)(14))

#define APLL_DSMPD_INTEGER           ((UINT32)(1))
#define APLL_DSMPD_FRAC              ((UINT32)(0))
#define APLL_DSMPD_MASK              ((UINT32)(0x01))
#define APLL_DSMPD_SHIFT             ((UINT32)(12))
#define APLL_LOCK_SHIFT              ((UINT32)(10))
#define APLL_POSTDIV2_SHIFT          ((UINT32)(6) << 0)
#define APLL_POSTDIV2_MASK           ((UINT32)(0x07))
#define APLL_REFDIV_SHIFT            ((UINT32)(0) << 0)
#define APLL_REFDIV_MASK             ((UINT32)(0X3F))

//CRU_APLL_CON2
#define APLL_FOUT4PHASE_PD          ((UINT32)(0x01) << 27)
#define APLL_FOUTVCO_PD             ((UINT32)(0x01) << 26)
#define APLL_FOUTPOSTDIV_PD         ((UINT32)(0x01) << 25)
#define APLL_DAC_PD                 ((UINT32)(0x01) << 24)
#define APLL_FRAC_SHIFT             ((UINT32)(0) << 0)

//CRU_MODE_CON
#define DIV_CON_24M_SHIFT       ((UINT32)(8) << 0)
#define DIV_CON_24M_MASK        ((UINT32)(0x3F))

#define ACODEC_PLL_SEL_24M      ((UINT32)(0))
#define ACODEC_PLL_SEL_PLL      ((UINT32)(1))
#define ACODEC_PLL_SEL_MASK     ((UINT32)(0x01))
#define ACODEC_PLL_SEL_SHIFT    ((UINT32)(5) << 0)

#define USB480M_SEL_24M         ((UINT32)(0))
#define USB480M_SEL_480M        ((UINT32)(1))
#define USB480M_SEL_MASK        ((UINT32)(0x01))
#define USB480M_SEL_SHIFT       ((UINT32)(4) << 0)

#define APLL_WORK_MODE_SLOW     ((UINT32)(0) << 0)
#define APLL_WORK_MODE_NORMAL   ((UINT32)(1) << 0)
#define APLL_WORK_MODE_MASK     ((UINT32)(0x01))
#define APLL_WORK_MODE_SHIFT    ((UINT32)(0) << 0)

//CRU_CLKSEL0_CON
#define CAL_CORE_SRC_SEL_PLL        ((UINT32)0)
#define CAL_CORE_SRC_SEL_480M       ((UINT32)1)
#define CAL_CORE_SRC_SEL_SHIFT        ((UINT32)(15) << 0)

#define CAL_CORE_DIV_CON_MASK         ((UINT32)(0x07))
#define CAL_CORE_DIV_CON_SHIFT        ((UINT32)(12) << 0)

#define CAL_STCLK_DIV_CON_MASK         ((UINT32)(0x07))
#define CAL_STCLK_DIV_CON_SHIFT        ((UINT32)(8) << 0)

#define SYS_STCLK_DIV_CON_MASK         ((UINT32)(0x07))
#define SYS_STCLK_DIV_CON_SHIFT        ((UINT32)(4) << 0)

#define SYS_CORE_SRC_SEL_PLL       ((UINT32)0)
#define SYS_CORE_SRC_SEL_480M      ((UINT32)1)
#define SYS_CORE_SRC_SEL_SHIFT        ((UINT32)(3) << 0)

#define SYS_CORE_DIV_CON_MASK         ((UINT32)(0x07))
#define SYS_CORE_DIV_CON_SHIFT        ((UINT32)(0) << 0)

//CRU_CLKSEL1_CON
#define PCLK_LOGIC_DIV_CON_1        ((UINT32)(0x00) << 8)
#define PCLK_LOGIC_DIV_CON_2        ((UINT32)(0x01) << 8)
#define PCLK_LOGIC_DIV_CON_4        ((UINT32)(0x02) << 8)
#define PCLK_LOGIC_DIV_CON_8        ((UINT32)(0x03) << 8)

//CRU_CLKSEL2_CON
#define TIMER1_PLL_SEL_24M          ((UINT32)0)
#define TIMER1_PLL_SEL_PCLK         ((UINT32)1)

#define TIMER1_DIV_CON_MASK         ((UINT32)(0x1f))
#define TIMER1_DIV_CON_SHIFT        ((UINT32)(8) << 0)

#define TIMER0_PLL_SEL_24M          ((UINT32)0)
#define TIMER0_PLL_SEL_PCLK         ((UINT32)1)

#define TIMER0_DIV_CON_MASK         ((UINT32)(0x1f))
#define TIMER0_DIV_CON_SHIFT        ((UINT32)(0) << 0)

//CRU_CLKSEL3_CON
#define I2S1_PLL_SEL_SOC             ((UINT32)0)
#define I2S1_PLL_SEL_ACODEC          ((UINT32)1)

#define I2S1_OUT_SEL_CLK_I2S1       ((UINT32)0)
#define I2S1_OUT_SEL_CLK_12M        ((UINT32)1)

#define I2S1_CLK_SEL_PLL_CLOCK      ((UINT32)0)
#define I2S1_CLK_SEL_FRAC_DIV       ((UINT32)1)
#define I2S1_CLK_SEL_12M            ((UINT32)2)

#define I2S1_DIV_CON_MASK             ((UINT32)(0x7f))
#define I2S1_DIV_CON_SHIFT            ((UINT32)(0) << 0)


//CRU_CLKSEL4_CON
#define I2S0_PLL_SEL_SOC             ((UINT32)0)
#define I2S0_PLL_SEL_ACODEC          ((UINT32)1)

#define I2S0_CLK_SEL_PLL_CLOCK      ((UINT32)0)
#define I2S0_CLK_SEL_FRAC_DIV       ((UINT32)1)
#define I2S0_CLK_SEL_12M            ((UINT32)2)

#define I2S0_DIV_CON_MASK             ((UINT32)(0x7f))
#define I2S0_DIV_CON_SHIFT            ((UINT32)(0) << 0)

//CRU_CLKSEL5_CON
#define EBC_DCLK_SEL_PLL            ((UINT32)0)
#define EBC_DCLK_SEL_480M              ((UINT32)1)

#define EBC_DCLK_DIV_CON_MASK       ((UINT32)(0xFF))
#define EBC_DCLK_DIV_CON_SHIFT      ((UINT32)(0) << 0)

//CRU_CLKSEL6_CON
#define SARADC_DIV_CON_MASK         ((UINT32)(0xFF))
#define SARADC_DIV_CON_SHIFT        ((UINT32)(0) << 0)


//CRU_CLKSEL7_CON
#define EMMC_PLL_SEL_24M            ((UINT32)1)
#define EMMC_PLL_SEL_PLL               ((UINT32)0)

#define EMMC_DIV_CON_MASK             ((UINT32)(0x1f))
#define EMMC_DIV_CON_SHIFT            ((UINT32)(8) << 0)

#define SDMMC_CLK_PLL_SEL_24M       ((UINT32)1)
#define SDMMC_CLK_PLL_SEL_PLL       ((UINT32)0)

#define TIMER0_DIV_CON_MASK         ((UINT32)(0x1f))
#define SDMMC_DIV_CON_SHIFT            ((UINT32)(0) << 0)

//CRU_CLKSEL8_CON
#define SPI1_CLK_PLL_SEL_24M        ((UINT32)1)
#define SPI1_CLK_PLL_SEL_PLL       ((UINT32)0)

#define SPI1_DIV_CON_MASK             ((UINT32)(0x3f))
#define SPI1_DIV_CON_SHIFT            ((UINT32)(8) << 0)

#define SPI0_CLK_PLL_SEL_24M         ((UINT32)1)
#define SPI0_CLK_PLL_SEL_PLL        ((UINT32)0)

#define SPI0_DIV_CON_MASK             ((UINT32)(0x3f))
#define SPI0_DIV_CON_SHIFT            ((UINT32)(0) << 0)

//CRU_CLKSEL9_CON
#define UART0_PLL_SEL_24M             ((UINT32)1)
#define UART0_PLL_SEL_PLL           ((UINT32)0)

#define UART0_DIV_CON_MASK             ((UINT32)(0x3f))
#define UART0_DIV_CON_SHIFT            ((UINT32)(8) << 0)

#define UART1_PLL_SEL_PLL              ((UINT32)0)
#define UART1_PLL_SEL_24M             ((UINT32)1)
#define UART1_PLL_SEL_480M             ((UINT32)2)

#define UART1_DIV_CON_MASK             ((UINT32)(0x3f))
#define UART1_DIV_CON_SHIFT            ((UINT32)(0) << 0)

//CRU_CLKSEL10_CON
#define SFC_PLL_SEL_24M             ((UINT32)1)
#define SFC_PLL_SEL_PLL             ((UINT32)0)

#define SFC_PLL_SEL_MASK             ((UINT32)(0x1f))
#define SFC_PLL_SEL_SHIFT            ((UINT32)(8) << 0)

#define UART2_PLL_SEL_PLL              ((UINT32)0)
#define UART2_PLL_SEL_24M             ((UINT32)1)

#define UART2_DIV_CON_MASK             ((UINT32)(0x3f))
#define UART2_DIV_CON_SHIFT            ((UINT32)(0) << 0)
//CRU_CLKSEL11_CON
#define UART3_PLL_SEL_PLL              ((UINT32)0)
#define UART3_PLL_SEL_24M             ((UINT32)1)

#define UART3_DIV_CON_MASK             ((UINT32)(0x3f))
#define UART3_DIV_CON_SHIFT            ((UINT32)(8) << 0)

#define CORE_CLK_PLL_SEL_ACODEC        ((UINT32)0x00 << 5)
#define CORE_CLK_PLL_SEL_PVTM       ((UINT32)0x01 << 5)
#define CORE_CLK_PLL_SEL_SYS_CORE   ((UINT32)0x02 << 5)
#define CORE_CLK_PLL_SEL_CAL_CORE   ((UINT32)0x03 << 5)
#define CORE_CLK_PLL_SEL_SDMMC        ((UINT32)0x04 << 5)
#define CORE_CLK_PLL_SEL_PCLk_LOGIC   ((UINT32)0x05 << 5)
#define CORE_CLK_PLL_SEL_CLK_I2S0    ((UINT32)0x06 << 5)
#define CORE_CLK_PLL_SEL_CLK_I2S1    ((UINT32)0x07 << 5)

#define TEST_DIV_CON_MASK     ((UINT32)(0x1f))
#define TEST_DIV_CON_SHIFT    ((UINT32)(0) << 0)

//CRU_CLKSEL12_CON
#define UART5_PLL_SEL_24M       ((UINT32)1)
#define UART5_PLL_SEL_PLL       ((UINT32)0)

#define UART5_DIV_CON_MASK     ((UINT32)(0x3f))
#define UART5_DIV_CON_SHIFT    ((UINT32)(8) << 0)

#define UART4_PLL_SEL_PLL       ((UINT32)0)
#define UART4_PLL_SEL_24M       ((UINT32)1)

#define UART4_DIV_CON_MASK     ((UINT32)(0x3f))
#define UART4_DIV_CON_SHIFT    ((UINT32)(0) << 0)

#define CLK_SEL_MASK_1          ((UINT32)(0x01) << 0)
#define CLK_SEL_MASK_3          ((UINT32)(0x03) << 0)
#define CLK_SEL_MASK_7          ((UINT32)(0x07) << 0)
#define CLK_SEL_MASK_F          ((UINT32)(0x0F) << 0)
#define CLK_SEL_MASK_1F         ((UINT32)(0x1F) << 0)
#define CLK_SEL_MASK_3F         ((UINT32)(0x3F) << 0)
#define CLK_SEL_MASK_7F         ((UINT32)(0x7F) << 0)
#define CLK_SEL_MASK_FF         ((UINT32)(0xFF) << 0)
#define CLK_SEL_MASK_FFFF       ((UINT32)(0xFFFF) << 0)


#define MPMU_DAC_VOL_UPD_ACTIVE     ((UINT32)1 << 1)
#define MPMU_DAC_VOL_UPD_INACTIVE   ((UINT32)0 << 1)
#define MPMU_DAC_VOL_UPD_MASK       ((UINT32)1 << 1)

#define MPMU_DAC_MUTE               ((UINT32)1 << 2)
#define MPMU_DAC_UNMUTE             ((UINT32)0 << 2)
#define MPMU_DAC_MUTE_MASK          ((UINT32)1 << 2)
//CRU_PLL_CON
#define PLL_BYPASS          ((UINT32)(1) << 0)
#define PLL_CLKOD_SHIFT     ((UINT32)(1) << 0)
#define PLL_CLKM_SHIFT      ((UINT32)(5) << 0)
#define PLL_CLKN_SHIFT      ((UINT32)(13)<< 0)
#define PLL_RESET           ((UINT32)(1) << 17)
#define PLL_POWER_DOWN      ((UINT32)(1) << 18)
#define PLL_POWER_LOCK      ((UINT32)(1) << 19)


//CRU_CLKSEL_CPU_CON
#define X24MIN_CLKDIV_SHIFT     ((UINT32)(9))
#define X24MIN_CLKDIV_MASK      ((UINT32)(0x03))
#define SYSTICK_CLKDIV_SHIFT    ((UINT32)(0))
#define SYSTICK_CLKDIV_MASK     ((UINT32)(0x07))
#define AHBCLK_CLKDIV_SHIFT     ((UINT32)(3))
#define AHBCLK_CLKDIV_MASK      ((UINT32)(0x07))
#define APBCLK_CLKDIV_SHIFT     ((UINT32)(7))
#define APBCLK_CLKDIV_MASK      ((UINT32)(0x03))
#define ARMCLK_SEL_SHIFT        ((UINT32)(6))
#define ARMCLK_SEL_24M          ((UINT32)(0))
#define ARMCLK_SEL_PLL          ((UINT32)(1))
#define ARMCLK_SEL_MASK         ((UINT32)(1))

//CRU_CLKSEL_CODEC_CON
#define I2S_CLK_SEL_PLL         ((UINT32)(0))
#define I2S_CLK_SEL_12M         ((UINT32)(1))
#define I2S_CLK_SEL_MASK        ((UINT32)(1))
#define I2S_CLK_DIV_SHIFT       ((UINT32)(1))
#define I2S_CLK_DIV_MASK        ((UINT32)(0xff))

//CRU_CLKSEL_MMC_CON
#define MMC_CLK_SEL_PLL         ((UINT32)(0))
#define MMC_CLK_SEL_24M         ((UINT32)(1))
#define MMC_CLK_SEL_MASK        ((UINT32)(1))
#define MMC_CLK_DIV_SHIFT       ((UINT32)(1))
#define MMC_CLK_DIV_MASK        ((UINT32)(0x1f))

//CRU_CLKSEL_SPI_CON
#define SPI_CLK_SEL_PLL         ((UINT32)(0))
#define SPI_CLK_SEL_24M         ((UINT32)(1))
#define SPI_CLK_SEL_MASK        ((UINT32)(1))
#define SPI_CLK_DIV_SHIFT       ((UINT32)(1))
#define SPI_CLK_DIV_MASK        ((UINT32)(0x3f))

//CRU_CLKSEL_UART_CON
#define UART_CLK_SEL_PLL        ((UINT32)(0))
#define UART_CLK_SEL_24M        ((UINT32)(1))
#define UART_CLK_SEL_MASK       ((UINT32)(1))
#define UART_CLK_DIV_SHIFT      ((UINT32)(1))
#define UART_CLK_DIV_MASK       ((UINT32)(0x3f))

//CRU_CLKSEL_SARADC_CON (24M sel Only)
#define ADC_CLK_DIV_SHIFT       ((UINT32)(0))
#define ADC_CLK_DIV_MASK        ((UINT32)(0x3f))


/*
********************************************************************************
*
*                         End of hw_Pll.h
*
********************************************************************************
*/

#endif

