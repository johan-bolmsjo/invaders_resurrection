$(call define-tool-srcs, gfxgen, \
	src/gfxgen/image.c \
	src/gfxgen/def.c \
	src/gfxgen/main.c \
	src/gfxgen/colours.c \
	src/gfxgen/clip.c \
	src/gfxgen/gfx_write.c \
	src/gfxgen/ff.c \
	src/gfxgen/scale.c \
	src/gfxgen/crop.c \
	src/gfxgen/ff_targa.c \
)

$(call define-tool-exe, gfxgen, gfxgen libgfx)
$(TARGET_TOOL_EXE_gfxgen): BUILD_LDFLAGS += -lz
