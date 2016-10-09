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
#ifdef __GUI_GUIMSGBOX_C__

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
#include "GUIMsgBox.h"
#include "GUIText.h"
#include "..\Resource\ImageResourceID.h"
#include "..\Resource\MenuResourceID.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _DIALOGBOX_X_LEFT_      6
#define _DIALOGBOX_X_RIGHT_     4
#define _DIALOGBOX_Y_UP_        31
#define _DIALOGBOX_Y_DOWN_      27

#define _WARNING_X_LEFT_        6
#define _WARNING_X_RIGHT_       4
#define _WARNING_Y_UP_          31
#define _WARNING_Y_DOWN_        15
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


/*******************************************************************************
** Name: GUI_MsgBoxCallBack
** Input:uint32 evnet_type, uint32 event, void * arg, HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_MsgBoxCallBack(uint32 evnet_type, uint32 event, void * arg, HGC pGc)
{
    if (evnet_type!= GUI_EVENT_KEY)return RK_ERROR;
    GUI_MSGBOX_CLASS * pstClass= (GUI_MSGBOX_CLASS *)pGc;

    if(pstClass->cmd==MSGBOX_CMD_WARNING)
    {
        GUI_CallBackApp(APP_RECIVE_MSG_EVENT_WARING, event, NULL, pGc);
        return RK_SUCCESS;
    }
    else
    {
        switch(event)
        {
            case KEY_VAL_FFD_SHORT_UP:
            case KEY_VAL_FFD_LONG_UP:
            case KEY_VAL_FFD_PRESS:
                pstClass->on_off= ~pstClass->on_off;
                break;

            case KEY_VAL_FFW_SHORT_UP:
            case KEY_VAL_FFW_LONG_UP:
            case KEY_VAL_FFW_PRESS:
                pstClass->on_off= ~pstClass->on_off;
                break;

            case KEY_VAL_MENU_SHORT_UP:
                GUI_CallBackApp(APP_RECIVE_MSG_EVENT_DIALOG, 0, (void *)pstClass->on_off, pstClass);
                return RK_SUCCESS;

            default:
                return RK_ERROR;
        }
    }

    GUI_MsgBoxDisplay(pstClass);
    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/

/*******************************************************************************
** Name: GUI_MsgBoxDisplay
** Input:HDC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_MsgBoxDisplay(HDC pGc)
{
    int x0, x1, y0, y1;
    int StartX, StartY, xSize, ySize;
    GUI_MSGBOX_CLASS * pstClass= (GUI_MSGBOX_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( pstClass->cmd==MSGBOX_CMD_WARNING )
    {
        if( FW_GetPicInfoWithIDNum(pstClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay ,pstClass->pfControl.x0, pstClass->pfControl.y0, -1, -1, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        if( GUI_TextDisplayID(pstClass->pfControl.hDisplay, pstClass->pfControl.x0+ 49, pstClass->pfControl.y0+ 14, 128, 12, pstClass->title, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        x0= pstClass->pfControl.x0+ _WARNING_X_LEFT_;
        x1= pstClass->pfControl.xSize- _WARNING_X_LEFT_- _WARNING_X_RIGHT_;
        y0= pstClass->pfControl.y0+ _WARNING_Y_UP_;
        y1= pstClass->pfControl.ySize- _WARNING_Y_UP_- _WARNING_Y_DOWN_;
        GUI_TextAutoAlign(TYPE12X12, pstClass->align, x0, y0, x1, y1, &StartX, &StartY, &xSize , &ySize, pstClass->text);

        if(GUI_TextDisplayBuff(pstClass->pfControl.hDisplay, StartX, StartY, xSize, ySize, pstClass->text, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
    }
    else
    {
        if( FW_GetPicInfoWithIDNum(pstClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay ,pstClass->pfControl.x0, pstClass->pfControl.y0, -1, -1, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        if(pstClass->on_off)
        {
            if( FW_GetPicInfoWithIDNum(IMG_ID_DIALOGBOX_BUTTONNO, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
            if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay ,pstClass->pfControl.x0+ 4, pstClass->pfControl.y0+ 54, -1, -1, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
        }
        else
        {
            if( FW_GetPicInfoWithIDNum(IMG_ID_DIALOGBOX_BUTTONYES, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
            if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay ,pstClass->pfControl.x0+ 4, pstClass->pfControl.y0+ 54, -1, -1, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
        }

        if( GUI_TextDisplayID(pstClass->pfControl.hDisplay, pstClass->pfControl.x0+ 38, pstClass->pfControl.y0+ 12, 128, 12, pstClass->title, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        x0= pstClass->pfControl.x0+ _DIALOGBOX_X_LEFT_;
        x1= pstClass->pfControl.xSize- _DIALOGBOX_X_LEFT_- _DIALOGBOX_X_RIGHT_;
        y0= pstClass->pfControl.y0+ _DIALOGBOX_Y_UP_;
        y1= pstClass->pfControl.ySize- _DIALOGBOX_Y_UP_- _DIALOGBOX_Y_DOWN_;

        GUI_TextAutoAlign(TYPE12X12, pstClass->align, x0, y0, x1, y1, &StartX, &StartY, &xSize , &ySize, pstClass->text);
        if( GUI_TextDisplayBuff(pstClass->pfControl.hDisplay, StartX, StartY, xSize, ySize, pstClass->text, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS)
        {
            return RK_ERROR;
        }

        if( GUI_TextDisplayID(pstClass->pfControl.hDisplay, pstClass->pfControl.x0+ 15, pstClass->pfControl.y0+ 58, 128, 12, pstClass->Button_On, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }

        if(GUI_TextDisplayID(pstClass->pfControl.hDisplay, pstClass->pfControl.x0+ 60, pstClass->pfControl.y0+ 58, 128, 12, pstClass->Button_Off, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
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
** Name: GUI_IconCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_MsgBoxCreate(void *arg)
{
    PICTURE_INFO_STRUCT psPictureInfo;
    GUI_MSGBOX_CLASS * pstClass;
    RKGUI_MSGBOX_ARG * pstMsgBoxArg= (RKGUI_MSGBOX_ARG *)arg;

    pstClass=  rkos_memory_malloc(sizeof(GUI_MSGBOX_CLASS));
    if (pstClass== NULL)
    {
        printf("GUI_MsgBoxCreate: malloc GUI_MSGBOX_CLASS fault\n");
        return NULL;
    }

    if(pstMsgBoxArg->level)
    {
        pstClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstClass->pfControl.hDisplay = hDisplay0;
    }

    pstClass->pfControl.ClassID = GUI_CLASS_MSG_BOX;
    pstClass->pfControl.x0 = pstMsgBoxArg->x;
    pstClass->pfControl.y0 = pstMsgBoxArg->y;
    pstClass->pfControl.display= pstMsgBoxArg->display;
    pstClass->pfControl.level= pstMsgBoxArg->level;
    pstClass->pfControl.rotate= ROTATE_NOMAL;

    pstClass->cmd= pstMsgBoxArg->cmd;
    pstClass->text_cmd= pstMsgBoxArg->text_cmd;

    if( pstMsgBoxArg->text_cmd==TEXT_CMD_ID )
    {
        pstClass->text= GUI_TextExtractString((int)pstMsgBoxArg->text);
        if( pstClass->text== NULL )
        {
            return NULL;
        }
    }
    else
    {
        pstClass->text= pstMsgBoxArg->text;
    }

    pstClass->title= pstMsgBoxArg->title;
    pstClass->Button_On= pstMsgBoxArg->Button_On;
    pstClass->Button_Off= pstMsgBoxArg->Button_Off;
    pstClass->on_off= 0;
    pstClass->pfControl.flag= 0;
    pstClass->pfControl.lucency= OPACITY;
    pstClass->pfControl.translucence= 1;
    pstClass->align= pstMsgBoxArg->align;

    if( pstClass->cmd==MSGBOX_CMD_WARNING  )
    {
        pstClass->Backdrop= IMG_ID_WARNING_BOX;
    }
    else if( pstClass->cmd==MSGBOX_CMD_DIALOG  )
    {
        pstClass->Backdrop= IMG_ID_DIALOGBOX_BACKGROUND;
    }
    else
    {
        return NULL;
    }

    if( FW_GetPicInfoWithIDNum(pstClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
    {
        return NULL;
    }
    pstClass->pfControl.xSize = psPictureInfo.xSize;
    pstClass->pfControl.ySize = psPictureInfo.ySize;

    if(pstClass->pfControl.display)
    {
        GuiTask_TimerLock();
        GUI_Setfocus(pstClass);
        DisplayDev_Blur(pstClass->pfControl.hDisplay);
        GUI_MsgBoxDisplay(pstClass);
        GUI_Createfocus(GUI_MsgBoxCallBack, pstClass);
    }

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
COMMON API rk_err_t GUI_MsgBoxDelete(HGC pGc)
{
    GUI_MSGBOX_CLASS * pstClass= (GUI_MSGBOX_CLASS *)pGc;

    if( pstClass->text_cmd==TEXT_CMD_ID && pstClass->text!= NULL )
    {
        rkos_memory_free(pstClass->text);
    }

    GuiTask_TimerUnLock();
    GUI_Deletefocus(pGc);
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

