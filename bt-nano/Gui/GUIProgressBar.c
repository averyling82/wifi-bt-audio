/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIProgressBar.c
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
#ifdef __GUI_GUIPROGRESSBAR_C__

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
#include "GUIProgressBar.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef struct _GUI_PROGRESSBAR_CLASS
{
    GUI_CONTROL_CLASS pfControl;

    int Backdrop;
    int BackdropX;
    int BackdropY;

    int Bar;
    int BarX;
    int BarY;
    int percentage;
    uint8 * dat;
}GUI_PROGRESSBAR_CLASS;
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
*                                                   API(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GUI_TextSetContent
** Input:HGC pGc, void * arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ProgressBarContent(HGC pGc, void * arg)
{
    GUI_PROGRESSBAR_CLASS * pstProgressBarClass= (GUI_PROGRESSBAR_CLASS *)pGc;
    RKGUI_PROGRESSBAR_ARG * pstProgressBarArg= (RKGUI_PROGRESSBAR_ARG *)arg;
    if (pstProgressBarClass== NULL)
    {
        return RK_ERROR;
    }

    if( pstProgressBarArg->cmd==PROGRESSBAR_SET_CONTENT )
    {
        pstProgressBarClass->percentage= pstProgressBarArg->percentage;
    }

    GcRelevanceOper(pGc);
    return GcDisplay(pGc);
}

/*******************************************************************************
** Name: GUI_ImageDisplay
** Input:HGC pGc,void * buf
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ProgressBarDisplay(HGC pGc)
{
    int xSize= 0;
    GUI_PROGRESSBAR_CLASS * pstProgressBarClass= (GUI_PROGRESSBAR_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if (pstProgressBarClass== NULL)
    {
        return RK_ERROR;
    }

    xSize= pstProgressBarClass->pfControl.xSize;
    pstProgressBarClass->pfControl.xSize = (pstProgressBarClass->BarX* pstProgressBarClass->percentage)/ 100;

    if( xSize < pstProgressBarClass->pfControl.xSize )
    {
        xSize= pstProgressBarClass->pfControl.xSize;
    }

    if( FW_GetPicInfoWithIDNum(pstProgressBarClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if( GUI_IconAreaUpdata(pstProgressBarClass->pfControl.hDisplay, pstProgressBarClass->pfControl.x0, pstProgressBarClass->pfControl.y0, xSize, pstProgressBarClass->pfControl.ySize,
        pstProgressBarClass->BackdropX, pstProgressBarClass->BackdropY, &psPictureInfo)!= RK_SUCCESS)
        return RK_ERROR;

    DisplayDev_SetWindow(pstProgressBarClass->pfControl.hDisplay, pstProgressBarClass->pfControl.x0, pstProgressBarClass->pfControl.y0, pstProgressBarClass->pfControl.xSize, pstProgressBarClass->pfControl.ySize);
    DisplayDev_Write(pstProgressBarClass->pfControl.hDisplay, pstProgressBarClass->dat, pstProgressBarClass->pfControl.ySize* pstProgressBarClass->pfControl.xSize* ICON_PIXEL_WIDE);

    return RK_SUCCESS;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
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
** Name: GUI_ProgressBarCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_ProgressBarCreate(void *arg)
{
    GUI_PROGRESSBAR_CLASS * pstProgressBarClass;
    RKGUI_PROGRESSBAR_ARG * pstProgressBarArg= (RKGUI_PROGRESSBAR_ARG *)arg;;
    PICTURE_INFO_STRUCT psPictureInfo;

    pstProgressBarClass=  rkos_memory_malloc(sizeof(GUI_PROGRESSBAR_CLASS));
    if (pstProgressBarClass== NULL)
    {
        return NULL;

    }
    if(pstProgressBarArg->level)
    {
        pstProgressBarClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstProgressBarClass->pfControl.hDisplay = hDisplay0;
    }

    pstProgressBarClass->pfControl.ClassID = GUI_CLASS_PROGRESSBAR;
    pstProgressBarClass->pfControl.x0 = pstProgressBarArg->x;
    pstProgressBarClass->pfControl.y0 = pstProgressBarArg->y;
    pstProgressBarClass->pfControl.rotate= ROTATE_NOMAL;
    pstProgressBarClass->pfControl.level= pstProgressBarArg->level;

    pstProgressBarClass->pfControl.display= pstProgressBarArg->display;
    pstProgressBarClass->pfControl.flag= 0;

    pstProgressBarClass->pfControl.lucency= OPACITY;
    pstProgressBarClass->pfControl.translucence= 0;

    pstProgressBarClass->Backdrop = pstProgressBarArg->Backdrop;
    pstProgressBarClass->BackdropX= pstProgressBarArg->BackdropX;
    pstProgressBarClass->BackdropY= pstProgressBarArg->BackdropY;
    pstProgressBarClass->Bar= pstProgressBarArg->Bar;
    pstProgressBarClass->percentage= pstProgressBarArg->percentage;

    if( pstProgressBarClass->pfControl.display )
    {
        if( FW_GetPicInfoWithIDNum(pstProgressBarClass->Bar, &psPictureInfo)!= RK_SUCCESS )
        {
            return NULL;
        }

        pstProgressBarClass->dat= (uint8 *)rkos_memory_malloc( psPictureInfo.totalSize);
        if(FW_GetPicResource(psPictureInfo.offsetAddr, pstProgressBarClass->dat, psPictureInfo.totalSize)!= RK_SUCCESS)
        {
            RKDev_Close(pstProgressBarClass->pfControl.hDisplay);
            rkos_memory_free(pstProgressBarClass->dat);
            rkos_memory_free(pstProgressBarClass);
            return NULL;
        }

        pstProgressBarClass->BarX = psPictureInfo.xSize;
        pstProgressBarClass->BarY = psPictureInfo.ySize;

        if(pstProgressBarClass->percentage== 0)
        {
            pstProgressBarClass->pfControl.xSize = 0;
        }
        else
        {
            pstProgressBarClass->pfControl.xSize = (pstProgressBarClass->BarX* pstProgressBarClass->percentage)/ 10000;
        }
        pstProgressBarClass->pfControl.ySize = pstProgressBarClass->BarY;

        GUI_ProgressBarDisplay(pstProgressBarClass);
    }

    return &pstProgressBarClass->pfControl;
}

/*******************************************************************************
** Name: GUI_ProgressBarDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ProgressBarDelete(HGC pGc)
{
    GUI_PROGRESSBAR_CLASS * pstProgressBarClass= (GUI_PROGRESSBAR_CLASS *)pGc;

    if ( pstProgressBarClass== NULL )
    {
        printf("delete ProgressBar fault\n");
        return RK_ERROR;
    }

    rkos_memory_free(pstProgressBarClass->dat);
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

