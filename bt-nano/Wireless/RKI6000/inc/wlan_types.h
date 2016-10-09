#ifndef __WLAN_TYPES_H__
#define __WLAN_TYPES_H__

#ifdef CONFIG_RX_QUEUE
#define WIFI_DSR_TASK_STACK_SIZE        (256*4*4)
#define WIFI_RX_TASK_STACK_SIZE         (256*2*4)
#define WIFI_TX_TASK_STACK_SIZE         (256*4)
#define WIFI_WLAN_TASK_STACK_SIZE   (256*4)

#define WIFI_DSR_TASK_PRORITY           0
#define WIFI_RX_TASK_PRORITY            1
#define WIFI_TX_TASK_PRORITY            1
#define WIFI_WLAN_TASK_PRORITY          0
#else
#define WIFI_DSR_TASK_STACK_SIZE        (256*4)
#define WIFI_RX_TASK_STACK_SIZE         (256*5*4)
#define WIFI_TX_TASK_STACK_SIZE         (256*4)
#define WIFI_WLAN_TASK_STACK_SIZE   (256*4)

#define WIFI_DSR_TASK_PRORITY           1
#define WIFI_RX_TASK_PRORITY            1
#define WIFI_TX_TASK_PRORITY            1
#define WIFI_WLAN_TASK_PRORITY          0
#endif

#define WIFI_DSR_TASK_QUEUE_NUM     1024
#define WIFI_RX_TASK_QUEUE_NUM      1024
#define WIFI_TX_TASK_QUEUE_NUM      512
#define WIFI_WLAN_TASK_QUEUE_NUM    512

#define WIFI_MP_TASK_PRORITY            2
#define WIFI_MP_TASK_QUEUE_NUM      512
#define WIFI_MP_TASK_STACK_SIZE         1024

#define MAX_RSVD_PAGE_PACKET_NUM        5

enum
{
    UP_LINK,
    DOWN_LINK
};

enum wlan_var_map
{
    /*reg map */

    /*efuse map */

    /*CAM map */

    /*IMR map */

    /*CCK Rates, TxHT = 0 */

    /*OFDM Rates, TxHT = 0 */

    /*Queue Index map*/
    QUEUE_INX_VO = 0,
    QUEUE_INX_VI,
    QUEUE_INX_BE,
    QUEUE_INX_BK,
    QUEUE_INX_TS,
    QUEUE_INX_MGT,
    QUEUE_INX_BMC,
    QUEUE_INX_BCN,

    /*others*/
    LENGTH_ALL_DUMMY,
    SIZE_TXDESC,
    OFFSET_TXDESC,
    /*keep it last */
    WLAN_VAR_SETTINGS_MAX
};

