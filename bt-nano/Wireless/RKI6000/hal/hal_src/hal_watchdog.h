#ifndef __HAL_WATCHDOG_H__
#define __HAL_WATCHDOG_H__

//============================================================
// structure and define
//============================================================
typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
    DIG_TYPE_THRESH_HIGH    = 0,
    DIG_TYPE_THRESH_LOW = 1,
    DIG_TYPE_BACKOFF        = 2,
    DIG_TYPE_RX_GAIN_MIN    = 3,
    DIG_TYPE_RX_GAIN_MAX    = 4,
    DIG_TYPE_ENABLE         = 5,
    DIG_TYPE_DISABLE        = 6,
    DIG_OP_TYPE_MAX
} DIG_OP_E;

typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
    CCK_PD_STAGE_LowRssi = 0,
    CCK_PD_STAGE_HighRssi = 1,
    CCK_PD_STAGE_MAX = 3
} CCK_PDTH_E;


typedef enum tag_DIG_Connect_Definition
{
    DIG_STA_DISCONNECT = 0,
    DIG_STA_CONNECT = 1,
    DIG_STA_BEFORE_CONNECT = 2,
    DIG_MultiSTA_DISCONNECT = 3,
    DIG_MultiSTA_CONNECT = 4,
    DIG_CONNECT_MAX
} DIG_CONNECT_E;


#define     BW_AUTO_SWITCH_HIGH_LOW 25
#define     BW_AUTO_SWITCH_LOW_HIGH 30

#define     DIG_THRESH_HIGH         40
#define     DIG_THRESH_LOW          35

#define     FALSEALARM_THRESH_LOW   400
#define     FALSEALARM_THRESH_HIGH  1000

#define     DIG_MAX                 0x3e
#define     DIG_MIN                 0x1c

#define     DIG_FA_UPPER                0x32
#define     DIG_FA_LOWER                0x20
#define     DIG_FA_TH0                  0x20
#define     DIG_FA_TH1                  0x100
#define     DIG_FA_TH2                  0x200

#define     DIG_BACKOFF_MAX         12
#define     DIG_BACKOFF_MIN         (-4)
#define     DIG_BACKOFF_DEFAULT     10

#define     RATR_STA_INIT           0
#define     RATR_STA_HIGH           1
#define         RATR_STA_MIDDLE     2
#define         RATR_STA_LOW            3

#define     TX_POWER_NEAR_FIELD_THRESH_LVL2 74
#define     TX_POWER_NEAR_FIELD_THRESH_LVL1 67

#define     TxHighPwrLevel_Normal       0
#define     TxHighPwrLevel_Level1       1
#define     TxHighPwrLevel_Level2       2


//============================================================
// function prototype
//============================================================

void hal_watchdog_init(ADAPTER *Adapter);

void
hal_watchdog_func(
    ADAPTER*    Adapter
);

#endif
