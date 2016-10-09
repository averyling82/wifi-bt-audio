/*
********************************************************************************
*                   Copyright (c) 2015,anzhiguo
*                         All rights reserved.
*
* File Name：   Hw_grf.h
*
* Description:
*
* History:      <author>          <time>        <version>
*
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HWAPI_GRF_H
#define _HWAPI_GRF_H

typedef volatile struct GRFREG_t
{
    UINT32 GPIO_IO0MUX[4];
    UINT32 GPIO_IO0PULL[4];     //0x10

    UINT32 GPIO_IO1MUX[4];      //0x20
    UINT32 GPIO_IO1PULL[4];

    UINT32 GPIO_IO2MUX[4];      //0x40
    UINT32 GPIO_IO2PULL[4];

    UINT32 GRF_PVTM_CON0;   //GRF_DLL_CON0 0x60
    UINT32 GRF_PVTM_CON1;
    UINT32 GRF_PVTM_CON2;

    UINT32 GRF_PVTM_STATUS0;    //GRF_DLL_STATUS0
    UINT32 GRF_PVTM_STATUS1;    //0x70

    UINT32 PAD0[3];

    UINT32 GRF_USBPHY_CON0;     //0x80
    UINT32 GRF_USBPHY_CON1;
    UINT32 GRF_USBPHY_CON2;
    UINT32 GRF_USBPHY_CON3;
    UINT32 GRF_USBPHY_CON4;     //0x90
    UINT32 GRF_USBPHY_CON5;
    UINT32 GRF_USBPHY_CON6;
    UINT32 GRF_USBPHY_CON7;
    UINT32 GRF_USBPHY_CON8;     //0xa0
    UINT32 GRF_USBPHY_CON9;
    UINT32 GRF_USBPHY_CON10;
    UINT32 GRF_USBPHY_CON11;

    UINT32 GRF_UOC_CON0;        //0xb0
    UINT32 GRF_UOC_CON1;
    UINT32 GRF_UOC_CON2;

    UINT32 GRF_IOMUX_CON;
    UINT32 PAD1;                //0xc0
    UINT32 GRF_INTER_CON0;
    UINT32 PAD2;
    UINT32 GRF_VREF_TRIM_CON;   //0xcc
    UINT32 PAD3[4];             //0xd0 - 0xdc
    UINT32 GRF_SOC_STATUS0;     //0xe0
    UINT32 GRF_SOC_STATUS1;
    UINT32 GRF_SOC_USB_STATUS;  //0xe8
    UINT32 PAD4[3];             //0xec - 0xf4
    UINT32 GRF_PRJ_ID;          //f8
    UINT32 GRF_CPU_ID;

}GRFReg_t, *pGRFReg_t; //将IO mux 放在别处处理

#define Grf     ((GRFReg_t *) GRF_BASE)

/*
*-------------------------------------------------------------------------------
*
*                         GRF memap register define
*
*-------------------------------------------------------------------------------
*/

//GRF_GPIO0A_IOMUX
#define IOMUX_GPIO0A0_RESERVE            ((uint32)(3))
#define IOMUX_GPIO0A0_I2S1B_CLK          ((uint32)(2))
#define IOMUX_GPIO0A0_EMMC_PWREN         ((uint32)(1))
#define IOMUX_GPIO0A0_IO                 ((uint32)(0))

#define IOMUX_GPIO0A1_UART2C_TX          ((uint32)(3))
#define IOMUX_GPIO0A1_I2S1B_LRCK          ((uint32)(2))
#define IOMUX_GPIO0A1_EMMC_CLK           ((uint32)(1))
#define IOMUX_GPIO0A1_IO           		 ((uint32)(0))

#define IOMUX_GPIO0A2_UART2C_RX          ((uint32)(3))
#define IOMUX_GPIO0A2_I2S1B_SCLK         ((uint32)(2))
#define IOMUX_GPIO0A2_EMMC_CMD           ((uint32)(1))
#define IOMUX_GPIO0A2_IO                 ((uint32)(0))

#define IOMUX_GPIO0A3_UART2C_CTS         ((uint32)(3))
#define IOMUX_GPIO0A3_I2S1B_SDO          ((uint32)(2))
#define IOMUX_GPIO0A3_EMMC_D0            ((uint32)(1))
#define IOMUX_GPIO0A3_IO                 ((uint32)(0))

#define IOMUX_GPIO0A4_UART2C_RTS         ((uint32)(3))
#define IOMUX_GPIO0A4_I2S1B_SDI          ((uint32)(2))
#define IOMUX_GPIO0A4_EMMC_D1            ((uint32)(1))
#define IOMUX_GPIO0A4_IO                 ((uint32)(0))

#define IOMUX_GPIO0A5_I2C0C_SDA          ((uint32)(3))
#define IOMUX_GPIO0A5_SFC_D3             ((uint32)(2))
#define IOMUX_GPIO0A5_EMMC_D2            ((uint32)(1))
#define IOMUX_GPIO0A5_IO                 ((uint32)(0))

#define IOMUX_GPIO0A6_I2C0C_SCL          ((uint32)(3))
#define IOMUX_GPIO0A6_SFC_D2             ((uint32)(2))
#define IOMUX_GPIO0A6_EMMC_D3            ((uint32)(1))
#define IOMUX_GPIO0A6_IO                 ((uint32)(0))

#define IOMUX_GPIO0A7_RESERVE            ((uint32)(3))
#define IOMUX_GPIO0A7_SFC_D1             ((uint32)(2))
#define IOMUX_GPIO0A7_EMMC_D4            ((uint32)(1))
#define IOMUX_GPIO0A7_IO                 ((uint32)(0))

//--------------------------------------------------------------------------------

