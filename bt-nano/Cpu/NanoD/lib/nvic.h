/*
********************************************************************************
*                   Copyright (c) 2008,ZhengYongzhi
*                         All rights reserved.
*
* File Name£º   nvic.h
*
* Description:
*
*
* History:      <author>          <time>        <version>
*             ZhengYongzhi      2008-8-13          1.0
*    desc:    ORG.
********************************************************************************
*/

#ifndef _NVIC_H_
#define _NVIC_H_

/*
*-------------------------------------------------------------------------------
*
*                           Function Declaration
*
*-------------------------------------------------------------------------------
*/

extern void  NvicSystemReset(void);
extern void  NvicCpuReset(void);
extern void  NvicClearActive(void);
extern void  NvicVectorTableSet(uint32 vectortaboffset);


/*
********************************************************************************
*
*                         End of nvic.h
*
********************************************************************************
*/
#endif
