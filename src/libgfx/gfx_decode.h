#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

// Decode uncompressed graphics data of the gfxgen format.
// Graphics objects are added to a global list 'object_list' and can be
// discovered using for example 'gfx_object_find'.
bool gfx_decode(const uint8_t* src, size_t len);
