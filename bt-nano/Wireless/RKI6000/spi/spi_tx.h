#ifndef __SPI_TX_H_
#define __SPI_TX_H_

void spi_pkt_complete(ADAPTER *Adapter, _pkt *pkt);
void spi_xmit_complete(ADAPTER *Adapter, struct xmit_frame *pxframe);
void spi_send_xframe(ADAPTER *Adapter, struct xmit_frame *pxmitframe);

int32 spi_write_tx_fifo(uint8 *buf, uint32 len, uint32 fifo);

#endif
