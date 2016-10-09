/******************************************************************/
/*   Copyright (C) 2008 ROCK-CHIPS FUZHOU . All Rights Reserved.  */
/*******************************************************************
File    :  uart.h
Desc    :  define UART register structure\registers macro bit definition\interface function.

Author  : lhh
Date    : 2008-10-29
Modified:
Revision:           1.00
$Log:   uart.h,v $
*********************************************************************/
//#ifdef DRIVERS_UART

#ifndef _DRIVER_UART_H_
#define _DRIVER_UART_H_

///uartWorkStatusFlag
//#define   UART0_TX_WORK                      ((uint32)(1))
//#define   UART0_RX_WORK                      ((uint32)(1)<<1)
//#define   UART1_TX_WORK                      ((uint32)(1)<<2)
//#define   UART1_RX_WORK                      ((uint32)(1)<<3)
//#define   UART_ERR_RX                        ((uint32)(1)<<4)

///UART_IER
#define   THRE_INT_ENABLE                    ((uint32)(1)<<7)
#define   THRE_INT_DISABLE                   ((uint32)(0))
#define   ENABLE_MODEM_STATUS_INT            ((uint32)(1)<<3)
#define   DISABLE_MODEM_STATUS_INT           ((uint32)(0))
#define   ENABLE_RECEIVER_LINE_STATUS_INT    ((uint32)(1)<<2)
#define   DISABLE_RECEIVER_LINE_STATUS_INT   ((uint32)(0))
#define   ENABLE_TRANSMIT_HOLDING_EM_INT     ((uint32)(1)<<1) ///Enable Transmit Holding Register Empty Interrupt.
#define   DISABLE_TRANSMIT_HOLDING_EM_INT    ((uint32)(0))
#define   ENABLE_RECEIVER_DATA_INT           ((uint32)(1))   ////Enable Received Data Available Interrupt.
#define   DISABLE_RECEIVER_DATA_INT          ((uint32)(0))

///UART_IIR
#define   IR_MODEM_STATUS                    ((uint32)(0))
#define   NO_INT_PENDING                     ((uint32)(1))
#define   THR_EMPTY                          ((uint32)(2))
#define   RECEIVER_DATA_AVAILABLE            ((uint32)(0x04))
#define   RECEIVER_LINE_AVAILABLE            ((uint32)(0x06))
#define   BUSY_DETECT                        ((uint32)(0x07))
#define   CHARACTER_TIMEOUT                  ((uint32)(0x0c))

///UART_LCR
#define  LCR_DLA_EN                          ((uint32)(1)<<7)
#define  BREAK_CONTROL_BIT                   ((uint32)(1)<<6)
#define  PARITY_DISABLED                     ((uint32)(0))
#define  PARITY_ENABLED                      ((uint32)(1)<<3)
#define  ONE_STOP_BIT                        ((uint32)(0))
#define  ONE_HALF_OR_TWO_BIT                 ((uint32)(1)<<2)
#define  LCR_WLS_5                           ((uint32)(0x00))
#define  LCR_WLS_6                           ((uint32)(0x01))
#define  LCR_WLS_7                           ((uint32)(0x02))
#define  LCR_WLS_8                           ((uint32)(0x03))
#define  UART_DATABIT_MASK                   ((uint32)(0x03))


///UART_MCR
#define  IRDA_SIR_DISABLED                   ((uint32)(0))
#define  IRDA_SIR_ENSABLED                   ((uint32)(1)<<6)
#define  AUTO_FLOW_DISABLED                  ((uint32)(0))
#define  AUTO_FLOW_ENSABLED                  ((uint32)(1)<<5)

///UART_LSR
#define  THRE_BIT_EN                         ((uint32)(1)<<5)

