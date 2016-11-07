/*
********************************************************************************************
*
*  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                         All rights reserved.
*
* FileName: ..\Driver\File\FileDevice.c
* Owner: Aaron.sun
* Date: 2014.3.31
* Time: 18:03:12
* Desc: File Device Class
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Aaron.sun     2014.3.31     18:03:12   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __DRIVER_FILE_FILEDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define NOT_INCLUDE_OTHER
#include "typedef.h"
#include "BSP.h"
#include "device.h"
#include "RKOS.h"
#include "FileDevice.h"
#include "PartionDevice.h"
#include "FATDevice.h"
#include "FwAnalysis.h"
#include "ShellTask.h"
#include "global.h"



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define MAX_FILE_HANDLE 32

typedef struct _VOLUME_CLASS
{
    uint32 VolumeType;
    HDC    hVolume;

}VOLUME_CLASS;

typedef struct _FILE_OPER_CLASS
{
    VOLUME_CLASS * pstVolume;
    HDC hOper;
    uint32 CurOffset;
    uint32 FileSize;

}FILE_OPER_CLASS;

typedef  struct _FILE_DEVICE_CLASS
{
    DEVICE_CLASS stFileDevice;
    VOLUME_CLASS stVolume[VOLUME_NUM_MAX];
    FILE_OPER_CLASS stFileOper[MAX_FILE_HANDLE];

}FILE_DEVICE_CLASS;


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_DRIVER_FILE_FILEDEVICE_COMMON_ static FILE_DEVICE_CLASS * gpstFileDevInf[FILE_DEV_NUM_MAX] = 0;

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
rk_err_t FileDevCheckHandler(HDC dev);
rk_err_t FileDevSuspend(HDC dev, uint32 Level);
rk_err_t FileDevResume(HDC dev);
rk_err_t FileShellDd(HDC dev,uint8 * pstr);
rk_err_t FileShellDf(HDC dev,uint8 * pstr);
rk_err_t FileShellSetPath(HDC dev, uint8 * pstr);
rk_err_t FileShellMf(HDC dev, uint8 * pstr);
rk_err_t FileShellMd(HDC dev, uint8 * pstr);
rk_err_t FileShellOperHelp(HDC dev,  uint8 * pstr);
rk_err_t FileShellHelp(HDC dev,  uint8 * pstr);
rk_err_t FileShellTest(HDC dev, uint8 * pstr);
rk_err_t FileShellDelete(HDC dev, uint8 * pstr);
rk_err_t FileShellCreate(HDC dev, uint8 * pstr);
rk_err_t FileShellLs(HDC dev, uint8 * pstr);
rk_err_t FileShellOper(HDC dev, uint8 * pstr, uint16 * wpath);
FILE_OPER_CLASS * GetFileOperClass(FILE_DEVICE_CLASS * pstFileDev);
rk_err_t FileShellPcb(HDC dev, uint8 * pstr);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FileDev_WriteVolumeName
** Input:HDC dev, uint32 label, uint8 * name
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.18
** Time: 15:28:51
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_WriteVolumeName(HDC dev, uint32 label, uint8 * name)
{
    FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    i = label - 'A';

    if(i < VOLUME_NUM_MAX)
    {
        if(pstFileDev->stVolume[i].VolumeType == VOLUME_TYPE_FAT)
        {
            return FatDev_WriteVolumeName(pstFileDev->stVolume[i].hVolume, name);
        }
    }

    return RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_ReadVolumeName
** Input:HDC dev, uint32 label, uint8 * name
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.7.18
** Time: 15:27:32
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_ReadVolumeName(HDC dev, uint32 label, uint8 * name)
{
    FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    i = label - 'A';

    if(i < VOLUME_NUM_MAX)
    {
        if(pstFileDev->stVolume[i].VolumeType == VOLUME_TYPE_FAT)
        {
            return FatDev_ReadVolumeName(pstFileDev->stVolume[i].hVolume, name);
        }
    }

    return RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_GetFirstPos
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.21
** Time: 9:38:46
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetFirstPos(HDC dev)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_GetFirstPos(pstFileOper->hOper);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_GetVolumeInfByLabel
** Input:HDC dev, uint32 Label, VOLUME_INF * pstVolumeInf
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.3.11
** Time: 14:45:01
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetVolumeInfByLabel(HDC dev, uint32 Label, VOLUME_INF * pstVolumeInf)
{
    FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    i = Label - 'A';

    if(i < VOLUME_NUM_MAX)
    {
        pstVolumeInf->hVolume = pstFileDev->stVolume[i].hVolume;
        pstVolumeInf->VolumeType = pstFileDev->stVolume[i].VolumeType;
        if(pstVolumeInf->VolumeType == VOLUME_TYPE_FAT)
        {
            FatDev_GetVolumeSize(pstVolumeInf->hVolume, &pstVolumeInf->VolumeSize, &pstVolumeInf->VolumeFreeSize);
        }
        pstVolumeInf->VolumeID = Label;
        return RK_SUCCESS;
    }

    return RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_CloneFileHandler
** Input:HDC hSrc
** Return: HDC
** Owner:aaron.sun
** Date: 2015.11.30
** Time: 9:25:44
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON FUN HDC FileDev_CloneFileHandler(HDC hSrc)
{
    FILE_OPER_CLASS * pstSrc = (FILE_OPER_CLASS *)hSrc;
    VOLUME_CLASS * pstSrcVolume;
    FILE_OPER_CLASS * pstFileOper;

    if(pstSrc == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }

    if(gpstFileDevInf[FILE_DEV_NUM0] == NULL)
    {
        return (HDC)RK_ERROR;
    }

    pstFileOper = GetFileOperClass(gpstFileDevInf[FILE_DEV_NUM0]);

    if(pstFileOper == NULL)
    {
        return NULL;
    }

    pstSrcVolume = pstSrc->pstVolume;

    if(pstSrcVolume->VolumeType == VOLUME_TYPE_FAT)
    {
        pstFileOper->hOper =  FatDev_CloneFileHandler(pstSrc->hOper);
        if((rk_err_t)pstFileOper->hOper <= 0)
        {
            pstFileOper->hOper = 0;
            return (HDC)RK_ERROR;
        }
        pstFileOper->CurOffset = pstSrc->CurOffset;
        pstFileOper->FileSize = pstSrc->FileSize;
        pstFileOper->pstVolume = pstSrcVolume;
        return pstFileOper;
    }

    return (HDC)RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_SynchFileHandler
** Input:HDC hTarget, HDC hSrc, uint32 Mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.7.7
** Time: 15:49:22
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_SynchFileHandler(HDC hTarget, HDC hSrc, uint32 Mode)
{
    FILE_OPER_CLASS * pstTarget = (FILE_OPER_CLASS *)hTarget;
    FILE_OPER_CLASS * pstSrc = (FILE_OPER_CLASS *)hSrc;
    VOLUME_CLASS * pstTargetVolume, * pstSrcVolume;

    if(pstTarget == NULL || pstSrc == NULL)
    {
        return RK_PARA_ERR;
    }

    pstTargetVolume = pstTarget->pstVolume;
    pstSrcVolume = pstSrc->pstVolume;

    if((pstTargetVolume->VolumeType == VOLUME_TYPE_FAT) && (pstSrcVolume->VolumeType == VOLUME_TYPE_FAT))
    {
        return FatDev_SynchFileHandler(pstTarget->hOper, pstSrc->hOper, Mode);
    }


}

/*******************************************************************************
** Name: FileDev_GetFileSize
** Input:HDC dev, uint32 * size
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 17:51:23
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetFileSize(HDC dev, uint32 * size)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;

    *size = pstFileOper->FileSize;

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: FileDev_GetFileOffset
** Input:HDC dev, uint32 * offset
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.18
** Time: 17:40:56
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetFileOffset(HDC dev, uint32 * offset)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;

    *offset = pstFileOper->CurOffset;

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: FileDev_FileSeek
** Input:HDC dev, uint32 pos, uint32 secoffset
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.12.24
** Time: 11:30:26
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_FileSeek(HDC dev, uint32 pos, uint32 Offset)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    switch (pos)
    {
       case SEEK_END:                     /* 从文件尾计算 */
         pstFileOper->CurOffset = pstFileOper->FileSize - Offset;
       break;

       case SEEK_SET:
         pstFileOper->CurOffset = Offset;
       break;

       case SEEK_CUR:                  /* 从当前位置计算 */
         pstFileOper->CurOffset += Offset;
       break;

       default:
       return RK_PARA_ERR;
    }

    //rk_printf("pos = %d, Offset = %d pstVolume->VolumeType=%d\n", pos, Offset,pstVolume->VolumeType);

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_FileSeek(pstFileOper->hOper, pos, Offset);
    }

    return RK_ERROR;

}