typedef enum _HW_VARIABLES
{
    HW_VAR_ETHER_ADDR,
    HW_VAR_BASIC_RATE,
    HW_VAR_BSSID,
    HW_VAR_MEDIA_STATUS,
    HW_VAR_SECURITY_CONF,
    HW_VAR_BEACON_INTERVAL,
    HW_VAR_ATIM_WINDOW,
    HW_VAR_SIFS,
    HW_VAR_DIFS,
    HW_VAR_EIFS,
    HW_VAR_SLOT_TIME,
    HW_VAR_ACK_PREAMBLE,
    HW_VAR_RETRY_COUNT,
    HW_VAR_WPA_CONFIG,          //2004/08/23, kcwu, for 8187 Security config
    HW_VAR_AMPDU_MIN_SPACE,     // The spacing between sub-frame. Roger, 2008.07.04.
    HW_VAR_SHORTGI_DENSITY,     // The density for shortGI. Roger, 2008.07.04.
    HW_VAR_AMPDU_FACTOR,
    HW_VAR_AC_PARAM,            // For AC Parameters, 2005.12.01, by rcnjko.
    HW_VAR_ACM_CTRL,            // For ACM Control, Annie, 2005-12-13.

    HW_VAR_RCR,                 //for RCR, David 2006,05,11
    HW_VAR_CHECK_BSSID,
    // Set Usb Rx Aggregation
    HW_VAR_RX_AGGR,
    HW_VAR_RETRY_LIMIT,
    HW_VAR_INIT_TX_RATE,        //Get Current Tx rate register. 2008.12.10. Added by tynli
    HW_VAR_EFUSE_USAGE,         //Get current EFUSE utilization. 2008.12.19. Added by Roger.
    HW_VAR_EFUSE_BYTES,
    HW_VAR_AUTOLOAD_STATUS,     //Get current autoload status, 0: autoload success, 1: autoload fail. 2008.12.19. Added by Roger.
    HW_VAR_CORRECT_TSF,         //Added by tynli. 2009.10.22. For Hw count TBTT time.
    HW_VAR_FWLPS_RF_ON,         //Added by tynli. 2009.11.09. For checking if Fw finishs RF on sequence.
    HW_VAR_DUAL_TSF_RST,        //Added by tynli. 2009.12.03. Suggested by TimChen.
    HW_VAR_USB_RX_AGGR_DMA,
    HW_VAR_USB_RX_AGGR_USBTH,
    HW_VAR_USB_RX_AGGR_USBTO,
    HW_VAR_USB_RX_AGGR_PGTH,
    HW_VAR_USB_RX_AGGR_PGTO,

    //fw lps
    HW_VAR_RESUME_CLK_ON,
    HW_VAR_TELLFW_CLK_OFF,
    HW_VAR_FW_PS_STATE,
    HW_VAR_FW_PSMODE_STATUS,        //Added by tynli. 2010.04.19.
    HW_VAR_SET_RPWM,
    HW_VAR_FW_CLK_CHANGE_STATE,

    HW_VAR_INIT_RTS_RATE,               //Added by tynli. 2010.11.16.
    HW_VAR_APFM_ON_MAC, //Auto FSM to Turn On, include clock, isolation, power control for MAC only
    HW_VAR_AGGR_TIMEOUT,    // For AMPDU per station on vwifi, 2009.11.18 by hpfan
    HW_VAR_AGGR_LIMIT,      // For AMPDU per station on vwifi, 2009.11.18 by hpfan
    HW_VAR_UPDATE_BCN_SUB_FUNC,
    HW_VAR_INIRTS_RATE_SEL,
    HW_VAR_REG_EDCA_BE,
    HW_VAR_H2C_FW_PWRMODE,
    HW_VAR_H2C_FW_JOINBSSRPT
} HW_VARIABLES;
typedef enum _HAL_DEF_VARIABLE
{
    HAL_DEF_RA_DECISION_RATE,
    HAL_DEF_RA_SGI,
    HAL_DEF_PT_PWR_STATUS,
    HAL_DEF_INT_MASK
} HAL_DEF_VARIABLE;

//-------------------------------------------------------------------------
//  Channel Plan
//-------------------------------------------------------------------------
enum ChannelPlan
{
    CHPL_FCC    = 0,
    CHPL_IC     = 1,
    CHPL_ETSI   = 2,
    CHPL_SPAIN  = 3,
    CHPL_FRANCE = 4,
    CHPL_MKK    = 5,
    CHPL_MKK1   = 6,
    CHPL_ISRAEL = 7,
    CHPL_TELEC  = 8,
    CHPL_GLOBAL = 9,
    CHPL_WORLD  = 10
};


enum THREAD_TYPE
{
    CMD_THX = 0,
    XMIT_THX,
    RECV_THX,
    INDI_THX,
    EVENT_THX,
    WORK_THX
};

struct config_struct
{
    //uint8 network_mode;   //infra, ad-hoc, auto
    NDIS_802_11_SSID    ssid;
    uint8   channel;//ad-hoc support requirement
    uint8   wireless_mode;//A, B, G, auto
    uint8   virtual_carrier_sense;//Enable, Disable, Auto
    uint8   vcs_type;//RTS/CTS, CTS-to-self
    uint16  rts_thresh;
    uint16  frag_thresh;
    uint8   preamble;//long, short, auto
    uint8  scan_mode;//active, passive

    uint8   software_encrypt;
    uint8   software_decrypt;

    //UAPSD
    uint8   wmm_enable;
    uint8   wsc_enable;

    WLAN_BSSID_EX    dev_network;

    uint8       ht_enable;
    uint8       bw40_enable;
    uint8       ampdu_enable;//for tx
    uint8       bsupport_shortgi;

    uint8       wifi_spec;// !turbo_mode

    uint8       channel_plan;
    boolean     bAcceptAddbaReq;
    boolean     bSendAddbaReq;
    boolean     bSupport8kAmsdu;

    uint8       usbss_enable;//0:disable,1:enable
#ifdef CONFIG_LAYER2_ROAMING
    uint8   max_roaming_times; // the max number driver will try to roaming
#endif
};


typedef struct _FALSE_ALARM_STATISTICS
{
    uint32  Cnt_Parity_Fail;
    uint32  Cnt_Rate_Illegal;
    uint32  Cnt_Crc8_fail;
    uint32  Cnt_Mcs_fail;
    uint32  Cnt_Ofdm_fail;
    uint32  Cnt_Cck_fail;
    uint32  Cnt_all;
    uint32  Cnt_Fast_Fsync;
    uint32  Cnt_SB_Search_fail;
} FALSE_ALARM_STATISTICS, *PFALSE_ALARM_STATISTICS;

