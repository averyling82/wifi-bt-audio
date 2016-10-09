#ifndef __WLAN_ENDIAN_FREE_H
#define __WLAN_ENDIAN_FREE_H


#ifdef CONFIG_LITTLE_ENDIAN
// Convert data
#define EF1Byte(_val)               ((uint8)(_val))
#define EF2Byte(_val)               ((uint16)(_val))
#define EF4Byte(_val)               ((uint32)(_val))

#else
// Convert data
#define EF1Byte(_val)               ((uint8)(_val))
#define EF2Byte(_val)               (   ((((uint16)(_val))&0x00ff)<<8)  |           \
                                    ((((uint16)(_val))&0xff00)>>8)  )
#define EF4Byte(_val)               (   ((((uint32)(_val))&0x000000ff)<<24) |       \
                                    ((((uint32)(_val))&0x0000ff00)<<8)  |       \
                                    ((((uint32)(_val))&0x00ff0000)>>8)  |       \
                                    ((((uint32)(_val))&0xff000000)>>24) )

#endif
//
// Read LE format data from memory
//
#define ReadEF1Byte(_ptr)       EF1Byte(*((uint8 *)(_ptr)))
#define ReadEF2Byte(_ptr)       EF2Byte(*((uint16 *)(_ptr)))
#define ReadEF4Byte(_ptr)       EF4Byte(*((uint32 *)(_ptr)))

//
// Write LE data to memory
//
#define WriteEF1Byte(_ptr, _val)    (*((uint8 *)(_ptr)))=EF1Byte(_val)
#define WriteEF2Byte(_ptr, _val)    (*((uint16 *)(_ptr)))=EF2Byte(_val)
#define WriteEF4Byte(_ptr, _val)    (*((uint32 *)(_ptr)))=EF4Byte(_val)

//
//  Example:
//      BIT_LEN_MASK_32(0) => 0x00000000
//      BIT_LEN_MASK_32(1) => 0x00000001
//      BIT_LEN_MASK_32(2) => 0x00000003
//      BIT_LEN_MASK_32(32) => 0xFFFFFFFF
//
#define BIT_LEN_MASK_32(__BitLen) \
    (0xFFFFFFFF >> (32 - (__BitLen)))
//
//  Example:
//      BIT_OFFSET_LEN_MASK_32(0, 2) => 0x00000003
//      BIT_OFFSET_LEN_MASK_32(16, 2) => 0x00030000
//
#define BIT_OFFSET_LEN_MASK_32(__BitOffset, __BitLen) \
    (BIT_LEN_MASK_32(__BitLen) << (__BitOffset))

//
//  Description:
//      Return 4-byte value in host byte ordering from
//      4-byte pointer in litten-endian system.
//
#define LE_P4BYTE_TO_HOST_4BYTE(__pStart) \
    (EF4Byte(*((uint32 *)(__pStart))))

//
//  Description:
//      Translate subfield (continuous bits in little-endian) of 4-byte value in litten byte to
//      4-byte value in host byte ordering.
//
#define LE_BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        ( LE_P4BYTE_TO_HOST_4BYTE(__pStart) >> (__BitOffset) ) \
        & \
        BIT_LEN_MASK_32(__BitLen) \
    )

//pclint
#define LE_BITS_TO_4BYTE_32BIT(__pStart, __BitOffset, __BitLen) \
    ( \
        ( LE_P4BYTE_TO_HOST_4BYTE(__pStart)  ) \
    )
//
//  Description:
//      Mask subfield (continuous bits in little-endian) of 4-byte value in litten byte oredering
//      and return the result in 4-byte value in host byte ordering.
//
#define LE_BITS_CLEARED_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        LE_P4BYTE_TO_HOST_4BYTE(__pStart) \
        & \
        ( ~ BIT_OFFSET_LEN_MASK_32(__BitOffset, __BitLen) ) \
    )

//
//  Description:
//      Set subfield of little-endian 4-byte value to specified value.
//

#define BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        ( (*((uint32 *)(__pStart))) >> (__BitOffset) ) \
        & \
        BIT_LEN_MASK_32(__BitLen) \
    )

