#ifndef __WLAN_POWER_SAVE_H_
#define __WLAN_POWER_SAVE_H_

#define IDLE_ACTIVE_THRESHOLD       3

enum LPS_CTRL_TYPE
{
    LPS_CTRL_CONNECT=2,
    LPS_CTRL_DISCONNECT=3,
    LPS_CTRL_SPECIAL_PACKET=4,
    LPS_CTRL_ENTER_SCAN=5,
    LPS_CTRL_LEAVE_SCAN=6
};


// Firmware PS mode for control LPS. by tynli.
typedef enum _FW_CTRL_PS_MODE_NUM
{
    FW_PS_ACTIVE_MODE    =0, // Active/Continuous access.
    //Legacy
    FW_PS_MIN_MODE   =1,
    FW_PS_MAX_MODE   =2,
    FW_PS_SELF_DEFINED_MODE = 3,
    //UAPSD
    FW_PS_UAPSD_MIN_MODE = 4,
    FW_PS_UAPSD_MAX_MODE = 5,
    FW_PSE_UAPSD_SELF_DEFINED_MODE = 6,
    FW_PS_NUM_MODE= 7
} FW_CTRL_PS_MODE;

// Fw PS state for RPWM.
/*
 BIT[2:0] = HW state
 BIT[3] = Protocol PS state,   1: register active state , 0: register sleep state
 BIT[4] = sub-state
*/

#define FW_PS_GO_ON         BIT(0)
#define FW_PS_TX_NULL           BIT(1)
#define FW_PS_RF_ON         BIT(2)
#define FW_PS_REGISTER_ACTIVE   BIT(3)

#define FW_PS_ACK               BIT(6)
#define FW_PS_TOGGLE        BIT(7)

// BIT[0] = 1: 32k, 0: 40M
#define FW_PS_CLOCK_OFF     BIT(0)      // 32k
#define FW_PS_CLOCK_ON      0       // 40M

#define FW_PS_STATE_MASK    (0x0F)
#define FW_PS_STATE_HW_MASK (0x07)
#define FW_PS_STATE_INT_MASK (0x3F) // ISR_ENABLE, IMR_ENABLE, and PS mode should be inherited.

#define FW_PS_STATE(x)          (FW_PS_STATE_MASK & (x))
#define FW_PS_STATE_HW(x)   (FW_PS_STATE_HW_MASK & (x))
#define FW_PS_STATE_INT(x)      (FW_PS_STATE_INT_MASK & (x))
#define FW_PS_ISR_VAL(x)        ((x) & 0x70)
#define FW_PS_IMR_MASK(x)       ((x) & 0xDF)
#define FW_PS_KEEP_IMR(x)       ((x) & 0x20)


#define FW_PS_STATE_S0      (FW_PS_DPS)     //BIT(0)
#define FW_PS_STATE_S1      (FW_PS_LCLK)    //BIT(0)
#define FW_PS_STATE_S2      (FW_PS_RF_OFF)  //BIT(1)
#define FW_PS_STATE_S3      (FW_PS_ALL_ON)  //BIT(2)
#define FW_PS_STATE_S4      ((FW_PS_ST_ACTIVE) | (FW_PS_ALL_ON))    //BIT(3) | BIT(2)


#define FW_PS_STATE_ALL_ON          (FW_PS_CLOCK_ON) // ((FW_PS_RF_ON) | (FW_PS_REGISTER_ACTIVE))
#define FW_PS_STATE_RF_ON           (FW_PS_CLOCK_ON) // (FW_PS_RF_ON)
#define FW_PS_STATE_RF_OFF          (FW_PS_CLOCK_ON) // 0x0
#define FW_PS_STATE_RF_OFF_LOW_PWR  (FW_PS_CLOCK_OFF) // (FW_PS_STATE_RF_OFF)

#define FW_PWR_STATE_ACTIVE ((FW_PS_RF_ON) | (FW_PS_REGISTER_ACTIVE))
#define FW_PWR_STATE_RF_OFF 0

#define FW_PS_IS_ACK(x)             ((x) & FW_PS_ACK )
#define FW_PS_IS_CLK_ON(x)      ((x) & (FW_PS_RF_OFF |FW_PS_ALL_ON ))
#define FW_PS_IS_RF_ON(x)       ((x) & (FW_PS_ALL_ON))
#define FW_PS_IS_ACTIVE(x)          ((x) & (FW_PS_ST_ACTIVE))
#define FW_PS_IS_CPWM_INT(x)        ((x) & 0x40)

#define FW_CLR_PS_STATE(x)      ((x) = ((x) & (0xF0)))

