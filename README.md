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

### emscripten

Steps for building invaders using emscripten.

NOTE: Building with emscripten is work in progress. SDL initialization currently fails at run time.

Install emscripten core as described at https://emscripten.org/docs/getting_started/downloads.html.

Create a directory for installing emscripten compiled libraries:

```bash
mkdir .../emscripten
```

Build zlib:

```bash
git clone https://github.com/emscripten-ports/zlib .../zlib
cd .../zlib
emconfigure ./configure --prefix=/opt/emscripten --static
emmake make install
```

Build SDL:

```bash
git clone --depth 1 --branch release-2.30.7 https://github.com/libsdl-org/SDL .../SDL-2.30.7
cd .../SDL-2.30.7
emconfigure ./configure --prefix=/opt/emscripten --disable-pthreads --enable-static --disable-shared
emmake make -j8
emmake make install
```

Build invaders:

```bash
NO_THREADS=1 LDFLAGS="-sLEGACY_GL_EMULATION" EM_PKG_CONFIG_PATH=/opt/emscripten/lib/pkgconfig emmake make -j8 VARIANT=release exe/invaders
```

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
