/*
********************************************************************************
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* File Name：   i2s.c
*
* Description:  C program template
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14         1.0
*    desc:    ORG.
********************************************************************************
*/
#include "BspConfig.h"
#ifdef __DRIVER_I2S_I2SDEVICE_C__
#define _IN_I2S_
#include "typedef.h"
#include <DriverInclude.h>
#define I2SRSD(n)     ((n-1) << 8)
#define I2STSD(n)     ((n-1) << 0)

uint32 I2SGetIntType(eI2s_DEV i2sChNumb)
{
    uint32 dwI2sIntStaus;
    uint event;
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_DEV0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    dwI2sIntStaus = I2sGetInt(i2sChNumb);
    event = 0;

    if((dwI2sIntStaus & INTSR_TX_empty_interrupt_active) != 0)
    {
        event |= TX_interrupt_active;
    }

    if((dwI2sIntStaus & INTSR_TX_underrun_interrupt_active) != 0)
    {
        event |= TX_interrupt_fifo_empty;
        i2sReg->I2S_INTCR |= 0x04;
    }

    if((dwI2sIntStaus & INTSR_RX_full_interrupt_active) != 0)
    {
        event |= RX_interrupt_active;
    }

    if((dwI2sIntStaus & INTSR_RX_overrun_interrupt_active) != 0)
    {
        event |= RX_interrupt_fifo_full;
        i2sReg->I2S_INTCR |= 0x040000;
    }

    return event;
}

uint32 I2sGetRxFIFOaddr(eI2s_DEV i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_DEV0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(i2sReg->I2S_RXDR));
}

uint32 I2sGetTxFIFOaddr(eI2s_DEV i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_DEV0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(i2sReg->I2S_TXDR));
}