#define IOMUX_GPIO0B0_JTG1_TDI    ((uint32)(3))
#define IOMUX_GPIO0B0_SFC_D0      ((uint32)(2))
#define IOMUX_GPIO0B0_EMMC_D5     ((uint32)(1))
#define IOMUX_GPIO0B0_IO          ((uint32)(0))

#define IOMUX_GPIO0B1_JTG1_TDO    ((uint32)(3))
#define IOMUX_GPIO0B1_SFC_CLK     ((uint32)(2))
#define IOMUX_GPIO0B1_EMMC_D6     ((uint32)(1))
#define IOMUX_GPIO0B1_IO          ((uint32)(0))

#define IOMUX_GPIO0B2_JTG1_TRST   ((uint32)(3))
#define IOMUX_GPIO0B2_SFC_CS      ((uint32)(2))
#define IOMUX_GPIO0B2_EMMC_D7     ((uint32)(1))
#define IOMUX_GPIO0B2_IO          ((uint32)(0))

#define IOMUX_GPIO0B3_EBC_VCOM    ((uint32)(3))
#define IOMUX_GPIO0B3_I2S0_SDI    ((uint32)(2))
#define IOMUX_GPIO0B3_UART2A_RTS  ((uint32)(1))
#define IOMUX_GPIO0B3_IO          ((uint32)(0))

#define IOMUX_GPIO0B4_EBC_SDCLK   ((uint32)(3))
#define IOMUX_GPIO0B4_I2S0_SDO    ((uint32)(2))
#define IOMUX_GPIO0B4_UART2A_CTS  ((uint32)(1))
#define IOMUX_GPIO0B4_IO          ((uint32)(0))

#define IOMUX_GPIO0B5_EBC_GDCLK   ((uint32)(3))
#define IOMUX_GPIO0B5_I2S0_SCLK   ((uint32)(2))
#define IOMUX_GPIO0B5_UART2A_RX   ((uint32)(1))
#define IOMUX_GPIO0B5_IO          ((uint32)(0))

#define IOMUX_GPIO0B6_EBC_GDSP    ((uint32)(3))
#define IOMUX_GPIO0B6_I2S0_LRCK   ((uint32)(2))
#define IOMUX_GPIO0B6_UART2A_TX   ((uint32)(1))
#define IOMUX_GPIO0B6_IO          ((uint32)(0))

#define IOMUX_GPIO0B7_EBC_GDOE    ((uint32)(3))
#define IOMUX_GPIO0B7_I2S0_CLK    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_CSN     ((uint32)(1))
#define IOMUX_GPIO0B7_IO          ((uint32)(0))
//--------------------------------------------------------------------------------

#define IOMUX_GPIO0C0_EBC_SDDO0    ((uint32)(3))
#define IOMUX_GPIO0C0_SPI0A_TX     ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D0       ((uint32)(1))
#define IOMUX_GPIO0C0_IO           ((uint32)(0))

#define IOMUX_GPIO0C1_EBC_SDDO1    ((uint32)(3))
#define IOMUX_GPIO0C1_SPI0A_RX     ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D1       ((uint32)(1))
#define IOMUX_GPIO0C1_IO           ((uint32)(0))

#define IOMUX_GPIO0C2_EBC_SDDO2    ((uint32)(3))
#define IOMUX_GPIO0C2_SPI0A_CLK    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D2       ((uint32)(1))
#define IOMUX_GPIO0C2_IO           ((uint32)(0))

#define IOMUX_GPIO0C3_EBC_SDDO3    ((uint32)(3))
#define IOMUX_GPIO0C3_SPI0A_CS     ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D3       ((uint32)(1))
#define IOMUX_GPIO0C3_IO           ((uint32)(0))

#define IOMUX_GPIO0C4_EBC_SDDO4    ((uint32)(3))
#define IOMUX_GPIO0C4_UART2B_RX    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D4       ((uint32)(1))
#define IOMUX_GPIO0C4_IO           ((uint32)(0))

#define IOMUX_GPIO0C5_EBC_SDDO5    ((uint32)(3))
#define IOMUX_GPIO0C5_UART2B_TX    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D5       ((uint32)(1))
#define IOMUX_GPIO0C5_IO           ((uint32)(0))

#define IOMUX_GPIO0C6_EBC_SDDO6    ((uint32)(3))
#define IOMUX_GPIO0C6_UART2B_RTS   ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D6       ((uint32)(1))
#define IOMUX_GPIO0C6_IO           ((uint32)(0))

#define IOMUX_GPIO0C7_EBC_SDDO7    ((uint32)(3))
#define IOMUX_GPIO0C7_UART2B_CTS   ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_D7       ((uint32)(1))
#define IOMUX_GPIO0C7_IO           ((uint32)(0))
//--------------------------------------------------------------------------------

#define IOMUX_GPIO0D0_EBC_SDLE     ((uint32)(3))
#define IOMUX_GPIO0D0_I2C2B_SCL    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_WRN      ((uint32)(1))
#define IOMUX_GPIO0D0_IO           ((uint32)(0))

#define IOMUX_GPIO0D1_EBC_SDOE     ((uint32)(3))
#define IOMUX_GPIO0D1_I2C2B_SDA    ((uint32)(2))
#define IOMUX_GPIO0C0_VOP_RS       ((uint32)(1))
#define IOMUX_GPIO0D1_IO           ((uint32)(0))
//--------------------------------------------------------------------------------

#define IOMUX_GPIO1A0_RESERVE       ((uint32)(3))
#define IOMUX_GPIO1A0_EBC_GDRL      ((uint32)(2))
#define IOMUX_GPIO1A0_I2S1A_CLK     ((uint32)(1))
#define IOMUX_GPIO1A0_IO            ((uint32)(0))

#define IOMUX_GPIO1A1_EBC_SDSHR     ((uint32)(3))
#define IOMUX_GPIO1A1_UART1B_RX     ((uint32)(2))
#define IOMUX_GPIO1A1_I2S1A_LRCK    ((uint32)(1))
#define IOMUX_GPIO1A1_IO            ((uint32)(0))

