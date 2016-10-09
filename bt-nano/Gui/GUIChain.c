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
#ifdef __GUI_GUICHAIN_C__

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
#include "GUIChain.h"


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

/*******************************************************************************
** Name: GUI_TextTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_ChainTimer(HGC pGc)
{
    GUI_CHAIN_CLASS * pstClass= (GUI_CHAIN_CLASS *)pGc;

    if( pstClass->speed ++ < pstClass->delay )
    {
        return RK_SUCCESS;
    }
    pstClass->speed= 0;

    GcRelevanceOper(pGc);
    GcDisplay(pGc);

    if( ++pstClass->Current >= (pstClass->num + pstClass->ResourceId))
    {
        pstClass->Current = pstClass->ResourceId ;
    }

    return RK_SUCCESS;
}





/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: GUI_TextTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ChainDisplay(HGC pGc)
{
    PICTURE_INFO_STRUCT psPictureInfo;
    GUI_CHAIN_CLASS * pstClass= (GUI_CHAIN_CLASS *)pGc;

    if( FW_GetPicInfoWithIDNum(pstClass->Current, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if(GUI_IconDisplayResource(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, -1, -1, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

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


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ChainCallBack(uint32 evnet_type, uint32 event, void * arg, HGC pGc)
{
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
** Name: GUI_IconCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_ChainCreate(void *arg)
{
    PICTURE_INFO_STRUCT psPictureInfo;
    GUI_CHAIN_CLASS * pstClass;
    RKGUI_CHAIN_ARG * pstChainArg= (RKGUI_CHAIN_ARG *)arg;

    pstClass=  rkos_memory_malloc(sizeof(GUI_CHAIN_CLASS));
    if (pstClass== NULL)
    {
        printf("GUI_ChainCreate: malloc GUI_CHAIN_CLASS fault\n");
        return NULL;
    }

    if(pstChainArg->level)
    {
        pstClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstClass->pfControl.hDisplay = hDisplay0;
    }

    pstClass->pfControl.ClassID = GUI_CLASS_CHAIN;
    pstClass->pfControl.x0 = pstChainArg->x;
    pstClass->pfControl.y0 = pstChainArg->y;
    pstClass->pfControl.xSize = -1;
    pstClass->pfControl.ySize = -1;

    pstClass->pfControl.display= pstChainArg->display;
    pstClass->pfControl.level= pstChainArg->level;

    pstClass->pfControl.rotate= ROTATE_NOMAL;
    pstClass->ResourceId= pstChainArg->resource;
    pstClass->Current= pstChainArg->resource;
    pstClass->num= pstChainArg->num;
    pstClass->delay= pstChainArg->delay;
    pstClass->speed= 0;

    if( FW_GetPicInfoWithIDNum(pstClass->Current, &psPictureInfo)!= RK_SUCCESS )
    {
        //return (GUI_CONTROL_CLASS *) RK_ERROR;
        return &pstClass->pfControl;
    }

    if(pstClass->pfControl.x0< 0)pstClass->pfControl.x0= psPictureInfo.x;
    if(pstClass->pfControl.y0< 0)pstClass->pfControl.y0= psPictureInfo.y;
    if(pstClass->pfControl.xSize< 0)pstClass->pfControl.xSize = psPictureInfo.xSize;
    if(pstClass->pfControl.ySize< 0)pstClass->pfControl.ySize = psPictureInfo.ySize;


    pstClass->blurry = pstChainArg->blurry;
    if(pstChainArg->blurry)
    {
        GUI_Createfocus(GUI_ChainCallBack, pstClass);
        GUI_Setfocus(pstClass);
        DisplayDev_Blur(pstClass->pfControl.hDisplay);
    }

    rkgui_create_timer(pstClass,GUI_ChainTimer);

    if(pstChainArg->display)
    {
        rkgui_start_timer(pstClass);
        GUI_ChainDisplay(pstClass);
    }

    pstClass->pfControl.translucence= pstChainArg->blurry;
    pstClass->pfControl.lucency= OPACITY;
    pstClass->pfControl.flag = 0;

    return &pstClass->pfControl;
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
COMMON API rk_err_t GUI_ChainDelete(HGC pGc)
{
    GUI_CHAIN_CLASS * pstClass= (GUI_CHAIN_CLASS *)pGc;

    if(pstClass->blurry)
    {
        GUI_Deletefocus(pGc);
    }
    rkgui_delete_timer(pstClass);
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

