/* Convert to and from MSB first.
 */

#include "msb.h"

inline int16_t
msb_short(uint16_t v)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (v >> 8) | (v << 8);
#else
    return v;
#endif
}

inline int32_t
msb_long(uint32_t v)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (v << 24) | (v << 8 & 0xFF << 16) | (v >> 8 & 0xFF << 8) | (v >> 24);
#else
    return v;
#endif
}

inline uint16_t
msb_ushort(uint16_t v)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (v >> 8) | (v << 8);
#else
    return v;
#endif
}

inline uint32_t
msb_ulong(uint32_t v)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    return (v << 24) | (v << 8 & 0xFF << 16) | (v >> 8 & 0xFF << 8) | (v >> 24);
#else
    return v;
#endif
}

inline void
msb_short_mem(uint16_t* m, int n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (; n > 0; n--)
        *m++ = msb_ushort(*m);
#endif
}

inline void
msb_long_mem(uint32_t* m, int n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (; n > 0; n--)
        *m++ = msb_ulong(*m);
#endif
}
