/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\Spi.c
* Owner: wangping
* Date: 2016.4.27
* Time: 10:09:39
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2016.4.27     10:09:39   1.0
********************************************************************************************
*/
#define _IN_SPI_


#include "typedef.h"
#include "DriverInclude.h"
uint32 g_spimFreq;

pSPI_REG SpiGetCH(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            spiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            spiReg = (pSPI_REG)SPI1_BASE;
            break;
        default:
            break;
    }

    return spiReg;
}

void SPIClearAllInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_ICR = 0;
}

void SpiWaitIdle(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    while(spiReg->SPI_SR & 0X01);
}

uint32 SpiGetInt(eSPI_ch_t spiChNumb)
{
    uint32 flag;
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    flag = spiReg->SPI_ISR;
    spiReg->SPI_ICR = 0;
    return flag;
}
uint32 SpiGetRxFIFOaddr(eSPI_ch_t spiChNumb)
{
    pSPI_REG SpiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            SpiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            SpiReg = (pSPI_REG)SPI1_BASE;
        default:
            break;
    }

    return (uint32)(&(SpiReg->SPI_RXDR));
}

uint32 SpiGetTxFIFOaddr(eSPI_ch_t spiChNumb)
{
    pSPI_REG SpiReg;

    switch (spiChNumb)
    {
        case SPI_CH0:
            SpiReg = (pSPI_REG)SPI0_BASE;
            break;
        case SPI_CH1:
            SpiReg = (pSPI_REG)SPI1_BASE;
            break;
        default:
            break;
    }

    return (uint32)(&(SpiReg->SPI_TXDR));
}
void SpiEnableTxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR |= SPI_TX_FIFO_EMPTY;
}

void SpiDisalbeTxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR &= ~SPI_TX_FIFO_EMPTY;
}

void SpiEnableRxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR |= SPI_RX_FIFO_FULL;// | (1 << 3) | (1 << 2);
}

void SpiDisalbeRxInt(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_IMR &= ~SPI_RX_FIFO_FULL;
}



uint32 SpiEnableChannel(eSPI_ch_t spiChNumb,uint32 ch)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    spiReg->SPI_SER  |= ((uint32)(0x01) << ch);
}


uint32 SpiDisableChannel(eSPI_ch_t spiChNumb,uint32 ch)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return 0;
    }
    spiReg->SPI_SER  &= ~((uint32)(0x01) << ch);
}

