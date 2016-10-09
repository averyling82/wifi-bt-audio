/*
********************************************************************************************
*
*        Copyright (c): Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Driver\Display\DisplayDevice.c
* Owner: aaron.sun
* Date: 2015.5.29
* Time: 10:20:23
* Version: 1.0
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2015.5.29     10:20:23   1.0
********************************************************************************************
*/


#include "BspConfig.h"
#ifdef __DRIVER_DISPLAY_DISPLAYDEVICE_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "BSP.h"
#include "DeviceInclude.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
typedef enum DISPLAY_DEV
{
    DISPLAY_DEV0 = 0,
    DISPLAY_DEV1,
    DISPLAY_DEV_MAX
}eDISPLAY_DEV;

typedef struct RectInfoStruct
{
    int x0;
    int y0;
    int x1;
    int y1;
    int x;
    int y;
    int xdir;
    int ydir;

    int xMin;
    int yMin;
    int xMax;
    int yMax;
    int flag;
    int finish;
} RECT_INFO_STRUCT;

typedef struct _FRAME_DEVICE_CLASS
{
    void * buffer;
} FRAME_DEVICE_CLASS;

typedef  struct _DISPLAY_DEVICE_CLASS
{
    DEVICE_CLASS stDisplayDevice;
    uint32 DevID;

    HDC hLcd;
    LCD_INFO_CLASS * pfLcd;
#ifdef _FRAME_BUFFER_
    uint16 xSize;
    uint16 ySize;
    FRAME_DEVICE_CLASS frame;
#endif
    RECT_INFO_STRUCT stRectInfo;
}DISPLAY_DEVICE_CLASS;