#define GET_POWER_SAVE_CONTROL(pAdapter)    ((struct pwrctrl_struct*)(&((pAdapter)->pwr_ctrl)))


#define IS_IN_LOW_POWER_STATE(pAdapter, FwPSState)  \
    (FW_PS_STATE(FwPSState) == FW_PS_CLOCK_OFF)

//#define IS_PCIE_POWER_ON_IO_REG(offset)   (((offset < 0x100) || (offset >= 0x0300 && offset < 0x0400)) ? TRUE: FALSE)
////1A0->1AF is C2H, CPU must be alive, so we don't need to leave 32k
//#define IS_SDIO_POWER_ON_IO_REG(domain_id, offset) ( (domain_id == WLAN_IOREG_DOMAIN && (offset < 0x100)) ||(domain_id == SPI_LOCAL_DOMAIN)
//  /*||(domain_id == WLAN_IOREG_DOMAIN && (offset <= 0x1AF) && (offset >= 0x1A0))*/)

#define IS_SDIO_POWER_ON_IO_REG(domain_id, offset) ((domain_id == SPI_LOCAL_DOMAIN) || (domain_id == WLAN_IOREG_DOMAIN && (offset < 0x100)))

#define     HW_CLOCK_OFF_TIMEOUT        2 // unit: ms

struct reportpwrstate_parm
{
    uint8 mode;
    uint8 state; //the CPWM value
    uint16 rsvd;
};

#define LPS_DELAY_TIME  1*HZ // 1 sec

// RF state.
typedef enum _rt_rf_power_state
{
    rf_on,      // RF is on after RFSleep or RFOff
    rf_sleep,   // 802.11 Power Save mode
    rf_off,     // HW/SW Radio OFF or Inactive Power Save
    //=====Add the new RF state above this line=====//
    rf_max
} rt_rf_power_state;

enum _SS_LEVEL_
{
    SS_LEVEL_MIN,//rf on/off ,keep 8051
    SS_LEVEL_DEEP//card disable, 8051 reset
};

struct  pwrctrl_struct
{
    _sema lock;

    uint8 ips_on;
    uint8 mp_processing;

    uint8 bLeisurePs;
    uint8 bFwCurrentInPSMode;
    uint8 bAllowSwToChangeHwClock;  //by tynli. for 32k.
    uint8 bInLPSPeriod;  // by tynli. for 32k.
    // Record Fw PS mode status. Added by tynli. 2010.04.19.
    uint8 FwPSState;
    uint8 bFwClkChangeInProgress;
    uint8 bLowPowerEnable;  // 32k
    uint32 FwLPSLevel;  //level3: reg level2: rf level1: 32k
    uint32 SmartPs; // 1: open 0: close
    uint8 RegMaxLPSAwakeIntvl;
    uint8 FWCtrlPSMode;
    uint8 BcnMode;
    uint8 SwAntMode;
    uint8 BcnAntMode; //BIT(0):BcnMode; BIT(1):SwAntMode.
    uint8 dot11PowerSaveMode;
    uint32 DelayLPSLastTimeStamp;
    //int32 DhcpDelayTimes;

    uint8   bInternalAutoSuspend;
//===========================================
    uint8   ips_state_check_cnts;
    uint32  ips_processing;

    struct delayed_work ips_wq;

    rt_rf_power_state current_rfpwrstate;
    rt_rf_power_state   change_rfpwrstate;

    uint8       wepkeymask;
    uint8       bkeepfwalive;
    uint8       brfoffbyhw;
    //=========================================
};

void  wlan_ProcessForWiFiConnect(PADAPTER Adapter);
extern void wlan_dhcp_callback_handler (void *FunctionContext);
extern void wlan_dhcp_callback_wq (void *context, uint8 *data);
extern void wlan_init_pwrctrl_struct(ADAPTER *Adapter);
extern void wlan_free_pwrctrl_struct(ADAPTER * Adapter);
extern void LeaveAllPowerSaveMode(ADAPTER *Adapter, uint32 lps_leave);

#ifdef CONFIG_LPS
void PowerSaveMode(PADAPTER Adapter, boolean IsEnable);
void LeisurePSEnter(PADAPTER Adapter);
void LeisurePSLeave(PADAPTER Adapter);
void lps_ctrl_cmd(ADAPTER *Adapter, uint32 lps_ctrl_type, uint8 is_dhcp);
uint32 lps_can_process(ADAPTER *Adapter);

#endif

#ifdef CONFIG_WOWLAN_SUPPORT
void  wlan_wowlan_process(PADAPTER Adapter);
#endif

#ifdef CONFIG_LPS_SUPPORT
void  wlan_lps_process(PADAPTER Adapter);
#endif

#endif