#define IOMUX_GPIO1A2_EBC_SDCE2     ((uint32)(3))
#define IOMUX_GPIO1A2_UART1B_TX     ((uint32)(2))
#define IOMUX_GPIO1A2_I2S1A_SCLK    ((uint32)(1))
#define IOMUX_GPIO1A2_IO            ((uint32)(0))

#define IOMUX_GPIO1A3_EBC_SDCE1     ((uint32)(3))
#define IOMUX_GPIO1A3_UART1B_CTS    ((uint32)(2))
#define IOMUX_GPIO1A3_I2S1A_SDO     ((uint32)(1))
#define IOMUX_GPIO1A3_IO            ((uint32)(0))

#define IOMUX_GPIO1A4_EBC_SDCE4     ((uint32)(3))
#define IOMUX_GPIO1A4_UART1B_RTS    ((uint32)(2))
#define IOMUX_GPIO1A4_I2S1A_SDI     ((uint32)(1))
#define IOMUX_GPIO1A4_IO            ((uint32)(0))

#define IOMUX_GPIO1A5_UART3_TX      ((uint32)(3))
#define IOMUX_GPIO1A5_SPI1A_CS      ((uint32)(2))
#define IOMUX_GPIO1A5_SDMMC_CMD     ((uint32)(1))
#define IOMUX_GPIO1A5_IO            ((uint32)(0))

#define IOMUX_GPIO1A6_UART3_RX      ((uint32)(3))
#define IOMUX_GPIO1A6_SPI1A_CLK     ((uint32)(2))
#define IOMUX_GPIO1A6_SDMMC_CLK     ((uint32)(1))
#define IOMUX_GPIO1A6_IO            ((uint32)(0))

#define IOMUX_GPIO1A7_UART4_TX      ((uint32)(3))
#define IOMUX_GPIO1A7_SPI1A_RX      ((uint32)(2))
#define IOMUX_GPIO1A7_SDMMC_D0      ((uint32)(1))
#define IOMUX_GPIO1A7_IO            ((uint32)(0))

//--------------------------------------------------------------------------------
#define IOMUX_GPIO1B0_UART4_RX      ((uint32)(3))
#define IOMUX_GPIO1B0_SPI1A_TX      ((uint32)(2))
#define IOMUX_GPIO1B0_SDMMC_D1      ((uint32)(1))
#define IOMUX_GPIO1B0_IO            ((uint32)(0))

#define IOMUX_GPIO1B1_UART5_TX      ((uint32)(3))
#define IOMUX_GPIO1B1_I2C1B_SCL     ((uint32)(2))
#define IOMUX_GPIO1B1_SDMMC_D2      ((uint32)(1))
#define IOMUX_GPIO1B1_IO            ((uint32)(0))

#define IOMUX_GPIO1B2_UART5_RX      ((uint32)(3))
#define IOMUX_GPIO1B2_I2C1B_SDA     ((uint32)(2))
#define IOMUX_GPIO1B2_SDMMC_D3      ((uint32)(1))
#define IOMUX_GPIO1B2_IO            ((uint32)(0))

#define IOMUX_GPIO1B3_EMMC_RSTN     ((uint32)(1))
#define IOMUX_GPIO1B3_IO            ((uint32)(0))

#define IOMUX_GPIO1B4_IO            ((uint32)(0))

#define IOMUX_GPIO1B5_IO            ((uint32)(0))

#define IOMUX_GPIO1B6_IO            ((uint32)(0))

#define IOMUX_GPIO1B7_IO            ((uint32)(0))

//--------------------------------------------------------------------------------
#define IOMUX_GPIO1C0_IO            ((uint32)(0))

#define IOMUX_GPIO1C1_IO            ((uint32)(0))

#define IOMUX_GPIO1C2_IO            ((uint32)(0))

#define IOMUX_GPIO1C3_IO            ((uint32)(0))

#define IOMUX_GPIO1C4_IO            ((uint32)(0))

#define IOMUX_GPIO1C5_IO            ((uint32)(0))

#define IOMUX_GPIO1C6_IO            ((uint32)(0))

#define IOMUX_GPIO1C7_IO            ((uint32)(0))
//--------------------------------------------------------------------------------
#define IOMUX_GPIO1D0_IO            ((uint32)(0))

#define IOMUX_GPIO1D1_IO            ((uint32)(0))

#define IOMUX_GPIO1D2_IO            ((uint32)(0))

#define IOMUX_GPIO1D3_IO            ((uint32)(0))

#define IOMUX_GPIO1D4_IO            ((uint32)(0))

#define IOMUX_GPIO1D5_IO            ((uint32)(0))

#define IOMUX_GPIO1D6_IO            ((uint32)(0))

#define IOMUX_GPIO1D7_IO            ((uint32)(0))

//--------------------------------------------------------------------------------

#define IOMUX_GPIO2A0_EBC_GDPWR0     ((uint32)(3))
#define IOMUX_GPIO2A0_I2C2A_SDA      ((uint32)(2))
#define IOMUX_GPIO2A0_PWM4_OUT       ((uint32)(1))
#define IOMUX_GPIO2A0_IO             ((uint32)(0))

#define IOMUX_GPIO2A1_EBC_SDCE0      ((uint32)(3))
#define IOMUX_GPIO2A1_I2C2A_SCL      ((uint32)(2))
#define IOMUX_GPIO2A1_PWM3_OUT       ((uint32)(1))
#define IOMUX_GPIO2A1_IO           	 ((uint32)(0))

#define IOMUX_GPIO2A2_PMU_ST3        ((uint32)(3))
#define IOMUX_GPIO2A2_EBC_GDPWR2     ((uint32)(2))
#define IOMUX_GPIO2A2_PWM2_OUT       ((uint32)(1))
#define IOMUX_GPIO2A2_IO              ((uint32)(0))

