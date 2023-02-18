$(call define-host-srcs, invaders, \
	src/invaders/armada.c \
	src/invaders/bomber.c \
	src/invaders/collision.c \
	src/invaders/dg.c \
	src/invaders/error.c \
	src/invaders/gfx.c \
	src/invaders/main.c \
	src/invaders/missiles.c \
	src/invaders/mystery.c \
	src/invaders/player.c \
	src/invaders/prim.c \
	src/invaders/runlevel.c \
	src/invaders/sfx.c \
	src/invaders/shields.c \
	src/invaders/shot.c \
	src/invaders/snap.c \
	src/invaders/sprite.c \
	src/invaders/stars.c \
	src/invaders/status.c \
	src/invaders/text.c \
	src/invaders/title.c \
	src/invaders/ufo.c \
)

$(call define-host-exe, invaders, invaders libgfx libsynth)
$(TARGET_HOST_EXE_invaders): $(TARGET_HOST_LIB_gfx) $(TARGET_HOST_LIB_synth)

# Make sure data files are generated as they are included by these source files.
src/invaders/text.c: $(TARGET_SRCS_font_data.c)
src/invaders/gfx.c:  $(TARGET_SRCS_gfx_data.c)
