/*
********************************************************************************************
*
*        Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\SPIFlash\SpiFlashDev.c
* Owner: wangping
* Date: 2015.6.23
* Time: 10:29:54
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    wangping     2015.6.23     10:29:54   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_SPIFLASH_SPIFLASHDEV_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"
#include "SFC.h"
#include "SPIFlash.h"
#include "SFNand.c"
#include "SFNor.c"
#include "spiflashdev.h"
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define SPIFLASH_DEV_NUM 1

typedef  struct _SPIFLASH_DEVICE_CLASS
{
    DEVICE_CLASS stSpiFlashDevice;
    pSemaphore osSpiFlashOperSem;
    uint32 capacity;
    HDC hDma;

} SPIFLASH_DEVICE_CLASS;

typedef __packed struct _SFC_STRUCT
{
    uint total_size;    //the total data size need to send/read
    uint complete_size; //
    uint8 *payload;      //current data point
    pSemaphore rw_complete_sema;
} SFC_STRUCT, * P_SFC_STRUCT;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static SPIFLASH_DEVICE_CLASS * gpstSpiFlashDevInf[SPIFLASH_DEV_NUM] = {(SPIFLASH_DEVICE_CLASS *)NULL};

static SFLASH_DRIVER *pSFlashDrv;

SFC_STRUCT sfc_ctrl;
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
rk_err_t SpiFlashDevCheckHandler(HDC dev);
rk_err_t SpiFlashDevShellTest(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellDel(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellMc(HDC dev, uint8 * pstr);
rk_err_t SpiFlashDevShellPcb(HDC dev, uint8 * pstr);
void SpiFlashDevIntIsr0(void);
void SpiFlashDevIntIsr(uint32 DevID);
rk_err_t SpiFlashDevDeInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev);
rk_err_t SpiFlashDevInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev);
rk_err_t SpiFlashDevResume(HDC dev);
rk_err_t SpiFlashDevSuspend(HDC dev, uint32 Level);
uint32 SFlashRead(uint32 sec, uint32 nSec, void *pData);
uint32 SFlashWrite(uint32 sec, uint32 nSec, void *pData);
uint32 SFlashGetCapacity(void);
void SpiFlash_Test(HDC dev);
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t SpiFlashDev_Write(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SpiFlashDev Write...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

    rkos_semaphore_take(pstSpiFlashDev->osSpiFlashOperSem, MAX_DELAY);
   // printf("writeLBA = %x, len =%d,\n",LBA, len);
    FREQ_Disable();
    SFlashWrite(LBA, len, buffer);
    FREQ_Enable();
    rkos_semaphore_give(pstSpiFlashDev->osSpiFlashOperSem);
    return len;
}
/*******************************************************************************
** Name: SpiFlashDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t SpiFlashDev_Read(HDC dev, uint32 LBA, uint8* buffer, uint32 len)
{
    //SpiFlashDev Read...
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev =  (SPIFLASH_DEVICE_CLASS *)dev;
    if(pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }
    rkos_semaphore_take(pstSpiFlashDev->osSpiFlashOperSem, MAX_DELAY);
    FREQ_Disable();
    SFlashRead(LBA, len, buffer);
    FREQ_Enable();
    rkos_semaphore_give(pstSpiFlashDev->osSpiFlashOperSem);
    return len;
}

_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON API rk_err_t  SpiFlashDev_GetSize(HDC dev, uint32 * Size)
{
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev = (SPIFLASH_DEVICE_CLASS *)dev;
    if (pstSpiFlashDev == NULL)
    {
        return RK_ERROR;
    }

    *Size = pstSpiFlashDev->capacity;
    return RK_SUCCESS;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.18
** Time: 16:54:26
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN rk_err_t SpiFlashDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < SPIFLASH_DEV_NUM; i++)
    {
        if(gpstSpiFlashDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: sfc_int_enable
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
void sfc_int_enable(void)
{
    IntEnable(INT_ID_SFC);
}

/*******************************************************************************
** Name: sfc_int_disable
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
void sfc_int_disable(void)
{
    IntDisable(INT_ID_SFC);
}


/*******************************************************************************
** Name: SFC_Request
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
int32 SFC_Request(uint32 sfcmd, uint32 sfctrl, uint32 addr, void *data)
{
    SFCCMD_DATA     cmd;
    int32           ret = SFC_OK;
    cmd.d32 = sfcmd;
#if 1
    if (SFC_WRITE == cmd.b.rw)
    {
        sfc_set_intmask(~(TXEMPTY_INT | FINISH_INT | NSPIERR_INT | AHBERR_INT | TXOF_INT));
    }
    else if(SFC_READ == cmd.b.rw)
    {

        sfc_set_intmask(~(RXUF_INT | FINISH_INT | NSPIERR_INT | AHBERR_INT | RXFULL_INT));
    }
    else
    {
        sfc_set_intmask(0xff);
    }
#endif

    ret = _sfc_request(sfcmd, sfctrl, addr, data);

    if(ret != SFC_OK)
    {
        printf("\nsfc err");
        return ret;
    }

#if 1
    if(cmd.b.datasize)
    {
        sfc_ctrl.total_size = cmd.b.datasize;
        sfc_ctrl.complete_size = 0;
        sfc_ctrl.payload = data;
        if (SFC_WRITE == cmd.b.rw)
        {
            //sfc_ctrl.complete_size = sfc_write_fifo(uint8 *data , sfc_ctrl.total_size);
            sfc_int_enable();
            rkos_semaphore_take(sfc_ctrl.rw_complete_sema, MAX_DELAY);
            sfc_wait_busy();

        }
        else if(SFC_READ == cmd.b.rw)
        {
            sfc_int_enable();
            rkos_semaphore_take(sfc_ctrl.rw_complete_sema, MAX_DELAY);
            sfc_wait_busy();
        }
    }
#endif

    return ret;
}


/*******************************************************************************
** Name: SFlashRead
** Input:uint32 sec, uint32 nSec, void *pData
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashRead(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->read(sec, nSec, pData);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashWrite
** Input:uint32 sec, uint32 nSec, void *pData
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashWrite(uint32 sec, uint32 nSec, void *pData)
{

    if (pSFlashDrv)
        return pSFlashDrv->write(sec, nSec, pData);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashErase
** Input:uint32 SecAddr
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashErase(uint32 SecAddr)
{

    if (pSFlashDrv)
        return pSFlashDrv->erase(SecAddr);
    else
        return ERROR;
}

/*******************************************************************************
** Name: SFlashGetCapacity
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashGetCapacity(void)
{
    SFLASH_INFO *pInfo;

    if (!pSFlashDrv)
        return 0;

    pInfo = (SFLASH_INFO *)pSFlashDrv->pDev;
    return pInfo->capacity;
}

/*******************************************************************************
** Name: SFlashReadID
** Input:uint8* data, bool bNand
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
static int32 SFlashReadID(uint8* data, bool bNand)
{
    int32 ret;
    SFCCMD_DATA     sfcmd;
    SFCCTRL_DATA    sfctrl;

    sfcmd.d32 = 0;
    sfcmd.b.cmd = CMD_READ_JEDECID;
    sfcmd.b.datasize = 3;               //ID 字节数是2-3B, 不同厂商不一样

    sfctrl.d32 = 0;
    if (bNand)
    {
        sfcmd.b.addrbits = SFC_ADDR_XBITS;
        sfctrl.b.addrbits = 8;
    }
    ret = SFC_Request(sfcmd.d32, sfctrl.d32, 0, data);

    return ret;
}

/*******************************************************************************
** Name: SFlashInit
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
uint32 SFlashInit(void)
{
    int32 ret = ERROR;
    uint8 data[3];
    uint8 bSpiNand;
    uint32 i;

    pSFlashDrv = NULL;

    if (SFC_OK != SFlashReadID(data, FALSE))
        return ERROR;

    //DEBUG("SPI FLASH ID:%x %x %x\n", data[0], data[1], data[2]);
    //不使用data[0]判断, 是考虑兼容nand, 接nand时data[0]=0xFF(IO默认有上拉电阻)
    if ((0xFF==data[2] && 0xFF==data[1]) || (0x00==data[2] && 0x00==data[1]))
    {
        return ERROR;
    }

    //bSpiNand = 0;
    bSpiNand = (0xFF==data[0])? 1 : 0;

#if 1//def SPINAND_DRIVER
    if (bSpiNand)
    {
        ret = SNAND_Init(&data[1], &pSFlashDrv);
    }
    else
#endif
    {
        //#ifdef SPINOR_DRIVER
        ret = SNOR_Init(data, &pSFlashDrv);
        //#endif
    }

    if (ret != SFC_OK)
        return ret;

    return ret;
}

/*******************************************************************************
** Name: SpiFlashDevIntIsr0
** Input:void
** Return: void
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN void SpiFlashDevIntIsr0(void)
{
    //Call total int service...

    uint32 int_status = sfc_get_int_status();

    if(int_status & RXFULL_INT) //read fifo full
    {
        //printf("\nr = %d, %d",sfc_ctrl.total_size, sfc_ctrl.complete_size);

        if(sfc_ctrl.total_size != sfc_ctrl.complete_size)
        {
            sfc_ctrl.complete_size += sfc_read_fifo(sfc_ctrl.payload+sfc_ctrl.complete_size ,
                                                    sfc_ctrl.total_size- sfc_ctrl.complete_size);
        }

    }

    if(int_status & TXEMPTY_INT)//tx fifo empty
    {
        //printf("\nw = %d, %d",sfc_ctrl.total_size, sfc_ctrl.complete_size);

        if(sfc_ctrl.total_size != sfc_ctrl.complete_size)
        {
            sfc_ctrl.complete_size += sfc_write_fifo(sfc_ctrl.payload+sfc_ctrl.complete_size ,
                                      sfc_ctrl.total_size- sfc_ctrl.complete_size);

//            if(sfc_ctrl.total_size == sfc_ctrl.complete_size)
//            {
//                sfc_int_disable();
//                rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
//            }
        }
    }

    if(int_status & FINISH_INT)
    {
        sfc_int_disable();
        rkos_semaphore_give_fromisr(sfc_ctrl.rw_complete_sema);
    }

    if(int_status & (1<< 1))
    {
         printf("\n fifo err");
    }

    if(int_status & (1<< 2))
    {
         printf("\n fifo err");
    }




}

/*******************************************************************************
** Name: SpiFlashDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN rk_err_t SpiFlashDevResume(HDC dev)
{
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev = (SPIFLASH_DEVICE_CLASS *)dev;
    if(SpiFlashDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    SpiFlashDevHwInit();
    SpiFlashDevInit(pstSpiFlashDev);

    RKDev_Resume(pstSpiFlashDev->hDma);

    pstSpiFlashDev->stSpiFlashDevice.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiFlashDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_COMMON_
COMMON FUN rk_err_t SpiFlashDevSuspend(HDC dev, uint32 Level)
{
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev = (SPIFLASH_DEVICE_CLASS *)dev;
    if(SpiFlashDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstSpiFlashDev->stSpiFlashDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstSpiFlashDev->stSpiFlashDevice.State = DEV_SATE_IDLE2;
    }

    SpiFlashDevDeInit(pstSpiFlashDev);
    SpiFlashDevHwDeInit();

    RKDev_Suspend(pstSpiFlashDev->hDma);

    return RK_SUCCESS;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API rk_err_t SpiFlashDev_Delete(uint32 DevID, void * arg)
{
    //Check SpiFlashDev is not exist...

    SPIFLASH_DEV_ARG * pstSpiFlashDevArg;

    pstSpiFlashDevArg = (SPIFLASH_DEV_ARG *)arg;

    if(gpstSpiFlashDevInf[DevID] == NULL)
    {
        return RK_ERROR;
    }

    if(pstSpiFlashDevArg == NULL)
    {
        return RK_ERROR;
    }

    pstSpiFlashDevArg->hDma = gpstSpiFlashDevInf[DevID]->hDma;

    //SpiFlashDev deinit...
    SpiFlashDevDeInit(gpstSpiFlashDevInf[DevID]);
    SpiFlashDevHwDeInit();

    //Free SpiFlashDev memory...
    rkos_semaphore_delete(gpstSpiFlashDevInf[DevID]->osSpiFlashOperSem);
    rkos_memory_free(gpstSpiFlashDevInf[DevID]);

    //Delete SpiFlashDev...
    gpstSpiFlashDevInf[DevID] = NULL;

    //Delete SpiFlashDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_SPIFLASH_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT API HDC SpiFlashDev_Create(uint32 DevID, void * arg)
{
    SPIFLASH_DEV_ARG * pstSpiFlashDevArg;
    DEVICE_CLASS* pstDev;
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev;

//  if(arg == NULL)
//  {
//      return (HDC)RK_PARA_ERR;
//  }

    // Create handler...
    pstSpiFlashDevArg = (SPIFLASH_DEV_ARG *)arg;
    pstSpiFlashDev =  rkos_memory_malloc(sizeof(SPIFLASH_DEVICE_CLASS));
    memset(pstSpiFlashDev, 0, sizeof(SPIFLASH_DEVICE_CLASS));
    if (pstSpiFlashDev == NULL)
    {
        return NULL;
    }

    //init handler...

    pstSpiFlashDev->osSpiFlashOperSem  = rkos_semaphore_create(1,1);

    if((pstSpiFlashDev->osSpiFlashOperSem) == 0)
    {
        rkos_memory_free(pstSpiFlashDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstSpiFlashDev;
    pstDev->suspend = SpiFlashDevSuspend;
    pstDev->resume  = SpiFlashDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    //init arg...
    //pstSpiFlashDev->usbmode = pstSpiFlashDevArg->usbmode;
    pstSpiFlashDev->hDma = pstSpiFlashDevArg->hDma;

    //device init...
    gpstSpiFlashDevInf[DevID] = NULL;

    SpiFlashDevHwInit();
    if(SpiFlashDevInit(pstSpiFlashDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstSpiFlashDev->osSpiFlashOperSem);
        rkos_memory_free(pstSpiFlashDev);
        return (HDC) RK_ERROR;
    }
    gpstSpiFlashDevInf[DevID] = pstSpiFlashDev;
    //SpiFlash_Test(pstSpiFlashDev);
    return (HDC)pstDev;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDevDeInit
** Input:SPIFLASH_DEVICE_CLASS * pstSpiFlashDev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT FUN rk_err_t SpiFlashDevDeInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: SpiFlashDevInit
** Input:SPIFLASH_DEVICE_CLASS * pstSpiFlashDev
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_INIT_
INIT FUN rk_err_t SpiFlashDevInit(SPIFLASH_DEVICE_CLASS * pstSpiFlashDev)
{
    sfc_bsp_init();
    sfc_set_fifo_threshold(0,0);
    memset(&sfc_ctrl, 0, sizeof(SFC_STRUCT));

    sfc_ctrl.rw_complete_sema = rkos_semaphore_create(1, 0);
    if(sfc_ctrl.rw_complete_sema == NULL)
    {
        return RK_ERROR;
    }

    if(SFlashInit())
    {
        rkos_semaphore_delete(sfc_ctrl.rw_complete_sema);
    }

    pstSpiFlashDev->capacity = SFlashGetCapacity();
    return RK_SUCCESS;

}


#ifdef _SPIFLASH_DEV_SHELL_
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
static SHELL_CMD ShellSpiFlashName[] =
{
    "pcb",SpiFlashDevShellPcb,"list spiflash device pcb inf","spiflash.pcb [object id]",
    "create",SpiFlashDevShellMc,"create spiflash device","spiflash.create [object id]",
    "delete",SpiFlashDevShellDel,"delete spiflash device","spiflash.delete [object id]",
    "test",SpiFlashDevShellTest,"test spiflash device","spiflash.test [object id]",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
void SpiFlash_Test(HDC dev)
{
    int i;
    uint8 *readbuf = rkos_memory_malloc(2048);
    uint8 *writebuf = rkos_memory_malloc(2048);

    memset(readbuf, 0, 512);
    memset(writebuf, 0, 512);

    for(i=0; i<256; i++)
    {
        writebuf[i] = i + 200;
        writebuf[i+256] = i + 200;
    }

    SpiFlashDev_Write(dev, 0 + 0, writebuf, 1);
    SpiFlashDev_Read(dev,  0 + 0, readbuf, 1);

    //LUNWriteDB(256, 1, writebuf);
    //LUNReadDB(256, 1, readbuf);

    for(i=0; i<256; i++)
    {
        if(readbuf[i] != (uint8)(i + 200) || readbuf[i+256] != (uint8)(i + 200))
        {
            rk_printf("SpiFlashDev test fail = %d", i);
            rkos_memory_free(readbuf);
            rkos_memory_free(writebuf);
            return;
        }

    }
    rk_print_string("SpiFlashDev test ok\n");
    rkos_memory_free(readbuf);
    rkos_memory_free(writebuf);
}
/*******************************************************************************
** Name: SpiFlashDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL API rk_err_t SpiFlashDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret = RK_SUCCESS;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellSpiFlashName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);

    if ((StrCnt == 0) || (*(pstr - 1) != '.'))
    {
        return RK_ERROR;
    }
    ret = ShellCheckCmd(ShellSpiFlashName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellSpiFlashName[i].CmdDes, pItem);
    if(ShellSpiFlashName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellSpiFlashName[i].ShellCmdParaseFun(dev, pItem);
    }
    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: SpiFlashDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellTest(HDC dev, uint8 * pstr)
{
    HDC hSpiFlashDev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Get SpiFlashDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Open SpiFlashDev...
    hSpiFlashDev = RKDev_Open(DEV_CLASS_SPIFLASH, 0, NOT_CARE);
    if((hSpiFlashDev == NULL) || (hSpiFlashDev == (HDC)RK_ERROR) || (hSpiFlashDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("SpiFlashDev open failure");
        return RK_SUCCESS;
    }

    //do test....

    //close SpiFlashDev...
    SpiFlash_Test(hSpiFlashDev);
    RKDev_Close(hSpiFlashDev);
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Get SpiFlashDev ID...
    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }
    if(RKDev_Delete(DEV_CLASS_SPIFLASH, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("SPIFLASHDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellMc(HDC dev, uint8 * pstr)
{
    SPIFLASH_DEV_ARG stSpiFlashDevArg;
    rk_err_t ret;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "0", 1) == 0)
    {
        DevID = 0;
    }
    else if(StrCmpA(pstr, "1", 1) == 0)
    {
        DevID = 1;
    }
    else
    {
        DevID = 0;
    }

    //Init SpiFlashDev arg...
    //stSpiFlashDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create SpiFlashDev...
    ret = RKDev_Create(DEV_CLASS_SPIFLASH, DevID, &stSpiFlashDevArg);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("SpiFlashDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: SpiFlashDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:wangping
** Date: 2015.6.23
** Time: 10:30:03
*******************************************************************************/
_DRIVER_SPIFLASH_SPIFLASHDEV_SHELL_
SHELL FUN rk_err_t SpiFlashDevShellPcb(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    SPIFLASH_DEVICE_CLASS * pstSpiFlashDev;
    uint32 i;

    // TODO:
    //add other code below:
    //...

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    DevID = String2Num(pstr);

    if(DevID >= SPIFLASH_DEV_NUM)
    {
        return RK_ERROR;
    }

    pstSpiFlashDev = gpstSpiFlashDevInf[DevID];

    if(pstSpiFlashDev == NULL)
    {
        rk_printf("spiflash%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstSpiFlashDevInf[%d]", DevID);
    rk_printf_no_time("    .stSpiFlashDevice");
    rk_printf_no_time("        .next = %08x",pstSpiFlashDev->stSpiFlashDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstSpiFlashDev->stSpiFlashDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstSpiFlashDev->stSpiFlashDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstSpiFlashDev->stSpiFlashDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstSpiFlashDev->stSpiFlashDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstSpiFlashDev->stSpiFlashDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstSpiFlashDev->stSpiFlashDevice.resume);
    rk_printf_no_time("    .osSpiFlashOperSem = %08x",pstSpiFlashDev->osSpiFlashOperSem);
    rk_printf_no_time("    .capacity = %08x",pstSpiFlashDev->capacity);
    return RK_SUCCESS;
}


#endif

#endif
