/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIList.c
* Owner: Benjo.lei
* Date: 2015.10.20
* Time: 15:07:35
* Desc:
* History:
*   <author>    <date>       <time>     <version>     <Desc>
* Benjo.lei      2015.10.14     15:07:35   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __GUI_GUITEXTBOX_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "GUITask.h"
#include "GUIManager.h"
#include "GUIImage.h"
#include "GUIIcon.h"
#include "GUISelect.h"
#include "GUIText.h"
#include "GUITextBox.h"
#include "GUISeekBar.h"
#include "..\Resource\ImageResourceID.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _GUI_TEXTBOX_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    GUI_CONTROL_CLASS * relevance;
    pTimer timer;
    HGC Cursor;
}GUI_TEXTBOX_CLASS;

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
static GUI_TEXTBOX_CLASS * gpstGuiTextBoxISR= NULL;

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
** Name: GUI_TextBoxCallBack
** Input:uint32 evnet_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.02
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_TextBoxCallBack(uint32 evnet_type, uint32 event, void * arg, uint32 mode)
{
//    if (evnet_type!= GUI_EVENT_KEY)return RK_ERROR;
//    switch (event)
//    {
//        case KEY_VAL_MENU_SHORT_UP:
//            break;
//
//        case KEY_VAL_FFW_SHORT_UP:
//            break;
//
//        case KEY_VAL_FFD_SHORT_UP:
//            break;
//
//        default :
//            break;
//    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextBoxTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_TextBoxTimer(pTimer timer)
{
    static int CursorDsFlag= 0;

    if(CursorDsFlag)
    {
        CursorDsFlag= 0;
        GcRelevanceRegister(&gpstGuiTextBoxISR->relevance, gpstGuiTextBoxISR->Cursor);
    }
    else
    {
        CursorDsFlag= 1;
        GcRelevanceUnRegister(&gpstGuiTextBoxISR->relevance, gpstGuiTextBoxISR->Cursor);
    }
    GUI_TextBoxDisplay(gpstGuiTextBoxISR);
    DisplayDev_ScreenUpdate(gpstGuiTextBoxISR->pfControl.hDisplay);
    rkos_start_timer(gpstGuiTextBoxISR->timer);
}

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
** Name: GUI_TextBoxDisplay
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.20
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextBoxDisplay(HGC pGc)
{
    rk_err_t ret;
    GUI_TEXTBOX_CLASS * pstClass= (GUI_TEXTBOX_CLASS *)pGc;
    if (pstClass== NULL)
    {
        return RK_ERROR;
    }

    ret= GcRelevanceOper(pstClass->relevance, OPERATE_DISPLAY, NULL);
    return ret;
}

/*******************************************************************************
** Name: GUI_TextBoxStartInputMethods
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.20
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextBoxStartInputMethods(HGC pGc)
{
    GUI_TEXTBOX_CLASS * pstClass= (GUI_TEXTBOX_CLASS *)pGc;
    if (pstClass== NULL)
    {
        return RK_ERROR;
    }

    GUITask_AppReciveRegister(GUI_TextBoxCallBack);
    rkos_start_timer(pstClass->timer);
    GcRelevanceRegister(&pstClass->relevance, pstClass->Cursor);
    GUI_TextBoxDisplay(pstClass);
    DisplayDev_ScreenUpdate(pstClass->pfControl.hDisplay);

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
** Name: GUI_TextBoxCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.02
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_TextBoxCreate(void *arg)
{
    GUI_TEXTBOX_CLASS * pstClass;
    RKGUI_TEXTBOX_ARG * pstArg= (RKGUI_TEXTBOX_ARG *)arg;
    RKGUI_IMAGE_ARG pstImageArg;
    RKGUI_TEXT_ARG pstTextArg;
    GUI_CONTROL_CLASS * hGc;

    pstClass=  rkos_memory_malloc(sizeof(GUI_TEXTBOX_CLASS));
    if (pstClass== NULL)
    {
        printf("GUI_TextBoxCreate: malloc GUI_TEXTBOX_CLASS fault\n");
        return NULL;
    }

    pstClass->pfControl.hDisplay = hDisplay0;


    pstClass->pfControl.ClassID = GUI_CLASS_TEXTBOX;
    pstClass->pfControl.x0 = pstArg->x;
    pstClass->pfControl.y0 = pstArg->y;
    pstClass->pfControl.xSize = -1;
    pstClass->pfControl.ySize = -1;
    pstClass->pfControl.display = 1;

    pstClass->pfControl.rotate= ROTATE_NOMAL;

    if(pstArg->background!= NULL)
    {
        GcRelevanceRegister(&pstClass->relevance, pstArg->background);
    }
    else
    {
        pstImageArg.cmd= IMAGE_CMD_RECT;
        pstImageArg.x= pstArg->x;
        pstImageArg.y= pstArg->y;
        pstImageArg.xSize= pstArg->xSize;
        pstImageArg.ySize= pstArg->ySize;

        pstImageArg.R= 0xB0;
        pstImageArg.G= 0xB0;
        pstImageArg.B= 0xB0;
        pstImageArg.transparency= 0;

        hGc= GUI_ImageCreate(&pstImageArg);
        GcRelevanceRegister(&pstClass->relevance, hGc);
    }

    pstClass->timer= rkos_create_timer(100, 100, NULL, GUI_TextBoxTimer);

    pstImageArg.cmd= IMAGE_CMD_RECT;
    pstImageArg.x= pstArg->x + 2;
    pstImageArg.y= pstArg->y + 2;
    pstImageArg.xSize= 1;
    pstImageArg.ySize= pstArg->ySize - 4;

    pstImageArg.R= 0;
    pstImageArg.G= 0;
    pstImageArg.B= 0;
    pstImageArg.transparency= 0;
    pstClass->Cursor= GUI_ImageCreate(&pstImageArg);

    pstTextArg.font= pstArg->font;
    pstTextArg.R= 0;
    pstTextArg.G= 0;
    pstTextArg.B= 0;
    pstTextArg.transparency= 0;
    pstTextArg.x= pstArg->x + 2;
    pstTextArg.y= pstArg->y + 2;
    pstTextArg.xSize= pstArg->xSize;
    pstTextArg.ySize= pstArg->ySize;
    pstTextArg.retract= -4;
    pstTextArg.text= NULL;
    hGc= GUI_TextCreate(&pstTextArg);
    GcRelevanceRegister(&pstClass->relevance, hGc);

    gpstGuiTextBoxISR= pstClass;
    return &pstClass->pfControl;
}


/*******************************************************************************
** Name: GUI_TextBoxDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.11.02
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextBoxDelete(HGC pGc)
{
    GUI_TEXTBOX_CLASS * pstClass= (GUI_TEXTBOX_CLASS *)pGc;

    if ( pstClass== NULL )
    {
        printf("delete textbox fault\n");
        return RK_ERROR;
    }

    if (pstClass->relevance!= NULL)GcRelevanceDelete(&pstClass->relevance);
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