#define IOMUX_GPIO2A3_EBC_GDPWR1     ((uint32)(3))
#define IOMUX_GPIO2A3_CLK_OBS        ((uint32)(2))
#define IOMUX_GPIO2A3_PWM1_OUT       ((uint32)(1))
#define IOMUX_GPIO2A3_IO             ((uint32)(0))

#define IOMUX_GPIO2A4_PMU_ST2        ((uint32)(3))
#define IOMUX_GPIO2A4_JTG0_TDI       ((uint32)(2))
#define IOMUX_GPIO2A4_PWM0_OUT       ((uint32)(1))
#define IOMUX_GPIO2A4_IO             ((uint32)(0))

#define IOMUX_GPIO2A5_PMU_ST1        ((uint32)(3))
#define IOMUX_GPIO2A5_JTG0_TDO       ((uint32)(2))
#define IOMUX_GPIO2A5_I2C2C_SCL      ((uint32)(1))
#define IOMUX_GPIO2A5_IO             ((uint32)(0))

#define IOMUX_GPIO2A6_PMU_ST0        ((uint32)(3))
#define IOMUX_GPIO2A6_JTG0_TRST      ((uint32)(2))
#define IOMUX_GPIO2A6_I2C2C_SDA      ((uint32)(1))
#define IOMUX_GPIO2A6_IO             ((uint32)(0))

#define IOMUX_GPIO2A7_PMU_IDEL       ((uint32)(1))
#define IOMUX_GPIO2A7_IO             ((uint32)(0))
//--------------------------------------------------------------------------------

#define IOMUX_GPIO2B0_EBC_SDCE3    ((uint32)(3))
#define IOMUX_GPIO2B0_SPI0B_CS     ((uint32)(2))
#define IOMUX_GPIO2B0_I2C1A_SDA    ((uint32)(1))
#define IOMUX_GPIO2B0_IO           ((uint32)(0))

#define IOMUX_GPIO2B1_EBC_BORDER1  ((uint32)(3))
#define IOMUX_GPIO2B1_SPI0B_CLK    ((uint32)(2))
#define IOMUX_GPIO2B1_I2C1A_SCL    ((uint32)(1))
#define IOMUX_GPIO2B1_IO           ((uint32)(0))

#define IOMUX_GPIO2B2_EBC_SDCE5    ((uint32)(3))
#define IOMUX_GPIO2B2_SPI0B_TX     ((uint32)(2))
#define IOMUX_GPIO2B2_I2C0A_SCL    ((uint32)(1))
#define IOMUX_GPIO2B2_IO           ((uint32)(0))

#define IOMUX_GPIO2B3_EBC_BORDER0  ((uint32)(3))
#define IOMUX_GPIO2B3_SPI0B_RX     ((uint32)(2))
#define IOMUX_GPIO2B3_I2C0A_SDA    ((uint32)(1))
#define IOMUX_GPIO2B3_IO           ((uint32)(0))

#define IOMUX_GPIO2B4_I2C1C_SDA    ((uint32)(3))
#define IOMUX_GPIO2B4_JTG1_TMS     ((uint32)(2))
#define IOMUX_GPIO2B4_UART0A_RX    ((uint32)(1))
#define IOMUX_GPIO2B4_IO           ((uint32)(0))

#define IOMUX_GPIO2B5_I2C1C_SCL    ((uint32)(3))
#define IOMUX_GPIO2B5_JTG1_TCK     ((uint32)(2))
#define IOMUX_GPIO2B5_UART0A_TX    ((uint32)(1))
#define IOMUX_GPIO2B5_IO           ((uint32)(0))

#define IOMUX_GPIO2B6_SPI1B_CS     ((uint32)(3))
#define IOMUX_GPIO2B6_JTG0_TMS     ((uint32)(2))
#define IOMUX_GPIO2B6_UART1A_RTS   ((uint32)(1))
#define IOMUX_GPIO2B6_IO           ((uint32)(0))

#define IOMUX_GPIO2B7_SPI1B_RX     ((uint32)(3))
#define IOMUX_GPIO2B7_JTG0_TCK     ((uint32)(2))
#define IOMUX_GPIO2B7_UART1A_CTS   ((uint32)(1))
#define IOMUX_GPIO2B7_IO           ((uint32)(0))
//--------------------------------------------------------------------------------
#define IOMUX_GPIO2C0_SPI1B_CLK    ((uint32)(3))
#define IOMUX_GPIO2C0_I2C0B_SDA    ((uint32)(2))
#define IOMUX_GPIO2C0_UART1A_TX    ((uint32)(1))
#define IOMUX_GPIO2C0_IO           ((uint32)(0))

#define IOMUX_GPIO2C1_SPI1B_TX     ((uint32)(3))
#define IOMUX_GPIO2C1_I2C0B_SCL    ((uint32)(2))
#define IOMUX_GPIO2C1_UART1A_RX    ((uint32)(1))
#define IOMUX_GPIO2C1_IO           ((uint32)(0))

#define IOMUX_GPIO2C2_IO           ((uint32)(0))

#define IOMUX_GPIO2C3_IO           ((uint32)(0))

#define IOMUX_GPIO2C4_IO           ((uint32)(0))

#define IOMUX_GPIO2C5_IO           ((uint32)(0))

#define IOMUX_GPIO2C6_IO           ((uint32)(0))

#define IOMUX_GPIO2C7_IO           ((uint32)(0))

//--------------------------------------------------------------------------------

#define IOMUX_GPIO2D0_IO           ((uint32)(0))

#define IOMUX_GPIO2D1_IO           ((uint32)(0))

#define IOMUX_GPIO2D2_IO           ((uint32)(0))

