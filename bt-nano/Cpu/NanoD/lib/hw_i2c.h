/*
********************************************************************************
*                   Copyright (c) 2008,yangwenjie
*                         All rights reserved.
*
* File Name£º   Hw_I2c.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             yangwenjie      2008-9-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _HW_I2C_H_
#define _HW_I2C_H_

#undef  EXT
#ifdef _IN_I2C_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                         I2c memap register define
*
*-------------------------------------------------------------------------------
*/
typedef volatile struct I2CReg
{
    uint32 I2C_CON;         // 0x00 control register
    uint32 I2C_CLKDIV;      // 0x04 clock divisor register
    uint32 I2C_MRXADDR;     // 0x08 the slave address accessed for master receive mode
    uint32 I2C_MRXRADDR;    // 0x0C the slave register address accessed for master receive mode
    uint32 I2C_MTXCNT;      // 0x10 master transmit count
    uint32 I2C_MRXCNT;      // 0x14 master reeive count
    uint32 I2C_IEN;         // 0x18 interrupt enable register
    uint32 I2C_IPD;         // 0x1C interrupt pending register
    uint32 I2C_FCNT;        // 0x20 finished count

    uint32 reserved[(0x100-0x24)/4];
    uint32 I2C_TXDATA[8];   //0x100
    uint32 reserved2[(0x200-0x120)/4];
    uint32 I2C_RXDATA[8];   //0x200
}I2CReg_t, *pI2CReg_t;

/*
********************************************************************************
*
*                         End of hw_i2s.h
*
********************************************************************************
*/

#endif
