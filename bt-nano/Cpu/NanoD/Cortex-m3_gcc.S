//        PRESERVE8
//        AREA |DriverLib|, CODE, READONLY, ALIGN=2

        .global  __WFI
__WFI:
        WFI
        BX      R14

        .global  __WFE
__WFE:
        WFE
        BX      R14

        .global  __SEV
__SEV:
        SEV
        BX      R14

        .global  __ISB
__ISB:
        ISB
        BX      R14

        .global  __DSB
__DSB:
        DSB
        BX      R14

        .global  __DMB
__DMB:
        DSB
        BX      R14

        .global  __SVC
__SVC:
        SVC     0x01
        BX      R14

        .global  __MRS_CONTROL
__MRS_CONTROL:
        MRS     R0, CONTROL
        BX      R14

        .global  __MSR_CONTROL
__MSR_CONTROL:
        MSR     CONTROL, R0
        ISB
        BX      R14

        .global  __MRS_PSP
__MRS_PSP:
        MRS     R0, PSP
        BX      R14

        .global  __MSR_PSP
__MSR_PSP: 
        MSR     PSP, R0
        BX      R14

        .global  __MRS_MSP
__MRS_MSP:
        MRS     R0, MSP
        BX      R14

        .global  __MSR_MSP
__MSR_MSP:
        MSR     MSP, R0
        BX      R14

        .global  __SETPRIMASK
__SETPRIMASK:
        CPSIE   i
        BX      R14

        .global  __RESETPRIMASK
__RESETPRIMASK:
        CPSID   i
        BX      R14

        .global  __SETFAULTMASK
__SETFAULTMASK:
        CPSID   f
        BX      R14

        .global  __RESETFAULTMASK
__RESETFAULTMASK:
        CPSIE   f
        BX      R14

        .global  __BASEPRICONFIG
__BASEPRICONFIG:
        MSR     BASEPRI, R0
        BX      R14

        .global  __GetBASEPRI
__GetBASEPRI:
        MRS     R0, BASEPRI_MAX
        BX      R14

        .global  __REV_HalfWord 
__REV_HalfWord:
        REV16   R0, R0
        BX      R14

        .global  __REV_Word
__REV_Word:
        REV     R0, R0
        BX      R14
    
        .end
        