gI2S_t I2sGetCH(eI2s_DEV i2sChNumb)
{
    gI2S_t i2sReg;

    switch (i2sChNumb)
    {
        case I2S_DEV0:
            i2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            i2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    return i2sReg;
}
uint32 I2sGetInt(eI2s_DEV I2S_DEV)
{
    uint32 flag;
    gI2S_t I2s_Reg;


    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }
    flag = I2s_Reg->I2S_INTSR;
    return flag;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SDMADisable(eI2s_DEV I2S_DEV,I2S_Start_t DMA_TxRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(DMA_TxRx == I2S_START_DMA_TX)
    {
        //disable DMA
        I2s_Reg->I2S_DMACR &= (~DMA_Transmit_enabled);
    }
    else if (DMA_TxRx == I2S_START_DMA_RX)
    {
        //disable DMA
        I2s_Reg->I2S_DMACR &= (~DMA_Receive_enabled);                         ;
    }
    else if(DMA_TxRx == I2S_START_DMA_RTX)
    {
        //disable DMA
        I2s_Reg->I2S_DMACR &= ~(DMA_Transmit_enabled | DMA_Receive_enabled);
    }
    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SDMAEnable(eI2s_DEV I2S_DEV,I2S_Start_t DMA_TxRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(DMA_TxRx == I2S_START_DMA_TX)
    {
        //enable DMA
        I2s_Reg->I2S_DMACR |= (0x10 << 0)               //transmit data level
                         |(DMA_Transmit_enabled)    //transmit data enable
                         ;
    }
    else if (DMA_TxRx == I2S_START_DMA_RX)
    {
        //enable DMA
        I2s_Reg->I2S_DMACR |= (0x10<<16)                //receive data level
                         |(DMA_Receive_enabled)     //receive data enable
                         ;
    }
    else if(DMA_TxRx == I2S_START_DMA_RTX)
    {
        //enable DMA
        I2s_Reg->I2S_DMACR |= (0x10<<16)                //receive data level
                         |(DMA_Receive_enabled)     //receive data enable
                         |(0x10 << 0)               //transmit data level
                         |(DMA_Transmit_enabled)    //transmit data enable
                         ;
    }
    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SIntDisable(eI2s_DEV I2S_DEV,I2S_INT_t INT_TxRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(INT_TxRx & I2S_INT_TX)
    {
        //enable INT
        I2s_Reg->I2S_INTCR &= ~INTCR_TX_empty_interrupt_enable;
    }

    if(INT_TxRx & I2S_INT_FIFO_EMPTY)
    {
        I2s_Reg->I2S_INTCR &= ~INTCR_TX_underrun_interrupt_enable;
    }

    if (INT_TxRx & I2S_INT_RX)
    {
        //enable INT
        I2s_Reg->I2S_INTCR &= ~INTCR_RX_full_interrupt_enable;
    }

    if(INT_TxRx & I2S_INT_FIFO_FULL)
    {
        I2s_Reg->I2S_INTCR &= ~INTCR_RX_overrun_interrupt_enable;
    }

    return 0;
}
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SIntEnable(eI2s_DEV I2S_DEV,I2S_INT_t INT_TxRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(INT_TxRx & I2S_INT_TX)
    {
        //enable INT
        I2s_Reg->I2S_INTCR |= INTCR_TX_empty_interrupt_enable;
    }

    if(INT_TxRx & I2S_INT_FIFO_EMPTY)
    {
        I2s_Reg->I2S_INTCR |= INTCR_TX_underrun_interrupt_enable;
    }

    if (INT_TxRx & I2S_INT_RX)
    {
        //enable INT
        I2s_Reg->I2S_INTCR |= INTCR_RX_full_interrupt_enable;
    }

    if(INT_TxRx & I2S_INT_FIFO_FULL)
    {
        I2s_Reg->I2S_INTCR |= INTCR_RX_overrun_interrupt_enable;
    }

    return 0;
}
/*----------------------------------------------------------------------
Name      : I2SInit(I2S_mode_t mode, I2S_Start_t TxOrRx)
Desc      : initializatin
Params    : mode：I2S_MASTER_MODE or I2S_SLAVE_MODE master or slave
Return    : 0 ok，-1 fail。
            fs(kHz)\Fmclk(MHz)        12        24.576    12.288    8.192    11.2896
                            8        1500    3072    1536    1024    --
                            11.025    1088*    --        --        --        1024
                            12        1000    2048    1024    --        --
                            16        750        1536    768        512        --
                            22.05    544*    --        --        --        512
                            24        500        1024    512        --        --
                            32        375        768        384        256        --
                            44.1    272*    --        --        --        256
                            48        250        512        256        --        --
                            96        125        256        128        --        --
                            192        62.5    128        64        --        --
                            表格 7  Fmclk与Flrck比值表
            Fs=Fmclk/(RMD*RSD);
            Fs=Fmclk/(TMD*TSD);
            Fmclk is the frequency of mclk
            TMD(RMD) is divider rato of mclk to sclk,TMD(RMD) = Fmclk/Fsclk,TMD(RMD) is even from 2 to 64.
            TSD(RSD) is divider rato of sclk to lrck,TSD(RSD) = Fsclk/Flrck,TSD(RSD) is any number from 32 to 287.
            for keep 32 bit transfer,so the max value of TSD/4 and RSD is needed to bigger than 64(2X32bit).
            the range of TMD(RMD) is get from this,so the smallest drivider is 48.

----------------------------------------------------------------------*/
uint32 I2sSetSampleRate(eI2s_DEV I2S_DEV,I2sFS_en_t I2S_FS)
{
    uint32 FS_Regcofig = 0,clock_sel;
    //这里配置I2S CLK来源 : select pll clock ; select frac_div output ; select 12M
    if(I2S_DEV == I2S_DEV0)
    {
      clock_sel = 12000;    //测试代码，默认为12M
    }
    else if(I2S_DEV == I2S_DEV1)
    {
      clock_sel = 12000;
    }
    else
    {

    }

    switch (I2S_FS)
    {
        case I2S_FS_8000Hz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_6 | I2SRSD(250) | I2STSD(250);
            }
            else
            {

            }
            break;
        case I2S_FS_11025Hz:
            if(12000 == clock_sel)
            {
               //FS_Regcofig = CKR_MDIV_Value_6 | I2SRSD(182) | I2STSD(182);
               FS_Regcofig = CKR_MDIV_Value_8 | I2SRSD(136) | I2STSD(136);
            }
            else
            {

            }
            break;
        case I2S_FS_12KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(250) | I2STSD(250);
            }
            else
            {

            }
            break;
        case I2S_FS_16KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_6 | I2SRSD(125) | I2STSD(125);
            }
            else
            {

            }
            break;
        case I2S_FS_22050Hz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(161) | I2STSD(161);
            }
            else
            {

            }
            break;
        case I2S_FS_24KHz:
            if(12000 == clock_sel)
            {
                FS_Regcofig = CKR_MDIV_Value_4 | I2SRSD(125) | I2STSD(125);
            }
            else
            {

            }
            break;
        case I2S_FS_32KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(188) | I2STSD(188);
            }
            else
            {

            }
            break;
        case I2S_FS_44100Hz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(136) | I2STSD(136);
            }
            else
            {

            }
            break;
        case I2S_FS_48KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(125) | I2STSD(125);
            }
            else
            {

            }
            break;
        case I2S_FS_96KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_2 | I2SRSD(63) | I2STSD(63);
            }
            else
            {

            }
            break;
        case I2S_FS_192KHz:
            if(12000 == clock_sel)
            {
               FS_Regcofig = CKR_MDIV_Value_0 | I2SRSD(63) | I2STSD(63);
            }
            else
            {

            }
            break;
        default:
            break;
    }
    return FS_Regcofig;
}

