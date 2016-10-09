/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: ..\Cpu\NanoC\lib\sdmmc.c
* Owner: Aaron.sun
* Date: 2014.2.20
* Time: 10:10:23
* Desc: sd mmc control bsp driver
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.2.20     10:10:23   1.0
********************************************************************************************
*/

#include "BspConfig.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "typedef.h"
#include "DriverInclude.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define RX_WMARK_SHIFT    (16)
#define TX_WMARK_SHIFT    (0)

uint32 SDMMC_GROUP[2] = {
    EMMC_ADDR,
    SDC0_ADDR,
};

uint32 SDMMC_FIFO_GROUP[2] = {
    EMMC_FIFO_ADDR,
    SDC0_FIFO_ADDR,
};


#define SDMMC(n)        ((SDC_REG_T *)(SDMMC_GROUP[n]))
#define SDMMCFIFO(n)    ((volatile uint32 *)(SDMMC_FIFO_GROUP[n]))


/***************************************************************/
//可配置的参数
/***************************************************************/
/* FIFO watermark */
#define RX_WMARK          (FIFO_DEPTH/2-1)      //RX watermark level set to 127
#define TX_WMARK          (FIFO_DEPTH/2)       //TX watermark level set to  128

#define DEBOUNCE_TIME         (25)     //卡拔插的消抖动时间,单位ms


/* SDMMC Control Register */
#define ENABLE_DMA        (1 << 5)     //Enable DMA transfer mode
#define ENABLE_INT        (1 << 4)     //Enable interrupt
#define DMA_RESET        (1 << 2)     //FIFO reset
#define FIFO_RESET        (1 << 1)     //FIFO reset
#define SDC_RESET         (1 << 0)     //controller reset

/* SDMMC FIFO Register */
#define SD_MSIZE_1        (0x0 << 28)  //DW_DMA_Multiple_Transaction_Size
#define SD_MSIZE_4        (0x1 << 28)
#define SD_MSIZE_8        (0x1 << 28)
#define SD_MSIZE_16       (0x3 << 28)
#define SD_MSIZE_32       (0x4 << 28)
#define SD_MSIZE_64       (0x5 << 28)
#define SD_MSIZE_128      (0x6 << 28)
#define SD_MSIZE_256      (0x7 << 28)

/* Card Type Register */
#define BUS_1_BIT         (0x0)
#define BUS_4_BIT         (0x1)
#define BUS_8_BIT         (0x10000)

/* SDMMC Clock source Register */
#define CLK_DIV_SRC_0     (0x0)        //clock divider 0 selected
#define CLK_DIV_SRC_1     (0x1)        //clock divider 1 selected
#define CLK_DIV_SRC_2     (0x2)        //clock divider 2 selected
#define CLK_DIV_SRC_3     (0x3)        //clock divider 3 selected

/* interrupt mask bit */
#define SDIO_INT          (1 << 16)    //SDIO interrupt
#define BDONE_INT          (1 << 16)   //busy Done interrupt
#define EBE_INT           (1 << 15)    //End Bit Error(read)/Write no CRC
#define ACD_INT           (1 << 14)    //Auto Command Done
#define SBE_INT           (1 << 13)    //Start Bit Error
#define HLE_INT           (1 << 12)    //Hardware Locked Write Error
#define FRUN_INT          (1 << 11)    //FIFO Underrun/Overrun Error
#define HTO_INT           (1 << 10)    //Data Starvation by Host Timeout
#define VSWTCH_INT        (1 << 10)    //Volt Switch interrupt
#define DRTO_INT          (1 << 9)     //Data Read TimeOut
#define RTO_INT           (1 << 8)     //Response TimeOut
#define DCRC_INT          (1 << 7)     //Data CRC Error
#define RCRC_INT          (1 << 6)     //Response CRC Error
#define RXDR_INT          (1 << 5)     //Receive FIFO Data Request
#define TXDR_INT          (1 << 4)     //Transmit FIFO Data Request
#define DTO_INT           (1 << 3)     //Data Transfer Over
#define CD_INT            (1 << 2)     //Command Done
#define RE_INT            (1 << 1)     //Response Error
#define CDT_INT           (1 << 0)     //Card Detect

