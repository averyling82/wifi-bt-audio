/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "BspConfig.h"



typedef wl_cnt_ver_six_t  wiced_counters_t;


#ifdef _WIFI_AP6181

#ifndef INCLUDED_CHIP_CONSTANTS_H_
#define INCLUDED_CHIP_CONSTANTS_H_

#include "wwd_wlioctl.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *             Architecture Constants
 ******************************************************/

/* General chip stats */
#define CHIP_RAM_SIZE      0x3C000

/* Backplane architecture */
#define CHIPCOMMON_BASE_ADDRESS  0x18000000    /* Chipcommon core register region   */
#define DOT11MAC_BASE_ADDRESS    0x18001000    /* dot11mac core register region     */
#define SDIO_BASE_ADDRESS        0x18002000    /* SDIOD Device core register region */
#define WLAN_ARMCM3_BASE_ADDRESS 0x18003000    /* ARMCM3 core register region       */
#define SOCSRAM_BASE_ADDRESS     0x18004000    /* SOCSRAM core register region      */
#define BACKPLANE_ADDRESS_MASK   0x7FFF

#define WLAN_MEM_BASE_ADDRESS   0x00

#define CHIP_STA_INTERFACE   0
#define CHIP_AP_INTERFACE    1
#define CHIP_P2P_INTERFACE   2

/* Maximum value of bus data credit difference */
#define CHIP_MAX_BUS_DATA_CREDIT_DIFF    20

/* Chipcommon registers */
#define CHIPCOMMON_GPIO_CONTROL ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x6C) )

/******************************************************
 *             SDIO Constants
 ******************************************************/
/* CurrentSdiodProgGuide r23 */

/* Base registers */
#define SDIO_CORE                    ((uint32_t) (SDIO_BASE_ADDRESS + 0x00) )
#define SDIO_INT_STATUS              ((uint32_t) (SDIO_BASE_ADDRESS + 0x20) )
#define SDIO_TO_SB_MAILBOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_TO_SB_MAILBOX_DATA      ((uint32_t) (SDIO_BASE_ADDRESS + 0x48) )
#define SDIO_TO_HOST_MAILBOX_DATA    ((uint32_t) (SDIO_BASE_ADDRESS + 0x4C) )
#define SDIO_TO_SB_MAIL_BOX          ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_INT_HOST_MASK           ((uint32_t) (SDIO_BASE_ADDRESS + 0x24) )
#define SDIO_FUNCTION_INT_MASK       ((uint32_t) (SDIO_BASE_ADDRESS + 0x34) )

/* SDIO Function 0 (SDIO Bus) register addresses */

/* SDIO Device CCCR offsets */
/* TODO: What does CIS/CCCR stand for? */
/* CCCR accesses do not require backpane clock */
#define SDIOD_CCCR_REV             ( (uint32_t)  0x00 )    /* CCCR/SDIO Revision */
#define SDIOD_CCCR_SDREV           ( (uint32_t)  0x01 )    /* SD Revision */
#define SDIOD_CCCR_IOEN            ( (uint32_t)  0x02 )    /* I/O Enable */
#define SDIOD_CCCR_IORDY           ( (uint32_t)  0x03 )    /* I/O Ready */
#define SDIOD_CCCR_INTEN           ( (uint32_t)  0x04 )    /* Interrupt Enable */
#define SDIOD_CCCR_INTPEND         ( (uint32_t)  0x05 )    /* Interrupt Pending */
#define SDIOD_CCCR_IOABORT         ( (uint32_t)  0x06 )    /* I/O Abort */
#define SDIOD_CCCR_BICTRL          ( (uint32_t)  0x07 )    /* Bus Interface control */
#define SDIOD_CCCR_CAPABLITIES     ( (uint32_t)  0x08 )    /* Card Capabilities */
#define SDIOD_CCCR_CISPTR_0        ( (uint32_t)  0x09 )    /* Common CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_CISPTR_1        ( (uint32_t)  0x0A )    /* Common CIS Base Address Pointer Register 1 */
#define SDIOD_CCCR_CISPTR_2        ( (uint32_t)  0x0B )    /* Common CIS Base Address Pointer Register 2 (MSB - only bit 1 valid)*/
#define SDIOD_CCCR_BUSSUSP         ( (uint32_t)  0x0C )    /*  */
#define SDIOD_CCCR_FUNCSEL         ( (uint32_t)  0x0D )    /*  */
#define SDIOD_CCCR_EXECFLAGS       ( (uint32_t)  0x0E )    /*  */
#define SDIOD_CCCR_RDYFLAGS        ( (uint32_t)  0x0F )    /*  */
#define SDIOD_CCCR_BLKSIZE_0       ( (uint32_t)  0x10 )    /* Function 0 (Bus) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_BLKSIZE_1       ( (uint32_t)  0x11 )    /* Function 0 (Bus) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_POWER_CONTROL   ( (uint32_t)  0x12 )    /* Power Control */
#define SDIOD_CCCR_SPEED_CONTROL   ( (uint32_t)  0x13 )    /* Bus Speed Select  (control device entry into high-speed clocking mode)  */
#define SDIOD_CCCR_UHS_I           ( (uint32_t)  0x14 )    /* UHS-I Support */
#define SDIOD_CCCR_DRIVE           ( (uint32_t)  0x15 )    /* Drive Strength */
#define SDIOD_CCCR_INTEXT          ( (uint32_t)  0x16 )    /* Interrupt Extension */
#define SDIOD_SEP_INT_CTL          ( (uint32_t)  0xF2 )    /* Separate Interrupt Control*/
#define SDIOD_CCCR_F1INFO          ( (uint32_t) 0x100 )    /* Function 1 (Backplane) Info */
#define SDIOD_CCCR_F1HP            ( (uint32_t) 0x102 )    /* Function 1 (Backplane) High Power */
#define SDIOD_CCCR_F1CISPTR_0      ( (uint32_t) 0x109 )    /* Function 1 (Backplane) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F1CISPTR_1      ( (uint32_t) 0x10A )    /* Function 1 (Backplane) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F1CISPTR_2      ( (uint32_t) 0x10B )    /* Function 1 (Backplane) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F1BLKSIZE_0     ( (uint32_t) 0x110 )    /* Function 1 (Backplane) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F1BLKSIZE_1     ( (uint32_t) 0x111 )    /* Function 1 (Backplane) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F2INFO          ( (uint32_t) 0x200 )    /* Function 2 (WLAN Data FIFO) Info */
#define SDIOD_CCCR_F2HP            ( (uint32_t) 0x202 )    /* Function 2 (WLAN Data FIFO) High Power */
#define SDIOD_CCCR_F2CISPTR_0      ( (uint32_t) 0x209 )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F2CISPTR_1      ( (uint32_t) 0x20A )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F2CISPTR_2      ( (uint32_t) 0x20B )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F2BLKSIZE_0     ( (uint32_t) 0x210 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F2BLKSIZE_1     ( (uint32_t) 0x211 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F3INFO          ( (uint32_t) 0x300 )    /* Function 3 (Bluetooth Data FIFO) Info */
#define SDIOD_CCCR_F3HP            ( (uint32_t) 0x302 )    /* Function 3 (Bluetooth Data FIFO) High Power */
#define SDIOD_CCCR_F3CISPTR_0      ( (uint32_t) 0x309 )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F3CISPTR_1      ( (uint32_t) 0x30A )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F3CISPTR_2      ( (uint32_t) 0x30B )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F3BLKSIZE_0     ( (uint32_t) 0x310 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F3BLKSIZE_1     ( (uint32_t) 0x311 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 1 (MSB) */


/* SDIO Function 1 (Backplane) register addresses */
/* Addresses 0x00000000 - 0x0000FFFF are directly access the backplane
 * throught the backplane window. Addresses above 0x0000FFFF are
 * registers relating to backplane access, and do not require a backpane
 * clock to access them
 */
#define SDIO_GPIO_SELECT              ( (uint32_t) 0x10005 )
#define SDIO_GPIO_OUTPUT              ( (uint32_t) 0x10006 )
#define SDIO_GPIO_ENABLE              ( (uint32_t) 0x10007 )
#define SDIO_FUNCTION2_WATERMARK      ( (uint32_t) 0x10008 )
#define SDIO_DEVICE_CONTROL           ( (uint32_t) 0x10009 )
#define SDIO_BACKPLANE_ADDRESS_LOW    ( (uint32_t) 0x1000A )
#define SDIO_BACKPLANE_ADDRESS_MID    ( (uint32_t) 0x1000B )
#define SDIO_BACKPLANE_ADDRESS_HIGH   ( (uint32_t) 0x1000C )
#define SDIO_FRAME_CONTROL            ( (uint32_t) 0x1000D )
#define SDIO_CHIP_CLOCK_CSR           ( (uint32_t) 0x1000E )
#define SDIO_PULL_UP                  ( (uint32_t) 0x1000F )
#define SDIO_READ_FRAME_BC_LOW        ( (uint32_t) 0x1001B )
#define SDIO_READ_FRAME_BC_HIGH       ( (uint32_t) 0x1001C )

/* Micky ++ */
#define SDIO_SB_ACCESS_2_4B_FLAG    0x08000        /* with b15, maps to 32-bit SB access */

#define I_HMB_SW_MASK                 ( (uint32_t) 0x000000F0 )
#define I_HMB_FRAME_IND               ( 1<<6 )
#define FRAME_AVAILABLE_MASK          I_HMB_SW_MASK


/******************************************************
 *             SPI Constants
 ******************************************************/
/* GSPI v1 */
#define SPI_FRAME_CONTROL              ( (uint32_t) 0x1000D )

/* Register addresses */
#define SPI_BUS_CONTROL                ( (uint32_t) 0x0000 )
#define SPI_RESPONSE_DELAY             ( (uint32_t) 0x0001 )
#define SPI_STATUS_ENABLE              ( (uint32_t) 0x0002 )
#define SPI_RESET_BP                   ( (uint32_t) 0x0003 )    /* (corerev >= 1) */
#define SPI_INTERRUPT_REGISTER         ( (uint32_t) 0x0004 )    /* 16 bits - Interrupt status */
#define SPI_INTERRUPT_ENABLE_REGISTER  ( (uint32_t) 0x0006 )    /* 16 bits - Interrupt mask */
#define SPI_STATUS_REGISTER            ( (uint32_t) 0x0008 )    /* 32 bits */
#define SPI_FUNCTION1_INFO             ( (uint32_t) 0x000C )    /* 16 bits */
#define SPI_FUNCTION2_INFO             ( (uint32_t) 0x000E )    /* 16 bits */
#define SPI_FUNCTION3_INFO             ( (uint32_t) 0x0010 )    /* 16 bits */
#define SPI_READ_TEST_REGISTER         ( (uint32_t) 0x0014 )    /* 32 bits */
#define SPI_RESP_DELAY_F0              ( (uint32_t) 0x001c )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F1              ( (uint32_t) 0x001d )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F2              ( (uint32_t) 0x001e )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F3              ( (uint32_t) 0x001f )    /* 8 bits (corerev >= 3) */

/******************************************************
 *             Bit Masks
 ******************************************************/

/* SDIOD_CCCR_REV Bits */
#define SDIO_REV_SDIOID_MASK       ( (uint32_t)  0xF0 )    /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK       ( (uint32_t)  0x0F )    /* CCCR format version number */

/* SDIOD_CCCR_SDREV Bits */
#define SD_REV_PHY_MASK            ( (uint32_t)  0x0F )    /* SD format version number */

/* SDIOD_CCCR_IOEN Bits */
#define SDIO_FUNC_ENABLE_1         ( (uint32_t)  0x02 )    /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2         ( (uint32_t)  0x04 )    /* function 2 I/O enable */
#define SDIO_FUNC_ENABLE_3         ( (uint32_t)  0x08 )    /* function 3 I/O enable */

/* SDIOD_CCCR_IORDY Bits */
#define SDIO_FUNC_READY_1          ( (uint32_t)  0x02 )    /* function 1 I/O ready */
#define SDIO_FUNC_READY_2          ( (uint32_t)  0x04 )    /* function 2 I/O ready */
#define SDIO_FUNC_READY_3          ( (uint32_t)  0x08 )    /* function 3 I/O ready */

/* SDIOD_CCCR_INTEN Bits */
#define INTR_CTL_MASTER_EN         ( (uint32_t)  0x01 )    /* interrupt enable master */
#define INTR_CTL_FUNC1_EN          ( (uint32_t)  0x02 )    /* interrupt enable for function 1 */
#define INTR_CTL_FUNC2_EN          ( (uint32_t)  0x04 )    /* interrupt enable for function 2 */

/* SDIOD_SEP_INT_CTL Bits */
#define SEP_INTR_CTL_MASK          ( (uint32_t)  0x01 )    /* out-of-band interrupt mask */
#define SEP_INTR_CTL_EN            ( (uint32_t)  0x02 )    /* out-of-band interrupt output enable */
#define SEP_INTR_CTL_POL           ( (uint32_t)  0x04 )    /* out-of-band interrupt polarity */

/* SDIOD_CCCR_INTPEND Bits */
#define INTR_STATUS_FUNC1          ( (uint32_t)  0x02 )    /* interrupt pending for function 1 */
#define INTR_STATUS_FUNC2          ( (uint32_t)  0x04 )    /* interrupt pending for function 2 */
#define INTR_STATUS_FUNC3          ( (uint32_t)  0x08 )    /* interrupt pending for function 3 */

/* SDIOD_CCCR_IOABORT Bits */
#define IO_ABORT_RESET_ALL         ( (uint32_t)  0x08 )    /* I/O card reset */
#define IO_ABORT_FUNC_MASK         ( (uint32_t)  0x07 )    /* abort selction: function x */

/* SDIOD_CCCR_BICTRL Bits */
#define BUS_CARD_DETECT_DIS        ( (uint32_t)  0x80 )    /* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP      ( (uint32_t)  0x40 )    /* support continuous SPI interrupt */
#define BUS_SPI_CONT_INTR_EN       ( (uint32_t)  0x20 )    /* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK     ( (uint32_t)  0x03 )    /* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT     ( (uint32_t)  0x02 )    /* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT     ( (uint32_t)  0x00 )    /* bus width 1-bit mode */

/* SDIOD_CCCR_CAPABLITIES Bits */
#define SDIO_CAP_4BLS              ( (uint32_t)  0x80 )    /* 4-bit support for low speed card */
#define SDIO_CAP_LSC               ( (uint32_t)  0x40 )    /* low speed card */
#define SDIO_CAP_E4MI              ( (uint32_t)  0x20 )    /* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI              ( (uint32_t)  0x10 )    /* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS               ( (uint32_t)  0x08 )    /* support suspend/resume */
#define SDIO_CAP_SRW               ( (uint32_t)  0x04 )    /* support read wait */
#define SDIO_CAP_SMB               ( (uint32_t)  0x02 )    /* support multi-block transfer */
#define SDIO_CAP_SDC               ( (uint32_t)  0x01 )    /* Support Direct commands during multi-byte transfer */

