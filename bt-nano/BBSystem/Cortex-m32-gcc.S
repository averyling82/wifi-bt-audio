
       .align 2
       .section .text

    .global  __WFI2
__WFI2:
        WFI
        BX      R14

        .global  __WFE2
__WFE2:
        WFE
        BX      R14

        .global  __SEV2
__SEV2:
        SEV
        BX      R14

        .global  __ISB2
__ISB2:
        ISB
        BX      R14

        .global  __DSB2
__DSB2:
        DSB
        BX      R14

        .global  __DMB2
__DMB2:
        DSB
        BX      R14

        .global  __SVC2
__SVC2:
        SVC     0x01
        BX      R14

        .global  __MRS_CONTROL2
__MRS_CONTROL2:
        MRS     R0, CONTROL
        BX      R14

        .global  __MSR_CONTROL2
__MSR_CONTROL2:
        MSR     CONTROL, R0
        ISB
        BX      R14

        .global  __MRS_PSP2
__MRS_PSP2:
        MRS     R0, PSP
        BX      R14

        .global  __MSR_PSP2
__MSR_PSP2:
        MSR     PSP, R0
        BX      R14

        .global  __MRS_MSP2
__MRS_MSP2:
        MRS     R0, MSP
        BX      R14

        .global  __MSR_MSP2
__MSR_MSP2:
        MSR     MSP, R0
        BX      R14

        .global  __SETPRIMASK2
__SETPRIMASK2:
        CPSIE   i
        BX      R14

        .global  __RESETPRIMASK2
__RESETPRIMASK2:
        CPSID   i
        BX      R14

        .global  __SETFAULTMASK2
__SETFAULTMASK2:
        CPSID   f
        BX      R14

        .global  __RESETFAULTMASK2
__RESETFAULTMASK2:
        CPSIE   f
        BX      R14

        .global  __BASEPRICONFIG2
__BASEPRICONFIG2:
        MSR     BASEPRI, R0
        BX      R14

        .global  __GetBASEPRI2
__GetBASEPRI2:
        MRS     R0, BASEPRI_MAX
        BX      R14

        .global  __REV_HalfWord2
__REV_HalfWord2:
        REV16   R0, R0
        BX      R14

        .global  __REV_Word2
__REV_Word2:
        REV     R0, R0
        BX      R14

        .end
