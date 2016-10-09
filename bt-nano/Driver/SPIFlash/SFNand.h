/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SFNand.h
* Owner: wangping
* Date: 2015.6.23
* Time: 14:34:29
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     14:34:29   1.0
********************************************************************************************
*/


#ifndef __DRIVER_SPIFLASH_SFNAND_H__
#define __DRIVER_SPIFLASH_SFNAND_H__

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
extern int32 SNAND_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv);

extern int32 SNAND_SetDLines(SFC_DATA_LINES lines);

extern int32 SNAND_ReadID(uint8* data);

extern int32 SNAND_Read(uint32 sec, uint32 nSec, void *pData);

extern int32 SNAND_Write(uint32 sec, uint32 nSec, void *pData);

extern int32 SNAND_ReadPage(uint32 RowAddr, uint32 ColAddr, void *pData, uint32 size);

extern int32 SNAND_ProgPage(uint32 RowAddr, void *pData, uint32 size);

extern int32 SNAND_Erase(uint32 RowAddr);

extern int32 SNAND_GetFeature(uint32 addr, uint32 *status);

extern int32 SNAND_SetFeature(uint32 addr, uint32 feature);

extern uint32 SNAND_GetCapacity(void);


#endif