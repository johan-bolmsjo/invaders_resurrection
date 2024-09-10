$(call define-srcs, invaders, \
	src/invaders/armada.c \
	src/invaders/bomber.c \
	src/invaders/collision.c \
	src/invaders/gfx.c \
	src/invaders/main.c \
	src/invaders/missiles.c \
	src/invaders/mystery.c \
	src/invaders/player.c \
	src/invaders/prim.c \
	src/invaders/runlevel.c \
	src/invaders/screenshot.c \
	src/invaders/sfx.c \
	src/invaders/shields.c \
	src/invaders/shot.c \
	src/invaders/sprite.c \
	src/invaders/stars.c \
	src/invaders/status.c \
	src/invaders/text.c \
	src/invaders/title.c \
	src/invaders/ufo.c \
)

$(call define-exe, invaders, invaders libgfx libmedia libsynth libutil)

# Make sure data files are generated as they are included by these source files.
src/invaders/text.c: $(TARGET_SRCS_compressed_font_data.c)
src/invaders/gfx.c:  $(TARGET_SRCS_compressed_gfx_data.c)
