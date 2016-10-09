/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SFNand.c
* Owner: wangping
* Date: 2015.6.23
* Time: 13:57:51
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     13:57:51   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"

#include "SFC.h"
#include "SPIFlash.h"
#include "SFNor.h"
//#include "sfc_common.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOR_PAGE_SIZE            256
#define NOR_BLOCK_SIZE          (64*1024)
#define NOR_SECS_BLK            (NOR_BLOCK_SIZE/512)
#define NOR_SECS_PAGE           4


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef int32 (*SNOR_WRITE_STATUS)(uint32 RegIndex, uint8 status);

typedef  struct tagSFNOR_DEV
{
    uint32          capacity;
    uint8           Manufacturer;
    uint8           MemType;
    uint16          PageSize;
    uint32          BlockSize;

    uint8           ReadCmd;
    uint8           ProgCmd;

    SNOR_READ_MODE  ReadMode;
    SNOR_ADDR_MODE  AddrMode;
    SNOR_IO_MODE    IOMode;

    SFC_DATA_LINES ReadLines;
    SFC_DATA_LINES ProgLines;

    SNOR_WRITE_STATUS WriteStatus;
} SFNOR_DEV, *pSFNOR_DEV;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/

static SFNOR_DEV SFNorDev;

const SFLASH_DRIVER SFNorDrv =
{
    SNOR_Read,
    SNOR_Write,
    SNOR_EraseBlk,
    &SFNorDev,
};


const uint8 SFNorDevCode[] =
{
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16,
    0x17,
    0x18,
    0x19
};

const uint32 SFNorCapacity[] =
{
    0x20000,        //128k-byte
    0x40000,        //256k-byte
    0x80000,        //512k-byte
    0x100000,       // 1M-byte
    0x200000,       // 2M-byte
    0x400000,       // 4M-byte
    0x800000,       // 8M-byte
    0x1000000,      //16M-byte
    0x2000000       // 32M-byte
};


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
static int32 SNOR_WaitBusy(int32 timeout);
extern int32 SFC_Request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: SNOR_WriteEn
** Input:void
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_WriteEn(void)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_WRITE_EN;

    ret = SFC_Request(sfcmd.d32, 0, 0, NULL);

    return ret;
}

/*******************************************************************************
** Name: SNOR_WriteEn
** Input:uint32 RegIndex, uint8 *status
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_ReadStatus(uint32 RegIndex, uint8 *status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           ReadStatCmd[] = {CMD_READ_STATUS, CMD_READ_STATUS2, CMD_READ_STATUS3};

    sfcmd.d32 = 0;
    sfcmd.b.cmd = ReadStatCmd[RegIndex];
    sfcmd.b.datasize = 1;

    ret = SFC_Request(sfcmd.d32, 0, 0, status);

    return ret;
}

/*******************************************************************************
** Name: SNOR_WriteStatus2
** Input:uint32 RegIndex, uint8 status
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_WriteStatus2(uint32 RegIndex, uint8 status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           status2[2];
    uint8           ReadIndex;

    status2[RegIndex] = status;
    ReadIndex = (RegIndex==0)? 1:0;
    ret = SNOR_ReadStatus(ReadIndex, &status2[ReadIndex]);
    if (ret != SFC_OK)
        return ret;

    SNOR_WriteEn();

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_WRITE_STATUS;
    sfcmd.b.datasize = 2;
    sfcmd.b.rw = SFC_WRITE;

    ret = SFC_Request(sfcmd.d32, 0, 0, &status2[0]);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);    // 10ms

    return ret;
}

/*******************************************************************************
** Name: SNOR_WriteStatus
** Input:uint32 RegIndex, uint8 status
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_WriteStatus(uint32 RegIndex, uint8 status)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           WriteStatCmd[] = {CMD_WRITE_STATUS, CMD_WRITE_STATUS2, CMD_WRITE_STATUS3};

    SNOR_WriteEn();

    sfcmd.d32 = 0;
    sfcmd.b.cmd = WriteStatCmd[RegIndex];
    sfcmd.b.datasize = 1;
    sfcmd.b.rw = SFC_WRITE;

    ret = SFC_Request(sfcmd.d32, 0, 0, &status);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);    // 10ms

    return ret;
}


/*******************************************************************************
** Name: SNOR_WaitBusy
** Input:int32 timeout
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_WaitBusy(int32 timeout)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint32           i,status;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_STATUS;
    sfcmd.b.datasize = 1;

    for (i=0; i<timeout; i++)
    {
        ret = SFC_Request(sfcmd.d32, 0, 0, &status);
        if (ret != SFC_OK)
        {
            return ret;
        }

        /*check the value of the Write in Progress (WIP) bit.The Write in Progress
        (WIP) bit is 1 during the self-timed Block Erase cycle, and is 0 when it is completed*/
        if ((status & 0x01) == 0)
            return SFC_OK;

        SFC_Delay(1);
    }

    printf("\n spiflash busy timeout");

    return SFC_BUSY_TIMEOUT;

}