#define IOMUX_GPIO2D3_IO           ((uint32)(0))

#define IOMUX_GPIO2D4_IO           ((uint32)(0))

#define IOMUX_GPIO2D5_IO           ((uint32)(0))

#define IOMUX_GPIO2D6_IO           ((uint32)(0))

#define IOMUX_GPIO2D7_IO           ((uint32)(0))


//GRF_PVTM_CON0
#define PVTM_FUNC_START         ((uint32)(0x01) << 0)
#define PVTM_FUNC_OSC_EN        ((uint32)(0x01) << 1)
#define PVTM_FUNC_OSC_DIS       ((uint32)(0x00 ) << 1)
#define PVTM_DIV_CON            ((uint32)(0x7F) << 4)

//GRF_PVTM_CON1
#define PVTM_FUNC_CAL_CNT       ((uint32)(0xFFFFFFFF) << 0)


//GRF_PVTM_CON2
#define CLK_24M_DIV_CON         ((uint32)(0x1FF) << 0)


//GRF_PVTM_STATUS0
#define PVTM_FUNC_FREQ_DONE     ((uint32)(0x01) << 0)


//GRF_PVTM_STATUS1
#define PVTM_FUNC_FREQ_CNT       ((uint32)(0xFFFFFFFF))

//GRF_USBPHY_CON0
#define HS_EYE_DIAG_ADJUST_IN_SOF       ((uint32)1 << 0)
#define HS_EYE_DIAG_ADJUST_IN_CHIRP     ((uint32)2 << 0)
#define HS_EYE_DIAG_ADJUST_IN_NON_CHIRP ((uint32)4 << 0)
#define HS_EYE_DIAG_ADJUST_ALWAYS       ((uint32)7 << 0)
#define HS_EYE_DIAG_ADJUST_MASK         ((uint32)7 << 0)

#define FLS_EYE_HEIGHT_LARGEST          ((uint32)0 << 3)
#define FLS_EYE_HEIGHT_SMALLEST         ((uint32)3 << 3)
#define FLS_EYE_HEIGHT_MASK             ((uint32)3 << 3)

#define RX_CLK_PHASE_CON_EARLIEST       ((uint32)0 << 5)
#define RX_CLK_PHASE_CON_LATEST         ((uint32)7 << 5)
#define RX_CLK_PHASE_CON_MASK           ((uint32)7 << 5)

#define TX_CLK_PHASE_CON_EARLIEST       ((uint32)0 << 8)
#define TX_CLK_PHASE_CON_LATEST         ((uint32)7 << 8)
#define TX_CLK_PHASE_CON_MASK           ((uint32)7 << 8)

#define NON_DRIVING_CONTROL_EN          ((uint32)1 << 11)

#define SQUEL_TRIGGER_CON_1125MV        ((uint32)0x0 << 13)
#define SQUEL_TRIGGER_CON_1625MV        ((uint32)0x9 << 13)
#define SQUEL_TRIGGER_CON_1750MV        ((uint32)0xb << 13)
#define SQUEL_TRIGGER_CON_1500MV        ((uint32)0xc << 13) //default
#define SQUEL_TRIGGER_CON_1250MV        ((uint32)0xe << 13)

//GRF_USBPHY_CON1
//squel trigger con bit3
#define SQUEL_TRIGGER_CON_3BIT0         ((uint32)0 << 0)
#define SQUEL_TRIGGER_CON_3BIT1         ((uint32)1 << 0)
#define SQUEL_TRIGGER_CON_BIT3_MASK     ((uint32)1 << 0)

//bit2 - bit0 of hs_eye_height
#define HS_EYE_HEIGHT_LOW3BIT_LARGEST   ((uint32)0 << 13)
#define HS_EYE_HEIGHT_LOW3BIT_SMALLEST  ((uint32)7 << 13)
#define HS_EYE_HEIGHT_LOW3BIT_MASK      ((uint32)7 << 13)

//GRF_USBPHY_CON2
//bit7 - bit3 of hs_eye_height
#define HS_EYE_HEIGHT_HI5BIT_LARGEST   ((uint32)0 << 0)
#define HS_EYE_HEIGHT_HI5BIT_SMALLEST  ((uint32)0x1F << 0)
#define HS_EYE_HEIGHT_HI5BIT_MASK      ((uint32)0x1F << 0)

//hfs_driver_strength
#define HFS_DRIVER_STRENGTH_LARGEST     ((uint32)0x1F << 5)
#define HFS_DRIVER_STRENGTH_SMALLEST    ((uint32)0x10 << 5)
#define HFS_DRIVER_STRENGTH_MASK        ((uint32)0x1F << 5)

#define AUTO_COMPENSATION_BYPASS        ((uint32)3 << 10)
#define AUTO_COMPENSATION_BYPASS_MASK   ((uint32)3 << 10)

#define BG_VERF_ADJ_DEFAULT             ((uint32)0 << 12)
#define BG_VERF_ADJ                     ((uint32)1 << 12)
#define BG_VERF_ADJ_MASK                ((uint32)1 << 12)

#define BATTERY_CHARGE_RELATED_DEFAULT  ((uint32)0 << 13)
#define BATTERY_CHARGE_RELATED_MASK     ((uint32)3 << 13)

//bit0 of odt_compensation
#define ODT_COMPENSATION_0BIT0          ((uint32)0 << 15)
#define ODT_COMPENSATION_0BIT1          ((uint32)1 << 15)
#define ODT_COMPENSATION_0BIT_MASK      ((uint32)1 << 15)

//GRF_USBPHY_CON3
//bit2- bit1 of odt_compensation
#define ODT_COMPENSATION_2BIT00         ((uint32)0 << 0)
#define ODT_COMPENSATION_2BIT01         ((uint32)1 << 0)
#define ODT_COMPENSATION_2BIT11         ((uint32)3 << 0)
#define ODT_COMPENSATION_2BIT_MASK      ((uint32)3 << 0)