typedef struct _RATE_ADAPTIVE
{
    uint8               RateAdaptiveDisabled;
    uint8               RATRState;
    uint16              reserve;

    uint32              HighRSSIThreshForRA;
    uint32              High2LowRSSIThreshForRA;
    uint8               Low2HighRSSIThreshForRA40M;
    uint32              LowRSSIThreshForRA40M;
    uint8               Low2HighRSSIThreshForRA20M;
    uint32              LowRSSIThreshForRA20M;
    uint32              UpperRSSIThresholdRATR;
    uint32              MiddleRSSIThresholdRATR;
    uint32              LowRSSIThresholdRATR;
    uint32              LowRSSIThresholdRATR40M;
    uint32              LowRSSIThresholdRATR20M;
    uint8               PingRSSIEnable; //cosa add for Netcore long range ping issue
    uint32              PingRSSIRATR;   //cosa add for Netcore long range ping issue
    uint32              PingRSSIThreshForRA;//cosa add for Netcore long range ping issue
    uint32              LastRATR;
    uint8               PreRATRState;

} RATE_ADAPTIVE, *PRATE_ADAPTIVE;


typedef struct _Dynamic_Initial_Gain_Threshold_
{
    uint8       Dig_Enable_Flag;
    uint8       Dig_Ext_Port_Stage;

    int     RssiLowThresh;
    int     RssiHighThresh;

    uint32      FALowThresh;
    uint32      FAHighThresh;

    uint8       CurSTAConnectState;
    uint8       PreSTAConnectState;
    uint8       CurMultiSTAConnectState;

    uint8       PreIGValue;
    uint8       CurIGValue;

    uint8       BackoffVal;
    uint8       BackoffVal_range_max;
    uint8       BackoffVal_range_min;
    uint8       rx_gain_range_max;
    uint8       rx_gain_range_min;
    uint8       Rssi_val_min;

    uint8       PreCCKPDState;
    uint8       CurCCKPDState;
    uint8       PreCCKFAState;
    uint8       CurCCKFAState;
    uint8       PreCCAState;
    uint8       CurCCAState;

    uint8       LargeFAHit;
    uint8       ForbiddenIGI;
    uint32      Recover_cnt;

} DIG_T;

/* Dynamic Calibration Method */
struct  dcm_struct
{
    uint8   dcm_type;

    //for DIG
    uint8   dig_enable;
    DIG_T   dig_table;

    FALSE_ALARM_STATISTICS false_alarm_cnt;

    //for rate adaptive
    uint8 ra_support;
    uint8 use_ra_mask;
    RATE_ADAPTIVE rate_adaptive;

    //* Upper and Lower Signal threshold for Rate Adaptive*/
    int UndecoratedSmoothedPWDB;
    int EntryMinUndecoratedSmoothedPWDB;
    int EntryMaxUndecoratedSmoothedPWDB;

    int32       RxPWDBAll;
    int32       RecvSignalPower;

    //for tx power tracking
    uint8 TXPowercount;
    uint8 bTXPowerTrackingInit;
    uint8 TxPowerTrackControl;  //for mp mode, turn off txpwrtracking as default

    uint8   ThermalValue;
    uint8   ThermalValue_LCK;
    uint8   ThermalValue_IQK;

    uint8 CCK_index;
    uint8 OFDM_index[2];
};

