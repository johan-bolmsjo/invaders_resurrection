CFLAGS = @CFLAGS@ @MEDIA_CFLAGS@

LIBS = @LIBS@ @MEDIA_LIBS@

INCLUDES = \
	-I../..		\
	-I../libgfx	\
	-I../libsynth

BUILT_SOURCES =	\
	gfx_data.c	\
	font_data.c

bin_PROGRAMS = \
	invaders

invaders_LDADD = \
	../libgfx/libgfx.a	\
	../libsynth/libsynth.a

invaders_SOURCES = \
	all.h		\
	armada.c	\
	armada.h	\
	bomber.c	\
	bomber.h	\
	collision.c	\
	collision.h	\
	defs.h		\
	dg.c		\
	dg.h		\
	error.c		\
	error.h		\
	gfx.c		\
	gfx.h		\
	gids.h		\
	groups.h	\
	joy.h		\
	main.c		\
	missiles.c	\
	missiles.h	\
	mystery.c	\
	mystery.h	\
	player.c	\
	player.h	\
	prim.c		\
	prim.h		\
	runlevel.c	\
	runlevel.h	\
	sfx.c		\
	sfx.h		\
	shields.c	\
	shields.h	\
	shot.c		\
	shot.h		\
	snap.c		\
	snap.h		\
	sprite.c	\
	sprite.h	\
	stars.c		\
	stars.h		\
	status.c	\
	status.h	\
	text.c		\
	text.h		\
	title.c		\
	title.h		\
	ufo.c		\
	ufo.h

gfx_data.c :	../../data/invaders.gfx.deflate
		../tools/btoc ../../data/invaders.gfx.deflate gfx > \
			gfx_data.c

font_data.c :	../../data/pearl_8x8.dat.deflate
		../tools/btoc ../../data/pearl_8x8.dat.deflate font > \
			font_data.c
