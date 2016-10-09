#ifndef __HAL_DESC_H
#define __HAL_DESC_H

#define SPI_TX_DESC_SIZE                32
#define RX_STATUS_DESC_SIZE         8

#undef LE_BITS_TO_4BYTE
#define LE_BITS_TO_4BYTE    BITS_TO_4BYTE

//DWORD 0
#define GET_RX_STATUS_DESC_PKT_LEN(__pRxStatusDesc)         LE_BITS_TO_4BYTE( __pRxStatusDesc, 0, 16)
#define GET_RX_STATUS_DESC_RX_MCS(__pRxStatusDesc)          LE_BITS_TO_4BYTE( __pRxStatusDesc, 16, 4)
#define GET_RX_STATUS_DESC_SPLCP(__pRxStatusDesc)           LE_BITS_TO_4BYTE( __pRxStatusDesc, 20, 1)
#define GET_RX_STATUS_DESC_LDPC(__pRxStatusDesc)                LE_BITS_TO_4BYTE( __pRxStatusDesc, 21, 1)
#define GET_RX_STATUS_DESC_CRC32(__pRxStatusDesc)           LE_BITS_TO_4BYTE( __pRxStatusDesc, 22, 1)
#define GET_RX_STATUS_DESC_MIC(__pRxStatusDesc)             LE_BITS_TO_4BYTE( __pRxStatusDesc, 23, 1)
#define GET_RX_STATUS_DESC_ICV(__pRxStatusDesc)             LE_BITS_TO_4BYTE( __pRxStatusDesc, 24, 1)
#define GET_RX_STATUS_DESC_SECURITY(__pRxStatusDesc)        LE_BITS_TO_4BYTE( __pRxStatusDesc, 25, 3)

//DWORD 1
#define GET_RX_STATUS_DESC_RSSI(__pRxStatusDesc)                LE_BITS_TO_4BYTE( __pRxStatusDesc+4, 0, 8)
#define GET_RX_STATUS_DESC_SNR(__pRxStatusDesc)             LE_BITS_TO_4BYTE( __pRxStatusDesc+4, 8, 8)
#define GET_RX_STATUS_DESC_SQ_LOW(__pRxStatusDesc)          LE_BITS_TO_4BYTE( __pRxStatusDesc+4, 16, 8)
#define GET_RX_STATUS_DESC_SQ_HIGH(__pRxStatusDesc)         LE_BITS_TO_4BYTE( __pRxStatusDesc+4, 24, 8)


#undef SET_BITS_TO_LE_4BYTE
#define SET_BITS_TO_LE_4BYTE    SET_BITS_TO_4BYTE
// Dword 0
#define SET_TX_DESC_PKT_SIZE(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc, 0, 16, __Value)
#define SET_TX_DESC_QUEUE_SEL(__pTxDesc, __Value)       SET_BITS_TO_LE_4BYTE(__pTxDesc, 16, 5, __Value)
#define SET_TX_DESC_RATE_ID(__pTxDesc, __Value)             SET_BITS_TO_LE_4BYTE(__pTxDesc, 21, 2, __Value)
#define SET_TX_DESC_USE_RATE(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc, 23, 1, __Value)
#define SET_TX_DESC_MACID(__pTxDesc, __Value)               SET_BITS_TO_LE_4BYTE(__pTxDesc, 24, 4, __Value)
#define SET_TX_DESC_SEC_TYPE(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc, 28, 3, __Value)