typedef struct _BB_REGISTER_DEFINITION
{
    uint32 rfintfs;         // set software control:
    //      0x870~0x877[8 bytes]

    uint32 rfintfi;         // readback data:
    //      0x8e0~0x8e7[8 bytes]

    uint32 rfintfo;         // output data:
    //      0x860~0x86f [16 bytes]

    uint32 rfintfe;         // output enable:
    //      0x860~0x86f [16 bytes]

    uint32 rf3wireOffset;   // LSSI data:
    //      0x840~0x84f [16 bytes]

    uint32 rfLSSI_Select;   // BB Band Select:
    //      0x878~0x87f [8 bytes]

    uint32 rfTxGainStage;   // Tx gain stage:
    //      0x80c~0x80f [4 bytes]

    uint32 rfHSSIPara1;     // wire parameter control1 :
    //      0x820~0x823,0x828~0x82b, 0x830~0x833, 0x838~0x83b [16 bytes]

    uint32 rfHSSIPara2;     // wire parameter control2 :
    //      0x824~0x827,0x82c~0x82f, 0x834~0x837, 0x83c~0x83f [16 bytes]

    uint32 rfSwitchControl; //Tx Rx antenna control :
    //      0x858~0x85f [16 bytes]

    uint32 rfAGCControl1;   //AGC parameter control1 :
    //      0xc50~0xc53,0xc58~0xc5b, 0xc60~0xc63, 0xc68~0xc6b [16 bytes]

    uint32 rfAGCControl2;   //AGC parameter control2 :
    //      0xc54~0xc57,0xc5c~0xc5f, 0xc64~0xc67, 0xc6c~0xc6f [16 bytes]

    uint32 rfRxIQImbalance; //OFDM Rx IQ imbalance matrix :
    //      0xc14~0xc17,0xc1c~0xc1f, 0xc24~0xc27, 0xc2c~0xc2f [16 bytes]

    uint32 rfRxAFE;         //Rx IQ DC ofset and Rx digital filter, Rx DC notch filter :
    //      0xc10~0xc13,0xc18~0xc1b, 0xc20~0xc23, 0xc28~0xc2b [16 bytes]

    uint32 rfTxIQImbalance; //OFDM Tx IQ imbalance matrix
    //      0xc80~0xc83,0xc88~0xc8b, 0xc90~0xc93, 0xc98~0xc9b [16 bytes]

    uint32 rfTxAFE;         //Tx IQ DC Offset and Tx DFIR type
    //      0xc84~0xc87,0xc8c~0xc8f, 0xc94~0xc97, 0xc9c~0xc9f [16 bytes]

    uint32 rfLSSIReadBack;  //LSSI RF readback data SI mode
    //      0x8a0~0x8af [16 bytes]

    uint32 rfLSSIReadBackPi;    //LSSI RF readback data PI mode 0x8b8-8bc for Path A and B

} BB_REGISTER_DEFINITION_T, *PBB_REGISTER_DEFINITION_T;

struct pkt_file
{
    _pkt *pkt;
    SIZE_T pkt_len;  //the remainder length of the open_file
    uint8 *cur_buffer;
    uint8 *buf_start;
    uint8 *cur_addr;
    SIZE_T buf_len;
};

//====================RX==============================
/* rx desc */
struct recv_stat
{
    uint32 rxdw0;

    uint32 rxdw1;

    uint32 rxdw2;

    uint32 rxdw3;

    uint32 rxdw4;

    uint32 rxdw5;
};

struct phy_cck_rx_status
{
    /* For CCK rate descriptor. This is a unsigned 8:1 variable. LSB bit presend
       0.5. And MSB 7 bts presend a signed value. Range from -64~+63.5. */
    uint8   adc_pwdb_X[4];
    uint8   sq_rpt;
    uint8   cck_agc_rpt;
};

struct phy_stat
{
    uint32 phydw0;

    uint32 phydw1;

    uint32 phydw2;

    uint32 phydw3;

    uint32 phydw4;

    uint32 phydw5;

    uint32 phydw6;
};

#define RF90_PATH_MAX           2
#define CHANNEL_MAX_NUMBER      14  // 14 is the max channel number
#define MAX_RF_PATH 4


#define     MAX_TX_COUNT                4
#define HWSET_MAX_SIZE                  512


/*
2.4G:
{
{1,2},
{3,4,5},
{6,7,8},
{9,10,11},
{12,13},
{14}
}

5G:
{
{36,38,40},
{44,46,48},
{52,54,56},
{60,62,64},
{100,102,104},
{108,110,112},
{116,118,120},
{124,126,128},
{132,134,136},
{140,142,144},
{149,151,153},
{157,159,161},
{173,175,177},
}
*/

#define MAX_CHNL_GROUP_24G      6
#define MAX_CHNL_GROUP_5G       14
/* halpg.h, put it here temp */
typedef struct _TxPowerInfo24G
{
    uint8 IndexCCK_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
    uint8 IndexBW40_Base[MAX_RF_PATH][MAX_CHNL_GROUP_24G];
    //If only one tx, only BW20 and OFDM are used.
    uint8 CCK_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 OFDM_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW20_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW40_Diff[MAX_RF_PATH][MAX_TX_COUNT];
} TxPowerInfo24G, *PTxPowerInfo24G;

typedef struct _TxPowerInfo5G
{
    uint8 IndexBW40_Base[MAX_RF_PATH][MAX_CHNL_GROUP_5G];
    //If only one tx, only BW20, OFDM, BW80 and BW160 are used.
    uint8 OFDM_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW20_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW40_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW80_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    uint8 BW160_Diff[MAX_RF_PATH][MAX_TX_COUNT];
} TxPowerInfo5G, *PTxPowerInfo5G;

struct hal_struct
{
    uint16 hardware_type;
    uint16 version_id;
    uint16 customer_id;
    uint16 fw_version;
    uint16 fw_subversion;
    uint8   board_type;

    uint8  rf_chip;
    WLAN_RF_TYPE_DEF_E rf_type;
    uint8   external_PA;
    uint8 total_rf_path;
    uint8   cck_in_ch14;

