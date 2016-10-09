/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIIcon.c
* Owner: Benjo.lei
* Date: 2015.10.14
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __GUI_GUIICON_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "GUIManager.h"
#include "GUIIcon.h"


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
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/


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


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GUI_IconAreaUpdata
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconAreaUpdata(HDC hDisplay, int x, int y, int xSize, int ySize, int bckx, int bcky, PICTURE_INFO_STRUCT * psPictureInfo)
{
    int totalSize, reqSize;
    uint32 offsetAddr, px= 0, py= 0;
    uint8 * pstBuf= NULL;

    totalSize= psPictureInfo->xSize* ySize * ICON_PIXEL_WIDE;
    offsetAddr= psPictureInfo->offsetAddr + (psPictureInfo->xSize * bcky) * ICON_PIXEL_WIDE;

    DisplayDev_SetWindow(hDisplay, x, y, xSize, ySize);

    while(1)
    {
        pstBuf= (uint8 *)rkos_memory_malloc(psPictureInfo->xSize* ICON_PIXEL_WIDE);
        if(pstBuf != NULL)
            break;

        rkos_sleep(10);
    }

    px= bckx;
    py= 0;

    while( totalSize )
    {
        reqSize= psPictureInfo->xSize* ICON_PIXEL_WIDE;
        if(FW_GetPicResource(offsetAddr, pstBuf, reqSize)!= RK_SUCCESS)
        {
            rkos_memory_free(pstBuf);
            return RK_ERROR;
        }
        DisplayDev_Write(hDisplay, &pstBuf[(px+ py* psPictureInfo->xSize)* ICON_PIXEL_WIDE], xSize);

        offsetAddr += reqSize;
        totalSize  -= reqSize;
    }

    rkos_memory_free(pstBuf);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_IconDisplayResource
** Input:HDC hDisplay, int x0, int y0, int x1, int y1, int ResourceId
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconDisplayResource(HDC hDisplay, int x0, int y0, int x1, int y1, PICTURE_INFO_STRUCT * psPictureInfo)
{
    uint32 totalSize, reqSize;
    uint32 offsetAddr;
    uint8 * pstBuf= NULL;

    offsetAddr = psPictureInfo->offsetAddr;
    if (hDisplay == NULL)
    {
        return RK_ERROR;
    }

    if ( x0 < 0 )x0= psPictureInfo->x;
    if ( y0 < 0 )y0= psPictureInfo->y;
    if ( y1 < 0 )y1= psPictureInfo->ySize;
    x1= psPictureInfo->xSize;

    DisplayDev_SetWindow(hDisplay, x0, y0, x1, y1);

    totalSize= x1* y1 *ICON_PIXEL_WIDE;

    while (totalSize)
    {
        reqSize= (totalSize > (1024*4))? (1024*4) : totalSize;
        pstBuf= (uint8 *)rkos_memory_malloc(reqSize);
        if(pstBuf!= NULL)
        {
            if( FW_GetPicResource(offsetAddr, pstBuf, reqSize)!= RK_SUCCESS )
            {
                rkos_memory_free(pstBuf);
                return RK_ERROR;
            }
            offsetAddr += reqSize;
            totalSize  -= reqSize;
            DisplayDev_Write(hDisplay, pstBuf, reqSize/ICON_PIXEL_WIDE);
            rkos_memory_free(pstBuf);
            pstBuf= NULL;
        }
        else
        {
            rkos_sleep(1000);
        }
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: GUI_IconDisplay
** Input:HDC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconDisplay(HDC pGc)
{
    GUI_ICON_CLASS * pstIconClass= (GUI_ICON_CLASS *)pGc;
    uint32 totalSize;

    return GUI_IconDisplayResource(pstIconClass->pfControl.hDisplay, pstIconClass->pfControl.x0, pstIconClass->pfControl.y0, pstIconClass->pfControl.xSize, pstIconClass->pfControl.ySize, &pstIconClass->psPictureInfo);
}

/*******************************************************************************
** Name: GUI_IconInit
** Input:HDC pGc, void * arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconInit(HDC pGc, void * arg)
{
    GUI_ICON_CLASS * pstIconClass= (GUI_ICON_CLASS *)pGc;
    RKGUI_ICON_ARG * pstIconArg= (RKGUI_ICON_ARG *)arg;

    pstIconClass->pfControl.ClassID = GUI_CLASS_ICON;
    pstIconClass->pfControl.x0 = pstIconArg->x;
    pstIconClass->pfControl.y0 = pstIconArg->y;
    pstIconClass->pfControl.xSize = -1;
    pstIconClass->pfControl.ySize = -1;
    pstIconClass->pfControl.display= pstIconArg->display;
    pstIconClass->pfControl.level= pstIconArg->level;

    pstIconClass->pfControl.rotate = ROTATE_NOMAL;
    pstIconClass->pfControl.flag= 0;

    pstIconClass->pfControl.lucency = OPACITY;
    pstIconClass->pfControl.translucence= 0;

    if( FW_GetPicInfoWithIDNum(pstIconArg->resource, &pstIconClass->psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if(pstIconClass->pfControl.x0< 0) pstIconClass->pfControl.x0 = pstIconClass->psPictureInfo.x;
    if(pstIconClass->pfControl.y0< 0) pstIconClass->pfControl.y0 = pstIconClass->psPictureInfo.y;
    if(pstIconClass->pfControl.xSize< 0) pstIconClass->pfControl.xSize = pstIconClass->psPictureInfo.xSize;
    if(pstIconClass->pfControl.ySize< 0) pstIconClass->pfControl.ySize = pstIconClass->psPictureInfo.ySize;

    return RK_SUCCESS;

}

/*******************************************************************************
** Name: GUI_IconSetID
** Input:HDC pGc, uint32 ID
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconSetID(HDC pGc, uint32 ResourceId)
{
    GUI_ICON_CLASS * pstIconClass= (GUI_ICON_CLASS *)pGc;

    if( FW_GetPicInfoWithIDNum(ResourceId, &pstIconClass->psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if(pstIconClass->pfControl.x0< 0) pstIconClass->pfControl.x0 = pstIconClass->psPictureInfo.x;
    if(pstIconClass->pfControl.y0< 0) pstIconClass->pfControl.y0 = pstIconClass->psPictureInfo.y;
    pstIconClass->pfControl.xSize = pstIconClass->psPictureInfo.xSize;
    pstIconClass->pfControl.ySize = pstIconClass->psPictureInfo.ySize;

    return RK_SUCCESS;

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
** Name: GUI_IconCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_IconCreate(void *arg)
{
    GUI_ICON_CLASS * pstIconClass;
    RKGUI_ICON_ARG * pstIconArg= (RKGUI_ICON_ARG *)arg;

    pstIconClass=  rkos_memory_malloc(sizeof(GUI_ICON_CLASS));
    if (pstIconClass== NULL)
    {
        printf("GUI_IconCreate: malloc GUI_ICON_CLASS fault\n");
        return NULL;
    }

    if(pstIconArg->level)
    {
        pstIconClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstIconClass->pfControl.hDisplay = hDisplay0;
    }

    GUI_IconInit((HGC)pstIconClass, arg);

    if(pstIconClass->pfControl.display)
    {
        GUI_IconDisplay(pstIconClass);
    }

    return &pstIconClass->pfControl;
}

/*******************************************************************************
** Name: GUI_IconDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_IconDelete(HGC pGc)
{
    GUI_ICON_CLASS * pstIconClass= (GUI_ICON_CLASS *)pGc;
    rkos_memory_free(pGc);
    return RK_SUCCESS;
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

