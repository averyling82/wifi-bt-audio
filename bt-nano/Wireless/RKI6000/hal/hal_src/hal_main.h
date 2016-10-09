#ifndef __HAL_MAIN_H__
#define __HAL_MAIN_H__


//-----------------------------------------------------------
//
//  Rate
//
//-----------------------------------------------------------
// CCK Rates, TxHT = 0
#define DESC_RATE1M                 0x00
#define DESC_RATE2M                 0x01
#define DESC_RATE5_5M                   0x02
#define DESC_RATE11M                    0x03

// OFDM Rates, TxHT = 0
#define DESC_RATE6M                 0x04
#define DESC_RATE9M                 0x05
#define DESC_RATE12M                    0x06
#define DESC_RATE18M                    0x07
#define DESC_RATE24M                    0x08
#define DESC_RATE36M                    0x09
#define DESC_RATE48M                    0x0a
#define DESC_RATE54M                    0x0b

// MCS Rates, TxHT = 1
#define DESC_RATEMCS0               0x0c
#define DESC_RATEMCS1               0x0d
#define DESC_RATEMCS2               0x0e
#define DESC_RATEMCS3               0x0f
#define DESC_RATEMCS4               0x10
#define DESC_RATEMCS5               0x11
#define DESC_RATEMCS6               0x12
#define DESC_RATEMCS7               0x13
#define DESC_RATEMCS8               0x14
#define DESC_RATEMCS9               0x15
#define DESC_RATEMCS10              0x16
#define DESC_RATEMCS11              0x17
#define DESC_RATEMCS12              0x18
#define DESC_RATEMCS13              0x19
#define DESC_RATEMCS14              0x1a
#define DESC_RATEMCS15              0x1b
#define DESC_RATEMCS15_SG           0x1c
#define DESC_RATEMCS32              0x20


//======================================================
// Tx Related
//======================================================

#define MAX_TX_BUF_LEN                  1536
#define MAX_FW_IMAGE_SIZE               0x6800  //0x6000
#define FW_PAGE_SIZE                    128

#ifdef CONFIG_TEST_FW

#define DOWNLOAD_RAM_BASE_ADDR          0x0800
#define TX_STREAMQ_BASE_ADDR                0x2000
#define KEY_MEM_BASE_ADDR                   0x2b00
#define RX_STREAMQ_BASE_ADDR                0x2c00

#define BCNQ_HEAD_PAGE                      0x40
#define CPUQ_HEAD_PAGE                      0x48
#define KEY_MEM_HEAD_PAGE                   ((KEY_MEM_BASE_ADDR-TX_STREAMQ_BASE_ADDR)/(FW_PAGE_SIZE>>2))


#define TX_TOTAL_PAGE_NUMBER        63
#define TX_PAGE_BOUNDARY            (TX_TOTAL_PAGE_NUMBER + 1)
#define RX_BUFFER_SIZE_DWORD        (0x4000 - RX_STREAMQ_BASE_ADDR)

// (HPQ + LPQ + NPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER
#define PAGE_NUM_PUBQ           0x0f
#define PAGE_NUM_LPQ            0x10
#define PAGE_NUM_NPQ            0x10
#define PAGE_NUM_HPQ            0x10


#else

#define DOWNLOAD_RAM_BASE_ADDR          0x0600  //0x0800
#define TX_STREAMQ_BASE_ADDR                0x2000
#define KEY_MEM_BASE_ADDR                   0x3300  //0x2f00    //0x3b00
#define RX_STREAMQ_BASE_ADDR                0x3400  //0x3000    //0x3c00

#define BCNQ_HEAD_PAGE                      0x80    //0x60  //0xc0
#define CPUQ_HEAD_PAGE                      0x88    //0x68  //0xc8
#define KEY_MEM_HEAD_PAGE                   ((KEY_MEM_BASE_ADDR-TX_STREAMQ_BASE_ADDR)/(FW_PAGE_SIZE>>2))


#define TX_TOTAL_PAGE_NUMBER        127     //95        //191
#define TX_PAGE_BOUNDARY            (TX_TOTAL_PAGE_NUMBER + 1)
#define RX_BUFFER_SIZE_DWORD        (0x4000 - RX_STREAMQ_BASE_ADDR)

