/*
********************************************************************************************
*
*        Copyright (c):Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\File\DirDevice.c
* Owner: aaron.sun
* Date: 2015.6.23
* Time: 13:54:25
* Version: 1.0
* Desc: dir device class
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.6.23     13:54:25   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_FILE_DIRDEVICE_C__

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
#include "DirDevice.h"
#include "FileDevice.h"
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
#define DIR_DEV_NUM 2

typedef struct _SUB_DIR
{
    HDC hDir;
    uint32 TotalFile;
    uint32 TotalSubDir;
    uint16 CurDirNum;
    uint32 DirNum;
    uint32 CurFileNum;

} SUB_DIR;

typedef struct _DIR_DEVICE_CLASS
{
    DEVICE_CLASS stDirDevice;
    pSemaphore osDirOperReqSem;
    uint16 path[MAX_DIRPATH_LEN];
    uint8 * ExtName;
    uint32 TotalFile;
    uint32 TotalDir;
    uint32 TotalAllDir;
    uint32 pathlen;
    uint32 CurDirDeep;
    SUB_DIR SubDirInfo[MAX_DIR_DEPTH];
    HDC hDir;
    uint32 CurFileNum;


}DIR_DEVICE_CLASS;



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static DIR_DEVICE_CLASS * gpstDirDevInf[DIR_DEV_NUM] = {(DIR_DEVICE_CLASS *)NULL,(DIR_DEVICE_CLASS *)NULL};



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
//char        AudioFileExtString[]   = "MP1MP2MP3WMAWAVAPELACAACM4AOGGMP43GPSBC";
//char        RecordFileExtString[]  = "WAV";


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
rk_err_t DirDevCheckHandler(HDC dev);
rk_err_t DirDevShellDel(HDC dev, uint8 * pstr);
HDC ChangeDir(HDC hFatherDir);
rk_err_t DirDevShelllist(HDC dev, uint8 * pstr);
rk_err_t DirDevShellDelete(HDC dev, uint8 * pstr);
rk_err_t DirDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t DirDevShellPcb(HDC dev, uint8 * pstr);
void DirDevIntIsr1(void);
void DirDevIntIsr0(void);
void DirDevIntIsr(uint32 DevID);
rk_err_t DirDevDeInit(DIR_DEVICE_CLASS * pstDirDev);
rk_err_t DirDevInit(DIR_DEVICE_CLASS * pstDirDev);
rk_err_t DirDevResume(HDC dev);
rk_err_t DirDevSuspend(HDC dev, uint32 Level);



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DirDev_DeleteDir
** Input:HDC dev, uint16 * path
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.17
** Time: 9:35:46
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_DeleteDir(HDC dev, uint16 * path)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    FILE_ATTR stFileAttr;
    uint32 TotalDir, TotalDeleFile, TotalFile;
    uint32 dir;
    uint16 CurPath[MAX_DIRPATH_LEN];
    uint16 PathLen;
    uint16 * pPath;


    TotalDir = 0;
    TotalDeleFile = 0;


    if(DirDev_BuildDirInfo(dev, NULL, path) != RK_SUCCESS)
    {
        printf("1");
        return RK_ERROR;
    }

    do
    {
        uint32 Index;

        Index = 0;

        TotalFile = pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile;

        rk_printf("Deep = %d, totalfile = %d, totaldir = %d", pstDirDev->CurDirDeep, TotalFile, pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir);

        if(TotalFile)
        {

            if(DirDev_FindFirst(dev, &dir) != RK_SUCCESS)
            {
                printf("2");
                return RK_ERROR;
            }

            do
            {
                if(dir == 0)
                {
                    break;
                }

            }while(DirDev_FindNext(dev, &dir) == RK_SUCCESS);

            if(dir)
            {
                printf("3");
                return RK_ERROR;
            }

            do
            {
                if(dir)
                {
                    printf("4");
                    return RK_ERROR;
                }

                DirDev_GetCurPath(dev, CurPath);

                printf("\n1");
                //debug_hex((uint8 *)CurPath, 64, 16);

                stFileAttr.Path = CurPath;
                stFileAttr.FileName = NULL;
                if(FileDev_DeleteFile(FileSysHDC, NULL, &stFileAttr) != RK_SUCCESS)
                {
                    printf("5");
                    return RK_ERROR;
                }

                TotalFile--;
                TotalDeleFile++;

                if(TotalFile == 0)
                {
                    break;
                }

            }while(DirDev_FindNext(dev, &dir) == RK_SUCCESS);            //找到目录项

            if(TotalFile)
            {
                printf("6");
                return RK_ERROR;
            }

        }

        if(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir == 0)    //该目录下没有目录即为叶结点,要找父目录或同级目录
        {

            while(1)
            {
                FileDev_CloseDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir);

                pstDirDev->path[pstDirDev->pathlen - 1] = 0;

                stFileAttr.Path = pstDirDev->path;
                stFileAttr.FileName = NULL;

                printf("\n3");
                debug_hex((uint8 *)pstDirDev->path, 512, 16);


                if(FileDev_DeleteDir(FileSysHDC, NULL, &stFileAttr) != RK_SUCCESS)
                {
                    printf("8");
                    return RK_ERROR;
                }

                TotalDir++;

                pstDirDev->path[pstDirDev->pathlen - 1] = '\\';

                pstDirDev->CurDirDeep--; //即指向上一级目录

                pPath = (uint16 *)pstDirDev->path;
                PathLen = pstDirDev->pathlen;

                pPath += PathLen;


                PathLen--;
                pPath--;

                do
                {
                    PathLen--;
                    pPath--;
                }while(*pPath != '\\');

                PathLen++;
                pPath++;

                *pPath = 0;
                pstDirDev->pathlen = PathLen;

                if((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum + 1)< pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir)
                {
                    rk_printf("find next dir");
                    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum++;
                    pstDirDev->CurDirDeep++;
                    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum = 0;
                    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
                    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].DirNum = pstDirDev->TotalAllDir;
                    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir = ChangeDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir);
                    break;
                }

                if (pstDirDev->CurDirDeep == 0)                    //找到根目录了不能再往上找
                {
                    printf("\n2");
                    debug_hex((uint8 *)path, 64, 16);

                    stFileAttr.Path = path;
                    stFileAttr.FileName = NULL;
                    if(FileDev_DeleteDir(FileSysHDC, NULL, &stFileAttr) != RK_SUCCESS)
                    {
                        printf("7");
                        return RK_ERROR;
                    }

                    TotalDir++;

                    goto END;
                }


            }

        }
        else if(pstDirDev->CurDirDeep == (MAX_DIR_DEPTH - 1))
        {
              printf("path too deep\n");
              return RETURN_FAIL;
        }
        else            //该目录下还有子目录,要找它的第一个子目录
        {
            pstDirDev->CurDirDeep++;
            pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum = 0;
            pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
            pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].DirNum = pstDirDev->TotalAllDir;
            pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir = ChangeDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir);
        }

        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir = FileDev_GetTotalDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir, NULL, NULL);//fileInfo->pExtStr
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile = FileDev_GetTotalFile(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir, pstDirDev->ExtName, NULL);//fileInfo->pExtStr
        pstDirDev->TotalAllDir += pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir;
        FileDev_GetFileName(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir, pstDirDev->path + pstDirDev->pathlen);
        pstDirDev->pathlen += StrLenW(pstDirDev->path + pstDirDev->pathlen);
        pstDirDev->path[pstDirDev->pathlen++] = '\\';
        pstDirDev->path[pstDirDev->pathlen] = 0;

        if(pstDirDev->hDir != NULL)
        {
            FileDev_CloseDir(pstDirDev->hDir);
            pstDirDev->hDir = NULL;
        }

 END:

    }while(pstDirDev->CurDirDeep != 0);


    printf("deletedir totoaldir = %d, totalfile = %d\n", TotalDir, TotalDeleFile);

    return RETURN_OK;


}


/*******************************************************************************
** Name: DirDev_GetFatherDirNum
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 17:03:07
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetFatherDirNum(HDC dev)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    return pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].DirNum + pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].CurDirNum;
}
/*******************************************************************************
** Name: DirDev_OpenCurFile
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 13:45:40
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API HDC DirDev_OpenCurFile(HDC dev)
{
    FILE_ATTR pstFileAttr;
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;

    pstFileAttr.Path = NULL;
    pstFileAttr.FileName = NULL;

    return FileDev_OpenFile(FileSysHDC, pstDirDev->hDir, NOT_CARE, &pstFileAttr);

}

/*******************************************************************************
** Name: DirDev_GetCurPath
** Input:HDC dev, uint16 * path
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 13:45:04
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetCurPath(HDC dev, uint16 * path)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;

    memcpy(path, pstDirDev->path, pstDirDev->pathlen * 2 + 2);

    if(pstDirDev->hDir != NULL)
    {
        FileDev_GetFileName(pstDirDev->hDir, path + pstDirDev->pathlen);
    }
}
/*******************************************************************************
** Name: DirDev_FindNext
** Input:HDC dev, SEARCH_TYPE type
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 13:43:43
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_FindNext(HDC dev, uint32 *dir)
{

     DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;

     if((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum + 1) >= (pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile
        + pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir))
     {
        return RK_ERROR;
     }

     if((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum + 1) < pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir)
     {
        FileDev_NextDir(pstDirDev->hDir, NULL, NULL);
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum++;
        *dir = 1;
     }
     else if((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum + 1) == pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir)
     {
        if(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile == 0)
        {
            return RK_ERROR;
        }
        FileDev_GetFirstPos(pstDirDev->hDir);
        FileDev_NextFile(pstDirDev->hDir, NULL, pstDirDev->ExtName, NULL);
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum++;
        *dir = 0;

     }
     else
     {
        FileDev_NextFile(pstDirDev->hDir, NULL,pstDirDev->ExtName, NULL);
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum++;
        *dir = 0;
     }

     return RK_SUCCESS;
}
/*******************************************************************************
** Name: DirDev_FindFirst
** Input:HDC dev,  SEARCH_TYPE type
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 13:42:56
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_FindFirst(HDC dev, uint32 *dir)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    FILE_ATTR pstFileAttr;

    if((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir == 0) &&
        (pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile == 0))
    {
        return RK_ERROR;
    }


    if(pstDirDev->CurDirDeep != 0)
    {
        pstFileAttr.Path = NULL;
        pstFileAttr.FileName = NULL;

        pstDirDev->hDir = FileDev_OpenDir(FileSysHDC, pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir, NOT_CARE, &pstFileAttr);
    }
    else
    {
        pstFileAttr.Path = pstDirDev->path;
        pstFileAttr.FileName = NULL;

        pstDirDev->hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &pstFileAttr);//根目录
        if((int)pstDirDev->hDir <= 0)
        {
            rk_printf("path open fail2");
            return RK_ERROR;
        }
    }

    if((int32)pstDirDev->hDir <= 0)
    {
        rk_printf("find first fail");
        return RK_ERROR;
    }

    if(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir != 0)
    {
        FileDev_NextDir(pstDirDev->hDir, NULL, NULL);
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
        *dir = 1;
    }
    else
    {
        FileDev_NextFile(pstDirDev->hDir, NULL, pstDirDev->ExtName, NULL);
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
        *dir = 0;
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: DirDev_GotoNextDir
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.4.19
** Time: 9:39:24
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GotoNextDir(HDC dev)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    uint16 PathLen;
    uint16 * pPath;

    if ((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir == 0) || (pstDirDev->CurDirDeep == (MAX_DIR_DEPTH - 1)))    //该目录下没有目录即为叶结点,要找父目录或同级目录
    {
        if(pstDirDev->CurDirDeep == (MAX_DIR_DEPTH - 1))
        {
            rk_printf("dir too deep");
            while(1);
        }

        while (1)
        {
            if (pstDirDev->CurDirDeep == 0)                    //找到根目录了不能再往上找
            {
                pstDirDev->SubDirInfo[0].CurDirNum = 0;
                pstDirDev->SubDirInfo[0].DirNum = 0;
                pstDirDev->SubDirInfo[0].CurFileNum = 0;
                FileDev_GetFirstPos(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir);

                if(pstDirDev->hDir != NULL)
                {
                    FileDev_CloseDir(pstDirDev->hDir);
                    pstDirDev->hDir = NULL;
                }

                return RK_ERROR;
            }

            pPath = (uint16 *)pstDirDev->path;
            PathLen = pstDirDev->pathlen;

            pPath += PathLen;


            PathLen--;
            pPath--;

            do
            {
                PathLen--;
                pPath--;
            }while(*pPath != '\\');

            PathLen++;
            pPath++;

            *pPath = 0;
            pstDirDev->pathlen = PathLen;


            FileDev_CloseDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir);

            pstDirDev->CurDirDeep--;                       //即指向上一级目录

            //rk_printf("pstDirDev->CurDirDeep = %d",pstDirDev->CurDirDeep);

            if ((pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum + 1) < pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir)
            {
                //rk_printf("enter next dir");
                pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum++;
                pstDirDev->CurDirDeep++;
                pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum = 0;
                pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
                pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].DirNum = pstDirDev->TotalAllDir;
                pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir = ChangeDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir);
                break;
            }
        }
    }
    else            //该目录下还有子目录,要找它的第一个子目录
    {
        pstDirDev->CurDirDeep++;
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurDirNum = 0;
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = 0;
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].DirNum = pstDirDev->TotalAllDir;
        pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir = ChangeDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir);
    }

    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir = FileDev_GetTotalDir(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir, NULL, NULL);//fileInfo->pExtStr
    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile = FileDev_GetTotalFile(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].hDir, pstDirDev->ExtName, NULL);//fileInfo->pExtStr
    pstDirDev->TotalAllDir += pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir;
    FileDev_GetFileName(pstDirDev->SubDirInfo[pstDirDev->CurDirDeep - 1].hDir, pstDirDev->path + pstDirDev->pathlen);
    pstDirDev->pathlen += StrLenW(pstDirDev->path + pstDirDev->pathlen);
    pstDirDev->path[pstDirDev->pathlen++] = '\\';
    pstDirDev->path[pstDirDev->pathlen] = 0;

    if(pstDirDev->hDir != NULL)
    {
        FileDev_CloseDir(pstDirDev->hDir);
        pstDirDev->hDir = NULL;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDev_GetCurDirTotalFile
** Input:HDC dev, uint32 * totalfile
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:02:25
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetCurDirTotalFile(HDC dev, uint32 * totalfile)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    *totalfile = pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDev_GetCurDirTotalFile
** Input:HDC dev, uint32 * totalfile
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:02:25
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetCurDirTotalSubDir(HDC dev, uint32 * totalsubdir)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    *totalsubdir = pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalSubDir;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDev_GetTotalFile
** Input:HDC dev, uint32 * totalfile
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:01:17
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetTotalFile(HDC dev, uint32 * totalfile)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    *totalfile = pstDirDev->TotalFile;
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DirDev_GetTotalFile
** Input:HDC dev, uint32 * totalfile
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 14:01:17
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetTotalSubDir(HDC dev, uint32 * totalsubdir)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    *totalsubdir = pstDirDev->TotalDir;
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDev_GetFilePathByGlobalNum
** Input:HDC dev, uint16 * path, uint32 curnum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 13:58:08
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetFilePathByGlobalNum(HDC dev, uint16 * path, uint32 curnum)
{

}

/*******************************************************************************
** Name: DirDev_GetFilePathByCurNum
** Input:HDC dev, uint16 * path, uint32 curnum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.11.3
** Time: 13:56:02
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_GetFilePathByCurNum(HDC dev, uint16 * path, uint32 curnum)
{
    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    uint32 bCurNum;
    uint32 i;
    FILE_ATTR stFileAttr;

    if(pstDirDev == NULL)
    {
        return RK_PARA_ERR;
    }

    if(pstDirDev->pathlen == 0)
    {
        return RK_ERROR;
    }

    if(pstDirDev->hDir == NULL)
    {
        stFileAttr.Path = pstDirDev->path;
        stFileAttr.FileName = NULL;

        pstDirDev->hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &stFileAttr);//根目录
        if((int)pstDirDev->hDir <= 0)
        {
            rk_printf("path open fail");
            return RK_ERROR;
        }

        FileDev_NextFile(pstDirDev->hDir, NULL, pstDirDev->ExtName, &stFileAttr);
    }

    if(curnum >= pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].TotalFile)
    {
        return RK_ERROR;
    }

    bCurNum = pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum;

    if(curnum > bCurNum)
    {
        for(i = 0; i < (curnum - bCurNum); i++)
        {
            FileDev_NextFile(pstDirDev->hDir, NULL, pstDirDev->ExtName, &stFileAttr);
        }
    }
    else
    {
        for(i = 0; i < (bCurNum - curnum); i++)
        {
            FileDev_PrevFile(pstDirDev->hDir, NULL, pstDirDev->ExtName, &stFileAttr);
        }
    }

    FileDev_GetFileName(pstDirDev->hDir, pstDirDev->path + pstDirDev->pathlen);

    pstDirDev->SubDirInfo[pstDirDev->CurDirDeep].CurFileNum = curnum;

    memcpy((uint8 *)path, (uint8 *)pstDirDev->path, StrLenW(pstDirDev->path) * 2 + 2);

    #if 0
    {
        uint32 i;
        for(i = 0; i < 259 * 2; i++)
        {
            if(i % 16 == 0)
                printf("\n");
            printf("0x%02x ", ((uint8 *)path)[i]);
        }
    }
    #endif


    return RK_SUCCESS;

}

/*******************************************************************************
** Name: DirDev_BuildDirInfo
** Input:uint8* ExtName, FS_TYPE FsType, uint32 * TotalNum
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 15:00:08
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_BuildDirInfo(HDC dev, uint8* ExtName, uint16 * path)
{

    DIR_DEVICE_CLASS *pstDirDev = (DIR_DEVICE_CLASS *)dev;
    FILE_ATTR pstFileAttr;
    uint32 pathLen,CurDirLeve;
    uint32 Deep;
    uint32 TotalFile = 0, TotalDir = 0, CurTotalFile, CurTotalDir;

    for(Deep = 0; Deep <= pstDirDev->CurDirDeep; Deep++)
    {
        if(pstDirDev->SubDirInfo[Deep].hDir != NULL)
        {
            FileDev_CloseDir(pstDirDev->SubDirInfo[Deep].hDir);
            pstDirDev->SubDirInfo[Deep].hDir = NULL;
        }
    }

    if(pstDirDev->hDir)
    {
        FileDev_CloseDir(pstDirDev->hDir);
        pstDirDev->hDir = NULL;
    }

    pstDirDev->TotalAllDir = 0;
    pstDirDev->TotalFile = 0;

    memset(pstDirDev->path, 0, 518);

    Deep = 0;

    pathLen = 0;
    pathLen = StrLenW(path);

    memcpy(pstDirDev->path, path, pathLen * 2);

    if(pstDirDev->path[pathLen - 1] != '\\')
    {
        pstDirDev->path[pathLen] = '\\';
        pathLen++;
    }

    pstDirDev->ExtName = ExtName;
    pstDirDev->pathlen = pathLen;

    pstFileAttr.Path = pstDirDev->path;
    pstFileAttr.FileName =NULL;

    pstDirDev->SubDirInfo[Deep].hDir = FileDev_OpenDir(FileSysHDC, NULL, NOT_CARE, &pstFileAttr);//根目录
    if((int)pstDirDev->SubDirInfo[Deep].hDir <= 0)
    {
        rk_printf("path open fail1");
        return RK_ERROR;
    }

    //first node
    pstDirDev->SubDirInfo[Deep].TotalSubDir = FileDev_GetTotalDir(pstDirDev->SubDirInfo[Deep].hDir, NULL, NULL);//fileInfo->pExtStr
    pstDirDev->SubDirInfo[Deep].TotalFile = FileDev_GetTotalFile(pstDirDev->SubDirInfo[Deep].hDir, pstDirDev->ExtName, NULL);//fileInfo->pExtStr

    pstDirDev->TotalAllDir +=  pstDirDev->SubDirInfo[Deep].TotalSubDir;

    pstDirDev->SubDirInfo[Deep].CurDirNum = 0;
    pstDirDev->SubDirInfo[Deep].DirNum = 0;
    pstDirDev->SubDirInfo[Deep].CurFileNum = 0;

    pstDirDev->CurDirDeep = Deep;

    do
    {

        DirDev_GetCurDirTotalFile(pstDirDev, &CurTotalFile);
        DirDev_GetCurDirTotalSubDir(pstDirDev, &CurTotalDir);
        TotalFile += CurTotalFile;
        TotalDir += CurTotalDir;

    }while(DirDev_GotoNextDir(pstDirDev) == RK_SUCCESS);

    pstDirDev->TotalFile = TotalFile;
    pstDirDev->TotalDir = TotalDir;

    pstDirDev->TotalAllDir =  pstDirDev->SubDirInfo[Deep].TotalSubDir;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_Write(HDC dev)
{
    //DirDev Write...
    DIR_DEVICE_CLASS * pstDirDev =  (DIR_DEVICE_CLASS *)dev;
    if(pstDirDev == NULL)
    {
        return RK_ERROR;
    }

}
/*******************************************************************************
** Name: DirDev_Read
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON API rk_err_t DirDev_Read(HDC dev)
{
    //DirDev Read...
    DIR_DEVICE_CLASS * pstDirDev =  (DIR_DEVICE_CLASS *)dev;
    if(pstDirDev == NULL)
    {
        return RK_ERROR;
    }

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DirDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.16
** Time: 8:53:56
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN rk_err_t DirDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < DIR_DEV_NUM; i++)
    {
        if(gpstDirDevInf[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
    return RK_ERROR;
}
/*******************************************************************************
** Name: ChangeDir
** Input:HDC hFatherDir
** Return: HDC
** Owner:aaron.sun
** Date: 2016.4.20
** Time: 15:56:01
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN HDC ChangeDir(HDC hFatherDir)
{
    FILE_ATTR pstFileAttr;
    HDC hDir;

    FileDev_NextDir(hFatherDir, NULL, &pstFileAttr);

    pstFileAttr.Path = NULL;
    pstFileAttr.FileName = NULL;

    hDir = FileDev_OpenDir(FileSysHDC, hFatherDir, NOT_CARE, &pstFileAttr);

    if((int32)hDir <= 0)
    {
        rk_printf("change dir fail");
        return NULL;
    }

    return hDir;

}

/*******************************************************************************
** Name: DirDevIntIsr1
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN void DirDevIntIsr1(void)
{
    //Call total int service...
    DirDevIntIsr(1);

}
/*******************************************************************************
** Name: DirDevIntIsr0
** Input:void
** Return: void
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN void DirDevIntIsr0(void)
{
    //Call total int service...
    DirDevIntIsr(0);

}
/*******************************************************************************
** Name: DirDevIntIsr
** Input:uint32 DevID
** Return: void
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN void DirDevIntIsr(uint32 DevID)
{
    uint32 DirDevIntType;


    //Get DirDev Int type...
    //DirDevIntType = GetIntType();
    if (gpstDirDevInf[DevID] != NULL)
    {
        //if (DirDevIntType & INT_TYPE_MAP)
        {
            //write serice code...
        }

        //wirte other int service...
    }

}
/*******************************************************************************
** Name: DirDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:26
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN rk_err_t DirDevResume(HDC dev)
{
     DIR_DEVICE_CLASS * pstDirDev = (DIR_DEVICE_CLASS *)dev;
     if(DirDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     pstDirDev->stDirDevice.State = DEV_STATE_WORKING;

     return RK_SUCCESS;
}

/*******************************************************************************
** Name: DirDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:26
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_COMMON_
COMMON FUN rk_err_t DirDevSuspend(HDC dev, uint32 Level)
{
    DIR_DEVICE_CLASS * pstDirDev = (DIR_DEVICE_CLASS *)dev;
    if(DirDevCheckHandler(dev) == RK_ERROR)
    {
       return RK_ERROR;
    }

    if(Level == DEV_STATE_IDLE1)
    {
        pstDirDev->stDirDevice.State = DEV_STATE_IDLE1;
    }
    else if(Level == DEV_SATE_IDLE2)
    {
        pstDirDev->stDirDevice.State = DEV_SATE_IDLE2;
    }

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
** Name: DirDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_INIT_
INIT API rk_err_t DirDev_Delete(uint32 DevID, void *arg)
{
    uint32 Deep;
    //Check DirDev is not exist...
    if(gpstDirDevInf[DevID] == NULL)
    {
        return RK_ERROR;
    }

    for(Deep = 0; Deep < MAX_DIR_DEPTH; Deep++)
    {
        if(gpstDirDevInf[DevID]->SubDirInfo[Deep].hDir != NULL)
        {
            FileDev_CloseDir(gpstDirDevInf[DevID]->SubDirInfo[Deep].hDir);
            gpstDirDevInf[DevID]->SubDirInfo[Deep].hDir = NULL;
        }
    }

    if(gpstDirDevInf[DevID]->hDir)
    {
        FileDev_CloseDir(gpstDirDevInf[DevID]->hDir);
        gpstDirDevInf[DevID]->hDir = NULL;
    }

    //DirDev deinit...
    DirDevDeInit(gpstDirDevInf[DevID]);

    //Free DirDev memory...
    rkos_semaphore_delete(gpstDirDevInf[DevID]->osDirOperReqSem);
    rkos_memory_free(gpstDirDevInf[DevID]);

    //Delete DirDev...
    gpstDirDevInf[DevID] = NULL;

    //Delete DirDev Read and Write Module...
#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_DIR_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DirDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:26
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_INIT_
INIT API HDC DirDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* pstDev;
    DIR_DEVICE_CLASS * pstDirDev;

    // Create handler...
    pstDirDev =  rkos_memory_malloc(sizeof(DIR_DEVICE_CLASS));
    memset(pstDirDev, 0, sizeof(DIR_DEVICE_CLASS));
    if (pstDirDev == NULL)
    {
        rk_printf("pstDirDev = NULL\n");
        return NULL;
    }

    //init handler...
    pstDirDev->osDirOperReqSem  = rkos_semaphore_create(1,1);
    if((pstDirDev->osDirOperReqSem) == 0)
    {
        rkos_semaphore_delete(pstDirDev->osDirOperReqSem);
        rkos_memory_free(pstDirDev);
        return (HDC) RK_ERROR;
    }
    pstDev = (DEVICE_CLASS *)pstDirDev;
    pstDev->suspend = DirDevSuspend;
    pstDev->resume  = DirDevResume;
    pstDev->SuspendMode = ENABLE_MODE;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_DIR_DEV, SEGMENT_OVERLAY_CODE);
#endif
    //device init...
    gpstDirDevInf[DevID] = NULL;
    //DirHwInit(DevID, pstDirDev->Channel);
    if(DirDevInit(pstDirDev) != RK_SUCCESS)
    {
        rkos_semaphore_delete(pstDirDev->osDirOperReqSem);
        rkos_memory_free(pstDirDev);
        return (HDC) RK_ERROR;
    }
    gpstDirDevInf[DevID] = pstDirDev;
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
** Name: DirDevDeInit
** Input:DIR_DEVICE_CLASS * pstDirDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_INIT_
INIT FUN rk_err_t DirDevDeInit(DIR_DEVICE_CLASS * pstDirDev)
{
    rk_printf("dir device deinit");
    return RK_ERROR;
}
/*******************************************************************************
** Name: DirDevInit
** Input:DIR_DEVICE_CLASS * pstDirDev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:26
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_INIT_
INIT FUN rk_err_t DirDevInit(DIR_DEVICE_CLASS * pstDirDev)
{
    return RK_SUCCESS;
}


#ifdef _DIR_DEV_SHELL_
_DRIVER_FILE_DIRDEVICE_DATA_
static SHELL_CMD ShellDirName[] =
{
    "pcb",DirDevShellPcb,"list dir device pcb","dir.pcb [/objectid]\n",
    "create",DirDevShellCreate,"create a dir device","dir.create [/objectid]\n",
    "delete",DirDevShellDelete,"delete a already exist dir device","dir.delete [/objectid]\n",
    "list",DirDevShelllist,"list Specified directory all file and sub dir ","dir.list [/objectid] <dirpath>\n",
    "del",DirDevShellDel,"delete Specified directory all file and sub dir ","dir.del [/objectid] <dirpath>\n",
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
** Name: DirDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL API rk_err_t DirDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;

    if(ShellHelpSampleDesDisplay(dev, ShellDirName, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }


    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDirName, pItem, StrCnt);
    if(ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    ShellHelpDesDisplay(dev, ShellDirName[i].CmdDes, pItem);
    if(ShellDirName[i].ShellCmdParaseFun != NULL)
    {
        ret = ShellDirName[i].ShellCmdParaseFun(dev, pItem);
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
** Name: DirDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.5.17
** Time: 13:38:21
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL FUN rk_err_t DirDevShellDel(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    DIR_DEVICE_CLASS * pstDirDev;
    uint16 Path[MAX_DIRPATH_LEN];
    uint32 dir;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "/", 1) == 0)
    {
        pstr++;

        DevID = String2Num(pstr);

        if(DevID >= DIR_DEV_NUM)
        {
            return RK_ERROR;
        }
    }
    else
    {
        DevID = 0;
    }

    pstDirDev = gpstDirDevInf[DevID];

    if(pstDirDev == NULL)
    {
        rk_printf_no_time("dirdev%d is not exit", DevID);
        return RK_SUCCESS;
    }

    if(*pstr != 0)
    {
        //setpath
        Path[Ascii2Unicode(pstr, Path, strlen(pstr)) / 2] = 0;
        if(DirDev_DeleteDir(pstDirDev, Path) != RK_SUCCESS)
        {
            rk_print_string("\ndir delete fail");
        }
    }
    else
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: DirDevShelllist
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL FUN rk_err_t DirDevShelllist(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    DIR_DEVICE_CLASS * pstDirDev;
    uint16 Path[MAX_DIRPATH_LEN];
    uint8  Path2[MAX_DIRPATH_LEN * 2];
    uint32 dir;
    uint32 TotalFile, TotalDir, CurTotalFile, CurTotalDir;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    if(StrCmpA(pstr, "/", 1) == 0)
    {
        pstr++;

        DevID = String2Num(pstr);

        if(DevID >= DIR_DEV_NUM)
        {
            return RK_ERROR;
        }
    }
    else
    {
        DevID = 0;
    }

    pstDirDev = gpstDirDevInf[DevID];

    if(pstDirDev == NULL)
    {
        rk_printf_no_time("dirdev%d is not exit", DevID);
        return RK_SUCCESS;
    }

    pstr--;
    if(*pstr != 0)
    {
        pstr++;
        //setpath
        Path[Ascii2Unicode(pstr, Path, strlen(pstr)) / 2 ] = 0;

        if(DirDev_BuildDirInfo(pstDirDev,NULL, Path) != RK_SUCCESS)
        {
            rk_printf_no_time("path err");
            return RK_SUCCESS;
        }
    }
    else
    {
         rk_printf_no_time("path err");
         return RK_SUCCESS;
    }

    TotalFile = 0;
    TotalDir = 0;


    do
    {


        DirDev_GetCurDirTotalFile(pstDirDev, &CurTotalFile);
        DirDev_GetCurDirTotalSubDir(pstDirDev, &CurTotalDir);
        TotalFile += CurTotalFile;
        TotalDir += CurTotalDir;

        DirDev_GetCurPath(pstDirDev, Path);

        Path2[Unicode2Ascii(Path2, Path, StrLenW(Path) * 2)] = 0;

        rk_printf_no_time("dir:%s", Path2);

        if(DirDev_FindFirst(pstDirDev, &dir) == RK_SUCCESS)
        {
            do
            {
                DirDev_GetCurPath(pstDirDev, Path);

                Path2[Unicode2Ascii(Path2, Path, StrLenW(Path) * 2)] = 0;

                rk_printf_no_time("file:%s,dir = %d", Path2, dir);
            }while(DirDev_FindNext(pstDirDev, &dir) == RK_SUCCESS);
        }
    }while(DirDev_GotoNextDir(pstDirDev) == RK_SUCCESS);


    rk_printf("toal %d dirs, %d files", TotalDir, TotalFile);

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DirDevShellDelete
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL FUN rk_err_t DirDevShellDelete(HDC dev, uint8 * pstr)
{
    uint32 DevID;

    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    //Get DirDev ID...
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
    if(RKDev_Delete(DEV_CLASS_DIR, DevID, NULL) != RK_SUCCESS)
    {
        rk_print_string("DIRDev delete failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DirDevShellCreate
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL FUN rk_err_t DirDevShellCreate(HDC dev, uint8 * pstr)
{
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

    //Init DirDev arg...
    //stDirDevArg.usbmode = USBOTG_MODE_DEVICE;

    //Create DirDev...

    ret = RKDev_Create(DEV_CLASS_DIR, DevID, NULL);
    if(ret != RK_SUCCESS)
    {
        rk_print_string("DirDev create failure");
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DirDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.6.23
** Time: 13:58:27
*******************************************************************************/
_DRIVER_FILE_DIRDEVICE_SHELL_
SHELL FUN rk_err_t DirDevShellPcb(HDC dev, uint8 * pstr)
{
    uint32 DevID;
    DIR_DEVICE_CLASS * pstDirDev;
    uint32 i;


    if(ShellHelpSampleDesDisplay(dev, NULL, pstr) == RK_SUCCESS)
    {
        return RK_SUCCESS;
    }

    DevID = String2Num(pstr);

    if(DevID >= DIR_DEV_NUM)
    {
        return RK_ERROR;
    }

    pstDirDev = gpstDirDevInf[DevID];

    if(pstDirDev == NULL)
    {
        rk_printf_no_time("dir%d in not exist", DevID);
        return RK_SUCCESS;
    }

    rk_printf_no_time(".gpstDirDevInf[%d]", DevID);
    rk_printf_no_time("    .stDirDevice");
    rk_printf_no_time("        .next = %08x",pstDirDev->stDirDevice.next);
    rk_printf_no_time("        .UseCnt = %d",pstDirDev->stDirDevice.UseCnt);
    rk_printf_no_time("        .SuspendCnt = %d",pstDirDev->stDirDevice.SuspendCnt);
    rk_printf_no_time("        .DevClassID = %d",pstDirDev->stDirDevice.DevClassID);
    rk_printf_no_time("        .DevID = %d",pstDirDev->stDirDevice.DevID);
    rk_printf_no_time("        .suspend = %08x",pstDirDev->stDirDevice.suspend);
    rk_printf_no_time("        .resume = %08x",pstDirDev->stDirDevice.resume);
    rk_printf_no_time("    .osDirOperReqSem = %08x",pstDirDev->osDirOperReqSem);
    rk_printf_no_time("    .ExtName = %08x",pstDirDev->ExtName);
    rk_printf_no_time("    .TotalFile = %08x",pstDirDev->TotalFile);
    rk_printf_no_time("    .TotalAllDir = %08x",pstDirDev->TotalAllDir);
    rk_printf_no_time("    .pathlen = %d",pstDirDev->pathlen);
    rk_printf_no_time("    .CurDirDeep = %d",pstDirDev->CurDirDeep);

    for(i = 0; i < MAX_DIR_DEPTH; i++)
    {
        rk_printf_no_time("    .SubDirInfo[%d]",i);
        rk_printf_no_time("        .hDir = %08x",pstDirDev->SubDirInfo[i].hDir);
        rk_printf_no_time("        .TotalFile = %08x",pstDirDev->SubDirInfo[i].TotalFile);
        rk_printf_no_time("        .TotalSubDir = %08x",pstDirDev->SubDirInfo[i].TotalSubDir);
        rk_printf_no_time("        .CurDirNum = %08x",pstDirDev->SubDirInfo[i].CurDirNum);
        rk_printf_no_time("        .DirNum = %08x",pstDirDev->SubDirInfo[i].DirNum);
        rk_printf_no_time("        .CurFileNum = %08x",pstDirDev->SubDirInfo[i].CurFileNum);
    }

    return RK_SUCCESS;
}


#endif

#endif