// Dword 1
#define SET_TX_DESC_MORE_FRAG(__pTxDesc, __Value)       SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 0, 1, __Value)
#define SET_TX_DESC_QOS(__pTxDesc, __Value)                 SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 1, 1, __Value)
#define SET_TX_DESC_PIFS(__pTxDesc, __Value)                SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 2, 1, __Value)
#define SET_TX_DESC_BMC(__pTxDesc, __Value)                 SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 3, 1, __Value)
#define SET_TX_DESC_NO_ACM(__pTxDesc, __Value)          SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 4, 1, __Value)
#define SET_TX_DESC_HW_NAV(__pTxDesc, __Value)          SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 5, 1, __Value)
#define SET_TX_DESC_RAW(__pTxDesc, __Value)                 SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 6, 1, __Value)
#define SET_TX_DESC_HW_RTS_ENABLE(__pTxDesc, __Value)   SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 8, 1, __Value)
#define SET_TX_DESC_RTS_ENABLE(__pTxDesc, __Value)      SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 9, 1, __Value)
#define SET_TX_DESC_CTS2SELF(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 10, 1, __Value)
#define SET_TX_DESC_RTS_SHORT(__pTxDesc, __Value)       SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 11, 1, __Value)
#define SET_TX_DESC_DATA_SHORT(__pTxDesc, __Value)      SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 12, 1, __Value)
#define SET_TX_DESC_DISABLE_RTS_FB(__pTxDesc, __Value)  SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 13, 1, __Value)
#define SET_TX_DESC_DISABLE_FB(__pTxDesc, __Value)      SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 14, 1, __Value)
#define SET_TX_DESC_HW_SEQ(__pTxDesc, __Value)          SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 15, 1, __Value)
#define SET_TX_DESC_RTS_RATE_FB_LIMIT(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 16, 4, __Value)
#define SET_TX_DESC_DATA_RATE_FB_LIMIT(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 20, 4, __Value)
#define SET_TX_DESC_DATA_RETRY_LIMIT(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 24, 6, __Value)
#define SET_TX_DESC_RETRY_LIMIT_ENABLE(__pTxDesc, __Value) SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 30, 1, __Value)
#define SET_TX_DESC_CCX_TAG(__pTxDesc, __Value)             SET_BITS_TO_LE_4BYTE(__pTxDesc+4, 31, 1, __Value)

// Dword 2
#define SET_TX_DESC_RTS_RATE(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 0, 4, __Value)
#define SET_TX_DESC_TX_RATE(__pTxDesc, __Value)             SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 4, 4, __Value)
//#define SET_TX_DESC_FINAL_DATA_RATE(__pTxDesc, __Value)   SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 8, 4, __Value)
#define SET_TX_DESC_RTS_RC(__pTxDesc, __Value)          SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 16, 6, __Value)
#define SET_TX_DESC_DATA_RC(__pTxDesc, __Value)             SET_BITS_TO_LE_4BYTE(__pTxDesc+8, 24, 6, __Value)

// Dword 3
#define SET_TX_DESC_TAIL_PAGE(__pTxDesc, __Value)           SET_BITS_TO_LE_4BYTE(__pTxDesc+12, 0, 8, __Value)
#define SET_TX_DESC_NEXT_HEAP_PAGE(__pTxDesc, __Value)  SET_BITS_TO_LE_4BYTE(__pTxDesc+12, 8, 8, __Value)
#define SET_TX_DESC_TSF_LOW(__pTxDesc, __Value)             SET_BITS_TO_LE_4BYTE(__pTxDesc+12, 16, 8, __Value)
#define SET_TX_DESC_TSF_HIGH(__pTxDesc, __Value)            SET_BITS_TO_LE_4BYTE(__pTxDesc+12, 24, 8, __Value)

#define PHY_STAT_GAIN_TRSW_SHT 0
#define PHY_STAT_PWDB_ALL_SHT 4
#define PHY_STAT_CFOSHO_SHT 5
#define PHY_STAT_CCK_AGC_RPT_SHT 5
#define PHY_STAT_CFOTAIL_SHT 9
#define PHY_STAT_RXEVM_SHT 13
#define PHY_STAT_RXSNR_SHT 15
#define PHY_STAT_PDSNR_SHT 19
#define PHY_STAT_CSI_CURRENT_SHT 21
#define PHY_STAT_CSI_TARGET_SHT 23
#define PHY_STAT_SIGEVM_SHT 25
#define PHY_STAT_MAX_EX_PWR_SHT 26

// Rx smooth factor
#define Rx_Smooth_Factor (20)

void hal_set_tx_desc(struct xmit_frame *pxmitframe, uint32 *pmem, int32 sz, uint8 frag_index);

void hal_parse_rx_desc(ADAPTER *pAdapter, struct rx_pkt_attrib *pattrib, uint8 *pbuf);

uint32 hal_get_rx_pkt_len(PADAPTER Adapter);

#endif
