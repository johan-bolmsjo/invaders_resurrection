libgfx_srcs := \
	src/libgfx/gfx_decode.c \
	src/libgfx/gfx_object.c

$(call define-host-srcs, libgfx, $(libgfx_srcs))
$(call define-tool-srcs, libgfx, $(libgfx_srcs))
