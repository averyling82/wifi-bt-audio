/*
********************************************************************************************
*
*                  Copyright (c):  Fuzhou Rockchip Electronics Co., Ltd
*                               All rights reserved.
*
* FileName: Common\Driver\PMU\hw_pmu.h
* Owner: aaron.sun
* Date: 2015.3.2
* Time: 18:31:08
* Desc:
* History:
*    <author>     <date>       <time>     <version>       <Desc>
*    aaron.sun     2015.3.2     18:31:08   1.0
********************************************************************************************
*/

#ifndef __COMMON_DRIVER_PMU_HW_PMU_H__
#define __COMMON_DRIVER_PMU_HW_PMU_H__

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
typedef volatile struct {

    UINT32 PMU_WAKEUP_CFG0;
    UINT32 PMU_WAKEUP_CFG1;
    UINT32 PMU_WAKEUP_CFG2;
    UINT32 PMU_PWRDN_CN;
    UINT32 PMU_PWRDN_ST;
    UINT32 PMU_POWER_MODE_CON;
    UINT32 PAD;
    UINT32 PMU_OSC_CNT;
    UINT32 PMU_CORE_PWRDW_CNT;
    UINT32 PMU_CORE_PWRUP_CNT;
    UINT32 PMU_SOFT_CON;
    UINT32 PMU_PLLLOCK_CNT;
    UINT32 PMU_INT_CON;
    UINT32 PMU_INT_ST;
    UINT32 PMU_GPIO_POS_INT_ST;
    UINT32 PMU_GPIO_NEG_INT_ST;
    UINT32 PMU_SYS_REG0;
    UINT32 PMU_SYS_REG1;
    UINT32 PMU_SYS_REG2;
    UINT32 PMU_SYS_REG3;
    UINT32 PAD1[4];
    UINT32 PMU_GPIO_POS_INT_CON;
    UINT32 PMU_GPIO_NEG_INT_CON;
    UINT32 PAD2[6];
    UINT32 PMU_SOFTRST_CON;

} PMU_REG, *gPMU_REG;

#define Pmu_Reg ((PMU_REG *)PMU_BASE)


//PMU_WAKEUP_CFG0
#define      PMU_GPIO_INT_WAKEUP_ENABLE          ((UINT32)1<<1)


//PMU_WAKEUP_CFG1
#define PMU_GPIO2A_POSEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<0)
#define PMU_GPIO2B_POSEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<8)
#define PMU_GPIO2C_POSEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<16)
#define PMU_GPIO2D_POSEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<24)


//PMU_WAKEUP_CFG2
#define PMU_GPIO2A_NEGEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<0)
#define PMU_GPIO2B_NEGEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<8)
#define PMU_GPIO2C_NEGEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<16)
#define PMU_GPIO2D_NEGEDGE_PLUSE_WAKEUP_ENABLE   ((UINT32)0XFF<<24)


//PMU_PWRDN_CN
//PMU_PWRDN_ST
#define PMU_PD_LOGIC_POWER_DOWN                  ((UINT32)1<<0)
#define PMU_PD_HIGH_POWER_DOWN                   ((UINT32)1<<1)


//PMU_POWER_MODE_CON
#define PMU_POWER_MODE_EN                                   ((UINT32)1<<0)
#define PMU_POWER_MODE_GLOBAL_INTRERUPT_DISABLE             ((UINT32)1<<1)
#define PMU_POWER_MODE_POWER_DOWN_VD_LOGIC                  ((UINT32)1<<2)
#define PMU_POWER_MODE_USE_LOW_FREQ_EN                      ((UINT32)1<<3)
#define PMU_POWER_MODE_INT_ENABLE                           ((UINT32)1<<5)
#define PMU_POWER_MODE_OSC_POWER_DOWN_EN                    ((UINT32)1<<6)
#define PMU_POWER_MODE_PLL_POWER_DOWN_EN                    ((UINT32)1<<7)


//PMU_SOFT_CON
#define PMU_USE_LOW_FREQ_CLK_EN                  ((UINT32)1<<0)
#define PMU_LOW_FREQ_CLK_SEL_PVTM                ((UINT32)1<<1)
#define PMU_LOW_FREQ_CLK_SEL_24M                 ((UINT32)0<<1)
#define PMU_OSC_POWER_DOWN_EN                    ((UINT32)1<<2)


//PMU_INT_CON
#define PMU_POWER_MODE_WAKEUP_INT_EN            ((UINT32)1<<1)
#define PMU_PD_HIGH_INT_EN                      ((UINT32)1<<2)
#define PMU_PD_LOW_INT_EN                       ((UINT32)1<<3)


//PMU_INT_ST
#define PMU_WAKEUP_FROM_POWER_MODE              ((UINT32)1<<1)
#define PMU_WAKEUP_BY_PD_HIGH                   ((UINT32)1<<2)
#define PMU_WAKEUP_BY_INTERRUPT_GPIO            ((UINT32)1<<3)
#define PMU_WAKEUP_BY_INTERRUPT_VD_LOGIC        ((UINT32)1<<5)
#define PMU_CAL_CORE_SLEEP                      ((UINT32)1<<6)


//PMU_GPIO_POS_INT_ST
#define PMU_WAKEUP_BY_GPIO2A_POSEDGE_PLUS       ((UINT32)0XFF<<0)
#define PMU_WAKEUP_BY_GPIO2B_POSEDGE_PLUS       ((UINT32)0XFF<<8)
#define PMU_WAKEUP_BY_GPIO2C_POSEDGE_PLUS       ((UINT32)0XFF<<16)
#define PMU_WAKEUP_BY_GPIO2D_POSEDGE_PLUS       ((UINT32)0XFF<<24)

//PMU_GPIO_NEG_INT_ST
#define PMU_WAKEUP_BY_GPIO2A_NEGEDGE_PLUS       ((UINT32)0XFF<<0)
#define PMU_WAKEUP_BY_GPIO2B_NEGEDGE_PLUS       ((UINT32)0XFF<<8)
#define PMU_WAKEUP_BY_GPIO2C_NEGEDGE_PLUS       ((UINT32)0XFF<<16)
#define PMU_WAKEUP_BY_GPIO2D_NEGEDGE_PLUS       ((UINT32)0XFF<<24)

//PMU_GPIO_POS_INT_CON
#define PMU_GPIO2A_POSEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<0)
#define PMU_GPIO2B_POSEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<8)
#define PMU_GPIO2C_POSEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<16)
#define PMU_GPIO2D_POSEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<24)

//PMU_GPIO_NEG_INT_CON
#define PMU_GPIO2A_NEGEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<0)
#define PMU_GPIO2B_NEGEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<8)
#define PMU_GPIO2C_NEGEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<16)
#define PMU_GPIO2D_NEGEDGE_PLUS_INT_ENABLE       ((UINT32)0XFF<<24)

//PMU_SOFTRST_CON
#define PMU_SRAM_RST_SEL       ((UINT32)1<<0)
#define PMU_GPIO2_RST_SEL      ((UINT32)1<<1)
#define PMU_GRF_RST_SEL        ((UINT32)1<<2)
#define PMU_PVTM_RST_SEL       ((UINT32)1<<3)
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



#endif