///UART_USR
#define  UART_RECEIVE_FIFO_EMPTY             ((uint32)(0))
#define  UART_RECEIVE_FIFO_NOT_EMPTY         ((uint32)(1)<<3)
#define  UART_TRANSMIT_FIFO_FULL             ((uint32)(0))
#define  UART_TRANSMIT_FIFO_NOT_FULL         ((uint32)(1)<<1)

///UART_SFE
#define  SHADOW_SHIFT                        ((uint32)(0))
#define  SHADOW_FIFI_ENABLED                 ((uint32)(1))
#define  SHADOW_FIFI_DISABLED                ((uint32)(0))

///UART_SRT
#define  RCVR_TRIGGER_SHIF                   ((uint32)(1))
#define  RCVR_TRIGGER_ONE                    ((uint32)(0) << RCVR_TRIGGER_SHIF)
#define  RCVR_TRIGGER_QUARTER_FIFO           ((uint32)(1) << RCVR_TRIGGER_SHIF)
#define  RCVR_TRIGGER_HALF_FIFO              ((uint32)(2) << RCVR_TRIGGER_SHIF)
#define  RCVR_TRIGGER_TWO_LESS_FIFO          ((uint32)(3) << RCVR_TRIGGER_SHIF)

//UART_STET
#define  TX_TRIGGER_PARA_SHIFT               ((uint32)(3))
#define  TX_TRIGGER_EMPTY                    ((uint32)(0) << TX_TRIGGER_PARA_SHIFT)
#define  TX_TRIGGER_TWO_IN_FIFO              ((uint32)(1) << TX_TRIGGER_PARA_SHIFT)
#define  TX_TRIGGER_ONE_FOUR_FIFO            ((uint32)(2) << TX_TRIGGER_PARA_SHIFT)
#define  TX_TRIGGER_HALF_FIFO                ((uint32)(3) << TX_TRIGGER_PARA_SHIFT)


///UART_SRR
#define  UART_RESET                          ((uint32)(1))
#define  RCVR_FIFO_REST                      ((uint32)(1)<<1)
#define  XMIT_FIFO_RESET                     ((uint32)(1)<<2)

//UART_FCR
#define TX_TRIGGER_TWO                       ((uint32)(0x1)<<4) // 2 characters in the TX FIFO
#define RX_TRIGGER_HALF_FIFO                 ((uint32)(0x1)<<7) // 1/2 in the rx FIFO
#define TX_LEN_PER_INT                       ((uint32)(64))//(16)
#define RX_LEN_PER_INT                       ((uint32)(8))

//UART Registers
typedef volatile struct tagUART_STRUCT
{
    uint32 UART_RBR;
    uint32 UART_DLH;
    uint32 UART_IIR;
    uint32 UART_LCR;
    uint32 UART_MCR;
    uint32 UART_LSR;
    uint32 UART_MSR;
    uint32 UART_SCR;
    uint32 RESERVED1[(0x30-0x20)/4];
    uint32 UART_SRBR[(0x70-0x30)/4];
    uint32 UART_FAR;
    uint32 UART_TFR;
    uint32 UART_RFW;
    uint32 UART_USR;
    uint32 UART_TFL;
    uint32 UART_RFL;
    uint32 UART_SRR;
    uint32 UART_SRTS;
    uint32 UART_SBCR;
    uint32 UART_SDMAM;
    uint32 UART_SFE;
    uint32 UART_SRT;
    uint32 UART_STET;
    uint32 UART_HTX;
    uint32 UART_DMASA;
    uint32 RESERVED2[(0xf4-0xac)/4];
    uint32 UART_CPR;
    uint32 UART_UCV;
    uint32 UART_CTR;
} UART_REG, *pUART_REG;



#define UART_THR UART_RBR
#define UART_DLL UART_RBR
#define UART_IER UART_DLH
#define UART_FCR UART_IIR
#define UART_STHR[(0x6c-0x30)/4]	UART_SRBR[(0x6c-0x30)/4]

#define MODE_X_DIV            16

#endif
//#endif

