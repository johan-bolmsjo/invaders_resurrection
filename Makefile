OUTDIR := _out
OUTDIR := $(abspath $(OUTDIR))
include mk/prologue.mk

###############################################################################
# Check for presence of SDL library.
# TODO: This needs to be adjusted to support cross compilation.
TARGET_CHECK_sdl.mk := $(OUTDIR_CHECK)/sdl.mk
$(TARGET_CHECK_sdl.mk):
	$(Q)$(RM) $@.tmp
	$(Q)echo CFLAGS_SDL := $(shell pkg-config sdl --cflags) >> $@.tmp
	$(Q)echo LDFLAGS_SDL := $(shell pkg-config sdl --libs) >> $@.tmp
	$(Q)pkg-config sdl --exists
	$(Q)mv $@.tmp $@

include $(TARGET_CHECK_sdl.mk)

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
include src/libsynth/libsynth.mk
include src/invaders/invaders.mk

include mk/epilogue.mk
