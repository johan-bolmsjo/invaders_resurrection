#pragma once

#include <inttypes.h>
#include <arpa/inet.h>

static inline void
htons_n(uint16_t* v, size_t n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (size_t i = 0; i < n; i++)
        v[i] = htons(v[i]);
#endif
}

static inline void
ntohs_n(uint16_t* v, size_t n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (size_t i = 0; i < n; i++)
        v[i] = ntohs(v[i]);
#endif
}

static inline void
htonl_n(uint32_t* v, size_t n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (size_t i = 0; i < n; i++)
        v[i] = htonl(v[i]);
#endif
}

static inline void
ntohl_n(uint32_t* v, size_t n)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    for (size_t i = 0; i < n; i++)
        v[i] = ntohl(v[i]);
#endif
}
