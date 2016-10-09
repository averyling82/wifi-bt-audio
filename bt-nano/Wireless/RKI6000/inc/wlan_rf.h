#ifndef __WLAN_RF_H_
#define __WLAN_RF_H_

#define NumRates        (13)

#define NUM_CHANNELS    15

struct  regulatory_class
{
    uint32  starting_freq;                  //MHz,
    uint8   channel_set[NUM_CHANNELS];
    uint8   channel_cck_power[NUM_CHANNELS];//dbm
    uint8   channel_ofdm_power[NUM_CHANNELS];//dbm
    uint8   txpower_limit;                  //dbm
    uint8   channel_spacing;                //MHz
    uint8   modem;
};


enum    _REG_PREAMBLE_MODE
{
    PREAMBLE_LONG   = 1,
    PREAMBLE_AUTO   = 2,
    PREAMBLE_SHORT  = 3
};


// Bandwidth Offset
#define HAL_PRIME_CHNL_OFFSET_DONT_CARE 0
#define HAL_PRIME_CHNL_OFFSET_LOWER 1
#define HAL_PRIME_CHNL_OFFSET_UPPER 2

// Represent Channel Width in HT Capabilities
//
typedef enum _HT_CHANNEL_WIDTH
{
    HT_CHANNEL_WIDTH_20 = 0,
    HT_CHANNEL_WIDTH_40 = 1
} HT_CHANNEL_WIDTH, *PHT_CHANNEL_WIDTH;

//
// Represent Extention Channel Offset in HT Capabilities
// This is available only in 40Mhz mode.
//
typedef enum _HT_EXTCHNL_OFFSET
{
    HT_EXTCHNL_OFFSET_NO_EXT = 0,
    HT_EXTCHNL_OFFSET_UPPER = 1,
    HT_EXTCHNL_OFFSET_NO_DEF = 2,
    HT_EXTCHNL_OFFSET_LOWER = 3
} HT_EXTCHNL_OFFSET, *PHT_EXTCHNL_OFFSET;

/* 2007/11/15 MH Define different RF type. */
typedef enum _WLAN_RF_TYPE_DEFINITION
{
    RF_1T2R = 0,
    RF_2T4R = 1,
    RF_2T2R = 2,
    RF_1T1R = 3,
    RF_2T2R_GREEN = 4,
    RF_819X_MAX_TYPE = 5
} WLAN_RF_TYPE_DEF_E;

#endif