/* Clock Enable Register */
#define CCLK_LOW_POWER    (1 << 16)    //Low-power control for SD/MMC card clock
#define NO_CCLK_LOW_POWER (0 << 16)    //low-power mode disabled
#define CCLK_ENABLE       (1 << 0)     //clock enable control for SD/MMC card clock
#define CCLK_DISABLE      (0 << 0)     //clock disabled

/* SDMMC status Register */
#define DATA_BUSY           (1 << 9)     //Card busy
#define FIFO_FULL           (1 << 3)     //FIFO is full status
#define FIFO_EMPTY          (1 << 2)     //FIFO is empty status


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
rk_err_t SDC_StartCmd(SDMMC_PORT SDCPort, uint32 cmd);
rk_err_t SDC_SetClock(SDMMC_PORT SDCPort, bool enable);
void SDC_Delay(uint32 us);


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: SDC_DisableDMA
** Input:void
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.25
** Time: 16:51:42
*******************************************************************************/
COMMON API void SDC_DisableDMA(SDMMC_PORT SDCPort)
{
        SDC_REG_T *      pReg = SDMMC(SDCPort);
        pReg->SDMMC_CTRL &=~ENABLE_DMA;
}

/*******************************************************************************
** Name: SDC_EnableDMA
** Input:SDMMC_PORT SDCPort
** Return: void
** Owner:Aaron.sun
** Date: 2014.4.25
** Time: 16:50:45
*******************************************************************************/
COMMON API void SDC_EnableDMA(SDMMC_PORT SDCPort)
{
     SDC_REG_T *      pReg = SDMMC(SDCPort);
     pReg->SDMMC_CTRL |= ENABLE_DMA;
}

/*******************************************************************************
** Name: SDC_GetFIFOAddr
** Input:SDMMC_PORT SDCPort
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.4.25
** Time: 15:26:46
*******************************************************************************/
COMMON API uint32 SDC_GetFIFOAddr(SDMMC_PORT SDCPort)
{
    return (uint32)SDMMCFIFO(SDCPort);
}

/*******************************************************************************
** Name: SDC_ClearIRQ
** Input:SDMMC_PORT SDCPort
** Return: void
** Owner:Aaron.sun
** Date: 2014.3.4
** Time: 11:24:22
*******************************************************************************/
void SDC_ClearIRQ(SDMMC_PORT SDCPort, uint32 intmask)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    pReg->SDMMC_RINISTS = intmask;
}


/*******************************************************************************
** Name: SDC_GetResponse
** Input:uint32 ResponseNum
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 17:25:41
*******************************************************************************/
uint32 SDC_GetResponse(SDMMC_PORT SDCPort, uint32 ResponseNum)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);

    if(ResponseNum == 0)
    {
        return pReg->SDMMC_RESP0;
    }
    else if(ResponseNum == 1)
    {
        return pReg->SDMMC_RESP1;
    }
    else if(ResponseNum == 2)
    {
        return pReg->SDMMC_RESP2;
    }
    else if(ResponseNum == 3)
    {
        return pReg->SDMMC_RESP3;
    }

}
/*******************************************************************************
** Name: SDC_SetByteCnt
** Input:SDMMC_PORT SDCPort, uint32 ByteCnt
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 16:23:34
*******************************************************************************/
rk_err_t SDC_SetByteCnt(SDMMC_PORT SDCPort, uint32 ByteCnt)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    pReg->SDMMC_BYTCNT = ByteCnt;
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SDC_SetCmdArg
** Input:SDMMC_PORT SDCPort, uint32 CmdArg
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 16:22:30
*******************************************************************************/
rk_err_t SDC_SetCmdArg(SDMMC_PORT SDCPort, uint32 CmdArg)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    pReg->SDMMC_CMDARG = CmdArg;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SDC_SetBlockSize
** Input:SDMMC_PORT SDCPort, uint32 BlockSize
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 16:20:59
*******************************************************************************/
rk_err_t SDC_SetBlockSize(SDMMC_PORT SDCPort, uint32 BlockSize)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    pReg->SDMMC_BLKSIZ = BlockSize;
    return RK_SUCCESS;
}



