# Invaders

A space invaders game, insanely complicated done. One can either use
my NEO Linux frame buffer library or SDL with it.

These are the configure flags that can be used:
  --enable-audio-44kHz    Use 44kHz audio mode
  --enable-fullscreen     Use full screen video mode
  --enable-debug          Compile with debug information
  --with-libneo           Use libneo as media library
  --with-libsdl           Use libSDL as media library

## Building

To build the game, first make sure that NEO or SDL is installed. You
probably want to use SDL even if NEO works best with this game, at
least at the moment. NEO provides solid 1 sync screen update, while
SDL is a bit jerky.

Get the NEO library at:
http://johan.nocrew.org/software-neo/

Get the SDL library at:
http://www.libsdl.org/

Example:

    $ ./configure --with-libsdl --enable-fullscreen
    $ make
    $ su -c 'make install-strip'

## Input

Keys to use in the game:

SPACE  Fire
ESCAPE Quit
Z      Turn left
X      Turn right
P      Create snapshot (invaders_snap.tga)

There is joystick support as well.