#define BIAS_CURRENT_REF_400MV          ((uint32)0 << 2)
#define BIAS_CURRENT_REF_362P5MV	    ((uint32)1 << 2)
#define BIAS_CURRENT_REF_350MV          ((uint32)2 << 2)
#define BIAS_CURRENT_REF_425MV          ((uint32)5 << 2)
#define BIAS_CURRENT_REF_450MV          ((uint32)7 << 2)
#define BIAS_CURRENT_REF_MASK           ((uint32)7 << 2)

#define COMPEN_CURRENT_REF_200MV        ((uint32)0 << 5)
#define COMPEN_CURRENT_REF_187P5MV      ((uint32)1 << 5)
#define COMPEN_CURRENT_REF_225MV        ((uint32)2 << 5)
#define COMPEN_CURRENT_REF_175MV        ((uint32)6 << 5)
#define COMPEN_CURRENT_REF_162P5MV      ((uint32)7 << 5)
#define COMPEN_CURRENT_REF_MASK         ((uint32)7 << 5)

#define BG_OUT_VOLTAGE_ADJ_DEFAULT      ((uint32)0 << 8)
#define BG_OUT_VOLTAGE_ADJ              ((uint32)1 << 8)
#define BG_OUT_VOLTAGE_ADJ_MASK         ((uint32)1 << 8)

//only be used when auto_compensation_bypass set '11'.
#define ODT_AUTO_REFRESH_BYPASS         ((uint32)0 << 9)
#define ODT_AUTO_REFRESH                ((uint32)1 << 9)
#define ODT_AUTO_REFRESH_MASK           ((uint32)1 << 9)

//only active when bit[65]is set "1"  ????
#define VOL_TOLERAN_DET_CON             ((uint32)1 << 14)
#define VOL_TOLERAN_DET_CON_MASK        ((uint32)1 << 14)


//GRF_USBPHY_CON4
//bypass 5v tolerance detection function ,active high
#define BYPASS_5V_TOLERANCE_DET         ((uint32)1 << 1)
#define BYPASS_5V_TOLERANCE_DET_MASK    ((uint32)1 << 1)

//GRF_USBPHY_CON5
//all reserve

//GRF_USBPHY_CON6

//turn off diff receiver, active low.
#define TURN_OFF_DIFF_RECEIVER          ((uint32)0 << 2)
#define TURN_ON_DIFF_RECEIVER           ((uint32)1 << 2)
#define TURN_OFF_DIFF_RECEIVER_MASK     ((uint32)1 << 2)

//force output A_sessionvalid asserted.active high
#define FORCE_A_SEESION_VAL             ((uint32)1 << 3)
#define FORCE_A_SEESION_VAL_MASK        ((uint32)1 << 3)

//force output B_sessionvalid asserted.active high
#define FORCE_B_SEESION_VAL             ((uint32)1 << 4)
#define FORCE_B_SEESION_VAL_MASK        ((uint32)1 << 4)

//force output session_end asserted.active high
#define FORCE_SEESION_END_VAL           ((uint32)1 << 5)
#define FORCE_SEESION_END_VAL_MASK      ((uint32)1 << 5)

//force output VBUS valid asserted.active high
#define FORCE_VBUD_VALID                ((uint32)1 << 6)
#define FORCE_VBUD_VALID_MASK           ((uint32)1 << 6)

//a_session_con
//#define A_SESSION_CON                   ((uint32)1 << 7)
#define A_SESSION_CON_MASK              ((uint32)7 << 7)

//b_session_con
//#define B_SESSION_CON                   ((uint32)1 << 10)
#define B_SESSION_CON_MASK              ((uint32)7 << 10)

//session_end_con
//#define SESSION_END_CON                 ((uint32)1 << 13)
#define SESSION_END_CON_MASK            ((uint32)7 << 13)


//GRF_USBPHY_CON7
//vbus valis reference turning
//#define VBUS_VALID_CON                ((uint32)0 << 0)
#define VBUS_VALID_CON_MASK             ((uint32)7 << 0)

//half bit pre-emphasize enable, active high.
#define FULL_BIT_PRE_EMPHA_EN           ((uint32)0 << 6)
#define HALF_BIT_PRE_EMPHA_EN           ((uint32)1 << 6)
#define HALF_BIT_PRE_EMPHA_EN_MASK      ((uint32)1 << 6)

//bypass squelch trigger point auto configure in chirp modes ,active high
#define BYPASS_SQUELCH_TRIGGER          ((uint32)1 << 7)
#define BYPASS_SQUELCH_TRIGGER_MASK     ((uint32)1 << 7)

//HOST disconnect detection trigger point configure, only used inHOST mode
#define HOST_DISCON_CON_575MV           ((uint32)0 << 11)
#define HOST_DISCON_CON_600MV           ((uint32)1 << 11)
#define HOST_DISCON_CON_625MV           ((uint32)9 << 11)
#define HOST_DISCON_CON_650MV           ((uint32)13 << 11)  //default
#define HOST_DISCON_CON_MASK            ((uint32)15 << 11)

//vbus voltage level detection function power down, active high.
#define VBUS_VOL_DET_ON                 ((uint32)0 << 15)
#define VBUS_VOL_DET_OFF                ((uint32)1 << 15)
#define VBUS_VOL_DET_MASK               ((uint32)1 << 15)

//GRF_USBPHY_CON8
#define BATTERY_CHARGING_RELATED        ((uint32)0 << 0)
#define BATTERY_CHARGING_RELATED_MASK   ((uint32)0x1f << 0)

//TX HS pre_emphasize strength configure
#define TX_HS_PRE_EMPHASIZE_STR_STRONGEST   ((uint32)7  << 5)
#define TX_HS_PRE_EMPHASIZE_STR_WEAKEST     ((uint32)0  << 5)
#define TX_HS_PRE_EMPHASIZE_STR_MASK        ((uint32)7  << 5)

