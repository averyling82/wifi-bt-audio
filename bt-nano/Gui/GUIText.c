/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronic Co., Ltd
*                             All rights reserved.
*
* FileName: ..\Gui\GUIText.c
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
#ifdef __GUI_GUITEXT_C__

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
#include "GUIText.h"
#include "GUIIcon.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define _RKGUI_TEXT_EDIAN_
#define _RKGUI_TEXT_MIRROR_

#ifdef _RKGUI_TEXT_MIRROR_
    #define _RKGUI_TEXT_MASK_ 0x8000
#else
    #define _RKGUI_TEXT_MASK_ 0x0001
#endif

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
** Name: Font12_CompiType
** Input:uint8 * pBuf, short dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_GUI_GUITASK_COMMON_
static uint16 Font12_CompiType(unsigned short dat)
{
    uint32 CharInNFAddr;
    uint16 iType;

    CharInNFAddr = gstFwInf.Font12LogicAddress + (uint32)((uint32)dat<<5)+24;

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Font12== NULL)return 0xFFFF;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Font12, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Font12, (char *)&iType, 2);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, (char *)&iType, 2);
#endif
    return iType;
}


/*******************************************************************************
** Name: Font16_CompiType
** Input:uint8 * pBuf, short dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_GUI_GUITASK_COMMON_
static uint16 Font16_CompiType(unsigned short dat)
{
    uint32 CharInNFAddr;
    uint16 iType;

    CharInNFAddr = gstFwInf.Font16LogicAddress + (uint32)(((uint32)dat)*33) + 32;

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Font16== NULL)return 0xFFFF;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Font16, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Font16, (char *)&iType, 2);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, (char *)&iType, 2);
#endif
    return iType;
}

/*******************************************************************************
** Name: Font12_CompData
** Input:uint8 * pBuf, short dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_GUI_GUITASK_COMMON_
static uint16 Font12_CompData(uint8 * pBuf, unsigned short dat)
{
    uint32 CharInNFAddr;
    uint16 iType;

    CharInNFAddr = gstFwInf.Font12LogicAddress + (uint32)((uint32)dat<<5);

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Font12== NULL)return 0xFFFF;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Font12, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Font12, pBuf, 24);
    FileDev_ReadFile(gstSpiResoure.Font12, (char *)&iType, 2);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, pBuf, 24);
    FW_ReadFirmwaveByByte(CharInNFAddr + 24, (char *)&iType, 2);
#endif
    return iType;
}

/*******************************************************************************
** Name: Font16_CompData
** Input:uint8 * pBuf, short dat
** Return: uint16
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 13:53:09
*******************************************************************************/
_GUI_GUITASK_COMMON_
static uint16 Font16_CompData(uint8 * pBuf, unsigned short dat)
{
    uint32 CharInNFAddr;
    uint16 iType;

    CharInNFAddr = gstFwInf.Font16LogicAddress + (uint32)(((uint32)dat)*33);

#ifdef _SPI_BOOT_
    if(gstSpiResoure.Font16== NULL)return 0xFFFF;
    rkos_semaphore_take(gstSpiResoure.osReadSource, MAX_DELAY);
    FileDev_FileSeek(gstSpiResoure.Font16, SEEK_SET, CharInNFAddr);
    FileDev_ReadFile(gstSpiResoure.Font16, pBuf, 32);
    FileDev_ReadFile(gstSpiResoure.Font16, (char *)&iType, 2);
    rkos_semaphore_give(gstSpiResoure.osReadSource);
#endif

#ifdef _EMMC_BOOT_
    FW_ReadFirmwaveByByte(CharInNFAddr, pBuf, 32);
    FW_ReadFirmwaveByByte(CharInNFAddr + 32, (char *)&iType, 2);
#endif
    return iType;
}