/*******************************************************************************
** Name: SDC_SetDmaMode
** Input:uint32 OpenDma
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 9:04:38
*******************************************************************************/
rk_err_t SDC_SetDmaMode(SDMMC_PORT SDCPort, uint32 OpenDma)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    volatile uint32  value = 0;
    if (OpenDma)
    {
        value &= ~(RXDR_INT | TXDR_INT);
        pReg->SDMMC_INTMASK = value;
        pReg->SDMMC_CTRL |= ENABLE_DMA;
    }
    else
    {
        value |= (RXDR_INT | TXDR_INT);
        pReg->SDMMC_INTMASK = value;
        pReg->SDMMC_CTRL &= ~ENABLE_DMA;
    }
}

/*******************************************************************************
** Name: SDC_GetIRQ
** Input:SDMMC_PORT
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.2.21
** Time: 8:40:53
*******************************************************************************/
uint32 SDC_GetIRQ(SDMMC_PORT SDCPort, uint32 * Int)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);
    volatile uint32  value = 0;
    uint32 event = 0;

    value = pReg->SDMMC_MINTSTS;
    *Int = value;

    if (value & RXDR_INT)
    {
        event |= SDC_EVENT_RX_DATA_REQ;
    }
    if (value & TXDR_INT)
    {
        event |= SDC_EVENT_TX_DATA_REQ;
    }

    if (value & CD_INT)
    {
        event |= SDC_EVENT_CMD_OK;
    }

    if (value & DTO_INT)
    {
        event |= SDC_EVENT_DATA_TRANS_OK;
    }

    if (value & (SBE_INT | EBE_INT | FRUN_INT | DRTO_INT | DCRC_INT | RCRC_INT | RE_INT))
    {
        printf("\n sdc err = %x SDCPort = %d", value, SDCPort);
        event |= SDC_EVENT_SDC_ERR;
    }

    if(value & RTO_INT)
    {
        printf("\n sdc resp timout");
        event |= SDC_EVENT_RESP_TIMEOUT;
    }

    if(value & SDIO_INT)
    {
        event |= SDC_EVENT_SDIO;
    }

    return event;
}
/*******************************************************************************
** Name: SDC_ReadData
** Input:(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize)
** Return: rk_size_t
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 17:45:43
*******************************************************************************/
rk_size_t SDC_ReadData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize)
{
    volatile uint32 *pFIFO = SDMMCFIFO(SDCPort);
    uint32 dwRealSize, dwReqSize;
    SDC_REG_T * pReg = SDMMC(SDCPort);
    uint32 * pbuf;
    uint32 temp,i;

    dwRealSize = 0;
    dwReqSize = dwSize / 4 + ((dwSize % 4)? 1 : 0);
    pbuf = (uint32 * )buf;

    pbuf--;
    while (dwReqSize > 1)
    {
        if (pReg->SDMMC_STATUS & FIFO_EMPTY)
        {
            goto end;
        }
        pbuf++;
        *pbuf = *pFIFO;
        dwRealSize++;
        dwReqSize--;
    }

    pbuf++;

    if(dwSize % 4)
    {
        temp = *pFIFO;
        buf = (uint8 *)pbuf;

        for(i = 0; i < dwSize % 4; i++)
        {
            buf[i] = (uint8)temp;
            temp >>= 8;
        }

    }
    else
    {
        *pbuf = *pFIFO;
        dwRealSize++;
    }

    dwReqSize--;

end:

    if(dwReqSize)
    {
        return (dwRealSize * 4);
    }
    else
    {
        return dwSize;
    }


}

