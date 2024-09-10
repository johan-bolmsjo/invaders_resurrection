$(call define-build-srcs, btoc, src/tools/btoc.c)
$(call define-build-exe, btoc, btoc)

$(call define-build-srcs, compress, src/tools/compress.c)
$(call define-build-exe, compress, compress)
$(TARGET_BUILD_EXE_compress): BUILD_LDFLAGS += -lz
