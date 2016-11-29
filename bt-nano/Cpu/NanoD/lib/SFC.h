/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\SFC.h
* Owner: wangping
* Date: 2015.6.18
* Time: 10:05:07
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.18     10:05:07   1.0
********************************************************************************************
*/


#ifndef __CPU_NANOD_LIB_SFC_H__
#define __CPU_NANOD_LIB_SFC_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_SFC_COMMON_  __attribute__((section("cpu_nanod_lib_sfc_common")))
#define _CPU_NANOD_LIB_SFC_INIT_    __attribute__((section("cpu_nanod_lib_sfc_init")))
#define _CPU_NANOD_LIB_SFC_SHELL_   __attribute__((section("cpu_nanod_lib_sfc_shell")))


#define SFC_MAX_IOSIZE              (1024*8)    //8K byte


#define SFC_EN_INT                  (0)         //enable interrupt
#define SFC_EN_DMA                  (0)         //enable dma

#define SFC_FIFO_DEPTH              (0x10)      // 16 words

/* FIFO watermark */
#define SFC_RX_WMARK                (SFC_FIFO_DEPTH)      //RX watermark level
#define SFC_TX_WMARK                (SFC_FIFO_DEPTH)       //TX watermark level
#define SFC_RX_WMARK_SHIFT          (8)
#define SFC_TX_WMARK_SHIFT          (0)

/*return value*/
#define SFC_OK                      (0)
#define SFC_ERROR                   (-1)
#define SFC_PARAM_ERR               (-2)
#define SFC_TX_TIMEOUT              (-3)
#define SFC_RX_TIMEOUT              (-4)
#define SFC_WAIT_TIMEOUT            (-5)
#define SFC_BUSY_TIMEOUT            (-6)
#define SFC_ECC_FAIL                (-7)
#define SFC_PROG_FAIL               (-8)
#define SFC_ERASE_FAIL              (-9)

/* SFC_CMD Register */
#define SFC_ADDR_0BITS              (0)
#define SFC_ADDR_24BITS             (1)
#define SFC_ADDR_32BITS             (2)
#define SFC_ADDR_XBITS              (3)

#define SFC_WRITE                   (1)
#define SFC_READ                    (0)

/* SFC_CTRL Register */
#define SFC_1BITS_LINE              (0)
#define SFC_2BITS_LINE              (1)
#define SFC_4BITS_LINE              (2)

#define SFC_ENABLE_DMA              (1<<14)

#define DMA_INT                     (1 << 7)      //dma interrupt
#define NSPIERR_INT                 (1 << 6)      //Nspi error interrupt
#define AHBERR_INT                  (1 << 5)      //Ahb bus error interrupt
#define FINISH_INT                  (1 << 4)      //Transfer finish interrupt
#define TXEMPTY_INT                 (1 << 3)      //Tx fifo empty interrupt
#define TXOF_INT                    (1 << 2)      //Tx fifo overflow interrupt
#define RXUF_INT                    (1 << 1)      //Rx fifo underflow interrupt
#define RXFULL_INT                  (1 << 0)      //Rx fifo full interrupt

#define MIN(x, y)  ((x) < (y) ? (x) : (y))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum
{
    DATA_LINES_X1 = 0,
    DATA_LINES_X2,
    DATA_LINES_X4
} SFC_DATA_LINES;


typedef union tagSFCCTRL_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*spi mode select*/
        unsigned mode : 1;
        /*Shift in phase selection
        0: shift in the flash data at posedge sclk_out
        1: shift in the flash data at negedge sclk_out*/
        unsigned sps : 1;

        unsigned reserved3_2 : 2;
        /*sclk_idle_level_cycles*/
        unsigned scic : 4;
        /*Cmd bits number*/
        unsigned cmdlines : 2;
        /*Address bits number*/
        unsigned addrlines : 2;
        /*Data bits number*/
        unsigned datalines : 2;
        /*this bit is not exit in regiseter, just use for code param*/
        unsigned enbledma : 1;

        unsigned reserved15 : 1;

        unsigned addrbits : 5;

        unsigned reserved31_21 : 11;
    } b;
}SFCCTRL_DATA;

typedef union tagSFCCMD_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*Command that will send to Serial Flash*/
        unsigned cmd : 8;               //
        /*Dummy bits number*/
        unsigned dummybits : 4;
        /*0:read, 1: write*/
        unsigned rw : 1;
        /*Continuous read mode*/
        unsigned readmode : 1;
        /*Address bits number*/
        unsigned addrbits : 2;
        /*Transferred bytes number*/
        unsigned datasize : 14;
        /*Chip select*/
        unsigned cs : 2;
    } b;
}SFCCMD_DATA;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void SFC_Delay(uint32 us);
extern void sfc_wait_busy(void);


#endif