/*----------------------------------------------------------------------
Name      : SPIMInit(uint16 baudRate,  eSPIM_cs_t slaveNumb, eSPIM_TRANSFER_MODE_t transferMode, eSPIM_PHASE_t serialClockPhase, eSPIM_POLARITY_t  polarity)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
void SPIInit(eSPI_ch_t spiChNumb,eSPI_cs_t slaveNumb, uint32 baudRate, uint32 CtrMode)
{
    int8 ret=0;
    uint32 APBnKHz;
    pSPI_REG spiReg;
    uint32 clk;
    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }

    g_spimFreq = baudRate;
    //APBnKHz = GetPerAPBCLK();

    if (spiReg->SPI_SER)
    {
        return;
    }

    //Grf->IOMUX_CON1 = 0x00020000 | (ch << 1);

    clk = GetSPIFreq(spiChNumb);
    printf("spi clk = %d\n",clk);
    //clk  = 24;  //FOR TEST
    if ((baudRate > 12*1000*1000) && (clk > 24000000))
    {
        //Need Configer spi clk to other
        //spiReg->CLKSEL_SPI = ((SPI_CLK_SEL_MASK << 16) | MMC_CLK_SEL_PLL);

        //spiReg->SPI_BAUDR = clk * 1000 * 1000 / baudRate;
        spiReg->SPI_BAUDR = clk / baudRate;
    }
    else
    {
        //Need Configer spi clk to 24M
        //spiReg->CLKSEL_SPI = ((SPI_CLK_SEL_MASK << 16) | SPI_CLK_SEL_24M);

        spiReg->SPI_BAUDR = 24 * 1000 * 1000 / baudRate;
    }

    spiReg->SPI_CTRLR0 = CtrMode;
    //spiReg->SPI_SER    = ((uint32)(0x01) << slaveNumb);

}
/*
--------------------------------------------------------------------------------
  Function name : int32 SPIWrite(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
void SPIDeInit(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return;
    }
    spiReg->SPI_ENR = 0;
    spiReg->SPI_SER = 0;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 SPIWrite(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   ¨°?Hz?a¦Ì£¤??
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
rk_err_t SPIPioWrite(eSPI_ch_t spiChNumb)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return RK_ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= TRANSMIT_RECEIVE;
    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | TRANSMIT_ONLY;
    #ifdef _USE_RKI6000_
    if(spiChNumb == SPI_CH1)
    {
        spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(1ul<<6) | SERIAL_CLOCK_PHASE_START;
    }
    #endif
    spiReg->SPI_ENR = SPI_ENABLE;

    return RK_SUCCESS;
}

rk_size_t SPIWriteFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size)
{
     int32  totalsize;
     pSPI_REG spiReg;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return RK_ERROR;
     }
     totalsize = size;

     while (size)
     {
         if ((spiReg->SPI_SR & TRANSMIT_FIFO_FULL) != TRANSMIT_FIFO_FULL)
         {
             spiReg->SPI_TXDR[0] = *pdata++;
             size--;
         }
         else
         {
             break;
         }
     }

    return (totalsize - size);
}


/*
--------------------------------------------------------------------------------
  Function name : int32 SPIRead(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:
--------------------------------------------------------------------------------
*/
rk_err_t SPIPioRead(eSPI_ch_t spiChNumb,uint32 size)
{
     pSPI_REG spiReg;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return RK_ERROR;
     }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= TRANSMIT_RECEIVE;
    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | RECEIVE_ONLY;
    #ifdef _USE_RKI6000_
    if(spiChNumb == SPI_CH1)
    {
        spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(1ul<<6);
    }
    #endif
    spiReg->SPI_CTRLR1 = size - 1;

 //   spiReg->SPI_TXFTLR = 1;
    spiReg->SPI_RXFTLR = 0;

 //   SpiEnableRxInt(spiChNumb);

    spiReg->SPI_ENR = SPI_ENABLE;

    return RK_SUCCESS;
}


rk_size_t SPIReadFIFO(eSPI_ch_t spiChNumb,uint8 *pdata, uint32 size)
{

    int32  totalsize;
    pSPI_REG spiReg;

    totalsize = size;

     if(spiChNumb == SPI_CH0)
     {
         spiReg = (pSPI_REG) SPI0_BASE;
     }
     else if(spiChNumb == SPI_CH1)
     {
         spiReg = (pSPI_REG) SPI1_BASE;
     }
     else
     {
        return RK_ERROR;
     }
    while (size)
    {
        //spiReg->SPI_TXDR[0] = 0xFF;     //send clock
        //DelayUs(1);
        if ((spiReg->SPI_SR & RECEIVE_FIFO_EMPTY) != RECEIVE_FIFO_EMPTY)
        {
            *pdata++ = (uint8)(spiReg->SPI_RXDR[0] & 0xFF);
            size--;
        }
        else
        {
            break;
        }
    }

    return (totalsize - size);


}



/*
--------------------------------------------------------------------------------
  Function name : int32 SPIDmaWrite(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size, pFunc CallBack)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:        After finish spi transfer,need call SPIDeInit function to release bus.
--------------------------------------------------------------------------------
*/
rk_err_t SPIDmaWrite(eSPI_ch_t spiChNumb)
{
     pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return RK_ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= SPI_CTL_TX_SPINOR;
    spiReg->SPI_DMACR = TRANSMIT_DMA_ENABLE | RECEIVE_DMA_ENABLE;
    spiReg->SPI_ENR = SPI_ENABLE;

    return RK_SUCCESS;
}