/*******************************************************************************
** Name: SDC_WriteData
** Input:uint8 * buf, uint32 dwSize
** Return: rk_size_t
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 14:38:38
*******************************************************************************/
rk_size_t SDC_WriteData(SDMMC_PORT SDCPort, uint8 * buf, uint32 dwSize)
{
    volatile uint32 *pFIFO = SDMMCFIFO(SDCPort);
    uint32 dwRealSize, dwReqSize;
    SDC_REG_T * pReg = SDMMC(SDCPort);
    uint32 * pbuf;

    dwRealSize = 0;
    dwReqSize = dwSize / 4 + ((dwSize % 4)? 1 : 0);
    pbuf = (uint32 * )buf;

    pbuf--;
    while(dwReqSize > 0)
    {
        if (pReg->SDMMC_STATUS & FIFO_FULL)
        {
            break;
        }

        pbuf++;
        *pFIFO = *pbuf;
        dwRealSize++;
        dwReqSize--;
    }

    if(dwReqSize)
    {
        return (dwRealSize * 4);
    }
    else
    {
        return dwSize;
    }
}

/*******************************************************************************
** Name: SDC_ResetFIFO
** Input:SDMMC_PORT SDCPort
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 14:20:29
*******************************************************************************/
rk_err_t SDC_ResetFIFO(SDMMC_PORT SDCPort)
{
    volatile uint32 value = 0;
    int32 timeOut = 0;
    SDC_REG_T * pReg = SDMMC(SDCPort);

    value = pReg->SDMMC_STATUS;
    if (!(value & FIFO_EMPTY))
    {
        pReg->SDMMC_CTRL |= FIFO_RESET;
        timeOut = 5000;
        while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET)) && (timeOut > 0))
        {
            timeOut--;
        }

        if (timeOut == 0)
            return RK_ERROR;
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: SDC_Init
** Input:SDMMC_PORT SDCPort
** Return: int32
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 10:58:22
*******************************************************************************/
rk_err_t SDC_Init(SDMMC_PORT SDCPort, uint32 FreqKHz)
{
    volatile uint32  value = 0;
    SDC_REG_T *      pReg = SDMMC(SDCPort);
    int32            timeOut = 0;

    pReg->SDMMC_CTRL = (FIFO_RESET | SDC_RESET | DMA_RESET);
    pReg->SDMMC_CTRL &=~ENABLE_DMA;
    pReg->SDMMC_CTRL |= ENABLE_INT;

    timeOut = 1000;
    while (((value = pReg->SDMMC_CTRL) & (FIFO_RESET | SDC_RESET | DMA_RESET)) && (timeOut > 0))
    {
        SDC_Delay(1);
        timeOut--;
    }

    if (timeOut == 0)
    {
        return RK_ERROR;
    }

    /* config FIFO */
    pReg->SDMMC_FIFOTH = (SD_MSIZE_16 | (RX_WMARK << RX_WMARK_SHIFT) | (TX_WMARK << TX_WMARK_SHIFT));
    pReg->SDMMC_CTYPE = BUS_1_BIT;
    pReg->SDMMC_CLKDIV = 0;//1;
    pReg->SDMMC_CLKSRC = CLK_DIV_SRC_0;
    /* config debounce */
    pReg->SDMMC_DEBNCE = (DEBOUNCE_TIME*FreqKHz)&0xFFFFFF;
    pReg->SDMMC_TMOUT = 0xFFFFFF40;
    /* config interrupt */
    pReg->SDMMC_RINISTS = 0xFFFFFFFF;
    value = 0;

    #ifdef _WIFI_OB
    value |= (RXDR_INT | TXDR_INT | DTO_INT | CD_INT);
    #else
    value |= (RXDR_INT | TXDR_INT | DTO_INT | CD_INT| SDIO_INT);
    #endif

    value |= (SBE_INT | EBE_INT | DRTO_INT | RTO_INT | DCRC_INT | RCRC_INT | RE_INT);

    pReg->SDMMC_INTMASK = value;

    pReg->SDMMC_CTRL = ENABLE_INT;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SDC_UpdateFreq
** Input:SDMMC_PORT SDCPort, uint32 FreqKHz
** Return: init32
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 10:13:36
*******************************************************************************/
rk_err_t SDC_UpdateFreq(SDMMC_PORT SDCPort, uint32 FreqKHz)
{
    uint32          SrcClkDiv, SrcClkSel = 0;
    uint32          ClkInDiv;
    uint32          SrcFreq;
    uint32          ClkInFreq;
    int32           ret = RK_SUCCESS;
    SDC_REG_T *       pReg = SDMMC(SDCPort);

    SrcFreq = 24000;
    SrcClkSel = 1;

    ret = SDC_SetClock(SDCPort, FALSE);
    if (ret != RK_SUCCESS)
    {
        return ret;
    }

    ClkInDiv = SrcFreq / FreqKHz;
    if (ClkInDiv > 510)
    {
        return RK_ERROR;
    }

    if (((ClkInDiv & 0x1) == 1) && (ClkInDiv != 1))
    {
        ClkInDiv++;  //除了1分频，保证是偶数倍
    }

    pReg->SDMMC_CLKDIV = (ClkInDiv >> 1);


    if (RK_SUCCESS != SDC_StartCmd(SDCPort, (START_CMD | UPDATE_CLOCK | WAIT_PREV)))
    {
        return RK_ERROR;
    }

    return SDC_SetClock(SDCPort, TRUE);

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SDC_StartCmd
** Input:SDMMC_PORT SDCPort, uint32 cmd
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 13:42:23
*******************************************************************************/
rk_err_t SDC_StartCmd(SDMMC_PORT SDCPort, uint32 cmd)
{
    int32 timeOut = 50000;
    SDC_REG_T *      pReg = SDMMC(SDCPort);

    pReg->SDMMC_CMD = cmd;
    while ((pReg->SDMMC_CMD & START_CMD) && (timeOut > 0))
    {
        timeOut--;
    }

    if(timeOut == 0)
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SDC_SetClock
** Input:SDMMC_PORT SDCPort, bool enable
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 13:39:07
*******************************************************************************/
rk_err_t SDC_SetClock(SDMMC_PORT SDCPort, bool enable)
{
    volatile uint32 value;
    SDC_REG_T *      pReg = SDMMC(SDCPort);

    value = (TRUE == enable)? (CCLK_ENABLE|CCLK_LOW_POWER): 0;
    pReg->SDMMC_CLKENA = value;

    if (RK_SUCCESS != SDC_StartCmd(SDCPort, (START_CMD | UPDATE_CLOCK | WAIT_PREV)))
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SDA_Delay
** Input:uint32 us
** Return: void
** Owner:Aaron.sun
** Date: 2014.2.20
** Time: 11:24:01
*******************************************************************************/
void SDC_Delay(uint32 us)
{
    DelayUs(us);
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SDC_CheckDataBusy
** Input:SDMMC_PORT SDCPort
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.6
** Time: 13:57:19
*******************************************************************************/
rk_err_t SDC_CheckDataBusy(SDMMC_PORT SDCPort)
{
    SDC_REG_T * pReg = SDMMC(SDCPort);

    if(pReg->SDMMC_STATUS & DATA_BUSY)
    {
        return RK_ERROR;
    }
    else
    {
        return RK_SUCCESS;
    }
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
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
/*******************************************************************************
** Name: SDC_SetBusWidth
** Input:SDMMC_PORT SDCPort, BUS_WIDTH width
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.2.28
** Time: 8:53:44
*******************************************************************************/
rk_err_t SDC_SetBusWidth(SDMMC_PORT SDCPort, BUS_WIDTH width)
{
    uint32       value = 0;
    SDC_REG_T * pReg = SDMMC(SDCPort);

    switch (width)
    {
        case BUS_WIDTH_1_BIT:
            value = BUS_1_BIT;
            break;

        case BUS_WIDTH_4_BIT:
            value = BUS_4_BIT;
            break;

        case BUS_WIDTH_8_BIT:    //现在IC不支持8bit
            value = BUS_8_BIT;
            break;

        default:
            return RK_PARA_ERR;
    }

    pReg->SDMMC_CTYPE = value;

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: SDC_SetPower
** Input:bool PowerOn
** Return: void
** Owner:Aaron.sun
** Date: 2014.2.24
** Time: 14:20:42
*******************************************************************************/
void SDC_SetPower(SDMMC_PORT SDCPort, bool enable)
{
     SDC_REG_T * pReg = SDMMC(SDCPort);

    if (enable)
    {
        pReg->SDMMC_PWREN = 1;   // power enable
    }
    else
    {

        pReg->SDMMC_PWREN = 0;   // power disable
    }
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

