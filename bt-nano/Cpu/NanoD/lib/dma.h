/*
********************************************************************************
*                   Copyright (c) 2008,Huiweiguo
*                         All rights reserved.
*
* File Name：  dma.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*               HuWeiGuo        2009-01-05         1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _DMA_H_
#define _DMA_H_

#undef  EXT
#ifdef _IN_DMA_
#define EXT
#else
#define EXT extern
#endif
/*
--------------------------------------------------------------------------------

                        Macro define

--------------------------------------------------------------------------------
*/
#define DMA_MAX_BLOCK_SIZE      4088        //do not changed.

#define LLP_MAX_NUM     8
#define LLP_BLOCK_SIZE  4088

//------------------------------------------------------------------------------
/*
//DMA配置参数说明，以I2S举例
#define     DMA_CTLL_I2S0_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \     :数据位宽
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \        : FIFO/MEMORY 偏移方式。FIFO一般为UNC；MEMORY一般为INC
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\   :一次发送数据位宽的个数 I2S为32bit*8
                            |B_CTLL_MEM2PER_DMAC \                      :数据方向，这里为MEMORY到PER外设
                            |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)            :DMA总线选择，一般错开选择0与1

#define     DMA_CFGL_I2S0_TX (B_CFGL_CH_PRIOR(2) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)
            //B_CFGL_CH_PRIOR(2)为优先级，0-7八个优先级；0为最小
            //B_CFGL_H_SEL_DST表示目标源为H，即硬件
            //B_CFGL_S_SEL_SRC表示起源为S，即软件

#define     DMA_CFGH_I2S0_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(0))
            //B_CFGH_FCMODE : 流控，一般为1开启
            //B_CFGH_FIFO_MODE : 配置DMA请求启动前FIFO的剩余大小，一般为1，即FIFO剩余大于一半才启动DMA请求
            //B_CFGH_PROTCTL : 一般为1.协议建议配置
            //B_CFGH_DST_PER : 硬件握手标号 I2S0为0；
*/

//M2M SINGLE
#define     DMA_CTLL_M2M_WORD (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                                |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                                |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                                |B_CTLL_MEM2MEM_DMAC \
                                |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2M_WORD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2M_WORD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)



//M2IMDCT
#define     DMA_CTLL_M2IMDCT (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_M2IMDCT (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2IMDCT (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//IMDCT2M
#define     DMA_CTLL_IMDCT2M (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_IMDCT2M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_IMDCT2M (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//M2SYNTH
#define     DMA_CTLL_M2SYNTH (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_M2SYNTH (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2SYNTH (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)


//SYNTH2M
#define     DMA_CTLL_SYNTH2M (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SYNTH2M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SYNTH2M (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//----------------------------------------------------------------------------------------------------//
//M2I2S_0 MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S0_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_I2S0_TX (B_CFGL_CH_PRIOR(2) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_I2S0_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(0))

//I2S_02M MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S0_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_I2S0_RX (B_CFGL_CH_PRIOR(2) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_I2S0_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(1))
//----------------------------------------------------------------------------------------------------//

//M2I2S_1 MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S1_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_I2S1_TX (B_CFGL_CH_PRIOR(2) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_I2S1_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(2))

//I2S_12M MSIZE不能使用16，否则I2S端接收错误?
#define     DMA_CTLL_I2S1_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                            |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_I2S1_RX (B_CFGL_CH_PRIOR(2) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_I2S1_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(3))

//----------------------------------------------------------------------------------------------------//

//M2SPI_0 MASTER 8BIT
#define     DMA_CTLL_SPI0_8_MTX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_8_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_8_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))


//SPI_02M MASTER 8BIT
#define     DMA_CTLL_SPI0_8_MRX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_8_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_8_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_0 SLAVE 8BIT
#define     DMA_CTLL_SPI0_8_STX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_32 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_8_STX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_8_STX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))


//SPI_02M SLAVE 8BIT
#define     DMA_CTLL_SPI0_8_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_8_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_8_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))

