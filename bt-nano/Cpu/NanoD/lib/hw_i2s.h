/*
********************************************************************************
*                   Copyright (c) 2008,yangwenjie
*                         All rights reserved.
*
* File Name£º   HW_I2s.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_I2S_H_
#define _HW_I2S_H_

#undef  EXT
#ifdef _IN_I2S_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                         I2S memap register define
*
*-------------------------------------------------------------------------------
*/
typedef volatile struct tagI2S_REG{

    UINT32 I2S_TXCR;                            //0x00      Transmit Operation Control Regsiter
    UINT32 I2S_RXCR;                            //0x04      Receive Operation Control Regsiter
    UINT32 I2S_CKR;                             //0x08      Clock Generation Register
    UINT32 I2S_FIFOLR;                          //0x0C      FIFO Level Register
    UINT32 I2S_DMACR;                           //0x10      DMA Control Register
    UINT32 I2S_INTCR;                           //0x14      Interrupt Control Register
    UINT32 I2S_INTSR;                           //0x18      Interrupt Status Register
    UINT32 I2S_XFER;                            //0x1C      Transfer Start Register.
    UINT32 I2S_CLR;                             //0x20      Transmit Soft Reset Register.
    //UINT32 I2S_Reserved[(0x400-0x24)/4];
    UINT32 I2S_TXDR;
    UINT32 I2S_RXDR;

} I2SReg_t,*gI2S_t;

/*
*-------------------------------------------------------------------------------
*
*                         I2S memap register define
*
*-------------------------------------------------------------------------------
*/
//I2S_TXCR
//TXCR_RCNT
#define I2S_FIFO_DEPTH        (0x20)       //FIFO depth = 32 word

//TXCR_CSR
#define TXCR_Channel_0_Enable       ((UINT32)0x00000000 << 15)  //Channel 0
#define TXCR_Channel_01_Enable      ((UINT32)0x00000001 << 15)  //Channel 0 - 1
#define TXCR_Channel_012_Enable     ((UINT32)0x00000002 << 15)  //Channel 0 - 1 -2
#define TXCR_Channel_0123_Enable    ((UINT32)0x00000003 << 15)  //Channel 0 - 1 - 2 -3

//TXCR_HWT
#define TXCR_HWT_Bit16              ((UINT32)0x00000000 << 14)  //low 16bit data valid from AHB/APB bus
#define TXCR_HWT_Bit32              ((UINT32)0x00000001 << 14)  //Low 16 bit for left channel and high 16 bit for right channel

//TXCR_SJM Store justified mode
#define TXCR_SJM_right_justified    ((UINT32)0x00000000 << 12)
#define TXCR_SJM_left_justified     ((UINT32)0x00000001 << 12)

//TXCR_FBM First Bit Mode
#define TXCR_FBM_MSB                ((UINT32)0x00000000 << 11)
#define TXCR_FBM_LSB                ((UINT32)0x00000001 << 11)

//TXCR_IBM I2S bus mode
#define TXCR_I2S_normal             ((UINT32)0x00000000 << 9)
#define TXCR_I2S_left_justified     ((UINT32)0x00000001 << 9)
#define TXCR_I2S_right_justified    ((UINT32)0x00000002 << 9)
#define TXCR_I2S_reserved           ((UINT32)0x00000003 << 9)

//TXCR_PBM PCM bus mode
#define TXCR_PCM_no_delay           ((UINT32)0x00000000 << 7)
#define TXCR_PCM_1_delay            ((UINT32)0x00000001 << 7)
#define TXCR_PCM_2_delay            ((UINT32)0x00000002 << 7)
#define TXCR_PCM_3_delay            ((UINT32)0x00000003 << 7)

//TXCR_TFS Tranfer format select
#define TXCR_TFS_I2S                ((UINT32)0x00000000 << 5)
#define TXCR_TFS_PCM                ((UINT32)0x00000001 << 5)

