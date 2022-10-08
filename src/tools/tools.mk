$(call define-tool-srcs, btoc, src/tools/btoc.c)
$(call define-tool-exe, btoc, btoc)

$(call define-tool-srcs, compress, src/tools/compress.c)
$(call define-tool-exe, compress, compress)
$(TARGET_TOOL_EXE_compress): BUILD_LDFLAGS += -lz