//----------------------------------------------------------------------------------------------------//

//M2SPI_0 MASTER 16BIT
#define     DMA_CTLL_SPI0_16_MTX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_16_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_16_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))

//SPI_02M MASTER 16BIT
#define     DMA_CTLL_SPI0_16_MRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_16_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_16_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_0 SLAVE 16BIT
#define     DMA_CTLL_SPI0_16_STX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI0_16_STX   (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI0_16_STX   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(4))

//SPI_02M SLAVE 16BIT
#define     DMA_CTLL_SPI0_16_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI0_16_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI0_16_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(5))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 MASTER 8BIT
#define     DMA_CTLL_SPI1_8_MTX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_8_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_8_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))


//SPI_12M MASTER 8BIT
#define     DMA_CTLL_SPI1_8_MRX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_8_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_8_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 SLAVE 8BIT
#define     DMA_CTLL_SPI1_8_STX (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_32 \
                              |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                              |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4 \
                              |B_CTLL_MEM2PER_DMAC \
                              |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_8_STX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_8_STX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))


//SPI_12M SLAVE 8BIT
#define     DMA_CTLL_SPI1_8_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_8 \
                              |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                              |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                              |B_CTLL_PER2MEM_DMAC \
                              |B_CTLL_DMS_AM0 |B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_8_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_8_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))

//----------------------------------------------------------------------------------------------------//

//M2SPI_1 MASTER 16BIT
#define     DMA_CTLL_SPI1_16_MTX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 |B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_16_MTX (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_16_MTX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))

//SPI_12M MASTER 16BIT
#define     DMA_CTLL_SPI1_16_MRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_16_MRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_16_MRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//

//M2SPI_1 SLAVE 16BIT
#define     DMA_CTLL_SPI1_16_STX (B_CTLL_DST_TR_WIDTH_16 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_4\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SPI1_16_STX   (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SPI1_16_STX   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(6))

//SPI_12M SLAVE 16BIT
#define     DMA_CTLL_SPI1_16_SRX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_16 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SPI1_16_SRX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SPI1_16_SRX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(7))
//----------------------------------------------------------------------------------------------------//
//M2UART_02
#define     DMA_CTLL_M2UART_02 (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2UART_02 (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2UART_02   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(8))


//UART_022M
#define     DMA_CTLL_UART_022M (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_UART_022M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_UART_022M   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(9))
//----------------------------------------------------------------------------------------------------//
//M2UART_1
#define     DMA_CTLL_M2UART_1 (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_MEM2PER_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_M2UART_1 (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_M2UART_1   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(10))


//UART_12M
#define     DMA_CTLL_UART_12M (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_4 | B_CTLL_SRC_MSIZE_4 \
                            |B_CTLL_PER2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_UART_12M (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_UART_12M   (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(11))
//----------------------------------------------------------------------------------------------------//
//M2SDMMC
#define     DMA_CTLL_SDMMC_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_SDMMC_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_SDMMC_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(12))

//SDMMC2M
#define     DMA_CTLL_SDMMC_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_SDMMC_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_SDMMC_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(12))
//----------------------------------------------------------------------------------------------------//
//M2EMMC
#define     DMA_CTLL_EMMC_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_EMMC_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_EMMC_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(13))

//EMMC2M
#define     DMA_CTLL_EMMC_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_EMMC_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_EMMC_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(13))
//----------------------------------------------------------------------------------------------------//
//M2VOP
#define     DMA_CTLL_VOP_WR  (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_VOP_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_VOP_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(14))

//----------------------------------------------------------------------------------------------------//
//M2HIFI
#define     DMA_CTLL_HIFI_WR  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_MEM2PER_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_HIFI_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_H_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_HIFI_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_DST_PER(1))

//HIFI2M
#define     DMA_CTLL_HIFI_RD  (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_8 | B_CTLL_SRC_MSIZE_8\
                               |B_CTLL_PER2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_HIFI_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_H_SEL_SRC)