//Tx HS driver strength configure
#define TX_HS_DRIVER_STR_STRONGEST   ((uint32)7  << 8)
#define TX_HS_DRIVER_STR_WEAKEST     ((uint32)0  << 8)
#define TX_HS_DRIVER_STR_MASK        ((uint32)7  << 8)

//GRF_USBPHY_CON9
//all reserve

//GRF_USBPHY_CON10
//all reserve

//GRF_USBPHY_CON11
//all reserve

//2 GRF_UOC_CON0
#define OTG_DISABLE0_BYPASS   	        ((uint32)(1) << 0)
#define BYPASSSDMEN0_BYPASS   	        ((uint32)(1) << 1)
#define BYPASSSEL0_BYPASS   	        ((uint32)(1) << 2)
#define OTG_DISABLE0_NOBYPASS   	    ((uint32)(0) << 0)
#define BYPASSSDMEN0_NOBYPASS   	    ((uint32)(0) << 1)
#define BYPASSSEL0_NOBYPASS   	        ((uint32)(0) << 2)
#define USBOTGCON_SEL_MASK   	        ((uint32)(7) << 0)

//2 GRF_UOC_CON1
//data path selector of usb controller
#define USB_CTRL_NON_SEL                ((uint32)(0) << 0)
#define USB_CTRL_SEL                    ((uint32)(1) << 0)
#define USB_CTRL_SEL_MASK               ((uint32)(1) << 0)

#define UTMI_SUSPEND_N_SUSPEND          ((uint32)(0) << 1)
#define UTMI_SUSPEND_N_NORMAL           ((uint32)(1) << 1)
#define UTMI_SUSPEND_N_MASK             ((uint32)(1) << 1)

#define UTMI_OPMODE_1                   ((uint32)(0) << 2)
#define UTMI_OPMODE_2                   ((uint32)(1) << 2)
#define UTMI_OPMODE_3                   ((uint32)(2) << 2)
#define UTMI_OPMODE_4                   ((uint32)(3) << 2)
#define UTMI_OPMODE_MASK                ((uint32)(3) << 2)

#define UTMI_XCVRSELECT
#define UTMI_XCVRSELECT_MASK            ((uint32)(3) << 4)

#define UTMI_TERMSELECT                 ((uint32)(1) << 6)
#define UTMI_TERMSELECT_MASK            ((uint32)(1) << 6)

#define UTMIOTG_IDDIG_SEL_OTGIDDIG      ((uint32)(1) << 9)
#define UTMIOTG_IDDIG_SEL_OTGPHY        ((uint32)(0) << 9)
#define UTMIOTG_IDDIG_SEL_MASK          ((uint32)(1) << 9)

//usb plug indicator output
#define UTMIOTG_IDDIG                   ((uint32)(1) << 10)
#define UTMIOTG_IDDIG_MASK              ((uint32)(1) << 10)

//2 GRF_UOC_CON2
//configure PLL clock output in suspend mode
#define USBPHY_COMMONON_PLL_RESUME      ((uint32)(0) << 0)
#define USBPHY_COMMONON_PLL_SUSPEND     ((uint32)(1) << 0)
#define USBPHY_COMMONON_MASK            ((uint32)(1) << 0)

//enable DPLUS Pull down resister
#define UTMI_DPPULLDOWN_ENABLE          ((uint32)(1) << 1)
#define UTMI_DPPULLDOWN_DISABLE         ((uint32)(0) << 1)
#define UTMI_DPPULLDOWN_MASK            ((uint32)(1) << 1)

//Enable DMINUS Pull Down resistor
#define UTMI_DMPULLDOWN_ENABLE          ((uint32)(1) << 2)
#define UTMI_DMPULLDOWN_DISABLE         ((uint32)(0) << 2)
#define UTMI_DMPULLDOWN_MASK            ((uint32)(1) << 2)

//signal that enable analog ID line sampling
#define UTMI_IDPULLUP_ENABLE            ((uint32)(1) << 3)
#define UTMI_IDPULLUP_DISABLE           ((uint32)(0) << 3)
#define UTMI_IDPULLUP_MASK              ((uint32)(1) << 3)

//connect to INNO_USB_PHY
#define UTMI_DRVVBUS                    ((uint32)(1) << 4)
#define UTMI_DRVVBUS_MASK               ((uint32)(1) << 4)

//VBUS charging enable
#define UTMI_CHRGBUS_ENABLE             ((uint32)(1) << 5)
#define UTMI_CHRGBUS_DISABLE            ((uint32)(0) << 5)
#define UTMI_CHRGBUS_MASK               ((uint32)(1) << 5)

//VBUS discharging enable
#define UTMI_DISCHRGBUS_ENABLE          ((uint32)(0) << 6)
#define UTMI_DISCHRGBUS_DISABLE         ((uint32)(1) << 6)
#define UTMI_DISCHRGBUS_MASK            ((uint32)(1) << 6)


//2 GRF_IOMUX_CON
#define I2C0_SEL_A          ((uint32)(0) << 0)
#define I2C0_SEL_B          ((uint32)(1) << 0)
#define I2C0_SEL_C          ((uint32)(2) << 0)
#define I2C0_SEL_RESERVE    ((uint32)(3) << 0)
#define I2C0_SEL_MASK   	((uint32)(3) << 0)

#define I2C1_SEL_A          ((uint32)(0) << 2)
#define I2C1_SEL_B          ((uint32)(1) << 2)
#define I2C1_SEL_C          ((uint32)(2) << 2)
#define I2C1_SEL_RESERVE    ((uint32)(3) << 2)
#define I2C1_SEL_MASK   	((uint32)(3) << 2)