/*
--------------------------------------------------------------------------------
  Function name : int32 SPIDmaRead(uint32 ch, uint32 baudRate, uint32 CtrMode, uint8 *pdata, uint32 size, pFunc CallBack)
  Author        :
  Description   :

  Input         : baudRate:   unit:HZ
                  ch¡êo        spi cs
  Return        :

  History:     <author>         <time>         <version>
  desc:         ORG
  Note:        After finish spi transfer,need call SPIDeInit function to release bus.
--------------------------------------------------------------------------------
*/
rk_err_t  SPIDmaRead(eSPI_ch_t spiChNumb,uint32 size)
{
    pSPI_REG spiReg;

    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return RK_ERROR;
    }
    spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 &= ~(TRANSMIT_RECEIVE_MASK);
    //spiReg->SPI_CTRLR0 |= SPI_CTL_RX_SPINOR;

    spiReg->SPI_CTRLR1 = size - 1;
    spiReg->SPI_DMACR   = RECEIVE_DMA_ENABLE | TRANSMIT_DMA_ENABLE;
    spiReg->SPI_ENR = SPI_ENABLE;

    return RK_SUCCESS;
}

void SPISetRwMode(eSPI_ch_t spiChNumb, uint32 mode)
{
    pSPI_REG spiReg;
    if(spiChNumb == SPI_CH0)
    {
        spiReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       return ;
    }
    //spiReg->SPI_ENR = SPI_DISABLE;
    //spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(3ul<<18) | mode;
    spiReg->SPI_CTRLR0 = spiReg->SPI_CTRLR0 & ~(1ul<<7) | (mode << 7);
    //spiReg->SPI_ENR = SPI_ENABLE;
}