static DISPLAY_DEVICE_CLASS * gpstDisplayDevISR[DISPLAY_DEV_MAX];

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
rk_err_t DisplayDevCheckHandler(HDC dev);
rk_err_t DisplayDevShellTest(HDC dev, uint8 * pstr);
rk_err_t DisplayDevShellDel(HDC dev, uint8 * pstr);
rk_err_t DisplayDevShellCreate(HDC dev, uint8 * pstr);
rk_err_t DisplayDevShellPcb(HDC dev, uint8 * pstr);
rk_err_t DisplayDevDeInit(DISPLAY_DEVICE_CLASS * pstDisplayDev);
rk_err_t DisplayDevInit(uint32 DevID, DISPLAY_DEVICE_CLASS * pstDisplayDev);
rk_err_t DisplayDevResume(HDC dev);
rk_err_t DisplayDevSuspend(HDC dev, uint32 Level);

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DisplayDev_ScreenL1Update
** Input:HDC dev
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.14
** Time: 14:15:57
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_Blur(HDC dev)
{
    uint16 * dest;
    uint16 color,R,G,B;
    uint32 i;

    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;
    if (pstDisplayDev == NULL)
    {
        return RK_ERROR;
    }

    dest= (uint16 *)pstDisplayDev->frame.buffer;

    for(i= 0; i< pstDisplayDev->xSize* pstDisplayDev->ySize; i++)
    {
        color= dest[i];
        B= ((color>>8)&0x1F)*4/10;
        R= ((color>>0)&0xF8)*4/10;
        G= (((color&0x07)<<3)|(color>>13))*4/10;
        dest[i]= (B&0x1F)<<8 | (G&0x38)>>3 | (G&0x07)<<13 | (R&0xF8);
    }
    pstDisplayDev->stRectInfo.xMin= 0;
    pstDisplayDev->stRectInfo.xMax= pstDisplayDev->xSize;
    pstDisplayDev->stRectInfo.yMin= 0;
    pstDisplayDev->stRectInfo.yMax= pstDisplayDev->ySize;
    pstDisplayDev->stRectInfo.flag= 1;

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: LCD_ScreenUpdate
** Input:HDC dev
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.14
** Time: 14:15:57
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_ScreenUpdate(void)
{
    uint16 *prospect, *background, *dest;
    uint32 i;
    int y0;
    rk_err_t ret= RK_SUCCESS;
    uint32 addr;

#ifdef _FRAME_BUFFER_
#ifdef DISPLAY_DOUBLE_LAYER
    if(gpstDisplayDevISR[0]->stRectInfo.flag==1)
    {
        y0= DISPLAY_DEV1_HEIGHT;
        switch( gpstDisplayDevISR[0]->pfLcd->pallet )
        {
        #ifdef PALLET_RGB565
            case PALLET_RGB565:
                addr= (uint32)gpstDisplayDevISR[0]->frame.buffer+ y0*  gpstDisplayDevISR[0]->pfLcd->width* 2;
                break;
        #endif
        }
        LcdSetWindow(gpstDisplayDevISR[0]->hLcd ,0, y0 ,gpstDisplayDevISR[1]->pfLcd->width, gpstDisplayDevISR[0]->stRectInfo.yMax);
        ret= Lcd_Write(gpstDisplayDevISR[0]->hLcd, (void *)addr, gpstDisplayDevISR[1]->pfLcd->width * (gpstDisplayDevISR[0]->stRectInfo.yMax - y0) * 2);

        gpstDisplayDevISR[0]->stRectInfo.xMax= -1;
        gpstDisplayDevISR[0]->stRectInfo.xMin= gpstDisplayDevISR[0]->xSize;
        gpstDisplayDevISR[0]->stRectInfo.yMax= -1;
        gpstDisplayDevISR[0]->stRectInfo.yMin= gpstDisplayDevISR[0]->ySize;
        gpstDisplayDevISR[0]->stRectInfo.flag= 0;
    }


    if(gpstDisplayDevISR[1]->stRectInfo.flag==1)
    {
        addr= (uint32)gpstDisplayDevISR[1]->frame.buffer+ gpstDisplayDevISR[1]->stRectInfo.yMin *  gpstDisplayDevISR[1]->pfLcd->width* 2;

        LcdSetWindow(gpstDisplayDevISR[1]->hLcd ,0, gpstDisplayDevISR[1]->stRectInfo.yMin ,gpstDisplayDevISR[1]->pfLcd->width, gpstDisplayDevISR[1]->stRectInfo.yMax);
        ret= Lcd_Write(gpstDisplayDevISR[1]->hLcd,  (void *)addr,
             gpstDisplayDevISR[1]->pfLcd->width * (gpstDisplayDevISR[1]->stRectInfo.yMax - gpstDisplayDevISR[1]->stRectInfo.yMin) * 2);

	    gpstDisplayDevISR[1]->stRectInfo.xMax= -1;
        gpstDisplayDevISR[1]->stRectInfo.xMin= gpstDisplayDevISR[1]->xSize;
        gpstDisplayDevISR[1]->stRectInfo.yMax= -1;
        gpstDisplayDevISR[1]->stRectInfo.yMin= gpstDisplayDevISR[1]->ySize;
        gpstDisplayDevISR[1]->stRectInfo.flag= 0;
    }


#else
    addr= (uint32)gpstDisplayDevISR[0]->frame.buffer+ gpstDisplayDevISR[0]->stRectInfo.yMin *  gpstDisplayDevISR[0]->pfLcd->width* 2;
    LcdSetWindow(gpstDisplayDevISR[0]->hLcd ,0, gpstDisplayDevISR[0]->stRectInfo.yMin ,gpstDisplayDevISR[0]->pfLcd->width, gpstDisplayDevISR[0]->stRectInfo.yMax);
    ret= Lcd_Write(gpstDisplayDevISR[0]->hLcd, (void *)addr,
       gpstDisplayDevISR[0]->pfLcd->width * (gpstDisplayDevISR[0]->stRectInfo.yMax- gpstDisplayDevISR[0]->stRectInfo.yMin) * 2);
#endif
#endif
    return ret;
}

/*******************************************************************************
** Name: DisplayDev_for_Select
** Input:HDC dev ,int y, int ySize
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.14
** Time: 14:15:57
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_for_Select(HDC dev ,int y0, int y1, int ySize)
{
    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

    pstDisplayDev->stRectInfo.xMin= 0;
    pstDisplayDev->stRectInfo.xMax= pstDisplayDev->pfLcd->width;
    pstDisplayDev->stRectInfo.yMin= 0;
    pstDisplayDev->stRectInfo.yMax= pstDisplayDev->pfLcd->height;
    pstDisplayDev->stRectInfo.flag= 1;

    memcpy((char *)((uint32)pstDisplayDev->frame.buffer+ pstDisplayDev->pfLcd->width* y0 *2), (char *)((uint32)pstDisplayDev->frame.buffer+ 128* y1 *2), 128* ySize *2);

    return RK_SUCCESS;
}


/*******************************************************************************
** Name: DisplayDev_RotateCanvas
** Input:HDC dev ,RK_LCD_ROTATE rotate
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.14
** Time: 14:15:57
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_RotateCanvas(HDC dev ,RK_LCD_ROTATE rotate)
{
    uint16 x0,x1,y0,y1;
    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

    x0= pstDisplayDev->stRectInfo.x0;
    y0= pstDisplayDev->stRectInfo.y0;
    x1= pstDisplayDev->stRectInfo.x1;
    y1= pstDisplayDev->stRectInfo.y1;

//    if ( LcdSetRotateWindow(pstDisplayDev->hLcd, x0, y0, x1, y1, rotate )== RK_SUCCESS )
//    {
//        return RK_SUCCESS;
//    }

    switch (rotate)
    {
        case ROTATE_NOMAL:
            pstDisplayDev->stRectInfo.x= x0;
            pstDisplayDev->stRectInfo.y= y0;
            pstDisplayDev->stRectInfo.xdir = 1;
            pstDisplayDev->stRectInfo.ydir = 1;
            break;

        case ROTATE_FLIP_LR:
            pstDisplayDev->stRectInfo.x0= pstDisplayDev->xSize - x0 - 1;
            pstDisplayDev->stRectInfo.x1= pstDisplayDev->xSize - x1 - 1;
            pstDisplayDev->stRectInfo.x= pstDisplayDev->stRectInfo.x0;
            pstDisplayDev->stRectInfo.y= y0;
            pstDisplayDev->stRectInfo.xdir = -1;
            pstDisplayDev->stRectInfo.ydir = 1;
            break;

        case ROTATE_FLIP_UD:
            pstDisplayDev->stRectInfo.y0= pstDisplayDev->ySize - y0 - 1;
            pstDisplayDev->stRectInfo.y1= pstDisplayDev->ySize - y1 - 1;
            pstDisplayDev->stRectInfo.x= x0;
            pstDisplayDev->stRectInfo.y= pstDisplayDev->stRectInfo.y0;
            pstDisplayDev->stRectInfo.xdir = 1;
            pstDisplayDev->stRectInfo.ydir = -1;
            break;

        case ROTATE_ROTATE:
            pstDisplayDev->stRectInfo.x0= pstDisplayDev->xSize - x0 - 1;
            pstDisplayDev->stRectInfo.x1= pstDisplayDev->xSize - x1 - 1;
            pstDisplayDev->stRectInfo.y0= pstDisplayDev->ySize - y0 - 1;
            pstDisplayDev->stRectInfo.y1= pstDisplayDev->ySize - y1 - 1;
            pstDisplayDev->stRectInfo.x= pstDisplayDev->stRectInfo.x0;
            pstDisplayDev->stRectInfo.y= pstDisplayDev->stRectInfo.y0;
            pstDisplayDev->stRectInfo.xdir = -1;
            pstDisplayDev->stRectInfo.ydir = -1;
            break;

        default:
            return RK_ERROR;
    }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DisplayDev_SetWindow
** Input:HDC dev ,uint16 x0, uint16 y0, uint16 x1, int16 y1
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.12
** Time: 14:15:57
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_SetWindow(HDC dev ,int x, int y, int xSize, int ySize)
{
    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

    if ( x >= pstDisplayDev->xSize || y >= pstDisplayDev->ySize )
    {
        pstDisplayDev->stRectInfo.flag= 0;
        return RK_ERROR;
    }

    if( xSize + x > pstDisplayDev->xSize )xSize= pstDisplayDev->xSize- x;
    if( ySize + y > pstDisplayDev->ySize )ySize= pstDisplayDev->ySize- y;

#ifdef _FRAME_BUFFER_
    pstDisplayDev->stRectInfo.x0 = x;
    pstDisplayDev->stRectInfo.y0 = y;
    pstDisplayDev->stRectInfo.x1 = x+ xSize;
    pstDisplayDev->stRectInfo.y1 = y+ ySize;
    pstDisplayDev->stRectInfo.x = x;
    pstDisplayDev->stRectInfo.y = y;
    pstDisplayDev->stRectInfo.xdir = 1;
    pstDisplayDev->stRectInfo.ydir = 1;
    pstDisplayDev->stRectInfo.finish= 0;

    if( pstDisplayDev->stRectInfo.x0 < pstDisplayDev->stRectInfo.xMin )
        pstDisplayDev->stRectInfo.xMin= pstDisplayDev->stRectInfo.x0;
    if( pstDisplayDev->stRectInfo.x1 > pstDisplayDev->stRectInfo.xMax )
        pstDisplayDev->stRectInfo.xMax= pstDisplayDev->stRectInfo.x1;

    if( pstDisplayDev->stRectInfo.y0 < pstDisplayDev->stRectInfo.yMin )
        pstDisplayDev->stRectInfo.yMin= pstDisplayDev->stRectInfo.y0;
    if( pstDisplayDev->stRectInfo.y1 > pstDisplayDev->stRectInfo.yMax )
        pstDisplayDev->stRectInfo.yMax= pstDisplayDev->stRectInfo.y1;
#else
    LcdSetWindow(pstDisplayDev->hLcd ,x ,y ,x+ xSize, y+ ySize);
#endif
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: DisplayDev_SetOffset
** Input:HDC dev,uint16 data
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.12
** Time: 17:02:10
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
inline COMMON API rk_err_t DisplayDev_SetOffset(HDC dev)
{
    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

    pstDisplayDev->stRectInfo.x+= pstDisplayDev->stRectInfo.xdir;
    if ( pstDisplayDev->stRectInfo.x== pstDisplayDev->stRectInfo.x1 )
    {
        pstDisplayDev->stRectInfo.x= pstDisplayDev->stRectInfo.x0;

        pstDisplayDev->stRectInfo.y+= pstDisplayDev->stRectInfo.ydir;
        if ( pstDisplayDev->stRectInfo.y== pstDisplayDev->stRectInfo.y1)
        {
            pstDisplayDev->stRectInfo.y= pstDisplayDev->stRectInfo.y0;
            pstDisplayDev->stRectInfo.finish= 1;
        }
    }
}

/*******************************************************************************
** Name: DisplayDev_SendData
** Input:HDC dev, uint8 R, uint8 G, uint8 B, uint8 transparency
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.12
** Time: 17:02:10
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_SendData(HDC dev, uint8 R, uint8 G, uint8 B, uint8 transparency)
{
    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

//    rkos_semaphore_take(pstDisplayDev->osDisplayWriteSem, MAX_DELAY);
#ifdef _FRAME_BUFFER_
#ifdef PALLET_RGB565
    uint16 color= (uint16)(R>>3)<<11 | (uint16)(G>>2)<< 5 | (uint16)(B>>3);
    uint8 * pstbuffer= (uint8 *)((uint32)pstDisplayDev->frame.buffer + (pstDisplayDev->stRectInfo.x + pstDisplayDev->stRectInfo.y * pstDisplayDev->xSize ) * 2 );
    pstbuffer[1]= color& 0xFF;
    pstbuffer[0]= color>> 8;
#endif
    DisplayDev_SetOffset(pstDisplayDev);
#else
    uint16 color= (uint16)(R>>3)<<11 | (uint16)(G>>2)<< 5 | (uint16)(B>>3);
    Lcd_Write(pstDisplayDev->hLcd, &color, 2);
#endif
    pstDisplayDev->stRectInfo.flag= 1;
//    rkos_semaphore_give(pstDisplayDev->osDisplayWriteSem);
    return RK_SUCCESS;
}


/*******************************************************************************
** Name: DisplayDev_Write
** Input:HDC dev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_Write(HDC dev, void *color, uint16 size)
{
    uint16 i;
    int xSize, ySize;

    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

#ifdef _FRAME_BUFFER_
#ifdef PALLET_RGB565
    for (i=0; i< size; i++)
    {
        if( pstDisplayDev->stRectInfo.finish )
        {
            break;
        }
        uint8 * pstbuffer= (uint8 *)((uint32)pstDisplayDev->frame.buffer + (pstDisplayDev->stRectInfo.x + pstDisplayDev->stRectInfo.y * pstDisplayDev->xSize ) * 2);

        *(uint16 *)pstbuffer = *(uint16 *)color;

        ((uint16 *)color)++;
        DisplayDev_SetOffset(pstDisplayDev);
    }
#endif
#else
    Lcd_Write(pstDisplayDev->hLcd, color, size);
#endif
    pstDisplayDev->stRectInfo.flag= 1;

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DisplayDev_ClrRect
** Input:HDC dev , uint8 R, uint8 G, uint8 B, uint8 transparency
** Return: void
** Owner:Benjo.lei
** Date: 2015.10.12
** Time: 14:26:11
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON API rk_err_t DisplayDev_ClrRect(HDC dev, uint8 R, uint8 G, uint8 B, uint8 transparency)
{
    uint32 i,size;

    DISPLAY_DEVICE_CLASS * pstDisplayDev =  (DISPLAY_DEVICE_CLASS *)dev;

#ifdef _FRAME_BUFFER_
    size= (pstDisplayDev->stRectInfo.x1) * (pstDisplayDev->stRectInfo.y1);
#ifdef PALLET_RGB565
    uint16 color= (uint16)(R>>3)<<11 | (uint16)(G>>2)<< 5 | (uint16)(B>>3);

    for (i=0; i< size; i++)
    {
        uint8 * pstbuffer= (uint8 *)((uint32)pstDisplayDev->frame.buffer + (pstDisplayDev->stRectInfo.x + pstDisplayDev->stRectInfo.y * pstDisplayDev->xSize ) * 2 );
        if( pstDisplayDev->stRectInfo.finish )
        {
            break;
        }
        pstbuffer[1]= color& 0xFF;
        pstbuffer[0]= color>> 8;
        DisplayDev_SetOffset(pstDisplayDev);
    }
#endif
#else
    uint16 color= (uint16)(R>>3)<<11 | (uint16)(G>>2)<< 5 | (uint16)(B>>3);
    for (i=0; i< size; i++)
    {
        Lcd_Write(pstDisplayDev->hLcd, &color, 2);
    }
#endif
    pstDisplayDev->stRectInfo.flag= 1;
    return RK_SUCCESS;
}


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(common) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DisplayDevCheckHandler
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2016.8.12
** Time: 20:05:16
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_COMMON_
COMMON FUN rk_err_t DisplayDevCheckHandler(HDC dev)
{
    uint32 i;
    for(i = 0; i < DISPLAY_DEV_MAX; i++)
    {
        if(gpstDisplayDevISR[i] == dev)
        {
            return RK_SUCCESS;
        }
    }
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
** Name: DisplayDevResume
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT FUN rk_err_t DisplayDevResume(HDC dev)
{
     DISPLAY_DEVICE_CLASS * pstDisplayDev = (DISPLAY_DEVICE_CLASS *)dev;
     if(DisplayDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }
     RKDev_Resume(pstDisplayDev->hLcd);
     pstDisplayDev->stDisplayDevice.State = DEV_STATE_WORKING;
     return RK_SUCCESS;
}
/*******************************************************************************
** Name: DisplayDevSuspend
** Input:HDC dev
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT FUN rk_err_t DisplayDevSuspend(HDC dev, uint32 Level)
{
     DISPLAY_DEVICE_CLASS * pstDisplayDev = (DISPLAY_DEVICE_CLASS *)dev;
     if(DisplayDevCheckHandler(dev) == RK_ERROR)
     {
        return RK_ERROR;
     }

     RKDev_Suspend(pstDisplayDev->hLcd);

     if(Level == DEV_STATE_IDLE1)
     {
         pstDisplayDev->stDisplayDevice.State = DEV_STATE_IDLE1;
     }
     else if(Level == DEV_SATE_IDLE2)
     {
         pstDisplayDev->stDisplayDevice.State = DEV_SATE_IDLE2;
     }

    return RK_SUCCESS;
}

/*******************************************************************************
** Name: DisplayDev_Delete
** Input:uint32 DevID
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT API rk_err_t DisplayDev_Delete(uint32 DevID, void * arg)
{
    DISPLAY_DEV_ARG * pstDisplayDevArg = (DISPLAY_DEV_ARG *)arg;

    //Check DisplayDev is not exist...
    if (gpstDisplayDevISR[DevID] == NULL)
    {
        return RK_ERROR;
    }

    pstDisplayDevArg->hLcd = gpstDisplayDevISR[DevID]->hLcd;

    //DisplayDev deinit...
    DisplayDevDeInit(gpstDisplayDevISR[DevID]);

    //Free DisplayDev memory...

#ifdef _FRAME_BUFFER_
    if (gpstDisplayDevISR[DevID]->frame.buffer!= NULL)
        rkos_memory_free(gpstDisplayDevISR[DevID]->frame.buffer);
#endif

    rkos_memory_free(gpstDisplayDevISR[DevID]);
    gpstDisplayDevISR[DevID] = NULL;

    //Delete DisplayDev Read and Write Module...

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_RemoveSegment(SEGMENT_ID_DISPLAY_DEV);
#endif

    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DisplayDev_Create
** Input:uint32 DevID, void * arg
** Return: HDC
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT API HDC DisplayDev_Create(uint32 DevID, void * arg)
{
    DEVICE_CLASS* DevClass;
    DISPLAY_DEVICE_CLASS * pstDisplayDev;
    DISPLAY_DEV_ARG * pstDisplayDevArg;

    if (arg == NULL)
    {
        return (HDC)RK_PARA_ERR;
    }
    pstDisplayDevArg = (DISPLAY_DEV_ARG *)arg;

    pstDisplayDev =  rkos_memory_malloc(sizeof(DISPLAY_DEVICE_CLASS));
    if (pstDisplayDev == NULL)
    {
        return pstDisplayDev;
    }

    //init handler...
    DevClass = (DEVICE_CLASS *)pstDisplayDev;
    DevClass->suspend = DisplayDevSuspend;
    DevClass->resume  = DisplayDevResume;
    DevClass->SuspendMode = ENABLE_MODE;
    DevClass->Idle1EventTime = 10 * PM_TIME;

    pstDisplayDev->DevID = DevID;
    pstDisplayDev->hLcd = pstDisplayDevArg->hLcd;

#ifdef __OS_FWANALYSIS_FWANALYSIS_C__
    FW_LoadSegment(SEGMENT_ID_DISPLAY_DEV, SEGMENT_OVERLAY_CODE);
#endif
    if (DisplayDevInit(DevID,pstDisplayDev) != RK_SUCCESS)
    {
        rkos_memory_free(pstDisplayDev);
        return (HDC) RK_ERROR;
    }

    pstDisplayDev->stRectInfo.xMax= -1;
    pstDisplayDev->stRectInfo.xMin= pstDisplayDev->xSize;
    pstDisplayDev->stRectInfo.yMax= -1;
    pstDisplayDev->stRectInfo.yMin= pstDisplayDev->ySize;
    pstDisplayDev->stRectInfo.flag= 0;

    gpstDisplayDevISR[DevID] = pstDisplayDev;

    return (HDC)DevClass;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DisplayDevDeInit
** Input:DISPLAY_DEVICE_CLASS * pstDisplayDev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT FUN rk_err_t DisplayDevDeInit(DISPLAY_DEVICE_CLASS * pstDisplayDev)
{
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DisplayDevInit
** Input:DISPLAY_DEVICE_CLASS * pstDisplayDev
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_INIT_
INIT FUN rk_err_t DisplayDevInit(uint32 DevID, DISPLAY_DEVICE_CLASS * pstDisplayDev)
{
    uint32 pSize;
    LCD_INFO_CLASS * pfLcd;

    pfLcd= LcdGetParameter(pstDisplayDev->hLcd);
    if (pfLcd== NULL)
    {
        printf("Get LCD information fault\n");
        return RK_ERROR;
    }

    pstDisplayDev->pfLcd = pfLcd;

#ifdef _FRAME_BUFFER_
    if ( pstDisplayDev->pfLcd->BW )
    {
        printf("nonsupport pallet-1bit now\n");
        return RK_ERROR;
    }
    else
    {
        if(DevID==0)
        {
            pSize= pfLcd->width * pfLcd->height;
            pstDisplayDev->xSize= pfLcd->width;
            pstDisplayDev->ySize= pfLcd->height;
        }
        else
        {
            pSize= pfLcd->width * DISPLAY_DEV1_HEIGHT;
            pstDisplayDev->xSize= pfLcd->width;
            pstDisplayDev->ySize= DISPLAY_DEV1_HEIGHT;
        }
        switch ( pstDisplayDev->pfLcd->pallet )
        {
#ifdef PALLET_RGB565
            case PALLET_RGB565:
                pSize= pSize * 2;
                break;
#endif

#ifdef PALLET_RGB888
            case PALLET_RGB888:
                pSize= pSize * 3;
                break;
#endif

#ifdef PALLET_RGB8888
            case PALLET_RGB8888:
                pSize= pSize * 4;
                break;
#endif
            default :
                printf("nonsupport RGB mode\n");
                return RK_ERROR;
        }
    }
    pstDisplayDev->frame.buffer = rkos_memory_malloc(pSize);
    if (pstDisplayDev->frame.buffer == NULL)
    {
        printf("malloc framebuffer memory error:\n");
        printf("width:%d\n",pstDisplayDev->xSize );
        printf("height:%d\n",pstDisplayDev->ySize );
        return RK_ERROR;
    }
    //printf ("\n malloc %d width=%d height=%d addr=%x\n",pstDisplayDev->DevID, pstDisplayDev->xSize, pstDisplayDev->ySize,(uint32)pstDisplayDev->frame.buffer);
#endif
    return RK_SUCCESS;
}


#ifdef _DISPLAY_DEV_SHELL_
_DRIVER_DISPLAY_DISPLAYDEVICE_DATA_
static SHELL_CMD ShellDisplayName[] =
{
    "pcb",NULL,"NULL","NULL",
    "mc",NULL,"NULL","NULL",
    "del",NULL,"NULL","NULL",
    "test",NULL,"NULL","NULL",
    "\b",NULL,"NULL","NULL",
};

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DisplayDev_Shell
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
SHELL API rk_err_t DisplayDev_Shell(HDC dev, uint8 * pstr)
{
    uint32 i = 0;
    uint8  *pItem;
    uint16 StrCnt = 0;
    rk_err_t   ret;
    uint8 Space;
    StrCnt = ShellItemExtract(pstr, &pItem, &Space);
    if (StrCnt == 0)
    {
        return RK_ERROR;
    }

    ret = ShellCheckCmd(ShellDisplayName, pItem, StrCnt);
    if (ret < 0)
    {
        return RK_ERROR;
    }

    i = (uint32)ret;

    pItem += StrCnt;
    pItem++;

    switch (i)
    {
        case 0x00:
            ret = DisplayDevShellPcb(dev,pItem);
            break;

        case 0x01:
            ret = DisplayDevShellMc(dev,pItem);
            break;

        case 0x02:
            ret = DisplayDevShellDel(dev,pItem);
            break;

        case 0x03:
            ret = DisplayDevShellTest(dev,pItem);
            break;

        default:
            ret = RK_ERROR;
            break;
    }
    return ret;

}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: DisplayDevShellTest
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
SHELL FUN rk_err_t DisplayDevShellTest(HDC dev, uint8 * pstr)
{
//    HDC hDisplay;
//    uint32 i;

//    DISPLAY_DEVICE_CLASS * p;

//    hDisplay = RKDev_Open(DEV_CLASS_DISPLAY, 0, NULL);
//    if (hDisplay == NULL)
//    {
//        printf("Open DEV_CLASS_DISPLAY failure\n\n");
//        return RK_ERROR;
//    }

//    p= (DISPLAY_DEVICE_CLASS *)hDisplay;

//    memset(p->frame.buffer,0x000F,128*160*2);
//    DisplayDev_ScreenUpdate(hDisplay);
//    DelayMs(1000);

//    DisplayDev_SetWindow(hDisplay,0,0,128,160);
//    for (i=0; i<(128*160); i++)
//    {
//        DisplayDev_SendData(hDisplay,0,0,0,0);
//    }
//    DisplayDev_ScreenUpdate(hDisplay);
//    DelayMs(1000);

//    {
//        uint16 buf[5][50];
//        memset(buf,0xFFFF,sizeof(buf));
//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_Write(hDisplay,&buf,sizeof(buf)/2);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        for (i=10; i<5*50; i++)
//        {
//            DisplayDev_SendData(hDisplay,0xF0,0,0,0);
//        }
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_ClrRect(hDisplay,0,0,0xFF,0);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        /*********************************************************/
//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_LR);
//        DisplayDev_Write(hDisplay,&buf,sizeof(buf)/2);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_LR);
//        for (i=10; i<5*50; i++)
//        {
//            DisplayDev_SendData(hDisplay,0xFF,0,0,0);
//        }
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_LR);
//        DisplayDev_ClrRect(hDisplay,0,0,0xFF,0);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        /*********************************************************/
//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_UD);
//        DisplayDev_Write(hDisplay,&buf,sizeof(buf)/2);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_UD);
//        for (i=10; i<5*50; i++)
//        {
//            DisplayDev_SendData(hDisplay,0xFF,0,0,0);
//        }
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_FLIP_UD);
//        DisplayDev_ClrRect(hDisplay,0,0,0xFF,0);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        /*********************************************************/
//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_ROTATE);
//        DisplayDev_Write(hDisplay,&buf,sizeof(buf)/2);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_ROTATE);
//        for (i=10; i<5*50; i++)
//        {
//            DisplayDev_SendData(hDisplay,0xFF,0,0,0);
//        }
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);

