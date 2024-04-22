TARGET_SRCS_compressed_font_data.c := $(OUTDIR_SRCS)/compressed_font_data.c
$(TARGET_SRCS_compressed_font_data.c): $(TARGET_TOOL_EXE_btoc)
$(TARGET_SRCS_compressed_font_data.c): data/pearl_8x8.dat.deflate
	$(TARGET_TOOL_EXE_btoc) $< compressed_font > $@

TARGET_SRCS_compressed_gfx_data.c := $(OUTDIR_SRCS)/compressed_gfx_data.c
$(TARGET_SRCS_compressed_gfx_data.c): $(TARGET_TOOL_EXE_btoc)
$(TARGET_SRCS_compressed_gfx_data.c): data/invaders.gfx.deflate
	$(TARGET_TOOL_EXE_btoc) $< compressed_gfx > $@

TARGET_DATA_invaders.gfx.gz := $(OUTDIR_DATA)/invaders.gfx.gz
$(TARGET_DATA_invaders.gfx.gz): $(TARGET_TOOL_EXE_gfxgen)

# This data file really depends on a ton of input files listed in
# 'data/invaders.def'. Ignore that for now!
$(TARGET_DATA_invaders.gfx.gz): data/invaders.def
	cd $(dir $<) && $(TARGET_TOOL_EXE_gfxgen) -i $(notdir $<) -o $@

# Expose 'invaders.gfx.gz' as a command in the help menu.
PHONY_TARGETS += data/invaders.gfx.gz
.PHONY: data/invaders.gfx.gz
data/invaders.gfx.gz: $(TARGET_DATA_invaders.gfx.gz)
