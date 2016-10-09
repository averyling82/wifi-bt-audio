/*
********************************************************************************
*                   Copyright (C),2004-2015, Fuzhou Rockchip Electronics Co.,Ltd.
*                         All rights reserved.
*
* File Name£º  SysIntHandler.c
*
* Description:
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-9-13          1.0
*    desc:
********************************************************************************
*/
#define _IN_SYSTICK_HANDLER_

#include "RKOS.h"
#include "SysTickHandler.h"

uint32 SysTickCounter2;

/*
--------------------------------------------------------------------------------
  Function name : void SysTickTimerRun(void)
  Author        : ZHengYongzhi
  Description   : Default SysTimer, the tick of system

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void SysTickTimerRun2(void)
{
    SysTickCounter2++;
}

 uint32 GetSysTick2(void)
{
    return SysTickCounter2;
}

SYSTICK_LIST SysTickHead2 =
{
    NULL,
    0,
    0,
    0,
    SysTickTimerRun2,
};


UINT32 SystickTimerStartRaw2(SYSTICK_LIST *pListHead, SYSTICK_LIST *pSystick)
{
    //SYSTICK_LIST *pListHead = &SysTickHead;

    //IntDisable(FAULT_ID15_SYSTICK);
    nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_TICKINT;

    while(pListHead->pNext != NULL)
    {
        if ((pListHead->pNext) == pSystick)
            break;
        pListHead = pListHead->pNext;
    }

    if ((pListHead->pNext) == pSystick)
    {
        pSystick->Counter = 0;
    }
    else
    {
        pListHead->pNext = pSystick;
        pSystick->Counter = 0;
        pSystick->pNext = NULL;
    }

    //IntEnable(FAULT_ID15_SYSTICK);
    nvic->SysTick.Ctrl |= NVIC_SYSTICKCTRL_TICKINT;

    return TRUE;
}

UINT32 SystickTimerStart2(SYSTICK_LIST *pSystick)
{
    return (SystickTimerStartRaw2(&SysTickHead2, pSystick));
}

/*
--------------------------------------------------------------------------------
  Function name : UINT32 SystickTimerStop(SYSTICK_LIST *pSystick)
  Author        : ZHengYongzhi
  Description   : Stop a Timer

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
UINT32 SystickTimerStopRaw2(SYSTICK_LIST *pListHead, SYSTICK_LIST *pSystick)
{
    uint32 tmp;
    //SYSTICK_LIST *pListHead = &SysTickHead;

    //tmp = IsrDisable(FAULT_ID15_SYSTICK);
    tmp = (nvic->SysTick.Ctrl & NVIC_SYSTICKCTRL_TICKINT);
    nvic->SysTick.Ctrl &= ~NVIC_SYSTICKCTRL_TICKINT;

    while(pListHead->pNext != pSystick)
    {
        pListHead = pListHead->pNext;
    }

    pListHead->pNext = pSystick->pNext;

    //IsrEnable(FAULT_ID15_SYSTICK, tmp);
    if (tmp)
    {
        nvic->SysTick.Ctrl |= NVIC_SYSTICKCTRL_TICKINT;
    }

    return TRUE;
}


UINT32 SystickTimerStop2(SYSTICK_LIST *pSystick)
{
    return (SystickTimerStopRaw2(&SysTickHead2, pSystick));
}


/*
--------------------------------------------------------------------------------
  Function name : void SysTickHandler(void)
  Author        : ZHengYongzhi
  Description   : SysTick Timer Isr

  Input         :
  Return        :

  History:     <author>         <time>         <version>
             ZhengYongzhi     2008/07/21         Ver1.0
  desc:         ORG
--------------------------------------------------------------------------------
*/
void SysTickHandlerRaw2(SYSTICK_LIST *pListHead, pSysTickStopCallBack StopCallBack)
{
    //SYSTICK_LIST *pListHead = &SysTickHead;

    (pListHead->pHandler)();

    pListHead = pListHead->pNext;
    while(pListHead != NULL)
    {
        if ((pListHead->pHandler) && (pListHead->Reload))
        {
            if (++pListHead->Counter >= pListHead->Reload)
            {
                (pListHead->pHandler)();
                pListHead->Counter = 0;

                if (pListHead->Times)
                {
                    if (--pListHead->Times == 0)
                    {
                        //SystickTimerStop(pListHead);
                        StopCallBack(pListHead);
                    }
                }
            }
        }
        pListHead = pListHead->pNext;
    }
}

__irq void SysTickHandler2(void)
{
    SysTickHandlerRaw2(&SysTickHead2, SystickTimerStop2);
}

/*
********************************************************************************
*
*                         End of SysIntHandler.c
*
********************************************************************************
*/

