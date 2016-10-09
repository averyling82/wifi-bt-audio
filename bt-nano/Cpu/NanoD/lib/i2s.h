/*
********************************************************************************
*                   Copyright (c) 2008,Yangwenjie
*                         All rights reserved.
*
* File Name£º   i2s.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2009-1-14          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _I2S_H_
#define _I2S_H_

#undef  EXT
#ifdef _IN_I2S_
#define EXT
#else
#define EXT extern
#endif


/*
--------------------------------------------------------------------------------

                        Struct Define

--------------------------------------------------------------------------------
*/

typedef enum _EI2S_DEV
{
    I2S_DEV0,   //0
    I2S_DEV1,

    I2S_DEV_MAX,
}eI2s_DEV;

typedef enum
{
    I2S_INT_TX = 0x00000001,
    I2S_INT_RX = 0x00000002,
    I2S_INT_RTX = 0x00000004,
    I2S_INT_FIFO_FULL = 0x00000008,
    I2S_INT_FIFO_EMPTY = 0x00000010,
}I2S_INT_t;

typedef enum
{
    I2S_START_NULL = 0,
    I2S_START_PIO_RX,
    I2S_START_DMA_RX,
    I2S_START_PIO_TX,
    I2S_START_DMA_TX,
    I2S_START_PIO_RTX,
    I2S_START_DMA_RTX
}I2S_Start_t;

typedef enum
{
    I2S_NORMAL_MODE = 0,
    I2S_LEFT_MODE,
    I2S_RIGHT_MODE
}I2S_BUS_MODE_t;

typedef enum
{
    PCM_EARLY_MODE = 0,
    PCM_LATE_MODE
}PCM_BUS_MODE_t;

typedef enum
{
    I2S_BUS_MODE = 0,
    PCM_BUS_MODE
}I2S_PCM_SEL_t;


typedef enum
{
    I2S_SLAVE_MODE = 0,
    I2S_MASTER_MODE,
    I2S_MASTER_MODE_LOCK,
    I2S_MASTER_MODE_UNLOCK
}I2S_mode_t;
typedef enum I2S_FS
{
    I2S_FS_8000Hz   = 8000,
    I2S_FS_11025Hz  = 11025,
    I2S_FS_12KHz    = 12000,
    I2S_FS_16KHz    = 16000,
    I2S_FS_22050Hz  = 22050,
    I2S_FS_24KHz    = 24000,
    I2S_FS_32KHz    = 32000,
    I2S_FS_44100Hz  = 44100,
    I2S_FS_48KHz    = 48000,
    I2S_FS_96KHz    = 96000,
    I2S_FS_192KHz   = 192000,
    I2S_FSSTOP      = 192000
} I2sFS_en_t;
typedef enum
{
    I2S_FORMAT = 0,
    PCM_FORMAT  = 1,
}eI2sFormat_t;
typedef enum
{
    I2S_EXT = 0,
    I2S_IN  = 1,

}eI2sCs_t;
typedef enum
{
    // Vailid Data width = n+1 bit
    I2S_DATA_WIDTH16 = 0xF,
    I2S_DATA_WIDTH24  = 0x17,
}eI2sDATA_WIDTH_t;

typedef enum
{
    TX_interrupt_active = 1,
    RX_interrupt_active = 2,
    TX_interrupt_fifo_empty = 4,
    RX_interrupt_fifo_full = 8,
}eI2sINT_SR_Event;
/*
--------------------------------------------------------------------------------

                        Funtion Declaration

--------------------------------------------------------------------------------
*/


extern int32 I2SStart(eI2s_DEV I2S_DEV,I2S_Start_t TxOrRx);
extern void  I2SStop(eI2s_DEV I2S_DEV,I2S_Start_t TxOrRx);
extern void  I2SDeInit(eI2s_DEV I2S_DEV);
extern int32 I2SIntEnable(eI2s_DEV I2S_DEV,I2S_INT_t INT_TxRx);
extern int32 I2SIntDisable(eI2s_DEV I2S_DEV,I2S_INT_t INT_TxRx);
extern int32 I2SDMAEnable(eI2s_DEV I2S_DEV,I2S_Start_t INT_TxRx);
extern int32 I2SDMADisable(eI2s_DEV I2S_DEV,I2S_Start_t INT_TxRx);

extern uint32 I2sGetInt(eI2s_DEV I2S_DEV);
extern gI2S_t I2sGetCH(eI2s_DEV i2sChNumb);
extern uint32 I2sGetRxFIFOaddr(eI2s_DEV i2sChNumb);
extern uint32 I2sGetTxFIFOaddr(eI2s_DEV i2sChNumb);
extern uint32 I2SGetIntType(eI2s_DEV i2sChNumb);

extern rk_size_t I2S_PIO_Read(eI2s_DEV I2S_DEV,uint32* buffer,uint32 size);
extern rk_size_t I2S_PIO_Write(eI2s_DEV I2S_DEV,uint32* buffer,uint32 size);
extern void I2sCs(eI2sCs_t data);
extern int32  I2SInit(eI2s_DEV I2S_DEV,
              I2S_mode_t I2S_mode,
              eI2sCs_t I2S_CS,
              I2sFS_en_t I2S_FS,
              eI2sFormat_t BUS_FORMAT,
              eI2sDATA_WIDTH_t I2S_Data_width,
              I2S_BUS_MODE_t I2S_Bus_mode,
              eI2sFormat_t RX_BUS_FORMAT,
              eI2sDATA_WIDTH_t Rx_I2S_Data_width,
              I2S_BUS_MODE_t Rx_I2S_Bus_mode);

extern uint32 I2sSetDataW(eI2s_DEV I2S_DEV,
                        eI2sFormat_t BUS_FORMAT,
                        eI2sDATA_WIDTH_t I2S_Data_width,
                        I2S_BUS_MODE_t I2S_Bus_mode);

extern uint32 I2sSetRxDataW(eI2s_DEV I2S_DEV,
                        eI2sFormat_t BUS_FORMAT,
                        eI2sDATA_WIDTH_t I2S_Data_width,
                        I2S_BUS_MODE_t I2S_Bus_mode);

extern uint32 I2sSetSampleRate(eI2s_DEV I2S_DEV,I2sFS_en_t I2S_FS);

/*
********************************************************************************
*
*                         End of i2s.h
*
********************************************************************************
*/
#endif