/*******************************************************************************
** Name: DateToString
** Input:uint16 Data, uint8 * pstr
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.5.7
** Time: 14:10:11
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API uint32 DateToString(uint16 date, uint8 * pstr)
{
    uint32 temp;

    temp = (date >> 9) + 1980;

    pstr[3] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[2] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[1] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[0] = temp % 10 + 0x30;

    pstr[4] = '-';

    temp = (date >> 5) & 0x000F;

    pstr[6] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[5] = temp % 10 + 0x30;

    pstr[7] = '-';


    temp = date & 0x001F;

    pstr[9] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[8] = temp % 10 + 0x30;

    return 10;

}

/*******************************************************************************
** Name: TimeToString
** Input:uint16 time, uint8 ms, uint8 * pstr
** Return: uint32
** Owner:Aaron.sun
** Date: 2014.5.7
** Time: 14:08:59
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API uint32 TimeToString(uint16 time, uint8 ms, uint8 * pstr)
{
    uint32 temp;
    temp = time >> 11;

    pstr[1] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[0] = temp % 10 + 0x30;

    pstr[2] = ':';

    temp = (time >> 5) & 0x003F;

    pstr[4] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[3] = temp % 10 + 0x30;

    pstr[5] = ':';


    temp = (time & 0x001F) * 2 + (ms * 10) / 1000;

    pstr[7] = temp % 10 + 0x30;
    temp = temp / 10;
    pstr[6] = temp % 10 + 0x30;

    return 8;

}


/*******************************************************************************
** Name:  FileDev_DeleteVolume
** Input:HDC dev, VOLUME_INF pstVolumeInfo
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 15:07:06
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t  FileDev_DeleteVolume(HDC dev, VOLUME_INF * pstVolumeInf)
{
     FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
     VOLUME_CLASS * pstVolumeClass;
     uint32 i;

     if(pstFileDev == NULL)
     {
         return RK_ERROR;
     }

     pstVolumeClass = NULL;

     if(pstVolumeInf->VolumeID)
     {
        pstVolumeClass = &pstFileDev->stVolume[pstVolumeInf->VolumeID - 'A'];
     }
     else
     {
        for(i = 0; i < VOLUME_NUM_MAX; i++)
        {
            pstVolumeClass = &pstFileDev->stVolume[i];
            if((pstVolumeClass == pstVolumeInf->hVolume)
                && (pstVolumeClass->VolumeType != 0))
            {
                break;
            }

        }
     }

     if((pstVolumeClass == NULL) || (pstVolumeClass->VolumeType == 0))
     {
         return RK_ERROR;
     }

     pstVolumeInf->VolumeType = pstVolumeClass->VolumeType;
     pstVolumeInf->hVolume = pstVolumeClass->hVolume;
     pstVolumeClass->VolumeType = 0;

     return RK_SUCCESS;

}

/*******************************************************************************
** Name: FileDev_AddVolume
** Input:HDC dev,  VOLUME_INF * pstVolumeInf
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 15:03:46
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_AddVolume(HDC dev,  VOLUME_INF * pstVolumeInf)
{
    FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolumeClass;


    if(pstFileDev == NULL)
    {
        return RK_ERROR;
    }

    pstVolumeClass = &pstFileDev->stVolume[pstVolumeInf->VolumeID - 'A'];

    if(pstVolumeClass->VolumeType != 0)
    {
        return RK_ERROR;
    }

    pstVolumeClass->hVolume = pstVolumeInf->hVolume;
    pstVolumeClass->VolumeType = pstVolumeInf->VolumeType;

    return RK_SUCCESS;


}

/*******************************************************************************
** Name: FileDev_GetVolumeInfByNum
** Input:HDC dev, VOLUME_INF * pstVolumeInfo
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:47:53
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetVolumeInfByNum(HDC dev, uint32 VolumeNum, VOLUME_INF * pstVolumeInf)
{
    FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    for(i = 0; i < VOLUME_NUM_MAX; i++)
    {
        if(pstFileDev->stVolume[i].VolumeType != 0)
        {
            if(VolumeNum == 0)
            {
                break;
            }
            else
            {
               VolumeNum--;
            }
        }
    }

    if(i < VOLUME_NUM_MAX)
    {
        pstVolumeInf->hVolume = pstFileDev->stVolume[i].hVolume;
        pstVolumeInf->VolumeType = pstFileDev->stVolume[i].VolumeType;
        if(pstVolumeInf->VolumeType == VOLUME_TYPE_FAT)
        {
            FatDev_GetVolumeSize(pstVolumeInf->hVolume, &pstVolumeInf->VolumeSize, &pstVolumeInf->VolumeFreeSize);
        }
        pstVolumeInf->VolumeID = i + 'A';
        return RK_SUCCESS;
    }

    return RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_GetVolumeTotalCnt
** Input:HDC dev, uint32 * pVolumeCnt
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:46:44
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetVolumeTotalCnt(HDC dev, uint32 * pVolumeCnt)
{
      FILE_DEVICE_CLASS * pstFileDev =  (FILE_DEVICE_CLASS *)dev;
      uint32 i, Cnt;

      Cnt = 0;

      for(i = 0; i < VOLUME_NUM_MAX; i++)
      {
          if(pstFileDev->stVolume[i].VolumeType != 0)
          {
              Cnt++;
          }
      }

      *pVolumeCnt = Cnt;

      return RK_SUCCESS;

}

/*******************************************************************************
** Name: FileDev_GetTotalFile
** Input:HDC dev, uint8 * ExtName, uint8 Attr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:39:44
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetTotalFile(HDC dev, uint8 * ExtName, uint8 Attr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_GetTotalFile(pstFileOper->hOper, ExtName, Attr);
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FileDev_GetTotalDir
** Input:HDC dev, uint8 * ExtName, uint8 Attr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:38:46
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetTotalDir(HDC dev, uint8 * ExtName, uint8 Attr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_GetTotalDir(pstFileOper->hOper, ExtName, Attr);
    }

    return RK_ERROR;



}

/*******************************************************************************
** Name: FileDev_CloseDir
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:28:48
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_CloseDir(HDC dev)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    rk_err_t ret;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       ret = FatDev_CloseDir(pstFileOper->hOper);
       pstFileOper->hOper = 0;
       pstFileOper->pstVolume = 0;
       pstFileOper->CurOffset = 0;
       pstFileOper->FileSize = 0;
       return ret;
    }
    return RK_ERROR;
}


/*******************************************************************************
** Name: FileDev_CloseFile
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:28:20
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_CloseFile(HDC dev)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    rk_err_t ret;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       ret = FatDev_CloseFile(pstFileOper->hOper);
       pstFileOper->hOper = 0;
       pstFileOper->pstVolume = 0;
       pstFileOper->CurOffset = 0;
       pstFileOper->FileSize = 0;
       return ret;
    }

    return RK_ERROR;

}


/*******************************************************************************
** Name: FileDev_OpenFile
** Input: HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:24:43
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API HDC FileDev_OpenFile(HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    FILE_ATTR stFileAttr;
    FILE_OPER_CLASS * pstFileOper;
    //rk_err_t ret;

    if(pstFileDev == NULL)
    {
        return (HDC)RK_ERROR;
    }

    pstFileOper = GetFileOperClass(pstFileDev);

    if(pstFileOper == NULL)
    {
        return NULL;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    pstFileOper->CurOffset = 0;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       pstFileOper->hOper = FatDev_OpenFile(pstVolume->hVolume, ((FILE_OPER_CLASS *)hFather)->hOper, pstFileAttr);

       if((rk_err_t)pstFileOper->hOper <= 0)
       {
            pstFileOper->hOper = 0;
            return (HDC)RK_ERROR;
       }
       pstFileOper->FileSize = pstFileAttr->FileSize;
       pstFileOper->pstVolume = pstVolume;
       return pstFileOper;
    }

    return (HDC)RK_ERROR;

}


/*******************************************************************************
** Name: FileDev_OpenDir
** Input:HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:23:11
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API HDC FileDev_OpenDir(HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    FILE_ATTR stFileAttr;
    FILE_OPER_CLASS * pstFileOper;
    rk_err_t ret;

    if(pstFileDev == NULL)
    {
        return (HDC)RK_ERROR;
    }

    pstFileOper = GetFileOperClass(pstFileDev);

    if(pstFileOper == NULL)
    {
        return NULL;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       pstFileOper->hOper = FatDev_OpenDir(pstVolume->hVolume,((FILE_OPER_CLASS *)hFather)->hOper, pstFileAttr);
       if((rk_err_t) pstFileOper->hOper <= 0)
       {
            pstFileOper->hOper = 0;
            return (HDC)RK_ERROR;
       }

       pstFileOper->pstVolume = pstVolume;

       return pstFileOper;
    }

    return (HDC)RK_ERROR;

}

/*******************************************************************************
** Name: FileDev_GetFileName
** Input:HDC dev,  uint16 * FileName
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:21:17
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_GetFileName(HDC dev,  uint16 * FileName)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_GetFileName(pstFileOper->hOper, FileName);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_NextFile
** Input:(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr)
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:20:23
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_NextFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
        return FatDev_NextFile(pstFileOper->hOper,FileMaskAttr, ExtName, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_PrevFile
** Input:(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr)
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:19:58
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_PrevFile(HDC dev, uint8 FileMaskAttr, uint8 * ExtName, FILE_ATTR * pstFileAttr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_PrevFile(pstFileOper->hOper,FileMaskAttr, ExtName, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_NextFile
** Input:HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:18:35
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_NextDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_NextDir(pstFileOper->hOper,DirMaskAttr, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_PrevFile
** Input:HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:17:50
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_PrevDir(HDC dev, uint8 DirMaskAttr, FILE_ATTR * pstFileAttr)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_PrevDir(pstFileOper->hOper,DirMaskAttr, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_DeleteFile
** Input:HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:36:24
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_DeleteFile(HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileDev == NULL)
    {
        return RK_ERROR;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_DeleteFile(pstVolume->hVolume, hFather, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_DeleteDir
** Input:HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:35:48
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_DeleteDir(HDC dev,  HDC hFather, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileDev == NULL)
    {
        return RK_ERROR;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_DeleteDir(pstVolume->hVolume, hFather, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_CreateFile
** Input:HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:22:44
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_CreateFile(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileDev == NULL)
    {
        return RK_ERROR;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_CreateFile(pstVolume->hVolume, hFather, pstFileAttr);
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: FileDev_CreateDir
** Input:HDC dev, HDC hFather, uint32 Mode, FILE_ATTR * pstFileAttr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.22
** Time: 14:22:18
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON API rk_err_t FileDev_CreateDir(HDC dev, HDC hFather, FILE_ATTR * pstFileAttr)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    VOLUME_CLASS * pstVolume;

    if(pstFileDev == NULL)
    {
        return RK_ERROR;
    }

    if(hFather != NULL)
    {
        pstVolume = ((FILE_OPER_CLASS *)hFather)->pstVolume;
    }
    else
    {
        pstVolume = &pstFileDev->stVolume[pstFileAttr->Path[0] - 'A'];
    }

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
       return FatDev_CreateDir(pstVolume->hVolume, hFather, pstFileAttr);
    }

    return RK_ERROR;
}


/*******************************************************************************
** Name: FileDevWrite
** Input:HDC dev, uint32 pos, const void* buffer, uint32 size,uint8 mode,pTx_complete Tx_complete
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON FUN rk_err_t FileDev_WriteFile(HDC dev, uint8* buffer, uint32 len)
{
    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    uint32 ret;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
        ret = FatDev_WriteFile(pstFileOper->hOper, buffer, len);

        if(ret > 0)
        {
            pstFileOper->CurOffset += ret;
            pstFileOper->FileSize += len;
        }
        return ret;
    }

    return RK_ERROR;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FileDevCreate
** Input:void * Args
** Return: HDC
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_INIT_
INIT API HDC FileDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* pstDev;
    FILE_DEVICE_CLASS * pstFileDev;


    pstFileDev =  rkos_memory_malloc(sizeof(FILE_DEVICE_CLASS));
    if (pstFileDev == NULL)
    {
        return pstFileDev;
    }

    memset((uint8 *)pstFileDev, 0, sizeof(FILE_DEVICE_CLASS));

    pstDev = (DEVICE_CLASS *)pstFileDev;
    pstDev->suspend = FileDevSuspend;
    pstDev->resume  = FileDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_FILE_DEV, SEGMENT_OVERLAY_ALL);
    #endif

    gpstFileDevInf[DevID] = pstFileDev;

    return pstDev;
}


/*******************************************************************************
** Name: FileDevDelete
** Input:HDC dev
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_INIT_
INIT API rk_err_t FileDevDelete(uint32 DevID, void * arg)
{
    FILE_DEVICE_CLASS * pstFileDev =  gpstFileDevInf[DevID];

    rkos_memory_free(pstFileDev);


    gpstFileDevInf[DevID] = NULL;


    #ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_FILE_DEV, SEGMENT_OVERLAY_ALL);
    #endif

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
** Name: FileDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 11:10:25
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON FUN rk_err_t FileDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < FILE_DEV_NUM_MAX; i++)
    {
        if(gpstFileDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}

/*******************************************************************************
** Name: GetFileOperClass
** Input:HDC dev
** Return: FILE_OPER_CLASS *
** Owner:Aaron.sun
** Date: 2014.4.8
** Time: 9:10:25
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON FUN FILE_OPER_CLASS * GetFileOperClass(FILE_DEVICE_CLASS * pstFileDev)
{
    uint32 i;

    for (i = 0; i < MAX_FILE_HANDLE; i++)
    {
        if (pstFileDev->stFileOper[i].hOper == 0)
        {
            return &pstFileDev->stFileOper[i];
        }
    }

    return NULL;

}

/*******************************************************************************
** Name: FileDevRead
** Input:HDC dev,uint32 pos, void* buffer, uint32 size,uint8 mode,pRx_indicate Rx_indicate
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_COMMON_
COMMON FUN rk_err_t FileDev_ReadFile(HDC dev, uint8* buffer, uint32 len)
{

    FILE_OPER_CLASS * pstFileOper = (FILE_OPER_CLASS *)dev;
    VOLUME_CLASS * pstVolume;
    rk_err_t ret;

    if(pstFileOper == NULL)
    {
        return RK_ERROR;
    }

    pstVolume = pstFileOper->pstVolume;

    if(pstVolume->VolumeType == VOLUME_TYPE_FAT)
    {
        ret = FatDev_ReadFile(pstFileOper->hOper, buffer, len);
        if(ret > 0)
        {
            pstFileOper->CurOffset += ret;
        }

        return ret;
    }

    return RK_ERROR;
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: FileDevSuspend
** Input:HDC dev, uint32 Level
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 11:09:47
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_INIT_
INIT FUN rk_err_t FileDevSuspend(HDC dev, uint32 Level)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    if(FileDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstFileDev->stFileDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstFileDev->stFileDevice.State = DEV_SATE_IDLE2;
    }

    for(i = 0; i < VOLUME_NUM_MAX; i++)
    {
        if(pstFileDev->stVolume[i].VolumeType != 0)
        {
            RKDev_Suspend(pstFileDev->stVolume[i].hVolume);
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FileDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 11:08:29
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_INIT_
INIT FUN rk_err_t FileDevResume(HDC dev)
{
    FILE_DEVICE_CLASS * pstFileDev = (FILE_DEVICE_CLASS *)dev;
    uint32 i;

    if(FileDevCheckHandler(dev) == RK_ERROR)
    {
        return RK_ERROR;
    }

    for(i = 0; i < VOLUME_NUM_MAX; i++)
    {
        if(pstFileDev->stVolume[i].VolumeType != 0)
        {
            RKDev_Resume(pstFileDev->stVolume[i].hVolume);
        }
    }

    pstFileDev->stFileDevice.State = DEV_STATE_WORKING;

    return RK_SUCCESS;
}




/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#ifdef _FILE_SHELL_
_DRIVER_FILE_FILEDEVICE_DATA_
static SHELL_CMD ShellFileName[] =
{
    "pcb",FileShellPcb,"display file device pcb information","file.pcb [objectid]\n",
    "create",FileShellCreate,"create a file device","file.create [/objectid]\n",
    "delete",FileShellDelete,"delete a file device","file.delete [/objectid]\n",
    "test",FileShellTest,"test file system","file.test [/objectid]\ncreate a file in root dir and write some data\n",
    "setpath",FileShellSetPath,"set current path","file.setpath <path>\nnotice current path is only one\n",
    "ls",FileShellLs,"list current dir all file and subdir","file.ls [/objectid] [</f>|</d>] \nonly support shortname\nf",
    "mf",FileShellMf,"create a file in current dir","file.mf <filename> [/objectid]\n",
    "md",FileShellMd,"create a subdir in current dir","file.md [/objectid] <filename>\n",
    "df",FileShellDf,"delete a file in current dir","file.df [/objectid] <filename>\n",
    "dd",FileShellDd,"delete a subdir in current dir","file.md [/objectid] <filename>\n",
    "\b",NULL,"NULL","NULL",
};

_DRIVER_FILE_FILEDEVICE_DATA_
uint16 CurPath[MAX_DIRPATH_LEN];


/*******************************************************************************
** Name: FileShell
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL API rk_err_t FileDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;

    uint8 Space;


    if(ShellHelpSampleDesDisplay(dev, ShellFileName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr,&pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellFileName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;
    pItem += StrCnt;
    pItem++;                                            //remove '.',the point is the useful item

    ShellHelpDesDisplay(dev, ShellFileName[i].CmdDes, pItem);
    if(ShellFileName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellFileName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: FileShellDd
** Input:HDC dev,uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.16
** Time: 17:13:02
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellDd(HDC dev,uint8 * pstr)
{
    FILE_ATTR stFileAttr;
    uint16 FileName[255];
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*CurPath> 'Z')
        *CurPath -= ('a' - 'A');

    stFileAttr.Path = CurPath;
    FileName[Ascii2Unicode(pstr, FileName, strlen(pstr)) / 2 ] = 0;

    stFileAttr.FileName = FileName;

    ret = FileDev_DeleteDir(FileSysHDC, NULL, &stFileAttr);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("dir delete failure");
        return RK_SUCCESS;
    }
    return RK_SUCCESS;

}


/*******************************************************************************
** Name: FileShellDf
** Input:HDC dev,uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.16
** Time: 14:57:29
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellDf(HDC dev,uint8 * pstr)
{
    FILE_ATTR stFileAttr;
    uint16 FileName[255];
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(*CurPath> 'Z')
        *CurPath -= ('a' - 'A');

    stFileAttr.Path = CurPath;
    FileName[Ascii2Unicode(pstr, FileName, strlen(pstr)) / 2 ] = 0;

    stFileAttr.FileName = FileName;

    ret = FileDev_DeleteFile(FileSysHDC, NULL, &stFileAttr);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("file delete failure");
        return RK_SUCCESS;
    }
    return RK_SUCCESS;

}


/*******************************************************************************
** Name: FileShellSetPath
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.20
** Time: 17:39:23
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellSetPath(HDC dev, uint8 * pstr)
{
    uint32 len;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    len = Ascii2Unicode(pstr, CurPath, strlen(pstr)) / 2;
    CurPath[len] = 0;
    if(CurPath[len - 1] != '\\')
    {
        CurPath[len] = '\\';
        CurPath[len + 1] = 0;
    }

    if(*CurPath> 'Z')
        *CurPath -= ('a' - 'A');

    rk_printf("curpath:%s",pstr);



    return RK_SUCCESS;


}
/*******************************************************************************
** Name: FileShellMf
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.7
** Time: 10:20:11
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellMf(HDC dev, uint8 * pstr)
{
    FILE_ATTR stFileAttr;
    uint16 FileName[255];
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    stFileAttr.Path = CurPath;
    FileName[Ascii2Unicode(pstr, FileName, strlen(pstr)) / 2 ] = 0;

    stFileAttr.FileName = FileName;

    ret = FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("file create failure");
        return RK_SUCCESS;
    }
    return RK_SUCCESS;

}

/*******************************************************************************
** Name: FileShellMd
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.1.7
** Time: 10:19:47
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellMd(HDC dev, uint8 * pstr)
{
    FILE_ATTR stFileAttr;
    uint16 FileName[255];
    rk_err_t ret;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    stFileAttr.Path = CurPath;
    FileName[Ascii2Unicode(pstr, FileName, strlen(pstr)) / 2 ] = 0;

    stFileAttr.FileName = FileName;

    ret = FileDev_CreateDir(FileSysHDC, NULL, &stFileAttr);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("dir create failure");
        return RK_SUCCESS;
    }
    return RK_SUCCESS;

}


/*******************************************************************************
** Name: FileShellTest
** Input:HDC dev, uint8 * pItem
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.6.9
** Time: 9:21:06
*******************************************************************************/
static int index = 0;
uint8 testfilename1[20];
uint16 testfilename[20];

