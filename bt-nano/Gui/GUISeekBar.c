/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUISeekBar.c
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
#ifdef __GUI_GUISEEKBAR_C__

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
#include "GUISeekBar.h"
#include "..\Resource\ImageResourceID.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _GUI_SEEKBAR_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    GUI_CONTROL_CLASS * relevance;

    HGC pScoll;
    uint32 Scoll_Width;
    uint32 Scoll_Height;

    HGC pBlock;
    uint32 Block_Width;
    uint32 Block_Height;
    uint32 CurBlock;

    uint32 Scale;
}GUI_SEEKBAR_CLASS;


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
/*******************************************************************************
** Name: GUI_SeekBarSetStyle
** Input:HGC pGc, int *x, int *y
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarGetScollSize(HGC pGc, int *x, int *y)
{
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if ( pstClass== NULL )
    {
        return RK_ERROR;
    }

    if (x!= NULL)*x= pstClass->Scoll_Width;
    if (y!= NULL)*y= pstClass->Scoll_Height;

    return RK_SUCCESS;
}


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
** Name: GUI_SeekBarSetStyle
** Input:HGC pGc, uint32 Scoll, uint32 Block
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarSetStyle(HGC pGc, uint32 Scoll, uint32 Block)
{
    RKGUI_ICON_ARG pstIconArg;
    PICTURE_INFO_STRUCT psPictureInfo;
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if ( pstClass== NULL )
    {
        return RK_ERROR;
    }

    pstIconArg.resource= Scoll;
    pstIconArg.x= -1;
    pstIconArg.y= -1;
    pstClass->pScoll= GUI_IconCreate(&pstIconArg);

    pstIconArg.resource= Block;
    pstIconArg.x= -1;
    pstIconArg.y= -1;
    pstClass->pBlock= GUI_IconCreate(&pstIconArg);
    if (pstClass->pScoll== NULL || pstClass->pBlock== NULL)
    {
        printf("GUI_SeekBarSetStyle error: Create icon fault %d %d\n", Scoll, Block);
        return RK_ERROR;
    }

    FW_GetPicInfoWithIDNum(Scoll,&psPictureInfo);
    pstClass->Scoll_Width= psPictureInfo.xSize;
    pstClass->Scoll_Height= psPictureInfo.ySize;

    FW_GetPicInfoWithIDNum(Block,&psPictureInfo);
    pstClass->Block_Width= psPictureInfo.xSize;
    pstClass->Block_Height= psPictureInfo.ySize;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SeekBarSetCoordinates
** Input:HGC pGc, int x, int y
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarSetCoordinates(HGC pGc, int x, int y)
{
    RKGUI_XY_ARG parameter;
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if ( pstClass== NULL )
    {
        return RK_ERROR;
    }

    parameter.x= x;
    parameter.y= y;
    pstClass->pfControl.x0= x;
    pstClass->pfControl.y0= y;
    pstClass->CurBlock= parameter.y;

    return GcRelevanceOper(pstClass->relevance, OPERATE_SET_XY, &parameter);
}

/*******************************************************************************
** Name: GUI_SeekBarDrag
** Input:HGC pGc, uint32 drag
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarDrag(HGC pGc, uint32 drag)
{
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if (pstClass== NULL)
    {
        return RK_ERROR;
    }

    if (pstClass->pfControl.y0>= 0)
        pstClass->CurBlock= pstClass->pfControl.y0+ pstClass->Scale* drag;

    if ( pstClass->Scoll_Height- pstClass->Block_Height- pstClass->CurBlock< pstClass->Scale)
    {
        pstClass->CurBlock= pstClass->Scoll_Height- pstClass->Block_Height;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SeekBarSetScale
** Input:HGC pGc, uint32 scale
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarSetScale(HGC pGc, uint32 scale)
{
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if (pstClass== NULL || scale==0)
    {
        return RK_ERROR;
    }

    if (scale> 1)
        pstClass->Scale= (pstClass->Scoll_Height - pstClass->Block_Height) / (scale- 1);
    else
        pstClass->Scale= (pstClass->Scoll_Height - pstClass->Block_Height);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SeekBarDisplay
** Input:HGC pGc, int dray
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarDisplay(HGC pGc)
{
    GUI_SEEKBAR_CLASS * pstClass= (GUI_SEEKBAR_CLASS *)pGc;
    if ( pstClass== NULL )
    {
        return RK_ERROR;
    }

    GUI_IconSetCoordinates(pstClass->pBlock, pstClass->pfControl.x0, pstClass->CurBlock);
//    GcRelevanceOper(pstClass->relevance, OPERATE_DISPLAY, NULL);

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
** Name: GUI_SeekBarCreate
** Input:uint32 id, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_SeekBarCreate(void *arg)
{
    GUI_SEEKBAR_CLASS * pstSeekBarClass;
    PICTURE_INFO_STRUCT psPictureInfo;

    pstSeekBarClass=  rkos_memory_malloc(sizeof(GUI_SEEKBAR_CLASS));
    if (pstSeekBarClass== NULL)
    {
        printf("Create SeekBar: malloc fault\n");
        return NULL;
    }

    pstSeekBarClass->pfControl.hDisplay= hDisplay0;

    //pstSeekBarClass->pfControl.ClassID = GUI_CLASS_SEEKBAR;
    pstSeekBarClass->pfControl.x0 = -1;
    pstSeekBarClass->pfControl.y0 = -1;
    pstSeekBarClass->pfControl.xSize = -1;
    pstSeekBarClass->pfControl.ySize = -1;
    pstSeekBarClass->pfControl.display = 1;
    pstSeekBarClass->CurBlock= 0;

    if ( GUI_SeekBarSetStyle(pstSeekBarClass, IMG_ID_BROWSER_SCOLL, IMG_ID_BROWSER_SCOLL_BLOCK)!= RK_SUCCESS )
        return NULL;

    pstSeekBarClass->relevance= NULL;
    GcRelevanceRegister(&pstSeekBarClass->relevance, pstSeekBarClass->pScoll);
    GcRelevanceRegister(&pstSeekBarClass->relevance, pstSeekBarClass->pBlock);

    return &pstSeekBarClass->pfControl;
}

/*******************************************************************************
** Name: GUI_SeekBarDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SeekBarDelete(HGC pGc)
{
    GUI_SEEKBAR_CLASS * pstSeekBarClass= (GUI_SEEKBAR_CLASS *)pGc;

    if ( pstSeekBarClass== NULL )
    {
        printf("delete SeekBar fault\n");
        return RK_ERROR;
    }

    GcRelevanceDelete(&pstSeekBarClass->relevance);
    RKDev_Close(pstSeekBarClass->pfControl.hDisplay);
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