    uint8 last_cmd_box_num;
    uint32 host_cmd_in_progress;
    _lock  host_cmd_spinlock;

    _lock   spi_lock;

    uint8 common_page_offset;
    uint8 offload_page_offset;
    uint8 common_page_downloaded;

    uint32 intr_mask;
    uint32 intr_mask_to_set;
    _lock intr_mask_lock;

    _timer dhcp_callback_timer;

    const uint8 *fw_image;
    uint32 fw_image_len;

    const uint32 *agc_table;
    uint32 agc_table_len;

    uint32 recv_config;
    uint8 current_channel;
    HT_CHANNEL_WIDTH current_bandwidth;
    uint8 current_40mhz_prime_sc;// Control channel sub-carrier

    uint8 traffic_busy_threshold;
    uint8 tx_traffic_busy_threshold;
    uint8 rx_traffic_busy_threshold;
    uint8 idle_time_threshold;

    uint8 hw_desc_length;

    uint8 fw_ra_ctrl;

    uint8 b_fix_rate;
    uint8 fixed_tx_rate;

#if 0
    // The current Tx Power Level
    uint8   CurrentCckTxPwrIdx;
    uint8   CurrentOfdm24GTxPwrIdx;
    uint8   OriginalCckTxPwrIdx;
    uint8   OriginalOfdm24GTxPwrIdx;

    /* for phy parameter */
    uint8 Index24G_CCK_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
    uint8 Index24G_BW40_Base[MAX_RF_PATH][CHANNEL_MAX_NUMBER];
    //If only one tx, only BW20 and OFDM are used.
    int8 CCK_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    int8 OFDM_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    int8 BW20_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
    int8 BW40_24G_Diff[MAX_RF_PATH][MAX_TX_COUNT];
#endif
};


typedef struct hal_struct HAL_DATA_TYPE, *PHAL_DATA_TYPE;

struct hal_cfg
{

    uint32 settings[WLAN_VAR_SETTINGS_MAX];
};


enum _HCI_TYPE
{
    HCI_GSPI,
    HCI_SDIO,
    HCI_USB,
    HCI_PCIE
};

//will move to odm.h
//
// Define Different SW team support
//
#define ODM_AP          0x01    //BIT0 
#define ODM_ADSL        0x02    //BIT1
#define ODM_CE          0x04    //BIT2
#define ODM_MP          0x08    //BIT3

typedef struct _ODM_RATE_ADAPTIVE
{
    uint8               Type;               // DM_Type_ByFW/DM_Type_ByDriver
    uint8               HighRSSIThresh;     // if RSSI > HighRSSIThresh => RATRState is DM_RATR_STA_HIGH
    uint8               LowRSSIThresh;      // if RSSI <= LowRSSIThresh => RATRState is DM_RATR_STA_LOW
    uint8               RATRState;          // Current RSSI level, DM_RATR_STA_HIGH/DM_RATR_STA_MIDDLE/DM_RATR_STA_LOW
    uint32              LastRATR;           // RATR Register Content

} ODM_RATE_ADAPTIVE, *PODM_RATE_ADAPTIVE;

typedef struct _ODM_RA_Info_
{
    uint8 RateID;
    uint32 RateMask;
    uint32 RAUseRate;
    uint8 RateSGI;
    uint8 RssiStaRA;
    uint8 PreRssiStaRA;
    uint8 SGIEnable;
    uint8 DecisionRate;
    uint8 PreRate;
    uint8 HighestRate;
    uint8 LowestRate;
    uint32 NscUp;
    uint32 NscDown;
    uint16 RTY[5];
    uint32 TOTAL;
    uint16 DROP;
    uint8 Active;
    uint16 RptTime;
    uint8 RAWaitingCounter;
    uint8 RAPendingCounter;
} ODM_RA_INFO_T,*PODM_RA_INFO_T;


// BB ODM section BIT 0-15
#define ODM_BB_DIG                  BIT0
#define ODM_BB_RA_MASK          BIT1
#define ODM_BB_DYNAMIC_TXPWR        BIT2
#define ODM_BB_FA_CNT               BIT3
#define ODM_BB_RSSI_MONITOR     BIT4
#define ODM_BB_CCK_PD               BIT5
#define ODM_BB_ANT_DIV              BIT6
#define ODM_BB_PWR_SAVE         BIT7
#define ODM_BB_PWR_TRAIN            BIT8
#define ODM_BB_RATE_ADAPTIVE        BIT9
#define ODM_BB_PATH_DIV         BIT10
#define ODM_BB_PSD                  BIT11

