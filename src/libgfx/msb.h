#pragma once

#include <inttypes.h>

int16_t  msb_short(uint16_t v);
int32_t  msb_long(uint32_t v);
uint16_t msb_ushort(uint16_t v);
uint32_t msb_ulong(uint32_t v);
void     msb_long_mem(uint32_t* m, int n);
void     msb_short_mem(uint16_t* m, int n);
