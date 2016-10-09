;
;********************************************************************************************
;
;  Copyright (c): 2014 - 2014 + 5, Fuzhou Rockchip Electronics Co., Ltd
;                         All rights reserved.
;
; FileName: ..\Driver\EMMC\EmmcDevice.c
; Owner: Aaron.sun
; Date: 2014.2.24
; Time: 10:35:32
; Desc:
; History:
;   <author>    <date>       <time>     <version>     <Desc>
; Aaron.sun     2014.2.24     10:35:32   1.0
;********************************************************************************************
;

SYSTICK_CONTROL     EQU     0xE000E010
VECT_TAB_OFFSET     EQU     0xE000ED08
RESET_CONTROL       EQU     0xE000ED0C
HANDLER_CONTROL     EQU     0xE000ED24

IRQ_DISABLE0        EQU     0xE000E180

;
;--------------------------------------------------------------------------------
; Function name :
; Author        : ZHengYongzhi
; Description   : The vector table.
;
; History:     <author>         <time>         <version>
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        PRESERVE8
        AREA    AP_START_VECT, DATA, READONLY

        IMPORT  |Image$$AP_MAIN_STACK$$RW$$Limit|
        IMPORT  Main
        IMPORT  ScatterLoader

        EXPORT  __Vectors
__Vectors
        DCD     |Image$$AP_MAIN_STACK$$RW$$Limit|       ; Top of Stack
        DCD     Main                                       ; Reset Handler
        DCD     DefaultHandler                          ; NMI Handler
        DCD     DefaultHandler                          ; Hard Fault Handler
        DCD     DefaultHandler                          ; MPU Fault Handler
        DCD     DefaultHandler                          ; Bus Fault Handler
        DCD     DefaultHandler                          ; Usage Fault Handler
        DCD     DefaultHandler                          ; Reserved
        DCD     DefaultHandler                          ; Reserved
        DCD     DefaultHandler                          ; Reserved
        DCD     DefaultHandler                          ; Reserved
        DCD     DefaultHandler                          ; SVCall Handler
        DCD     DefaultHandler                          ; Debug Monitor Handler
        DCD     DefaultHandler                          ; Reserved
        DCD     DefaultHandler                          ; PendSV Handler
        DCD     DefaultHandler                          ; SysTick Handler

        ;Peripheral Intterupt
        DCD     DefaultHandler                          ; int_dma
        DCD     DefaultHandler                          ; int_dma_trans0
        DCD     DefaultHandler                          ; int_dma_halftrans0
        DCD     DefaultHandler                          ; int_dma_trans0
        DCD     DefaultHandler                          ; int_dma_halftrans0
        DCD     DefaultHandler                          ; imdct36
        DCD     DefaultHandler                          ; synthesize
        DCD     DefaultHandler                          ; usb
        DCD     DefaultHandler                          ; i2c
        DCD     DefaultHandler                          ; i2s
        DCD     DefaultHandler                          ; gpio
        DCD     DefaultHandler                          ; spi
        DCD     DefaultHandler                          ; pwm
        DCD     DefaultHandler                          ; timer
        DCD     DefaultHandler                          ; sar-adc
        DCD     DefaultHandler                          ; extern int0
        DCD     DefaultHandler                          ; extern int1
        DCD     DefaultHandler                          ; extern int2
        DCD     DefaultHandler                          ; extern int3


;
;--------------------------------------------------------------------------------
; Function name : Reset_Handler(void)
; Author        : ZHengYongzhi
; Description   : Reset Handler
;
; Input         :
; Return        :
;
; History:     <author>         <time>         <version>
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
       PRESERVE8
        AREA    AP_START_CODE, CODE, READONLY
        ;ENTRY

        EXPORT  PowerOn_Reset
PowerOn_Reset
        LDR     R0, =0x00000000
        LDR     R1, =0x00000000
        LDR     R2, =0x00000000
        LDR     R3, =0x00000000
        LDR     R4, =0x00000000
        LDR     R5, =0x00000000
        LDR     R6, =0x00000000
        LDR     R7, =0x00000000
        LDR     R8, =0x00000000
        LDR     R9, =0x00000000
        LDR     R10, =0x00000000
        LDR     R11, =0x00000000
        LDR     R12, =0x00000000
        ;
        LDR     R2, =0x00000000
        LDR     R1, =HANDLER_CONTROL
        STR     R2, [R1, #0x00]

        ;Systick Int Disable
        LDR     R2, =0x00000000
        LDR     R1, =SYSTICK_CONTROL
        STR     R2, [R1, #0x00]

        ;General Int Disable
        LDR     R2, =0xFFFFFFFF
        LDR     R1, =IRQ_DISABLE0
        STR     R2, [R1, #0x00]
        STR     R2, [R1, #0x04]
        STR     R2, [R1, #0x08]
        STR     R2, [R1, #0x0c]
        STR     R2, [R1, #0x10]
        STR     R2, [R1, #0x14]
        STR     R2, [R1, #0x18]
        STR     R2, [R1, #0x1c]
    LDR     R0, =0x01000100
        LDR     R1, =0x500100c4
        STR     R0, [R1]

        BL      ScatterLoader

        LDR     R0, =__Vectors
        LDR     R1, =VECT_TAB_OFFSET
        STR     R0, [R1, #0x00]

        LDR     R0, =0x05FA0002
        LDR     R1, =RESET_CONTROL
        STR     R0, [R1, #0x00]

        LDR     R2, =__Vectors
        LDR     R0, [R2, #0x00]
        LDR     R1, [R2, #0x04]
        MOV     SP, R0
        MOV     LR, R1
        BX      LR
;
;--------------------------------------------------------------------------------
; Function name : DefaultHandler
; Author        : ZHengYongzhi
; Description   : Reset Handler
;
; Input         :
; Return        :
;
; History:     <author>         <time>         <version>
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        PRESERVE8
        AREA    AP_START_CODE, CODE, READONLY
DefaultHandler
        B       DefaultHandler

;
;*******************************************************************************
;
;                         End of Start.s
;
;*******************************************************************************
;/
        END
