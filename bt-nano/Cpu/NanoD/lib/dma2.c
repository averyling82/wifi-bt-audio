/*
********************************************************************************
*                   Copyright (c) 2008,Huweiguo
*                         All rights reserved.
*
* File Name£º   hw_dma.c
*
* Description:´ÓRK27ÒÆÖ²¹ýÀ´
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _IN_DMA_

#include <typedef.h>
#include <BspConfig.h>
#include <DriverInclude.h>

//#ifdef DRIVERLIB_IRAM
/*
--------------------------------------------------------------------------------
  Function name : DMAStart(UINT32 ch, UINT32 src, UINT32 dst, UINT32 size, UINT32 mode)
  Author        :
  Description   :

  Input         : [ch]       --  the channel number that DMA use for data transfer.
                  [src]      --  source address,WORD alignment
                  [dst]      --  destination£¬WORD alignment
                  [size]     --  transfer length.WORD(4 byte) is unit
                  [g_dmaPar] --  parameter.


  Return        : DMAState

  History:     <author>         <time>         <version>
               yangwenjie    2009-1-5        Ver1.0
  desc:         the unit of DMA transfer must be word.
--------------------------------------------------------------------------------
*/
rk_err_t DmaGetState2(uint32 ch)
{
    uint32 tmp;

    tmp = (((uint32)(DmaReg2->ChEnReg)) & ((uint32)(0x01) << ch));

    if (tmp != 0)
    {
        return DMA_BUSY;
    }

    return DMA_IDLE;
}

rk_err_t DmaDisableInt2(uint32 ch)
{
        //int status clear
    DmaReg2->ClearTfr = (0x01u  << ch);
    DmaReg2->MaskTfr  = (0x0100ul << ch);
    return RK_SUCCESS;
}


rk_err_t DmaEnableInt2(uint32 ch)
{
        //int status clear
    DmaReg2->ClearTfr = (0x01u  << ch);
    DmaReg2->MaskTfr  = (0x0101ul << ch);
    return RK_SUCCESS;
}


rk_err_t DmaStop2(uint32 ch)
{
    DmaReg2->ChEnReg &= ~((uint32)(0x01) << ch);
}


rk_err_t DmaReConfig2(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_LLP pllplist)
{
    DmaReg2->CHANNEL[ch].SAR   = srcAddr;
    DmaReg2->CHANNEL[ch].DAR   = dstAddr;

    if (size > DMA_MAX_BLOCK_SIZE)
    {
        DmaReg2->CHANNEL[ch].LLP = (uint64)(((uint32)(&pllplist[0])));
    }
    else
    {
        DmaReg2->CHANNEL[ch].LLP = 0;
    }

    DmaReg2->ChEnReg  |= (0x101 << ch);

    return(RK_SUCCESS);
}