_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellTest(HDC dev, uint8 * pstr)
{
    HDC hFile;
    FILE_ATTR stFileAttr;
    uint32 TotalSec;
    uint8 * pBuf;
    int32 BufSec;
    uint32 i, j;
    rk_err_t ret;
    uint32 secsize;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    testfilename1[snprintf(testfilename1, 20, "%d.mp3", index++)] = 0;

    i = 0;
    while(testfilename1[i] != 0)
    {
        testfilename[i] = testfilename1[i];
        i++;
    }

    testfilename[i] = 0;


    stFileAttr.Path = L"C:\\";
    stFileAttr.FileName = testfilename;

    printf("index = %d\r\n",index);

    hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);

    if((rk_err_t)hFile <= 0)
    {
        if(FileDev_CreateFile(FileSysHDC, NULL, &stFileAttr) == RK_SUCCESS)
        {
            hFile = FileDev_OpenFile(FileSysHDC, NULL, READ_WRITE, &stFileAttr);
        }

        if((int)hFile <= 0)
        {
            printf("create file fail\r\n");
            return RK_ERROR;
        }
    }

    secsize = 600;

    for(BufSec = 3; BufSec > 0; BufSec--)
    {
        pBuf = rkos_memory_malloc(secsize * BufSec);

        if(pBuf != NULL)
        {
            break;
        }
    }

    if(BufSec <= 0)
    {
        return RK_ERROR;
    }

    TotalSec = 0x80;

    rk_printf("test start = %d\n", BufSec);

    for(i = 1;  i <= (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * secsize; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];


        ret = FileDev_WriteFile(hFile, pBuf, BufSec * secsize);

     if(i % 50 == 0)
            rk_printf("write: LBA = 0x%08x, Len = %d", i - 1, BufSec);

        if(ret != (BufSec * secsize))
        {
            rk_print_string("file write error");
            return RK_SUCCESS;
        }
    }

    if(TotalSec % BufSec != 0)
    {

        pBuf[0] = i % 256;
        pBuf[1] = i % 256;

        for(j = 2; j < BufSec * secsize; j++)
            pBuf[j] = pBuf[j - 1] + pBuf[j - 2];



        ret = FileDev_WriteFile(hFile,pBuf, (TotalSec % BufSec) * secsize);

        rk_printf("write: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if(ret != ((TotalSec % BufSec) * secsize))
        {
            rk_print_string("file write error");
            return RK_SUCCESS;
        }
    }

    FileDev_FileSeek(hFile, 0, 0);

    for(i = 1;  i <= (TotalSec / BufSec) * BufSec; i += BufSec)
    {

        ret = FileDev_ReadFile(hFile, pBuf, BufSec * secsize);

       if(i % 50 == 0)
        rk_printf("read: LBA = 0x%08x, Len = %d", i - 1, BufSec);

        if(ret != (BufSec * secsize))
        {
            rk_print_string("file read error");
            return RK_SUCCESS;
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_print_string("file data error");
             return RK_SUCCESS;
        }


        for(j = 2; j < BufSec * secsize; j++)
        {
            if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                 rk_print_string("file data error");
                 return RK_SUCCESS;
            }
        }

    }

    if(TotalSec % BufSec != 0)
    {

        ret = FileDev_ReadFile(hFile, pBuf, (TotalSec % BufSec) * secsize);

        rk_printf("read: LBA = 0x%08x, Len = %d", i, TotalSec % BufSec);

        if(ret != (TotalSec % BufSec) * secsize)
        {
            rk_print_string("file read error");
        }

        if((pBuf[0] != i % 256) || (pBuf[1] != i % 256))
        {
             rk_print_string("file data error");
             return RK_SUCCESS;
        }


        for(j = 2; j < (TotalSec % BufSec) * secsize; j++)
        {
            if(pBuf[j] != (uint8)(pBuf[j - 1] + pBuf[j - 2]))
            {
                 rk_print_string("file data error");
                 return RK_SUCCESS;
            }
        }

    }

    rk_printf("test end");

    rkos_memory_free(pBuf);

    FileDev_CloseFile(hFile);
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FileShellDelete
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.23
** Time: 15:42:26
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellDelete(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: FileShellMake
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.23
** Time: 15:42:03
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellCreate(HDC dev, uint8 * pstr)
{
    rk_err_t ret;
    HDC hFileDev;
    VOLUME_INF stVolumeInf;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    ret = RKDev_Create(DEV_CLASS_FILE, 0, NULL);

    if (ret != RK_SUCCESS)
    {
        rk_print_string("file create failure");
        return RK_SUCCESS;
    }

    hFileDev = RKDev_Open(DEV_CLASS_FILE, 0, NOT_CARE);

    if ((hFileDev == NULL) || (hFileDev == (HDC)RK_ERROR) || (hFileDev == (HDC)RK_PARA_ERR))
    {
        rk_print_string("file open failure");
        return RK_SUCCESS;
    }

    stVolumeInf.hVolume = RKDev_Open(DEV_CLASS_FAT, 0, NOT_CARE);

    stVolumeInf.VolumeID = 'C';

    stVolumeInf.VolumeType = VOLUME_TYPE_FAT32;

    ret = FileDev_AddVolume(hFileDev, &stVolumeInf);
    if (ret != RK_SUCCESS)
    {
        rk_print_string("add volume failure");
        return RK_ERROR;
    }

    RKDev_Close(hFileDev);

    return ret;

}

/*******************************************************************************
** Name: FileShellLs
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.4.23
** Time: 15:41:40
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellLs(HDC dev, uint8 * pstr)
{
    HDC hDir;
    FILE_ATTR stFileAttr;
    uint8 Buf[512];
    uint32 TotalFile, TotalDir,BufLen;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    #if 0
    GuiTask_UnloadResource();

    DeviceTask_RemoveFs(DEVICE_LIST_SPI_FS1, SYNC_MODE);

    DelayMs(1000);

    DeviceTask_LoadFs(DEVICE_LIST_SPI_FS1, SYNC_MODE);

    GuiTask_LoadResource();
    #endif

    stFileAttr.Path = CurPath;
    stFileAttr.FileName = NULL;

    hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &stFileAttr);
    if ((rk_err_t)hDir <= 0)
    {
        rk_print_string("dir open failure");
        return RK_SUCCESS;
    }

    rk_print_string("\ndir and file as Follow:");

    TotalDir = 0;

    BufLen = 0;
    while(1)
    {
        rk_print_string("\r\n");

        if (FileDev_NextDir(hDir, 0, &stFileAttr) != RK_SUCCESS)
        {
            break;
        }


        BufLen += DateToString(stFileAttr.CrtDate, Buf + BufLen);
        Buf[BufLen] = ' ';
        BufLen++;
        BufLen += TimeToString(stFileAttr.CrtTime, stFileAttr.CrtTimeTenth, Buf + BufLen);
        Buf[BufLen] = ' ';
        BufLen++;

        memcpy(Buf + BufLen, "<DIR>", 5);

        BufLen += 5;

        Buf[BufLen] = ' ';
        BufLen++;

        memcpy(Buf+ BufLen, stFileAttr.ShortName, 8);
        BufLen += 8;
        *(Buf + BufLen) = '.';
        BufLen += 1;
        memcpy(Buf+ BufLen, stFileAttr.ShortName + 8, 3);
        BufLen += 3;
        UartDev_Write(dev, Buf, BufLen, SYNC_MODE, NULL);

        TotalDir++;
        BufLen = 0;

    }

    FileDev_CloseDir(hDir);

    rk_print_string("\r\n");

    stFileAttr.Path = CurPath;
    stFileAttr.FileName = NULL;

    hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &stFileAttr);
    if ((rk_err_t)hDir <= 0)
    {
        rk_print_string("dir open failure");
        return RK_ERROR;
    }

    TotalFile = 0;

    BufLen = 0;
    while(1)
    {
        if (FileDev_NextFile(hDir, 0, NULL, &stFileAttr) != RK_SUCCESS)
        {
            break;
        }

        BufLen += DateToString(stFileAttr.CrtDate, Buf + BufLen);
        Buf[BufLen] = ' ';
        BufLen++;
        BufLen += TimeToString(stFileAttr.CrtTime, stFileAttr.CrtTimeTenth, Buf + BufLen);
        Buf[BufLen] = ' ';
        BufLen++;

        memcpy(Buf + BufLen, "<FILE>", 6);

        BufLen += 6;

        Buf[BufLen] = ' ';
        BufLen++;

        memcpy(Buf+ BufLen, stFileAttr.ShortName, 8);
        BufLen += 8;
        *(Buf + BufLen) = '.';
        BufLen += 1;
        memcpy(Buf+ BufLen, stFileAttr.ShortName + 8, 3);
        BufLen += 3;
        UartDev_Write(dev, Buf, BufLen, SYNC_MODE, NULL);

        rk_print_string("\r\n");

        TotalFile++;
        BufLen = 0;

    }

    BufLen = sprintf(Buf, "   total dir %d, totoal file %d", TotalDir, TotalFile);
    UartDev_Write(dev, Buf, BufLen, SYNC_MODE, NULL);

    FileDev_CloseDir(hDir);

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: FileShellPcb
** Input:HDC dev, const uint8 * pstr
** Return: rk_err_t
** Owner:Aaron.sun
** Date: 2014.3.31
** Time: 18:06:38
*******************************************************************************/
_DRIVER_FILE_FILEDEVICE_SHELL_
SHELL FUN rk_err_t FileShellPcb(HDC dev,  uint8 * pstr)
{
    uint32 DevID;
    FILE_DEVICE_CLASS * pstFileDev;
    uint32 i;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    DevID = String2Num(pstr);

    if(DevID >= FILE_DEV_NUM_MAX)
    {
        return RK_ERROR;
    }

    pstFileDev = gpstFileDevInf[DevID];

    if(pstFileDev == NULL)
    {
        rk_printf("filedev%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstFileDevInf[%d]", DevID);
    rk_printf_no_time("    .stFileDevice");
    rk_printf_no_time("        .next = %08x",pstFileDev->stFileDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstFileDev->stFileDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstFileDev->stFileDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstFileDev->stFileDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstFileDev->stFileDevice.DevID);
    rk_printf_no_time("        .State = %d",pstFileDev->stFileDevice.State);
    rk_printf_no_time("        .suspend = %08x",pstFileDev->stFileDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstFileDev->stFileDevice.resume);

    for(i = 0; i < VOLUME_NUM_MAX; i++)
    {
        rk_printf_no_time("    .stVolume[%d]", i);
        rk_printf_no_time("        .VolumeType = %d", pstFileDev->stVolume[i].VolumeType);
        rk_printf_no_time("        .hVolume = %08x", pstFileDev->stVolume[i].hVolume);
    }

    for(i = 0; i < MAX_FILE_HANDLE; i++)
    {
        rk_printf_no_time("    .stFileOper[%d] = %08x", i, &(pstFileDev->stFileOper[i]));
        rk_printf_no_time("        .pstVolume = %08x", pstFileDev->stFileOper[i].pstVolume);
        rk_printf_no_time("        .hOper = %08x", pstFileDev->stFileOper[i].hOper);
        rk_printf_no_time("        .CurOffset = %d", pstFileDev->stFileOper[i].CurOffset);
        rk_printf_no_time("        .FileSize = %d", pstFileDev->stFileOper[i].FileSize);
    }

    return RK_SUCCESS;

}



#endif
#endif

