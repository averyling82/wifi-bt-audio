#ifndef __SPI_IO_H__
#define __SPI_IO_H__


#define SPI_LOCAL_DOMAIN                0x0
#define WLAN_IOREG_DOMAIN           0x4
#define TX_HIQ_DOMAIN                   0x8
#define TX_MIQ_DOMAIN                   0x9
#define TX_LOQ_DOMAIN                   0xa
#define TX_BCNQ_DOMAIN              0xb
#define RX_RXFIFO_DOMAIN                0xf

//IO Bus domain address mapping
#define DEFUALT_OFFSET                  0x0
#define SPI_LOCAL_OFFSET                    0x10250000
#define WLAN_IOREG_OFFSET               0x10260000
#define FW_FIFO_OFFSET                  0x10270000
#define TX_HIQ_OFFSET                       0x10310000
#define TX_MIQ_OFFSET                   0x1032000
#define TX_LOQ_OFFSET                   0x10330000
#define RX_RXOFF_OFFSET                 0x10340000

//SPI Local registers
#define SPI_REG_TX_CTRL                 0x0000 // SPI Tx Control
#define SPI_REG_HISR_CLEAR              0x0004 // SPI HISR Clear reg
//#define SPI_REG_INT_TIMEOUT           0x0006
#define SPI_REG_HIMR                    0x0008 // SPI Host Interrupt Mask
#define SPI_REG_HISR                    0x000C // SPI Host Interrupt Service Routine
#define SPI_REG_RX0_REQ_LEN         0x0010 // RXDMA Request Length
#define SPI_REG_FREE_TXPG               0x0020 // Free Tx Buffer Page
#define SPI_REG_HCPWM1                  0x0024 // HCI Current Power Mode 1
#define SPI_REG_HCPWM2                  0x0026 // HCI Current Power Mode 2
#define SPI_REG_HTSFR_INFO              0x0030 // HTSF Informaion
#define SPI_REG_DUMMY                   0x003c
#define SPI_REG_HRPWM1                  0x0080 // HCI Request Power Mode 1
#define SPI_REG_HRPWM2                  0x0082 // HCI Request Power Mode 2
#define SPI_REG_HPS_CLKR                0x0084 // HCI Power Save Clock
#define SPI_REG_HSUS_CTRL               0x0086 // SPI HCI Suspend Control
#define SPI_REG_HIMR_ON             0x0090 //SPI Host Extension Interrupt Mask Always
#define SPI_REG_HISR_ON             0x0091 //SPI Host Extension Interrupt Status Always
#define SPI_REG_CFG                     0x00F0 //SPI Configuration Register

#define SPI_TX_CTRL                             (SPI_REG_TX_CTRL  |SPI_LOCAL_OFFSET)
#define SPI_HISR_CLEAR                          (SPI_REG_HISR_CLEAR |SPI_LOCAL_OFFSET)
#define SPI_HIMR                                (SPI_REG_HIMR |SPI_LOCAL_OFFSET)
#define SPI_HISR                                (SPI_REG_HISR |SPI_LOCAL_OFFSET)
#define SPI_RX0_REQ_LEN_1_BYTE      (SPI_REG_RX0_REQ_LEN |SPI_LOCAL_OFFSET)
#define SPI_FREE_TXPG                       (SPI_REG_FREE_TXPG |SPI_LOCAL_OFFSET)
#define SPI_DUMMY                           (SPI_REG_DUMMY |SPI_LOCAL_OFFSET)


#define SPI_HISR_RX_REQUEST             BIT(0)
#define SPI_HISR_ATIMEND                BIT(3)
#define SPI_HISR_BCNERLY_INT            BIT(4)
#define SPI_HISR_FW_STATUS              BIT(11)


#define REG_LEN_FORMAT(pcmd, x)             SET_BITS_TO_LE_4BYTE(pcmd, 0, 8, x)//(x<<(uint32)24)
#define REG_ADDR_FORMAT(pcmd,x)             SET_BITS_TO_LE_4BYTE(pcmd, 8, 16, x)//(x<<(uint32)16)
#define REG_DOMAIN_ID_FORMAT(pcmd,x)        SET_BITS_TO_LE_4BYTE(pcmd, 26, 5, x)//(x<<(uint32)0)
#define REG_RW_FORMAT(pcmd,x)               SET_BITS_TO_LE_4BYTE(pcmd, 31, 1, x)//(x<<(uint32)7)

#define FIFO_LEN_FORMAT(pcmd, x)            SET_BITS_TO_LE_4BYTE(pcmd, 0, 16, x)//(x<<(uint32)24)
#define FIFO_DOMAIN_ID_FORMAT(pcmd,x)   SET_BITS_TO_LE_4BYTE(pcmd, 26, 5, x)//(x<<(uint32)0)
#define FIFO_RW_FORMAT(pcmd,x)          SET_BITS_TO_LE_4BYTE(pcmd, 31, 1, x)//(x<<(uint32)7)

#define GET_STATUS_LOW_PAGE_NUM(status)     LE_BITS_TO_4BYTE(status, 16, 8)
#define GET_STATUS_MID_PAGE_NUM(status)     LE_BITS_TO_4BYTE(status, 8, 8)
#define GET_STATUS_HI_PAGE_NUM(status)      LE_BITS_TO_4BYTE(status, 0, 8)

#define GET_STATUS_HISR_LOW16BIT(status)        LE_BITS_TO_4BYTE(status, 16, 16)
#define GET_STATUS_RX_LENGTH(status)            LE_BITS_TO_4BYTE(status, 0, 16)

#define RXDESC_SIZE 8

#define GSPI_IO_MAX_RETRY_COUNT     10000

struct spi_more_data
{
    uint32 more_data;
    uint32 len;
};

void spi_get_tx_page_info(uint8 *response);
void spi_get_status_info(uint8 *status);

uint8 spi_read8(ADAPTER *Adapter, uint32 addr);

uint16 spi_read16(ADAPTER *Adapter, uint32 addr);

uint32 spi_read32(ADAPTER *Adapter, uint32 addr);

uint32 spi_write8_endian(ADAPTER *Adapter, uint32 addr, uint32 buf, uint32 big);

uint32 spi_write8(ADAPTER *Adapter, uint32 addr, uint32 buf);

uint32 spi_write16(ADAPTER *Adapter, uint32 addr,uint32 buf);

uint32 spi_write32(ADAPTER *Adapter, uint32 addr, uint32 buf);

#endif

