/*
********************************************************************************
*                   Copyright (c) 2008,Huweiguo
*                         All rights reserved.
*
* File Name£º   Hw_dma.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_DMA_H_
#define _HW_DMA_H_

#undef  EXT
#ifdef _IN_DMA_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                          Dma memap register define
*
*-------------------------------------------------------------------------------
*/
//cfg low word
#define         B_CFGL_CH_PRIOR(pri)     ((uint32)(pri)<<5)//pri = 0~2
#define         B_CFGL_CH_SUSP           ((uint32)(1)<<8)
#define         B_CFGL_FIFO_EMPTY        ((uint32)(1)<<9)
#define         B_CFGL_H_SEL_DST         ((uint32)(0)<<10)
#define         B_CFGL_S_SEL_DST         ((uint32)(1)<<10)
#define         B_CFGL_H_SEL_SRC         ((uint32)(0)<<11)
#define         B_CFGL_S_SEL_SRC         ((uint32)(1)<<11)
#define         B_CFGL_LOCK_CH_L_OTF     ((uint32)(0)<<12)
#define         B_CFGL_LOCK_CH_L_OBT     ((uint32)(1)<<12)
#define         B_CFGL_LOCK_CH_L_OTN     ((uint32)(2)<<12)
#define         B_CFGL_LOCK_B_L_OTF      ((uint32)(0)<<14)
#define         B_CFGL_LOCK_B_L_OBT      ((uint32)(1)<<14)
#define         B_CFGL_LOCK_B_L_OTN      ((uint32)(2)<<14)
#define         B_CFGL_LOCK_CH_EN        ((uint32)(0)<<16)
#define         B_CFGL_LOCK_B_EN         ((uint32)(0)<<17)
#define         B_CFGL_DST_HS_POL_H      ((uint32)(0)<<18)
#define         B_CFGL_DST_HS_POL_L      ((uint32)(1)<<18)
#define         B_CFGL_SRC_HS_POL_H      ((uint32)(0)<<19)
#define         B_CFGL_SRC_HS_POL_L      ((uint32)(1)<<19)
#define         B_CFGL_RELOAD_SRC        ((uint32)(1)<<30)
#define         B_CFGL_RELOAD_DST        ((uint32)(1)<<31)
//cfg high word
#define         B_CFGH_FCMODE            ((uint32)(1)<<0)
#define         B_CFGH_FIFO_MODE         ((uint32)(1)<<1)
#define         B_CFGH_PROTCTL           ((uint32)(1)<<2)
#define         B_CFGH_DS_UPD_EN         ((uint32)(1)<<5)
#define         B_CFGH_SS_UPD_EN         ((uint32)(1)<<6)
#define         B_CFGH_SRC_PER(HS)       ((uint32)(HS)<<7)
#define         B_CFGH_DST_PER(HS)       ((uint32)(HS)<<11)

