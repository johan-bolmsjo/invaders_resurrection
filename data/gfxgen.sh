#!/bin/sh
# This will generate a new invaders.gfx file if gfxgen has been built
# and all targa files are present in the tga/ directory.
#

if [ -x ../src/gfxgen/gfxgen ]
then
    ../src/gfxgen/gfxgen -i invaders.def -o invaders.gfx.gz
else
    echo "Compile gfxgen first."
fi