// MAC DM section BIT 16-23
#define ODM_MAC_EDCA_TURBO      BIT16
#define ODM_MAC_EARLY_MODE      BIT17

// RF ODM section BIT 24-31
#define ODM_RF_TX_PWR_TRACK     BIT24
#define ODM_RF_RX_GAIN_TRACK        BIT25
#define ODM_RF_CALIBRATION          BIT26

//
// 2011/10/20 MH Define Common info enum for all team.
//
typedef enum _ODM_Common_Info_Definition
{
//-------------REMOVED CASE-----------//
    //ODM_CMNINFO_CCK_HP,
    //ODM_CMNINFO_RFPATH_ENABLE,        // Define as ODM write???
    //ODM_CMNINFO_BT_COEXIST,               // ODM_BT_COEXIST_E
    //ODM_CMNINFO_OP_MODE,              // ODM_OPERATION_MODE_E
//-------------REMOVED CASE-----------//

    //
    // Fixed value:
    //

    //-----------HOOK BEFORE REG INIT-----------//
    ODM_CMNINFO_PLATFORM = 0,
    ODM_CMNINFO_ABILITY,                    // ODM_ABILITY_E
    ODM_CMNINFO_INTERFACE,              // ODM_INTERFACE_E
    ODM_CMNINFO_MP_TEST_CHIP,
    ODM_CMNINFO_IC_TYPE,                    // ODM_IC_TYPE_E
    ODM_CMNINFO_CUT_VER,                    // ODM_CUT_VERSION_E
    ODM_CMNINFO_FAB_VER,                    // ODM_FAB_E
    ODM_CMNINFO_RF_TYPE,                    // ODM_RF_PATH_E or ODM_RF_TYPE_E?
    ODM_CMNINFO_BOARD_TYPE,             // ODM_BOARD_TYPE_E
    ODM_CMNINFO_EXT_LNA,                    // TRUE
    ODM_CMNINFO_EXT_PA,
    ODM_CMNINFO_EXT_TRSW,
    ODM_CMNINFO_PATCH_ID,               //CUSTOMER ID
    ODM_CMNINFO_BINHCT_TEST,
    ODM_CMNINFO_BWIFI_TEST,
    ODM_CMNINFO_SMART_CONCURRENT,
    //-----------HOOK BEFORE REG INIT-----------//


    //
    // Dynamic value:
    //
//--------- POINTER REFERENCE-----------//
    ODM_CMNINFO_MAC_PHY_MODE,           // ODM_MAC_PHY_MODE_E
    ODM_CMNINFO_TX_UNI,
    ODM_CMNINFO_RX_UNI,
    ODM_CMNINFO_WM_MODE,                // ODM_WIRELESS_MODE_E
    ODM_CMNINFO_BAND,                   // ODM_BAND_TYPE_E
    ODM_CMNINFO_SEC_CHNL_OFFSET,        // ODM_SEC_CHNL_OFFSET_E
    ODM_CMNINFO_SEC_MODE,               // ODM_SECURITY_E
    ODM_CMNINFO_BW,                     // ODM_BW_E
    ODM_CMNINFO_CHNL,

    ODM_CMNINFO_DMSP_GET_VALUE,
    ODM_CMNINFO_BUDDY_ADAPTOR,
    ODM_CMNINFO_DMSP_IS_MASTER,
    ODM_CMNINFO_BT_OPERATION,
    ODM_CMNINFO_BT_BUSY,                    //Check Bt is using or not//neil
    ODM_CMNINFO_BT_DISABLE_EDCA,
    ODM_CMNINFO_SCAN,
    ODM_CMNINFO_POWER_SAVING,
    ODM_CMNINFO_ONE_PATH_CCA,           // ODM_CCA_PATH_E
    ODM_CMNINFO_DRV_STOP,
    ODM_CMNINFO_PNP_IN,
    ODM_CMNINFO_INIT_ON,
    ODM_CMNINFO_ANT_TEST,
//--------- POINTER REFERENCE-----------//

//------------CALL BY VALUE-------------//
    ODM_CMNINFO_WIFI_DIRECT,
    ODM_CMNINFO_WIFI_DISPLAY,
    ODM_CMNINFO_LINK,
    ODM_CMNINFO_RSSI_MIN,
    ODM_CMNINFO_DBG_COMP,               // uint64
    ODM_CMNINFO_DBG_LEVEL,              // uint32
    ODM_CMNINFO_RA_THRESHOLD_HIGH,      // uint8
    ODM_CMNINFO_RA_THRESHOLD_LOW,       // uint8
    ODM_CMNINFO_RF_ANTENNA_TYPE,        // uint8
//------------CALL BY VALUE-------------//

    //
    // Dynamic ptr array hook itms.
    //
    ODM_CMNINFO_STA_STATUS,
    ODM_CMNINFO_PHY_STATUS,
    ODM_CMNINFO_MAC_STATUS,

    ODM_CMNINFO_MAX


} ODM_CMNINFO_E;