//TXCR_VDW Valid Data width
//RXCR_VDW Valid Data width
#define Valid_Data_width16          ((UINT32)0x0000000F << 0)
#define Valid_Data_width17          ((UINT32)0x00000010 << 0)
#define Valid_Data_width18          ((UINT32)0x00000011 << 0)
#define Valid_Data_width19          ((UINT32)0x00000012 << 0)
#define Valid_Data_width20          ((UINT32)0x00000013 << 0)
#define Valid_Data_width21          ((UINT32)0x00000014 << 0)
#define Valid_Data_width22          ((UINT32)0x00000015 << 0)
#define Valid_Data_width23          ((UINT32)0x00000016 << 0)
#define Valid_Data_width24          ((UINT32)0x00000017 << 0)
#define Valid_Data_width25          ((UINT32)0x00000018 << 0)
#define Valid_Data_width26          ((UINT32)0x00000019 << 0)
#define Valid_Data_width27          ((UINT32)0x0000001A << 0)
#define Valid_Data_width28          ((UINT32)0x0000001B << 0)
#define Valid_Data_width29          ((UINT32)0x0000001C << 0)
#define Valid_Data_width30          ((UINT32)0x0000001D << 0)
#define Valid_Data_width31          ((UINT32)0x0000001E << 0)
#define Valid_Data_width32          ((UINT32)0x0000001F << 0)

//I2S_RXCR
//RXCR_HWT
#define RXCR_HWT_Bit16              ((UINT32)0x00000000 << 14)  //low 16bit data valid from AHB/APB bus.
#define RXCR_HWT_Bit32              ((UINT32)0x00000001 << 14)  //Low 16 bit for left channel and high 16 bit for right channel

//RXCR_SJM Store justified mode
#define RXCR_SJM_right_justified    ((UINT32)0x00000000 << 12)
#define RXCR_SJM_left_justified     ((UINT32)0x00000001 << 12)

//RXCR_FBM First Bit Mode
#define RXCR_FBM_MSB                ((UINT32)0x00000000 << 11)
#define RXCR_FBM_LSB                ((UINT32)0x00000001 << 11)

//RXCR_IBM I2S bus mode
#define RXCR_I2S_normal             ((UINT32)0x00000000 << 9)
#define RXCR_I2S_left_justified     ((UINT32)0x00000001 << 9)
#define RXCR_I2S_right_justified    ((UINT32)0x00000002 << 9)
#define RXCR_I2S_reserved           ((UINT32)0x00000003 << 9)

//RXCR_PBM PCM bus mode
#define RXCR_PCM_no_delay           ((UINT32)0x00000000 << 7)
#define RXCR_PCM_1_delay            ((UINT32)0x00000001 << 7)
#define RXCR_PCM_2_delay            ((UINT32)0x00000002 << 7)
#define RXCR_PCM_3_delay            ((UINT32)0x00000003 << 7)

//RXCR_TFS Tranfer format select
#define RXCR_TFS_I2S                ((UINT32)0x00000000 << 5)
#define RXCR_TFS_PCM                ((UINT32)0x00000001 << 5)

//TXCR_VDW Valid Data width

//I2S_CKR
//CKR_MSS  Master/slave mode select
#define CKR_MSS_master              ((UINT32)0x00000000 << 27)  //master mode(sclk output)
#define CKR_MSS_slave               ((UINT32)0x00000001 << 27)  //slave mode(sclk input)

//CKR_CKP  Sclk polarity
#define CKR_CKP_posedge             ((UINT32)0x00000000 << 26)  //sample data at posedge sclk and drive data at negedge sclk
#define CKR_CKP_negedge             ((UINT32)0x00000001 << 26)  //sample data at negedge sclk and drive data at posedge sclk

//CKR_RLP Receive lrck polarity
#define CKR_RLP_normal              ((UINT32)0x00000000 << 25)  /*   (I2S normal: low for left channel, high for right channel
                                                                I2S left/right just: high for left channel, low for right channel
                                                                PCM start signal:high valid)
                                                                */
#define CKR_RLP_oppsite             ((UINT32)0x00000001 << 25)      /* (I2S normal: high for left channel, low for right channel
                                                                I2S left/right just: low for left channel, high for right channel
                                                                PCM start signal:low valid)
                                                                */
//CKR_TLP Transmit lrck polarity.
#define CKR_TLP_normal              ((UINT32)0x00000000 << 24)  /*(  I2S normal: low for left channel, high for right channel
                                                                I2S left/right just: high for left channel, low for right channel
                                                                PCM start signal:high valid)
                                                                */
#define CKR_TLP_oppsite             ((UINT32)0x00000001 << 24)  /*(  I2S normal: high for left channel, low for right channel
                                                                I2S left/right just: low for left channel, high for right channel
                                                                PCM start signal:low valid)
                                                                */