#if 0
/*----------------------------------------------------------------------
Name      : SPIMBaudRate(uint32 baudRate,uint32 APBnKHz)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPIMBaudRate(eSPI_ch_t spiChNumb,uint32 baudRate,uint32 APBnKHz)
{
    int32 ret=0;
    pSPI_REG spiMasterReg;

    if(spiChNumb == SPI_CH0)
    {
        spiMasterReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiMasterReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }

    spiMasterReg->SPI_BAUDR = APBnKHz/baudRate;
    if (spiMasterReg->SPI_BAUDR == 0)
    {
        ret = -1;
    }

    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPIMUpdateAllApbFreq(void)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
void SPIMUpdateAllApbFreq(uint32 APBnKHz)
{
    if(g_spimFreq > 0)
    {
        SPIMBaudRate(g_spimFreq,APBnKHz);
    }
}

/*----------------------------------------------------------------------
Name      : SPIMInit(uint16 baudRate,  eSPIM_cs_t slaveNumb, eSPIM_TRANSFER_MODE_t transferMode, eSPIM_PHASE_t serialClockPhase, eSPIM_POLARITY_t  polarity)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPIMInit(eSPI_ch_t spiChNumb,uint16 baudRate,  SPI_DATA_WIDTH dataWidth,eSPI_cs_t slaveNumb,
                    eSPI_TRANSFER_MODE_t transferMode, eSPI_PHASE_t serialClockPhase,
                    eSPI_POLARITY_t  polarity, uint8 spiImr)
{
    int8 ret=0;
    uint32 APBnKHz;
    pSPI_REG spiMasterReg;

    if(spiChNumb == SPI_CH0)
    {
        spiMasterReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
        spiMasterReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }

    if (slaveNumb > 2)
    {
        ret = -1;
    }

    if (transferMode > 2)
    {
        ret = -1;
    }

    if (serialClockPhase > 1)
    {
        ret = -1;
    }

    if (polarity > 1)
    {
        ret = -1;
    }

    if (dataWidth > 2)
    {
        ret = -1;
    }

    g_spimFreq = baudRate;
    spiMasterReg->SPI_ENR = SPI_DISABLE;

    //APBnKHz = GetPerAPBCLK();

    ret = SPIMBaudRate(baudRate,APBnKHz);

    spiMasterReg->SPI_CTRLR0 = 0;
    spiMasterReg->SPI_CTRLR1 = 0;
    spiMasterReg->SPI_SER = 0;

    //spiMasterReg->SPI_CTRLR0 = (spiMasterReg->SPI_CTRLR0 & (~0x3c))  |( (dataWidth-1)<<2)  | (transferMode<<18) | (polarity<<7) | (serialClockPhase<<6) |0x2;
    spiMasterReg->SPI_CTRLR0 = (spiMasterReg->SPI_CTRLR0 & (~0x3))
                                | dataWidth
                                | (transferMode<<18)
                                |  MOTOROLA_SPI
                                | RXD_SAMPLE_NO_DELAY
                                | APB_BYTE_WR
                                | MSB_FBIT
                                | LITTLE_ENDIAN_MODE
                                | CS_2_SCLK_OUT_1_CK
                                | CS_KEEP_LOW
                                | (polarity<<7)
                                | (serialClockPhase<<6);
    spiMasterReg->SPI_CTRLR1 = 0x1f;
    spiMasterReg->SPI_SER = slaveNumb;
    spiMasterReg->SPI_IMR = spiImr;
    spiMasterReg->SPI_ENR = 1;
    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPIMDeinit(void)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
void SPIMDeinit(eSPI_ch_t spiChNumb)
{
    uint32 i;
    pSPI_REG spiMasterReg;
    int8 ret=0;

    if(spiChNumb == SPI_CH0)
    {
        spiMasterReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiMasterReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    //spiMasterReg->SPI_CTRLR0 = 0;
    //spiMasterReg->SPI_CTRLR1 = 0;
    spiMasterReg->SPI_BAUDR = 0;
    spiMasterReg->SPI_SER = 0;
    spiMasterReg->SPI_ENR = 0;
}

/*----------------------------------------------------------------------
Name      : SPIMRead(void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPIMRead(eSPI_ch_t spiChNumb,void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
{
    uint8 *pdataWrite1;
    uint16 *pdataWrite2;
    int32 ret=0;
    pSPI_REG spiMasterReg;

    if(spiChNumb == SPI_CH0)
    {
        spiMasterReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiMasterReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }

    if (DATA_WIDTH16 == dataWidth)
    {
        pdataWrite2 = (uint16 *) pdata;
        while (length !=0)
        {
            if (spiMasterReg->SPI_RXFLR!= 0)
            {
                *pdataWrite2++ = spiMasterReg->SPI_RXDR[0];
                length--;
            }
        }
    }
    else
    {
        pdataWrite1 = (uint8 *) pdata;
        while (length !=0)
        {
            if ((spiMasterReg->SPI_SR & RECEIVE_SPI_TX_FIFO_EMPTY) == RECEIVE_SPI_TX_FIFO_EMPTY)
            {
                *pdataWrite1++ = spiMasterReg->SPI_RXDR[0];
                length--;
            }
        }
    }

    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPIMWrite(void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPIMWrite(eSPI_ch_t spiChNumb,void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
{
    uint8 *pdataWrite1;
    uint16 *pdataWrite2;
    int32 ret=0;
    pSPI_REG spiMasterReg;

    if(spiChNumb == SPI_CH0)
    {
        spiMasterReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiMasterReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    //spiMasterReg->SPI_ENR = 1;
    if (DATA_WIDTH16 == dataWidth)
    {
    	pdataWrite2 = (uint16 *) pdata;
	    while (length != 0 )
        {
            if ((spiMasterReg->SPI_SR & TRANSMIT_SPI_RX_FIFO_FULL) == TRANSMIT_SPI_RX_FIFO_FULL)
            {
                spiMasterReg->SPI_TXDR[0] = *pdataWrite2++;
		        length--;
            }
        }
    }
    else
    {
        pdataWrite1 = (uint8 *) pdata;
        while (length != 0 )
        {
            if ((spiMasterReg->SPI_SR & TRANSMIT_SPI_RX_FIFO_FULL) == TRANSMIT_SPI_RX_FIFO_FULL)
            {
                spiMasterReg->SPI_TXDR[0] = *pdataWrite1++;
                length--;
            }
        }
    }

    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPISInit()
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPISInit(eSPI_ch_t spiChNumb,SPI_DATA_WIDTH dataWidth,eSPI_TRANSFER_MODE_t transferMode, eSPI_PHASE_t serialClockPhase, eSPI_POLARITY_t  polarity,uint8 spiImr)

{
    int32 ret=0;
    pSPI_REG spiSlaveReg;

    if(spiChNumb == SPI_CH0)
    {
        spiSlaveReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiSlaveReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    if (transferMode > 3)
    {
        ret = -1;
    }

    if (serialClockPhase > 1)
    {
        ret = -1;
    }
    if (dataWidth > 2)
    {
        ret = -1;
    }
    //dataWidth = dataWidth + 4;
    //SCUEnableClk(CLK_GATE_SPI0);

    spiSlaveReg->SPI_ENR = 0;
    spiSlaveReg->SPI_CTRLR0 = 0;

    //spiSlaveReg->SPI_CTRLR0 = (spiSlaveReg->SPI_CTRLR0 & (~0x3c)) | ((dataWidth-1)<<2) | SPI_SLAVE_MODE | (transferMode<<18) | (polarity<<7) | (serialClockPhase<<6) |0x2;
    spiSlaveReg->SPI_CTRLR0 = (spiSlaveReg->SPI_CTRLR0 & (~0x3))  | dataWidth | (transferMode<<18) | (polarity<<7) | (serialClockPhase<<6)|SPI_SLAVE_MODE;
    spiSlaveReg->SPI_IMR = spiImr;
    spiSlaveReg->SPI_ENR = 1;

    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPISDeinit()
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
void SPISDeinit(eSPI_ch_t spiChNumb,)
{
    uint32 i;
    pSPI_REG spiSlaveReg;

    if(spiChNumb == SPI_CH0)
    {
        spiSlaveReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiSlaveReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    //spiSlaveReg->SPI_CTRLR0 = 0;
    spiSlaveReg->SPI_ENR = 0;

    SCUDisableClk(CLK_GATE_SPI0);
}

/*----------------------------------------------------------------------
Name      : SPISRead(void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPISRead(eSPI_ch_t spiChNumb,void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
{
    uint8 *pdataWrite1;
    uint16 *pdataWrite2;
    int32 ret=0;
    pSPI_REG spiSlaveReg;

    if(spiChNumb == SPI_CH0)
    {
        spiSlaveReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiSlaveReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    if ( 2 == dataWidth)
    {
        pdataWrite2 = (uint16 *) pdata;
        while (length !=0)
        {
            if (spiSlaveReg->SPI_RXFLR!= 0)
            {
            	  *pdataWrite2++ = spiSlaveReg->SPI_RXDR[0];
                  //PRINTF("length = %d!\n",length);
                  length--;
            }
        }
    }
    else
    {
        pdataWrite1 = (uint8 *) pdata;
        while (length !=0)
        {
            if ((spiSlaveReg->SPI_SR & RECEIVE_SPI_TX_FIFO_EMPTY) == RECEIVE_FIFO_NOT_EMPTY)
            {
                *pdataWrite1++ = spiSlaveReg->SPI_RXDR[0];
                length--;
            }
        }
    }

    return (ret);
}

/*----------------------------------------------------------------------
Name      : SPISWrite(void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
Desc      :
Params    :
Return    :
----------------------------------------------------------------------*/
int32 SPISWrite(eSPI_ch_t spiChNumb,void *pdata, SPI_DATA_WIDTH dataWidth, uint32 length)
{
    uint8 *pdataWrite1;
    uint16 *pdataWrite2;
    int32 ret=0;
    pSPI_REG spiSlaveReg;

    if(spiChNumb == SPI_CH0)
    {
        spiSlaveReg = (pSPI_REG) SPI0_BASE;
    }
    else if(spiChNumb == SPI_CH1)
    {
         spiSlaveReg = (pSPI_REG) SPI1_BASE;
    }
    else
    {
       ret = -1;
    }
    if ( 2 == dataWidth)
    {
        pdataWrite2 = (uint16 *) pdata;
        while (length != 0 )
        {
            if ((spiSlaveReg->SPI_SR & TRANSMIT_FIFO_NOT_FULL) == TRANSMIT_FIFO_NOT_FULL)
            {
                spiSlaveReg->SPI_TXDR[0] = *pdataWrite2++;
                length--;
            }
        }
    }
    else
    {
        pdataWrite1 = (uint8 *) pdata;
        while (length != 0 )
        {
            if ((spiSlaveReg->SPI_SR & TRANSMIT_FIFO_NOT_FULL) == TRANSMIT_FIFO_NOT_FULL)
            {
                spiSlaveReg->SPI_TXDR[0] = *pdataWrite1++;
                length--;
            }
        }
    }

    return (ret);
}
#endif
/*
********************************************************************************
*
*                         End of spi.c
*
********************************************************************************
*/

