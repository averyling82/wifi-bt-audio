/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUISpectrum.c
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
#ifdef __GUI_GUISPECTRUM_C__

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
#include "GUISpectrum.h"


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef struct _GUI_SPECTRUM_CLASS
{
    GUI_CONTROL_CLASS pfControl;

    int StripStyle;
    int Backdrop;
    int BackdropX;
    int BackdropY;
    int StripNum;
    uint8 * StripDat;
    uint8 * StripBuf;
    int StripXSize;
    int StripYSize;
}GUI_SPECTRUM_CLASS;

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
** Name: GUI_SpectrumContent
** Input:HGC pGc, void * arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SpectrumContent(HGC pGc, void * arg)
{
    int i;
    GUI_SPECTRUM_CLASS * pstSpectrumClass= (GUI_SPECTRUM_CLASS *)pGc;
    RKGUI_SPECTRUM_ARG * pstSpectrumArg= (RKGUI_SPECTRUM_ARG *)arg;
    PICTURE_INFO_STRUCT psPictureInfo;

    if( pstSpectrumClass== NULL || pstSpectrumArg== NULL )
    {
        return RK_ERROR;
    }

    if( FW_GetPicInfoWithIDNum(pstSpectrumClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    GUI_IconAreaUpdata(pstSpectrumClass->pfControl.hDisplay, pstSpectrumClass->pfControl.x0, pstSpectrumClass->pfControl.y0,
                    pstSpectrumClass->pfControl.xSize, pstSpectrumClass->pfControl.ySize,
                    pstSpectrumClass->BackdropX, pstSpectrumClass->BackdropY, &psPictureInfo);

    if(  pstSpectrumArg->cmd==SPECTRUM_SET_CONTENT  )
    {
        for(i= 0; i< pstSpectrumClass->StripNum; i++)
        {
            pstSpectrumClass->StripDat[i]= pstSpectrumArg->StripDat[i];
        }
    }

    GcRelevanceOper(pGc);
    if( GcDisplay(pGc)!= RK_SUCCESS )
    {
        return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_SpectrumDisplay
** Input:HDC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SpectrumDisplay(HDC pGc)
{
    int x, y, i;
    GUI_SPECTRUM_CLASS * pstSpectrumClass= (GUI_SPECTRUM_CLASS *)pGc;

    x= pstSpectrumClass->pfControl.x0;

    for(i= 0; i< pstSpectrumClass->StripNum; i++)
    {
        if( pstSpectrumClass->StripDat[i] > pstSpectrumClass->StripYSize )
            pstSpectrumClass->StripDat[i] = pstSpectrumClass->StripYSize;

        y= pstSpectrumClass->pfControl.y0+ (pstSpectrumClass->StripYSize- pstSpectrumClass->StripDat[i]);

        if( pstSpectrumClass->StripDat[i]==0 )
        {
            x+= pstSpectrumClass->StripXSize;
            continue;
        }
        DisplayDev_SetWindow(pstSpectrumClass->pfControl.hDisplay, x, y, pstSpectrumClass->StripXSize, pstSpectrumClass->StripDat[i]);
        DisplayDev_Write(pstSpectrumClass->pfControl.hDisplay, &pstSpectrumClass->StripBuf[( pstSpectrumClass->StripYSize- pstSpectrumClass->StripDat[i])* pstSpectrumClass->StripXSize* ICON_PIXEL_WIDE], pstSpectrumClass->StripXSize* pstSpectrumClass->StripDat[i]* ICON_PIXEL_WIDE);
        x+= pstSpectrumClass->StripXSize;
    }
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
** Name: GUI_SpectrumCreate
** Input:void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_SpectrumCreate(void *arg)
{
    PICTURE_INFO_STRUCT psPictureInfo;
    GUI_SPECTRUM_CLASS * pstSpectrumClass;
    RKGUI_SPECTRUM_ARG * pstSpectrumArg= (RKGUI_SPECTRUM_ARG *)arg;

    pstSpectrumClass=  rkos_memory_malloc(sizeof(GUI_SPECTRUM_CLASS));
    if (pstSpectrumClass== NULL)
    {
        printf("GUI_SpectrumCreate: malloc GUI_SPECTRUM_CLASS fault\n");
        return NULL;
    }

    if(pstSpectrumArg->level)
    {
        pstSpectrumClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstSpectrumClass->pfControl.hDisplay = hDisplay0;
    }

    pstSpectrumClass->StripStyle= pstSpectrumArg->StripStyle;
    if( FW_GetPicInfoWithIDNum(pstSpectrumClass->StripStyle, &psPictureInfo)!= RK_SUCCESS )
    {
        return NULL;
    }

    pstSpectrumClass->StripXSize= psPictureInfo.xSize;
    pstSpectrumClass->StripYSize= psPictureInfo.ySize;

    pstSpectrumClass->StripBuf= (uint8 *)rkos_memory_malloc(psPictureInfo.xSize * psPictureInfo.ySize* 2);
    if(FW_GetPicResource(psPictureInfo.offsetAddr, pstSpectrumClass->StripBuf, psPictureInfo.totalSize)!= RK_SUCCESS)
    {
        RKDev_Close(pstSpectrumClass->pfControl.hDisplay);
        rkos_memory_free(pstSpectrumClass->StripBuf);
        return NULL;
    }

    pstSpectrumClass->pfControl.x0= pstSpectrumArg->x;
    pstSpectrumClass->pfControl.y0= pstSpectrumArg->y;
    pstSpectrumClass->pfControl.xSize= psPictureInfo.xSize* pstSpectrumArg->StripNum;
    pstSpectrumClass->pfControl.ySize= psPictureInfo.ySize;
    if(pstSpectrumArg->x< 0) pstSpectrumClass->pfControl.x0= psPictureInfo.x;
    if(pstSpectrumArg->y< 0) pstSpectrumClass->pfControl.y0= psPictureInfo.y;

    pstSpectrumClass->Backdrop= pstSpectrumArg->Backdrop;
    pstSpectrumClass->BackdropX= pstSpectrumArg->BackdropX;
    pstSpectrumClass->BackdropY= pstSpectrumArg->BackdropY;

    pstSpectrumClass->pfControl.display= pstSpectrumArg->display;
    pstSpectrumClass->pfControl.level= pstSpectrumArg->level;
    pstSpectrumClass->StripNum= pstSpectrumArg->StripNum;

    pstSpectrumClass->StripDat= (uint8 *)rkos_memory_malloc(pstSpectrumArg->StripNum * sizeof(uint8));
    if( pstSpectrumClass->StripDat== NULL )
    {
        RKDev_Close(pstSpectrumClass->pfControl.hDisplay);
        rkos_memory_free(pstSpectrumClass->StripBuf);
        rkos_memory_free(pstSpectrumClass);
        return NULL;
    }
    memcpy(pstSpectrumClass->StripDat, pstSpectrumArg->StripDat, pstSpectrumArg->StripNum * sizeof(uint8));

    pstSpectrumClass->pfControl.ClassID= GUI_CLASS_SPECTRUM;
    pstSpectrumClass->pfControl.rotate = ROTATE_NOMAL;
    pstSpectrumClass->pfControl.flag= 0;
    pstSpectrumClass->pfControl.lucency = OPACITY;
    pstSpectrumClass->pfControl.translucence= 0;

    if(pstSpectrumClass->pfControl.display)
    {
        GUI_SpectrumDisplay(pstSpectrumClass);
    }
    return &pstSpectrumClass->pfControl;
}

/*******************************************************************************
** Name: GUI_SpectrumDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.16
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_SpectrumDelete(HGC pGc)
{
    GUI_SPECTRUM_CLASS * pstSpectrumClass= (GUI_SPECTRUM_CLASS *)pGc;

    if( pstSpectrumClass->StripDat!= NULL )rkos_memory_free(pstSpectrumClass->StripDat);
    rkos_memory_free(pstSpectrumClass->StripBuf);
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