/* SDIOD_CCCR_POWER_CONTROL Bits */
#define SDIO_POWER_SMPC            ( (uint32_t)  0x01 )    /* supports master power control (RO) */
#define SDIO_POWER_EMPC            ( (uint32_t)  0x02 )    /* enable master power control (allow > 200mA) (RW) */

/* SDIOD_CCCR_SPEED_CONTROL Bits */
#define SDIO_SPEED_SHS             ( (uint32_t)  0x01 )    /* supports high-speed [clocking] mode (RO) */
#define SDIO_SPEED_EHS             ( (uint32_t)  0x02 )    /* enable high-speed [clocking] mode (RW) */



/* GSPI */
#define SPI_READ_TEST_REGISTER_VALUE   ( (uint32_t) 0xFEEDBEAD )
#define SPI_READ_TEST_REG_LSB          ( ( ( SPI_READ_TEST_REGISTER_VALUE      )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT1     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT2     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 ) + 1 ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT3     ( ( ( SPI_READ_TEST_REGISTER_VALUE +1  ) << 1 ) & 0xff )


/* SPI_BUS_CONTROL Bits */
#define WORD_LENGTH_32             ( (uint32_t)  0x01 )    /* 0/1 16/32 bit word length */
#define ENDIAN_BIG                 ( (uint32_t)  0x02 )    /* 0/1 Little/Big Endian */
#define CLOCK_PHASE                ( (uint32_t)  0x04 )    /* 0/1 clock phase delay */
#define CLOCK_POLARITY             ( (uint32_t)  0x08 )    /* 0/1 Idle state clock polarity is low/high */
#define HIGH_SPEED_MODE            ( (uint32_t)  0x10 )    /* 1/0 High Speed mode / Normal mode */
#define INTR_POLARITY_HIGH         ( (uint32_t)  0x20 )    /* 1/0 Interrupt active polarity is high/low */
#define WAKE_UP                    ( (uint32_t)  0x80 )    /* 0/1 Wake-up command from Host to WLAN */

/* SPI_RESPONSE_DELAY Bit mask */
#define RESPONSE_DELAY_MASK 0xFF    /* Configurable rd response delay in multiples of 8 bits */

/* SPI_STATUS_ENABLE Bits */
#define STATUS_ENABLE              ( (uint32_t)  0x01 )    /* 1/0 Status sent/not sent to host after read/write */
#define INTR_WITH_STATUS           ( (uint32_t)  0x02 )    /* 0/1 Do-not / do-interrupt if status is sent */
#define RESP_DELAY_ALL             ( (uint32_t)  0x04 )    /* Applicability of resp delay to F1 or all func's read */
#define DWORD_PKT_LEN_EN           ( (uint32_t)  0x08 )    /* Packet len denoted in dwords instead of bytes */
#define CMD_ERR_CHK_EN             ( (uint32_t)  0x20 )    /* Command error check enable */
#define DATA_ERR_CHK_EN            ( (uint32_t)  0x40 )    /* Data error check enable */



/* SPI_RESET_BP Bits*/
#define RESET_ON_WLAN_BP_RESET     ( (uint32_t)  0x04 )    /* enable reset for WLAN backplane */
#define RESET_ON_BT_BP_RESET       ( (uint32_t)  0x08 )    /* enable reset for BT backplane */
#define RESET_SPI                  ( (uint32_t)  0x80 )    /* reset the above enabled logic */



/* SPI_INTERRUPT_REGISTER and SPI_INTERRUPT_ENABLE_REGISTER Bits */
#define DATA_UNAVAILABLE           ( (uint32_t) 0x0001 )   /* Requested data not available; Clear by writing a "1" */
#define F2_F3_FIFO_RD_UNDERFLOW    ( (uint32_t) 0x0002 )
#define F2_F3_FIFO_WR_OVERFLOW     ( (uint32_t) 0x0004 )
#define COMMAND_ERROR              ( (uint32_t) 0x0008 )   /* Cleared by writing 1 */
#define DATA_ERROR                 ( (uint32_t) 0x0010 )   /* Cleared by writing 1 */
#define F2_PACKET_AVAILABLE        ( (uint32_t) 0x0020 )
#define F3_PACKET_AVAILABLE        ( (uint32_t) 0x0040 )
#define F1_OVERFLOW                ( (uint32_t) 0x0080 )   /* Due to last write. Bkplane has pending write requests */
#define MISC_INTR0                 ( (uint32_t) 0x0100 )
#define MISC_INTR1                 ( (uint32_t) 0x0200 )
#define MISC_INTR2                 ( (uint32_t) 0x0400 )
#define MISC_INTR3                 ( (uint32_t) 0x0800 )
#define MISC_INTR4                 ( (uint32_t) 0x1000 )
#define F1_INTR                    ( (uint32_t) 0x2000 )
#define F2_INTR                    ( (uint32_t) 0x4000 )
#define F3_INTR                    ( (uint32_t) 0x8000 )




/* SPI_STATUS_REGISTER Bits */
#define STATUS_DATA_NOT_AVAILABLE  ( (uint32_t) 0x00000001 )
#define STATUS_UNDERFLOW           ( (uint32_t) 0x00000002 )
#define STATUS_OVERFLOW            ( (uint32_t) 0x00000004 )
#define STATUS_F2_INTR             ( (uint32_t) 0x00000008 )
#define STATUS_F3_INTR             ( (uint32_t) 0x00000010 )
#define STATUS_F2_RX_READY         ( (uint32_t) 0x00000020 )
#define STATUS_F3_RX_READY         ( (uint32_t) 0x00000040 )
#define STATUS_HOST_CMD_DATA_ERR   ( (uint32_t) 0x00000080 )
#define STATUS_F2_PKT_AVAILABLE    ( (uint32_t) 0x00000100 )
#define STATUS_F2_PKT_LEN_MASK     ( (uint32_t) 0x000FFE00 )
#define STATUS_F2_PKT_LEN_SHIFT    ( (uint32_t) 9          )
#define STATUS_F3_PKT_AVAILABLE    ( (uint32_t) 0x00100000 )
#define STATUS_F3_PKT_LEN_MASK     ( (uint32_t) 0xFFE00000 )
#define STATUS_F3_PKT_LEN_SHIFT    ( (uint32_t) 21         )





/* SDIO_CHIP_CLOCK_CSR Bits */
#define SBSDIO_FORCE_ALP           ( (uint32_t)  0x01 )    /* Force ALP request to backplane */
#define SBSDIO_FORCE_HT            ( (uint32_t)  0x02 )    /* Force HT request to backplane */
#define SBSDIO_FORCE_ILP           ( (uint32_t)  0x04 )    /* Force ILP request to backplane */
#define SBSDIO_ALP_AVAIL_REQ       ( (uint32_t)  0x08 )    /* Make ALP ready (power up xtal) */
#define SBSDIO_HT_AVAIL_REQ        ( (uint32_t)  0x10 )    /* Make HT ready (power up PLL) */
#define SBSDIO_FORCE_HW_CLKREQ_OFF ( (uint32_t)  0x20 )    /* Squelch clock requests from HW */
#define SBSDIO_ALP_AVAIL           ( (uint32_t)  0x40 )    /* Status: ALP is ready */
#define SBSDIO_HT_AVAIL            ( (uint32_t)  0x80 )    /* Status: HT is ready */
#define SBSDIO_Rev8_HT_AVAIL       ( (uint32_t)  0x40 )
#define SBSDIO_Rev8_ALP_AVAIL      ( (uint32_t)  0x80 )


/* SDIO_FRAME_CONTROL Bits */
#define SFC_RF_TERM                ( (uint32_t) (1 << 0) ) /* Read Frame Terminate */
#define SFC_WF_TERM                ( (uint32_t) (1 << 1) ) /* Write Frame Terminate */
#define SFC_CRC4WOOS               ( (uint32_t) (1 << 2) ) /* HW reports CRC error for write out of sync */
#define SFC_ABORTALL               ( (uint32_t) (1 << 3) ) /* Abort cancels all in-progress frames */

/* SDIO_TO_SB_MAIL_BOX bits corresponding to intstatus bits */
#define SMB_NAK                    ( (uint32_t) (1 << 0) ) /* To SB Mailbox Frame NAK */
#define SMB_INT_ACK                ( (uint32_t) (1 << 1) ) /* To SB Mailbox Host Interrupt ACK */
#define SMB_USE_OOB                ( (uint32_t) (1 << 2) ) /* To SB Mailbox Use OOB Wakeup */
#define SMB_DEV_INT                ( (uint32_t) (1 << 3) ) /* To SB Mailbox Miscellaneous Interrupt */


#define WL_CHANSPEC_BAND_MASK             (0xf000)
#define WL_CHANSPEC_BAND_5G               (0x1000)
#define WL_CHANSPEC_BAND_2G               (0x2000)
#define INVCHANSPEC            255
#define WL_CHANSPEC_CTL_SB_MASK           (0x0300)
#define WL_CHANSPEC_CTL_SB_SHIFT         8
#define WL_CHANSPEC_CTL_SB_LOWER          (0x0100)
#define WL_CHANSPEC_CTL_SB_UPPER          (0x0200)
#define WL_CHANSPEC_CTL_SB_NONE           (0x0300)
#define WL_CHANSPEC_BW_MASK               (0x0C00)
#define WL_CHANSPEC_BW_SHIFT            10
#define WL_CHANSPEC_BW_10                 (0x0400)
#define WL_CHANSPEC_BW_20                 (0x0800)
#define WL_CHANSPEC_BW_40                 (0x0C00)


/* CIS accesses require backpane clock */


#define CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
#define CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR

struct ether_addr;
struct wl_join_scan_params;

typedef struct wl_assoc_params
{
    struct ether_addr bssid;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
    uint16_t    bssid_cnt;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
    uint32_t     chanspec_num;
    chanspec_t  chanspec_list[1];
} wl_assoc_params_t;
#define WL_ASSOC_PARAMS_FIXED_SIZE     (sizeof(wl_assoc_params_t) - sizeof(wl_chanspec_t))
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE    WL_ASSOC_PARAMS_FIXED_SIZE
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE        WL_ASSOC_PARAMS_FIXED_SIZE
typedef struct wl_join_params
{
    wlc_ssid_t         ssid;
    struct wl_assoc_params  params;
} wl_join_params_t;
#define WL_JOIN_PARAMS_FIXED_SIZE     (sizeof(wl_join_params_t) - sizeof(wl_chanspec_t))

