/*;
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
;*/

.equ     SYSTICK_CONTROL,   0xE000E010
.equ     VECT_TAB_OFFSET,   0xE000ED08
.equ     RESET_CONTROL,     0xE000ED0C
.equ     HANDLER_CONTROL,   0xE000ED24

.equ     IRQ_DISABLE0,      0xE000E180
/*
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
;*/
        .align 8
        .data
        .extern    Image_AP_MAIN_STACK_RW_Limit      
        .extern    Main
        .extern    ScatterLoader
        
        .global    __Vectors
__Vectors:
        .long     Main                                 // Reset Handler
        .long     DefaultHandler                       // NMI Handler
        .long     DefaultHandler                       // Hard Fault Handler
        .long     DefaultHandler                       // MPU Fault Handler
        .long     DefaultHandler                       // Bus Fault Handler
        .long     DefaultHandler                       // Usage Fault Handler
        .long     DefaultHandler                       // Reserved
        .long     DefaultHandler                       // Reserved
        .long     DefaultHandler                       // Reserved
        .long     DefaultHandler                       // Reserved
        .long     DefaultHandler                       // SVCall Handler
        .long     DefaultHandler                       // Debug Monitor Handler
        .long     DefaultHandler                       // Reserved
        .long     DefaultHandler                       // PendSV Handler
        .long     DefaultHandler                       // SysTick Handler
        
        //Peripheral Intterupt
        .long     DefaultHandler                       // int_dma           
        .long     DefaultHandler                       // int_dma_trans0    
        .long     DefaultHandler                       // int_dma_halftrans0
        .long     DefaultHandler                       // int_dma_trans0    
        .long     DefaultHandler                       // int_dma_halftrans0
        .long     DefaultHandler                       // imdct36           
        .long     DefaultHandler                       // synthesize        
        .long     DefaultHandler                       // usb               
        .long     DefaultHandler                       // i2c               
        .long     DefaultHandler                       // i2s               
        .long     DefaultHandler                       // gpio              
        .long     DefaultHandler                       // spi               
        .long     DefaultHandler                       // pwm               
        .long     DefaultHandler                       // timer             
        .long     DefaultHandler                       // sar-adc           
        .long     DefaultHandler                       // extern int0       
        .long     DefaultHandler                       // extern int1       
        .long     DefaultHandler                       // extern int2       
        .long     DefaultHandler                       // extern int3       

/*
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
;*/	
        .align 8
        .code 16
		
        .global  PowerOn_Reset
PowerOn_Reset:   
        LDR     R0, =0x00000000
        LDR     R1, =0x00000000
        LDR     R2, =0x00000000
        LDR     R3, =0x00000000
        LDR     R4, =0x00000000
        LDR     R5, =0x00000000
        LDR     R6, =0x00000000
        LDR     R7, =0x00000000
        //LDR     R8, =0x00000000
        MOV     R8,R0
        //LDR     R9, =0x00000000
        MOV     R9,R0 
        //LDR     R10, =0x00000000
        MOV     R10,R0
        //LDR     R11, =0x00000000
        MOV     R11,R0
        //LDR     R12, =0x00000000
        MOV     R12,R0
//
        LDR     R2, =0x00000000
        LDR     R1, =HANDLER_CONTROL
        STR     R2, [R1, #0x00]
		
//       ;Systick Int Disable
        LDR     R2, =0x00000000
        LDR     R1, =SYSTICK_CONTROL
        STR     R2, [R1, #0x00]
        
//        ;General Int Disable
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
/*;
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
;*/
        .align  8
        .code  16
DefaultHandler:
        B       DefaultHandler
/*
;
;*******************************************************************************
;
;                         End of Start.s
;
;*******************************************************************************
;*/
        .end
