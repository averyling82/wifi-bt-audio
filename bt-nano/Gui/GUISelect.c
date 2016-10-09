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
#ifdef __GUI_GUISELECT_C__

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
#include "..\Resource\ImageResourceID.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _SELECT_MARGIN_TOP_           2            // 顶部留边区域
#define _SELECT_MARGIN_CUTOFF_        1            // 单元分割线宽度
#define _SELECT_MIN_SEEKBAR_SIZE_    16            // 拖动条最小尺寸

typedef struct _GUI_SELECT_CLASS
{
    GUI_CONTROL_CLASS pfControl;
    eTEXT_DISPLAY_CMD cmd;

    RKGUI_SELECT_ITEM * pList;
    Ucs2 * text;
    Ucs2 * text_bck;
    int pListCurrent;
    int pDisplayCurrent;

    int Background;
    int SeekBar;
    int ItemHeight;
    int IconAreaStart;
    int IconAreaSize;
    int ItemWidth;
    int CursorStyle;
    int ItemTopMargin;

    uint32 itemSum;
    uint32 limit;
    uint32 MaxDisplayItem;

    int CursorOffset;
    int Cursor;

    P_SELECT_RECIVE_MSG pReviceMsg;

    int BarX;
    int BarY;
    int BarXSize;
    int BarYSize;
    int BarYSizeBck;
    float BarYScale;

    int KeepBackground;
}GUI_SELECT_CLASS;


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
** Name: GUI_TextTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectTimer(HGC pGc)
{
    static int speed= 0;
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;

    if(pstClass->pfControl.display == 0)
    {
        return RK_SUCCESS;
    }

    if( speed++ < 20 )
    {
        return RK_SUCCESS;
    }
    speed= 0;

    if(*pstClass->text!= 0)
        pstClass->text++;
    else
        pstClass->text= pstClass->text_bck;

    pstClass->KeepBackground= 1;
    GcRelevanceOper(pstClass);
    GcDisplay(pstClass);
    pstClass->KeepBackground= 0;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SelectStartTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectStartTimer(HGC pGc)
{
    GUI_SELECT_CLASS * pstSelectClass= (GUI_SELECT_CLASS *)pGc;

    if( GUI_TextSizeCmp(TYPE12X12, pstSelectClass->pList[pstSelectClass->pListCurrent].text, pstSelectClass->ItemWidth- 5)== RK_ERROR)
    {
        pstSelectClass->text= pstSelectClass->pList[pstSelectClass->pListCurrent].text;
        pstSelectClass->text_bck= pstSelectClass->pList[pstSelectClass->pListCurrent].text;
        rkgui_start_timer(pstSelectClass);
    }
    else
    {
        pstSelectClass->text= NULL;
        pstSelectClass->text_bck= NULL;
        rkgui_stop_timer(pstSelectClass);
    }

    return RK_SUCCESS;
}
/*******************************************************************************
** Name: GUI_SelectStopTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectStopTimer(HGC pGc)
{
    GUI_SELECT_CLASS * pstSelectClass= (GUI_SELECT_CLASS *)pGc;

    pstSelectClass->text= NULL;
    pstSelectClass->text_bck= NULL;
    rkgui_stop_timer(pstSelectClass);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_ItemDisplay
** Input: HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_ItemDisplay(HGC pGc, int Cursor, RKGUI_SELECT_ITEM * item)
{
    RKGUI_TEXT_ARG pstTextArg;
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;
    GUI_TEXT_CLASS pstTextClass;
    Ucs2 * text;
    int x, y, xSize, ySize;
    PICTURE_INFO_STRUCT psPictureInfo;

    x= pstClass->pfControl.x0+ pstClass->IconAreaStart+ pstClass->IconAreaSize;
    y= pstClass->pfControl.y0+ _SELECT_MARGIN_TOP_+ _SELECT_MARGIN_CUTOFF_ + pstClass->ItemTopMargin + (_SELECT_MARGIN_CUTOFF_+ pstClass->ItemHeight)* Cursor;
    xSize= pstClass->ItemWidth- 6;
    ySize= 12;

    if( item!= NULL )
    {
        if( item->sel_icon>= 0 && Cursor== pstClass->Cursor )
        {
            if( FW_GetPicInfoWithIDNum(item->sel_icon, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }

            if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay, pstClass->IconAreaStart, y, -1, -1, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
        }
        else if(item->unsel_icon>= 0 )
        {
            if( FW_GetPicInfoWithIDNum(item->unsel_icon, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }

            if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay, pstClass->IconAreaStart, y, -1, -1, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_SUCCESS;
            }
        }

        if(item->cmd==TEXT_CMD_ID)
        {
            if( GUI_TextDisplayID(pstClass->pfControl.hDisplay, x, y, xSize, ySize, item->text_id, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
            {
                return RK_SUCCESS;
            }
        }
        else
        {
            if( pstClass->text!= NULL && Cursor== pstClass->Cursor)
            {
                if( GUI_TextDisplayBuff(pstClass->pfControl.hDisplay, x, y, xSize, ySize, pstClass->text, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
                {
                     return RK_ERROR;
                }
            }
            else
            {
                if( GUI_TextDisplayBuff(pstClass->pfControl.hDisplay, x, y, xSize, ySize, item->text, TYPE12X12, 0, 0, 0, 0)!= RK_SUCCESS )
                {
                     return RK_ERROR;
                }
            }
        }
        return RK_SUCCESS;
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: GUI_CellCursorDisplay
** Input: HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_ClearBackCursor(HDC hDisplay, int x0, int y0, int limit, int Cursor, int Background, int ItemHeight, int IconAreaSize, int ItemWidth, int ItemTopMargin)
{
    int y;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( Cursor >= 0 && Cursor< limit)
    {
        y= _SELECT_MARGIN_TOP_ + ItemTopMargin + (_SELECT_MARGIN_CUTOFF_ + ItemHeight) * Cursor;
    }
    else if( Cursor >= limit)
    {
        y= _SELECT_MARGIN_TOP_ + ItemTopMargin;
    }
    else
    {
        y= _SELECT_MARGIN_TOP_ + ItemTopMargin + (_SELECT_MARGIN_CUTOFF_ + ItemHeight) * limit - 1;
    }

    if( FW_GetPicInfoWithIDNum(Background, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }
    return GUI_IconAreaUpdata(hDisplay, x0, y+ y0, IconAreaSize+ ItemWidth, ItemHeight- _SELECT_MARGIN_CUTOFF_, x0, y, &psPictureInfo);;
}

/*******************************************************************************
** Name: GUI_CellCursorDisplay
** Input: HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_CellCursorDisplay(HDC hDisplay, int x0, int y0, int Cursor, int ItemHeight, int IconAreaSize, int CursorStyle)
{
    int x, y;
    PICTURE_INFO_STRUCT psPictureInfo;
    x= IconAreaSize+ x0;
    y= _SELECT_MARGIN_TOP_ + _SELECT_MARGIN_CUTOFF_ + 1 + (_SELECT_MARGIN_CUTOFF_ + ItemHeight) * Cursor+ y0;

    if( FW_GetPicInfoWithIDNum(CursorStyle, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    return GUI_IconDisplayResource(hDisplay, x, y, -1, -1, &psPictureInfo);
}

/*******************************************************************************
** Name: GUI_SelectDrag
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectDrag(HGC pGc)
{
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( pstClass->BarYScale== 0 )
    {
        return RK_SUCCESS;
    }

    pstClass->BarY= (int)((pstClass->BarYScale* (pstClass->CursorOffset- pstClass->Cursor))+ pstClass->pfControl.y0);

    if( FW_GetPicInfoWithIDNum(pstClass->SeekBar, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }
    return GUI_IconAreaUpdata(pstClass->pfControl.hDisplay, pstClass->BarX , pstClass->BarY, pstClass->BarXSize, pstClass->BarYSize,
        0, 0, &psPictureInfo);
}
/*******************************************************************************
** Name: GUI_SelectClearDrag
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectClearDrag(HGC pGc)
{
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( pstClass->BarYScale== 0 )
    {
        return RK_SUCCESS;
    }

    if( FW_GetPicInfoWithIDNum(pstClass->Background, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    return GUI_IconAreaUpdata(pstClass->pfControl.hDisplay, pstClass->BarX , pstClass->BarY, pstClass->BarXSize, pstClass->BarYSize,
        pstClass->BarX- pstClass->pfControl.x0, pstClass->BarY- pstClass->pfControl.y0, &psPictureInfo);
}
/*******************************************************************************
** Name: GUI_SelectCallBackApp
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SelectCallBackApp(HGC pGc)
{
    int i= 0;
    Ucs2 * ntext;
    GUI_SELECT_CLASS * pstClass = (GUI_SELECT_CLASS *)pGc;

    if( pstClass->pList[pstClass->pListCurrent].text != NULL)
    {
        rkos_memory_free(pstClass->pList[pstClass->pListCurrent].text);
        pstClass->pList[pstClass->pListCurrent].text = NULL;
    }

    pstClass->pList[pstClass->pListCurrent].text = rkos_memory_malloc(RKGUI_MAX_RESOURCE * sizeof(Ucs2));
    if(pstClass->pList[pstClass->pListCurrent].text == NULL)
    {
        return RK_SUCCESS;
    }

    pstClass->pReviceMsg(pstClass, SELECT_ENVEN_UPDATA, &pstClass->pList[pstClass->pListCurrent], pstClass->CursorOffset);

    if( pstClass->pList[pstClass->pListCurrent].cmd==TEXT_CMD_BUF )
    {
        while( pstClass->pList[pstClass->pListCurrent].text[i++]!= 0 );//changed cjh

        ntext= rkos_memory_malloc(i * sizeof(Ucs2));
        if(ntext == NULL)
        {
            return RK_ERROR;
        }

        if( pstClass->pList[pstClass->pListCurrent].text != NULL)
        {
            memcpy(ntext, pstClass->pList[pstClass->pListCurrent].text, i* sizeof(Ucs2));
            rkos_memory_free(pstClass->pList[pstClass->pListCurrent].text);
            pstClass->pList[pstClass->pListCurrent].text = NULL;

        }
        pstClass->pList[pstClass->pListCurrent].text= ntext;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SelectCallBack
** Input:uint32 evnet_type, uint32 event, void * arg, uint32 mode
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SelectCallBack(uint32 evnet_type, uint32 event, void * arg, HGC pGc)
{
    int sel_icon;
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;

    if (evnet_type!= GUI_EVENT_KEY)
    {
        return RK_SUCCESS;
    }
    if( pstClass->pfControl.display==0 )
    {
        return RK_SUCCESS;
    }

    switch( event )
    {
        case KEY_VAL_UP_SHORT_UP:
        case KEY_VAL_UP_LONG_UP:
        case KEY_VAL_UP_PRESS:
        case KEY_VAL_UP_PRESS_START:
            GUI_SelectStopTimer(pstClass);
            if( GUI_ClearBackCursor(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0,
            pstClass->limit, pstClass->Cursor, pstClass->Background, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->ItemWidth, pstClass->ItemTopMargin)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }

            if(pstClass->itemSum== 0)
            {
                return RK_SUCCESS;
            }

            sel_icon= pstClass->pList[pstClass->pListCurrent].sel_icon;
            pstClass->pList[pstClass->pListCurrent].sel_icon= -1;
            if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
            pstClass->pList[pstClass->pListCurrent].sel_icon= sel_icon;

            pstClass->CursorOffset--;
            pstClass->Cursor--;
            pstClass->pListCurrent--;

            if(pstClass->pListCurrent< 0)
            {
                pstClass->pListCurrent= pstClass->limit- 1;
            }
            if( pstClass->Cursor < 0 )
            {
                if (pstClass->CursorOffset < 0 )
                {
                    pstClass->CursorOffset= pstClass->itemSum- 1;
                    pstClass->Cursor= pstClass->limit-1;

                    GUI_SelectClearDrag(pGc);
                    GUI_SelectDrag(pstClass);

                    if(pstClass->itemSum > pstClass->MaxDisplayItem)
                    {
                        pstClass->pDisplayCurrent = 0;

                        for(pstClass->pListCurrent= pstClass->limit-1; pstClass->pListCurrent> 0; pstClass->pListCurrent--)
                        {
                            GUI_SelectCallBackApp(pstClass);
                            pstClass->CursorOffset--;
                        }

                        GUI_SelectCallBackApp(pstClass);

                        pstClass->CursorOffset= pstClass->itemSum- 1;
                        pstClass->pListCurrent= pstClass->Cursor;
                        pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);

                        GUI_SelectStartTimer(pstClass);

                        GcRelevanceOper(pstClass);
                        if( GcDisplay(pstClass)!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                    }
                    else
                    {
                        pstClass->pDisplayCurrent = 0;
                        pstClass->pListCurrent= pstClass->limit-1;
                        GUI_SelectStartTimer(pstClass);
                        if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                        if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                        pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
                    }
                }
                else
                {
                    GUI_SelectClearDrag(pGc);

                    pstClass->Cursor= 0;
                    if( pstClass->pDisplayCurrent== 0 )
                    {
                        pstClass->pDisplayCurrent= pstClass->limit- 1;
                    }
                    else
                    {
                        pstClass->pDisplayCurrent--;
                    }

                    DisplayDev_for_Select(pstClass->pfControl.hDisplay,_SELECT_MARGIN_TOP_+ _SELECT_MARGIN_CUTOFF_+ pstClass->ItemHeight + pstClass->pfControl.y0, _SELECT_MARGIN_TOP_+ pstClass->pfControl.y0, (pstClass->limit- 1)*  (_SELECT_MARGIN_CUTOFF_+pstClass->ItemHeight));
                    GUI_SelectDrag(pstClass);

                    GUI_SelectCallBackApp(pstClass);
                    pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);

                    GUI_SelectStartTimer(pstClass);
                    if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                    {
                        return RK_ERROR;
                    }
                    if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                    {
                        return RK_ERROR;
                    }
                }
            }
            else
            {
                GUI_SelectStartTimer(pstClass);
                if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                {
                    return RK_ERROR;
                }
                if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                {
                    return RK_ERROR;
                }
                pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
            }
            return RK_SUCCESS;

        case KEY_VAL_DOWN_SHORT_UP:
        case KEY_VAL_DOWN_LONG_UP:
        case KEY_VAL_DOWN_PRESS:
        case KEY_VAL_DOWN_PRESS_START:
            GUI_SelectStopTimer(pstClass);
            if(GUI_ClearBackCursor(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0,
            pstClass->limit, pstClass->Cursor, pstClass->Background, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->ItemWidth, pstClass->ItemTopMargin)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
            if(pstClass->itemSum== 0)
            {
                return RK_SUCCESS;
            }

            sel_icon= pstClass->pList[pstClass->pListCurrent].sel_icon;
            pstClass->pList[pstClass->pListCurrent].sel_icon= -1;
            if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
            {
                return RK_ERROR;
            }
            pstClass->pList[pstClass->pListCurrent].sel_icon= sel_icon;

            pstClass->CursorOffset++;
            pstClass->Cursor++;
            pstClass->pListCurrent++;

            if(pstClass->pListCurrent> pstClass->limit- 1)
            {
                pstClass->pListCurrent = 0;
            }

            if(pstClass->Cursor>= pstClass->limit)
            {
                if(pstClass->CursorOffset>= pstClass->itemSum )
                {
                    pstClass->Cursor= 0;
                    pstClass->CursorOffset= 0;

                    GUI_SelectClearDrag(pGc);
                    GUI_SelectDrag(pstClass);

                    if(pstClass->itemSum > pstClass->MaxDisplayItem)
                    {
                        for(pstClass->pListCurrent= 0; pstClass->pListCurrent< pstClass->limit; pstClass->pListCurrent++)
                        {
                            GUI_SelectCallBackApp(pstClass);
                            pstClass->CursorOffset++;
                        }
                        pstClass->CursorOffset= 0;

                        pstClass->pListCurrent= 0;
                        pstClass->pDisplayCurrent= 0;
                        pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
                        GUI_SelectStartTimer(pstClass);

                        GcRelevanceOper(pstClass);
                        if( GcDisplay(pstClass)!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                    }
                    else
                    {
                        pstClass->pListCurrent= 0;
                        pstClass->pDisplayCurrent= 0;
                        GUI_SelectStartTimer(pstClass);
                        if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                        if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                        {
                            return RK_ERROR;
                        }
                        pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
                    }
                }
                else
                {
                    GUI_SelectClearDrag(pGc);

                    pstClass->Cursor= pstClass->limit- 1;
                    pstClass->pDisplayCurrent= (pstClass->pDisplayCurrent+1)% pstClass->limit;

                    DisplayDev_for_Select(pstClass->pfControl.hDisplay,
                    _SELECT_MARGIN_TOP_+ pstClass->pfControl.y0,
                    _SELECT_MARGIN_TOP_+ _SELECT_MARGIN_CUTOFF_+ pstClass->ItemHeight + pstClass->pfControl.y0,
                    ((pstClass->limit- 1)* (_SELECT_MARGIN_CUTOFF_+pstClass->ItemHeight)));

                    GUI_SelectCallBackApp(pstClass);
                    pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
                    GUI_SelectStartTimer(pstClass);

                    GUI_SelectDrag(pstClass);

                    if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor,pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                    {
                        return RK_ERROR;
                    }
                    if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                    {
                        return RK_ERROR;
                    }
                }
            }
            else
            {
                GUI_SelectStartTimer(pstClass);
                if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
                {
                    return RK_ERROR;
                }
                if( GUI_ItemDisplay(pstClass, pstClass->Cursor, &pstClass->pList[pstClass->pListCurrent])!= RK_SUCCESS )
                {
                    return RK_ERROR;
                }
                pstClass->pReviceMsg(pstClass, SELECT_ENVEN_MOVE, NULL, pstClass->CursorOffset);
            }
            return RK_SUCCESS;

        case KEY_VAL_MENU_SHORT_UP:
            pstClass->pReviceMsg(pstClass, SELECT_ENVEN_ENTER, (void *)pstClass->Cursor, pstClass->CursorOffset);
            return RK_SUCCESS;

        default:
            GUI_CallBackApp(APP_RECIVE_MSG_EVENT_KEY, event, NULL, pstClass);
            return RK_SUCCESS;
    }

    return RK_ERROR;
}

/*******************************************************************************
** Name: GUI_SelectSetContent
** Input:,RKGUI_SELECT_CONTENT * parameter
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SelectSetContent(HGC pGc ,RKGUI_SELECT_CONTENT * parameter)
{
    int button, Offset;
    int num;

    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;

    if (pstClass->MaxDisplayItem< parameter->itemNum)
    {
        pstClass->limit= pstClass->MaxDisplayItem;
    }
    else
    {
        pstClass->limit= parameter->itemNum;
    }

    button= pstClass->CursorOffset+ (pstClass->limit- pstClass->Cursor- 1);
    if( (parameter->itemNum- pstClass->itemSum)< 0 )
    {
        Offset= pstClass->CursorOffset;
        pstClass->CursorOffset-= pstClass->Cursor;

        for(pstClass->pListCurrent= 0; pstClass->pListCurrent< pstClass->limit; pstClass->pListCurrent++)
        {
            GUI_SelectCallBackApp(pstClass);
            pstClass->CursorOffset++;
        }
        pstClass->pDisplayCurrent= 0;
        pstClass->CursorOffset= Offset;
        pstClass->pListCurrent= pstClass->Cursor;
        pstClass->itemSum= parameter->itemNum;
    }
    else
    {
        if( pstClass->Cursor >= pstClass->limit )
        {
            pstClass->Cursor= pstClass->limit- 1;
            pstClass->CursorOffset= parameter->itemNum- 1;

            Offset= pstClass->CursorOffset;

            pstClass->CursorOffset-= pstClass->limit- 1;
            for(pstClass->pListCurrent= 0; pstClass->pListCurrent< pstClass->limit; pstClass->pListCurrent++)
            {
                GUI_SelectCallBackApp(pstClass);
                pstClass->CursorOffset++;
            }
            pstClass->pDisplayCurrent= 0;
            pstClass->CursorOffset= Offset;
            pstClass->pListCurrent= pstClass->Cursor;
            pstClass->itemSum= parameter->itemNum;
        }
        else
        {
            if( button >= parameter->itemNum- 1 )
            {
                pstClass->CursorOffset= parameter->itemNum- pstClass->limit;
                Offset= pstClass->CursorOffset+ pstClass->Cursor;
            }
            else
            {
                Offset= pstClass->CursorOffset;
                pstClass->CursorOffset= pstClass->CursorOffset- pstClass->Cursor;
            }

            for(pstClass->pListCurrent= 0; pstClass->pListCurrent< pstClass->limit && pstClass->CursorOffset< parameter->itemNum; pstClass->pListCurrent++)
            {
                GUI_SelectCallBackApp(pstClass);
                pstClass->CursorOffset++;
            }
            pstClass->pDisplayCurrent= 0;
            pstClass->CursorOffset= Offset;
            pstClass->pListCurrent= pstClass->Cursor;
            pstClass->itemSum= parameter->itemNum;
        }
    }

    if( pstClass->itemSum<= pstClass->limit )
    {
        pstClass->BarYSize= pstClass->BarYSizeBck;
        pstClass->BarYScale= 0;
    }
    else
    {
        pstClass->BarYSize= pstClass->BarYSizeBck/ (pstClass->itemSum- pstClass->limit);
        if( pstClass->BarYSize< _SELECT_MIN_SEEKBAR_SIZE_ )
        {
            pstClass->BarYSize= _SELECT_MIN_SEEKBAR_SIZE_;
        }
        pstClass->BarYScale= (float)(pstClass->BarYSizeBck- pstClass->BarYSize)/ (pstClass->itemSum- pstClass->limit);
    }

    pstClass->text= NULL;
    pstClass->text_bck= NULL;
    rkgui_stop_timer(pstClass);

    GcRelevanceOper(pstClass);
    GcDisplay(pstClass);

    GUI_SelectStartTimer(pstClass);
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
** Name: GUI_TextIsrUnRegister
** Input:GUI_TEXT_CLASS *pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t DisplayMenuStrWithIDNum(HGC pGc)
{
    int i= 0, Current;

    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;

    Current= pstClass->pDisplayCurrent;
    for(i= 0; i< pstClass->limit; i++)
    {
        GUI_ItemDisplay(pstClass, i, &pstClass->pList[Current] );
        Current= (Current+1)% pstClass->limit;
    }
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: GUI_SelectDisplay
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.20
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SelectDisplay(HGC pGc)
{
    GUI_SELECT_CLASS * pstClass= (GUI_SELECT_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( pstClass->KeepBackground )
    {
        if(GUI_ClearBackCursor(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0,
        pstClass->limit, pstClass->Cursor, pstClass->Background, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->ItemWidth, pstClass->ItemTopMargin)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
    }
    else
    {
        if( FW_GetPicInfoWithIDNum(pstClass->Background, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
        if( GUI_IconDisplayResource(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, -1, -1, &psPictureInfo)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
    }

    if( pstClass->itemSum== 0 )
    {
        return RK_SUCCESS;
    }

    if( GUI_SelectClearDrag(pGc)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if( GUI_SelectDrag(pstClass)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    if( GUI_CellCursorDisplay(pstClass->pfControl.hDisplay, pstClass->pfControl.x0, pstClass->pfControl.y0, pstClass->Cursor, pstClass->ItemHeight, pstClass->IconAreaSize, pstClass->CursorStyle)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    return DisplayMenuStrWithIDNum(pstClass);
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
** Name: GUI_SelectCreate
** Input:uint32 id, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.20
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_SelectCreate(void *arg)
{
    int i;
    int CursorOffset= 0;
    GUI_SELECT_CLASS * pstSelectClass;
    RKGUI_SELECT_ARG * pstSelectArg= (RKGUI_SELECT_ARG *)arg;
    PICTURE_INFO_STRUCT psPictureInfo;

    pstSelectClass=  rkos_memory_malloc(sizeof(GUI_SELECT_CLASS));
    if (pstSelectClass== NULL)
    {
        printf("GUI_SelectCreate: malloc fault\n");
        return NULL;
    }

    pstSelectClass->pfControl.ClassID = GUI_CLASS_SELECT;
    pstSelectClass->pfControl.x0 = pstSelectArg->x;
    pstSelectClass->pfControl.y0 = pstSelectArg->y;
    pstSelectClass->pfControl.xSize = pstSelectArg->xSize;
    pstSelectClass->pfControl.ySize = pstSelectArg->ySize;
    pstSelectClass->pfControl.display= pstSelectArg->display;

    if(pstSelectArg->level)
    {
        pstSelectClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstSelectClass->pfControl.hDisplay = hDisplay0;
    }

    pstSelectClass->MaxDisplayItem= pstSelectArg->MaxDisplayItem;
    pstSelectClass->Background= pstSelectArg->Background;
    pstSelectClass->CursorStyle= pstSelectArg->CursorStyle;
    pstSelectClass->SeekBar= pstSelectArg->SeekBar;
    pstSelectClass->ItemHeight= (pstSelectArg->ySize- _SELECT_MARGIN_TOP_) /  pstSelectArg->MaxDisplayItem;
    pstSelectClass->ItemTopMargin= (pstSelectClass->ItemHeight- 12)/ 2;

    pstSelectClass->IconAreaStart= 3;
    pstSelectClass->IconAreaSize= pstSelectArg->IconBoxSize;

    pstSelectClass->itemSum= pstSelectArg->itemNum;
    if (pstSelectClass->MaxDisplayItem< pstSelectClass->itemSum)
        pstSelectClass->limit= pstSelectClass->MaxDisplayItem;
    else
        pstSelectClass->limit= pstSelectClass->itemSum;

    if( FW_GetPicInfoWithIDNum(pstSelectArg->SeekBar, &psPictureInfo)== RK_SUCCESS )
    {
        pstSelectClass->BarX= pstSelectArg->xSize- psPictureInfo.xSize- 1+ pstSelectArg->x;
        pstSelectClass->BarXSize= psPictureInfo.xSize;
        pstSelectClass->BarYSizeBck= psPictureInfo.ySize;

        if( pstSelectClass->itemSum<= pstSelectClass->limit )
        {
            pstSelectClass->BarYSize= pstSelectClass->BarYSizeBck;
            pstSelectClass->BarYScale= 0;
        }
        else
        {
            pstSelectClass->BarYSize= pstSelectClass->BarYSizeBck/ (pstSelectClass->itemSum- pstSelectClass->limit);
            if( pstSelectClass->BarYSize< _SELECT_MIN_SEEKBAR_SIZE_ )
            {
                pstSelectClass->BarYSize= _SELECT_MIN_SEEKBAR_SIZE_;
            }
            pstSelectClass->BarYScale= (float)(pstSelectClass->BarYSizeBck- pstSelectClass->BarYSize)/ (pstSelectClass->itemSum- pstSelectClass->limit);
        }
    }
    pstSelectClass->ItemWidth=  pstSelectArg->xSize- pstSelectArg->IconBoxSize - psPictureInfo.xSize;

    pstSelectClass->pDisplayCurrent= 0;
    pstSelectClass->Cursor= pstSelectArg->Cursor;
    pstSelectClass->CursorOffset= pstSelectArg->ItemStartOffset;
    CursorOffset= pstSelectArg->ItemStartOffset;

    pstSelectClass->pReviceMsg= pstSelectArg->pReviceMsg;

    pstSelectClass->pList= rkos_memory_malloc(pstSelectClass->MaxDisplayItem* sizeof(RKGUI_SELECT_ITEM));//pstSelectClass->limit
    if (pstSelectClass== NULL)
    {
        goto err;
    }
    for(i= 0; i< pstSelectClass->MaxDisplayItem; i++)
    {
        pstSelectClass->pList[i].text= NULL;
    }
    for(pstSelectClass->pListCurrent= 0; pstSelectClass->pListCurrent< pstSelectClass->limit; pstSelectClass->pListCurrent++)
    {
        GUI_SelectCallBackApp(pstSelectClass);
        pstSelectClass->CursorOffset++;
    }
    pstSelectClass->pListCurrent= pstSelectClass->Cursor;
    pstSelectClass->CursorOffset= CursorOffset+ pstSelectClass->Cursor;

    pstSelectClass->pfControl.translucence= 0;
    pstSelectClass->pfControl.lucency= OPACITY;
    pstSelectClass->pfControl.flag= 0;
    pstSelectClass->KeepBackground= 0;
    rkgui_create_timer(pstSelectClass, GUI_SelectTimer);
    GUI_SelectStartTimer(pstSelectClass);

    if( pstSelectClass->pfControl.display )
    {
        GUI_SelectDisplay(pstSelectClass);
    }
    GUI_Createfocus(GUI_SelectCallBack, pstSelectClass);
    GUI_Setfocus(pstSelectClass);
    return &pstSelectClass->pfControl;

err:
    GUI_Deletefocus(pstSelectClass);
    if (pstSelectClass->pfControl.hDisplay!= NULL)RKDev_Close(pstSelectClass->pfControl.hDisplay);
    if (pstSelectClass->pList!= NULL)rkos_memory_free(pstSelectClass->pList);
    if (pstSelectClass!= NULL)rkos_memory_free(pstSelectClass);
    return NULL;
}

/*******************************************************************************
** Name: GUI_SelectDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.20
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SelectDelete(HGC pGc)
{
    int i;
    GUI_SELECT_CLASS * pstSelectClass= (GUI_SELECT_CLASS *)pGc;

    if ( pstSelectClass== NULL )
    {
        return RK_ERROR;
    }
    rkgui_delete_timer(pstSelectClass);

    GUI_Deletefocus(pstSelectClass);
    if (pstSelectClass->pList!= NULL)
    {
        for(i= 0; i< pstSelectClass->limit; i++)
        {
            if(pstSelectClass->pList[i].text!= NULL)
            {
                rkos_memory_free(pstSelectClass->pList[i].text);
            }
        }
        rkos_memory_free(pstSelectClass->pList);
    }
    if (pstSelectClass!= NULL)
    {
        rkos_memory_free(pstSelectClass);
    }

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