/* extended join params */
typedef struct wl_extjoin_params
{
        wlc_ssid_t ssid; /* {0, ""}: wildcard scan */
        struct wl_join_scan_params scan_params;
        wl_join_assoc_params_t assoc_params; /* optional field, but it must include the fixed portion of the wl_join_assoc_params_t struct when it does present. */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE    (sizeof(wl_extjoin_params_t) - sizeof(chanspec_t))

//typedef wl_cnt_ver_six_t  wiced_counters_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_CHIP_CONSTANTS_H_ */

#endif/* _WIFI_AP6181 */


#ifdef _WIFI_5G_AP6234

#ifndef INCLUDED_CHIP_CONSTANTS_H_
#define INCLUDED_CHIP_CONSTANTS_H_

#include "wwd_wlioctl.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *             Architecture Constants
 ******************************************************/

/* General chip stats */
#define CHIP_RAM_SIZE      (512*1024)

/* Backplane architecture */
#define CHIPCOMMON_BASE_ADDRESS  0x18000000    /* Chipcommon core register region   */
#define DOT11MAC_BASE_ADDRESS    0x18001000    /* dot11mac core register region     */
#define SDIO_BASE_ADDRESS        0x18002000    /* SDIOD Device core register region */
#define WLAN_ARMCM3_BASE_ADDRESS 0x18003000    /* ARMCM3 core register region       */
#define SOCSRAM_BASE_ADDRESS     0x18004000    /* SOCSRAM core register region      */
#define BACKPLANE_ADDRESS_MASK   0x7FFF

#define WLAN_MEM_BASE_ADDRESS   0x00

#define CHIP_STA_INTERFACE   0
#define CHIP_AP_INTERFACE    1
#define CHIP_P2P_INTERFACE   2

/* Maximum value of bus data credit difference */
#define CHIP_MAX_BUS_DATA_CREDIT_DIFF    20

/* Chipcommon registers */
#define CHIPCOMMON_GPIO_CONTROL ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x6C) )

/******************************************************
 *             SDIO Constants
 ******************************************************/
/* CurrentSdiodProgGuide r23 */

/* Base registers */
#define SDIO_CORE                    ((uint32_t) (SDIO_BASE_ADDRESS + 0x00) )
#define SDIO_INT_STATUS              ((uint32_t) (SDIO_BASE_ADDRESS + 0x20) )
#define SDIO_TO_SB_MAILBOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_TO_SB_MAILBOX_DATA      ((uint32_t) (SDIO_BASE_ADDRESS + 0x48) )
#define SDIO_TO_HOST_MAILBOX_DATA    ((uint32_t) (SDIO_BASE_ADDRESS + 0x4C) )
#define SDIO_TO_SB_MAIL_BOX          ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_INT_HOST_MASK           ((uint32_t) (SDIO_BASE_ADDRESS + 0x24) )
#define SDIO_FUNCTION_INT_MASK       ((uint32_t) (SDIO_BASE_ADDRESS + 0x34) )

/* SDIO Function 0 (SDIO Bus) register addresses */

/* SDIO Device CCCR offsets */
/* TODO: What does CIS/CCCR stand for? */
/* CCCR accesses do not require backpane clock */
#define SDIOD_CCCR_REV             ( (uint32_t)  0x00 )    /* CCCR/SDIO Revision */
#define SDIOD_CCCR_SDREV           ( (uint32_t)  0x01 )    /* SD Revision */
#define SDIOD_CCCR_IOEN            ( (uint32_t)  0x02 )    /* I/O Enable */
#define SDIOD_CCCR_IORDY           ( (uint32_t)  0x03 )    /* I/O Ready */
#define SDIOD_CCCR_INTEN           ( (uint32_t)  0x04 )    /* Interrupt Enable */
#define SDIOD_CCCR_INTPEND         ( (uint32_t)  0x05 )    /* Interrupt Pending */
#define SDIOD_CCCR_IOABORT         ( (uint32_t)  0x06 )    /* I/O Abort */
#define SDIOD_CCCR_BICTRL          ( (uint32_t)  0x07 )    /* Bus Interface control */
#define SDIOD_CCCR_CAPABLITIES     ( (uint32_t)  0x08 )    /* Card Capabilities */
#define SDIOD_CCCR_CISPTR_0        ( (uint32_t)  0x09 )    /* Common CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_CISPTR_1        ( (uint32_t)  0x0A )    /* Common CIS Base Address Pointer Register 1 */
#define SDIOD_CCCR_CISPTR_2        ( (uint32_t)  0x0B )    /* Common CIS Base Address Pointer Register 2 (MSB - only bit 1 valid)*/
#define SDIOD_CCCR_BUSSUSP         ( (uint32_t)  0x0C )    /*  */
#define SDIOD_CCCR_FUNCSEL         ( (uint32_t)  0x0D )    /*  */
#define SDIOD_CCCR_EXECFLAGS       ( (uint32_t)  0x0E )    /*  */
#define SDIOD_CCCR_RDYFLAGS        ( (uint32_t)  0x0F )    /*  */
#define SDIOD_CCCR_BLKSIZE_0       ( (uint32_t)  0x10 )    /* Function 0 (Bus) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_BLKSIZE_1       ( (uint32_t)  0x11 )    /* Function 0 (Bus) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_POWER_CONTROL   ( (uint32_t)  0x12 )    /* Power Control */
#define SDIOD_CCCR_SPEED_CONTROL   ( (uint32_t)  0x13 )    /* Bus Speed Select  (control device entry into high-speed clocking mode)  */
#define SDIOD_CCCR_UHS_I           ( (uint32_t)  0x14 )    /* UHS-I Support */
#define SDIOD_CCCR_DRIVE           ( (uint32_t)  0x15 )    /* Drive Strength */
#define SDIOD_CCCR_INTEXT          ( (uint32_t)  0x16 )    /* Interrupt Extension */
#define SDIOD_SEP_INT_CTL          ( (uint32_t)  0xF2 )    /* Separate Interrupt Control*/
#define SDIOD_CCCR_F1INFO          ( (uint32_t) 0x100 )    /* Function 1 (Backplane) Info */
#define SDIOD_CCCR_F1HP            ( (uint32_t) 0x102 )    /* Function 1 (Backplane) High Power */
#define SDIOD_CCCR_F1CISPTR_0      ( (uint32_t) 0x109 )    /* Function 1 (Backplane) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F1CISPTR_1      ( (uint32_t) 0x10A )    /* Function 1 (Backplane) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F1CISPTR_2      ( (uint32_t) 0x10B )    /* Function 1 (Backplane) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F1BLKSIZE_0     ( (uint32_t) 0x110 )    /* Function 1 (Backplane) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F1BLKSIZE_1     ( (uint32_t) 0x111 )    /* Function 1 (Backplane) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F2INFO          ( (uint32_t) 0x200 )    /* Function 2 (WLAN Data FIFO) Info */
#define SDIOD_CCCR_F2HP            ( (uint32_t) 0x202 )    /* Function 2 (WLAN Data FIFO) High Power */
#define SDIOD_CCCR_F2CISPTR_0      ( (uint32_t) 0x209 )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F2CISPTR_1      ( (uint32_t) 0x20A )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F2CISPTR_2      ( (uint32_t) 0x20B )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F2BLKSIZE_0     ( (uint32_t) 0x210 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F2BLKSIZE_1     ( (uint32_t) 0x211 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F3INFO          ( (uint32_t) 0x300 )    /* Function 3 (Bluetooth Data FIFO) Info */
#define SDIOD_CCCR_F3HP            ( (uint32_t) 0x302 )    /* Function 3 (Bluetooth Data FIFO) High Power */
#define SDIOD_CCCR_F3CISPTR_0      ( (uint32_t) 0x309 )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F3CISPTR_1      ( (uint32_t) 0x30A )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F3CISPTR_2      ( (uint32_t) 0x30B )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F3BLKSIZE_0     ( (uint32_t) 0x310 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F3BLKSIZE_1     ( (uint32_t) 0x311 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 1 (MSB) */


/* SDIO Function 1 (Backplane) register addresses */
/* Addresses 0x00000000 - 0x0000FFFF are directly access the backplane
 * throught the backplane window. Addresses above 0x0000FFFF are
 * registers relating to backplane access, and do not require a backpane
 * clock to access them
 */
#define SDIO_GPIO_SELECT              ( (uint32_t) 0x10005 )
#define SDIO_GPIO_OUTPUT              ( (uint32_t) 0x10006 )
#define SDIO_GPIO_ENABLE              ( (uint32_t) 0x10007 )
#define SDIO_FUNCTION2_WATERMARK      ( (uint32_t) 0x10008 )
#define SDIO_DEVICE_CONTROL           ( (uint32_t) 0x10009 )
#define SDIO_BACKPLANE_ADDRESS_LOW    ( (uint32_t) 0x1000A )
#define SDIO_BACKPLANE_ADDRESS_MID    ( (uint32_t) 0x1000B )
#define SDIO_BACKPLANE_ADDRESS_HIGH   ( (uint32_t) 0x1000C )
#define SDIO_FRAME_CONTROL            ( (uint32_t) 0x1000D )
#define SDIO_CHIP_CLOCK_CSR           ( (uint32_t) 0x1000E )
#define SDIO_PULL_UP                  ( (uint32_t) 0x1000F )
#define SDIO_READ_FRAME_BC_LOW        ( (uint32_t) 0x1001B )
#define SDIO_READ_FRAME_BC_HIGH       ( (uint32_t) 0x1001C )

/* Micky ++ */
#define SDIO_SB_ACCESS_2_4B_FLAG    0x08000        /* with b15, maps to 32-bit SB access */

#define I_HMB_SW_MASK                 ( (uint32_t) 0x000000F0 )
#define I_HMB_FRAME_IND               ( 1<<6 )
#define FRAME_AVAILABLE_MASK          I_HMB_SW_MASK


/******************************************************
 *             SPI Constants
 ******************************************************/
/* GSPI v1 */
#define SPI_FRAME_CONTROL              ( (uint32_t) 0x1000D )

/* Register addresses */
#define SPI_BUS_CONTROL                ( (uint32_t) 0x0000 )
#define SPI_RESPONSE_DELAY             ( (uint32_t) 0x0001 )
#define SPI_STATUS_ENABLE              ( (uint32_t) 0x0002 )
#define SPI_RESET_BP                   ( (uint32_t) 0x0003 )    /* (corerev >= 1) */
#define SPI_INTERRUPT_REGISTER         ( (uint32_t) 0x0004 )    /* 16 bits - Interrupt status */
#define SPI_INTERRUPT_ENABLE_REGISTER  ( (uint32_t) 0x0006 )    /* 16 bits - Interrupt mask */
#define SPI_STATUS_REGISTER            ( (uint32_t) 0x0008 )    /* 32 bits */
#define SPI_FUNCTION1_INFO             ( (uint32_t) 0x000C )    /* 16 bits */
#define SPI_FUNCTION2_INFO             ( (uint32_t) 0x000E )    /* 16 bits */
#define SPI_FUNCTION3_INFO             ( (uint32_t) 0x0010 )    /* 16 bits */
#define SPI_READ_TEST_REGISTER         ( (uint32_t) 0x0014 )    /* 32 bits */
#define SPI_RESP_DELAY_F0              ( (uint32_t) 0x001c )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F1              ( (uint32_t) 0x001d )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F2              ( (uint32_t) 0x001e )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F3              ( (uint32_t) 0x001f )    /* 8 bits (corerev >= 3) */

/******************************************************
 *             Bit Masks
 ******************************************************/

/* SDIOD_CCCR_REV Bits */
#define SDIO_REV_SDIOID_MASK       ( (uint32_t)  0xF0 )    /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK       ( (uint32_t)  0x0F )    /* CCCR format version number */

/* SDIOD_CCCR_SDREV Bits */
#define SD_REV_PHY_MASK            ( (uint32_t)  0x0F )    /* SD format version number */

/* SDIOD_CCCR_IOEN Bits */
#define SDIO_FUNC_ENABLE_1         ( (uint32_t)  0x02 )    /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2         ( (uint32_t)  0x04 )    /* function 2 I/O enable */
#define SDIO_FUNC_ENABLE_3         ( (uint32_t)  0x08 )    /* function 3 I/O enable */

/* SDIOD_CCCR_IORDY Bits */
#define SDIO_FUNC_READY_1          ( (uint32_t)  0x02 )    /* function 1 I/O ready */
#define SDIO_FUNC_READY_2          ( (uint32_t)  0x04 )    /* function 2 I/O ready */
#define SDIO_FUNC_READY_3          ( (uint32_t)  0x08 )    /* function 3 I/O ready */

/* SDIOD_CCCR_INTEN Bits */
#define INTR_CTL_MASTER_EN         ( (uint32_t)  0x01 )    /* interrupt enable master */
#define INTR_CTL_FUNC1_EN          ( (uint32_t)  0x02 )    /* interrupt enable for function 1 */
#define INTR_CTL_FUNC2_EN          ( (uint32_t)  0x04 )    /* interrupt enable for function 2 */

/* SDIOD_SEP_INT_CTL Bits */
#define SEP_INTR_CTL_MASK          ( (uint32_t)  0x01 )    /* out-of-band interrupt mask */
#define SEP_INTR_CTL_EN            ( (uint32_t)  0x02 )    /* out-of-band interrupt output enable */
#define SEP_INTR_CTL_POL           ( (uint32_t)  0x04 )    /* out-of-band interrupt polarity */

/* SDIOD_CCCR_INTPEND Bits */
#define INTR_STATUS_FUNC1          ( (uint32_t)  0x02 )    /* interrupt pending for function 1 */
#define INTR_STATUS_FUNC2          ( (uint32_t)  0x04 )    /* interrupt pending for function 2 */
#define INTR_STATUS_FUNC3          ( (uint32_t)  0x08 )    /* interrupt pending for function 3 */

/* SDIOD_CCCR_IOABORT Bits */
#define IO_ABORT_RESET_ALL         ( (uint32_t)  0x08 )    /* I/O card reset */
#define IO_ABORT_FUNC_MASK         ( (uint32_t)  0x07 )    /* abort selction: function x */

/* SDIOD_CCCR_BICTRL Bits */
#define BUS_CARD_DETECT_DIS        ( (uint32_t)  0x80 )    /* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP      ( (uint32_t)  0x40 )    /* support continuous SPI interrupt */
#define BUS_SPI_CONT_INTR_EN       ( (uint32_t)  0x20 )    /* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK     ( (uint32_t)  0x03 )    /* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT     ( (uint32_t)  0x02 )    /* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT     ( (uint32_t)  0x00 )    /* bus width 1-bit mode */

/* SDIOD_CCCR_CAPABLITIES Bits */
#define SDIO_CAP_4BLS              ( (uint32_t)  0x80 )    /* 4-bit support for low speed card */
#define SDIO_CAP_LSC               ( (uint32_t)  0x40 )    /* low speed card */
#define SDIO_CAP_E4MI              ( (uint32_t)  0x20 )    /* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI              ( (uint32_t)  0x10 )    /* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS               ( (uint32_t)  0x08 )    /* support suspend/resume */
#define SDIO_CAP_SRW               ( (uint32_t)  0x04 )    /* support read wait */
#define SDIO_CAP_SMB               ( (uint32_t)  0x02 )    /* support multi-block transfer */
#define SDIO_CAP_SDC               ( (uint32_t)  0x01 )    /* Support Direct commands during multi-byte transfer */

/* SDIOD_CCCR_POWER_CONTROL Bits */
#define SDIO_POWER_SMPC            ( (uint32_t)  0x01 )    /* supports master power control (RO) */
#define SDIO_POWER_EMPC            ( (uint32_t)  0x02 )    /* enable master power control (allow > 200mA) (RW) */

/* SDIOD_CCCR_SPEED_CONTROL Bits */
#define SDIO_SPEED_SHS             ( (uint32_t)  0x01 )    /* supports high-speed [clocking] mode (RO) */
#define SDIO_SPEED_EHS             ( (uint32_t)  0x02 )    /* enable high-speed [clocking] mode (RW) */



/* GSPI */
#define SPI_READ_TEST_REGISTER_VALUE   ( (uint32_t) 0xFEEDBEAD )
#define SPI_READ_TEST_REG_LSB          ( ( ( SPI_READ_TEST_REGISTER_VALUE      )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT1     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT2     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 ) + 1 ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT3     ( ( ( SPI_READ_TEST_REGISTER_VALUE +1  ) << 1 ) & 0xff )


/* SPI_BUS_CONTROL Bits */
#define WORD_LENGTH_32             ( (uint32_t)  0x01 )    /* 0/1 16/32 bit word length */
#define ENDIAN_BIG                 ( (uint32_t)  0x02 )    /* 0/1 Little/Big Endian */
#define CLOCK_PHASE                ( (uint32_t)  0x04 )    /* 0/1 clock phase delay */
#define CLOCK_POLARITY             ( (uint32_t)  0x08 )    /* 0/1 Idle state clock polarity is low/high */
#define HIGH_SPEED_MODE            ( (uint32_t)  0x10 )    /* 1/0 High Speed mode / Normal mode */
#define INTR_POLARITY_HIGH         ( (uint32_t)  0x20 )    /* 1/0 Interrupt active polarity is high/low */
#define WAKE_UP                    ( (uint32_t)  0x80 )    /* 0/1 Wake-up command from Host to WLAN */

/* SPI_RESPONSE_DELAY Bit mask */
#define RESPONSE_DELAY_MASK 0xFF    /* Configurable rd response delay in multiples of 8 bits */

/* SPI_STATUS_ENABLE Bits */
#define STATUS_ENABLE              ( (uint32_t)  0x01 )    /* 1/0 Status sent/not sent to host after read/write */
#define INTR_WITH_STATUS           ( (uint32_t)  0x02 )    /* 0/1 Do-not / do-interrupt if status is sent */
#define RESP_DELAY_ALL             ( (uint32_t)  0x04 )    /* Applicability of resp delay to F1 or all func's read */
#define DWORD_PKT_LEN_EN           ( (uint32_t)  0x08 )    /* Packet len denoted in dwords instead of bytes */
#define CMD_ERR_CHK_EN             ( (uint32_t)  0x20 )    /* Command error check enable */
#define DATA_ERR_CHK_EN            ( (uint32_t)  0x40 )    /* Data error check enable */



/* SPI_RESET_BP Bits*/
#define RESET_ON_WLAN_BP_RESET     ( (uint32_t)  0x04 )    /* enable reset for WLAN backplane */
#define RESET_ON_BT_BP_RESET       ( (uint32_t)  0x08 )    /* enable reset for BT backplane */
#define RESET_SPI                  ( (uint32_t)  0x80 )    /* reset the above enabled logic */



/* SPI_INTERRUPT_REGISTER and SPI_INTERRUPT_ENABLE_REGISTER Bits */
#define DATA_UNAVAILABLE           ( (uint32_t) 0x0001 )   /* Requested data not available; Clear by writing a "1" */
#define F2_F3_FIFO_RD_UNDERFLOW    ( (uint32_t) 0x0002 )
#define F2_F3_FIFO_WR_OVERFLOW     ( (uint32_t) 0x0004 )
#define COMMAND_ERROR              ( (uint32_t) 0x0008 )   /* Cleared by writing 1 */
#define DATA_ERROR                 ( (uint32_t) 0x0010 )   /* Cleared by writing 1 */
#define F2_PACKET_AVAILABLE        ( (uint32_t) 0x0020 )
#define F3_PACKET_AVAILABLE        ( (uint32_t) 0x0040 )
#define F1_OVERFLOW                ( (uint32_t) 0x0080 )   /* Due to last write. Bkplane has pending write requests */
#define MISC_INTR0                 ( (uint32_t) 0x0100 )
#define MISC_INTR1                 ( (uint32_t) 0x0200 )
#define MISC_INTR2                 ( (uint32_t) 0x0400 )
#define MISC_INTR3                 ( (uint32_t) 0x0800 )
#define MISC_INTR4                 ( (uint32_t) 0x1000 )
#define F1_INTR                    ( (uint32_t) 0x2000 )
#define F2_INTR                    ( (uint32_t) 0x4000 )
#define F3_INTR                    ( (uint32_t) 0x8000 )




/* SPI_STATUS_REGISTER Bits */
#define STATUS_DATA_NOT_AVAILABLE  ( (uint32_t) 0x00000001 )
#define STATUS_UNDERFLOW           ( (uint32_t) 0x00000002 )
#define STATUS_OVERFLOW            ( (uint32_t) 0x00000004 )
#define STATUS_F2_INTR             ( (uint32_t) 0x00000008 )
#define STATUS_F3_INTR             ( (uint32_t) 0x00000010 )
#define STATUS_F2_RX_READY         ( (uint32_t) 0x00000020 )
#define STATUS_F3_RX_READY         ( (uint32_t) 0x00000040 )
#define STATUS_HOST_CMD_DATA_ERR   ( (uint32_t) 0x00000080 )
#define STATUS_F2_PKT_AVAILABLE    ( (uint32_t) 0x00000100 )
#define STATUS_F2_PKT_LEN_MASK     ( (uint32_t) 0x000FFE00 )
#define STATUS_F2_PKT_LEN_SHIFT    ( (uint32_t) 9          )
#define STATUS_F3_PKT_AVAILABLE    ( (uint32_t) 0x00100000 )
#define STATUS_F3_PKT_LEN_MASK     ( (uint32_t) 0xFFE00000 )
#define STATUS_F3_PKT_LEN_SHIFT    ( (uint32_t) 21         )





/* SDIO_CHIP_CLOCK_CSR Bits */
#define SBSDIO_FORCE_ALP           ( (uint32_t)  0x01 )    /* Force ALP request to backplane */
#define SBSDIO_FORCE_HT            ( (uint32_t)  0x02 )    /* Force HT request to backplane */
#define SBSDIO_FORCE_ILP           ( (uint32_t)  0x04 )    /* Force ILP request to backplane */
#define SBSDIO_ALP_AVAIL_REQ       ( (uint32_t)  0x08 )    /* Make ALP ready (power up xtal) */
#define SBSDIO_HT_AVAIL_REQ        ( (uint32_t)  0x10 )    /* Make HT ready (power up PLL) */
#define SBSDIO_FORCE_HW_CLKREQ_OFF ( (uint32_t)  0x20 )    /* Squelch clock requests from HW */
#define SBSDIO_ALP_AVAIL           ( (uint32_t)  0x40 )    /* Status: ALP is ready */
#define SBSDIO_HT_AVAIL            ( (uint32_t)  0x80 )    /* Status: HT is ready */
#define SBSDIO_Rev8_HT_AVAIL       ( (uint32_t)  0x40 )
#define SBSDIO_Rev8_ALP_AVAIL      ( (uint32_t)  0x80 )


/* SDIO_FRAME_CONTROL Bits */
#define SFC_RF_TERM                ( (uint32_t) (1 << 0) ) /* Read Frame Terminate */
#define SFC_WF_TERM                ( (uint32_t) (1 << 1) ) /* Write Frame Terminate */
#define SFC_CRC4WOOS               ( (uint32_t) (1 << 2) ) /* HW reports CRC error for write out of sync */
#define SFC_ABORTALL               ( (uint32_t) (1 << 3) ) /* Abort cancels all in-progress frames */

/* SDIO_TO_SB_MAIL_BOX bits corresponding to intstatus bits */
#define SMB_NAK                    ( (uint32_t) (1 << 0) ) /* To SB Mailbox Frame NAK */
#define SMB_INT_ACK                ( (uint32_t) (1 << 1) ) /* To SB Mailbox Host Interrupt ACK */
#define SMB_USE_OOB                ( (uint32_t) (1 << 2) ) /* To SB Mailbox Use OOB Wakeup */
#define SMB_DEV_INT                ( (uint32_t) (1 << 3) ) /* To SB Mailbox Miscellaneous Interrupt */


#define WL_CHANSPEC_BAND_MASK             (0xf000)
#define WL_CHANSPEC_BAND_5G               (0x1000)
#define WL_CHANSPEC_BAND_2G               (0x2000)
#define INVCHANSPEC            255
#define WL_CHANSPEC_CTL_SB_MASK           (0x0300)
#define WL_CHANSPEC_CTL_SB_SHIFT         8
#define WL_CHANSPEC_CTL_SB_LOWER          (0x0100)
#define WL_CHANSPEC_CTL_SB_UPPER          (0x0200)
#define WL_CHANSPEC_CTL_SB_NONE           (0x0300)
#define WL_CHANSPEC_BW_MASK               (0x0C00)
#define WL_CHANSPEC_BW_SHIFT            10
#define WL_CHANSPEC_BW_10                 (0x0400)
#define WL_CHANSPEC_BW_20                 (0x0800)
#define WL_CHANSPEC_BW_40                 (0x0C00)


/* CIS accesses require backpane clock */


#define CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
#define CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR

struct ether_addr;
struct wl_join_scan_params;

typedef struct wl_assoc_params
{
    struct ether_addr bssid;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
    uint16_t    bssid_cnt;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
    uint32_t     chanspec_num;
    chanspec_t  chanspec_list[1];
} wl_assoc_params_t;
#define WL_ASSOC_PARAMS_FIXED_SIZE     (sizeof(wl_assoc_params_t) - sizeof(wl_chanspec_t))
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE    WL_ASSOC_PARAMS_FIXED_SIZE
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE        WL_ASSOC_PARAMS_FIXED_SIZE
typedef struct wl_join_params
{
    wlc_ssid_t         ssid;
    struct wl_assoc_params  params;
} wl_join_params_t;
#define WL_JOIN_PARAMS_FIXED_SIZE     (sizeof(wl_join_params_t) - sizeof(wl_chanspec_t))

/* extended join params */
typedef struct wl_extjoin_params
{
        wlc_ssid_t ssid; /* {0, ""}: wildcard scan */
        struct wl_join_scan_params scan_params;
        wl_join_assoc_params_t assoc_params; /* optional field, but it must include the fixed portion of the wl_join_assoc_params_t struct when it does present. */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE    (sizeof(wl_extjoin_params_t) - sizeof(chanspec_t))

//typedef wl_cnt_ver_six_t  wiced_counters_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_CHIP_CONSTANTS_H_ */

#endif/* ap6234*/

#ifdef _WIFI_5G_AP6255

#ifndef INCLUDED_CHIP_CONSTANTS_H_
#define INCLUDED_CHIP_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *             Architecture Constants
 ******************************************************/

/* General chip stats */
#define CHIP_RAM_SIZE      0xC8000

/* Backplane architecture */
#define CHIPCOMMON_BASE_ADDRESS  0x18000000    /* Chipcommon core register region   */
#define DOT11MAC_BASE_ADDRESS    0x18001000    /* dot11mac core register region     */
#define WLAN_ARMCR4_BASE_ADDRESS 0x18002000    /* ARMCR4 core register region       */
#define PCIE_GEN2_BASE_ADDRESS   0x18003000    /* PCIE_GEN2 core register region       */
#define SDIO_BASE_ADDRESS        0x18004000    /* SDIOD Device core register region */
#define WLAN_DEBUG_BASE_ADDRESS  0x18005000    /* ARMCR4 core debug register region       */
#define BACKPLANE_ADDRESS_MASK   0x7FFF

#define CHIP_STA_INTERFACE   0
#define CHIP_AP_INTERFACE    1
#define CHIP_P2P_INTERFACE   2

/* Maximum value of bus data credit difference */
#define CHIP_MAX_BUS_DATA_CREDIT_DIFF     24//20

// Micky add to diff CM3 and CR4 ++
#define WLAN_MCU_BASE_ADDRESS   WLAN_ARMCR4_BASE_ADDRESS
#define WLAN_MEM_BASE_ADDRESS   0x00198000

// for get ram size ++
/* CR4 specific defines used by the host driver */
#define SI_CR4_CAP            (0x04)
#define SI_CR4_BANKIDX        (0x40)
#define SI_CR4_BANKINFO        (0x44)
#define SI_CR4_BANKPDA        (0x4C)

#define    ARMCR4_TCBBNB_MASK    0xf0
#define    ARMCR4_TCBBNB_SHIFT    4
#define    ARMCR4_TCBANB_MASK    0xf
#define    ARMCR4_TCBANB_SHIFT    0

#define    ARMCR4_BSZ_MASK        0x3f
#define    ARMCR4_BSZ_MULT        8192

#define    WL_CNT_T_VERSION    10    /* current version of wl_cnt_t struct */
// for get ram size --

#define SBSDIO_DEVCTL_PADS_ISO        0x08        /* 1: isolate internal sdio signals, put
                                                                 * external pads in tri-state; requires
                                                                 * sdio bus power cycle to clear (rev 9)
                                                                 */
// Micky --

/* Chipcommon registers */
#define CHIPCOMMON_CHIP_ID ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x00) )
#define CHIPCOMMON_GPIO_CONTROL ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x6C) )

// for 4339, 43455 ++
// and also for 4335, 43349, 4345, 43454, 4356, 4358, 4371, 4350
#define CHIPCOMMON_CHIPCTRL_ADDR ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x650) )
#define CHIPCOMMON_CHIPCTRL_DATA ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x654) )
#define CC_PMUCC3    (0x3)
// for 4339, 43455 --

// for 4339, 43455 ++
// and also for 4350, 4345, 43454, 4354, 4356, 4358, 4371
#define    CC_CHIPCTRL3_SR_ENG_ENABLE    (1  << 2)
// for 4339, 43455 --

/******************************************************
 *             SDIO Constants
 ******************************************************/
/* CurrentSdiodProgGuide r23 */

/* Base registers */
#define SDIO_CORE                    ((uint32_t) (SDIO_BASE_ADDRESS + 0x00) )
#define SDIO_INT_STATUS              ((uint32_t) (SDIO_BASE_ADDRESS + 0x20) )
#define SDIO_INT_HOST_MASK           ((uint32_t) (SDIO_BASE_ADDRESS + 0x24) )
#define SDIO_SB_INT_MASK            ((uint32_t) (SDIO_BASE_ADDRESS + 0x30) )
#define SDIO_FUNCTION_INT_MASK       ((uint32_t) (SDIO_BASE_ADDRESS + 0x34) )
#define SDIO_TO_SB_MAILBOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_TO_SB_MAIL_BOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_TO_HOST_MAILBOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x44) )
#define SDIO_TO_SB_MAILBOX_DATA      ((uint32_t) (SDIO_BASE_ADDRESS + 0x48) )
#define SDIO_TO_HOST_MAILBOX_DATA    ((uint32_t) (SDIO_BASE_ADDRESS + 0x4C) )

/* Micky ++ */
#define SDIO_SB_ACCESS_2_4B_FLAG    0x08000        /* with b15, maps to 32-bit SB access */

/* sbintstatus */
#define I_SB_SERR    (1 << 8)    /* Backplane SError (write) */
#define I_SB_RESPERR    (1 << 9)    /* Backplane Response Error (read) */
#define I_SB_SPROMERR    (1 << 10)    /* Error accessing the sprom */

/* Statistic Registers */
#define  SDIO_Cmd52RdCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x110) )
#define  SDIO_Cmd52WrCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x114) )
#define  SDIO_Cmd53RdCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x118) )
#define  SDIO_Cmd53WrCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x11C) )
#define  SDIO_AbortCount             ((uint32_t) (SDIO_BASE_ADDRESS + 0x120) )
#define  SDIO_DataCrcErrorCount     ((uint32_t) (SDIO_BASE_ADDRESS + 0x124) )
#define  SDIO_RdOutOfSyncCount     ((uint32_t) (SDIO_BASE_ADDRESS + 0x128) )
#define  SDIO_WrOutOfSyncCount     ((uint32_t) (SDIO_BASE_ADDRESS + 0x12C) )
#define  SDIO_WriteBusyCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x130) )
#define  SDIO_ReadWaitCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x134) )
#define  SDIO_ReadTermCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x138) )
#define  SDIO_WriteTermCount         ((uint32_t) (SDIO_BASE_ADDRESS + 0x13C) )


/* Sdio Core Rev 12 */
#define SDIO_FUNC1_WAKEUPCTRL                0x1001E
#define SDIO_FUNC1_WCTRL_ALPWAIT_MASK        0x1
#define SDIO_FUNC1_WCTRL_ALPWAIT_SHIFT        0
#define SDIO_FUNC1_WCTRL_HTWAIT_MASK        0x2
#define SDIO_FUNC1_WCTRL_HTWAIT_SHIFT        1

#define SDIO_FUNC1_SLEEPCSR                    0x1001F
#define SDIO_FUNC1_SLEEPCSR_KSO_MASK        0x1
#define SDIO_FUNC1_SLEEPCSR_KSO_SHIFT        0
#define SDIO_FUNC1_SLEEPCSR_KSO_EN            1
#define SDIO_FUNC1_SLEEPCSR_DEVON_MASK        0x2
#define SDIO_FUNC1_SLEEPCSR_DEVON_SHIFT    1

/* Micky -- */


/* SDIO Function 0 (SDIO Bus) register addresses */

/* SDIO Device CCCR offsets */
/* TODO: What does CIS/CCCR stand for? */
/* CCCR accesses do not require backpane clock */
#define SDIOD_CCCR_REV             ( (uint32_t)  0x00 )    /* CCCR/SDIO Revision */
#define SDIOD_CCCR_SDREV           ( (uint32_t)  0x01 )    /* SD Revision */
#define SDIOD_CCCR_IOEN            ( (uint32_t)  0x02 )    /* I/O Enable */
#define SDIOD_CCCR_IORDY           ( (uint32_t)  0x03 )    /* I/O Ready */
#define SDIOD_CCCR_INTEN           ( (uint32_t)  0x04 )    /* Interrupt Enable */
#define SDIOD_CCCR_INTPEND         ( (uint32_t)  0x05 )    /* Interrupt Pending */
#define SDIOD_CCCR_IOABORT         ( (uint32_t)  0x06 )    /* I/O Abort */
#define SDIOD_CCCR_BICTRL          ( (uint32_t)  0x07 )    /* Bus Interface control */
#define SDIOD_CCCR_CAPABLITIES     ( (uint32_t)  0x08 )    /* Card Capabilities */
#define SDIOD_CCCR_CISPTR_0        ( (uint32_t)  0x09 )    /* Common CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_CISPTR_1        ( (uint32_t)  0x0A )    /* Common CIS Base Address Pointer Register 1 */
#define SDIOD_CCCR_CISPTR_2        ( (uint32_t)  0x0B )    /* Common CIS Base Address Pointer Register 2 (MSB - only bit 1 valid)*/
#define SDIOD_CCCR_BUSSUSP         ( (uint32_t)  0x0C )    /*  */
#define SDIOD_CCCR_FUNCSEL         ( (uint32_t)  0x0D )    /*  */
#define SDIOD_CCCR_EXECFLAGS       ( (uint32_t)  0x0E )    /*  */
#define SDIOD_CCCR_RDYFLAGS        ( (uint32_t)  0x0F )    /*  */
#define SDIOD_CCCR_BLKSIZE_0       ( (uint32_t)  0x10 )    /* Function 0 (Bus) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_BLKSIZE_1       ( (uint32_t)  0x11 )    /* Function 0 (Bus) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_POWER_CONTROL   ( (uint32_t)  0x12 )    /* Power Control */
#define SDIOD_CCCR_SPEED_CONTROL   ( (uint32_t)  0x13 )    /* Bus Speed Select  (control device entry into high-speed clocking mode)  */
#define SDIOD_CCCR_UHS_I           ( (uint32_t)  0x14 )    /* UHS-I Support */
#define SDIOD_CCCR_DRIVE           ( (uint32_t)  0x15 )    /* Drive Strength */
#define SDIOD_CCCR_INTEXT          ( (uint32_t)  0x16 )    /* Interrupt Extension */
#define SDIOD_CCCR_BRCM_CARDCAP    ( (uint32_t)  0xF0 )    /* Brcm Card Capability */
#define SDIOD_SEP_INT_CTL          ( (uint32_t)  0xF2 )    /* Separate Interrupt Control*/
#define SDIOD_CCCR_F1INFO          ( (uint32_t) 0x100 )    /* Function 1 (Backplane) Info */
#define SDIOD_CCCR_F1HP            ( (uint32_t) 0x102 )    /* Function 1 (Backplane) High Power */
#define SDIOD_CCCR_F1CISPTR_0      ( (uint32_t) 0x109 )    /* Function 1 (Backplane) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F1CISPTR_1      ( (uint32_t) 0x10A )    /* Function 1 (Backplane) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F1CISPTR_2      ( (uint32_t) 0x10B )    /* Function 1 (Backplane) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F1BLKSIZE_0     ( (uint32_t) 0x110 )    /* Function 1 (Backplane) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F1BLKSIZE_1     ( (uint32_t) 0x111 )    /* Function 1 (Backplane) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F2INFO          ( (uint32_t) 0x200 )    /* Function 2 (WLAN Data FIFO) Info */
#define SDIOD_CCCR_F2HP            ( (uint32_t) 0x202 )    /* Function 2 (WLAN Data FIFO) High Power */
#define SDIOD_CCCR_F2CISPTR_0      ( (uint32_t) 0x209 )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F2CISPTR_1      ( (uint32_t) 0x20A )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F2CISPTR_2      ( (uint32_t) 0x20B )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F2BLKSIZE_0     ( (uint32_t) 0x210 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F2BLKSIZE_1     ( (uint32_t) 0x211 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F3INFO          ( (uint32_t) 0x300 )    /* Function 3 (Bluetooth Data FIFO) Info */
#define SDIOD_CCCR_F3HP            ( (uint32_t) 0x302 )    /* Function 3 (Bluetooth Data FIFO) High Power */
#define SDIOD_CCCR_F3CISPTR_0      ( (uint32_t) 0x309 )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F3CISPTR_1      ( (uint32_t) 0x30A )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F3CISPTR_2      ( (uint32_t) 0x30B )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F3BLKSIZE_0     ( (uint32_t) 0x310 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F3BLKSIZE_1     ( (uint32_t) 0x311 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 1 (MSB) */


/* SDIO Function 1 (Backplane) register addresses */
/* Addresses 0x00000000 - 0x0000FFFF are directly access the backplane
 * throught the backplane window. Addresses above 0x0000FFFF are
 * registers relating to backplane access, and do not require a backpane
 * clock to access them
 */
#define SDIO_GPIO_SELECT              ( (uint32_t) 0x10005 )
#define SDIO_GPIO_OUTPUT              ( (uint32_t) 0x10006 )
#define SDIO_GPIO_ENABLE              ( (uint32_t) 0x10007 )
#define SDIO_FUNCTION2_WATERMARK      ( (uint32_t) 0x10008 )
#define SDIO_DEVICE_CONTROL           ( (uint32_t) 0x10009 )
#define SDIO_BACKPLANE_ADDRESS_LOW    ( (uint32_t) 0x1000A )
#define SDIO_BACKPLANE_ADDRESS_MID    ( (uint32_t) 0x1000B )
#define SDIO_BACKPLANE_ADDRESS_HIGH   ( (uint32_t) 0x1000C )
#define SDIO_FRAME_CONTROL            ( (uint32_t) 0x1000D )
#define SDIO_CHIP_CLOCK_CSR           ( (uint32_t) 0x1000E )
#define SDIO_PULL_UP                  ( (uint32_t) 0x1000F )
#define SDIO_READ_FRAME_BC_LOW        ( (uint32_t) 0x1001B )
#define SDIO_READ_FRAME_BC_HIGH       ( (uint32_t) 0x1001C )
#define SDIO_MES_BUSY_CTRL            ( (uint32_t) 0x1001D )
#define SDIO_WAKEUP_CTRL              ( (uint32_t) 0x1001E )
#define SDIO_SLEEP_CSR                ( (uint32_t) 0x1001F )

#define I_HMB_SW_MASK                 ( (uint32_t) 0x000000F0 )
#define FRAME_AVAILABLE_MASK          I_HMB_SW_MASK

/* intstatus */
#define I_SMB_SW0        (1 << 0)    /* To SB Mail S/W interrupt 0 */
#define I_SMB_SW1        (1 << 1)    /* To SB Mail S/W interrupt 1 */
#define I_SMB_SW2        (1 << 2)    /* To SB Mail S/W interrupt 2 */
#define I_SMB_SW3        (1 << 3)    /* To SB Mail S/W interrupt 3 */
#define I_HMB_SW0        (1 << 4)    /* To Host Mail S/W interrupt 0 */
#define I_HMB_SW1        (1 << 5)    /* To Host Mail S/W interrupt 1 */
#define I_HMB_SW2        (1 << 6)    /* To Host Mail S/W interrupt 2 */
#define I_HMB_SW3        (1 << 7)    /* To Host Mail S/W interrupt 3 */
#define I_WR_OOSYNC        (1 << 8)    /* Write Frame Out Of Sync */
#define I_RD_OOSYNC        (1 << 9)    /* Read Frame Out Of Sync */
#define   I_PC                (1 << 10)    /* descriptor error */
#define   I_PD                (1 << 11)    /* data error */
#define   I_DE                (1 << 12)    /* Descriptor protocol Error */
#define   I_RU                (1 << 13)    /* Receive descriptor Underflow */
#define   I_RO                (1 << 14)    /* Receive fifo Overflow */
#define   I_XU                (1 << 15)    /* Transmit fifo Underflow */
#define   I_RI                (1 << 16)    /* Receive Interrupt */
#define I_BUSPWR            (1 << 17)    /* SDIO Bus Power Change (rev 9) */
#define I_HOST_ACCESS       (1 << 18) /* Host need not handle this interrupt */
#define I_HOST_WAKEUP      (1 << 22) /* Indicate whether the device is woken up by the host */
#define I_XMTDATA_AVAIL     (1 << 23)    /* bits in fifo */
#define   I_XI                (1 << 24)    /* Transmit Interrupt */
#define I_RF_TERM            (1 << 25)    /* Read Frame Terminate */
#define I_WF_TERM        (1 << 26)    /* Write Frame Terminate */
#define I_PCMCIA_XU        (1 << 27)    /* PCMCIA Transmit FIFO Underflow */
#define I_SBINT            (1 << 28)    /* sbintstatus Interrupt */
#define I_CHIPACTIVE        (1 << 29)    /* chip transitioned from doze to active state */
#define I_SRESET            (1 << 30)    /* CCCR RES interrupt */
#define I_IOE2            (1U << 31)    /* CCCR IOE2 Bit Changed */

#define I_SMB_SW_MASK    0x0000000f    /* To SB Mail S/W interrupts mask */
//#define I_HMB_SW_MASK    0x000000f0    /* To Host Mail S/W interrupts mask */

#define I_SMB_SW_SHIFT    0        /* To SB Mail S/W interrupts shift */
#define I_HMB_SW_SHIFT    4        /* To Host Mail S/W interrupts shift */

#define I_DMA_ERRORS        (I_PC | I_PD | I_DE | I_RU | I_RO | I_XU)
#define I_RW_ERRORS        (I_WR_OOSYNC | I_RD_OOSYNC)


// Micky ++
#define I_HMB_FC_STATE    I_HMB_SW0    /* To Host Mailbox Flow Control State */
#define I_HMB_FC_CHANGE    I_HMB_SW1    /* To Host Mailbox Flow Control State Changed */
#define I_HMB_FRAME_IND    I_HMB_SW2    /* To Host Mailbox Frame Indication */
#define I_HMB_HOST_INT    I_HMB_SW3    /* To Host Mailbox Miscellaneous Interrupt */

#define I_TOHOSTMAIL    (I_HMB_FC_CHANGE | I_HMB_FRAME_IND | I_HMB_HOST_INT)

/* tohostmailbox bits corresponding to intstatus bits */
#define HMB_FC_ON    (1 << 0)    /* To Host Mailbox Flow Control State */
#define HMB_FC_CHANGE    (1 << 1)    /* To Host Mailbox Flow Control State Changed */
#define HMB_FRAME_IND    (1 << 2)    /* To Host Mailbox Frame Indication */
#define HMB_HOST_INT    (1 << 3)    /* To Host Mailbox Miscellaneous Interrupt */
#define HMB_MASK    0x0000000f    /* To Host Mailbox Mask */

/* tohostmailboxdata */
#define HMB_DATA_NAKHANDLED    0x01    /* we're ready to retransmit NAK'd frame to host */
#define HMB_DATA_DEVREADY    0x02    /* we're ready to to talk to host after enable */
#define HMB_DATA_FC        0x04    /* per prio flowcontrol update flag to host */
#define HMB_DATA_FWREADY    0x08    /* firmware is ready for protocol activity */
#define HMB_DATA_FWHALT        0x10    /* firmware has halted operation */

#define HMB_DATA_FCDATA_MASK    0xff000000    /* per prio flowcontrol data */
#define HMB_DATA_FCDATA_SHIFT    24        /* per prio flowcontrol data */

#define HMB_DATA_VERSION_MASK    0x00ff0000    /* device protocol version (with devready) */
#define HMB_DATA_VERSION_SHIFT    16        /* device protocol version (with devready) */
// Micky --

/******************************************************
 *             SPI Constants
 ******************************************************/
/* GSPI v1 */
#define SPI_FRAME_CONTROL              ( (uint32_t) 0x1000D )

/* Register addresses */
#define SPI_BUS_CONTROL                ( (uint32_t) 0x0000 )
#define SPI_RESPONSE_DELAY             ( (uint32_t) 0x0001 )
#define SPI_STATUS_ENABLE              ( (uint32_t) 0x0002 )
#define SPI_RESET_BP                   ( (uint32_t) 0x0003 )    /* (corerev >= 1) */
#define SPI_INTERRUPT_REGISTER         ( (uint32_t) 0x0004 )    /* 16 bits - Interrupt status */
#define SPI_INTERRUPT_ENABLE_REGISTER  ( (uint32_t) 0x0006 )    /* 16 bits - Interrupt mask */
#define SPI_STATUS_REGISTER            ( (uint32_t) 0x0008 )    /* 32 bits */
#define SPI_FUNCTION1_INFO             ( (uint32_t) 0x000C )    /* 16 bits */
#define SPI_FUNCTION2_INFO             ( (uint32_t) 0x000E )    /* 16 bits */
#define SPI_FUNCTION3_INFO             ( (uint32_t) 0x0010 )    /* 16 bits */
#define SPI_READ_TEST_REGISTER         ( (uint32_t) 0x0014 )    /* 32 bits */
#define SPI_RESP_DELAY_F0              ( (uint32_t) 0x001c )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F1              ( (uint32_t) 0x001d )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F2              ( (uint32_t) 0x001e )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F3              ( (uint32_t) 0x001f )    /* 8 bits (corerev >= 3) */

/******************************************************
 *             Bit Masks
 ******************************************************/

/* SDIOD_CCCR_REV Bits */
#define SDIO_REV_SDIOID_MASK       ( (uint32_t)  0xF0 )    /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK       ( (uint32_t)  0x0F )    /* CCCR format version number */

/* SDIOD_CCCR_SDREV Bits */
#define SD_REV_PHY_MASK            ( (uint32_t)  0x0F )    /* SD format version number */

/* SDIOD_CCCR_IOEN Bits */
#define SDIO_FUNC_ENABLE_1         ( (uint32_t)  0x02 )    /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2         ( (uint32_t)  0x04 )    /* function 2 I/O enable */
#define SDIO_FUNC_ENABLE_3         ( (uint32_t)  0x08 )    /* function 3 I/O enable */

/* SDIOD_CCCR_IORDY Bits */
#define SDIO_FUNC_READY_1          ( (uint32_t)  0x02 )    /* function 1 I/O ready */
#define SDIO_FUNC_READY_2          ( (uint32_t)  0x04 )    /* function 2 I/O ready */
#define SDIO_FUNC_READY_3          ( (uint32_t)  0x08 )    /* function 3 I/O ready */

/* SDIOD_CCCR_INTEN Bits */
#define INTR_CTL_MASTER_EN         ( (uint32_t)  0x01 )    /* interrupt enable master */
#define INTR_CTL_FUNC1_EN          ( (uint32_t)  0x02 )    /* interrupt enable for function 1 */
#define INTR_CTL_FUNC2_EN          ( (uint32_t)  0x04 )    /* interrupt enable for function 2 */

/* SDIOD_SEP_INT_CTL Bits */
#define SEP_INTR_CTL_MASK          ( (uint32_t)  0x01 )    /* out-of-band interrupt mask */
#define SEP_INTR_CTL_EN            ( (uint32_t)  0x02 )    /* out-of-band interrupt output enable */
#define SEP_INTR_CTL_POL           ( (uint32_t)  0x04 )    /* out-of-band interrupt polarity */

/* SDIOD_CCCR_INTPEND Bits */
#define INTR_STATUS_FUNC1          ( (uint32_t)  0x02 )    /* interrupt pending for function 1 */
#define INTR_STATUS_FUNC2          ( (uint32_t)  0x04 )    /* interrupt pending for function 2 */
#define INTR_STATUS_FUNC3          ( (uint32_t)  0x08 )    /* interrupt pending for function 3 */

/* SDIOD_CCCR_IOABORT Bits */
#define IO_ABORT_RESET_ALL         ( (uint32_t)  0x08 )    /* I/O card reset */
#define IO_ABORT_FUNC_MASK         ( (uint32_t)  0x07 )    /* abort selction: function x */

/* SDIOD_CCCR_BICTRL Bits */
#define BUS_CARD_DETECT_DIS        ( (uint32_t)  0x80 )    /* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP      ( (uint32_t)  0x40 )    /* support continuous SPI interrupt */
#define BUS_SPI_CONT_INTR_EN       ( (uint32_t)  0x20 )    /* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK     ( (uint32_t)  0x03 )    /* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT     ( (uint32_t)  0x02 )    /* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT     ( (uint32_t)  0x00 )    /* bus width 1-bit mode */

/* SDIOD_CCCR_CAPABLITIES Bits */
#define SDIO_CAP_4BLS              ( (uint32_t)  0x80 )    /* 4-bit support for low speed card */
#define SDIO_CAP_LSC               ( (uint32_t)  0x40 )    /* low speed card */
#define SDIO_CAP_E4MI              ( (uint32_t)  0x20 )    /* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI              ( (uint32_t)  0x10 )    /* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS               ( (uint32_t)  0x08 )    /* support suspend/resume */
#define SDIO_CAP_SRW               ( (uint32_t)  0x04 )    /* support read wait */
#define SDIO_CAP_SMB               ( (uint32_t)  0x02 )    /* support multi-block transfer */
#define SDIO_CAP_SDC               ( (uint32_t)  0x01 )    /* Support Direct commands during multi-byte transfer */

/* SDIOD_CCCR_POWER_CONTROL Bits */
#define SDIO_POWER_SMPC            ( (uint32_t)  0x01 )    /* supports master power control (RO) */
#define SDIO_POWER_EMPC            ( (uint32_t)  0x02 )    /* enable master power control (allow > 200mA) (RW) */

/* SDIOD_CCCR_SPEED_CONTROL Bits */
#define SDIO_SPEED_SHS             ( (uint32_t)  0x01 )    /* supports high-speed [clocking] mode (RO) */
#define SDIO_SPEED_EHS             ( (uint32_t)  0x02 )    /* enable high-speed [clocking] mode (RW) */

/* SDIOD_CCCR_BRCM_CARDCAP Bits */
#define SDIOD_CCCR_BRCM_CARDCAP_CMD14_SUPPORT ( (uint32_t)  0x02 )  /* Supports CMD14 */
#define SDIOD_CCCR_BRCM_CARDCAP_CMD14_EXT     ( (uint32_t)  0x04 )  /* CMD14 is allowed in FSM command state */
#define SDIOD_CCCR_BRCM_CARDCAP_CMD_NODEC     ( (uint32_t)  0x08 )  /* sdiod_aos does not decode any command */


/* GSPI */
#define SPI_READ_TEST_REGISTER_VALUE   ( (uint32_t) 0xFEEDBEAD )
#define SPI_READ_TEST_REG_LSB          ( ( ( SPI_READ_TEST_REGISTER_VALUE      )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT1     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT2     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 ) + 1 ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT3     ( ( ( SPI_READ_TEST_REGISTER_VALUE +1  ) << 1 ) & 0xff )


/* SPI_BUS_CONTROL Bits */
#define WORD_LENGTH_32             ( (uint32_t)  0x01 )    /* 0/1 16/32 bit word length */
#define ENDIAN_BIG                 ( (uint32_t)  0x02 )    /* 0/1 Little/Big Endian */
#define CLOCK_PHASE                ( (uint32_t)  0x04 )    /* 0/1 clock phase delay */
#define CLOCK_POLARITY             ( (uint32_t)  0x08 )    /* 0/1 Idle state clock polarity is low/high */
#define HIGH_SPEED_MODE            ( (uint32_t)  0x10 )    /* 1/0 High Speed mode / Normal mode */
#define INTR_POLARITY_HIGH         ( (uint32_t)  0x20 )    /* 1/0 Interrupt active polarity is high/low */
#define WAKE_UP                    ( (uint32_t)  0x80 )    /* 0/1 Wake-up command from Host to WLAN */

/* SPI_RESPONSE_DELAY Bit mask */
#define RESPONSE_DELAY_MASK 0xFF    /* Configurable rd response delay in multiples of 8 bits */

/* SPI_STATUS_ENABLE Bits */
#define STATUS_ENABLE              ( (uint32_t)  0x01 )    /* 1/0 Status sent/not sent to host after read/write */
#define INTR_WITH_STATUS           ( (uint32_t)  0x02 )    /* 0/1 Do-not / do-interrupt if status is sent */
#define RESP_DELAY_ALL             ( (uint32_t)  0x04 )    /* Applicability of resp delay to F1 or all func's read */
#define DWORD_PKT_LEN_EN           ( (uint32_t)  0x08 )    /* Packet len denoted in dwords instead of bytes */
#define CMD_ERR_CHK_EN             ( (uint32_t)  0x20 )    /* Command error check enable */
#define DATA_ERR_CHK_EN            ( (uint32_t)  0x40 )    /* Data error check enable */



/* SPI_RESET_BP Bits*/
#define RESET_ON_WLAN_BP_RESET     ( (uint32_t)  0x04 )    /* enable reset for WLAN backplane */
//#define RESET_ON_BT_BP_RESET       ( (uint32_t)  0x08 )    /* enable reset for BT backplane */
#define RESET_SPI                  ( (uint32_t)  0x80 )    /* reset the above enabled logic */



/* SPI_INTERRUPT_REGISTER and SPI_INTERRUPT_ENABLE_REGISTER Bits */
#define DATA_UNAVAILABLE           ( (uint32_t) 0x0001 )   /* Requested data not available; Clear by writing a "1" */
#define F2_F3_FIFO_RD_UNDERFLOW    ( (uint32_t) 0x0002 )
#define F2_F3_FIFO_WR_OVERFLOW     ( (uint32_t) 0x0004 )
#define COMMAND_ERROR              ( (uint32_t) 0x0008 )   /* Cleared by writing 1 */
#define DATA_ERROR                 ( (uint32_t) 0x0010 )   /* Cleared by writing 1 */
#define F2_PACKET_AVAILABLE        ( (uint32_t) 0x0020 )
#define F3_PACKET_AVAILABLE        ( (uint32_t) 0x0040 )
#define F1_OVERFLOW                ( (uint32_t) 0x0080 )   /* Due to last write. Bkplane has pending write requests */
#define MISC_INTR0                 ( (uint32_t) 0x0100 )
#define MISC_INTR1                 ( (uint32_t) 0x0200 )
#define MISC_INTR2                 ( (uint32_t) 0x0400 )
#define MISC_INTR3                 ( (uint32_t) 0x0800 )
#define MISC_INTR4                 ( (uint32_t) 0x1000 )
#define F1_INTR                    ( (uint32_t) 0x2000 )
#define F2_INTR                    ( (uint32_t) 0x4000 )
#define F3_INTR                    ( (uint32_t) 0x8000 )




/* SPI_STATUS_REGISTER Bits */
#define STATUS_DATA_NOT_AVAILABLE  ( (uint32_t) 0x00000001 )
#define STATUS_UNDERFLOW           ( (uint32_t) 0x00000002 )
#define STATUS_OVERFLOW            ( (uint32_t) 0x00000004 )
#define STATUS_F2_INTR             ( (uint32_t) 0x00000008 )
#define STATUS_F3_INTR             ( (uint32_t) 0x00000010 )
#define STATUS_F2_RX_READY         ( (uint32_t) 0x00000020 )
#define STATUS_F3_RX_READY         ( (uint32_t) 0x00000040 )
#define STATUS_HOST_CMD_DATA_ERR   ( (uint32_t) 0x00000080 )
#define STATUS_F2_PKT_AVAILABLE    ( (uint32_t) 0x00000100 )
#define STATUS_F2_PKT_LEN_MASK     ( (uint32_t) 0x000FFE00 )
#define STATUS_F2_PKT_LEN_SHIFT    ( (uint32_t) 9          )
#define STATUS_F3_PKT_AVAILABLE    ( (uint32_t) 0x00100000 )
#define STATUS_F3_PKT_LEN_MASK     ( (uint32_t) 0xFFE00000 )
#define STATUS_F3_PKT_LEN_SHIFT    ( (uint32_t) 21         )





/* SDIO_CHIP_CLOCK_CSR Bits */
#define SBSDIO_FORCE_ALP           ( (uint32_t)  0x01 )    /* Force ALP request to backplane */
#define SBSDIO_FORCE_HT            ( (uint32_t)  0x02 )    /* Force HT request to backplane */
#define SBSDIO_FORCE_ILP           ( (uint32_t)  0x04 )    /* Force ILP request to backplane */
#define SBSDIO_ALP_AVAIL_REQ       ( (uint32_t)  0x08 )    /* Make ALP ready (power up xtal) */
#define SBSDIO_HT_AVAIL_REQ        ( (uint32_t)  0x10 )    /* Make HT ready (power up PLL) */
#define SBSDIO_FORCE_HW_CLKREQ_OFF ( (uint32_t)  0x20 )    /* Squelch clock requests from HW */
#define SBSDIO_ALP_AVAIL           ( (uint32_t)  0x40 )    /* Status: ALP is ready */
#define SBSDIO_HT_AVAIL            ( (uint32_t)  0x80 )    /* Status: HT is ready */
#define SBSDIO_Rev8_HT_AVAIL       ( (uint32_t)  0x40 )
#define SBSDIO_Rev8_ALP_AVAIL      ( (uint32_t)  0x80 )


/* SDIO_FRAME_CONTROL Bits */
#define SFC_RF_TERM                ( (uint32_t) (1 << 0) ) /* Read Frame Terminate */
#define SFC_WF_TERM                ( (uint32_t) (1 << 1) ) /* Write Frame Terminate */
#define SFC_CRC4WOOS               ( (uint32_t) (1 << 2) ) /* HW reports CRC error for write out of sync */
#define SFC_ABORTALL               ( (uint32_t) (1 << 3) ) /* Abort cancels all in-progress frames */

/* SDIO_TO_SB_MAIL_BOX bits corresponding to intstatus bits */
#define SMB_NAK                    ( (uint32_t) (1 << 0) ) /* To SB Mailbox Frame NAK */
#define SMB_INT_ACK                ( (uint32_t) (1 << 1) ) /* To SB Mailbox Host Interrupt ACK */
#define SMB_USE_OOB                ( (uint32_t) (1 << 2) ) /* To SB Mailbox Use OOB Wakeup */
#define SMB_DEV_INT                ( (uint32_t) (1 << 3) ) /* To SB Mailbox Miscellaneous Interrupt */

/* SDIO_WAKEUP_CTRL bits */
#define SBSDIO_WCTRL_WAKE_TILL_ALP_AVAIL     ( (uint32_t) (1 << 0) ) /* WakeTillAlpAvail bit */
#define SBSDIO_WCTRL_WAKE_TILL_HT_AVAIL      ( (uint32_t) (1 << 1) ) /* WakeTillHTAvail bit */

/* SDIO_SLEEP_CSR bits */
#define SBSDIO_SLPCSR_KEEP_SDIO_ON           ( (uint32_t) (1 << 0) ) /* KeepSdioOn bit */
#define SBSDIO_SLPCSR_DEVICE_ON              ( (uint32_t) (1 << 1) ) /* DeviceOn bit */


#define WL_CHANSPEC_BAND_MASK      0xc000
#define WL_CHANSPEC_BAND_SHIFT         14
#define WL_CHANSPEC_BAND_2G        0x0000
#define WL_CHANSPEC_BAND_5G        0xc000

#define WL_CHANSPEC_CTL_SB_MASK    0x0700
#define WL_CHANSPEC_CTL_SB_SHIFT        8
#define WL_CHANSPEC_CTL_SB_LLL     0x0000
#define WL_CHANSPEC_CTL_SB_LLU     0x0100
#define WL_CHANSPEC_CTL_SB_LUL     0x0200
#define WL_CHANSPEC_CTL_SB_LUU     0x0300
#define WL_CHANSPEC_CTL_SB_ULL     0x0400
#define WL_CHANSPEC_CTL_SB_ULU     0x0500
#define WL_CHANSPEC_CTL_SB_UUL     0x0600
#define WL_CHANSPEC_CTL_SB_UUU     0x0700
#define WL_CHANSPEC_CTL_SB_LL      WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_LU      WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_UL      WL_CHANSPEC_CTL_SB_LUL
#define WL_CHANSPEC_CTL_SB_UU      WL_CHANSPEC_CTL_SB_LUU
#define WL_CHANSPEC_CTL_SB_L       WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_U       WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_LOWER   WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_CTL_SB_UPPER   WL_CHANSPEC_CTL_SB_LLU
#define WL_CHANSPEC_CTL_SB_NONE    WL_CHANSPEC_CTL_SB_LLL
#define WL_CHANSPEC_BW_MASK        0x3800
#define WL_CHANSPEC_BW_SHIFT           11
#define WL_CHANSPEC_BW_10          0x0800
#define WL_CHANSPEC_BW_20          0x1000
#define WL_CHANSPEC_BW_40          0x1800


/* CIS accesses require backpane clock */


#undef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
#undef CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR

struct ether_addr;
struct wl_join_scan_params;

typedef struct wl_assoc_params
{
    struct ether_addr bssid;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
    uint16_t    bssid_cnt;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
    uint32_t     chanspec_num;
    chanspec_t  chanspec_list[1];
} wl_assoc_params_t;
#define WL_ASSOC_PARAMS_FIXED_SIZE     (sizeof(wl_assoc_params_t) - sizeof(wl_chanspec_t))
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE    WL_ASSOC_PARAMS_FIXED_SIZE
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE        WL_ASSOC_PARAMS_FIXED_SIZE
typedef struct wl_join_params
{
    wlc_ssid_t         ssid;
    struct wl_assoc_params  params;
} wl_join_params_t;
#define WL_JOIN_PARAMS_FIXED_SIZE     (sizeof(wl_join_params_t) - sizeof(wl_chanspec_t))

/* extended join params */
typedef struct wl_extjoin_params
{
        wlc_ssid_t ssid; /* {0, ""}: wildcard scan */
        struct wl_join_scan_params scan_params;
        wl_join_assoc_params_t assoc_params; /* optional field, but it must include the fixed portion of the wl_join_assoc_params_t struct when it does present. */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE    (sizeof(wl_extjoin_params_t) - sizeof(chanspec_t))

//typedef wl_cnt_ver_eight_t  wiced_counters_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ifndef INCLUDED_CHIP_CONSTANTS_H_ */
#endif /* AP6255*/

#ifdef _WIFI_AP6212


#ifndef INCLUDED_CHIP_6212_CONSTANTS_H_
#define INCLUDED_CHIP_6212_CONSTANTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************
 *             Architecture Constants
 ******************************************************/

/* General chip stats */
#define CHIP_RAM_SIZE      (512*1024)

/* Backplane architecture */
#define CHIPCOMMON_BASE_ADDRESS  0x18000000    /* Chipcommon core register region   */
#define DOT11MAC_BASE_ADDRESS    0x18001000    /* dot11mac core register region     */
#define SDIO_BASE_ADDRESS        0x18002000    /* SDIOD Device core register region */
#define WLAN_ARMCM3_BASE_ADDRESS 0x18003000    /* ARMCM3 core register region       */
#define SOCSRAM_BASE_ADDRESS     0x18004000    /* SOCSRAM core register region      */
#define BACKPLANE_ADDRESS_MASK   0x7FFF

#define WLAN_MEM_BASE_ADDRESS   0x00

#define CHIP_STA_INTERFACE   0
#define CHIP_AP_INTERFACE    1

/* Maximum value of bus data credit difference */
#define CHIP_MAX_BUS_DATA_CREDIT_DIFF    20

/* Chipcommon registers */
#define CHIPCOMMON_GPIO_CONTROL ((uint32_t) (CHIPCOMMON_BASE_ADDRESS + 0x6C) )

/******************************************************
 *             SDIO Constants
 ******************************************************/
/* CurrentSdiodProgGuide r23 */

/* Base registers */
#define SDIO_CORE                    ((uint32_t) (SDIO_BASE_ADDRESS + 0x00) )
#define SDIO_INT_STATUS              ((uint32_t) (SDIO_BASE_ADDRESS + 0x20) )
#define SDIO_TO_SB_MAILBOX           ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_TO_SB_MAILBOX_DATA      ((uint32_t) (SDIO_BASE_ADDRESS + 0x48) )
#define SDIO_TO_HOST_MAILBOX_DATA    ((uint32_t) (SDIO_BASE_ADDRESS + 0x4C) )
#define SDIO_TO_SB_MAIL_BOX          ((uint32_t) (SDIO_BASE_ADDRESS + 0x40) )
#define SDIO_INT_HOST_MASK           ((uint32_t) (SDIO_BASE_ADDRESS + 0x24) )
#define SDIO_FUNCTION_INT_MASK       ((uint32_t) (SDIO_BASE_ADDRESS + 0x34) )


/* SDIO Function 0 (SDIO Bus) register addresses */

/* SDIO Device CCCR offsets */
/* TODO: What does CIS/CCCR stand for? */
/* CCCR accesses do not require backpane clock */
#define SDIOD_CCCR_REV             ( (uint32_t)  0x00 )    /* CCCR/SDIO Revision */
#define SDIOD_CCCR_SDREV           ( (uint32_t)  0x01 )    /* SD Revision */
#define SDIOD_CCCR_IOEN            ( (uint32_t)  0x02 )    /* I/O Enable */
#define SDIOD_CCCR_IORDY           ( (uint32_t)  0x03 )    /* I/O Ready */
#define SDIOD_CCCR_INTEN           ( (uint32_t)  0x04 )    /* Interrupt Enable */
#define SDIOD_CCCR_INTPEND         ( (uint32_t)  0x05 )    /* Interrupt Pending */
#define SDIOD_CCCR_IOABORT         ( (uint32_t)  0x06 )    /* I/O Abort */
#define SDIOD_CCCR_BICTRL          ( (uint32_t)  0x07 )    /* Bus Interface control */
#define SDIOD_CCCR_CAPABLITIES     ( (uint32_t)  0x08 )    /* Card Capabilities */
#define SDIOD_CCCR_CISPTR_0        ( (uint32_t)  0x09 )    /* Common CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_CISPTR_1        ( (uint32_t)  0x0A )    /* Common CIS Base Address Pointer Register 1 */
#define SDIOD_CCCR_CISPTR_2        ( (uint32_t)  0x0B )    /* Common CIS Base Address Pointer Register 2 (MSB - only bit 1 valid)*/
#define SDIOD_CCCR_BUSSUSP         ( (uint32_t)  0x0C )    /*  */
#define SDIOD_CCCR_FUNCSEL         ( (uint32_t)  0x0D )    /*  */
#define SDIOD_CCCR_EXECFLAGS       ( (uint32_t)  0x0E )    /*  */
#define SDIOD_CCCR_RDYFLAGS        ( (uint32_t)  0x0F )    /*  */
#define SDIOD_CCCR_BLKSIZE_0       ( (uint32_t)  0x10 )    /* Function 0 (Bus) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_BLKSIZE_1       ( (uint32_t)  0x11 )    /* Function 0 (Bus) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_POWER_CONTROL   ( (uint32_t)  0x12 )    /* Power Control */
#define SDIOD_CCCR_SPEED_CONTROL   ( (uint32_t)  0x13 )    /* Bus Speed Select  (control device entry into high-speed clocking mode)  */
#define SDIOD_CCCR_UHS_I           ( (uint32_t)  0x14 )    /* UHS-I Support */
#define SDIOD_CCCR_DRIVE           ( (uint32_t)  0x15 )    /* Drive Strength */
#define SDIOD_CCCR_INTEXT          ( (uint32_t)  0x16 )    /* Interrupt Extension */
#define SDIOD_SEP_INT_CTL          ( (uint32_t)  0xF2 )    /* Separate Interrupt Control*/
#define SDIOD_CCCR_F1INFO          ( (uint32_t) 0x100 )    /* Function 1 (Backplane) Info */
#define SDIOD_CCCR_F1HP            ( (uint32_t) 0x102 )    /* Function 1 (Backplane) High Power */
#define SDIOD_CCCR_F1CISPTR_0      ( (uint32_t) 0x109 )    /* Function 1 (Backplane) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F1CISPTR_1      ( (uint32_t) 0x10A )    /* Function 1 (Backplane) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F1CISPTR_2      ( (uint32_t) 0x10B )    /* Function 1 (Backplane) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F1BLKSIZE_0     ( (uint32_t) 0x110 )    /* Function 1 (Backplane) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F1BLKSIZE_1     ( (uint32_t) 0x111 )    /* Function 1 (Backplane) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F2INFO          ( (uint32_t) 0x200 )    /* Function 2 (WLAN Data FIFO) Info */
#define SDIOD_CCCR_F2HP            ( (uint32_t) 0x202 )    /* Function 2 (WLAN Data FIFO) High Power */
#define SDIOD_CCCR_F2CISPTR_0      ( (uint32_t) 0x209 )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F2CISPTR_1      ( (uint32_t) 0x20A )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F2CISPTR_2      ( (uint32_t) 0x20B )    /* Function 2 (WLAN Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F2BLKSIZE_0     ( (uint32_t) 0x210 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F2BLKSIZE_1     ( (uint32_t) 0x211 )    /* Function 2 (WLAN Data FIFO) SDIO Block Size Register 1 (MSB) */
#define SDIOD_CCCR_F3INFO          ( (uint32_t) 0x300 )    /* Function 3 (Bluetooth Data FIFO) Info */
#define SDIOD_CCCR_F3HP            ( (uint32_t) 0x302 )    /* Function 3 (Bluetooth Data FIFO) High Power */
#define SDIOD_CCCR_F3CISPTR_0      ( (uint32_t) 0x309 )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 0 (LSB) */
#define SDIOD_CCCR_F3CISPTR_1      ( (uint32_t) 0x30A )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 1       */
#define SDIOD_CCCR_F3CISPTR_2      ( (uint32_t) 0x30B )    /* Function 3 (Bluetooth Data FIFO) CIS Base Address Pointer Register 2 (MSB - only bit 1 valid) */
#define SDIOD_CCCR_F3BLKSIZE_0     ( (uint32_t) 0x310 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 0 (LSB) */
#define SDIOD_CCCR_F3BLKSIZE_1     ( (uint32_t) 0x311 )    /* Function 3 (Bluetooth Data FIFO) SDIO Block Size Register 1 (MSB) */


/* SDIO Function 1 (Backplane) register addresses */
/* Addresses 0x00000000 - 0x0000FFFF are directly access the backplane
 * throught the backplane window. Addresses above 0x0000FFFF are
 * registers relating to backplane access, and do not require a backpane
 * clock to access them
 */
#define SDIO_GPIO_SELECT              ( (uint32_t) 0x10005 )
#define SDIO_GPIO_OUTPUT              ( (uint32_t) 0x10006 )
#define SDIO_GPIO_ENABLE              ( (uint32_t) 0x10007 )
#define SDIO_FUNCTION2_WATERMARK      ( (uint32_t) 0x10008 )
#define SDIO_DEVICE_CONTROL           ( (uint32_t) 0x10009 )
#define SDIO_BACKPLANE_ADDRESS_LOW    ( (uint32_t) 0x1000A )
#define SDIO_BACKPLANE_ADDRESS_MID    ( (uint32_t) 0x1000B )
#define SDIO_BACKPLANE_ADDRESS_HIGH   ( (uint32_t) 0x1000C )
#define SDIO_FRAME_CONTROL            ( (uint32_t) 0x1000D )
#define SDIO_CHIP_CLOCK_CSR           ( (uint32_t) 0x1000E )
#define SDIO_PULL_UP                  ( (uint32_t) 0x1000F )
#define SDIO_READ_FRAME_BC_LOW        ( (uint32_t) 0x1001B )
#define SDIO_READ_FRAME_BC_HIGH       ( (uint32_t) 0x1001C )

/* Micky ++ */
#define SDIO_SB_ACCESS_2_4B_FLAG      0x08000        /* with b15, maps to 32-bit SB access */

#define I_HMB_SW_MASK                 ( (uint32_t) 0x000000F0 )
#define I_HMB_FRAME_IND               ( 1<<6 )
#define FRAME_AVAILABLE_MASK          I_HMB_SW_MASK

/******************************************************
 *             SPI Constants
 ******************************************************/
/* GSPI v1 */
#define SPI_FRAME_CONTROL              ( (uint32_t) 0x1000D )

/* Register addresses */
#define SPI_BUS_CONTROL                ( (uint32_t) 0x0000 )
#define SPI_RESPONSE_DELAY             ( (uint32_t) 0x0001 )
#define SPI_STATUS_ENABLE              ( (uint32_t) 0x0002 )
#define SPI_RESET_BP                   ( (uint32_t) 0x0003 )    /* (corerev >= 1) */
#define SPI_INTERRUPT_REGISTER         ( (uint32_t) 0x0004 )    /* 16 bits - Interrupt status */
#define SPI_INTERRUPT_ENABLE_REGISTER  ( (uint32_t) 0x0006 )    /* 16 bits - Interrupt mask */
#define SPI_STATUS_REGISTER            ( (uint32_t) 0x0008 )    /* 32 bits */
#define SPI_FUNCTION1_INFO             ( (uint32_t) 0x000C )    /* 16 bits */
#define SPI_FUNCTION2_INFO             ( (uint32_t) 0x000E )    /* 16 bits */
#define SPI_FUNCTION3_INFO             ( (uint32_t) 0x0010 )    /* 16 bits */
#define SPI_READ_TEST_REGISTER         ( (uint32_t) 0x0014 )    /* 32 bits */
#define SPI_RESP_DELAY_F0              ( (uint32_t) 0x001c )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F1              ( (uint32_t) 0x001d )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F2              ( (uint32_t) 0x001e )    /* 8 bits (corerev >= 3) */
#define SPI_RESP_DELAY_F3              ( (uint32_t) 0x001f )    /* 8 bits (corerev >= 3) */

/******************************************************
 *             Bit Masks
 ******************************************************/

/* SDIOD_CCCR_REV Bits */
#define SDIO_REV_SDIOID_MASK       ( (uint32_t)  0xF0 )    /* SDIO spec revision number */
#define SDIO_REV_CCCRID_MASK       ( (uint32_t)  0x0F )    /* CCCR format version number */

/* SDIOD_CCCR_SDREV Bits */
#define SD_REV_PHY_MASK            ( (uint32_t)  0x0F )    /* SD format version number */

/* SDIOD_CCCR_IOEN Bits */
#define SDIO_FUNC_ENABLE_1         ( (uint32_t)  0x02 )    /* function 1 I/O enable */
#define SDIO_FUNC_ENABLE_2         ( (uint32_t)  0x04 )    /* function 2 I/O enable */
#define SDIO_FUNC_ENABLE_3         ( (uint32_t)  0x08 )    /* function 3 I/O enable */

/* SDIOD_CCCR_IORDY Bits */
#define SDIO_FUNC_READY_1          ( (uint32_t)  0x02 )    /* function 1 I/O ready */
#define SDIO_FUNC_READY_2          ( (uint32_t)  0x04 )    /* function 2 I/O ready */
#define SDIO_FUNC_READY_3          ( (uint32_t)  0x08 )    /* function 3 I/O ready */

/* SDIOD_CCCR_INTEN Bits */
#define INTR_CTL_MASTER_EN         ( (uint32_t)  0x01 )    /* interrupt enable master */
#define INTR_CTL_FUNC1_EN          ( (uint32_t)  0x02 )    /* interrupt enable for function 1 */
#define INTR_CTL_FUNC2_EN          ( (uint32_t)  0x04 )    /* interrupt enable for function 2 */

/* SDIOD_SEP_INT_CTL Bits */
#define SEP_INTR_CTL_MASK          ( (uint32_t)  0x01 )    /* out-of-band interrupt mask */
#define SEP_INTR_CTL_EN            ( (uint32_t)  0x02 )    /* out-of-band interrupt output enable */
#define SEP_INTR_CTL_POL           ( (uint32_t)  0x04 )    /* out-of-band interrupt polarity */

/* SDIOD_CCCR_INTPEND Bits */
#define INTR_STATUS_FUNC1          ( (uint32_t)  0x02 )    /* interrupt pending for function 1 */
#define INTR_STATUS_FUNC2          ( (uint32_t)  0x04 )    /* interrupt pending for function 2 */
#define INTR_STATUS_FUNC3          ( (uint32_t)  0x08 )    /* interrupt pending for function 3 */

/* SDIOD_CCCR_IOABORT Bits */
#define IO_ABORT_RESET_ALL         ( (uint32_t)  0x08 )    /* I/O card reset */
#define IO_ABORT_FUNC_MASK         ( (uint32_t)  0x07 )    /* abort selction: function x */

/* SDIOD_CCCR_BICTRL Bits */
#define BUS_CARD_DETECT_DIS        ( (uint32_t)  0x80 )    /* Card Detect disable */
#define BUS_SPI_CONT_INTR_CAP      ( (uint32_t)  0x40 )    /* support continuous SPI interrupt */
#define BUS_SPI_CONT_INTR_EN       ( (uint32_t)  0x20 )    /* continuous SPI interrupt enable */
#define BUS_SD_DATA_WIDTH_MASK     ( (uint32_t)  0x03 )    /* bus width mask */
#define BUS_SD_DATA_WIDTH_4BIT     ( (uint32_t)  0x02 )    /* bus width 4-bit mode */
#define BUS_SD_DATA_WIDTH_1BIT     ( (uint32_t)  0x00 )    /* bus width 1-bit mode */

/* SDIOD_CCCR_CAPABLITIES Bits */
#define SDIO_CAP_4BLS              ( (uint32_t)  0x80 )    /* 4-bit support for low speed card */
#define SDIO_CAP_LSC               ( (uint32_t)  0x40 )    /* low speed card */
#define SDIO_CAP_E4MI              ( (uint32_t)  0x20 )    /* enable interrupt between block of data in 4-bit mode */
#define SDIO_CAP_S4MI              ( (uint32_t)  0x10 )    /* support interrupt between block of data in 4-bit mode */
#define SDIO_CAP_SBS               ( (uint32_t)  0x08 )    /* support suspend/resume */
#define SDIO_CAP_SRW               ( (uint32_t)  0x04 )    /* support read wait */
#define SDIO_CAP_SMB               ( (uint32_t)  0x02 )    /* support multi-block transfer */
#define SDIO_CAP_SDC               ( (uint32_t)  0x01 )    /* Support Direct commands during multi-byte transfer */

/* SDIOD_CCCR_POWER_CONTROL Bits */
#define SDIO_POWER_SMPC            ( (uint32_t)  0x01 )    /* supports master power control (RO) */
#define SDIO_POWER_EMPC            ( (uint32_t)  0x02 )    /* enable master power control (allow > 200mA) (RW) */

/* SDIOD_CCCR_SPEED_CONTROL Bits */
#define SDIO_SPEED_SHS             ( (uint32_t)  0x01 )    /* supports high-speed [clocking] mode (RO) */
#define SDIO_SPEED_EHS             ( (uint32_t)  0x02 )    /* enable high-speed [clocking] mode (RW) */



/* GSPI */
#define SPI_READ_TEST_REGISTER_VALUE   ( (uint32_t) 0xFEEDBEAD )
#define SPI_READ_TEST_REG_LSB          ( ( ( SPI_READ_TEST_REGISTER_VALUE      )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT1     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 )     ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT2     ( ( ( SPI_READ_TEST_REGISTER_VALUE << 1 ) + 1 ) & 0xff )
#define SPI_READ_TEST_REG_LSB_SFT3     ( ( ( SPI_READ_TEST_REGISTER_VALUE +1  ) << 1 ) & 0xff )


/* SPI_BUS_CONTROL Bits */
#define WORD_LENGTH_32             ( (uint32_t)  0x01 )    /* 0/1 16/32 bit word length */
#define ENDIAN_BIG                 ( (uint32_t)  0x02 )    /* 0/1 Little/Big Endian */
#define CLOCK_PHASE                ( (uint32_t)  0x04 )    /* 0/1 clock phase delay */
#define CLOCK_POLARITY             ( (uint32_t)  0x08 )    /* 0/1 Idle state clock polarity is low/high */
#define HIGH_SPEED_MODE            ( (uint32_t)  0x10 )    /* 1/0 High Speed mode / Normal mode */
#define INTR_POLARITY_HIGH         ( (uint32_t)  0x20 )    /* 1/0 Interrupt active polarity is high/low */
#define WAKE_UP                    ( (uint32_t)  0x80 )    /* 0/1 Wake-up command from Host to WLAN */

/* SPI_RESPONSE_DELAY Bit mask */
#define RESPONSE_DELAY_MASK 0xFF    /* Configurable rd response delay in multiples of 8 bits */

/* SPI_STATUS_ENABLE Bits */
#define STATUS_ENABLE              ( (uint32_t)  0x01 )    /* 1/0 Status sent/not sent to host after read/write */
#define INTR_WITH_STATUS           ( (uint32_t)  0x02 )    /* 0/1 Do-not / do-interrupt if status is sent */
#define RESP_DELAY_ALL             ( (uint32_t)  0x04 )    /* Applicability of resp delay to F1 or all func's read */
#define DWORD_PKT_LEN_EN           ( (uint32_t)  0x08 )    /* Packet len denoted in dwords instead of bytes */
#define CMD_ERR_CHK_EN             ( (uint32_t)  0x20 )    /* Command error check enable */
#define DATA_ERR_CHK_EN            ( (uint32_t)  0x40 )    /* Data error check enable */



/* SPI_RESET_BP Bits*/
#define RESET_ON_WLAN_BP_RESET     ( (uint32_t)  0x04 )    /* enable reset for WLAN backplane */
//#define RESET_ON_BT_BP_RESET       ( (uint32_t)  0x08 )    /* enable reset for BT backplane */
#define RESET_SPI                  ( (uint32_t)  0x80 )    /* reset the above enabled logic */



/* SPI_INTERRUPT_REGISTER and SPI_INTERRUPT_ENABLE_REGISTER Bits */
#define DATA_UNAVAILABLE           ( (uint32_t) 0x0001 )   /* Requested data not available; Clear by writing a "1" */
#define F2_F3_FIFO_RD_UNDERFLOW    ( (uint32_t) 0x0002 )
#define F2_F3_FIFO_WR_OVERFLOW     ( (uint32_t) 0x0004 )
#define COMMAND_ERROR              ( (uint32_t) 0x0008 )   /* Cleared by writing 1 */
#define DATA_ERROR                 ( (uint32_t) 0x0010 )   /* Cleared by writing 1 */
#define F2_PACKET_AVAILABLE        ( (uint32_t) 0x0020 )
#define F3_PACKET_AVAILABLE        ( (uint32_t) 0x0040 )
#define F1_OVERFLOW                ( (uint32_t) 0x0080 )   /* Due to last write. Bkplane has pending write requests */
#define MISC_INTR0                 ( (uint32_t) 0x0100 )
#define MISC_INTR1                 ( (uint32_t) 0x0200 )
#define MISC_INTR2                 ( (uint32_t) 0x0400 )
#define MISC_INTR3                 ( (uint32_t) 0x0800 )
#define MISC_INTR4                 ( (uint32_t) 0x1000 )
#define F1_INTR                    ( (uint32_t) 0x2000 )
#define F2_INTR                    ( (uint32_t) 0x4000 )
#define F3_INTR                    ( (uint32_t) 0x8000 )




/* SPI_STATUS_REGISTER Bits */
#define STATUS_DATA_NOT_AVAILABLE  ( (uint32_t) 0x00000001 )
#define STATUS_UNDERFLOW           ( (uint32_t) 0x00000002 )
#define STATUS_OVERFLOW            ( (uint32_t) 0x00000004 )
#define STATUS_F2_INTR             ( (uint32_t) 0x00000008 )
#define STATUS_F3_INTR             ( (uint32_t) 0x00000010 )
#define STATUS_F2_RX_READY         ( (uint32_t) 0x00000020 )
#define STATUS_F3_RX_READY         ( (uint32_t) 0x00000040 )
#define STATUS_HOST_CMD_DATA_ERR   ( (uint32_t) 0x00000080 )
#define STATUS_F2_PKT_AVAILABLE    ( (uint32_t) 0x00000100 )
#define STATUS_F2_PKT_LEN_MASK     ( (uint32_t) 0x000FFE00 )
#define STATUS_F2_PKT_LEN_SHIFT    ( (uint32_t) 9          )
#define STATUS_F3_PKT_AVAILABLE    ( (uint32_t) 0x00100000 )
#define STATUS_F3_PKT_LEN_MASK     ( (uint32_t) 0xFFE00000 )
#define STATUS_F3_PKT_LEN_SHIFT    ( (uint32_t) 21         )





/* SDIO_CHIP_CLOCK_CSR Bits */
#define SBSDIO_FORCE_ALP           ( (uint32_t)  0x01 )    /* Force ALP request to backplane */
#define SBSDIO_FORCE_HT            ( (uint32_t)  0x02 )    /* Force HT request to backplane */
#define SBSDIO_FORCE_ILP           ( (uint32_t)  0x04 )    /* Force ILP request to backplane */
#define SBSDIO_ALP_AVAIL_REQ       ( (uint32_t)  0x08 )    /* Make ALP ready (power up xtal) */
#define SBSDIO_HT_AVAIL_REQ        ( (uint32_t)  0x10 )    /* Make HT ready (power up PLL) */
#define SBSDIO_FORCE_HW_CLKREQ_OFF ( (uint32_t)  0x20 )    /* Squelch clock requests from HW */
#define SBSDIO_ALP_AVAIL           ( (uint32_t)  0x40 )    /* Status: ALP is ready */
#define SBSDIO_HT_AVAIL            ( (uint32_t)  0x80 )    /* Status: HT is ready */
#define SBSDIO_Rev8_HT_AVAIL       ( (uint32_t)  0x40 )
#define SBSDIO_Rev8_ALP_AVAIL      ( (uint32_t)  0x80 )


/* SDIO_FRAME_CONTROL Bits */
#define SFC_RF_TERM                ( (uint32_t) (1 << 0) ) /* Read Frame Terminate */
#define SFC_WF_TERM                ( (uint32_t) (1 << 1) ) /* Write Frame Terminate */
#define SFC_CRC4WOOS               ( (uint32_t) (1 << 2) ) /* HW reports CRC error for write out of sync */
#define SFC_ABORTALL               ( (uint32_t) (1 << 3) ) /* Abort cancels all in-progress frames */

/* SDIO_TO_SB_MAIL_BOX bits corresponding to intstatus bits */
#define SMB_NAK                    ( (uint32_t) (1 << 0) ) /* To SB Mailbox Frame NAK */
#define SMB_INT_ACK                ( (uint32_t) (1 << 1) ) /* To SB Mailbox Host Interrupt ACK */
#define SMB_USE_OOB                ( (uint32_t) (1 << 2) ) /* To SB Mailbox Use OOB Wakeup */
#define SMB_DEV_INT                ( (uint32_t) (1 << 3) ) /* To SB Mailbox Miscellaneous Interrupt */

#define WL_CHANSPEC_BAND_MASK        0xf000
#define WL_CHANSPEC_BAND_SHIFT        12
#define WL_CHANSPEC_BAND_5G        0x1000
#define WL_CHANSPEC_BAND_2G        0x2000
#define INVCHANSPEC            255

#define WL_CHANSPEC_CTL_SB_MASK        0x0300
#define WL_CHANSPEC_CTL_SB_SHIFT         8
#define WL_CHANSPEC_CTL_SB_LOWER    0x0100
#define WL_CHANSPEC_CTL_SB_UPPER    0x0200
#define WL_CHANSPEC_CTL_SB_NONE        0x0300

#define WL_CHANSPEC_BW_MASK        0x0C00
#define WL_CHANSPEC_BW_SHIFT            10
#define WL_CHANSPEC_BW_10        0x0400
#define WL_CHANSPEC_BW_20        0x0800
#define WL_CHANSPEC_BW_40        0x0C00

/* CIS accesses require backpane clock */


#undef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
#undef CHIP_FIRMWARE_SUPPORTS_PM_LIMIT_IOVAR

struct ether_addr;
struct wl_join_scan_params;

typedef struct wl_assoc_params
{
    struct ether_addr bssid;
#ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS
    uint16_t    bssid_cnt;
#endif /* ifdef CHIP_HAS_BSSID_CNT_IN_ASSOC_PARAMS */
    uint32_t     chanspec_num;
    chanspec_t  chanspec_list[1];
} wl_assoc_params_t;
#define WL_ASSOC_PARAMS_FIXED_SIZE     (sizeof(wl_assoc_params_t) - sizeof(wl_chanspec_t))
typedef wl_assoc_params_t wl_reassoc_params_t;
#define WL_REASSOC_PARAMS_FIXED_SIZE    WL_ASSOC_PARAMS_FIXED_SIZE
typedef wl_assoc_params_t wl_join_assoc_params_t;
#define WL_JOIN_ASSOC_PARAMS_FIXED_SIZE        WL_ASSOC_PARAMS_FIXED_SIZE
typedef struct wl_join_params
{
    wlc_ssid_t         ssid;
    struct wl_assoc_params  params;
} wl_join_params_t;
#define WL_JOIN_PARAMS_FIXED_SIZE     (sizeof(wl_join_params_t) - sizeof(wl_chanspec_t))

/* extended join params */
typedef struct wl_extjoin_params {
    wlc_ssid_t ssid;                /* {0, ""}: wildcard scan */
    struct wl_join_scan_params scan_params;
    wl_join_assoc_params_t assoc_params;   /* optional field, but it must include the fixed portion
                     * of the wl_join_assoc_params_t struct when it does
                     * present.
                     */
} wl_extjoin_params_t;
#define WL_EXTJOIN_PARAMS_FIXED_SIZE    (sizeof(wl_extjoin_params_t) - sizeof(chanspec_t))

//typedef wl_cnt_ver_six_t  wiced_counters_t;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif
#endif/*ap6212*/




