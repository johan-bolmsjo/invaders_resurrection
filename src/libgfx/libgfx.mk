libgfx_srcs := \
	src/libgfx/gfx_decode.c \
	src/libgfx/gfx_object.c

$(call define-srcs, libgfx, $(libgfx_srcs))
$(call define-build-srcs, libgfx, $(libgfx_srcs))
