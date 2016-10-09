/*
********************************************************************************
*                   Copyright (c) 2008,yangwenjie
*                         All rights reserved.
*
* File Name£º   hw_spi.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_SPI_H_
#define _HW_SPI_H_

//SPIM_CTRLR0  SPIS_CTRLR0
#define SPI_MASTER_MODE             ((uint32)(0)<<20)
#define SPI_SLAVE_MODE              ((uint32)(1)<<20)

#define TRANSMIT_RECEIVE            ((uint32)(0)<<18)
#define TRANSMIT_ONLY               ((uint32)(1)<<18)
#define RECEIVE_ONLY                ((uint32)(2)<<18)
#define TRANSMIT_RECEIVE_MASK       ((uint32)(3)<<18)

#define MOTOROLA_SPI                ((uint32)(0)<<16)
#define TEXAS_INSTRUMENTS_SSP       ((uint32)(1)<<16)
#define NATIONAL_SEMI_MICROWIRE     ((uint32)(2)<<16)

#define RXD_SAMPLE_NO_DELAY         ((uint32)(0)<<14)
#define RXD_SAMPLE_1CK_DELAY        ((uint32)(1)<<14)
#define RXD_SAMPLE_2CK_DELAY        ((uint32)(2)<<14)
#define RXD_SAMPLE_3CK_DELAY        ((uint32)(3)<<14)

#define APB_BYTE_WR                 ((uint32)(1)<<13)
#define APB_HALFWORD_WR             ((uint32)(0)<<13)

#define MSB_FBIT                    ((uint32)(0)<<12)
#define LSB_FBIT                    ((uint32)(1)<<12)

#define LITTLE_ENDIAN_MODE          ((uint32)(0)<<11)
#define BIG_ENDIAN_MODE             ((uint32)(1)<<11)

#define CS_2_SCLK_OUT_1_2_CK        ((uint32)(0)<<10)
#define CS_2_SCLK_OUT_1_CK          ((uint32)(1)<<10)

#define CS_KEEP_LOW                 ((uint32)(0)<<8)
#define CS_KEEP_1_2_CK              ((uint32)(1)<<8)
#define CS_KEEP_1_CK                ((uint32)(2)<<8)

#define SERIAL_CLOCK_POLARITY_LOW   ((uint32)(0)<<7)
#define SERIAL_CLOCK_POLARITY_HIGH  ((uint32)(1)<<7)

#define SERIAL_CLOCK_PHASE_MIDDLE   ((uint32)(0)<<6)
#define SERIAL_CLOCK_PHASE_START    ((uint32)(1)<<6)

#define DATA_FRAME_4BIT             ((uint32)(0))
#define DATA_FRAME_8BIT             ((uint32)(1))
#define DATA_FRAME_16BIT            ((uint32)(2))

//SPI ENR
#define SPI_DISABLE                 ((uint32)(0x00) << 0)
#define SPI_ENABLE                  ((uint32)(0x01) << 0)

///SPIM_SR  SPIS_SR
#define RECEIVE_FIFO_FULL           ((uint32)(1)<<4)
#define RECEIVE_FIFO_EMPTY          ((uint32)(1)<<3)
#define TRANSMIT_FIFO_EMPTY         ((uint32)(1)<<2)
#define TRANSMIT_FIFO_FULL          ((uint32)(1)<<1)
#define SPI_BUSY_FLAG               ((uint32)(1))

//SPIM_DMACR SPIS_DMACR
#define TRANSMIT_DMA_ENABLE         ((uint32)(1) << 1)
#define RECEIVE_DMA_ENABLE          ((uint32)(1) << 0)

//SPI_ISR
#define SPI_TX_FIFO_EMPTY                  ((uint32)(1) << 0)
#define SPI_RX_FIFO_FULL                   ((uint32)(1)<<4)



/*
********************************************************************************
*
*                         End of hw_spi.h
*
********************************************************************************
*/

#endif

