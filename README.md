# Invaders

A space invaders game, insanely complicated done.

## Building

The build system is using plain GNU Make (developed using version 4.3).

Make sure that SDL 1.2 is installed.
Consult http://www.libsdl.org/ or your Linux distributions package manager.

Build invaders as follows:

    make exe/invaders CPPFLAGS=-DFULLSCREEN

Currently there is no install target.
Launch the game directly from the build directory.

    _out/host/bin/invaders

### Build Options

The following build options are supported by the GNU make based build system.

| Argument | Description                      |
|----------|----------------------------------|
| V=1      | Output full compilation commands |

## Input

Keys to use in the game:

SPACE  Fire
ESCAPE Quit
Z      Turn left
X      Turn right
P      Create snapshot (invaders_snap.tga)

There is joystick support as well.
