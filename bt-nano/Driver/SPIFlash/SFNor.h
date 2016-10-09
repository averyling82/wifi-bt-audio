/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SFNor.h
* Owner: wangping
* Date: 2015.6.23
* Time: 14:32:00
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     14:32:00   1.0
********************************************************************************************
*/


#ifndef __DRIVER_SPIFLASH_SFNOR_H__
#define __DRIVER_SPIFLASH_SFNOR_H__

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

typedef enum
{
    ERASE_SECTOR = 0,
    ERASE_BLOCK64K,
    //ERASE_BLOCK32K,
    ERASE_CHIP
} NOR_ERASE_TYPE;


typedef enum
{
    IO_MODE_SPI = 0,
    IO_MODE_QPI
} SNOR_IO_MODE;

typedef enum
{
    READ_MODE_NOMAL = 0,
    READ_MODE_FAST
} SNOR_READ_MODE;

typedef enum
{
    ADDR_MODE_3BYTE = 0,
    ADDR_MODE_4BYTE
} SNOR_ADDR_MODE;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern int32 SNOR_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv);

extern uint32 SNOR_GetCapacity(void);

extern int32 SNOR_ReadID(uint8* data);

extern int32 SNOR_Read(uint32 sec, uint32 nSec, void *pData);

extern int32 SNOR_Write(uint32 sec, uint32 nSec, void *pData);

extern int32 SNOR_Prog(uint32 addr, void *pData, uint32 size);

extern int32 SNOR_Erase(uint32 addr, NOR_ERASE_TYPE EraseType);

extern int32 SNOR_EraseBlk(uint32 addr);


#endif
