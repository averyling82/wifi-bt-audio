#ifndef __HAL_INTERRUPT_H__
#define __HAL_INTERRUPT_H__

void
hal_init_interrupt(
    IN  PADAPTER Adapter
);

void
hal_enable_interrupt(
    IN PADAPTER         Adapter
);

void
hal_disable_interrupt (
    IN PADAPTER         Adapter
);

void
hal_update_interrupt_mask(
    PADAPTER Adapter,
    uint32 add_msr,
    uint32 rm_msr
);

boolean
hal_recognize_interrupt(
    IN  PADAPTER            Adapter,
    OUT uint32* interrupt_type
);

#endif
