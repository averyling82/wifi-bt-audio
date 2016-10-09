/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIManager.c
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
#ifdef __GUI_GUIMANAGER_C__
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
#include "GUIText.h"
#include "GUISelect.h"
#include "GUIMsgBox.h"
#include "GUIChain.h"
#include "GUIProgressBar.h"
#include "GUISpectrum.h"

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
extern rk_err_t GcRelevanceRegister(GUI_CONTROL_CLASS *pGc);
extern rk_err_t GcRelevanceUnRegister(GUI_CONTROL_CLASS *pGc);
extern rk_err_t GcRelevanceOper(GUI_CONTROL_CLASS * pGc);


extern GUI_CONTROL_CLASS *pfGcList;
extern GUI_CONTROL_CLASS *pfGcListTop;
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
** Name: GUI_DisplayWidge
** Input:HGC hGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_DisplayWidge(HGC hGc)
{
   rk_err_t ret;
   GUI_CONTROL_CLASS *pGc= (GUI_CONTROL_CLASS *)hGc;

    if (pGc== NULL)
    {
        return RK_ERROR;
    }

    switch ( pGc->ClassID )
    {
        case GUI_CLASS_ICON:
            ret= GUI_IconDisplay(hGc);
            break;

        case GUI_CLASS_IMAGE:
            ret= GUI_ImageDisplay(hGc);
            break;

        case GUI_CLASS_TEXT:
            ret= GUI_TextDisplay(hGc);
            break;

        case GUI_CLASS_MSG_BOX:
            ret= GUI_MsgBoxDisplay(hGc);
            break;

        case GUI_CLASS_SELECT:
            ret= GUI_SelectDisplay(hGc);
            break;

        case GUI_CLASS_CHAIN:
            ret= GUI_ChainDisplay(hGc);
            break;

        case GUI_CLASS_PROGRESSBAR:
            ret= GUI_ProgressBarDisplay(hGc);
            break;

        case GUI_CLASS_SPECTRUM:
            ret= GUI_SpectrumDisplay(hGc);
            break;
    }
    return ret;
}
/*******************************************************************************
** Name: GUI_SetDisplay
** Input:HGC hGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_SetDisplay(HGC hGc, int arg)
{
    rk_err_t ret;
    GUI_CONTROL_CLASS *pGc= (GUI_CONTROL_CLASS *)hGc;

    if(pGc->display && (int)arg)
    {
        ret= RK_SUCCESS;
    }
    else if((pGc->display == 0) && ((int)arg == 0))
    {
        ret= RK_SUCCESS;
    }
    else if((pGc->display == 0) && ((int)arg == 1))
    {
        pGc->display = (int)arg;
        GcRelevanceOper(pGc);
        if( GcDisplay(pGc)!= RK_SUCCESS )
        {
            pGc->display= 0;
            ret= RK_ERROR;
        }
    }
    else if((pGc->display == 1) && ((int)arg == 0))
    {
        uint32 lucency;

        pGc->display = (int)arg;
        lucency = pGc->lucency;
        pGc->lucency = LUCENCY;
        GcRelevanceOper(pGc);
        if( GcDisplay(pGc)!= RK_SUCCESS )
        {
            pGc->display= 1;
            ret= RK_ERROR;
        }
        pGc->lucency = lucency;
    }

    return ret;
}

/*******************************************************************************
** Name: GUI_OperWidge
** Input:HGC hGc, eICON_OPERATE_CMD cmd, void* arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_OperWidge(HGC hGc, eRKGUI_OPERATE_CMD cmd, void* arg)
{
    rk_err_t ret;
    GUI_CONTROL_CLASS *pGc= (GUI_CONTROL_CLASS *)hGc;

    if (pGc== NULL)
    {
        return RK_ERROR;
    }

    switch ( pGc->ClassID )
    {
        case GUI_CLASS_ICON:
        {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_IconDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;

                case OPERATE_SET_CONTENT:
                {
                    if( GUI_IconSetID(pGc,(uint32)arg)!= RK_SUCCESS )
                    {
                        ret= RK_ERROR;
                    }

                    if(pGc->display)
                    {
                        GcRelevanceOper(pGc);
                        ret= GcDisplay(pGc);
                    }
                    break;
                }
            }
            break;
        }

        case GUI_CLASS_IMAGE:
        {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_ImageDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;
            }
            break;
        }

        case GUI_CLASS_TEXT:
        {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_TextDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;

                case OPERATE_SET_CONTENT:
                {
                    ret= GUI_TextSetContent(pGc,arg);
                    break;
                }

                case OPERATE_START_MOVE:
                {
                    if(pGc->display)
                    {
                        ret= GUI_TextSetMove(hGc, (int)arg);
                    }
                }
                break;
            }
            break;
        }

        case GUI_CLASS_CHAIN:
        {
            switch (cmd)
            {
                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;
            }
            break;
       }

       case GUI_CLASS_MSG_BOX:
       {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_MsgBoxDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;
            }
            break;
        }

        case GUI_CLASS_PROGRESSBAR:
        {
            switch (cmd)
            {
                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;

                case OPERATE_SET_CONTENT:
                    GUI_ProgressBarContent(pGc, arg);
                    break;
            }
            break;
        }

        case GUI_CLASS_SELECT:
        {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_SelectDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;

                case OPERATE_SET_CONTENT:
                {
                    RKGUI_SELECT_CONTENT * parameter= (RKGUI_SELECT_CONTENT *)arg;
                    ret= GUI_SelectSetContent(hGc, parameter);
                    break;
                }
            }
            break;
        }

        case GUI_CLASS_SPECTRUM:
        {
            switch (cmd)
            {
                case OPERATE_DISPLAY:
                    ret= GUI_SpectrumDisplay(hGc);
                    break;

                case OPERATE_SET_DISPLAY:
                    ret= GUI_SetDisplay(hGc,(int)arg);
                    break;

                case OPERATE_SET_CONTENT:
                    ret= GUI_SpectrumContent(hGc, arg);
                    break;
            }
            break;
        }
    }
    return ret;
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
** Name: GUI_GcCreate
** Input:eGc eGcId, uint32 id, void* arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_GcCreate(eGc eGcId, void* arg)
{
    GUI_CONTROL_CLASS * pGUI= NULL;

    switch ( eGcId )
    {
        case GUI_CLASS_IMAGE:
            pGUI= GUI_ImageCreate(arg);
            break;

        case GUI_CLASS_ICON:
            pGUI= GUI_IconCreate(arg);
            break;

        case GUI_CLASS_TEXT:
            pGUI= GUI_TextCreate(arg);
            break;

        case GUI_CLASS_CHAIN:
            pGUI= GUI_ChainCreate(arg);
            break;

        case GUI_CLASS_MSG_BOX:
            pGUI= GUI_MsgBoxCreate(arg);
            break;

        case GUI_CLASS_SELECT:
            pGUI= GUI_SelectCreate(arg);
            break;

        case GUI_CLASS_PROGRESSBAR:
            pGUI= GUI_ProgressBarCreate(arg);
            break;

        case GUI_CLASS_SPECTRUM:
            pGUI= GUI_SpectrumCreate(arg);
            break;

        default:
            printf("nonsupport class id\n");
            return NULL;
    }

    GcRelevanceRegister(pGUI);
    return pGUI;
}

/*******************************************************************************
** Name: GUI_GcDelete
** Input:HGC hGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.15
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_GcDelete(HGC hGc)
{
    GUI_CONTROL_CLASS * pGUI= (GUI_CONTROL_CLASS *)hGc;
    rk_err_t ret= RK_ERROR;

    if (pGUI== NULL)
    {
        return RK_ERROR;
    }

    GcRelevanceUnRegister(pGUI);

    switch ( pGUI->ClassID )
    {
        case GUI_CLASS_ICON:
            ret= GUI_IconDelete(hGc);
            break;

        case GUI_CLASS_IMAGE:
            ret= GUI_ImageDelete(hGc);
            break;

        case GUI_CLASS_TEXT:
            ret= GUI_TextDelete(hGc);
            break;

        case GUI_CLASS_CHAIN:
            ret= GUI_ChainDelete(hGc);
            break;

        case GUI_CLASS_MSG_BOX:
            ret= GUI_MsgBoxDelete(hGc);
            break;

        case GUI_CLASS_SELECT:
            ret= GUI_SelectDelete(hGc);
            break;

        case GUI_CLASS_PROGRESSBAR:
            ret= GUI_ProgressBarDelete(hGc);
            break;

        case GUI_CLASS_SPECTRUM:
            ret= GUI_SpectrumDelete(hGc);
            break;

        default:
            break;
    }
    return ret;
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

