/* Copyright (C) 2011 ROCK-CHIP FUZHOU. All Rights Reserved. */
/*
File: SFComm.h
Desc:

Author:
Date: 14-02-25
Notes:

$Log: $
 *
 *
*/

#ifndef _SPIFLASH_H
#define _SPIFLASH_H

/*-------------------------------- Includes ----------------------------------*/


/*------------------------------ Global Defines ------------------------------*/
#define _ATTR_FLASH_CODE_           __attribute__((section("cpu_nanod_lib_sfc_common")))
#define _ATTR_FLASH_INIT_CODE_      __attribute__((section("cpu_nanod_lib_sfc_init")))
#define _CPU_NANOD_LIB_SFC_SHELL_   __attribute__((section("cpu_nanod_lib_sfc_shell")))

/*Manufactory ID*/
#define MID_WINBOND             0xEF
#define MID_GIGADEV             0xC8
#define MID_MICRON              0x2C
#define MID_MACRONIX            0xC2
#define MID_SPANSION            0x01
#define MID_EON                 0x1C
#define MID_ST                  0x20

/*Command Set*/
#define CMD_READ_JEDECID        (0x9F)
#define CMD_READ_DATA           (0x03)
#define CMD_READ_STATUS         (0x05)
#define CMD_WRITE_STATUS        (0x01)
#define CMD_PAGE_PROG           (0x02)
#define CMD_SECTOR_ERASE        (0x20)
#define CMD_BLK64K_ERASE        (0xD8)
#define CMD_BLK32K_ERASE        (0x52)
#define CMD_CHIP_ERASE          (0xC7)
#define CMD_WRITE_EN            (0x06)
#define CMD_WRITE_DIS           (0x04)

#define CMD_PAGE_READ           (0x13)
#define CMD_GET_FEATURE         (0x0F)
#define CMD_SET_FEATURE         (0x1F)

#define CMD_PROG_LOAD           (0x02)
#define CMD_PROG_EXEC           (0x10)
#define CMD_BLOCK_ERASE         (0xD8)

#define CMD_READ_DATA_X2        (0x3B)
#define CMD_READ_DATA_X4        (0x6B)
#define CMD_PROG_LOAD_X4        (0x32)

#define CMD_READ_STATUS2        (0x35)
#define CMD_READ_STATUS3        (0x15)

#define CMD_WRITE_STATUS2       (0x31)
#define CMD_WRITE_STATUS3       (0x11)

#define CMD_FAST_READ_X1        (0x0B)  // X1 cmd, X1 addr, X1 data
#define CMD_FAST_READ_X2        (0x3B)  // X1 cmd, X1 addr, X2 data
#define CMD_FAST_READ_X4        (0x6B)  // X1 cmd, X1 addr, X4 data SUPPORT GD MARCONIX WINBOND
#define CMD_FAST_READ_A4        (0xEB)  // X1 cmd, X4 addr, X4 data SUPPORT EON GD MARCONIX WINBOND

#define CMD_PAGE_PROG_X4        (0x32)  // X1 cmd, X1 addr, X4 data, SUPPORT GD WINBOND
#define CMD_PAGE_PROG_A4        (0x38)  // X1 cmd, X4 addr, X4 data, SUPPORT MARCONIX

#define CMD_RESET_NAND          (0xFF)

/*------------------------------ Global Typedefs -----------------------------*/

typedef int32 (*SFLASH_READ)(uint32 SecAdrr, uint32 nSec, void *pBuf);
typedef int32 (*SFLASH_WRITE)(uint32 SecAdrr, uint32 nSec, void *pBuf);
typedef int32 (*SFLASH_ERASE)(uint32 SecAdrr);

typedef  struct tagSFLASH_INFO
{
    uint32 capacity;
    uint8  Manufacturer;
    uint8  MemType;
    uint16 PageSize;
    uint32 BlockSize;

}SFLASH_INFO, *pSFLASH_INFO;


typedef struct tagSFLASH_DRIVER
{
    SFLASH_READ read;
    SFLASH_WRITE write;
    SFLASH_ERASE erase;
    void *pDev;
}SFLASH_DRIVER, *pSFLASH_DRIVER;


/*----------------------------- External Variables ---------------------------*/

/*------------------------- Global Function Prototypes -----------------------*/

extern uint32 SFlashInit(void);

extern uint32 SFlashDeInit(void);

extern uint32 SFlashReadIDB(uint32 sec, uint32 nSec, uint8 *pData);

extern uint32 SFlashWriteIDB(uint32 sec, uint32 nSec, uint8 *pData);

extern uint32 MDSFlashRead(uint8 lun, uint32 sec, uint32 nSec, void *pData);

extern uint32 MDSFlashWrite(uint8 lun, uint32 sec, uint32 nSec, void *pData);



extern uint32 MDSFlashErase(uint32 ChipSel, uint32 BlkStart, uint32 BlkNum, uint32 mod);

#endif
