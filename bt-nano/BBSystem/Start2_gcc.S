
.equ    SYSTICK_CONTROL, 0xE000E010
.equ    VECT_TAB_OFFSET, 0xE000ED08
.equ    RESET_CONTROL,0xE000ED0C
.equ    HANDLER_CONTROL,0xE000ED24

.equ    IRQ_DISABLE0,0xE000E180

        //--------------------------------------------------------------------
        .align 2
        .thumb
        .section BB_START_VECT
        .extern  Image$$BB_MAIN_STACK$$ZI$$Limit
        .extern  Main2

        .global  __Vectors2
        .type    __Vectors2, %function
__Vectors2:
        .long     Image$$BB_MAIN_STACK$$ZI$$Limit                             //; Top of Stack
        .long     PowerOn_Reset2                          //			; Reset Handler
        .long     DefaultHandler2                         // ; NMI Handler
        .long     DefaultHandler2                         // ; Hard Fault Handler
        .long     DefaultHandler2                         // ; MPU Fault Handler
        .long     DefaultHandler2                         // ; Bus Fault Handler
        .long     DefaultHandler2                         // ; Usage Fault Handler
        .long     DefaultHandler2                         // ; Reserved
        .long     DefaultHandler2                         // ; Reserved
        .long     DefaultHandler2                         // ; Reserved
        .long     DefaultHandler2                         // ; Reserved
        .long     DefaultHandler2                         // ; SVCall Handler
        .long     DefaultHandler2                         // ; Debug Monitor Handler
        .long     DefaultHandler2                         // ; Reserved
        .long     DefaultHandler2                         // ; PendSV Handler
        .long     DefaultHandler2                         // ; SysTick Handler

        //;Peripheral Intterupt
        .long     DefaultHandler2                          //; int_dma
        .long     DefaultHandler2                          //; int_dma_trans0
        .long     DefaultHandler2                          //; int_dma_halftrans0
        .long     DefaultHandler2                          //; int_dma_trans0
        .long     DefaultHandler2                          //; int_dma_halftrans0
        .long     DefaultHandler2                          //; imdct36
        .long     DefaultHandler2                          //; synthesize
        .long     DefaultHandler2                          //; usb
        .long     DefaultHandler2                          //; i2c
        .long     DefaultHandler2                          //; i2s
        .long     DefaultHandler2                          //; gpio
        .long     DefaultHandler2                          //; spi
        .long     DefaultHandler2                          //; pwm
        .long     DefaultHandler2                          //; timer
        .long     DefaultHandler2                          //; sar-adc
        .long     DefaultHandler2                          //; extern int0
        .long     DefaultHandler2                          //; extern int1
        .long     DefaultHandler2                          //; extern int2
        .long     DefaultHandler2                          //; extern int3




        //-----------------------------------------------
        .align 2
        .section BB_START_CODE
        .thumb      @.code 16
        .syntax unified
        .extern  exceptions_table2
        .global  PowerOn_Reset2
        .type    PowerOn_Reset2, %function
PowerOn_Reset2:
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
        //;
        LDR     R2, =0x00000000




        LDR     R1, =HANDLER_CONTROL
        STR     R2, [R1, #0x00]
 
        //;Systick Int Disable
        LDR     R2, =0x00000000
        LDR     R1, =SYSTICK_CONTROL
        STR     R2, [R1, #0x00]

        //;General Int Disable
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

        .align 2
        .section BB_START_CODE
        .thumb
        .type    DefaultHandler2, %function
DefaultHandler2:
        B       DefaultHandler2

        .end
