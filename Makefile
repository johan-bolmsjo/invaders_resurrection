OUTDIR := _out
OUTDIR := $(abspath $(OUTDIR))

VARIANT := debug

DEBUG_INFO := -g -fno-omit-frame-pointer

CFLAGS_debug       := -O0 $(DEBUG_INFO)
CFLAGS_debug-asan  := -O0 $(DEBUG_INFO) -fsanitize=address
LDFLAGS_debug-asan := -fsanitize=address
CFLAGS_release     := -O2 $(DEBUG_INFO)

CFLAGS  := $(CFLAGS_$(VARIANT))
LDFLAGS := $(LDFLAGS_$(VARIANT))

ifeq ($(CFLAGS),)
$(error Unsupported variant $(VARIANT))
endif

include mk/prologue.mk

###############################################################################
# Check for presence of SDL2 library.
# TODO(jb): This needs to be adjusted to support cross compilation.
TARGET_CHECK_sdl2.mk := $(OUTDIR_CHECK)/sdl2.mk
$(TARGET_CHECK_sdl2.mk):
	$(Q)$(RM) $@.tmp
	$(Q)echo CFLAGS_SDL := $(shell pkg-config sdl2 --cflags) >> $@.tmp
	$(Q)echo LDFLAGS_SDL := $(shell pkg-config sdl2 --libs) >> $@.tmp
	$(Q)pkg-config sdl --exists
	$(Q)mv $@.tmp $@

include $(TARGET_CHECK_sdl2.mk)

###############################################################################

CFLAGS   += -std=gnu11 -Wall -Wextra -pedantic -pthread
LDFLAGS  += -pthread

HOST_CFLAGS += $(CFLAGS_SDL)
HOST_CPPFLAGS += -iquote $(CURDIR)/src -iquote $(OUTDIR_SRCS)
HOST_LDFLAGS += -L$(OUTDIR_HOST_LIB) $(LDFLAGS_SDL) -lz

BUILD_CPPFLAGS += -iquote $(CURDIR)/src

include src/libgfx/libgfx.mk
include src/gfxgen/gfxgen.mk
include src/tools/tools.mk
include data/data.mk
include src/libmedia/libmedia.mk
include src/libutil/libutil.mk
include src/libsynth/libsynth.mk
include src/invaders/invaders.mk

include mk/epilogue.mk