/*******************************************************************************
** Name: I2sSetRxDataW
** Input:HDC dev,  CodecFS_en_t CodecFS
** Return: rk_err_t
** Owner: cjh
** Date: 2016.4.14
** Time: 14:18:09
*******************************************************************************/
uint32 I2sSetRxDataW(eI2s_DEV I2S_DEV,
                        eI2sFormat_t BUS_FORMAT,
                        eI2sDATA_WIDTH_t I2S_Data_width,
                        I2S_BUS_MODE_t I2S_Bus_mode)
{
    gI2S_t I2sReg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(I2S_Data_width == I2S_DATA_WIDTH24)
    {
        I2sReg->I2S_RXCR = ((Valid_Data_width32)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit32)                     //halfword word transform
                             );
    }
    else
    {
        //16 bit config
        //DEBUG1("16 bit config");
        I2sReg->I2S_RXCR = ((Valid_Data_width16)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit16)                     //halfword word transform
                             );
    }

}


uint32 I2sSetDataW(eI2s_DEV I2S_DEV,
                        eI2sFormat_t BUS_FORMAT,
                        eI2sDATA_WIDTH_t I2S_Data_width,
                        I2S_BUS_MODE_t I2S_Bus_mode)
{
    gI2S_t I2sReg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if(I2S_Data_width == I2S_DATA_WIDTH24)
    {
        I2sReg->I2S_TXCR = ((Valid_Data_width32)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(TXCR_FBM_MSB)                       //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit32)                   //halfword word transform
                             |(TXCR_Channel_0_Enable)             //channel select register
                             |(0<<17)                            //RCNT
                             );
/*
        I2sReg->I2S_RXCR = ((Valid_Data_width32)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit32)                     //halfword word transform
                             );*/
    }
    else
    {
        //16 bit config
        //DEBUG1("16 bit config");
        I2sReg->I2S_TXCR = ((Valid_Data_width16)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(TXCR_FBM_MSB)                       //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit16)                   //halfword word transform
                             |(TXCR_Channel_0_Enable)             //channel select register
                             |(0<<17)                            //RCNT
                             );
/*
        I2sReg->I2S_RXCR = ((Valid_Data_width16)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit16)                     //halfword word transform
                             );*/
    }

}
/*
--------------------------------------------------------------------------------
  Function name : void DataPortMuxSet(eDataPortIOMux_t data)
  Author        : anzhiguo
  Description   :

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             anzhiguo     2009-1-14         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void I2sCs(eI2sCs_t data)
{
    //Grf->IOMUX_CON1 = (0x00040000 | (data << 2));
}

//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : void I2sStart()
  Author        : yangwenjie
  Description   : start i2s transfer

  Input         : direction，
                            TX:  send to codec
                            RX:  input to inside
  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
int32 I2SStart(eI2s_DEV I2S_DEV,I2S_Start_t txRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    // modify I2S_DMACR to enable DMA
    // modify I2S_XFER to enable i2s
    // enable/disable/clear interrupt by modify I2S_INTCR&I2S_INTSR
    // read I2S_FIFOLR to get FIFO status

    if((txRx == I2S_START_PIO_TX) || (txRx == I2S_START_DMA_TX))
    {
        I2s_Reg->I2S_XFER |= TX_transfer_start;
    }
    else if ((txRx == I2S_START_PIO_RX) || (txRx == I2S_START_DMA_RX))
    {
        I2s_Reg->I2S_XFER |= RX_transfer_start;
    }
    else
    {
        I2s_Reg->I2S_XFER |= (RTX_start | TX_transfer_start | RX_transfer_start);
    }
    return 0;
}

/*
--------------------------------------------------------------------------------
  Function name : I2sStop(void)
  Author        : yangwenjie
  Description   : stop transfer

  Input         :

  Return        :

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void I2SStop(eI2s_DEV I2S_DEV,I2S_Start_t txRx)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    if((txRx == I2S_START_PIO_TX) || (txRx == I2S_START_DMA_TX))
    {
        I2s_Reg->I2S_XFER  &= ~(TX_transfer_start);
    }
    else if((txRx == I2S_START_PIO_RX) || (txRx == I2S_START_DMA_RX))
    {
        I2s_Reg->I2S_XFER  &= ~(RX_transfer_start);
    }
    else
    {
        I2s_Reg->I2S_XFER  = 0x0;
    }
}
/*----------------------------------------------------------------------
Name      : I2SInit(I2S_mode_t mode, I2S_Start_t TxOrRx)
Desc      : initializatin
Params    : mode：I2S_MASTER_MODE or I2S_SLAVE_MODE master or slave
Return    : 0 ok，-1 fail。
            fs(kHz)\Fmclk(MHz)        12        24.576    12.288    8.192    11.2896
                            8        1500    3072    1536    1024    --
                            11.025    1088*    --        --        --        1024
                            12        1000    2048    1024    --        --
                            16        750        1536    768        512        --
                            22.05    544*    --        --        --        512
                            24        500        1024    512        --        --
                            32        375        768        384        256        --
                            44.1    272*    --        --        --        256
                            48        250        512        256        --        --
                            96        125        256        128        --        --
                            192        62.5    128        64        --        --
                            表格 7  Fmclk与Flrck比值表
            Fs=Fmclk/(RMD*RSD);
            Fs=Fmclk/(TMD*TSD);
            Fmclk is the frequency of mclk
            TMD(RMD) is divider rato of mclk to sclk,TMD(RMD) = Fmclk/Fsclk,TMD(RMD) is even from 2 to 64.
            TSD(RSD) is divider rato of sclk to lrck,TSD(RSD) = Fsclk/Flrck,TSD(RSD) is any number from 32 to 287.
            for keep 32 bit transfer,so the max value of TSD/4 and RSD is needed to bigger than 64(2X32bit).
            the range of TMD(RMD) is get from this,so the smallest drivider is 48.

----------------------------------------------------------------------*/
int32 I2SInit(eI2s_DEV I2S_DEV,
              I2S_mode_t I2S_mode,
              eI2sCs_t I2S_CS,
              I2sFS_en_t I2S_FS,
              eI2sFormat_t BUS_FORMAT,
              eI2sDATA_WIDTH_t I2S_Data_width,
              I2S_BUS_MODE_t I2S_Bus_mode,
              eI2sFormat_t RX_BUS_FORMAT,
              eI2sDATA_WIDTH_t Rx_I2S_Data_width,
              I2S_BUS_MODE_t Rx_I2S_Bus_mode)
{
    uint32 FS_Regcofig = 0;
    uint32 timeout = 20000;
    gI2S_t I2sReg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2sReg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2sReg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    I2sReg->I2S_XFER = (TX_transfer_stop |RX_transfer_stop);

    I2sReg->I2S_CLR = CLR_TXC_cleard | CLR_RXC_cleard;
    while(I2sReg->I2S_CLR)
    {
        if (--timeout == 0)
        {
            break;
        }
    }
    //i2s cs set
    //if((I2S_CS == I2S_EXT) || (I2S_CS == I2S_IN))
        //I2sCs(I2S_CS);

    //24 bit config
    if(I2S_Data_width == I2S_DATA_WIDTH24)
    {
        I2sReg->I2S_TXCR = ((Valid_Data_width32)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(TXCR_FBM_MSB)                       //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit32)                   //halfword word transform
                             |(TXCR_Channel_0_Enable)             //channel select register
                             |(0<<17)                            //RCNT
                             );
    }
    else
    {
        //16 bit config
        //DEBUG1("16 bit config");
        I2sReg->I2S_TXCR = ((Valid_Data_width16)                //Valid data width
                             |(BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(TXCR_PCM_no_delay)                //PCM bus mode
                             |(I2S_Bus_mode << 9)                    //I2S bus mode
                             |(TXCR_FBM_MSB)                       //First Bit Mode
                             |(TXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(TXCR_HWT_Bit16)                   //halfword word transform
                             |(TXCR_Channel_0_Enable)             //channel select register
                             |(0<<17)                            //RCNT
                             );
    }

    if(Rx_I2S_Data_width == I2S_DATA_WIDTH24)
    {
        I2sReg->I2S_RXCR = ((Valid_Data_width32)                //Valid data width
                             |(RX_BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(Rx_I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit32)                     //halfword word transform
                             );
    }
    else
    {
        //16 bit config
        I2sReg->I2S_RXCR = ((Valid_Data_width16)                //Valid data width
                             |(RX_BUS_FORMAT << 5)                    //Tranfer format select, 0:I2S, 1:PCM
                             |(RXCR_PCM_no_delay)                //PCM bus mode
                             |(Rx_I2S_Bus_mode << 9)                    //I2S bus mode
                             |(RXCR_FBM_MSB)                       //First Bit Mode
                             |(RXCR_SJM_right_justified)        //store justified mode,0:right,1:left
                             |(RXCR_HWT_Bit16)                     //halfword word transform
                             );
    }
   // DEBUG1("I2s_Reg->I2S_TXCR = 0x%x",I2s_Reg->I2S_TXCR);
    //DEBUG1("I2s_Reg->I2S_RXCR = 0x%x",I2s_Reg->I2S_RXCR);

    if(I2S_mode == I2S_SLAVE_MODE)
    {
        I2sReg->I2S_CKR  = ((CKR_TSD_Value)                   //Fsclk/Ftxlrck TSD=187 no care
                             |(CKR_RSD_Value)                   //Fsclk/Frxlrck RSD=124 no care
                             |(CKR_MDIV_Value_0)                //MDIV=6M  Fmclk/Ftxsck-1,panda_FPGA:MCLK=12M
                             |(CKR_TLP_normal)                  //tx lrck polarity
                             |(CKR_RLP_normal)                  //rx lrck polarity
                             |(CKR_CKP_posedge)                 //rx/tx sclk polarity
                             |(CKR_MSS_slave)                   //slave mode
                             );
        //DEBUG1("I2s_Reg->I2S_CKR = 0x%x",I2s_Reg->I2S_CKR);

    }
    else
    {
        //get FS Reg config
        FS_Regcofig = I2sSetSampleRate(I2S_DEV, I2S_FS);
        I2sReg->I2S_CKR  = ( FS_Regcofig
                             |(CKR_TLP_normal)                  //rx/tx lrck polarity
                             |(CKR_RLP_normal)
                             |(CKR_CKP_posedge)                 //rx/tx sclk polarity
                             |(CKR_MSS_master)                  //master mode
                             );
    }

   I2sReg->I2S_INTCR = ((INTCR_TX_empty_interrupt_disabled)        // tx empty interrupt. 0:disable,1:enable
                         |(INTCR_TX_underrun_interrupt_disabled)    // tx under run int enable.0:disable,1:enable
                         |(0<<2)                                    // write 1 to clear TX underrun int
                         |(0x10<<4)                                    // transmit FIFO threshold
                         |(INTCR_RX_full_interrupt_disabled)        // RX Full int
                         |(INTCR_RX_overrun_interrupt_disabled)     // RX overrun
                         |(0<<18)                                   // write 1 to clear RX overrun
                         |(0x10<<20)                                // Reveive FIFO threshold
                         );
   //DEBUG1("I2s_Reg->I2S_INTCR = 0x%x",I2s_Reg->I2S_INTCR);

    return 0;
}

rk_size_t I2S_PIO_Read(eI2s_DEV I2S_DEV,uint32* buffer,uint32 size)
{
    gI2S_t I2s_Reg;
    uint32 dwRealSize, cnt;

    uint32 RX_FIFOLR;
    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    cnt = size > 16? 16 : size;
    dwRealSize = 0;

    while (cnt--)
    {
         *buffer = *(uint32 *)(&(I2s_Reg->I2S_RXDR));
         buffer++;
         dwRealSize++;
    }

    return dwRealSize;

}

rk_size_t I2S_PIO_Write(eI2s_DEV I2S_DEV,uint32* buffer,uint32 size)
{
    gI2S_t I2s_Reg;
    uint32 dwRealSize, cnt;

    uint32 TX_FIFOLR;
    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }
    cnt = size > 16? 16 : size;
    //cnt = size;
    //printf("i2s write cnt=%d\n",cnt);
    dwRealSize = 0;
    while (cnt--)
    {
        /*
        if(I2s_Reg->I2S_FIFOLR & 0x3f >= 0x1e)
        {
            break;
        }
        */
        //printf("i2swrite[%d]=0x%x\n",cnt,*buffer);
        *(uint32 *)(&(I2s_Reg->I2S_TXDR))  = *buffer;
        buffer++;
        dwRealSize++;
    }
    return dwRealSize;

}
/*
--------------------------------------------------------------------------------
  Function name : I2S_PowerOnInit(I2S_mode_t mode)
  Author        : yangwenjie
  Description   : I2S auti-initial

  Input         : NULL

  Return        : NULL

  History:     <author>         <time>         <version>
             yangwenjie     2008-11-20         Ver1.0
  desc:         ORG
  note:        if do not use i2s device,for example,using internal codec,
                 must call this function after power on.
--------------------------------------------------------------------------------
*/
void I2SDeInit(eI2s_DEV I2S_DEV)
{
    gI2S_t I2s_Reg;

    switch (I2S_DEV)
    {
        case I2S_DEV0:
            I2s_Reg = (gI2S_t)I2S0_BASE;
            break;
        case I2S_DEV1:
            I2s_Reg = (gI2S_t)I2S1_BASE;
            break;
        default:
            break;
    }

    I2s_Reg->I2S_XFER = 0;

//    #if(BOARDTYPE == CK610_FPGA)

 //   #else
//        SCUDisableClk(CLK_GATE_I2S0);
 //   #endif
}
//#endif

#endif
/*
********************************************************************************
*
*                         End of i2s.c
*
********************************************************************************
*/
