;
;*******************************************************************************
;                   Copyright (c) 2008,ZhengYongzhi
;                         All rights reserved.
;
; File Name£º   Start.s
; 
; Description://initial RW,clear ZI(BSS) to zero,then hand over the control to system code. 
;
; History:      <author>          <time>        <version>       
;             ZhengYongzhi      2008-7-22          1.0
;    desc:       ORG
;   --------------------------------------------------------------------
;
;*******************************************************************************
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
        AREA    BB_START_VECT, DATA, READONLY
        
        IMPORT  |Image$$BB_MAIN_STACK$$RW$$Limit|
        IMPORT Main2
        
        EXPORT  __Vectors2
__Vectors2
        DCD     |Image$$BB_MAIN_STACK$$RW$$Limit|       ; Top of Stack
        DCD     PowerOn_Reset2                          			; Reset Handler
        DCD     DefaultHandler2                          ; NMI Handler
        DCD     DefaultHandler2                          ; Hard Fault Handler
        DCD     DefaultHandler2                          ; MPU Fault Handler
        DCD     DefaultHandler2                          ; Bus Fault Handler
        DCD     DefaultHandler2                          ; Usage Fault Handler
        DCD     DefaultHandler2                          ; Reserved
        DCD     DefaultHandler2                          ; Reserved
        DCD     DefaultHandler2                          ; Reserved
        DCD     DefaultHandler2                          ; Reserved
        DCD     DefaultHandler2                          ; SVCall Handler
        DCD     DefaultHandler2                          ; Debug Monitor Handler
        DCD     DefaultHandler2                          ; Reserved
        DCD     DefaultHandler2                          ; PendSV Handler
        DCD     DefaultHandler2                          ; SysTick Handler
        
        ;Peripheral Intterupt
        DCD     DefaultHandler2                          ; int_dma           
        DCD     DefaultHandler2                          ; int_dma_trans0    
        DCD     DefaultHandler2                          ; int_dma_halftrans0
        DCD     DefaultHandler2                          ; int_dma_trans0    
        DCD     DefaultHandler2                          ; int_dma_halftrans0
        DCD     DefaultHandler2                          ; imdct36           
        DCD     DefaultHandler2                          ; synthesize        
        DCD     DefaultHandler2                          ; usb               
        DCD     DefaultHandler2                          ; i2c               
        DCD     DefaultHandler2                          ; i2s               
        DCD     DefaultHandler2                          ; gpio              
        DCD     DefaultHandler2                          ; spi               
        DCD     DefaultHandler2                          ; pwm               
        DCD     DefaultHandler2                          ; timer             
        DCD     DefaultHandler2                          ; sar-adc           
        DCD     DefaultHandler2                          ; extern int0       
        DCD     DefaultHandler2                          ; extern int1       
        DCD     DefaultHandler2                          ; extern int2       
        DCD     DefaultHandler2                          ; extern int3       


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
        AREA    BB_START_CODE, CODE, READONLY
        ;ENTRY 
		
		IMPORT  exceptions_table2
		
        EXPORT  PowerOn_Reset2
PowerOn_Reset2
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
        
        LDR     R0, =exceptions_table2
        LDR     R1, =VECT_TAB_OFFSET
        STR     R0, [R1, #0x00]
        
        LDR     R0, =0x05FA0002
        LDR     R1, =RESET_CONTROL
        STR     R0, [R1, #0x00]
        
        LDR     R2, =exceptions_table2
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
        AREA    BB_START_CODE, CODE, READONLY
DefaultHandler2
        B       DefaultHandler2

;
;*******************************************************************************
;
;                         End of Start.s
;
;*******************************************************************************
;/
        END
