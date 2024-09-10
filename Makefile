OUTDIR := _out
OUTDIR := $(abspath $(OUTDIR))

VARIANT := debug

DEBUG_INFO := -g -fno-omit-frame-pointer

CFLAGS_debug       := -O0 $(DEBUG_INFO)
CFLAGS_debug-asan  := -O0 $(DEBUG_INFO) -fsanitize=address
LDFLAGS_debug-asan := -fsanitize=address
CFLAGS_release     := -O2 $(DEBUG_INFO)

CFLAGS  := $(CFLAGS_$(VARIANT))
LDFLAGS := $(LDFLAGS_$(VARIANT)) $(LDFLAGS)

ifeq ($(CFLAGS),)
$(error Unsupported variant $(VARIANT))
endif

include mk/prologue.mk

###############################################################################
# Check for presence of SDL2 library.
TARGET_CHECK_sdl2.mk := $(OUTDIR_CHECK)/sdl2.mk
$(TARGET_CHECK_sdl2.mk):
	$(Q)$(RM) $@.tmp
	$(Q)echo CFLAGS_sdl := $(shell pkg-config sdl2 --cflags) >> $@.tmp
	$(Q)echo LDFLAGS_sdl := $(shell pkg-config sdl2 --libs) >> $@.tmp
	$(Q)pkg-config sdl2 --exists
	$(Q)mv $@.tmp $@

include $(TARGET_CHECK_sdl2.mk)

###############################################################################
# Check for presence of zlib library.
TARGET_CHECK_zlib.mk := $(OUTDIR_CHECK)/zlib.mk
$(TARGET_CHECK_zlib.mk):
	$(Q)$(RM) $@.tmp
	$(Q)echo CFLAGS_zlib := $(shell pkg-config zlib --cflags) >> $@.tmp
	$(Q)echo LDFLAGS_zlib := $(shell pkg-config zlib --libs) >> $@.tmp
	$(Q)pkg-config zlib --exists
	$(Q)mv $@.tmp $@

include $(TARGET_CHECK_zlib.mk)

###############################################################################

ifneq ($(NO_THREADS),1)
CFLAGS_threads := -pthread
LDFLAGS_threads := -pthread
endif

CFLAGS   += -std=gnu11 -Wall -Wextra -pedantic $(CFLAGS_threads) $(CFLAGS_sdl) $(CFLAGS_zlib)
CPPFLAGS += -iquote $(CURDIR)/src -iquote $(OUTDIR_SRCS)
LDFLAGS  += $(LDFLAGS_threads) -L$(OUTDIR_LIB) $(LDFLAGS_sdl) $(LDFLAGS_zlib) -lGL

BUILD_CFLAGS   += -std=gnu11 -Wall -Wextra -pedantic $(CFLAGS_threads)
BUILD_CPPFLAGS += -iquote $(CURDIR)/src
BUILD_LDFLAGS  += $(LDFLAGS_threads)

include src/libgfx/libgfx.mk
include src/gfxgen/gfxgen.mk
include src/tools/tools.mk
include data/data.mk
include src/libmedia/libmedia.mk
include src/libutil/libutil.mk
include src/libsynth/libsynth.mk
include src/invaders/invaders.mk

include mk/epilogue.mk
