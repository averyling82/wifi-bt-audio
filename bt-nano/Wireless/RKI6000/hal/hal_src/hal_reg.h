#ifndef __HAL_REG_H
#define __HAL_REG_H

#ifdef CONFIG_SPI_HCI
#include "spi_io.h"
#endif

#define TDE_CFG             0x0100
#define TDE_PGCFG           0x0104
#define TDE_STS                 0x0108
#define TDE_PGSTS           0x010c

#define TQM_CSR                 0x0200
#define CPUQ_CSR            0x021c
#define RETRY_LIMIT             0x0224
#define DATA_FB_TIME        0x0244
#define CTRL_FB_TIME        0x0248
#define CTRL_FB_TABLE       0x0238
#define DATA_FB_TABLE_LO    0x0230
#define DATA_FB_TABLE_HI    0x0234
#define CONTENTION_THRS     0x022c
#define INI_RATE_TABLE_LO   0x023c
#define INI_RATE_TABLE_HI   0x0240
#define PKT_LIFE_TIME       0x0228
#define RTS_CFG                 0x0220
#define DUMMY_REG           0x0258

#define SCH_CSR                     0x0300
#define VOQ_PARAMETER       0x0304
#define VIQ_PARAMETER           0x0308
#define BEQ_PARAMETER           0x030c
#define BKQ_PARAMETER           0x0310
#define BCQ_PARAMETER           0x0314
#define TRX_SIFS                    0x0318
#define TIMING_PARAMETER    0x031c
#define TXOP_CFG                    0x0320
#define ACM_CFG                     0x0324
#define VOQ_ACM                     0x0328
#define VIQ_ACM                     0x032c
#define BEQ_ACM                     0x0330
/*
#define BCN_CFG                 0x0334
#define BCN_SPACE               0x0338
#define TBTT_CFG                0x033c
#define ATIM_CFG                0x0340
#define TSF_LO                      0x0344
#define TSF_HI                      0x0348
#define P2P_CFG                 0x034c
#define TIMER1                      0x0350
#define TIMER2                      0x0354
#define TIMER3                      0x0358
*/
#define RAND_SET                    0x035c
//#define NOA_PARAMETER0       0x0360
//#define NOA_PARAMETER1       0x0364
//#define NOA_PARAMETER2       0x0368
//#define NOA_PARAMETER3       0x036c
#define DBG_MONITOR         0x0370

#define MAC_CSR                 0x0400
#define MAC_ADDR0_LO        0x0404
#define MAC_ADDR0_HI        0x0408
#define MAC_ADDR1_LO        0x040c
#define MAC_ADDR1_HI        0x0410
#define MAC_ADDR2_LO        0x0414
#define MAC_ADDR2_HI        0x0418
#define MAC_ADDR3_LO        0x041c
#define MAC_ADDR3_HI        0x0420
#define MAC_ADDR4_LO        0x0424
#define MAC_ADDR4_HI        0x0428
#define MAC_ADDR5_LO        0x042c
#define MAC_ADDR5_HI        0x0430
#define BSSID_LO            0x0434
#define BSSID_HI            0x0438
#define REG_AID                 0x043c
#define NC_CSR              0x0454
#define NC_DATA0            0x0458
#define NC_DATA1            0x045c
#define WOL_CTRL            0x0460
#define WOL_REASON          0x0464
// #define NC_DATA2             0x0460
// #define NC_DATA3             0x0464
// #define NC_DATA4             0x0468
// #define NC_DATA5             0x046c
#define RX_STREAMQ          0x044c
#define RESP_CHECK          0x0448
#define SIFS_TIMING             0x0440
#define RX_LATENCY          0x0444
#define BASIC_RATE          0x0450
#define NAV_TIMEOUT         0x0470
#define PHYREG_CTRL         0x0474
#define PHYREG_DATA         0x0478
#define TXPLT_CTRL          0x047c
#define TXPLT_DATA          0x0480
#define PKM_CTRL            0x0484
#define PKM_DATA            0x0488
#define BPS_CFG                 0x048c

#define RXPKT_TOTAL         0x0500

#define CSR                     0x0600
//#define CPU_ISR               0x0604
//#define CPU_IMR               0x0608
//#define CPU_MASKED_ISR        0x060c
//#define HOST_MSG0             0x0610
//#define HOST_MSG1             0x0614
#define RXPKT_CNT           0x0618
#define CPU_PRAM_BASE       0x061c
#define TX_STREAMQ_BASE     0x0620
#define RX_STREAMQ_BASE     0x0624
#define CPU_CNTL            0x0628
#define DEBUG_CFG           0x063c // RO for debug
//#define PKM_DEBUG             0x0640
//#define PLT_DEBUG             0x0644
//#define TQM_DEBUG             0x0648
//#define SCH_DEBUG             0x064c
#define HST_CMD0            0x0640
#define HST_CMD1            0x0644
#define HST_CMD2            0x0648
#define HST_CMD3            0x064c
#define TDE_DEBUG           0x0650
#define RDE_DEBUG           0x0654
#define MFP_DEBUG           0x0658
#define FW_STS0             0x0660
#define FW_STS1             0x0664
#define FW_STS2             0x0668
#define FW_STS3             0x066c
#define REG_DUMMY           0x06fc

#define ON_CSR              0x0700
#define BCN_CFG             0x0704
#define TBTT_CFG            0x0708
#define ATIM_CFG            0x070c
#define P2P_CFG             0x0710
#define TIMER1              0x0720
#define TIMER2              0x0724
#define TIMER3              0x0728
#define CPU_ISR                 0x072c
#define CPU_IMR                 0x0730
#define CPU_MASKED_ISR      0x0734
#define NOA_PARAMETER0_0    0x0740
#define NOA_PARAMETER0_1    0x0744
#define NOA_PARAMETER0_2    0x0748
#define NOA_PARAMETER0_3    0x074c
#define NOA_PARAMETER1_0    0x0750
#define NOA_PARAMETER1_1    0x0754
#define NOA_PARAMETER1_2    0x0758
#define NOA_PARAMETER1_3    0x075c
#define TSF_LO              0x0780
#define TSF_HI              0x0784
#define TBTT_LO             0x0788
#define TBTT_HI             0x078c

#define IO_ENABLE           0x7fc

#endif