#define I2C2_SEL_A          ((uint32)(0) << 4)
#define I2C2_SEL_B          ((uint32)(1) << 4)
#define I2C2_SEL_C          ((uint32)(2) << 4)
#define I2C2_SEL_RESERVE    ((uint32)(3) << 4)
#define I2C2_SEL_MASK   	((uint32)(3) << 4)

#define SPI0_SEL_A          ((uint32)(0) << 6)
#define SPI0_SEL_B          ((uint32)(1) << 6)
#define SPI0_SEL_MASK       ((uint32)(1) << 6)

#define SPI1_SEL_A          ((uint32)(0) << 7)
#define SPI1_SEL_B          ((uint32)(1) << 7)
#define SPI1_SEL_MASK       ((uint32)(1) << 7)

#define UART1_SEL_A          ((uint32)(0) << 9)
#define UART1_SEL_B          ((uint32)(1) << 9)
#define UART1_SEL_MASK       ((uint32)(1) << 9)

#define UART2_SEL_A          ((uint32)(0) << 10)
#define UART2_SEL_B          ((uint32)(1) << 10)
#define UART2_SEL_C          ((uint32)(2) << 10)
#define UART2_SEL_MASK       ((uint32)(3) << 10)

#define I2S0_SEL_PAD          ((uint32)(0) << 12)
#define I2S0_SEL_ACODEC       ((uint32)(1) << 12)
#define I2S0_SEL_MASK         ((uint32)(1) << 12)

//2 GRF_INTER_CON
// high dram
#define HDRAM_RESP_CYCLE_EN        ((uint32)(0x01) << 15)
#define HDRAM_RESP_CYCLE_DIS       ((uint32)(0x00) << 15)
#define HDRAM_RESP_CYCLE_MASK      ((uint32)(0x01) << 15)
// high iram
#define HIRAM_RESP_CYCLE_EN        ((uint32)(0x01) << 14)
#define HIRAM_RESP_CYCLE_DIS       ((uint32)(0x00) << 14)
#define HIRAM_RESP_CYCLE_MASK      ((uint32)(0x01) << 14)

// low dram
#define LDRAM_RESP_CYCLE_EN        ((uint32)(0x01) << 13)
#define LDRAM_RESP_CYCLE_DIS       ((uint32)(0x00) << 13)
#define LDRAM_RESP_CYCLE_MASK      ((uint32)(0x01) << 13)

// low iram
#define LIRAM_RESP_CYCLE_EN        ((uint32)(0x01) << 12)
#define LIRAM_RESP_CYCLE_DIS       ((uint32)(0x00) << 12)
#define LIRAM_RESP_CYCLE_MASK      ((uint32)(0x01) << 12)

#define NOC_REMAP_BOOT_ROM         ((uint32)(0x00) << 8) //remap boot rom to 00000000
#define NOC_REMAP_PMU_SRAM         ((uint32)(0x01) << 8) //remap pmu sram to 00000000
#define NOC_REMAP_MASK         	   ((uint32)(0x01) << 8)

#define PMURAM_RESP_CYCLE_DIS      ((uint32)(0x00) << 6)
#define PMURAM_RESP_CYCLE_EN       ((uint32)(0x01) << 6)

#define BTROM_RESP_CYCLE_DIS       ((uint32)(0x00) << 5)
#define BTROM_RESP_CYCLE_EN        ((uint32)(0x01) << 5)

#define DMA_UART2_SEL              ((uint32)(0x01) << 1)
#define DMA_UART0_SEL              ((uint32)(0x00) << 1)
#define DMA_UART_MASK              ((uint32)(1) << 1)

#define FORCE_JTAG_EN              ((uint32)(0x01) << 0)
#define FORCE_JTAG_DIS             ((uint32)(0X00) << 0)
#define FORCE_JTAG_MASK            ((uint32)(0x01) << 0)

//2 GRF_GRF_VREF_CON
/*When high the ADC REF is power down*/
#define GRF_VREF_PWD_MASK        ((uint32)(1) << 0)
#define GRF_VREF_TRIM_MASK		 ((uint32)(7) << 1)

/*Select the VBG voltage which should has the best temperature characteristics*/
#define GRF_VREG_VBG_SEL_SAMLLEST    ((uint32)(0) << 4)
#define GRF_VREG_VBG_SEL_LARGEEST    ((uint32)(7) << 4)
#define GRF_VREG_VBG_SEL_MASK        ((uint32)(7) << 4)

//GRF_SOC_STATUS0
#define SYSCORE_SLEEPING         ((uint32)(1) << 0) //read only
#define SYSCORE_SLEEPDEEP        ((uint32)(1) << 1)
#define SYSCORE_LOCKUP           ((uint32)(1) << 2)
#define SYSCORE_HALTED           ((uint32)(1) << 3)

//GRF_SOC_STATUS1
#define CALCORE_SLEEPING         ((uint32)(1) << 0) //read only
#define CALCORE_SLEEPDEEP        ((uint32)(1) << 1)
#define CALCORE_LOCKUP           ((uint32)(1) << 2)
#define CALCORE_HALTED           ((uint32)(1) << 3)

//GRF_SOC_USB_STATUS
#define UTMIOTG_VBUS_VALID       ((uint32)(1) << 0) //read only
#define UTMISRP_BSESS_VALID      ((uint32)(1) << 1)
#define UTMIOTG_ASESS_VALID      ((uint32)(1) << 2)
#define UTMIOTG_SESSEND_VALID    ((uint32)(1) << 3)

//GRF_PRJ_ID
//read only


//GRF_CPU_ID
#define SYS_CPU           ((uint32)(0) << 0)   //read only
#define CAL_CPU            ((uint32)(1) << 0)   //read only

/*
********************************************************************************
*
*                         End of Hw_grf.h
*
********************************************************************************
*/

#endif

