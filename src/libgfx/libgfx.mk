libgfx_srcs := \
	src/libgfx/gfx_common.c \
	src/libgfx/gfx_read.c \
	src/libgfx/msb.c

$(call define-host-srcs, libgfx, $(libgfx_srcs))
$(call define-tool-srcs, libgfx, $(libgfx_srcs))