/*******************************************************************************
** Name: GUI_TextGetText
** Input:HGC pGc, Ucs2 *pUni
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API int GUI_TextExtractSize(FONT_TYPE font, Ucs2 * text)
{
    uint16 iType;
    uint32 i= 0;
    uint32 xSize= 0;

    if(font==TYPE16X16)
    {
        while(text[i]!= 0)
        {
            iType= Font16_CompiType(text[i]);
            if( iType> 0x80 )
            {
                xSize+= iType- 0x80;
            }
            else if(iType== 0x80)
            {
                if( text[i]< 0x80 )xSize+= 8;
                else xSize+= 16;
            }
            else if(iType== 0xFFFF)
            {
                return -1;
            }
            else
            {
                xSize+= 16;
            }
            i++;
        }
    }

    else if(font==TYPE12X12)
    {
        while(text[i]!= 0)
        {
            iType= Font12_CompiType(text[i]);
            if( iType> 0x80 )
            {
                xSize+= iType- 0x80;
            }
            else if(iType== 0x80)
            {
                if( text[i]< 0x80 )xSize+= 6;
                else xSize+= 12;
            }
            else if(iType== 0xFFFF)
            {
                return -1;
            }
            else
            {
                xSize+= 12;
            }
            i++;
        }
    }

    return xSize;
}


/*******************************************************************************
** Name: GUI_TextAutoAlign
** Input:HGC pGc, Ucs2 *pUni
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextSizeCmp(FONT_TYPE font, Ucs2 * text, int size)
{
    uint16 iType;
    uint32 i= 0;
    uint32 xSize= 0;

    if(font==TYPE16X16)
    {
        while(text[i]!= 0)
        {
            iType= Font16_CompiType(text[i]);
            if( iType> 0x80 )
            {
                xSize+= iType- 0x80;
            }
            else if(iType== 0x80)
            {
                if( text[i]< 0x80 )xSize+= 8;
                else xSize+= 16;
            }
            else if(iType== 0xFFFF)
            {
                return RK_ERROR;
            }
            else
            {
                xSize+= 16;
            }
            i++;
            if( xSize > size )return RK_ERROR;
        }
    }
    else if(font==TYPE12X12)
    {
        while(text[i]!= 0)
        {
            iType= Font12_CompiType(text[i]);
            if( iType> 0x80 )
            {
                xSize+= iType- 0x80;
            }
            else if(iType== 0x80)
            {
                if( text[i]< 0x80 )xSize+= 6;
                else xSize+= 12;
            }
            else if(iType== 0xFFFF)
            {
                return -1;
            }
            else
            {
                xSize+= 12;
            }
            i++;
            if( xSize > size )return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextAutoAlign
** Input:HGC pGc, Ucs2 *pUni
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextAutoAlign(FONT_TYPE font, int align, int x0, int y0, int xSize, int ySize,
                            int * StartX, int * StartY, int *DestXSize, int *DestYSize, Ucs2 * text)
{
    int st_xSize, st_ySize;

    switch( align& TEXT_ALIGN_X_MASK )
    {
        case TEXT_ALIGN_X_LEFT:
            *StartX= x0;
            *DestXSize= xSize;
            break;

        case TEXT_ALIGN_X_RIGHT:
            st_xSize= GUI_TextExtractSize(font, text);

            if( st_xSize> xSize )
            {
                *StartX= x0;
                *DestXSize= xSize;
            }
            else
            {
                *StartX= xSize- st_xSize+ x0;
                *DestXSize= st_xSize;
            }
            break;

        case TEXT_ALIGN_X_CENTER:
            st_xSize= GUI_TextExtractSize(font, text);
            if( st_xSize> xSize )
            {
                *StartX= x0;
                *DestXSize= xSize;
            }
            else
            {
                *StartX= (xSize- st_xSize)/ 2+ x0;
                *DestXSize= st_xSize;
            }
            break;
    }

    switch( align& TEXT_ALIGN_Y_MASK )
    {
        case TEXT_ALIGN_Y_UP:
            *StartY= y0;
            *DestYSize= ySize;
            break;

        case TEXT_ALIGN_Y_DOWN:
            if( font== TYPE12X12 )
                st_ySize= 12;
            else
                st_ySize= 16;
            if( st_ySize> ySize )
            {
                *StartY= y0;
                *DestYSize= ySize;
            }
            else
            {
                *StartY= ySize- st_ySize+ y0;
                *DestYSize= st_ySize;
            }
            break;

        case TEXT_ALIGN_Y_CENTER:
            if( font== TYPE12X12 )
                st_ySize= 12;
            else
                st_ySize= 16;

            if( ySize> ySize )
            {
                *StartY= y0;
                *DestYSize= ySize;
            }
            else
            {
                *StartY= (ySize- st_ySize)/ 2+ y0;
                *DestYSize= st_ySize;
            }
            break;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextGetText
** Input:HGC pGc, Ucs2 *pUni
** Return: uint32
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API Ucs2 * GUI_TextExtractString(int resource)
{
    int num= 0;
    Ucs2 * pstText, *text;

    pstText= rkos_memory_malloc(RKGUI_MAX_RESOURCE* sizeof(Ucs2));

    if( FW_GetMenuResource(resource, pstText, RKGUI_MAX_RESOURCE)!= RK_SUCCESS )
    {
        return NULL;
    }

    while(pstText[num++]!= 0);
    text= rkos_memory_malloc(num * sizeof(Ucs2));
    memcpy((char *)text, (char *)pstText, num * sizeof(Ucs2));
    rkos_memory_free(pstText);

    return text;
}

/*******************************************************************************
** Name: GUI_TextTimer
** Input: pTimer timer
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.23
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
static COMMON API rk_err_t GUI_TextTimer(HGC pGc)
{
    RKGUI_ICON_ARG pstIconArg;
    HGC hBackdrop;
    int xSize,ySize;
    static int speed= 0;
    GUI_TEXT_CLASS * pstTextClass= (GUI_TEXT_CLASS *)pGc;

    if(pstTextClass->pfControl.display == 0)
    {
        return RK_SUCCESS;
    }

    if( speed++ < pstTextClass->speed )
    {
        return RK_SUCCESS;
    }
    speed= 0;

    if(*pstTextClass->text!= 0)
        pstTextClass->text++;
    else
        pstTextClass->text= pstTextClass->text_bck;

    if( pstTextClass->xAuto )
    {
        xSize= GUI_TextExtractSize(pstTextClass->font, pstTextClass->text);
        if(xSize < 0)
        {
            return RK_ERROR;
        }
        else if( xSize > pstTextClass->pfControl.xSize )
        {
            pstTextClass->pfControl.xSize = xSize;
        }
    }
    if( pstTextClass->yAuto )
    {
        pstTextClass->pfControl.ySize = 12;
    }

    GcRelevanceOper(pstTextClass);

    return GcDisplay(pGc);
}

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: GUI_TextEnableMove
** Input:HGC pGc,
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextSetMove(HGC pGc, int speed)
{
    GUI_TEXT_CLASS * pstTextClass= (GUI_TEXT_CLASS *)pGc;
    if (pstTextClass== NULL)
    {
        return RK_ERROR;
    }

    pstTextClass->speed= speed;
    rkgui_start_timer(pstTextClass);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_BmpTextDisplay
** Input:HGC pGc, short *pbuf, int xSize, int ySize, uint32 Mask
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_BmpFronDisplay(HDC hDisplay, void *pbuf,int x, int y, int xSize, int ySize, int R, int G, int B, int transparency)
{
    uint32 i,j;
    uint16 dat;

    DisplayDev_SetWindow(hDisplay, x, y, xSize, ySize);
    for (i=0 ;i< ySize ;i++)
    {
#ifdef _RKGUI_TEXT_EDIAN_
        dat= ((char *)pbuf)[i*2+1];
        dat|= (((char *)pbuf)[i*2])<<8;
#else
        dat= ((short *)pbuf)[i];
#endif
#ifdef _RKGUI_TEXT_MIRROR_
        for (j=0; j< xSize; j++)
        {
            if ( dat & _RKGUI_TEXT_MASK_>> j )
            {
                DisplayDev_SendData(hDisplay, R, G, B, transparency);
            }
            else DisplayDev_SetOffset(hDisplay);
        }
#else
        for (j=0; j< xSize; j++)
        {
            if ( dat & _RKGUI_TEXT_MASK_<< j )
            {
                DisplayDev_SendData(hDisplay, R, G, B, transparency);
            }
            else DisplayDev_SetOffset(hDisplay);
        }
#endif
    }
    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextDisplayBuff
** Input:HDC hDisplay, int x0, int y0, int x1, int y1, Ucs2 * text, FONT_TYPE font, int R, int G, int B, int transparency
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextDisplayBuff(HDC hDisplay, int x0, int y0, int x1, int y1, Ucs2 * text, FONT_TYPE font, int R, int G, int B, int transparency)
{
    uint16 iType;
    Ucs2 *pUni;
    int xSize, ySize;
    int CurrentX, CurrentY;
    char buf[32];

    CurrentX= x0;
    CurrentY= y0;
    pUni= text;
    while ( *pUni!= 0 )
    {
        if (font== TYPE16X16)
        {
            iType= Font16_CompData(buf, *pUni);
            if(iType> 0x80)
            {
                xSize= iType-0x80;
            }
            else if(iType== 0x80)
            {
                if( *pUni< 0x80 )xSize= 8;
                else xSize= 16;
            }
            else if(iType== 0xFFFF)
            {
                return RK_ERROR;
            }
            else
            {
                xSize= 16;
            }
            ySize= 16;
        }
        else if (font== TYPE12X12)
        {
            iType= Font12_CompData(buf, *pUni);
            if(iType> 0x80)
            {
                xSize= iType-0x80;
            }
            else if(iType== 0x80)
            {
                if( *pUni< 0x80 )xSize= 6;
                else xSize= 12;
            }
            else if(iType== 0xFFFF)
            {
                return RK_ERROR;
            }
            else
            {
                xSize= 12;
            }
            ySize= 12;
        }

        if( CurrentX - x0 + xSize> x1 )break;
        GUI_BmpFronDisplay(hDisplay, buf, CurrentX, CurrentY, xSize, ySize, R, G, B, transparency);
        CurrentX+= xSize;
        pUni++;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextDisplayID
** Input:HDC hDisplay, int x0, int y0, int x1, int y1, int id, FONT_TYPE font, int R, int G, int B, int transparency
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextDisplayID(HDC hDisplay, int x0, int y0, int x1, int y1, int id, FONT_TYPE font, int R, int G, int B, int transparency)
{
    Ucs2 * text;

    text= GUI_TextExtractString(id);
    if( text== NULL )
    {
        return RK_ERROR;
    }

    if(GUI_TextDisplayBuff(hDisplay, x0, y0, x1, y1, text, TYPE12X12, R, G, B, transparency)!= RK_SUCCESS )
    {
        rkos_memory_free(text);
        return RK_ERROR;
    }
    rkos_memory_free(text);

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: GUI_TextSetContent
** Input:HGC pGc, void * arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextSetContent(HGC pGc, void * arg)
{
    int xSize, ySize;
    GUI_TEXT_CLASS * pstTextClass= (GUI_TEXT_CLASS *)pGc;

    GUI_CONTROL_CLASS * pstGuiClass = (GUI_CONTROL_CLASS *)pGc;
    RKGUI_TEXT_ARG * parameter= (RKGUI_TEXT_ARG *)arg;

    if( parameter->opercmd==TEXT_SET_BACKGROUND)
    {
        if( parameter->Backdrop >= 0 )
        {
            pstTextClass->Backdrop= parameter->Backdrop;
            pstTextClass->BackdropX= parameter->BackdropX;
            pstTextClass->BackdropY= parameter->BackdropY;
        }

        pstTextClass->BackgroundR= parameter->BackgroundR;
        pstTextClass->BackgroundG= parameter->BackgroundG;
        pstTextClass->BackgroundB= parameter->BackgroundB;
    }
    else if(parameter->opercmd==TEXT_SET_COLOUR)
    {
        pstTextClass->ForegroundR= parameter->ForegroundR;
        pstTextClass->ForegroundG= parameter->ForegroundG;
        pstTextClass->ForegroundB= parameter->ForegroundB;
    }

    else if(parameter->opercmd==TEXT_SET_CONTENT)
    {
        if(pstTextClass->cmd== TEXT_CMD_ID)
        {
            if( pstTextClass->text_bck!= NULL )
            {
                rkos_memory_free(pstTextClass->text_bck);
            }

            pstTextClass->text= GUI_TextExtractString(parameter->resource);
            pstTextClass->text_bck= pstTextClass->text;
            if( pstTextClass->xAuto )
            {
                xSize= GUI_TextExtractSize(pstTextClass->font, pstTextClass->text);
                if( xSize > pstTextClass->pfControl.xSize)
                {
                    pstTextClass->pfControl.xSize= xSize;
                }
            }

            if( pstTextClass->yAuto )
            {
                pstTextClass->pfControl.ySize = 12;
            }
        }
        else
        {
            rkos_memory_free(pstTextClass->text_bck);

            pstTextClass->text = rkos_memory_malloc(StrLenW(parameter->text) * 2 + 2);

            pstTextClass->text_bck = pstTextClass->text;

            memcpy(pstTextClass->text, parameter->text, StrLenW(parameter->text) * 2 + 2);

            if( pstTextClass->xAuto )
            {
                xSize= GUI_TextExtractSize(pstTextClass->font, pstTextClass->text);
                if( xSize < 0 )
                {
                    return RK_ERROR;
                }
                else if( xSize > pstTextClass->pfControl.xSize )
                {
                    pstTextClass->pfControl.xSize = xSize;
                }
            }
            if( pstTextClass->yAuto )
            {
                pstTextClass->pfControl.ySize = 12;
            }
        }
    }
    else if(parameter->opercmd==TEXT_SET_ALIGN)
    {
        pstTextClass->align= parameter->align;
        GUI_TextAutoAlign(pstTextClass->font, pstTextClass->align,
                        pstTextClass->pfControl.x0, pstTextClass->pfControl.y0, pstTextClass->pfControl.xSize, pstTextClass->pfControl.ySize,
                        &pstTextClass->StartX, &pstTextClass->StartY, &pstTextClass->xSize, &pstTextClass->ySize, pstTextClass->text);
    }
    if(pstGuiClass->display)
    {
        GcRelevanceOper(pGc);
        if( GcDisplay(pGc)!= RK_SUCCESS )
        {
            return RK_ERROR;
        }
    }

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: GUI_TextDisplay
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextDisplay(HGC pGc)
{
    GUI_TEXT_CLASS * pstTextClass= (GUI_TEXT_CLASS *)pGc;
    PICTURE_INFO_STRUCT psPictureInfo;

    if (pstTextClass== NULL)
    {
        return RK_ERROR;
    }

    if( pstTextClass->pfControl.lucency == OPACITY )
    {
        if( pstTextClass->Backdrop>= 0 )
        {
            if( FW_GetPicInfoWithIDNum(pstTextClass->Backdrop, &psPictureInfo)!= RK_SUCCESS )
            {
                return RK_ERROR;
            }

            if( GUI_IconAreaUpdata(pstTextClass->pfControl.hDisplay, pstTextClass->pfControl.x0, pstTextClass->pfControl.y0, pstTextClass->pfControl.xSize,  pstTextClass->pfControl.ySize,
            pstTextClass->BackdropX, pstTextClass->BackdropY, &psPictureInfo)!= RK_SUCCESS )
                return RK_ERROR;
        }
        else
        {
            GUI_ImageDisplayRect(pstTextClass->pfControl.hDisplay, pstTextClass->pfControl.x0, pstTextClass->pfControl.y0, pstTextClass->pfControl.xSize,  pstTextClass->pfControl.ySize,
            pstTextClass->BackgroundR, pstTextClass->BackgroundG, pstTextClass->BackgroundB, 0);
        }
    }

    return GUI_TextDisplayBuff(pstTextClass->pfControl.hDisplay, pstTextClass->StartX, pstTextClass->StartY, pstTextClass->xSize,  pstTextClass->ySize,
        pstTextClass->text, pstTextClass->font, pstTextClass->ForegroundR , pstTextClass->ForegroundG, pstTextClass->ForegroundB, 0);
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
** Name: GUI_TextCreate
** Input:uint32 id, void *arg
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API GUI_CONTROL_CLASS * GUI_TextCreate(void *arg)
{
    GUI_TEXT_CLASS * pstTextClass;
    RKGUI_TEXT_ARG * pstTextArg= (RKGUI_TEXT_ARG *)arg;

    pstTextClass=  rkos_memory_malloc(sizeof(GUI_TEXT_CLASS));
    if (pstTextClass== NULL)
    {
        return NULL;
    }

    if(pstTextArg->level)
    {
        pstTextClass->pfControl.hDisplay = hDisplay1;
    }
    else
    {
        pstTextClass->pfControl.hDisplay = hDisplay0;
    }

    pstTextClass->pfControl.ClassID = GUI_CLASS_TEXT;
    pstTextClass->pfControl.x0= pstTextArg->x;
    pstTextClass->pfControl.y0= pstTextArg->y;
    pstTextClass->pfControl.xSize= pstTextArg->xSize;
    pstTextClass->pfControl.ySize= pstTextArg->ySize;

    if(pstTextClass->pfControl.xSize <= 0)
        pstTextClass->xAuto= 1;
    else
        pstTextClass->xAuto= 0;

    if(pstTextClass->pfControl.ySize <= 0)
        pstTextClass->yAuto= 1;
    else
        pstTextClass->yAuto= 0;

    pstTextClass->pfControl.display= pstTextArg->display;
    pstTextClass->pfControl.rotate= ROTATE_NOMAL;
    pstTextClass->pfControl.level= pstTextArg->level;

    pstTextClass->pfControl.lucency= pstTextArg->lucency;
    pstTextClass->Backdrop= pstTextArg->Backdrop;
    pstTextClass->BackdropX= pstTextArg->BackdropX;
    pstTextClass->BackdropY= pstTextArg->BackdropY;

    pstTextClass->ForegroundR = pstTextArg->ForegroundR;
    pstTextClass->ForegroundG = pstTextArg->ForegroundG;
    pstTextClass->ForegroundB = pstTextArg->ForegroundB;

    pstTextClass->BackgroundR = pstTextArg->BackgroundR;
    pstTextClass->BackgroundG = pstTextArg->BackgroundG;
    pstTextClass->BackgroundB = pstTextArg->BackgroundB;

    pstTextClass->cmd= pstTextArg->cmd;

    pstTextClass->font= TYPE12X12;
    pstTextClass->text= NULL;
    pstTextClass->text_bck= NULL;

    pstTextClass->pfControl.flag= 0;
    pstTextClass->pfControl.lucency= pstTextArg->lucency;
    pstTextClass->pfControl.translucence= 0;
    pstTextClass->align= pstTextArg->align;

    if(pstTextArg->resource>= 0 && pstTextArg->cmd== TEXT_CMD_ID)
    {
        pstTextClass->text= GUI_TextExtractString(pstTextArg->resource);
        pstTextClass->text_bck= pstTextClass->text;
        if( pstTextClass->xAuto )pstTextClass->pfControl.xSize = GUI_TextExtractSize(pstTextClass->font, pstTextClass->text);
        if( pstTextClass->yAuto )pstTextClass->pfControl.ySize = 12;

        GUI_TextAutoAlign(pstTextClass->font, pstTextClass->align,
                pstTextClass->pfControl.x0, pstTextClass->pfControl.y0, pstTextClass->pfControl.xSize, pstTextClass->pfControl.ySize,
                &pstTextClass->StartX, &pstTextClass->StartY, &pstTextClass->xSize, &pstTextClass->ySize, pstTextClass->text);
    }
    else if(pstTextClass->pfControl.display && pstTextArg->text!= NULL)
    {
        pstTextClass->text = rkos_memory_malloc(StrLenW(pstTextArg->text) * 2 + 2);

        pstTextClass->text_bck = pstTextClass->text;

        memcpy(pstTextClass->text, pstTextArg->text, StrLenW(pstTextArg->text) * 2 + 2);

        if( pstTextClass->xAuto )pstTextClass->pfControl.xSize = GUI_TextExtractSize(pstTextClass->font, pstTextClass->text);
        if( pstTextClass->yAuto )pstTextClass->pfControl.ySize = 12;

        GUI_TextAutoAlign(pstTextClass->font, pstTextClass->align,
                pstTextClass->pfControl.x0, pstTextClass->pfControl.y0, pstTextClass->pfControl.xSize, pstTextClass->pfControl.ySize,
                &pstTextClass->StartX, &pstTextClass->StartY, &pstTextClass->xSize, &pstTextClass->ySize, pstTextClass->text);
    }

    rkgui_create_timer(pstTextClass, GUI_TextTimer);

    if(pstTextClass->pfControl.display)
    {
        GUI_TextDisplay(pstTextClass);
    }
    return &pstTextClass->pfControl;
}

/*******************************************************************************
** Name: GUI_TextDelete
** Input:HGC pGc
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.21
** Time: 17:30:03
*******************************************************************************/
_GUI_GUITASK_COMMON_
COMMON API rk_err_t GUI_TextDelete(HGC pGc)
{
    GUI_TEXT_CLASS * pstTextClass= (GUI_TEXT_CLASS *)pGc;

    if ( pstTextClass== NULL )
    {
        printf("delete Text fault\n");
        return RK_ERROR;
    }

    if(pstTextClass->text_bck != NULL)
        rkos_memory_free(pstTextClass->text_bck);

    rkgui_delete_timer(pstTextClass);
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

