/*
********************************************************************************
*                   Copyright (c) 2008,Yangwenjie
*                         All rights reserved.
*
* File Name£º   spi.h
*
* Description:  define the master/slave register structure bit marco and related definition\interfaces.
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-15          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SPI_H_
#define _SPI_H_

#undef  EXT
#ifdef _IN_SPI_
#define EXT
#else
#define EXT extern
#endif
/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
#define SPI_CTL_TX_SPINOR   ( SPI_MASTER_MODE | TRANSMIT_ONLY | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_HIGH \
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_CTL_RX_SPINOR   ( SPI_MASTER_MODE | RECEIVE_ONLY | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_HIGH \
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)

#define SPI_CTL_TXRX_SPINOR ( SPI_MASTER_MODE | TRANSMIT_RECEIVE | MOTOROLA_SPI \
                            | RXD_SAMPLE_NO_DELAY | APB_BYTE_WR \
                            | MSB_FBIT | LITTLE_ENDIAN_MODE | CS_2_SCLK_OUT_1_CK \
                            | CS_KEEP_LOW | SERIAL_CLOCK_POLARITY_HIGH \
                            | SERIAL_CLOCK_PHASE_START | DATA_FRAME_8BIT)


#define SPI_MAX_CH 2
/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/
typedef enum
{
    SPI_TRANSMIT = 0,
    SPI_RECEIVE
}SPI_DMA_MODE;

typedef enum SPIM_cs
{
    SPI_CS0 = 1,
    SPI_CS1,

}eSPI_cs_t;

typedef enum SPIM_TRANSFER_MODE
{
    SPI_TRANSMIT_RECEIVE = 0,
    SPI_TRANSMIT_ONLY,
    SPI_RECEIVE_ONLY,
//    SPI_EEPROM_READ
}eSPI_TRANSFER_MODE_t;

typedef enum SPIM_PHASE
{
    SPI_MIDDLE_FIRST_DATA = 0,
    SPI_START_FIRST_DATA
}eSPI_PHASE_t;

typedef enum SPIM_POLARITY
{
    SPI_SERIAL_CLOCK_LOW = 0,
    SPI_SERIAL_CLOCK_HIGH
}eSPI_POLARITY_t;

typedef enum SPIM_DMA
{
    SPI_DMA_DISABLE = 0,
    SPI_DMA_ENABLE
}eSPI_DMA_t;

#if 0
typedef enum
{
    DATA_4BIT = 0,
    DATA_5BIT,
    DATA_6BIT,
    DATA_7BIT,
    DATA_8BIT,
    DATA_9BIT,
    DATA_10BIT,
    DATA_11BIT,
    DATA_12BIT,
    DATA_13BIT,
    DATA_14BIT,
    DATA_15BIT,
    DATA_16BIT
}eSPI_DATA_WIDTH;
#endif

typedef enum
{
    DATA_WIDTH4 = 0,
    DATA_WIDTH8,
    DATA_WIDTH16
}SPI_DATA_WIDTH;

typedef enum SPI_ch
{
    SPI_CH0,
    SPI_CH1,
}eSPI_ch_t;


typedef struct tagSPI_INFO
{
    uint16 spiNum;
    uint16 baudRate;        //kHz
    SPI_DATA_WIDTH dataWidth;
    eSPI_cs_t slaveNumb;
    eSPI_TRANSFER_MODE_t transferMode;
    eSPI_PHASE_t serialClockPhase;
    eSPI_POLARITY_t  polarity;

}SPI_INFO,*pSPI_INFO;
//SPI Registers
typedef volatile struct tagSPI_STRUCT
{
    uint32 SPI_CTRLR0;
    uint32 SPI_CTRLR1;
    uint32 SPI_ENR;
    uint32 SPI_SER;
    uint32 SPI_BAUDR;
    uint32 SPI_TXFTLR;
    uint32 SPI_RXFTLR;
    uint32 SPI_TXFLR;
    uint32 SPI_RXFLR;
    uint32 SPI_SR;
    uint32 SPI_IPR;
    uint32 SPI_IMR;
    uint32 SPI_ISR;
    uint32 SPI_RISR;
    uint32 SPI_ICR;
    uint32 SPI_DMACR;
    uint32 SPI_DMATDLR;
    uint32 SPI_DMARDLR;
    uint32 reserved[(0x400 - 0x0048)/4];
    uint32 SPI_TXDR[(0x800 - 0x400)/4];
    uint32 SPI_RXDR[(0xC00 - 0x800)/4];
}SPI_REG,*pSPI_REG;

#define SPICtl            		        ((SPI_REG *) SPI0_BASE)
/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
extern void  SPIInit(eSPI_ch_t spiChNumb,eSPI_cs_t slaveNumb, uint32 baudRate, uint32 CtrMode);
extern void SPIDeInit(eSPI_ch_t spiChNumb);
extern pSPI_REG SpiGetCH(eSPI_ch_t spiChNumb);
extern uint32 SpiGetRxFIFOaddr(eSPI_ch_t spiChNumb);
extern uint32 SpiGetTxFIFOaddr(eSPI_ch_t spiChNumb);
extern rk_err_t SPIPioWrite(eSPI_ch_t spiChNumb);
extern rk_size_t SPIWriteFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size);

extern rk_err_t SPIPioRead(eSPI_ch_t spiChNumb,uint32 size);
extern rk_size_t SPIReadFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size);

extern rk_err_t SPIDmaWrite(eSPI_ch_t spiChNumb);
extern rk_err_t  SPIDmaRead(eSPI_ch_t spiChNumb,uint32 size);
extern uint32 SpiGetInt(eSPI_ch_t spiChNumb);
extern void SPIClearAllInt(eSPI_ch_t spiChNumb);
extern uint32 SpiEanbleChannel(eSPI_ch_t spiChNumb,uint32 ch);
extern uint32 SpiDisableChannel(eSPI_ch_t spiChNumb,uint32 ch);
extern void SpiEnableTxInt(eSPI_ch_t spiChNumb);
extern void  SpiDisalbeTxInt(eSPI_ch_t spiChNumb);
extern void SpiEnableRxInt(eSPI_ch_t spiChNumb);
extern void  SpiDisalbeRxInt(eSPI_ch_t spiChNumb);

extern void SpiWaitIdle(eSPI_ch_t spiChNumb);



/*
********************************************************************************
*
*                         End of spi.h
*
********************************************************************************
*/
#endif
