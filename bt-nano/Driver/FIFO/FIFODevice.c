/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: Driver\FIFO\FIFODevice.c
* Owner: Aaron.sun
* Date: 2014.5.27
* Time: 20:37:09
* Desc: FIFO Device Class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    Aaron.sun     2014.5.27     20:37:09   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_FIFO_FIFODEVICE_C__

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
#include "AudioControlTask.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef  struct _FIFO_DEVICE_CLASS
{
    DEVICE_CLASS stfifoDevice;

    pSemaphore osfifoReadReqSem;
    pSemaphore osfifoWriteReqSem;

    pSemaphore osfifoReadSem;
    pSemaphore osfifoWriteSem;

    pSemaphore osfifoBufLevelSem;

    uint32 BlockSize;
    uint32 BlockCnt;
    uint32 CurSize;
    uint32 TotalSize;
    uint32 TotalWriteSize;
    uint8  *pbuf;
    uint32 front;
    uint32 back;
    uint32 ReadReqSize;
    uint32 ReadStatus;
    uint32 BufLevel;
    uint32 WriteReqSize;
    uint32 WriteStatus;
    uint32 UseFile;
    uint32 SizeFlag;
    uint32 FifoLevelMin;
    uint32 FifoLevelMax;
    uint32 FirstRead;
    uint32 ForceStop;
    uint32 ReqForceStop;
    uint32 WriteErr;
    HDC hReadFile;
    HDC hWriteFile;
    HDC hReadFile1;
    HDC hReadFile2;

    pFIFO_CALL_BACK pfReadCallBack;
    pFIFO_CALL_BACK pfWriteCallBack;

}FIFO_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static FIFO_DEVICE_CLASS * gpstfifoDevISR[FIFO_DEV_NUM];



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
void HttpStatus(int status, void *httppcb);
rk_err_t http_write_fifo(uint8 *buf, uint16 write_len, uint32 mlen);
rk_err_t fifoDev_Init(FIFO_DEVICE_CLASS * pstFIFODev);
rk_err_t fifoDevShellTest(HDC dev, uint8 * pstr);
rk_err_t fifoDevShellDel(HDC dev, uint8 * pstr);
rk_err_t fifoDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t fifoDevShellPcb(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: fifoDev_FlushBuf
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2016.9.21
** Time: 9:40:47
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_FlushBuf(HDC dev)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    if(pstFIFODev->CurSize > pstFIFODev->FifoLevelMin)
    {
        rkos_semaphore_take(pstFIFODev->osfifoReadReqSem, MAX_DELAY);
        pstFIFODev->front = pstFIFODev->back;
        if(pstFIFODev->UseFile)
        {
            FileDev_FileSeek(pstFIFODev->hReadFile, SEEK_CUR, pstFIFODev->CurSize);
        }
        pstFIFODev->CurSize = 0;

        rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: fifoDev_SetWriteErr
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.16
** Time: 16:42:12
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_SetWriteErr(HDC dev)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    pstFIFODev->WriteErr = 1;

    if(pstFIFODev->ReadStatus == DEVICE_STATUS_SYNC_PIO_READ)
    {
        pstFIFODev->ForceStop = 1;

        if(pstFIFODev->ReadStatus == DEVICE_STATUS_SYNC_PIO_READ)
        {
            rkos_semaphore_give(pstFIFODev->osfifoReadSem);
            pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
            pstFIFODev->ReadReqSize = 0;
        }

        rk_printf("fifo force stop4");

    }

    if(pstFIFODev->BufLevel == 1)
    {
        pstFIFODev->ForceStop = 1;
        if(pstFIFODev->BufLevel == 1)
        {
            rkos_semaphore_give(pstFIFODev->osfifoBufLevelSem);
            pstFIFODev->BufLevel = 0;
        }
        rk_printf("fifo force stop5");
    }

    if(pstFIFODev->WriteStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
    {

        pstFIFODev->ForceStop = 1;

        if(pstFIFODev->WriteStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
        {
            rkos_semaphore_give(pstFIFODev->osfifoWriteSem);
            pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
            pstFIFODev->WriteStatus = 0;
        }

        rk_printf("fifo force stop6");
    }


}


/*******************************************************************************
** Name: fifoDev_ForceStop
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.29
** Time: 14:55:02
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_ForceStop(HDC dev)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;

    pstFIFODev->ReqForceStop = 1;

    if(pstFIFODev->ReadStatus == DEVICE_STATUS_SYNC_PIO_READ)
    {

        pstFIFODev->ForceStop = 1;

        if(pstFIFODev->ReadStatus == DEVICE_STATUS_SYNC_PIO_READ)
        {
            rkos_semaphore_give(pstFIFODev->osfifoReadSem);
            pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
            pstFIFODev->ReadReqSize = 0;
        }

        rk_printf("fifo force stop1");
    }

    if(pstFIFODev->BufLevel == 1)
    {
        pstFIFODev->ForceStop = 1;
        if(pstFIFODev->BufLevel == 1)
        {
            rkos_semaphore_give(pstFIFODev->osfifoBufLevelSem);
            pstFIFODev->BufLevel = 0;
        }
        rk_printf("fifo force stop2");
    }

    if(pstFIFODev->WriteStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
    {

        pstFIFODev->ForceStop = 1;

        if(pstFIFODev->WriteStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
        {
            rkos_semaphore_give(pstFIFODev->osfifoWriteSem);
            pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
            pstFIFODev->WriteStatus = 0;
        }

        rk_printf("fifo force stop3");
    }


}

/*******************************************************************************
** Name: fifoDev_SetFIFOLevel
** Input:uint32 MaxLevel, uint32 MinLevel
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.10.29
** Time: 12:04:13
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_SetFIFOLevel(HDC dev, uint32 MaxLevel, uint32 MinLevel)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    if(MaxLevel > (pstFIFODev->BlockSize * pstFIFODev->BlockCnt))
    {
        MaxLevel = (pstFIFODev->BlockSize * pstFIFODev->BlockCnt);
    }

    if(MinLevel > MaxLevel)
    {
        MinLevel = MaxLevel;
    }

    pstFIFODev->FifoLevelMax = MaxLevel;
    pstFIFODev->FifoLevelMin = MinLevel;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: fifoDev_GetTotalSize
** Input:HDC dev, uint32 * Totalsize
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 10:03:20
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_GetTotalSize(HDC dev, uint32 * Totalsize)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;

    if(pstFIFODev->TotalSize == 0)
    {
        if(pstFIFODev->ReqForceStop)
        {
            rk_printf("get total size err!");
            return RK_ERROR;
        }

        rk_printf("get total size");
        pstFIFODev->SizeFlag = 1;
        rkos_semaphore_take(pstFIFODev->osfifoReadSem, MAX_DELAY);

        if(pstFIFODev->ForceStop)
        {
            rk_printf("get total size err!");
            return RK_ERROR;
        }
    }

    *Totalsize = pstFIFODev->TotalSize;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: fifoDev_GetOffset
** Input:HDC dev, uint32 * offset
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 10:02:42
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_GetOffset(HDC dev, uint32 * offset, uint32 fn)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;

    if(fn == 0)
    {
        *offset = pstFIFODev->back;
    }
    else if(fn == 1)
    {
        FileDev_GetFileOffset(pstFIFODev->hReadFile1, offset);
    }
    else if(fn == 2)
    {
        FileDev_GetFileOffset(pstFIFODev->hReadFile2, offset);
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: fifoDev_ReadSeek
** Input:HDC dev,  uint32 pos, int32 offset
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.14
** Time: 8:55:38
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_ReadSeek(HDC dev,  uint32 pos, int32 offset, uint32 fn)
{

    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    uint32 fifolen, front, back, front1, back1;

    if(pstFIFODev == NULL)
    {
        return RK_ERROR;
    }

    if(pstFIFODev->ForceStop)
    {
        rk_printf("fifo seek error!");
        return RK_ERROR;
    }

    rk_printf("file seek1");

    if(fn == 1)
    {
        return FileDev_FileSeek(pstFIFODev->hReadFile1, pos, offset);
    }
    else if(fn == 2)
    {
        return FileDev_FileSeek(pstFIFODev->hReadFile2, pos, offset);
    }

    rkos_semaphore_take(pstFIFODev->osfifoReadReqSem, MAX_DELAY);

    rkos_enter_critical();
    fifolen = pstFIFODev->BlockCnt * pstFIFODev->BlockSize + 1;
    front = pstFIFODev->front % fifolen;
    back = pstFIFODev->back % fifolen;
    front1 = pstFIFODev->front;
    back1 = pstFIFODev->back;
    rkos_exit_critical();


    if(pos == SEEK_END)
    {
        return RK_ERROR;
    }
    else if(pos == SEEK_CUR)
    {
        if(pstFIFODev->UseFile == 0)
        {
            if(offset < 0)
            {
                return RK_ERROR;
            }
        }

        while((pstFIFODev->front) <= ( pstFIFODev->back + offset))
        {
            pstFIFODev->ReadStatus = DEVICE_STATUS_SYNC_PIO_READ;

            if(pstFIFODev->ReqForceStop)
            {
                rk_printf("offf");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            if(pstFIFODev->WriteErr)
            {
                rk_printf("offg");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            pstFIFODev->ReadReqSize = offset - pstFIFODev->back;
            rkos_semaphore_take(pstFIFODev->osfifoReadSem, MAX_DELAY);

            if(pstFIFODev->ForceStop)
            {
                rk_printf("offh");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            rkos_enter_critical();
            fifolen = pstFIFODev->BlockCnt * pstFIFODev->BlockSize + 1;
            front = pstFIFODev->front % fifolen;
            back = pstFIFODev->back % fifolen;
            front1 = pstFIFODev->front;
            back1 = pstFIFODev->back;
            rkos_exit_critical();

        }

        if(pstFIFODev->UseFile == 0)
        {
            pstFIFODev->back += offset;
        }
        else
        {
            pstFIFODev->back += offset;
            FileDev_FileSeek(pstFIFODev->hReadFile, SEEK_SET, pstFIFODev->back % fifolen);
        }

        rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
        return RK_SUCCESS;

    }
    else if(pos == SEEK_SET)
    {
        if(pstFIFODev->UseFile == 0)
        {
            if(offset < pstFIFODev->back)
            {
                return RK_ERROR;
            }
        }

        while(pstFIFODev->front <= offset)
        {
            pstFIFODev->ReadStatus = DEVICE_STATUS_SYNC_PIO_READ;

            if(pstFIFODev->ReqForceStop)
            {
                rk_printf("offh");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            if(pstFIFODev->WriteErr)
            {
                rk_printf("offi");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            pstFIFODev->ReadReqSize = offset - pstFIFODev->back;

            rk_printf("file seek suspend");

            rkos_semaphore_take(pstFIFODev->osfifoReadSem, MAX_DELAY);

            if(pstFIFODev->ForceStop)
            {
                rk_printf("offj");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            rkos_enter_critical();
            fifolen = pstFIFODev->BlockCnt * pstFIFODev->BlockSize + 1;
            front = pstFIFODev->front % fifolen;
            back = pstFIFODev->back % fifolen;
            front1 = pstFIFODev->front;
            back1 = pstFIFODev->back;
            rkos_exit_critical();
        }

        if(pstFIFODev->UseFile == 0)
        {
            pstFIFODev->back = offset;
        }
        else
        {
            pstFIFODev->back = offset;
            FileDev_FileSeek(pstFIFODev->hReadFile, SEEK_SET, pstFIFODev->back % fifolen);
        }

        rk_printf("fifo seek ok, back = %d", pstFIFODev->back);

        rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
        return RK_SUCCESS;

    }


}

/*******************************************************************************
** Name: fifoDev_SetTotalSize
** Input:HDC dev, uint32 TotalSize
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.9
** Time: 9:14:55
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_SetTotalSize(HDC dev, uint32 TotalSize)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    FILE_ATTR stFileAttr;

    pstFIFODev->front = 0;
    pstFIFODev->back = 0;
    pstFIFODev->ReadReqSize = 0;
    pstFIFODev->WriteReqSize = 0;
    pstFIFODev->pfReadCallBack = NULL;
    pstFIFODev->pfWriteCallBack = NULL;
    pstFIFODev->CurSize = 0;
    pstFIFODev->TotalWriteSize = 0;
    pstFIFODev->SizeFlag = 0;
    pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
    pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
    pstFIFODev->BufLevel = 0;
    pstFIFODev->FirstRead = 1;
    pstFIFODev->FifoLevelMin = 0;
    pstFIFODev->FifoLevelMax = 0;
    pstFIFODev->ForceStop = 0;
    pstFIFODev->ReqForceStop = 0;
    pstFIFODev->WriteErr = 0;

    if(pstFIFODev->TotalSize == 0)
    {
        pstFIFODev->TotalSize = TotalSize;

        if(pstFIFODev->TotalSize != 0)
        {
            rk_printf("set total size");
            if(pstFIFODev->SizeFlag == 1)
            {
                pstFIFODev->SizeFlag = 0;
                rkos_semaphore_give(pstFIFODev->osfifoReadSem);
            }
        }
    }
    else
    {
        if(pstFIFODev->UseFile)
        {
            FileDev_FileSeek(pstFIFODev->hReadFile, SEEK_SET, 0);
            FileDev_FileSeek(pstFIFODev->hReadFile1, SEEK_SET, 0);
            FileDev_FileSeek(pstFIFODev->hReadFile2, SEEK_SET, 0);

            if(pstFIFODev->hWriteFile != NULL)
            {
                FileDev_CloseFile(pstFIFODev->hWriteFile);
            }

            pstFIFODev->hWriteFile = FileDev_CloneFileHandler(pstFIFODev->hReadFile);
            if((rk_err_t)pstFIFODev->hWriteFile <= 0)
            {
                rk_print_string("file cache buffer write handle open fail");
                return RK_ERROR;
            }
        }

        pstFIFODev->TotalSize = TotalSize;
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: fifoDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_Read(HDC dev, uint8 * pbuf, uint32 size, uint32 fn, uint32 Mode, pFIFO_CALL_BACK pfReadCallBack)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    uint32 fifolen, front, back, front1, back1;

    if(pstFIFODev == NULL)
    {
        return RK_ERROR;
    }

    if(pstFIFODev->ForceStop)
    {
        rk_printf("off1");
        return RK_ERROR;
    }

    if(fn == 1)
    {
        return FileDev_ReadFile(pstFIFODev->hReadFile1, pbuf, size);
    }
    else if(fn == 2)
    {
        return FileDev_ReadFile(pstFIFODev->hReadFile2, pbuf, size);
    }


    rkos_semaphore_take(pstFIFODev->osfifoReadReqSem, MAX_DELAY);

    if(pstFIFODev->TotalSize != 0xffffffff)
    {
        if(pstFIFODev->back >= pstFIFODev->TotalSize)
        {
            rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
            return RK_ERROR;
        }

        if((pstFIFODev->back + size) > pstFIFODev->TotalSize)
        {
            rk_printf("fifo end, back = %d", pstFIFODev->back);
            size = pstFIFODev->TotalSize - pstFIFODev->back;
        }
    }

    #if 0
    if((pstFIFODev->CurSize < size) && (pstFIFODev->CurSize > pstFIFODev->FifoLevelMin))
    {
        size = pstFIFODev->FifoLevelMin;
    }
    #endif

    if(pstFIFODev->FirstRead == 1)
    {
        if((pstFIFODev->CurSize < pstFIFODev->FifoLevelMax) && (pstFIFODev->front < pstFIFODev->TotalSize))
        {
             rk_printf("first buf level wait");

             pstFIFODev->BufLevel = 1;

             if(pstFIFODev->ReqForceStop)
             {
                 rk_printf("off2");
                 rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                 return RK_ERROR;
             }

             if(pstFIFODev->WriteErr)
             {
                 rk_printf("off3");
                 rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                 return RK_ERROR;
             }

             rkos_semaphore_take(pstFIFODev->osfifoBufLevelSem,  MAX_DELAY);

             rk_printf("first buf level wait over");

             if(pstFIFODev->ForceStop)
             {
                rk_printf("off4");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
             }
        }
        pstFIFODev->FirstRead = 0;

    }
    else if((pstFIFODev->CurSize < pstFIFODev->FifoLevelMin) && (pstFIFODev->front < pstFIFODev->TotalSize))
    {
         rk_printf("buf level wait = %d, %d", pstFIFODev->CurSize, pstFIFODev->FifoLevelMin);

         pstFIFODev->BufLevel = 1;

         if(pstFIFODev->ReqForceStop)
         {
             rk_printf("off5");
             rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
             return RK_ERROR;
         }

         if(pstFIFODev->WriteErr)
         {
             rk_printf("off6");
             rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
             return RK_ERROR;
         }

         rkos_semaphore_take(pstFIFODev->osfifoBufLevelSem,  MAX_DELAY);

         rk_printf("buf level wait over");

         if(pstFIFODev->ForceStop)
         {
            rk_printf("off7");
            rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
            return RK_ERROR;
         }
    }

    rkos_enter_critical();
    fifolen = pstFIFODev->BlockCnt * pstFIFODev->BlockSize + 1;
    front = pstFIFODev->front % fifolen;
    back = pstFIFODev->back % fifolen;
    front1 = pstFIFODev->front;
    back1 = pstFIFODev->back;
    rkos_exit_critical();

    if((front1 - back1) < size)
    {
        if(Mode == SYNC_MODE)
        {
            pstFIFODev->ReadStatus = DEVICE_STATUS_SYNC_PIO_READ;

            if(pstFIFODev->ReqForceStop)
            {
                rk_printf("off8");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            if(pstFIFODev->WriteErr)
            {
                rk_printf("off9");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            pstFIFODev->ReadReqSize = size;
            rk_printf("fifo read suspend front1 = %d, back1 = %d, size = %d", front1, back1, size);
            rkos_semaphore_take(pstFIFODev->osfifoReadSem, MAX_DELAY);
            rk_printf("fifo resume");
            if(pstFIFODev->ForceStop)
            {
                rk_printf("offa");
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }

            rkos_enter_critical();
            front = pstFIFODev->front % fifolen;
            back = pstFIFODev->back % fifolen;
            front1 = pstFIFODev->front;
            back1 = pstFIFODev->back;
            rkos_exit_critical();
        }
        else if(Mode == ASYNC_MODE)
        {
            pstFIFODev->ReadStatus = DEVICE_STATUS_ASYNC_PIO_READ;
            pstFIFODev->ReadReqSize = size;
            pstFIFODev->pfReadCallBack = pfReadCallBack;
            return 0;
        }
        else
        {
            rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
            return RK_PARA_ERR;
        }
    }

    if(front >= (back + size))
    {
        if(pstFIFODev->UseFile)
        {
            //rk_printf("fifo read front1 = %d, back1 = %d", front1, back1);
            FileDev_ReadFile(pstFIFODev->hReadFile, pbuf, size);
        }
        else
        {
            memcpy(pbuf, pstFIFODev->pbuf + back, size);
        }
    }
    else if(front <= (back - 1))
    {

        if(size <= (fifolen - back))
        {
            if(pstFIFODev->UseFile)
            {
                FileDev_ReadFile(pstFIFODev->hReadFile, pbuf, size);
            }
            else
            {
                memcpy(pbuf, pstFIFODev->pbuf + back, size);
            }
        }
        else
        {
            if(pstFIFODev->UseFile)
            {
                FileDev_ReadFile(pstFIFODev->hReadFile, pbuf, fifolen - back);
            }
            else
            {
                memcpy(pbuf, pstFIFODev->pbuf + back, fifolen - back);
            }

            if(front >= (size - fifolen + back))
            {
                if(pstFIFODev->UseFile)
                {
                    FileDev_FileSeek(pstFIFODev->hReadFile, SEEK_SET, 0);
                    FileDev_ReadFile(pstFIFODev->hReadFile, pbuf + fifolen - back, size - fifolen + back);
                }
                else
                {
                    memcpy(pbuf + fifolen - back, pstFIFODev->pbuf, size - fifolen + back);
                }
            }
            else
            {
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
                return RK_ERROR;
            }
        }
    }
    else
    {
       rk_printf("fifo read err  front = %d,  back = %d", front, back);
       rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
       return RK_ERROR;
    }

    rkos_enter_critical();
    pstFIFODev->back += size;
    rkos_exit_critical();

    if(pstFIFODev->WriteReqSize != 0)
    {
        if((fifolen - pstFIFODev->front + pstFIFODev->back) >= pstFIFODev->WriteReqSize)
        {
            if(pstFIFODev->WriteStatus == DEVICE_STATUS_ASYNC_PIO_WRITE)
            {
                if(pstFIFODev->pfWriteCallBack)
                pstFIFODev->pfWriteCallBack();
                pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
                pstFIFODev->WriteReqSize = 0;
                rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);

            }
            else if(pstFIFODev->WriteStatus == DEVICE_STATUS_SYNC_PIO_WRITE)
            {
                if(!pstFIFODev->ForceStop)
                {
                    pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
                    pstFIFODev->WriteReqSize = 0;
                    rkos_semaphore_give(pstFIFODev->osfifoWriteSem);
                }
            }
            else
            {
                pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
                pstFIFODev->WriteReqSize = 0;
            }
        }
    }

    rkos_enter_critical();
    pstFIFODev->CurSize = pstFIFODev->front - pstFIFODev->back;
    rkos_exit_critical();

    if(pstFIFODev->back >= fifolen)
    {
        pstFIFODev->back -= fifolen;
        pstFIFODev->front -= fifolen;
    }

    rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);
    return (rk_err_t)size;

}

/*******************************************************************************
** Name: fifoDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_COMMON_
COMMON API rk_err_t fifoDev_Write(HDC dev, uint8 * pbuf, uint32 size, uint32 Mode, pFIFO_CALL_BACK pfWriteCallBack)
{
    FIFO_DEVICE_CLASS * pstFIFODev = (FIFO_DEVICE_CLASS *)dev;
    uint32 fifolen, front, back, front1, back1;

    if(pstFIFODev == NULL)
    {
        return RK_ERROR;
    }

    if(pstFIFODev->front >= pstFIFODev->TotalSize)
    {
        rk_printf("fifo full");
        return RK_ERROR;
    }

    if(pstFIFODev->UseFile)
    {
        if(pstFIFODev->hWriteFile == NULL)
        {
            return RK_ERROR;
        }
    }

    //rk_printf("fifo write");

    rkos_semaphore_take(pstFIFODev->osfifoWriteReqSem, MAX_DELAY);

    rkos_enter_critical();
    fifolen =  pstFIFODev->BlockCnt * pstFIFODev->BlockSize + 1;
    front1 = pstFIFODev->front;
    back1 = pstFIFODev->back;
    front = pstFIFODev->front % fifolen;
    back = pstFIFODev->back % fifolen;
    rkos_exit_critical();

    if((fifolen - front1 + back1 - 1) < size)
    {
        if(Mode == SYNC_MODE)
        {

            pstFIFODev->WriteStatus = DEVICE_STATUS_SYNC_PIO_WRITE;

            if(pstFIFODev->ReqForceStop)
            {
                rk_printf("offc");
                rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
                return RK_ERROR;
            }

            if(pstFIFODev->WriteErr)
            {
                rk_printf("offd");
                rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
                return RK_ERROR;
            }

            rk_printf("fifo write suspend front1 = %d, back1 = %d, size = %d", front1, back1, size);

            pstFIFODev->WriteReqSize = size;
            rkos_semaphore_take(pstFIFODev->osfifoWriteSem, MAX_DELAY);
            rk_printf("write resume");

            if(pstFIFODev->ForceStop)
            {
                rk_printf("offe");
                rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
                return RK_ERROR;
            }

            rkos_enter_critical();
            front = pstFIFODev->front % fifolen;
            back = pstFIFODev->back % fifolen;
            rkos_exit_critical();
            pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
            pstFIFODev->WriteReqSize = 0;
        }
        else if(Mode == ASYNC_MODE)
        {
            pstFIFODev->WriteStatus = DEVICE_STATUS_ASYNC_PIO_WRITE;
            pstFIFODev->WriteReqSize = size;
            pstFIFODev->pfWriteCallBack = pfWriteCallBack;
            rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
            return 0;
        }
        else
        {
            rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
            return RK_PARA_ERR;
        }
    }

    if(front >= back)
    {
        if(size <= (fifolen - front))
        {
            if(pstFIFODev->UseFile)
            {
                if(pbuf != NULL)
                {
                    FileDev_WriteFile(pstFIFODev->hWriteFile, pbuf, size);
                }
                else
                {
                    FileDev_FileSeek(pstFIFODev->hWriteFile, SEEK_CUR, size);
                }

                FileDev_SynchFileHandler(pstFIFODev->hReadFile, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);
                FileDev_SynchFileHandler(pstFIFODev->hReadFile1, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);
                FileDev_SynchFileHandler(pstFIFODev->hReadFile2, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);
            }
            else
            {
                memcpy(pstFIFODev->pbuf + front, pbuf, size);
            }
        }
        else
        {
            if(pstFIFODev->UseFile)
            {
                rk_printf("fifo reback");

                if(pbuf != NULL)
                {
                    FileDev_WriteFile(pstFIFODev->hWriteFile, pbuf, fifolen - front);
                }
                else
                {
                    FileDev_FileSeek(pstFIFODev->hWriteFile, SEEK_CUR, fifolen - front);
                }

                FileDev_SynchFileHandler(pstFIFODev->hReadFile, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);
                FileDev_SynchFileHandler(pstFIFODev->hReadFile1, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);
                FileDev_SynchFileHandler(pstFIFODev->hReadFile2, pstFIFODev->hWriteFile,SYNCH_TOTAL_SIZE);

            }
            else
            {
                memcpy(pstFIFODev->pbuf + front, pbuf, fifolen - front);
            }

            if((back - 1) < size - fifolen + front)
            {
                rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
                return RK_ERROR;
            }
            else
            {
                if(pstFIFODev->UseFile)
                {
                    FileDev_FileSeek(pstFIFODev->hWriteFile, SEEK_SET, 0);

                    if(pbuf != NULL)
                    {
                        FileDev_WriteFile(pstFIFODev->hWriteFile, pbuf + fifolen - front, size - fifolen + front);
                    }
                    else
                    {
                        FileDev_FileSeek(pstFIFODev->hWriteFile, SEEK_CUR, size - fifolen + front);
                    }
                }
                else
                {
                    memcpy(pstFIFODev->pbuf, pbuf + fifolen - front, size - fifolen + front);
                }
            }
        }
    }
    else if((front + size) <= (back - 1))
    {
        if(pstFIFODev->UseFile)
        {
            if(pbuf != NULL)
            {
                FileDev_WriteFile(pstFIFODev->hWriteFile, pbuf, size);
            }
            else
            {
                FileDev_FileSeek(pstFIFODev->hWriteFile, SEEK_CUR, size);
            }
        }
        else
        {
            memcpy(pstFIFODev->pbuf + front, pbuf, size);
        }
    }
    else
    {
        rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
        return RK_ERROR;
    }


    if(pstFIFODev->UseFile)
    {
        if(pstFIFODev->TotalSize != 0xffffffff)
        {
            pstFIFODev->TotalWriteSize += size;

            if(pstFIFODev->TotalWriteSize >= pstFIFODev->TotalSize)
            {
                rk_printf("offb");
                FileDev_CloseFile(pstFIFODev->hWriteFile);
                pstFIFODev->hWriteFile = NULL;
            }
        }
    }

    rkos_enter_critical();
    pstFIFODev->front += size;
    pstFIFODev->CurSize += size;
    rkos_exit_critical();

    if(pstFIFODev->BufLevel)
    {
        if((pstFIFODev->CurSize >= pstFIFODev->FifoLevelMax) || (pstFIFODev->front >= pstFIFODev->TotalSize))
        {
            if(!pstFIFODev->ForceStop)
            {
                pstFIFODev->BufLevel = 0;
                rkos_semaphore_give(pstFIFODev->osfifoBufLevelSem);
            }
        }
    }

    if(pstFIFODev->ReadReqSize != 0)
    {
        if((pstFIFODev->front - pstFIFODev->back) >= pstFIFODev->ReadReqSize)
        {
            if(pstFIFODev->ReadStatus == DEVICE_STATUS_ASYNC_PIO_READ)
            {
                if(pstFIFODev->pfReadCallBack)
                pstFIFODev->pfReadCallBack();
                pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
                pstFIFODev->ReadReqSize = 0;
                rkos_semaphore_give(pstFIFODev->osfifoReadReqSem);

            }
            else if(pstFIFODev->ReadStatus == DEVICE_STATUS_SYNC_PIO_READ)
            {
                if(!pstFIFODev->ForceStop)
                {
                    pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
                    pstFIFODev->ReadReqSize = 0;
                    rkos_semaphore_give(pstFIFODev->osfifoReadSem);
                }
            }
            else
            {
                pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
                pstFIFODev->ReadReqSize = 0;
            }
        }
    }

    rkos_semaphore_give(pstFIFODev->osfifoWriteReqSem);
    return (rk_err_t)size;

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
/*******************************************************************************
** Name: fifoDev_Create
** Input:void * arg
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_INIT_
INIT API HDC fifoDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS * pstDev;
    FIFO_DEVICE_CLASS * pstFIFODev;
    FIFO_DEV_ARG * pstFIFOArg = (FIFO_DEV_ARG *)arg;
    rk_err_t ret;

    if (arg == NULL)
    {
        return (HDC)(RK_PARA_ERR);
    }

    pstFIFODev =  rkos_memory_malloc(sizeof(FIFO_DEVICE_CLASS));
    if (pstFIFODev == NULL)
    {
        return pstFIFODev;
    }

    if(pstFIFOArg->UseFile == 0)
    {
        pstFIFODev->pbuf = rkos_memory_malloc(pstFIFOArg->BlockCnt * pstFIFOArg->BlockSize + 1);
        if(pstFIFODev->pbuf == NULL)
        {
            return NULL;
        }
    }
    else
    {
        pstFIFODev->pbuf = 0;
    }

    pstFIFODev->osfifoReadReqSem        = rkos_semaphore_create(1,1);
    pstFIFODev->osfifoWriteReqSem       = rkos_semaphore_create(1,1);


    pstFIFODev->osfifoReadSem        = rkos_semaphore_create(1,0);
    pstFIFODev->osfifoWriteSem       = rkos_semaphore_create(1,0);
    pstFIFODev->osfifoBufLevelSem    = rkos_semaphore_create(1,0);

    if ((pstFIFODev->osfifoReadReqSem && pstFIFODev->osfifoWriteReqSem && pstFIFODev->osfifoReadSem
                && pstFIFODev->osfifoWriteSem && pstFIFODev->osfifoBufLevelSem) == 0)
    {

        rkos_semaphore_delete(pstFIFODev->osfifoReadReqSem);
        rkos_semaphore_delete(pstFIFODev->osfifoWriteReqSem);

        rkos_semaphore_delete(pstFIFODev->osfifoReadSem);
        rkos_semaphore_delete(pstFIFODev->osfifoWriteSem);
        rkos_semaphore_delete(pstFIFODev->osfifoBufLevelSem);
        rkos_memory_free(pstFIFODev);
        rkos_memory_free(pstFIFODev->pbuf);
        return (HDC) RK_ERROR;
    }

    pstDev = (DEVICE_CLASS *)pstFIFODev;

    pstDev->suspend = NULL;
    pstDev->resume  = NULL;


    pstFIFODev->BlockSize = pstFIFOArg->BlockSize;
    pstFIFODev->BlockCnt = pstFIFOArg->BlockCnt;
    pstFIFODev->hReadFile = pstFIFOArg->hReadFile;
    pstFIFODev->UseFile = pstFIFOArg->UseFile;

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_FIFO_DEV, SEGMENT_OVERLAY_ALL);
    #endif

    if(fifoDev_Init(pstFIFODev) != RK_SUCCESS)
    {

        rkos_semaphore_delete(pstFIFODev->osfifoReadReqSem);
        rkos_semaphore_delete(pstFIFODev->osfifoWriteReqSem);

        rkos_semaphore_delete(pstFIFODev->osfifoReadSem);
        rkos_semaphore_delete(pstFIFODev->osfifoWriteSem);
        rkos_semaphore_delete(pstFIFODev->osfifoBufLevelSem);
        rkos_memory_free(pstFIFODev);
        rkos_memory_free(pstFIFODev->pbuf);
        FW_RemoveSegment(SEGMENT_ID_FIFO_DEV);
        return (HDC) RK_ERROR;

    }

    gpstfifoDevISR[DevID] = pstFIFODev;

    return pstDev;
}

/*******************************************************************************
** Name: fifoDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_INIT_
INIT API rk_err_t fifoDev_Delete(uint32 DevID, void * arg)
{
    FIFO_DEV_ARG * pstFIFOArg = (FIFO_DEV_ARG *)arg;
    rk_printf("enter fifoDev_Delete");

    if (gpstfifoDevISR[DevID] == NULL)
    {
        rk_printf("gpstfifoDevISR NULL id=%d",DevID);
        return RK_ERROR;
    }

    if(pstFIFOArg != NULL)
    {
        if(gpstfifoDevISR[DevID]->hReadFile != NULL)
        {
            pstFIFOArg->hReadFile = gpstfifoDevISR[DevID]->hReadFile;
        }
        else
        {
            pstFIFOArg->hReadFile = NULL;
        }
    }

    if(gpstfifoDevISR[DevID]->hWriteFile != NULL)
    {
        FileDev_CloseFile(gpstfifoDevISR[DevID]->hWriteFile);
    }

    if(gpstfifoDevISR[DevID]->hReadFile1 != NULL)
    {
        FileDev_CloseFile(gpstfifoDevISR[DevID]->hReadFile1);
    }

    if(gpstfifoDevISR[DevID]->hReadFile2 != NULL)
    {
        FileDev_CloseFile(gpstfifoDevISR[DevID]->hReadFile2);
    }

    rkos_semaphore_delete(gpstfifoDevISR[DevID]->osfifoReadReqSem);
    rkos_semaphore_delete(gpstfifoDevISR[DevID]->osfifoWriteReqSem);
    rkos_semaphore_delete(gpstfifoDevISR[DevID]->osfifoReadSem);
    rkos_semaphore_delete(gpstfifoDevISR[DevID]->osfifoWriteSem);
    rkos_semaphore_delete(gpstfifoDevISR[DevID]->osfifoBufLevelSem);
    rkos_memory_free(gpstfifoDevISR[DevID]->pbuf);
    rkos_memory_free(gpstfifoDevISR[DevID]);

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_FIFO_DEV);
    #endif
    rk_printf("fifo deinit over \n");

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: fifoDev_Init
** Input:FIFO_DEVICE_CLASS * pstFIFODev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.30
** Time: 15:00:32
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_INIT_
INIT FUN rk_err_t fifoDev_Init(FIFO_DEVICE_CLASS * pstFIFODev)
{
    pstFIFODev->front = 0;
    pstFIFODev->back = 0;
    pstFIFODev->ReadReqSize = 0;
    pstFIFODev->WriteReqSize = 0;
    pstFIFODev->pfReadCallBack = NULL;
    pstFIFODev->pfWriteCallBack = NULL;
    pstFIFODev->TotalSize = 0;
    pstFIFODev->CurSize = 0;
    pstFIFODev->TotalWriteSize = 0;
    pstFIFODev->SizeFlag = 0;
    pstFIFODev->ReadStatus = DEVICE_STATUS_IDLE;
    pstFIFODev->WriteStatus = DEVICE_STATUS_IDLE;
    pstFIFODev->BufLevel = 0;
    pstFIFODev->FirstRead = 1;
    pstFIFODev->FifoLevelMin = 0;
    pstFIFODev->FifoLevelMax = 0;
    pstFIFODev->ForceStop = 0;
    pstFIFODev->ReqForceStop = 0;
    pstFIFODev->WriteErr = 0;

    if(pstFIFODev->UseFile == 1)
    {
        pstFIFODev->hWriteFile = FileDev_CloneFileHandler(pstFIFODev->hReadFile);
        if((rk_err_t)pstFIFODev->hWriteFile <= 0)
        {
            rk_print_string("file cache buffer write handle open fail");
            return RK_ERROR;
        }

        pstFIFODev->hReadFile1 = FileDev_CloneFileHandler(pstFIFODev->hReadFile);
        if((rk_err_t)pstFIFODev->hReadFile1 <= 0)
        {
            FileDev_CloseFile(pstFIFODev->hWriteFile);
            rk_print_string("file cache buffer read1 handle open fail");
            return RK_ERROR;
        }

        pstFIFODev->hReadFile2 = FileDev_CloneFileHandler(pstFIFODev->hReadFile);

        if ((rk_err_t)pstFIFODev->hReadFile2 <= 0)
        {
            FileDev_CloseFile(pstFIFODev->hWriteFile);
            FileDev_CloseFile(pstFIFODev->hReadFile1);
            rk_print_string("file cache buffer read2 handle open fail");
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;

}


#ifdef _FIFO_DEV_SHELL_
_DRIVER_FIFO_FIFODEVICE_DATA_
static SHELL_CMD ShellfifoName[] =
{
    "pcb",fifoDevShellPcb,"list fifo device pcb inf","fifo.pcb [objectid]\n",
    "create",fifoDevShellCreate,"create a fifo device","fifo.create [/objectid] [file path]\n",
    "delete",fifoDevShellDel,"delete a fifo device","fifo.delete [objectid]\n",
    "test",fifoDevShellTest,"use http play test fifo device","fifo.test <http url>\n",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: fifoDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL API rk_err_t fifoDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellfifoName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellfifoName[i].CmdDes, pItem);
    if(ShellfifoName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellfifoName[i].ShellCmdParaseFun(dev, pItem);
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
_DRIVER_FIFO_FIFODEVICE_SHELL_ HDC hFifoTest;

/*******************************************************************************
** Name: HttpStatus
** Input:int status, void *httppcb
** Return: void
** Owner:aaron.sun
** Date: 2016.6.16
** Time: 16:23:54
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN void HttpStatus(int status, void *httppcb)
{
    if((status == TCP_RECIVER_ERR) || (status == READ_DATA_ERR))
    {
        rk_printf("http error status = %d", status);
        fifoDev_SetWriteErr(hFifoTest);
    }
}
/*******************************************************************************
** Name: http_write_fifo
** Input:uint8 *buf, uint16 write_len, uint32 mlen
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.6.2
** Time: 10:31:55
*******************************************************************************/
static _DRIVER_FIFO_FIFODEVICE_SHELL_ uint8 packet[512];
_DRIVER_FIFO_FIFODEVICE_SHELL_ uint8 *http_packet = NULL;
_DRIVER_FIFO_FIFODEVICE_SHELL_ uint16 http_left_len = 0;
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN rk_err_t http_write_fifo(uint8 *buf, uint16 write_len, uint32 mlen)
{
    uint16 i,j;
    rk_err_t ret;
    uint16 data_len;

    data_len = http_left_len + write_len;
    http_packet = rkos_memory_malloc(data_len);
    if(http_packet == NULL)
    {
        rk_printf("dlna_write_fifo malloc fail");
        return RK_ERROR;
    }

    if(http_left_len > 0)
    {
        rkos_memcpy(http_packet, packet, http_left_len);
    }

    rkos_memcpy(http_packet+http_left_len, buf, write_len);

    i = data_len/512;
    j = data_len %512;

    if(i>0)
    {
       ret = fifoDev_Write(hFifoTest, http_packet, i*512, SYNC_MODE, NULL);
       if(ret == RK_ERROR)
       {
            rk_printf("dlna fifo write error1");
            rkos_memory_free(http_packet);
            http_packet = NULL;
            return RK_ERROR;
       }
    }

    if(j > 0)
    {
        http_left_len = j;
        if(mlen  > 0)
        {
            rkos_memcpy(packet, http_packet+i*512, j);

        }
        else
        {
            http_left_len = 0;
            rkos_memset(packet, 0x00, 512);
            ret = fifoDev_Write(hFifoTest, http_packet+i*512, j, SYNC_MODE, NULL);
            if(ret == RK_ERROR)
            {
                 rk_printf("dlna fifo write error2");
                 rkos_memory_free(http_packet);
                 http_packet = NULL;
                 return RK_ERROR;
            }
        }
    }
    else
    {
        http_left_len = 0;
    }
    rkos_memory_free(http_packet);
    http_packet = NULL;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: fifoDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN rk_err_t fifoDevShellTest(HDC dev, uint8 * pstr)
{
    rk_err_t ret;
    HDC hFifo;
    uint32 TotalSize;
    RK_TASK_AUDIOCONTROL_ARG pArg;
    HDC *DlnaHttp_Pcb1;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    hFifoTest = RKDev_Open(DEV_CLASS_FIFO, 0, NOT_CARE);
    if(hFifoTest == NULL)
    {
        rk_printf("fifo device open fail");
        return RK_SUCCESS;
    }


    //wifi play
    #ifdef __APP_AUDIO_AUDIOCONTROLTASK_C__

    pArg.ucSelPlayType = SOURCE_FROM_DLNA;
    pArg.FileNum = 1;
    pArg.pfAudioState = NULL;

    if(RKTaskFind(TASK_ID_AUDIOCONTROL, 0) != NULL)
    {
        AudioControlTask_SendCmd(AUDIO_CMD_STOP, (void *)Audio_Stop_Force, SYNC_MODE);
    }
    else
    {
        RKTaskCreate(TASK_ID_AUDIOCONTROL, 0, &pArg, SYNC_MODE);
    }

    #ifdef __WIFI_DLNA_C__

    FW_LoadSegment(SEGMENT_ID_HTTP, SEGMENT_OVERLAY_ALL);
    DlnaHttp_Pcb1 = HttpPcb_New(HttpStatus, http_write_fifo, FIFOWRITE);
    if(DlnaHttp_Pcb1 == NULL)
    {
        rk_printf("http open fail");
        return -1;
    }

    //ret = HttpGet_Url("http://192.168.1.100//xueyubaizhen.mp3", NULL, 0);
    //ret = HttpGet_Url(DlnaHttp_Pcb1,"http://192.168.1.100//linyuyizhizou.mp3", 0);
    //ret = HttpGet_Url(DlnaHttp_Pcb1,"http://www.zsctc-api.com:8001/box/1/01.mp3", 0);
    ret = HttpGet_Url(DlnaHttp_Pcb1, pstr, 0);

    if(ret == RK_SUCCESS)
    {
        printf("\nlocal http ok\n");
        AudioControlTask_SendCmd(AUDIO_CMD_DECSTART, NULL, ASYNC_MODE);
    }
    else
    {
        printf("\nlocal http error\n");
        return RK_SUCCESS;
    }
    #endif

    #endif

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: fifoDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN rk_err_t fifoDevShellDel(HDC dev, uint8 * pstr)
{
    FIFO_DEV_ARG pstFIFOArg;
    //add other code below:
    //...
    //RKDev_Delete(DEV_CLASS_FIFO, 0);
    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    if (RKDev_Delete(DEV_CLASS_FIFO, 0, &pstFIFOArg) != RK_SUCCESS)
    {
        rk_printf("fifo delete failure");
        return RK_ERROR;
    }

    rk_printf("fifo delete ok");

    if(pstFIFOArg.hReadFile != NULL)
    {
        if (FileDev_CloseFile(pstFIFOArg.hReadFile) != RK_SUCCESS)
        {
            rk_printf("ReadFile close failure\n");
            return RK_ERROR;
        }
    }
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: fifoDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN rk_err_t fifoDevShellCreate(HDC dev, uint8 * pstr)
{
    FIFO_DEV_ARG stFifoArg;
    rk_err_t ret;
    FILE_ATTR stFileAttr;
    uint32 DevID;
    uint8  *pItem;
    uint16 StrCnt = 0;
    uint8 Space;
    uint16 Path[MAX_DIRPATH_LEN];

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "/", 1) == 0)
    {
        pstr++;

        DevID = String2Num(pstr);

        if(DevID >= FIFO_DEV_NUM)
        {
            return RK_ERROR;
        }

        StrCnt = ShellItemExtract(pstr,&pItem, &Space);
        if (StrCnt == 0)
        {
            return RK_ERROR;
        }

        pstr = pItem + StrCnt + 1;
    }
    else
    {
        DevID = 0;
    }

    pstr--;

    if(*pstr != 0)
    {
        pstr++;

        Path[Ascii2Unicode(pstr, Path, strlen(pstr)) / 2 ] = 0;

        stFileAttr.Path = Path;
        stFileAttr.FileName = NULL;


        stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        if ((int)stFifoArg.hReadFile <= 0)
        {
            ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);
            if (ret != RK_SUCCESS)
            {
                rk_print_string("file create failure");
                return RK_SUCCESS;
            }

            stFifoArg.hReadFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
            if ((int)stFifoArg.hReadFile <= 0)
            {
                rk_print_string("file open failure");
                return RK_SUCCESS;
            }
        }

        stFifoArg.BlockCnt = 200000;
        stFifoArg.BlockSize = 1024;
        stFifoArg.UseFile = 1;

    }
    else
    {

        stFifoArg.BlockCnt = 20;
        stFifoArg.BlockSize = 1024;
        stFifoArg.UseFile = 0;
    }

    ret = RKDev_Create(DEV_CLASS_FIFO, 0, &stFifoArg);

    if(ret != RK_SUCCESS)
    {
        rk_print_string("fifo dev create failure");
        return RK_ERROR;
    }

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: fifoDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.5.27
** Time: 20:43:01
*******************************************************************************/
_DRIVER_FIFO_FIFODEVICE_SHELL_
SHELL FUN rk_err_t fifoDevShellPcb(HDC dev, uint8 * pstr)
{
    FIFO_DEVICE_CLASS *fifodev;
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //...
     DevID = String2Num(pstr);
     if(DevID > FIFO_DEV_NUM)
     {
        rk_printf("fifo dev num > fifo_dev_num");
        return RK_ERROR;
     }
     fifodev = gpstfifoDevISR[DevID];

     if(fifodev == NULL)
     {
         rk_printf("fifodev%d in not exist", DevID);
         return RK_SUCCESS;
     }

    rk_printf(".gpstfifoDevISR[%d]", DevID);
    rk_printf("      .fofpDevice");
    rk_printf("          .next = %08x",fifodev->stfifoDevice.next);
    rk_printf("          .UseCnt = %d",fifodev->stfifoDevice.UseCnt);
    rk_printf("          .SuspendCnt = %d",fifodev->stfifoDevice.SuspendCnt);
    rk_printf("          .DevClassID = %d",fifodev->stfifoDevice.DevClassID);
    rk_printf("          .DevID = %d",fifodev->stfifoDevice.DevID);
    rk_printf("          .suspend = %08x",fifodev->stfifoDevice.suspend);
    rk_printf("          .resume = %08x",fifodev->stfifoDevice.resume);
    rk_printf("          .osfifoReadReqSem = %08x",fifodev->osfifoReadReqSem);
    rk_printf("          .osfifoReadSem = %08x",fifodev->osfifoReadSem);
    rk_printf("          .osfifoWriteReqSem = %08x",fifodev->osfifoWriteReqSem);
    rk_printf("          .osfifoWriteSem = %08x",fifodev->osfifoWriteSem);
    rk_printf("          .osfifoBufLevelSem = %08x",fifodev->osfifoBufLevelSem);
    rk_printf("      .BlockSize = %d",fifodev->BlockSize);
    rk_printf("      .BlockCnt = %d",fifodev->BlockCnt);
    rk_printf("      .CurSize = %d",fifodev->CurSize);
    rk_printf("      .TotalSize = %d",fifodev->TotalSize);
    rk_printf("      .TotalWriteSize = %d",fifodev->TotalWriteSize);
    rk_printf("      .pbuf = %08x",fifodev->pbuf);
    rk_printf("      .front = %d",fifodev->front);
    rk_printf("      .back = %d",fifodev->back);
    rk_printf("      .ReadReqSize = %d",fifodev->ReadReqSize);
    rk_printf("      .ReadStatus = %d",fifodev->ReadStatus);
    rk_printf("      .BufLevel = %d",fifodev->BufLevel);
    rk_printf("      .WriteReqSize = %d",fifodev->WriteReqSize);
    rk_printf("      .WriteStatus = %d",fifodev->WriteStatus);
    rk_printf("      .UseFile = %d",fifodev->UseFile);
    rk_printf("      .SizeFlag = %d",fifodev->SizeFlag);
    rk_printf("      .FifoLevelMax = %d",fifodev->FifoLevelMax);
    rk_printf("      .FifoLevelMin = %d",fifodev->FifoLevelMin);
    rk_printf("      .FirstRead = %d",fifodev->FirstRead);
    rk_printf("      .ForceStop = %d",fifodev->ForceStop);
    rk_printf("      .ReqForceStop = %d",fifodev->ReqForceStop);
    rk_printf("      .hReadFile = %08x",fifodev->hReadFile);
    rk_printf("      .hWriteFile = %08x",fifodev->hWriteFile);
    rk_printf("      .hReadFile1 = %08x",fifodev->hReadFile1);
    rk_printf("      .hReadFile2 = %08x",fifodev->hReadFile2);
    rk_printf("      .pfReadCallBack = %08x",fifodev->pfReadCallBack);
    rk_printf("      .pfWriteCallBack = %08x",fifodev->pfWriteCallBack);

    return RK_SUCCESS;
}
#endif
#endif