//ctl low word
#define         B_CTLL_INT_EN            ((uint32)(1)<<0)
#define         B_CTLL_INT_UNEN          ((uint32)(0)<<0)
#define         B_CTLL_DST_TR_WIDTH_8    ((uint32)(0)<<1)
#define         B_CTLL_DST_TR_WIDTH_16   ((uint32)(1)<<1)
#define         B_CTLL_DST_TR_WIDTH_32   ((uint32)(2)<<1)
#define         B_CTLL_DST_TR_WIDTH_MASK ((uint32)(3)<<1)
#define         B_CTLL_SRC_TR_WIDTH_8    ((uint32)(0)<<4)
#define         B_CTLL_SRC_TR_WIDTH_16   ((uint32)(1)<<4)
#define         B_CTLL_SRC_TR_WIDTH_32   ((uint32)(2)<<4)
#define         B_CTLL_SRC_TR_WIDTH_MASK ((uint32)(3)<<4)
#define         B_CTLL_DINC_INC          ((uint32)(0)<<7)
#define         B_CTLL_DINC_DEC          ((uint32)(1)<<7)
#define         B_CTLL_DINC_UNC          ((uint32)(2)<<7)
#define         B_CTLL_DINC_MASK         ((uint32)(3)<<7)
#define         B_CTLL_SINC_INC          ((uint32)(0)<<9)
#define         B_CTLL_SINC_DEC          ((uint32)(1)<<9)
#define         B_CTLL_SINC_UNC          ((uint32)(2)<<9)
#define         B_CTLL_SINC_MASK         ((uint32)(3)<<9)
#define         B_CTLL_DST_MSIZE_1       ((uint32)(0)<<11)
#define         B_CTLL_DST_MSIZE_4       ((uint32)(1)<<11)
#define         B_CTLL_DST_MSIZE_8       ((uint32)(2)<<11)
#define         B_CTLL_DST_MSIZE_16      ((uint32)(3)<<11)
#define         B_CTLL_DST_MSIZE_32      ((uint32)(4)<<11)
#define         B_CTLL_SRC_MSIZE_1       ((uint32)(0)<<14)
#define         B_CTLL_SRC_MSIZE_4       ((uint32)(1)<<14)
#define         B_CTLL_SRC_MSIZE_8       ((uint32)(2)<<14)
#define         B_CTLL_SRC_MSIZE_16      ((uint32)(3)<<14)
#define         B_CTLL_SRC_MSIZE_32      ((uint32)(4)<<14)
#define         B_CTLL_SRC_GATHER        ((uint32)(1)<<17)
#define         B_CTLL_DST_SCATTER       ((uint32)(1)<<18)
#define         B_CTLL_MEM2MEM_DMAC      ((uint32)(0)<<20)
#define         B_CTLL_MEM2PER_DMAC      ((uint32)(1)<<20)
#define         B_CTLL_PER2MEM_DMAC      ((uint32)(2)<<20)
#define         B_CTLL_PER2MEM_PER       ((uint32)(4)<<20)
#define         B_CTLL_DMS_AM0           ((uint32)(0)<<23)
#define         B_CTLL_DMS_AM1           ((uint32)(1)<<23)
#define         B_CTLL_SMS_AM0           ((uint32)(0)<<25)
#define         B_CTLL_SMS_AM1           ((uint32)(1)<<25)
#define         B_CTLL_LLP_DST_EN        ((uint32)(1)<<27)
#define         B_CTLL_LLP_SRC_EN        ((uint32)(1)<<28)

typedef volatile struct tagDMA_CHN
{
    uint64 SAR;
    uint64 DAR;
    uint64 LLP;
    uint32 CTL_L;
    uint32 SIZE; //CTL_H
    uint64 SSTAT;
    uint64 DSTAT;
    uint64 SSTATAR;
    uint64 DSTATAR;
    uint32 CFG_L;
    uint32 CFG_H;
    uint64 SGR;
    uint64 DSR;
}DMA_CHN,*pDMA_CHN;

typedef volatile struct tagDMA_REG
{
    DMA_CHN CHANNEL[8];
    uint64 RawTfr;
    uint64 RawBlock;
    uint64 RawSrcTran;
    uint64 RawDstTran;
    uint64 RawErr;

    uint64 StatusTfr;
    uint64 StatusBlock;
    uint64 StatusSrcTran;
    uint64 StatusDstTran;
    uint64 StatusErr;

    uint64 MaskTfr;
    uint64 MaskBlock;
    uint64 MaskSrcTran;
    uint64 MaskDstTran;
    uint64 MaskErr;

    uint64 ClearTfr;
    uint64 ClearBlock;
    uint64 ClearSrcTran;
    uint64 ClearDstTran;
    uint64 ClearErr;
    uint64 StatusInt;  //0x360
    uint64 ReqSrcReg;
    uint64 ReqDstReg;
    uint64 SglReqSrcReg;
    uint64 SglReqDstReg;
    uint64 LstSrcReg;
    uint64 LstDstReg;
    uint64 DmaCfgReg;
    uint64 ChEnReg;
    uint64 DmaIdleReg;
    uint64 DmaTestReg;
    uint64 Pad[(0x3c0-0x3b8) / 8];
    uint64 DmaCompParams6;
    uint64 DmaCompParams5;
    uint64 DmaCompParams4;
    uint64 DmaCompParams3;
    uint64 DmaCompParams2;
    uint64 DmaCompParams1;
    uint64 DmaComponentID;

}DMA_REG, *pDMA_REG;
#define DmaReg              ((DMA_REG*)DMA_BASE)
#define DmaReg2              ((DMA_REG*)DMA_BASE2)

//llp register sturcture,the member serial order can not be changed.
typedef struct tagDMA_LLP
{
    uint32      SAR;
    uint32      DAR;
    struct      tagDMA_LLP  *LLP;
    uint32      CTLL;
    uint32      SIZE;   //CTL[63:32];
    uint32      DSTAT;

}DMA_LLP,*pDMA_LLP;

/*
********************************************************************************
*
*                         End of hw_dma.h
*
********************************************************************************
*/

#endif