#define SET_BITS_TO_4BYTE(__pStart, __BitOffset, __BitLen, __Value)  \
    *((uint32 *)(__pStart)) = \
        ( \
        (*((uint32 *)(__pStart))) \
        & \
        ( ~ BIT_OFFSET_LEN_MASK_32(__BitOffset, __BitLen) ) \
        | \
        ( (((uint32)__Value) & BIT_LEN_MASK_32(__BitLen)) << (__BitOffset) ) \
        );

#define SET_BITS_TO_LE_4BYTE(__pStart, __BitOffset, __BitLen, __Value)  \
    *((uint32 *)(__pStart)) = \
        EF4Byte( \
            LE_BITS_CLEARED_TO_4BYTE(__pStart, __BitOffset, __BitLen) \
            | \
            ( (((uint32)__Value) & BIT_LEN_MASK_32(__BitLen)) << (__BitOffset) ) \
        );

#define BIT_LEN_MASK_16(__BitLen) \
        (0xFFFF >> (16 - (__BitLen)))

#define BIT_OFFSET_LEN_MASK_16(__BitOffset, __BitLen) \
    (BIT_LEN_MASK_16(__BitLen) << (__BitOffset))

#define LE_P2BYTE_TO_HOST_2BYTE(__pStart) \
    (EF2Byte(*((uint16 *)(__pStart))))

#define LE_BITS_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        ( LE_P2BYTE_TO_HOST_2BYTE(__pStart) >> (__BitOffset) ) \
        & \
        BIT_LEN_MASK_16(__BitLen) \
    )

#define LE_BITS_CLEARED_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        LE_P2BYTE_TO_HOST_2BYTE(__pStart) \
        & \
        ( ~ BIT_OFFSET_LEN_MASK_16(__BitOffset, __BitLen) ) \
    )

#define SET_BITS_TO_LE_2BYTE(__pStart, __BitOffset, __BitLen, __Value) \
    *((uint16 *)(__pStart)) = \
        EF2Byte( \
            LE_BITS_CLEARED_TO_2BYTE(__pStart, __BitOffset, __BitLen) \
            | \
            ( (((uint16)__Value) & BIT_LEN_MASK_16(__BitLen)) << (__BitOffset) ) \
        );

#define BIT_LEN_MASK_8(__BitLen) \
        (0xFF >> (8 - (__BitLen)))

#define BIT_OFFSET_LEN_MASK_8(__BitOffset, __BitLen) \
    (BIT_LEN_MASK_8(__BitLen) << (__BitOffset))

#define LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
    (EF1Byte(*((uint8 *)(__pStart))))

#define LE_BITS_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        ( LE_P1BYTE_TO_HOST_1BYTE(__pStart) >> (__BitOffset) ) \
        & \
        BIT_LEN_MASK_8(__BitLen) \
    )
//pclint
#define LE_BITS_TO_1BYTE_8BIT(__pStart, __BitOffset, __BitLen) \
    LE_P1BYTE_TO_HOST_1BYTE(__pStart)

#define LE_BITS_CLEARED_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
    ( \
        LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
        & \
        ( ~ BIT_OFFSET_LEN_MASK_8(__BitOffset, __BitLen) ) \
    )
//pclint
#define LE_BITS_CLEARED_TO_1BYTE_8BIT(__pStart, __BitOffset, __BitLen) \
    ( \
        LE_P1BYTE_TO_HOST_1BYTE(__pStart) \
    )

#define SET_BITS_TO_LE_1BYTE(__pStart, __BitOffset, __BitLen, __Value) \
{ \
    *((uint8 *)(__pStart)) = \
        EF1Byte( \
            LE_BITS_CLEARED_TO_1BYTE(__pStart, __BitOffset, __BitLen) \
            | \
            ( (((uint8)__Value) & BIT_LEN_MASK_8(__BitLen)) << (__BitOffset) ) \
        ); \
}

//pclint
#define SET_BITS_TO_LE_1BYTE_8BIT(__pStart, __BitOffset, __BitLen, __Value) \
{ \
    *((uint8 *)(__pStart)) = \
        EF1Byte( \
            LE_BITS_CLEARED_TO_1BYTE_8BIT(__pStart, __BitOffset, __BitLen) \
            | \
            ((uint8)__Value) \
        ); \
}

// Get the N-bytes aligment offset from the current length
#define N_BYTE_ALIGMENT(__Value, __Aligment) ((__Aligment == 1) ? (__Value) : (((__Value + __Aligment - 1) / __Aligment) * __Aligment))

#endif

