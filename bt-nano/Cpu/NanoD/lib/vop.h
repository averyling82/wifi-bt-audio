/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Cpu\NanoC\lib\vop.h
* Owner: aaron.sun
* Date: 2014.11.21
* Time: 14:43:48
* Desc: vop bsp driver
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2014.11.21     14:43:48   1.0
********************************************************************************************
*/

#ifndef __CPU_NANOC_LIB_VOP_H__
#define __CPU_NANOC_LIB_VOP_H__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define VOP_CON_IDLE_INPUT          (uint32)(0x0 << 0)
#define VOP_CON_IDLE_OUTPUT         (uint32)(0x1 << 0)

#define VOP_CON_SPLIT_ONE           (uint32)(0x0 << 1)
#define VOP_CON_SPLIT_TWO           (uint32)(0x1 << 1)
#define VOP_CON_SPLIT_THREE         (uint32)(0x2 << 1)
#define VOP_CON_SPLIT_FOUR          (uint32)(0x3 << 1)
#define VOP_CON_SPLIT_MASK          (uint32)(0x3 << 1)

#define VOP_CON_DATA_WIDTH_8        (uint32)(0x0 << 3)
#define VOP_CON_DATA_WIDTH_16       (uint32)(0x1 << 3)

#define VOP_CON_HWORD_NO_SWAP       (uint32)(0x0 << 4)
#define VOP_CON_HWORD_SWAP          (uint32)(0x1 << 4)

#define VOP_CON_BYTE_NO_SWAP        (uint32)(0x0 << 5)
#define VOP_CON_BYTE_SWAP           (uint32)(0x1 << 5)

#define VOP_CON_YUV_BT601_L         (uint32)(0x0 << 6)
#define VOP_CON_YUV_BT701_L         (uint32)(0x1 << 6)
#define VOP_CON_YUV_BT601_F         (uint32)(0x2 << 6)
#define VOP_CON_YUV_MASK            (uint32)(0x3 << 6)

#define VOP_CON_FORMAT_RGB565       (uint32)(0x0 << 8)
#define VOP_CON_FORMAT_YUV420       (uint32)(0x1 << 8)
#define VOP_CON_FORMAT_MASK         (uint32)(0x1 << 8)

#define VOP_CON_YUV420_UV_KEEP      (uint32)(0x0 << 9)
#define VOP_CON_YUV420_UV_SWAP      (uint32)(0x1 << 9)

#define VOP_CON_DITHER_UP           (uint32)(0x0 << 10)
#define VOP_CON_DITHER_DWON         (uint32)(0x1 << 10)

#define VOP_CON_NOT_BYPASS_FIFO     (uint32)(0x0 << 11)
#define VOP_CON_BYPASS_FIFO         (uint32)(0x1 << 11)
#define VOP_CON_FIFO_MODE_MASK      (uint32)(0x1 << 11)

#define VOP_CON_AUTO_CLK_DISABLE    (uint32)(0x0 << 12)
#define VOP_CON_AUTO_CLK_EN         (uint32)(0x1 << 12)


#define VOP_INT_TYPE_FIFO_FULL      (uint32)(0x1 << 2)
#define VOP_INT_TYPE_FIFO_EMPTY     (uint32)(0x1 << 1)
#define VOP_INT_TYPE_FRAME_DONE     (uint32)(0x1 << 0)

#define VOP_INT_TYPE_RAW_FIFO_FULL  (uint32)(0x1 << 5)
#define VOP_INT_TYPE_RAW_FIFO_EMPTY (uint32)(0x1 << 4)
#define VOP_INT_TYPE_RAW_FRAME_DONE (uint32)(0x1 << 3)






/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable declare
*
*---------------------------------------------------------------------------------------------------------------------
*/



/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API Declare
*
*---------------------------------------------------------------------------------------------------------------------
*/
extern uint32 VopGetDmaReg(uint32 VopId);
extern rk_err_t VopSetMcu_FIFO_WATERMARK(uint32 VopId,int full_watermark,int empty_watermark);
extern rk_err_t VopSetStart(uint32 VopId ,uint32 mode);
extern rk_err_t VopEnableClk(uint32 VopId,uint32 IntType);
extern rk_err_t VopSetMcuIdle(uint32 VopId,uint32 idleFlag);
extern uint32 VopIntClear(uint32 VopId);
extern rk_err_t VopReset(uint32 VopId);
extern rk_err_t VopSetUvswap(uint32 VopId,uint32 mode);
extern rk_err_t VopSetFifoMode(uint32 VopId, uint32 fifomode);
extern rk_err_t VopDisableInt(uint32 VopId, uint32 IntType);
extern rk_err_t VopEnableInt(uint32 VopId, uint32 IntType);
extern uint32  VopGetInt(uint32 VopId);
extern rk_err_t VopSetYUVType(uint32 VopId, uint32 YuvType);
extern rk_err_t VopSetDisplayMode(uint32 VopId, uint32 DisplayMode);
extern rk_err_t VopSetSplit(uint32 VopId, uint32 mode);
extern rk_err_t VopSetWidthHeight(uint32 VopId, uint32 Width, uint32 Height);
extern rk_err_t VopSetTiming(uint32 VopId, uint32 csrw, uint32 rwpw, uint32 rwcs);
extern rk_err_t VopReadData(uint32 VopId, uint32 * data);
extern rk_err_t VopReadCmd(uint32 VopId, uint32 * cmd);
extern rk_err_t VopSendData(uint32 VopId, uint32 data);
extern rk_err_t VopSendCmd(uint32 VopId, uint32 cmd);
extern rk_err_t VopSetMode(uint32 VopId, uint32 mode);



#endif

