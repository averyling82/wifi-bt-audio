/*
********************************************************************************
*                   Copyright (c) 2008, Rock-Chips
*                         All rights reserved.
*
* File Name£º   scu.c
*
* Description:
*
* History:      <author>          <time>        <version>
*               anzhiguo         2009-3-24         1.0
*    desc:    ORG.
********************************************************************************
*/
#define _CRU_IN_

#include <typedef.h>
#include <DriverInclude.h>

#define POSTDIV1(n)         (((APLL_POSTDIV1_MASK << APLL_POSTDIV1_SHIFT)<<16) | ((n)<<APLL_POSTDIV1_SHIFT))
#define FBDIV(n)            (((APLL_FBDIV_MASK << APLL_FBDIV_SHIFT)<<16) | ((n)<<APLL_FBDIV_SHIFT))
#define DSMPD(n)            (((APLL_DSMPD_MASK<<APLL_DSMPD_SHIFT)<<16) | ((n)<<APLL_DSMPD_SHIFT))
#define POSTDIV2(n)         (((APLL_POSTDIV2_MASK<<APLL_POSTDIV2_SHIFT)<<16) | ((n)<<APLL_POSTDIV2_SHIFT))
#define REFDIV(n)           (((APLL_REFDIV_MASK<<APLL_REFDIV_SHIFT)<<16) | ((n)<<APLL_REFDIV_SHIFT))
#define LOCK(n)             (((n)>>APLL_LOCK_SHIFT)&0x1)


/*
--------------------------------------------------------------------------------
  Function name : ScuClockSet()
  Author        :
  Description   : selet open/close module by its id.

  Input         : clk_id module id
                  Enable :open flag: 1--open this id module.0--close this id module

  Return        : null

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void ScuClockGateCtr2(eCLOCK_GATE clk_id, BOOL Enable)
{
    UINT32  group;
    UINT32  num;

    group  = clk_id / 16 ;
    num    = clk_id % 16;
    //When HIGH, disable clock
    if (Enable)
    {
        CRU->CRU_CLKGATE_CON[group] = (uint32)(0x00010000) << (num);
    }
    else
    {
        CRU->CRU_CLKGATE_CON[group] = (uint32)(0x00010001) << (num);
    }
    Delay10cyc2(5);
}

/*
--------------------------------------------------------------------------------
  Function name : ScuModuleReset()
  Author        :
  Description   : reset module by id.

  Input         : clk_id module id
                  Reset:   1:  reset
                           0:  release reset
  Return        : null

  History:     <author>         <time>         <version>
  desc:         ORG
--------------------------------------------------------------------------------
*/
void ScuSoftResetCtr2(eSOFT_RST Module_id, BOOL Reset)
{
    UINT32  group;
    UINT32  num;

    group  = Module_id / 16 ;
    num    = Module_id % 16;

    if (Reset)
    {
        CRU->CRU_SOFTRST[group] = (uint32)(0x00010001) << (num);
    }
    else
    {
        CRU->CRU_SOFTRST[group] = (uint32)(0x00010000) << (num);
    }
    DelayUs2(5);
}