/*******************************************************************************
** Name: SNOR_Erase
** Input:uint32 addr, NOR_ERASE_TYPE EraseType
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_Erase(uint32 addr, NOR_ERASE_TYPE EraseType)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    uint8           EraseCmd[] = {CMD_SECTOR_ERASE, CMD_BLK64K_ERASE, CMD_CHIP_ERASE};
    int32           timeout[] = {10, 100, 20000};   //ms

    if (EraseType > ERASE_CHIP)
        return SFC_PARAM_ERR;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = EraseCmd[EraseType];
    sfcmd.b.addrbits = (EraseType != ERASE_CHIP)? SFC_ADDR_24BITS : SFC_ADDR_0BITS;

    SNOR_WriteEn();
    ret = SFC_Request(sfcmd.d32, 0, addr, NULL);
    if (ret != SFC_OK)
        return ret;
    ret = SNOR_WaitBusy(timeout[EraseType]*1000);

    return ret;
}

/*******************************************************************************
** Name: SNOR_EraseBlk
** Input:uint32 addr
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_EraseBlk(uint32 addr)
{
    return SNOR_Erase(addr, ERASE_BLOCK64K);
}

/*******************************************************************************
** Name: SNOR_ProgPage
** Input:uint32 addr, void *pData, uint32 size
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_ProgPage(uint32 addr, void *pData, uint32 size)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    pSFNOR_DEV      pDev = &SFNorDev;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = pDev->ProgCmd;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;
    sfcmd.b.datasize = size;
    sfcmd.b.rw = SFC_WRITE;

    sfctrl.d32 = 0;
    sfctrl.b.datalines = pDev->ProgLines;
    sfctrl.b.enbledma = 1;
    if (pDev->ProgCmd == CMD_PAGE_PROG_A4)
    {
        sfctrl.b.addrlines = SFC_4BITS_LINE;;
    }

    SNOR_WriteEn();

    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, pData);
    if (ret != SFC_OK)
        return ret;

    ret = SNOR_WaitBusy(10000);                 //10ms

    return ret;
}

/*******************************************************************************
** Name: SNOR_Prog
** Input:uint32 addr, void *pData, uint32 size
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_Prog(uint32 addr, void *pData, uint32 size)
{
    int32           ret = SFC_OK;
    uint32          PageSize, len;
    uint8           *pBuf =  (uint8*)pData;

    PageSize = NOR_PAGE_SIZE;
    while (size)
    {
        len = MIN(PageSize, size);
        ret = SNOR_ProgPage(addr, pBuf, len);
        if (ret != SFC_OK)
            return ret;

        size -= len;
        addr += len;
        pBuf += len;
    }

    return ret;
}

/*******************************************************************************
** Name: SNOR_EnableQE
** Input:void
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_EnableQE(void)
{
    int32 ret = SFC_OK;
    uint8 status;
    pSFNOR_DEV pDev = &SFNorDev;

    if (pDev->Manufacturer == MID_GIGADEV
        || pDev->Manufacturer == MID_WINBOND)
    {
        ret = SNOR_ReadStatus(1, &status);
        if (ret != SFC_OK)
            return ret;

        if (status & 0x2)   //is QE bit set
            return SFC_OK;

        status |= 0x2;

        return pDev->WriteStatus(1, status);
    }

    return ret;
}

/*******************************************************************************
** Name: SNOR_SetIOMode
** Input:SNOR_IO_MODE mode
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_SetIOMode(SNOR_IO_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->IOMode = mode;

    return SFC_OK;

}

/*******************************************************************************
** Name: SNOR_SetReadMode
** Input:SNOR_READ_MODE mode
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_SetReadMode(SNOR_READ_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->ReadMode = mode;

    return SFC_OK;
}

/*******************************************************************************
** Name: SNOR_SetAddrMode
** Input:SNOR_ADDR_MODE mode
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_SetAddrMode(SNOR_ADDR_MODE mode)
{
    pSFNOR_DEV pDev = &SFNorDev;

    pDev->AddrMode = mode;

    return SFC_OK;

}

/*******************************************************************************
** Name: SNOR_SetDLines
** Input:SFC_DATA_LINES lines
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SNOR_SetDLines(SFC_DATA_LINES lines)
{
    int32           ret;
    pSFNOR_DEV      pDev = &SFNorDev;
    uint8           ReadCmd[] = {CMD_FAST_READ_X1, CMD_FAST_READ_X2, CMD_FAST_READ_X4/*CMD_FAST_READ_A4*/};

    if (pDev->ReadMode != READ_MODE_FAST) //多线模式需使用Fast read mode
        return SFC_ERROR;

    if (lines == DATA_LINES_X4)
    {
        ret = SNOR_EnableQE();
        if (ret != SFC_OK)
            return ret;
    }

    pDev->ReadLines = lines;
    pDev->ReadCmd = ReadCmd[lines];

    if (pDev->Manufacturer == MID_GIGADEV || pDev->Manufacturer == MID_WINBOND
        || pDev->Manufacturer == MID_MACRONIX)
    {

        pDev->ProgLines = (lines != DATA_LINES_X2)? lines : DATA_LINES_X1;  //不支持两线编程
        if (lines == DATA_LINES_X1)
            pDev->ProgCmd = CMD_PAGE_PROG;
        else
        {
            if (pDev->Manufacturer == MID_GIGADEV || pDev->Manufacturer == MID_WINBOND)
                pDev->ProgCmd = CMD_PAGE_PROG_X4;
            else
                pDev->ProgCmd = CMD_PAGE_PROG_A4;   //MID_MACRONIX
        }
    }

    return SFC_OK;
}

