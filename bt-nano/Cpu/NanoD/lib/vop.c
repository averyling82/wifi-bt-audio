/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoC\lib\vop.c
* Owner: aaron.sun
* Date: 2014.11.21
* Time: 14:43:28
* Desc: vop bsp driver
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    aaron.sun     2014.11.21     14:43:28   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_VOP_C__
#include "RKOS.h"
#include "BSP.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define __INLINE__    inline


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_VOP_READ_  __attribute__((section("cpu_nanod_lib_vop_read")))
#define _CPU_NANOD_LIB_VOP_WRITE_ __attribute__((section("cpu_nanod_lib_vop_write")))
#define _CPU_NANOD_LIB_VOP_INIT_  __attribute__((section("cpu_nanod_lib_vop_init")))
#define _CPU_NANOD_LIB_VOP_SHELL_  __attribute__((section("cpu_nanod_lib_vop_shell")))


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
/*******************************************************************************
** Name: VopSetMcu_FIFO_WATERMARK
** Input:uint32 VopId
** Return: rk_err_t
** Owner:WJR
** Date: 2014.12.1
** Time: 9:43:01
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetMcu_FIFO_WATERMARK(uint32 VopId,int full_watermark,int empty_watermark)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuFIFOWaterMark |= ((full_watermark)<<8)|(empty_watermark);
}

/*******************************************************************************
** Name: VopSetStart
** Input:uint32 VopId ,uint32 mode
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.29
** Time: 14:04:27
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetStart(uint32 VopId ,uint32 mode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuStart= mode;
}

/*******************************************************************************
** Name: VopEnableClk
** Input:uint32 VopId,uint32 IntType
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.27
** Time: 15:23:48
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopEnableClk(uint32 VopId,uint32 IntType)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon |= IntType;
}
/*******************************************************************************
** Name: VopSetMcuIdle
** Input:uint32 VopId,uint32 idleFlag
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.27
** Time: 15:04:14
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetMcuIdle(uint32 VopId,uint32 idleFlag)
{
    VOP * pVop = VopPort(VopId);

    pVop->VopMcuCon |= idleFlag;
}
/*******************************************************************************
** Name: VopReset
** Input:uint32 VopId, uint32 mode
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.25
** Time: 19:09:35
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API uint32 VopIntClear(uint32 VopId)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuIntClear = 0x1;
}
/*******************************************************************************
** Name: VopReset
** Input:uint32 VopId, uint32 mode
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.25
** Time: 19:09:35
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopReset(uint32 VopId)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuSRT = 0x01;
}

/*******************************************************************************
** Name: VopSetUvswap
** Input:uint32 VopId,uint32 mode
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.25
** Time: 17:48:30
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetUvswap(uint32 VopId,uint32 mode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon |= mode;
}

/*******************************************************************************
** Name: VopSetFifoMode
** Input:uint32 VopId, uint32 fifomode
** Return: rk_err_t
** Owner:wjr
** Date: 2014.11.25
** Time: 11:00:28
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetFifoMode(uint32 VopId, uint32 fifomode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon &= ~VOP_CON_FIFO_MODE_MASK;
    pVop->VopMcuIntEn |= fifomode;
}

/*******************************************************************************
** Name: VopDisableInt
** Input:uint32 VopId, uint32 IntType
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 18:32:01
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopDisableInt(uint32 VopId, uint32 IntType)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuIntEn &= ~IntType;
}

/*******************************************************************************
** Name: VopEnableInt
** Input:uint32 VopId, uint32 IntType
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 18:30:01
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopEnableInt(uint32 VopId, uint32 IntType)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuIntEn |= IntType;
}

/*******************************************************************************
** Name: VopGetInt
** Input:void
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 18:13:55
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API uint32 VopGetInt(uint32 VopId)
{
    VOP * pVop = VopPort(VopId);
    uint32 IntStatus;
#if 1
    IntStatus = pVop->VopMcuIntStatus;
    if (IntStatus & VOP_INT_TYPE_FIFO_FULL)
    {
        pVop->VopMcuIntClear |= VOP_INT_TYPE_FIFO_FULL;
    }
    if (IntStatus & VOP_INT_TYPE_FIFO_EMPTY)
    {
        pVop->VopMcuIntClear |= VOP_INT_TYPE_FIFO_EMPTY;
    }
    if (IntStatus & VOP_INT_TYPE_FRAME_DONE)
    {
        pVop->VopMcuIntClear |= VOP_INT_TYPE_FRAME_DONE;
    }
    return (IntStatus &0x1);
#else
    IntStatus = pVop->VopMcuIntStatus  &0x1;
    if (IntStatus)
    {
        pVop->VopMcuIntClear = 0x1;//VOP_INT_TYPE_FIFO_FULL | VOP_INT_TYPE_FIFO_EMPTY | VOP_INT_TYPE_FRAME_DONE;
    }
    return (IntStatus );
#endif
}

/*******************************************************************************
** Name: VopSetYUVType
** Input:uint32 VopId, uint32 YuvType
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 18:12:32
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetYUVType(uint32 VopId, uint32 YuvType)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon &= ~VOP_CON_YUV_MASK;
    pVop->VopMcuCon |= YuvType;

}

/*******************************************************************************
** Name: VopSetDisplayMode
** Input:uint32 VopId, uint32 DisplayMode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 18:11:44
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetDisplayMode(uint32 VopId, uint32 DisplayMode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon = DisplayMode;
}


/*******************************************************************************
** Name: VopSetSplit
** Input:uint32 VopId, uint32 mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 14:00:10
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetSplit(uint32 VopId, uint32 mode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon &= ~VOP_CON_SPLIT_MASK;
    pVop->VopMcuCon |= mode;
}

/*******************************************************************************
** Name: VopSetWidthHeight
** Input:uint32 VopId, uint32 Width, uint32 Height
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 17:55:00
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSetWidthHeight(uint32 VopId, uint32 Width, uint32 Height)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuLcdSize = ((Height-1) << 12) | (Width-1) ;
}

/*******************************************************************************
** Name: VopReadData
** Input:uint32 * data
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 15:03:51
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopReadData(uint32 VopId, uint32 * data)
{
    VOP * pVop = VopPort(VopId);
}

/*******************************************************************************
** Name: VopReadCmd
** Input:uint32 * cmd
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 15:02:41
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopReadCmd(uint32 VopId, uint32 * cmd)
{
    VOP * pVop = VopPort(VopId);
}

/*******************************************************************************
** Name: VopGetDmaReg
** Input:uint32 data
** Return: uint32
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 15:02:06
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
COMMON API uint32 VopGetDmaReg(uint32 VopId)
{
    VOP * pVop = VopPort(VopId);
    return (uint32)&pVop->VopMcuData;
}

/*******************************************************************************
** Name: VopSendData
** Input:uint32 data
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 15:02:06
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSendData(uint32 VopId, uint32 data)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuData = data;
}

/*******************************************************************************
** Name: VopSendCmd
** Input:uint32 cmd
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 15:01:32
*******************************************************************************/
_CPU_NANOD_LIB_VOP_READ_
__INLINE__ COMMON API rk_err_t VopSendCmd(uint32 VopId, uint32 cmd)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCmd = cmd;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(read) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(write) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



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
** Name: VopSetTiming
** Input:uint32 VopID, uint32 csrw, uint32 rwpw, uint32 rwcs
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 17:29:18
*******************************************************************************/
_CPU_NANOD_LIB_VOP_INIT_
INIT API rk_err_t VopSetTiming(uint32 VopId, uint32 csrw, uint32 rwpw, uint32 rwcs)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuTiming = (csrw << 12) | (rwpw << 5) | (rwcs << 0);
}

/*******************************************************************************
** Name: VopSplitMode
** Input:uint32 mode
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.21
** Time: 14:54:56
*******************************************************************************/
_CPU_NANOD_LIB_VOP_INIT_
INIT API rk_err_t VopSetMode(uint32 VopId, uint32 mode)
{
    VOP * pVop = VopPort(VopId);
    pVop->VopMcuCon = mode;
}



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(init) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

