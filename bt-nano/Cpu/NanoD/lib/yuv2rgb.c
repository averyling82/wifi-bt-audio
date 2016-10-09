/*
********************************************************************************************
*
*                Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                             All rights reserved.
*
* FileName: Cpu\NanoD\lib\hw_yuv2rgb.c
* Owner: WJR
* Date: 2014.11.12
* Time: 10:54:06
* Desc:
* History:
*    <author>    <date>       <time>     <version>     <Desc>
*    WJR     2014.11.12     10:54:06   1.0
********************************************************************************************
*/

#include "BspConfig.h"
#ifdef __CPU_NANOD_LIB_HW_YUV2RGB_C__

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #include define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#include "RKOS.h"
#include "Bsp.h"
#include "dma.h"
#include "hw_160_128_uvy_testdata.h"
#include "hw_160_128_rgb565_testdata.h"
#include "hw_160_128_yuv_testdata.h"

/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   #define / #typedef define
*
*---------------------------------------------------------------------------------------------------------------------
*/

#define _CPU_NANOD_LIB_HW_YUV2RGB_READ_  __attribute__((section("cpu_nanod_lib_hw_yuv2rgb_read")))
#define _CPU_NANOD_LIB_HW_YUV2RGB_WRITE_ __attribute__((section("cpu_nanod_lib_hw_yuv2rgb_write")))
#define _CPU_NANOD_LIB_HW_YUV2RGB_INIT_  __attribute__((section("cpu_nanod_lib_hw_yuv2rgb_init")))
#define _CPU_NANOD_LIB_HW_YUV2RGB_SHELL_  __attribute__((section("cpu_nanod_lib_hw_yuv2rgb_shell")))
#define DMA_TRAN
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
uint32 YUV_DmaFinish;
uint32 YUV_DMA_CH = 0;
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   global variable define
*
*---------------------------------------------------------------------------------------------------------------------
*/
#define VOP_HEIGNT 128
#define VOP_WIDTH  160
DMA_LLP yuv_llpListn[DMA_CHN_MAX][VOP_HEIGNT];


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
** Name: send_UVY_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API void send_UVY_data(uint32 *src)
{
    int i,j;
    for(i=0;i<(VOP_HEIGNT/2);i++)
    {
        for(j = 0; j <(VOP_WIDTH / 4);j++)
        {
            VopSendData(0, *src++);
        }
        for(j = 0; j <(VOP_WIDTH/ 4);j++)
        {
            VopSendData(0,*src++);
        }
        for(j = 0; j <(VOP_WIDTH/ 4);j++)
 {
            VopSendData(0, *src++);
        }
    }
}
/*******************************************************************************
** Name: send_Y_UV_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API void send_Y_UV_data(uint32 *pst_y,uint32 *pst_uv)
{
   int i,j;
   for(i=0;i<(VOP_HEIGNT/2);i++)
    {
        for(j = 0; j <(VOP_WIDTH/4);j++)
        {
            VopSendData(0, *pst_uv++);
        }
        for(j = 0; j <(VOP_WIDTH/4);j++)
        {
            VopSendData(0, *pst_y++);
        }
        for(j = 0; j <(VOP_WIDTH/4);j++)
        {
            VopSendData(0, *pst_y++);
        }
    }
}
/*******************************************************************************
** Name: send_rgb_data
** Input:uint32 *src
** Return: void
** Owner:WJR
** Date: 2014.11.27
** Time: 17:34:19
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API void send_rgb_data(uint16 *src)
{
   int i,j;
   for(i=0;i<(VOP_HEIGNT);i++)
    {
        for(j = 0; j <(VOP_WIDTH /2);j++)
        {
            VopSendData(0, *src++);
        }
        for(j = 0; j <(VOP_WIDTH/2);j++)
        {
            VopSendData(0, *src++);
        }
    }
}
/*******************************************************************************
** Name: YuvSetMode
** Input:uint32 width, uint32 Height, uint32 Version
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 19:18:23
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API rk_err_t YuvSetMode(uint32 RawFormat, uint32 Version)
{

    VopSetDisplayMode(0,RawFormat); //8
    VopSetSplit(0, VOP_CON_SPLIT_TWO);//  1
    VopSetUvswap(0,VOP_CON_YUV420_UV_KEEP);//9
    VopSetFifoMode(0, VOP_CON_NOT_BYPASS_FIFO); //11
    VopSetMcuIdle(0,VOP_CON_IDLE_INPUT);//0
    VopEnableClk(0,VOP_CON_AUTO_CLK_DISABLE);//12

    if(Version ==  YUV_VERSION_0)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT601_L);
    }
    else if(Version == YUV_VERSION_1)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT701_L);
    }
    else if(Version == YUV_VERSION_2)
    {
        VopSetYUVType(0, VOP_CON_YUV_BT601_F);
    }
}

_CPU_NANOD_LIB_HW_YUV2RGB_READ_
void YUV_DmaIsr()
{
     YUV_DmaFinish = 1;
     //rk_print_string("\r\n yuv_DmaIsr\n");
     DmaDisableInt(YUV_DMA_CH);
}
/*******************************************************************************
** Name:  RgbWrite
** Input:uint32  VopId,uint32 Addr, uint32 width, uint32 Height
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.29
** Time: 17:07:11
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API rk_err_t Rgb565Write(uint32  VopId,uint32 Addr, uint32 width, uint32 Height)
{
    int i ,j,total_size;
    VOP * pVop = VopPort(VopId);
    uint32 YUV_ACC_addr =(uint32)&pVop->VopMcuData ;

    ScuClockGateCtr(CLOCK_GATE_DMA, 1);     //open uart clk
    ScuSoftResetCtr(RST_DMA, 1); //open rst uart ip
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,YUV_DmaIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
    DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0};
    YUV_DMA_CH = 0;
    {
        YUV_DmaFinish = 0;
        DmaEnableInt(YUV_DMA_CH);
        DmaConfig(YUV_DMA_CH, Addr, YUV_ACC_addr,Height*width*2/4, &DmaCfg, yuv_llpListn[YUV_DMA_CH]);
        while(!YUV_DmaFinish);
        DmaDisableInt(YUV_DMA_CH);
    }
}
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API rk_err_t YuvWrite(uint32  VopId,uint32 Addr, uint32 width, uint32 Height)
{
    int i ,j,total_size;
    VOP * pVop = VopPort(VopId);
    uint32 YUV_ACC_addr =(uint32)&pVop->VopMcuData ;

    ScuClockGateCtr(CLOCK_GATE_DMA, 1);     //open uart clk
    ScuSoftResetCtr(RST_DMA, 1); //open rst uart ip
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,YUV_DmaIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
    DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0};
    YUV_DMA_CH = 0;
    {
        YUV_DmaFinish = 0;
        DmaEnableInt(YUV_DMA_CH);
        DmaConfig(YUV_DMA_CH, Addr, YUV_ACC_addr,Height*width*3/8, &DmaCfg, yuv_llpListn[YUV_DMA_CH]);
        while(!YUV_DmaFinish);
        DmaDisableInt(YUV_DMA_CH);
    }
}
/*******************************************************************************
** Name: YuvLlpWrite
** Input:uint32  Yaddr, uint32 UvAddr, uint32 width, uint32 Height
** Return: rk_err_t
** Owner:aaron.sun
** Date: 2014.11.24
** Time: 19:10:21
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_READ_
COMMON API rk_err_t YuvLlpWrite(uint32  VopId,uint32  Yaddr,uint32 UvAddr, uint32 width, uint32 Height)
{
    int i = 0,j;
    VOP * pVop = VopPort(VopId);
    uint32 YUV_ACC_addr = (uint32)&pVop->VopMcuData ;
    ScuClockGateCtr(CLOCK_GATE_DMA, 1);
    ScuSoftResetCtr(RST_DMA, 1);
    DelayMs(1);
    ScuSoftResetCtr(RST_DMA, 0);

    IntRegister(INT_ID_DMA ,YUV_DmaIsr);
    IntPendingClear(INT_ID_DMA);
    IntEnable(INT_ID_DMA);
    DMA_CFGX DmaCfg = {DMA_CTLL_VOP_WR, DMA_CFGL_VOP_WR, DMA_CFGH_VOP_WR, 0};
    YUV_DMA_CH = 0;
    YUV_DmaFinish = 0;
    DmaEnableInt(YUV_DMA_CH);
    for(i=0;i<Height/2;i++)
    {
            yuv_llpListn[YUV_DMA_CH][2*i].SAR = UvAddr;
            yuv_llpListn[YUV_DMA_CH][2*i].DAR = YUV_ACC_addr;
            yuv_llpListn[YUV_DMA_CH][2*i].SIZE = width/4;
            UvAddr += width;
            yuv_llpListn[YUV_DMA_CH][2*i+1].SAR = Yaddr;
            yuv_llpListn[YUV_DMA_CH][2*i+1].DAR = YUV_ACC_addr ;
            yuv_llpListn[YUV_DMA_CH][2*i+1].SIZE = width/2;
            Yaddr += width*2;
    }
    DmaConfig_for_LLP(YUV_DMA_CH, width/2, Height,&DmaCfg, yuv_llpListn[YUV_DMA_CH]);
    while(!YUV_DmaFinish)
    {
        __WFI();
    };
    DmaDisableInt(YUV_DMA_CH);
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


/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   API(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/
/*******************************************************************************
** Name: hw_yuv2rgb_shell
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.12
** Time: 14:23:26
*******************************************************************************/
#ifdef _USE_SHELL_
#ifdef _YUV2RGB_SHELL_
_CPU_NANOD_LIB_HW_YUV2RGB_SHELL_
uint32 yuv;