//  ODM_CMNINFO_INTERFACE
typedef enum tag_ODM_Support_Interface_Definition
{
    ODM_ITRF_PCIE   =   0,
    ODM_ITRF_USB    =   1,
    ODM_ITRF_SDIO   =   2,
    ODM_ITRF_ALL        =   3
} ODM_INTERFACE_E;


typedef enum _ODM_BaseBand_Config_Type
{
    ODM_BaseBand_Config_PHY_REG = 0,   //Radio Path A
    ODM_BaseBand_Config_AGC_TAB = 1,   //Radio Path B
    ODM_BaseBand_Config_AGC_TAB_2G = 2,
    ODM_BaseBand_Config_AGC_TAB_5G = 3
} ODM_BaseBand_Config_Type, *PODM_BaseBand_Config_Type;

//ODM_CMNINFO_CUT_VER
typedef enum tag_ODM_Cut_Version_Definition
{
    ODM_CUT_A       =   1,
    ODM_CUT_B       =   2,
    ODM_CUT_C       =   3,
    ODM_CUT_D       =   4,
    ODM_CUT_E       =   5,
    ODM_CUT_F       =   6,
    ODM_CUT_TEST    =   7
} ODM_CUT_VERSION_E;

// ODM_CMNINFO_FAB_VER
typedef enum tag_ODM_Fab_Version_Definition
{
    ODM_TSMC    =   0,
    ODM_UMC     =   1
} ODM_FAB_E;

typedef enum tag_ODM_RF_Type_Definition
{
    ODM_1T1R    =   0,
    ODM_1T2R    =   1,
    ODM_2T2R    =   2,
    ODM_2T3R    =   3,
    ODM_2T4R    =   4,
    ODM_3T3R    =   5,
    ODM_3T4R    =   6,
    ODM_4T4R    =   7
} ODM_RF_TYPE_E;

typedef enum _ANT_DIV_TYPE
{
    NO_ANTDIV               = 0xFF,
    CGCS_RX_SW_ANTDIV       = BIT1,
    CG_TRX_HW_ANTDIV        = 0x01,
    CGCS_RX_HW_ANTDIV       = 0x02,
    FIXED_HW_ANTDIV         = 0x03,
    CG_TRX_SMART_ANTDIV     = BIT4

} ANT_DIV_TYPE_E, *PANT_DIV_TYPE_E;
//will move to odm.h


struct _ADAPTER
{

    struct wifi_hci_ops hci_ops;
    struct wifi_indicate_intf   ind_intf;
    struct hal_ops hal_ops;
    struct hal_cfg hal_cfg;

    void            *net_if;
    boolean     interrupt_context;

    uint8   dev_addr[ETH_ALEN];

    uint8   hw_up;
    uint8   fw_ready;
    uint8   hw_init_completed;
    uint8   do_reset;
    uint8   driver_stopped;
    uint8   surprise_removed;
    uint8   dhcp_ip_got;
    uint8    host_sleep; // for test
    uint8   ip_addr[4];
    uint8   fw_lps_enabled;
    uint8   ps_state;
    uint8   in_idle_ps;

    uint32  malloc_count;
    uint8   event_seq;

    struct config_struct    config;
    struct mlme_struct  mlme;
    struct xmit_struct      xmit_priv;
    struct recv_struct      recv_priv;
    struct sta_struct       sta_global;
    struct security_struct  security;
    struct pwrctrl_struct   pwr_ctrl;
    struct hal_struct       hal_data;
    struct dcm_struct       dcm_ctrl;
    struct skb_priv     skb_priv;
    //struct cmd_struct     command;
    //struct evt_struct     event;
#ifdef CONFIG_SOFTAP_SUPPORT
    struct ap_info_struct   ap_info;
#endif
#ifdef INCLUDE_WPA_WPA2_PSK
    void    *wpa_priv;
#endif

    struct rx_stats         rx_stats;
    struct tx_stats     tx_stats;

    struct workqueue_struct *wlan_wq;
    struct delayed_work scan_work;
    struct delayed_work scan_timeout_work;
    struct delayed_work join_work;
    struct delayed_work join_timeout_work;
    struct delayed_work disassoc_work;
    //struct delayed_work event_work;
    struct delayed_work add_ba_work;
    struct delayed_work fw_msg_work;
    struct delayed_work link_retry_work;
    struct delayed_work dyn_chk_work;
    struct delayed_work eapol_retry_work;
    struct delayed_work derive_psk_work;
    struct delayed_work dhcp_callback_work;