/*
--------------------------------------------------------------------------------
  Function name : DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                       pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                       pDMA_LLP pllplist)
  Author        :
  Description   : DmaStart call this function, main destination is used for saving to rom or flash memory

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
rk_err_t DmaConfig_for_LLP2(uint32 ch,uint32 size ,uint32 block_size,pDMA_CFGX g_dmaPar, pDMA_LLP pllplist)
{
    uint32 i;
    int32  llpSrcOffset;
    int32  llpDstOffset;
    uint32 llpNum;
    uint32 remaind;

    DmaReg2->CHANNEL[ch].CFG_L = g_dmaPar->CFGL;
    DmaReg2->CHANNEL[ch].CFG_H = g_dmaPar->CFGH;
    g_dmaPar->CTLL |= B_CTLL_INT_EN;

    if (size < LLP_BLOCK_SIZE)
    {
        if(pllplist)
        {
            for (i = 0; i < block_size ; i++)
            {
                pllplist[i].LLP = (struct tagDMA_LLP *)((uint32)(&pllplist[i+1]));
                pllplist[i].CTLL = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            }

            pllplist[i-1].LLP  = NULL;

            DmaReg2->CHANNEL[ch].CTL_L = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            DmaReg2->CHANNEL[ch].LLP   = (uint64)(((uint32)(&pllplist[0])));
        }
        else
        {
            DmaReg2->CHANNEL[ch].CTL_L = g_dmaPar->CTLL;
            DmaReg2->CHANNEL[ch].SIZE  = size;
            DmaReg2->CHANNEL[ch].LLP   = 0;
        }
    }

    //DMA Transmit Enable
    DmaReg2->DmaCfgReg = 1;
    DmaReg2->ChEnReg  |= (0x101 << ch);       //DMA Channel Enable

    return(RK_SUCCESS);

}
/*
--------------------------------------------------------------------------------
  Function name : DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size,
                       pDMA_CFGX g_dmaPar, pFunc CallBack, pFunc *pCallBackBuf,
                       pDMA_LLP pllplist)
  Author        :
  Description   : DmaStart call this function, main destination is used for saving to rom or flash memory

  Input         :

  Return        : DMAState

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
rk_err_t DmaConfig2(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar, pDMA_LLP pllplist)
{
    uint32 i;
    int32  llpSrcOffset;
    int32  llpDstOffset;
    uint32 llpNum;
    uint32 remaind;

    //dma channel configer
    DmaReg2->CHANNEL[ch].SAR   = srcAddr;
    DmaReg2->CHANNEL[ch].DAR   = dstAddr;
    DmaReg2->CHANNEL[ch].CFG_L = g_dmaPar->CFGL;
    DmaReg2->CHANNEL[ch].CFG_H = g_dmaPar->CFGH;

    g_dmaPar->CTLL |= B_CTLL_INT_EN;

    if (size > LLP_BLOCK_SIZE)
    {
        llpNum  = (size + (LLP_BLOCK_SIZE - 1)) / LLP_BLOCK_SIZE;
        remaind = size - LLP_BLOCK_SIZE * (llpNum - 1);
        if (remaind == 0) remaind = LLP_BLOCK_SIZE;

        llpSrcOffset = 0x0;
        if((g_dmaPar->CTLL & B_CTLL_SINC_MASK) == B_CTLL_SINC_INC)  //incr
        {
            llpSrcOffset = LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_SRC_TR_WIDTH_MASK) >> 4);
        }
        else if((g_dmaPar->CTLL & B_CTLL_SINC_MASK) == B_CTLL_SINC_DEC)  //dec
        {
            llpSrcOffset = -(LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_SRC_TR_WIDTH_MASK) >> 4));
        }

        llpDstOffset = 0x0;
        if((g_dmaPar->CTLL & B_CTLL_DINC_MASK) == B_CTLL_DINC_INC)  //incr
        {
            llpDstOffset = LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_DST_TR_WIDTH_MASK) >> 1);
        }
        else if((g_dmaPar->CTLL & B_CTLL_DINC_MASK) == B_CTLL_DINC_DEC) //decr
        {
            llpDstOffset = -(LLP_BLOCK_SIZE << ((g_dmaPar->CTLL & B_CTLL_DST_TR_WIDTH_MASK) >> 1));
        }

        for (i = 0; i < llpNum ; i++)
        {
            pllplist[i].SAR = srcAddr + llpSrcOffset * i;
            pllplist[i].DAR = dstAddr + llpDstOffset * i;
            pllplist[i].LLP = (struct tagDMA_LLP *)((uint32)(&pllplist[i+1]));
            pllplist[i].CTLL = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
            pllplist[i].SIZE = LLP_BLOCK_SIZE;
        }

        pllplist[i-1].LLP  = 0;
        pllplist[i-1].SIZE = remaind;

        DmaReg2->CHANNEL[ch].CTL_L = g_dmaPar->CTLL | (B_CTLL_LLP_SRC_EN | B_CTLL_LLP_DST_EN);
        DmaReg2->CHANNEL[ch].LLP   = (uint64)(((uint32)(&pllplist[0])));
    }
    else
    {
        DmaReg2->CHANNEL[ch].CTL_L = g_dmaPar->CTLL;
        DmaReg2->CHANNEL[ch].SIZE  = size;
        DmaReg2->CHANNEL[ch].LLP   = 0;
    }
    //DMA Transmit Enable
    DmaReg2->DmaCfgReg = 1;
    DmaReg2->ChEnReg  |= (0x101 << ch);       //DMA Channel Enable

    return(RK_SUCCESS);
}
//#endif


/*
--------------------------------------------------------------------------------
  Function name : void DmaSuspend(uint32 ch)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
void DmaSuspend2(uint32 ch)
{
  uint32 delaycnt = 800;

  if (DMA_BUSY == DmaGetState(ch))
    {
		DmaReg2->CHANNEL[ch].CFG_L |= B_CFGL_CH_SUSP;

		while(delaycnt)
		{
		    delaycnt--;
		    if ((DmaReg2->CHANNEL[ch].CFG_L & B_CFGL_FIFO_EMPTY))
		    {
		        break;
		    }
		}
    }
}

/*
--------------------------------------------------------------------------------
  Function name : void DmaResume(uint32 ch)
  Author        :
  Description   :

  Input         :
  Return        :

  History:     <author>         <time>         <version>
  desc:
--------------------------------------------------------------------------------
*/
void DmaResume2(uint32 ch)
{
    if (DMA_BUSY == DmaGetState(ch))
    {
        DmaReg2->CHANNEL[ch].CFG_L &= ~B_CFGL_CH_SUSP;
    }
}
/*
********************************************************************************
*
*                         End of dma.c
*
********************************************************************************
*/