void YuvIsr()
{
    yuv = 1;
}
/*******************************************************************************
** Name: hw_yuv2rgb_shell
** Input:void
** Return: rk_err_t
** Owner:WJR
** Date: 2014.11.12
** Time: 14:23:26
*******************************************************************************/
_CPU_NANOD_LIB_HW_YUV2RGB_SHELL_
#if 1
SHELL API rk_err_t hw_yuv2rgb_shell(void)
{
    uint32 i,j;
    uint32 IntStatus;
    int frm_cnt = 0;
    int Tick  = 0,Time;
      long long SysTick;
    /*IntRegister(INT_ID_VOP ,YuvIsr);
    IntPendingClear(INT_ID_VOP);
    IntEnable(INT_ID_VOP);*/
    VopEnableInt(0, VOP_INT_TYPE_FRAME_DONE);

    VopIntClear(0);
      LcdSetWindow(0, 0, VOP_WIDTH-1, VOP_HEIGNT-1);
    VopSetTiming(0,5,5,5);
    SysTick = SysTickCounter;
    while(1)
    {
       // printf("%d\n",frm_cnt++);
              frm_cnt++;
            #if 1
       /******第一帧***YUV****/
        LcdSetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_0);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_UVY_data((uint32 *)testdata_UVY1_160_128);
#else
        YuvWrite(0, (uint32)testdata_UVY1_160_128, VOP_WIDTH, VOP_HEIGNT);
#endif

        while(VopGetInt(0) == 0) ;

        /******第二帧***YUV****/
        LcdSetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_1);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_UVY_data((uint32 *)testdata_UVY_160_128);