//CKR_MDIV mclk divider.
#define CKR_MDIV_Value_0            ((UINT32)0x00000000 << 16)
#define CKR_MDIV_Value_2            ((UINT32)0x00000001 << 16)
#define CKR_MDIV_Value_4            ((UINT32)0x00000003 << 16)
#define CKR_MDIV_Value_6            ((UINT32)0x00000005 << 16)
#define CKR_MDIV_Value_8            ((UINT32)0x00000007 << 16)

//RSD Receive sclk divider
#define CKR_RSD_Value               ((UINT32)0x0000007c << 8)

//TSD Transmit sclk divider
#define CKR_TSD_Value               ((UINT32)0x000000BA << 0)
//I2S_FIFOLR  FIFO Level Register

//I2S_DMACR DMA Control Register
#define DMA_Receive_disabled        ((UINT32)0x00000000 << 24)
#define DMA_Receive_enabled         ((UINT32)0x00000001 << 24)

#define DMA_Transmit_disabled       ((UINT32)0x00000000 << 8)
#define DMA_Transmit_enabled        ((UINT32)0x00000001 << 8)

//I2S_INTCR Interrupt Control Register
#define INTCR_RX_overrun_interrupt_disabled     ((UINT32)0x00000000 << 17)
#define INTCR_RX_overrun_interrupt_enable       ((UINT32)0x00000001 << 17)
#define INTCR_RX_overrun_interrupt_clear        ((UINT32)0x00000001 << 18)
#define INTCR_RX_full_interrupt_disabled        ((UINT32)0x00000000 << 16)
#define INTCR_RX_full_interrupt_enable          ((UINT32)0x00000001 << 16)

#define INTCR_TX_underrun_interrupt_disabled    ((UINT32)0x00000000 << 1)
#define INTCR_TX_underrun_interrupt_enable      ((UINT32)0x00000001 << 1)
#define INTCR_TX_underrun_interrupt_clear       ((UINT32)0x00000001 << 2)

#define INTCR_TX_empty_interrupt_disabled       ((UINT32)0x00000000 << 0)
#define INTCR_TX_empty_interrupt_enable         ((UINT32)0x00000001 << 0)


//I2S_INTSR Interrupt status Register
#define INTSR_RX_overrun_interrupt_inactive     ((UINT32)0x00000000 << 17)
#define INTSR_RX_overrun_interrupt_active       ((UINT32)0x00000001 << 17)
#define INTSR_RX_full_interrupt_inactive        ((UINT32)0x00000000 << 16)
#define INTSR_RX_full_interrupt_active          ((UINT32)0x00000001 << 16)


#define INTSR_TX_underrun_interrupt_inactive    ((UINT32)0x00000000 << 1)
#define INTSR_TX_underrun_interrupt_active      ((UINT32)0x00000001 << 1)
#define INTSR_TX_empty_interrupt_inactive       ((UINT32)0x00000000 << 0)
#define INTSR_TX_empty_interrupt_active         ((UINT32)0x00000001 << 0)
//I2S_TXDR Transimt FIFO Data Register.When it is written to, data are moved into the transmit FIFO.

//I2S_RXDR Receive FIFO Data Register.When the register is read, data in the receive FIFO is accessed.

//I2S_XFER Transfer Start Register
#define RX_transfer_stop            ((UINT32)0x00000000 << 1)   //there are no RXLRCK and RXSCLK
#define RX_transfer_start           ((UINT32)0x00000001 << 1)

#define TX_transfer_stop            ((UINT32)0x00000000 << 0)   //there are no TXLRCK and TXSCLK
#define TX_transfer_start           ((UINT32)0x00000001 << 0)

#define RTX_start           ((UINT32)0x00000001 << 2)
#define RTX_stop            ((UINT32)0x00000000 << 2)
//I2S_CLR Soft Reset Register
#define CLR_RXC_cleard              ((UINT32)0x00000001 << 1)   //This bit can be written only when XFER[1] bit is 0.
#define CLR_TXC_cleard              ((UINT32)0x00000001 << 0)   //This bit can be written only when XFER[0] bit is 0.



/*
********************************************************************************
*
*                         End of hw_i2s.h
*
********************************************************************************
*/
#endif