/*******************************************************************************
** Name: SNOR_ReadData
** Input:uint32 addr, void *pData, uint32 size
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_ReadData(uint32 addr, void *pData, uint32 size)
{
    int32           ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;
    pSFNOR_DEV      pDev = &SFNorDev;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = pDev->ReadCmd;
    sfcmd.b.datasize = size;
    sfcmd.b.addrbits = SFC_ADDR_24BITS;

    sfctrl.d32 = 0;
    sfctrl.b.datalines = pDev->ReadLines;
    sfctrl.b.enbledma = 1;

    if (pDev->ReadCmd == CMD_FAST_READ_X1 || pDev->ReadCmd == CMD_FAST_READ_X4
        || pDev->ReadCmd == CMD_FAST_READ_X2)
    {
        sfcmd.b.dummybits = 8;
    }
    else if (pDev->ReadCmd == CMD_FAST_READ_A4)
    {
        sfcmd.b.addrbits = SFC_ADDR_32BITS;
        addr = (addr<<8) | 0xFF;                //Set M[7:0] = 0xFF
        sfcmd.b.dummybits = 4;
        sfctrl.b.addrlines = SFC_4BITS_LINE;
    }
    else
    {
        ;
    }

    ret = SFC_Request(sfcmd.d32, sfctrl.d32, addr, pData);

    return ret;
}

/*******************************************************************************
** Name: SNOR_Read
** Input:uint32 addr, void *pData, uint32 size
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_Read(uint32 sec, uint32 nSec, void *pData)
{
    int32       ret = SFC_OK;
    uint32      addr, size, len;
    pSFNOR_DEV  pDev = &SFNorDev;
    uint8       *pBuf =  (uint8*)pData;

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;

    addr = sec<<9;
    size = nSec<<9;
    while(size)
    {
        len = MIN(size, SFC_MAX_IOSIZE);
        ret = SNOR_ReadData(addr, pBuf, len);
        if (ret != SFC_OK)
            break;

        size -= len;
        addr += len;
        pBuf += len;
    }

    return ret;
}

/*******************************************************************************
** Name: SNOR_Write
** Input:uint32 addr, void *pData, uint32 size
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_Write(uint32 sec, uint32 nSec, void *pData)
{
    int32       ret = SFC_OK;
    uint32      addr, size, len ,BlockSize;
    pSFNOR_DEV  pDev = &SFNorDev;
    uint8       *pBuf =  (uint8*)pData;
    uint8 flag;
    uint32 i, sec_t, sec_s, cnt, sec_t1, sec_s1, cnt1;
    uint8 buf[512];

    if ((sec+nSec) > pDev->capacity)
        return SFC_PARAM_ERR;

    addr = sec<<9;
    size = nSec<<9;
    BlockSize = pDev->BlockSize*512;        //BlockSize is 64K Bytes

    //rk_printf("sec = %d, cnt = %d", sec, nSec);

    flag = 0;

    if (addr & (BlockSize-1))
    {
        cnt = (addr & (BlockSize-1)) / 512;
        sec_s = (sec / pDev->BlockSize) * pDev->BlockSize;
        sec_t = (pDev->capacity - pDev->BlockSize);

        //rk_printf("1sec_s = %d, sec_t = %d, cnt = %d", sec_s, sec_t, cnt);

        ret = SNOR_Erase(sec_t * 512, ERASE_BLOCK64K);
        if (ret != SFC_OK)
        {
            return ret;
        }

        for(i = 0; i < cnt; i++)
        {
            SNOR_Read(sec_s + i, 1, buf);
            SNOR_Prog((sec_t + i) * 512, buf, 512);
        }

        if(nSec < (pDev->BlockSize - cnt))
        {
            cnt1 = pDev->BlockSize - cnt - nSec;
            sec_s1 = sec_s + cnt + nSec;
            sec_t1 = sec_t + cnt;

            for(i = 0; i < cnt1; i++)
            {
                SNOR_Read(sec_s1 + i, 1, buf);
                SNOR_Prog((sec_t1 + i) * 512, buf, 512);
            }
            flag = 1;
        }

        ret = SNOR_Erase(sec_s * 512, ERASE_BLOCK64K);
        if (ret != SFC_OK)
        {
            return ret;
        }

        for(i = 0; i < cnt; i++)
        {
            SNOR_Read(sec_t + i, 1, buf);
            SNOR_Prog((sec_s + i) * 512, buf, 512);
        }

        len = MIN(BlockSize - cnt * 512, size);
        ret = SNOR_Prog(addr, pBuf, len);
        if (ret != SFC_OK)
        {
            return ret;
        }
        size -= len;
        addr += len;
        pBuf += len;

        if(flag)
        {
            for(i = 0; i < cnt1; i++)
            {
                SNOR_Read(sec_t1 + i, 1, buf);
                SNOR_Prog((sec_s1 + i) * 512, buf, 512);
            }

        }

    }



    flag = 0;

    while (size)
    {
        if (!(addr & (BlockSize-1)))
        {
            if(size < BlockSize)
            {
                cnt = size / 512;
                sec_s = (addr / 512) + cnt;
                sec_t = (pDev->capacity - pDev->BlockSize);

                ret = SNOR_Erase(sec_t * 512, ERASE_BLOCK64K);
                if (ret != SFC_OK)
                {
                    return ret;
                }

                cnt = pDev->BlockSize - cnt;

                //rk_printf("2sec_s = %d, sec_t = %d, cnt = %d", sec_s, sec_t, cnt);

                for(i = 0; i < cnt; i++)
                {
                    SNOR_Read(sec_s + i, 1, buf);
                    SNOR_Prog((sec_t + i) * 512, buf, 512);
                }

                flag = 1;
            }

            ret = SNOR_Erase(addr, ERASE_BLOCK64K);
            if (ret != SFC_OK)
            {
                return ret;
            }
        }

        len = MIN(BlockSize, size);

        ret = SNOR_Prog(addr, pBuf, len);
        if (ret != SFC_OK)
        {
            return ret;
        }
        size -= len;
        addr += len;
        pBuf += len;
    }

    if(flag)
    {
        for(i = 0; i < cnt; i++)
        {
            SNOR_Read(sec_t + i, 1, buf);
            SNOR_Prog((sec_s + i) * 512, buf, 512);
        }
    }

    return ret;
}

/*******************************************************************************
** Name: SNOR_ReadID
** Input:uint8* data
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SNOR_ReadID(uint8* data)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;

    ret = SFC_Request(sfcmd.d32, 0, 0, data);

    return ret;
}

/*******************************************************************************
** Name: SNOR_GetCapacity
** Input:void
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SNOR_GetCapacity(void)
{
    pSFNOR_DEV pDev = &SFNorDev;

    return pDev->capacity - pDev->BlockSize;
}

/*******************************************************************************
** Name: SNOR_Init
** Input:uint8* pFlashID, SFLASH_DRIVER **pDrv
** Return: rk_err_t
** Owner:chenfen
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
int32 SNOR_Init(uint8* pFlashID, SFLASH_DRIVER **pDrv)
{
    int32 i;
    pSFNOR_DEV pDev = &SFNorDev;
    uint8 IDByte[3];
    uint8 *data;

    memset(pDev, 0, sizeof(SFNOR_DEV));
    //SFC_Init();

    if (pFlashID)
    {
        data = pFlashID;
    }
    else
    {
        data = IDByte;
        SNOR_ReadID(data);
    }

    if ((0xFF==data[0] && 0xFF==data[1]) || (0x00==data[0] && 0x00==data[1]))
    {
        return SFC_ERROR;
    }

    for(i=0; i<sizeof(SFNorDevCode); i++)
    {
        if (data[2] == SFNorDevCode[i])
        {
            pDev->capacity = SFNorCapacity[i]>>9;
            break;
        }
    }

    if (i >= sizeof(SFNorDevCode))
        return SFC_ERROR;

    pDev->Manufacturer = data[0];
    pDev->MemType = data[1];
    pDev->BlockSize = NOR_SECS_BLK;
    pDev->PageSize = NOR_SECS_PAGE;

    pDev->ReadCmd = CMD_READ_DATA;
    pDev->ProgCmd = CMD_PAGE_PROG;

    pDev->WriteStatus = SNOR_WriteStatus2;      //不同的器件, 写状态寄存器的方式可能不一样

    if (pDrv)
        *pDrv = (SFLASH_DRIVER *)&SFNorDrv;

    return SFC_OK;
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