#else
        YuvWrite(0, (uint32)testdata_UVY_160_128, VOP_WIDTH, VOP_HEIGNT);
#endif
        while(VopGetInt(0) == 0);


        /******第三帧****RGB***/
        #ifndef DMA_TRAN
        LcdSetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_RGB565, YUV_VERSION_0);
        VopIntClear(0);
        send_rgb_data((uint16 *) testdata_160_128_RGB565);
        while(VopGetInt(0) == 0);
        #else
        LcdSetWindow(0, 0, VOP_WIDTH - 1, VOP_HEIGNT - 1);
        VopSetStart(0,1);
        VopSetWidthHeight(0, VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_RGB565, YUV_VERSION_0);
        VopSetSplit(0, VOP_CON_SPLIT_FOUR);//  1
        VopIntClear(0);
        Rgb565Write(0,(uint32)testdata_160_128_RGB565,VOP_WIDTH, VOP_HEIGNT);
                while(VopGetInt(0) == 0);
        #endif
         #endif
        /******第四帧*****128*160*YUV*/
        LcdSetWindow(0, 0,VOP_WIDTH - 1, VOP_HEIGNT - 1 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,VOP_WIDTH,  VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_2);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_Y_UV_data((uint32 *)testdata_Y_160_128,(uint32 *)testdata_UV_160_128);