    struct workqueue_struct *xmit_wq;
    struct delayed_work xmit_work;

    struct workqueue_struct *recv_wq;
    struct delayed_work recv_work;

//#ifdef CONFIG_SOFTAP_SUPPORT
    struct workqueue_struct *dsr_wq;
    struct delayed_work dsr_work;
//#endif

    uint32 pub_page;
    uint32 hi_page;
    uint32 low_page;
    uint32 mid_page;

    uint8 tx_fifo_processing;
    uint8 rx_fifo_processing;
    uint8 io_fifo_processing;
    uint8 h2c_processing;
    uint8 disable_dm;

    uint8 set_channel_done;
    uint8 config_phy_done;

    uint32 dhcp_doing;
    uint32 mp_rw_domain;

    uint32 imr_cfg;
};

typedef enum
{
    DOT11_ENC_NONE  = 0,
    DOT11_ENC_WEP40 = 1,
    DOT11_ENC_TKIP  = 2,
    DOT11_ENC_WRAP  = 3,
    DOT11_ENC_CCMP  = 4,
    DOT11_ENC_WEP104= 5,
    DOT11_ENC_WAPI  = 6
} DOT11_ENC_ALGO;

typedef struct _DOT11_SET_KEY
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint32   KeyIndex;
    uint32   KeyLen;
    uint8   KeyType;
    uint8   EncType;
    uint8   MACAddr[6];
    uint8   KeyRSC[8];
    uint8   KeyMaterial[64];
} DOT11_SET_KEY;

typedef struct _DOT11_DELETE_KEY
{
    uint8   EventId;
    uint8   IsMoreEvent;
    uint8            MACAddr[6];
    uint32   KeyIndex;
    uint8   KeyType;
} DOT11_DELETE_KEY;

typedef enum
{
    DOT11_KeyType_Group = 0,
    DOT11_KeyType_Pairwise = 1
} DOT11_KEY_TYPE;

typedef struct _DOT11_REQUEST
{
    uint8   EventId;
} DOT11_REQUEST;

typedef enum
{
    DOT11_EVENT_NO_EVENT = 1,
    DOT11_EVENT_REQUEST = 2,
    DOT11_EVENT_ASSOCIATION_IND = 3,
    DOT11_EVENT_ASSOCIATION_RSP = 4,
    DOT11_EVENT_AUTHENTICATION_IND = 5,
    DOT11_EVENT_REAUTHENTICATION_IND = 6,
    DOT11_EVENT_DEAUTHENTICATION_IND = 7,
    DOT11_EVENT_DISASSOCIATION_IND = 8,
    DOT11_EVENT_DISCONNECT_REQ = 9,
    DOT11_EVENT_SET_802DOT11 = 10,
    DOT11_EVENT_SET_KEY = 11,
    DOT11_EVENT_SET_PORT = 12,
    DOT11_EVENT_DELETE_KEY = 13,
    DOT11_EVENT_SET_RSNIE = 14,
    DOT11_EVENT_GKEY_TSC = 15,
    DOT11_EVENT_MIC_FAILURE = 16,
    DOT11_EVENT_ASSOCIATION_INFO = 17,
    DOT11_EVENT_INIT_QUEUE = 18,
    DOT11_EVENT_EAPOLSTART = 19,

    DOT11_EVENT_ACC_SET_EXPIREDTIME = 31,
    DOT11_EVENT_ACC_QUERY_STATS = 32,
    DOT11_EVENT_ACC_QUERY_STATS_ALL = 33,
    DOT11_EVENT_REASSOCIATION_IND = 34,
    DOT11_EVENT_REASSOCIATION_RSP = 35,
    DOT11_EVENT_STA_QUERY_BSSID = 36,
    DOT11_EVENT_STA_QUERY_SSID = 37,
    DOT11_EVENT_EAP_PACKET = 41,

    DOT11_EVENT_EAPOLSTART_PREAUTH = 45,
    DOT11_EVENT_EAP_PACKET_PREAUTH = 46,

    DOT11_EVENT_WPA2_MULTICAST_CIPHER = 47,
    DOT11_EVENT_WPA_MULTICAST_CIPHER = 48,

    DOT11_EVENT_MAX
} DOT11_EVENT;

//====================RX==============================


__inline static uint8 *get_my_bssid(WLAN_BSSID_EX *pnetwork)
{
    return (pnetwork->MacAddress);
}

#define GET_HAL_DATA(priv)  (&(priv)->hal_data)
#define GET_RF_TYPE(priv)   (GET_HAL_DATA(priv)->rf_type)

extern ADAPTER *padapter;

#endif //__DRV_TYPES_H__

