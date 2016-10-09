#ifndef __SPI_RX_H__
#define __SPI_RX_H__

#include "spi_io.h"

#define NR_RECVBUFF (4)
#define NR_PREALLOC_RECV_SKB (8)

#define MAX_VALID_RX_LENGTH         (1550 + 8)
#define MAX_VALID_RX_LENGTH_ALIGNED (1552 + 8)
#define MIN_VALID_RX_LENGTH         32

int32 spi_init_recv_struct(struct recv_struct *precv_priv, ADAPTER *Adapter);
void spi_free_recv_struct (struct recv_struct *precv_priv);
void spi_free_recv_pkt(ADAPTER *Adapter, struct recv_frame *precvframe);
void spi_rx_tasklet(ADAPTER *Adapter);
int32 spi_recvbuf_to_recvframe(ADAPTER *Adapter, _pkt *skb, struct rx_pkt_attrib *tmp_attrib);

int32 spi_read_rx_fifo(uint8 *buf, uint32 len, struct spi_more_data *pmore_data);

#endif