#else
        YuvLlpWrite(0,(uint32 )testdata_Y_160_128,(uint32 )testdata_UV_160_128,VOP_WIDTH,VOP_HEIGNT);
#endif
        while(VopGetInt(0) == 0);
        #if 0
        LcdSetWindow(0, 0,239, 319 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,240,  320);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_2);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_Y_UV_data((uint32 *)testdata_Y_240_320,(uint32 *)testdata_UV_240_320);
#else
        YuvLlpWrite(0,(uint32 )testdata_Y_240_320,(uint32 )testdata_UV_240_320,240,320);
#endif
        while(VopGetInt(0) == 0);
      #endif
       if(frm_cnt == 300)
        {
           break;
        }
    }
    Tick = SysTickCounter -  SysTick;
    Time = Tick *10/10;
    rk_printf_no_time(" DMA yuv 30 frm  is %d(ms) --- %d\r\n", Time, Tick);
}
#else
SHELL API rk_err_t hw_yuv2rgb_shell(void)  // height 160 width 128
{
    uint32 i=0,j=0;
    uint32 IntStatus;
    int frm_cnt = 0;
    int Tick  = 0,Time;
    long long SysTick;
   /* IntRegister(INT_ID_VOP ,YuvIsr);
    IntPendingClear(INT_ID_VOP);
    IntEnable(INT_ID_VOP);*/
    VopEnableInt(0, VOP_INT_TYPE_FRAME_DONE);
    VopIntClear(0);
      LcdSetWindow(0, 0, VOP_WIDTH-1, VOP_HEIGNT-1);
        VopSetTiming(0,5,5,5);
     for(j=0;j<=16;j+=2)
    {
           for(i=0;i<=16;i+=2)
        {
            frm_cnt=0;
    SysTick = SysTickCounter;
    while(1)
    {
        LcdSetWindow(0, 0,VOP_WIDTH - 1 ,VOP_HEIGNT - 1 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_0);
        VopSetMcu_FIFO_WATERMARK(0,j,i);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_Y_UV_data((uint32 *)dataY1_128_160,(uint32 *)dataUV1_128_160);
#else
        YuvLlpWrite(0,(uint32 )dataY1_128_160,(uint32 )dataUV1_128_160,VOP_WIDTH,VOP_HEIGNT);
#endif
        while(VopGetInt(0) == 0);
        frm_cnt++;
        LcdSetWindow(0, 0,VOP_WIDTH - 1 ,VOP_HEIGNT - 1 );
        VopSetStart(0,1);
        VopSetWidthHeight(0,VOP_WIDTH, VOP_HEIGNT);
        YuvSetMode(VOP_CON_FORMAT_YUV420, YUV_VERSION_0);
        VopSetMcu_FIFO_WATERMARK(0,j,i);
        VopIntClear(0);
#ifndef DMA_TRAN
        send_Y_UV_data((uint32 *)dataY_128_160,(uint32 *)dataUV_128_160);
#else
        YuvLlpWrite(0,(uint32 )dataY_128_160,(uint32 )dataUV_128_160,VOP_WIDTH,VOP_HEIGNT);
#endif
        while(VopGetInt(0) == 0);
          frm_cnt++;
                if(frm_cnt == 300)
        {
           break;
    }
}
    Tick = SysTickCounter -  SysTick;
            Time = Tick *10/10;
            rk_printf_no_time("%d %d DMA+LLP  yuv 30 frm  is %d(ms) --- %d\r\n",j,i, Time, Tick);
       }
  }
}
#endif
#endif
#endif
/*
*---------------------------------------------------------------------------------------------------------------------
*
*                                                   local function(shell) define
*
*---------------------------------------------------------------------------------------------------------------------
*/



#endif

