/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIImage.c
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
#ifdef __GUI_GUIIMAGE_C__

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
#include "GUIImage.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

typedef struct _GUI_IMAGE_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    eIMAGE_DISPLAY_CMD cmd;
    LUCENCY_TYPE lucency;

    void * buf;
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 transparency;
}GUI_IMAGE_CLASS;
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
** Name: GUI_ImageDisplay
** Input:HGC pGc,void * buf
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ImageDisplayRect(HDC hDisplay, int x, int y, int xSize, int ySize, int R, int G, int B, int transparency)
{
    uint32 size;

    if (xSize== 0 || ySize== 0)
    {
        return RK_ERROR;
    }

    DisplayDev_SetWindow(hDisplay, x, y, xSize, ySize);
    DisplayDev_ClrRect(hDisplay, R, G, B, transparency);

    return RK_SUCCESS;
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
COMMON API rk_err_t GUI_ImageDisplay(HGC pGc)
{
    uint32 size;

    GUI_IMAGE_CLASS * pstClass= (GUI_IMAGE_CLASS *)pGc;

    if (pstClass->pfControl.xSize == 0 || pstClass->pfControl.ySize == 0)
    {
        return RK_ERROR;
    }

    if (pstClass->cmd== IMAGE_CMD_RECT)
    {
        GUI_ImageDisplayRect(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->pfControl.xSize, pstClass->pfControl.ySize,
            pstClass->R, pstClass->G, pstClass->B, pstClass->transparency);
    }

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
** Name: GUI_ImageCreate
** Input:uint32 id, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_ImageCreate(void *arg)
{
    GUI_IMAGE_CLASS * pstImageClass;
    RKGUI_IMAGE_ARG * pstImageArg= (RKGUI_IMAGE_ARG *)arg;;

    pstImageClass=  rkos_memory_malloc(sizeof(GUI_IMAGE_CLASS));
    if (pstImageClass== NULL)
    {
        return NULL;
    }

    if(pstImageArg->level)
    {
        pstImageClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstImageClass->pfControl.hDisplay = hDisplay0;
    }

    pstImageClass->pfControl.ClassID = GUI_CLASS_IMAGE;
    pstImageClass->cmd= pstImageArg->cmd;
    pstImageClass->pfControl.x0 = pstImageArg->x;
    pstImageClass->pfControl.y0 = pstImageArg->y;
    pstImageClass->pfControl.xSize = pstImageArg->xSize;
    pstImageClass->pfControl.ySize = pstImageArg->ySize;
    pstImageClass->pfControl.rotate= ROTATE_NOMAL;
    pstImageClass->pfControl.level= pstImageArg->level;

    pstImageClass->buf = pstImageArg->buf;
    pstImageClass->R = pstImageArg->R;
    pstImageClass->G = pstImageArg->G;
    pstImageClass->B = pstImageArg->B;
    pstImageClass->transparency= pstImageArg->transparency;
    pstImageClass->pfControl.display= pstImageArg->display;
    pstImageClass->pfControl.flag= 0;

    pstImageClass->pfControl.lucency= pstImageArg->lucency;
    pstImageClass->pfControl.translucence= 0;

    if( pstImageClass->pfControl.display )
    {
        GUI_ImageDisplay(pstImageClass);
    }
    return &pstImageClass->pfControl;
}

/*******************************************************************************
** Name: GUI_ImageDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_ImageDelete(HGC pGc)
{
    GUI_IMAGE_CLASS * pstImageClass= (GUI_IMAGE_CLASS *)pGc;

    if ( pstImageClass== NULL )
    {
        printf("delete Image fault\n");
        return RK_ERROR;
    }
    rkos_memory_free(pGc);
}
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

