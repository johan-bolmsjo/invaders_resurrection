#pragma once

#include <stdbool.h>

#include "libmedia/libmedia.h"

/// Create screenshot and save it in in Targa (TGA) format to path.
/// Returns true on success.
bool
screenshot_create(const struct MLGraphicsBuffer* dst, const char* path);
