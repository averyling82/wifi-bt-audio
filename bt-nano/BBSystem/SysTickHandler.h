/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º   SystickHandler.h
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _SYSTICKHANDLER_H_
#define _SYSTICKHANDLER_H_

#undef  EXT
#ifdef _IN_SYSTICK_HANDLER_
#define EXT
#else
#define EXT extern
#endif

/*
*-------------------------------------------------------------------------------
*
*                            Struct Define
*
*-------------------------------------------------------------------------------
*/
typedef struct _SYSTICK_LIST {

    struct _SYSTICK_LIST  *pNext;

    uint32 Counter;
    uint32 Reload;
    uint32 Times;
    void   (*pHandler)(void);

}SYSTICK_LIST;

typedef UINT32 (*pSysTickStopCallBack)(SYSTICK_LIST *pSystick);


/*
*-------------------------------------------------------------------------------
*
*                            Function Declaration
*
*-------------------------------------------------------------------------------
*/

extern __irq void SysTickHandler(void);
extern __irq void SysTickHandler2(void);

/*
********************************************************************************
*
*                         End of SystickHandler.h
*
********************************************************************************
*/
#endif




