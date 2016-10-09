/*
********************************************************************************
*                   Copyright (c) 2008,Huweiguo
*                         All rights reserved.
*
* File Name£º   Hw_memap.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_MEMAP_H_
#define _HW_MEMAP_H_

#define ADC_BASE            ((UINT32)0x400d0000)
#define EFUSE_BASE          ((UINT32)0x40090000)
#define RTC_BASE            ((UINT32)0x400b0000)
#define NANDC_BASE          ((UINT32)0x60080000)

#define HIFIACC_BASE        ((UINT32)0x01060000)

//#define CRU_BASE            ((UINT32)0x40010000)
#define I2S0_BASE           ((UINT32)0x40020000)
#define I2S1_BASE           ((UINT32)0x40030000)
#define I2C0_BASE           ((UINT32)0x40040000)
#define I2C1_BASE           ((UINT32)0x40050000)
#define I2C2_BASE           ((UINT32)0x40060000)
#define SPI0_BASE           ((UINT32)0x40070000)
#define SPI1_BASE           ((UINT32)0x40080000)
#define ACODEC_BASE         ((UINT32)0x40090000)
#define UART0_BASE          ((UINT32)0x400a0000)
#define UART1_BASE          ((UINT32)0x400b0000)
#define UART2_BASE          ((UINT32)0x400c0000)
#define SARADC_BASE         ((UINT32)0x400d0000)
#define TIMER_BASE          ((UINT32)0x400e0000)
#define PWM0_BASE           ((UINT32)0x400f0000)
#define PWM1_BASE           ((UINT32)0x40100000)
#define MAILBOX_BASE        ((UINT32)0x40110000)
#define WDT_BASE            ((UINT32)0x40120000)
#define UART3_BASE          ((UINT32)0x40130000)
#define UART4_BASE          ((UINT32)0x40140000)
#define UART5_BASE          ((UINT32)0x40150000)

#define GPIO0_BASE          ((UINT32)0x40160000)
#define GPIO1_BASE          ((UINT32)0x40170000)
#define CRU_BASE            ((UINT32)0x40180000)

#define GRF_BASE            ((UINT32)0x50010000)
#define PMU_BASE            ((UINT32)0x50020000)
#define GPIO2_BASE          ((UINT32)0x50030000)
//#define GPIO0_BASE          ((UINT32)0x50030000)
//#define GPIO1_BASE          ((UINT32)0x50040000)

#define DMA_BASE            ((UINT32)0x60000000)
#define DMA_BASE2           ((UINT32)0x01070000)
#define IMDCT_BASE          ((UINT32)0x60010000)
#define SDMMC_BASE          ((UINT32)0x60020000)
#define EMMC_BASE           ((UINT32)0x60030000)
#define EBC_BASE            ((UINT32)0x60040000)
#define SYNTH_BASE          ((UINT32)0x60050000)
#define SFC_BASE            ((UINT32)0x60060000)
#define VOP_BASE            ((UINT32)0x60070000)
#define USB_BASE            ((UINT32)0x60080000)

#define SDC0_ADDR           ((UINT32)0x60020000)
#define SDC0_FIFO_ADDR      (SDC0_ADDR + 0x200)
#define EMMC_ADDR		    ((UINT32)0x60030000)
#define EMMC_FIFO_ADDR      (EMMC_ADDR + 0x200)


/*
********************************************************************************
*
*                         End of Hw_memap.h
*
********************************************************************************
*/

#endif
