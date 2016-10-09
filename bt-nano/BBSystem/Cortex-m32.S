;
;*******************************************************************************
;                   Copyright (c) 2008,ZhengYongzhi
;                         All rights reserved.
;
; File Name£º   Os.S
; 
; Description:  
;
; History:      <author>          <time>        <version>       
;             ZhengYongzhi      2008-7-22          1.0
;    desc:       ORG
;   --------------------------------------------------------------------
;
;*******************************************************************************
;

;
;--------------------------------------------------------------------------------
;
 
;
;--------------------------------------------------------------------------------
;

        PRESERVE8
        AREA |DriverLib|, CODE, READONLY, ALIGN=2
;
;--------------------------------------------------------------------------------
; Function name : void __WFI(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the WFI instruction.
;                  
; Input         : 
; Return        :  
;
; History:     <author>         <time>         <version>       
;             ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __WFI2
__WFI2
        WFI
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __WFE(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the WFE instruction.
;                 
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __WFE2
__WFE2
        WFE
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __SEV(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the SEV instruction.
;                 
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __SEV2
__SEV2
        SEV
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __ISB(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the ISB instruction.
;                
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __ISB2
__ISB2
        ISB
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __DSB(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the DSB instruction.
;                
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __DSB2
__DSB2
        DSB
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __DMB(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the DMB instruction.
;                
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __DMB2
__DMB2
        DSB
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __SVC(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the SVC instruction.
;                
; Input         :
; Return        :
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __SVC2
__SVC2
        SVC     0x01
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT32 __MRS_CONTROL(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MRS instruction.
;                
; Input         :
; Return        : CONTROL register value.
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MRS_CONTROL2
__MRS_CONTROL2
        MRS     R0, CONTROL
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __MSR_CONTROL(UINT32 Control)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MSR instruction.
;                
; Input         : CONTROL register new value.
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MSR_CONTROL2
__MSR_CONTROL2
        MSR     CONTROL, R0
        ISB
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT32 __MRS_PSP(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MRS instruction.
;                
; Input         :
; Return        : Process Stack value.
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MRS_PSP2
__MRS_PSP2
        MRS     R0, PSP
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __MSR_PSP(UINT32 ProcStack)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MSR instruction.
;                
; Input         : Process Stack new value.
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MSR_PSP2
__MSR_PSP2 
        MSR     PSP, R0
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT32 __MRS_MSP(void)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MRS instruction.
;                
; Input         : 
; Return        : Main Stack value.
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MRS_MSP2
__MRS_MSP2
        MRS     R0, MSP
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __MSR_MSP(UINT32 MainStack)
; Author        : ZHengYongzhi
; Description   : Assembler function for the MSR instruction.
;                
; Input         : Main Stack new value.  
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __MSR_MSP2
__MSR_MSP2
        MSR     MSP, R0
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __SETPRIMASK(void)
; Author        : ZHengYongzhi
; Description   : Assembler function to set the PRIMASK.
;                
; Input         : 
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __SETPRIMASK2
__SETPRIMASK2
        CPSIE   i
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __RESETPRIMASK(void)
; Author        : ZHengYongzhi
; Description   : Assembler function to reset the PRIMASK.
;                
; Input         : 
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __RESETPRIMASK2
__RESETPRIMASK2
        CPSID   i
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __SETFAULTMASK(void)
; Author        : ZHengYongzhi
; Description   : Assembler function to set the FAULTMASK.
;                
; Input         : 
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __SETFAULTMASK2
__SETFAULTMASK2
        CPSID   f
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __RESETFAULTMASK(void)
; Author        : ZHengYongzhi
; Description   : Assembler function to reset the FAULTMASK.
;                
; Input         : 
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __RESETFAULTMASK2
__RESETFAULTMASK2
        CPSIE   f
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : void __BASEPRICONFIG(UINT32 NewPriority)
; Author        : ZHengYongzhi
; Description   : Assembler function to set the Base Priority.
;                
; Input         : Base Priority new value.
; Return        : 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __BASEPRICONFIG2
__BASEPRICONFIG2
        MSR     BASEPRI, R0
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT32 __GetBASEPRI(void)
; Author        : ZHengYongzhi
; Description   : Assembler function to get the Base Priority value.
;                
; Input         : 
; Return        : Base Priority value 
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __GetBASEPRI2
__GetBASEPRI2
        MRS     R0, BASEPRI_MAX
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT16 __REV_HalfWord(UINT16 data)
; Author        : ZHengYongzhi
; Description   : Reverses the byte order in HalfWord(16-bit) input variable.
;                
; Input         : specifies the input variable
; Return        : holds tve variable value after byte reversing.
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __REV_HalfWord2 
__REV_HalfWord2
        REV16   R0, R0
        BX      R14

;
;--------------------------------------------------------------------------------
; Function name : UINT32 __REV_Word(UINT32 data)
; Author        : ZHengYongzhi
; Description   : Reverses the byte order in Word(32-bit) input variable.
;                
; Input         : specifies the input variable
; Return        : holds tve variable value after byte reversing.
;                
; History:     <author>         <time>         <version>       
;            ZhengYongzhi     2008/07/21         Ver1.0
; desc:         ORG
;--------------------------------------------------------------------------------
;
        EXPORT  __REV_Word2
__REV_Word2
        REV     R0, R0
        BX      R14
    
;*******************************************************************************


        END
        