// (HPQ + LPQ + NPQ + PUBQ) shall be TX_TOTAL_PAGE_NUMBER
#define PAGE_NUM_PUBQ           0x3f    //0x1f  //0x6f
#define PAGE_NUM_LPQ            0x10
#define PAGE_NUM_NPQ            0x20    //0x20
#define PAGE_NUM_HPQ            0x10    //0x20
#endif

#define WMM_TX_TOTAL_PAGE_NUMBER    0xF5
#define WMM_TX_PAGE_BOUNDARY        (WMM_TX_TOTAL_PAGE_NUMBER + 1)

#define WMM_PAGE_NUM_PUBQ       0xB0
#define WMM_PAGE_NUM_HPQ            0x29
#define WMM_PAGE_NUM_LPQ            0x1C
#define WMM_PAGE_NUM_NPQ            0x1C


//-----------------------------------------------------------
//
//  Queue mapping
//
//-----------------------------------------------------------
#define BK_QUEUE                        0
#define BE_QUEUE                        1
#define VI_QUEUE                        2
#define VO_QUEUE                        3
#define BEACON_QUEUE                    4
#define TXCMD_QUEUE                 5
#define MGNT_QUEUE                      6
#define HIGH_QUEUE                      7
#define HCCA_QUEUE                      8

#define LOW_QUEUE                       BE_QUEUE
#define NORMAL_QUEUE                    MGNT_QUEUE

#define VO_QUEUE_INX                0
#define VI_QUEUE_INX                1
#define BE_QUEUE_INX                2
#define BK_QUEUE_INX                3
#define TS_QUEUE_INX                4
#define MGT_QUEUE_INX               5
#define BMC_QUEUE_INX               6
#define BCN_QUEUE_INX               7


#define TX_DESC_SIZE        32

#define ALL_DUMMY_LENGTH    (0)

#define TXDESC_OFFSET       (TX_DESC_SIZE + ALL_DUMMY_LENGTH)

#define SPI_HWDESC_HEADER_LEN       (TX_DESC_SIZE + ALL_DUMMY_LENGTH)

typedef struct _Phy_OFDM_Rx_Status_Report
{
    uint8           gain_trsw[4];
    uint8           pwdb_all;
    uint8           cfosho[4];
    uint8           cfotail[4];
    int8            rxevm[2];
    int8            rxsnr[4];
    uint8           pdsnr[2];
    uint8           csi_current[2];
    uint8           csi_target[2];
    uint8           sigevm;
    uint8           max_ex_pwr;
    uint8           reserve;

} PHY_STS_OFDM_T;


uint32 hal_queue_mapping(uint32 qsel);

uint32 hal_hw_init(PADAPTER Adapter, uint32 download_firmware);

uint32 hal_hw_halt(PADAPTER Adapter, uint32 IpsFlow);

uint32 hal_hw_reset(PADAPTER Adapter);

int32  hal_data_init (PADAPTER pAdapter);

void  hal_data_deinit (PADAPTER pAdapter);

void hal_check_ap_alive(ADAPTER * Adapter, struct sta_info  *psta);

uint8 hal_disconnect_handle(ADAPTER *Adapter, uint8 *pbuf);

uint32 hal_get_wlan_header_offset(void);

void  hal_set_network_type(PADAPTER Adapter, NDIS_802_11_NETWORK_INFRASTRUCTURE type);

void hal_set_bssid_check(PADAPTER Adapter, boolean check_bssid);

void hal_config_beacon(
    IN  PADAPTER    Adapter,
    IN  NDIS_802_11_NETWORK_INFRASTRUCTURE type,
    IN  uint16          BcnInterval,
    IN  uint16          AtimWindow
);

void hal_read_reg(PADAPTER Adapter, uint32 address, uint32 *val);
void hal_write_reg(PADAPTER Adapter, uint32 address, uint32 *val);
void hal_write_reg_dummy(PADAPTER Adapter, uint32 variable, uint32 *val);

void hal_fix_tx_rate(PADAPTER Adapter, uint8 rate);

void hal_get_ps_state(ADAPTER *Adapter, uint32 *val);

#ifdef CONFIG_IDLE_PS_SUPPORT
void hal_enter_idle_ps(ADAPTER *Adapter);
void hal_leave_idle_ps(ADAPTER *Adapter);
#endif

#endif

