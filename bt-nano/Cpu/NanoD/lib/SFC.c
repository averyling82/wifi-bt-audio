/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\SFC.c
* Owner: wangping
* Date: 2015.6.18
* Time: 9:51:49
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.18     9:51:49   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__


#include <typedef.h>
#include "SFC.h"
#include <DriverInclude.h>
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/* SFC_FSR Register*/
#define SFC_RXFULL                  (1 << 3)      //rx fifo full
#define SFC_RXEMPTY                 (1 << 2)      //rx fifo empty
#define SFC_TXEMPTY                 (1 << 1)      //tx fifo empty
#define SFC_TXFULL                  (1 << 0)      //tx fifo full

/* SFC_RCVR Register */
#define SFC_RESET                   (1 << 0)     //controller reset

/* SFC_SR Register */
#define SFC_BUSY                    (1 << 0)     //sfc busy flag. When busy , don’t try to set the control register

/* SFC_DMA_TRIGGER Register */
#define SFC_DMA_START               (1 << 0)     //Dma start trigger signal. Auto cleared after write

#define MIN(x, y)  ((x) < (y) ? (x) : (y))
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef union tagSFCFTLR_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*Tx fifo threshold level
        当TX FIFO中的数据个数小等于该水线的时候，会触发
        tx fifo empty interrupt*/
        unsigned txthresholdlevel : 8;
        /*Rx fifo threshold level
        当RX FIFO中的数据数大等于该水线的时候，会触发rx
        fifo full interrupt*/
        unsigned rxthresholdlevel : 8;

        unsigned reserved31_16 : 16;
    } b;
} SFCFTLR_DATA;

typedef union tagSFCFSR_DATA
{
    /** raw register data */
    uint32 d32;
    /** register bits */
    struct
    {
        /*tx fifo empty*/
        unsigned txempty : 1;
        /*tx fifo full*/
        unsigned txfull :  1;
        /*rx fifo empty*/
        unsigned rxempty : 1;
        /*Tx fifo empty interrupt mask*/
        unsigned rxfull :  1;

        unsigned reserved7_4 : 4;
        /*tx fifo waterlevel:
        0x0: fifo is full
        0x1:left 1 entry
        …
        0x10: left 16 entry,fifo is empty*/
        unsigned txlevel : 5;

        unsigned reserved15_13 : 3;
        /*rx fifo waterlevel:
        0x0: fifo is empty
        0x1: one entry is taken
        …
        0x10: 16 entry is taken fifo is full*/
        unsigned rxlevel : 5;

        unsigned reserved31_21 : 11;
    } b;
} SFCFSR_DATA;

//SFC Registers
typedef volatile struct tagSFC_STRUCT
{
    uint32 SFC_CTRL;
    uint32 SFC_IMR;
    uint32 SFC_ICLR;
    uint32 SFC_FTLR;
    uint32 SFC_RCVR;
    uint32 SFC_AX;
    uint32 SFC_ABIT;
    uint32 SFC_MASKISR;
    uint32 SFC_FSR;
    uint32 SFC_SR;
    uint32 SFC_RAWISR;
    uint32 reserved1[(0x080 - 0x02c)/4];
    uint32 SFC_DMA_TRIGGER;
    uint32 SFC_DMA_ADDR;
    uint32 reserved2[(0x100 - 0x088)/4];
    uint32 SFC_CMD;
    uint32 SFC_ADDR;
    uint32 SFC_DATA;
} SFC_REG,*pSFC_REG;

typedef volatile struct tagSFC_HOST
{
    uint32  signal;
    //uint8   EnableDMA;
} SFC_HOST, *pSFC_HOST;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SFC_HOST SFCHost;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern void InterruptInit(void);
extern void EnableSFCIntr(void);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/******************************************************************************
 * SFC_Delay -
 * DESCRIPTION: -
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:01:58
 * --------------------
 ******************************************************************************/
void SFC_Delay(uint32 us)
{
    DelayUs(us);
}