//        DisplayDev_SetWindow(hDisplay,10,10,15,50);
//        DisplayDev_RotateCanvas(hDisplay,ROTATE_ROTATE);
//        DisplayDev_ClrRect(hDisplay,0,0,0xFF,0);
//        DisplayDev_ScreenUpdate(hDisplay);
//        DelayMs(1000);
//    }

//    RKDev_Close(hDisplay);
//    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DisplayDevShellDel
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
SHELL FUN rk_err_t DisplayDevShellDel(HDC dev, uint8 * pstr)
{
    if (RKDev_Delete(DEV_CLASS_DISPLAY, 0, NULL) != RK_SUCCESS)
    {
        rk_print_string("DISPLAYDev delete failure");
        return RK_ERROR;
    }
    return RK_SUCCESS;

}
/*******************************************************************************
** Name: DisplayDevShellMc
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:Benjo.lei
** Date: 2015.10.10
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
SHELL FUN rk_err_t DisplayDevShellMc(HDC dev, uint8 * pstr)
{
    DISPLAY_DEV_ARG stDisplayDevArg;
    rk_err_t ret;

    stDisplayDevArg.hLcd = RKDev_Open(DEV_CLASS_LCD, 0, NOT_CARE);
    if (stDisplayDevArg.hLcd == NULL)
    {
        printf( "Lcd OPEN fault\n");
        return RK_ERROR;
    }

    ret = RKDev_Create(DEV_CLASS_DISPLAY, 0, &stDisplayDevArg);
    if (ret != RK_SUCCESS)
    {
        printf("DisplayDev create failure\n");
        return RK_ERROR;
    }
    printf("\n DisplayDev create success\n");
    return RK_SUCCESS;
}
/*******************************************************************************
** Name: DisplayDevShellPcb
** Input:HDC dev, uint8 * pstr
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2015.5.29
** Time: 10:23:49
*******************************************************************************/
_DRIVER_DISPLAY_DISPLAYDEVICE_SHELL_
SHELL FUN rk_err_t DisplayDevShellPcb(HDC dev, uint8 * pstr)
{
    return RK_SUCCESS;

}


#endif

#endif