#define     DMA_CFGH_HIFI_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL | B_CFGH_SRC_PER(0))
//----------------------------------------------------------------------------------------------------//
//M2NAND
#define     DMA_CTLL_NNAD_WR  (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                               |B_CTLL_DINC_UNC | B_CTLL_SINC_INC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2MEM_DMAC \
                               |B_CTLL_DMS_AM0 | B_CTLL_SMS_AM1)

#define     DMA_CFGL_NAND_WR  (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_NAND_WR  (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//NANDC2M
#define     DMA_CTLL_NNAD_RD  (B_CTLL_DST_TR_WIDTH_8 | B_CTLL_SRC_TR_WIDTH_8 \
                               |B_CTLL_DINC_INC | B_CTLL_SINC_UNC \
                               |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32\
                               |B_CTLL_MEM2MEM_DMAC \
                               |B_CTLL_DMS_AM1 | B_CTLL_SMS_AM0)

#define     DMA_CFGL_NAND_RD (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_NAND_RD (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)
//----------------------------------------------------------------------------------------------------//


//M2USB DEVICE
#define     DMA_CTLL_USB_TX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_UNC| B_CTLL_SINC_INC \
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM0  | B_CTLL_SMS_AM1)

#define     DMA_CFGL_USB_TX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_USB_TX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)

//USB2M DEVICE
#define     DMA_CTLL_USB_RX (B_CTLL_DST_TR_WIDTH_32 | B_CTLL_SRC_TR_WIDTH_32 \
                            |B_CTLL_DINC_INC | B_CTLL_SINC_UNC\
                            |B_CTLL_DST_MSIZE_32 | B_CTLL_SRC_MSIZE_32 \
                            |B_CTLL_MEM2MEM_DMAC \
                            |B_CTLL_DMS_AM1  | B_CTLL_SMS_AM0)

#define     DMA_CFGL_USB_RX (B_CFGL_CH_PRIOR(0) | B_CFGL_S_SEL_DST | B_CFGL_S_SEL_SRC)

#define     DMA_CFGH_USB_RX (B_CFGH_FCMODE | B_CFGH_FIFO_MODE | B_CFGH_PROTCTL)
//----------------------------------------------------------------------------------------------------//

/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/
typedef enum
{

    DMA_BUSY    = (int32)(1),
    DMA_IDLE    = (int32)(2)

}DMAState;

typedef enum _DMA_CHN
{
    DMA_CHN0 = (uint32)(0),
    DMA_CHN1,
    DMA_CHN2,
    DMA_CHN3,
    DMA_CHN4,
    DMA_CHN5,
    DMA_CHN_MAX

}eDMA_CHN;

typedef struct tagDMA_CFGX
{
    uint32   CTLL;
    uint32   CFGL;
    uint32   CFGH;
    void    *pLLP;         //llp list addr

}DMA_CFGX,*pDMA_CFGX;

/*
--------------------------------------------------------------------------------

                        Variable Define

--------------------------------------------------------------------------------
*/

/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/
//#ifdef DRIVERLIB_IRAM

extern rk_err_t DmaGetState(uint32 ch);
extern rk_err_t DmaConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar,pDMA_LLP pllplist);
extern rk_err_t DmaStop(uint32 ch);
extern rk_err_t DmaReConfig(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_LLP pllplist);

extern void     DmaSuspend(uint32 ch);
extern void     DmaResume(uint32 ch);


extern rk_err_t DmaGetState2(uint32 ch);
extern rk_err_t DmaConfig2(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_CFGX g_dmaPar,pDMA_LLP pllplist);
extern rk_err_t DmaStop2(uint32 ch);
extern rk_err_t DmaReConfig2(uint32 ch, uint32 srcAddr, uint32 dstAddr, uint32 size, pDMA_LLP pllplist);

extern void     DmaSuspend2(uint32 ch);
extern void     DmaResume2(uint32 ch);

/*
********************************************************************************
*
*                         End of dma.h
*
********************************************************************************
*/
#endif