/******************************************************************************
 * SFC_Reset -
 * DESCRIPTION: - sfc reset
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
static void sfc_reset(pSFC_REG pReg)
{
    int32 timeout = 100000;

    pReg->SFC_RCVR = SFC_RESET;
    while ((pReg->SFC_RCVR == SFC_RESET) && (timeout > 0))
    {
        SFC_Delay(1);
        timeout--;
    }
    pReg->SFC_ICLR = 0xFFFFFFFF;
}

/******************************************************************************
 * SFC_Reset -
 * DESCRIPTION: - sfc_wait_busy
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
void sfc_wait_busy(void)
{

    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    while(pReg->SFC_SR & SFC_BUSY)
    {
        SFC_Delay(100);
    };
}


/******************************************************************************
 * sfc_bsp_init -
 * DESCRIPTION: - sfc bsp init
 *
 * Input:
 * Output:
 * Returns:
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
int32 sfc_bsp_init(void)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    pSFC_HOST   pSFC = &SFCHost;
    memset((void*)pSFC, 0, sizeof(SFC_HOST));
    sfc_reset(pReg);
    pReg->SFC_CTRL = 0;             //Data bits:1,Addr bits:1,Cmd bits:1, SPI mode 0
    return OK;
}

/******************************************************************************
 * _sfc_request -
 * DESCRIPTION: - send sfc request
 *
 * Input:uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data
 * Output:
 * Returns:ret
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
int32 _sfc_request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data)
{
    pSFC_REG        pReg = (pSFC_REG)SFC_BASE;
    int32           ret = SFC_OK;
    SFCCMD_DATA     cmd;
    volatile  int32 timeout = 0;

    if (!(pReg->SFC_FSR & SFC_TXEMPTY) || !(pReg->SFC_FSR & SFC_RXEMPTY) || (pReg->SFC_SR & SFC_BUSY))
    {
        sfc_reset(pReg);
    }

    cmd.d32 = sfcmd;
    if (SFC_ADDR_XBITS == cmd.b.addrbits)
    {
        printf("\n SFC ADDR XBITS");
        SFCCTRL_DATA ctrl;
        ctrl.d32 = sfctrl;
        if (!ctrl.b.addrbits)
        {
            printf("\n sfc para err");
            return SFC_PARAM_ERR;
        }
        pReg->SFC_ABIT = ctrl.b.addrbits-1;         //控制器内部会自动加1
    }

    //DelayMs(10);
    pReg->SFC_CTRL = sfctrl;
    //DelayMs(10);
    pReg->SFC_CMD = sfcmd;
    //DelayMs(10);
    if (cmd.b.addrbits)
        pReg->SFC_ADDR = addr;


#if 0

    if (cmd.b.datasize)
    {
#if (SFC_EN_DMA == 1)
        if (SFC_ENABLE_DMA & sfctrl)
        {
            pReg->SFC_ICLR = 0xFFFFFFFF;
            pReg->SFC_DMA_ADDR = (uint32)data;
            pReg->SFC_DMA_TRIGGER = SFC_DMA_START;
            timeout = cmd.b.datasize*10;

#if (SFC_EN_INT == 1)
            pReg->SFC_IMR &= ~FINISH_INT;    //not mask Transfer finish interrupt
            ret = SFC_WaitSignal(timeout, FINISH_INT);
            pReg->SFC_IMR |= FINISH_INT;
#else
            while ((!(pReg->SFC_RAWISR & FINISH_INT)) && (timeout-- > 0))
            {
                SFC_Delay(1);
            }
            pReg->SFC_ICLR = 0xFFFFFFFF;
            if (timeout <= 0)
                ret = SFC_WAIT_TIMEOUT;
#endif
        }
        else
#endif
        {
            uint32                  i, words;
            volatile SFCFSR_DATA    fifostat;
            uint32                  *pData = (uint32*)data;

            if (SFC_WRITE == cmd.b.rw) //write data
            {
                words  = (cmd.b.datasize+3)>>2;
                while (words)
                {

                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.txlevel > 0)
                    {
                        uint32 count = MIN(words, fifostat.b.txlevel);
                        for (i=0; i<count; i++)
                        {
                            pReg->SFC_DATA = *pData++;
                            words--;
                        }
                        if (0 == words)
                            break;
                        timeout = 0;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_TX_TIMEOUT;
                            break;
                        }
                    }
                }

            }
            else    //read data
            {
                uint32 bytes = cmd.b.datasize & 0x3;

                words = cmd.b.datasize>>2;
                while (words)
                {

                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.rxlevel > 0)
                    {
                        uint32 count = MIN(words, fifostat.b.rxlevel);
                        for (i=0; i<count; i++)
                        {
                            *pData++ = pReg->SFC_DATA;
                            words--;
                        }
                        if (0 == words)
                            break;
                        timeout = 0;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_RX_TIMEOUT;
                            break;
                        }
                    }
                }

                timeout = 0;
                while (bytes)
                {
                    fifostat.d32 = pReg->SFC_FSR;
                    if (fifostat.b.rxlevel > 0)
                    {
                        uint8 *pData1 = (uint8 *)pData;
                        words = pReg->SFC_DATA;
                        //memcpy(pData, &words, bytes);
                        for (i=0; i<bytes; i++)
                        {
                            pData1[i] = (uint8)((words>>(i*8)) & 0xFF);
                        }
                        break;
                    }
                    else
                    {
                        SFC_Delay(1);
                        if (timeout++ > 10000)
                        {
                            ret = SFC_RX_TIMEOUT;
                            break;
                        }
                    }
                }
            }
        }
    }

#endif

    timeout = 0;    //wait  cmd or data send complete
    while (!(pReg->SFC_FSR & SFC_TXEMPTY))
    {
        //DelayUs(1);
        rkos_sleep(1);
        if (timeout++ > 100000)         //wait 100ms
        {
            ret = SFC_TX_TIMEOUT;
            break;
        }
    }

    SFC_Delay(10); //CS# High Time (read/write) >100ns

    return ret;
}

/******************************************************************************
 * sfc_set_fifo_threshold -
 * DESCRIPTION: - sfc set fifo threshold
 *
 * Input:uint8 tx_fifo_threshold, uint8 rx_fifo_threshold
 * Output:
 * Returns:ret
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
void sfc_set_fifo_threshold(uint8 tx_fifo_threshold, uint8 rx_fifo_threshold)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    pReg->SFC_FTLR = tx_fifo_threshold | ((uint32)rx_fifo_threshold) << 8;
}

/******************************************************************************
 * sfc_read_fifo -
 * DESCRIPTION: - sfc set fifo threshold
 *
 * Input:uint8 *data , uint32 len
 * Output:
 * Returns:ret
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
uint32 sfc_read_fifo(uint8 *data , uint32 len)
{
    uint realcnt = 0;
    uint32                  i, words;
    volatile SFCFSR_DATA    fifostat;
    uint32                  *pData = (uint32*)data;
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    words  = (len+3)>>2;

    uint32 bytes = len & 0x3;

    words = len>>2;

    pData--;
    while (words)
    {
        fifostat.d32 = pReg->SFC_FSR;
        if (fifostat.b.rxlevel > 0)
        {
            uint32 count = MIN(words, fifostat.b.rxlevel);
            for (i=0; i<count; i++)
            {

                pData++;
                *pData =  pReg->SFC_DATA;
                 //printf("%x ", *(pData - 1));
                words--;
                realcnt += 4;
            }

            if (0 == words)
                break;

        }
        else
        {
            return realcnt;
        }
    }

    pData++;

    while (bytes)
    {
        fifostat.d32 = pReg->SFC_FSR;
        if (fifostat.b.rxlevel > 0)
        {
            uint8 *pData1 = (uint8 *)pData;
            words = pReg->SFC_DATA;
            //memcpy(pData, &words, bytes);
            for (i=0; i<bytes; i++)
            {
                pData1[i] = (uint8)((words >> (i*8)) & 0xFF);
                realcnt++;
            }
            break;
        }
        else
        {
            return realcnt;
        }
    }

    return realcnt;
}

/******************************************************************************
 * sfc_write_fifo -
 * DESCRIPTION: - sfc set fifo threshold
 *
 * Input:uint8 *data , uint32 len
 * Output:
 * Returns:ret
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
uint32 sfc_write_fifo(uint8 *data , uint32 len)
{
    uint32                  i, words;
    volatile SFCFSR_DATA    fifostat;
    uint32                  *pData = (uint32*)data;
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    words  = (len+3)>>2;
    uint realcnt = 0;

    fifostat.d32 = pReg->SFC_FSR;

    pData--;
    while(words)
    {
        if(fifostat.b.txlevel <= 0)
            break;

        uint32 count = MIN(words, fifostat.b.txlevel);

        for (i=0; i<count; i++)
        {
            pData++;
            pReg->SFC_DATA = *pData;

            //printf("%x ", *(pData - 1));
            words--;
            realcnt += 4;
        }

        fifostat.d32 = pReg->SFC_FSR;
    }

    return realcnt;

    //return (realcnt >= len) ? realcnt : len;
}

/******************************************************************************
 * sfc_get_int_status -
 * DESCRIPTION: - get int status
 *
 * Input:void
 * Output:int status
 * Returns:ret
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
uint32 sfc_get_int_status(void)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    uint32 status;
    status = pReg->SFC_MASKISR;
    pReg->SFC_ICLR = status;
    return status;
}

/******************************************************************************
 * sfc_get_fifo_status -
 * DESCRIPTION: - get int status
 *
 * Input:void
 * Output:void
 * Returns:status
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
uint32 sfc_get_fifo_status(void)
{
    volatile SFCFSR_DATA    fifostat;
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;
    fifostat.d32 = pReg->SFC_FSR;
}

/******************************************************************************
 * sfc_get_status -
 * DESCRIPTION: - get int status
 *
 * Input:void
 * Output:void
 * Returns:status
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
uint32 sfc_get_status(void)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;

    return pReg->SFC_SR;
}

/******************************************************************************
 * sfc_set_intmask -
 * DESCRIPTION: - sfc set intmask
 *
 * Input:void
 * Output:void
 * Returns:void
 *
 * modification history
 * --------------------
 * Owner: wangping
 * Date : 2015.6.18  written
 * Time : 10:02:18
 * --------------------
 ******************************************************************************/
void sfc_set_intmask(uint32 mask)
{
    pSFC_REG    pReg = (pSFC_REG)SFC_BASE;

    pReg->SFC_IMR = mask;
    pReg->SFC_ICLR = 0xFF;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif
