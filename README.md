# Invaders Resurrection

Bringing life back to a space invaders game from 1999.

Overall TODO list is kept in [plan.org](plan.org).

## Building

The build system is using plain GNU Make (developed using version 4.3).

Make sure that SDL 2.x is installed.
Consult http://www.libsdl.org/ or your Linux distributions package manager.

Build invaders as follows:

    make exe/invaders

Currently there is no install target.
Launch the game directly from the build directory.

    _out/host/bin/invaders

### Build Options

The following build options are supported by the GNU make based build system.

| Argument           | Description                              |
|--------------------|------------------------------------------|
| V=1 or VERBOSE=1   | Output full compilation commands         |
| VARIANT=release    | Optimization                             |
| VARIANT=debug      | No optimization and debug info (default) |
| VARIANT=debug-asan | Debug with address sanitizer             |

## Input

Keys to use in the game:

| Key      | Action                                    |
|----------|-------------------------------------------|
| SPACE    | Fire                                      |
| ESCAPE   | Quit                                      |
| A, LEFT  | Turn left                                 |
| D, RIGHT | Turn right                                |
| F11      | Toggle full screen mode                   |
| F12      | Save screenshot (invaders_screenshot.tga) |

There is game controller support as well.
