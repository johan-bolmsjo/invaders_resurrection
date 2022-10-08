# gfxgen

Batch based graphics program. Commands are read from an input file.
The output file can be read with libgfx.

    usage: gfxgen -i <input file> -o <output file>

Commands:

    object          foobar          # Start object with the name foobar
    frame           foobar/0.tga    # This image will be frame 0
    frame           foobar/1.tga    # And this will be frame 1
    rgb_to_alpha    000000          # Translate black to transparent
    autocrop_alpha                  # Autocrop by alpha == 0
    scale_maxspect  40 40           # Scale frames to a size of max 40 x 40 pixels
    end             gfx alpha cm !  # Terminate object foobar.
                                    # - Save 16 bit graphics data
									# - 8 bit _special_ alpha chanel
									# - 1 bit 32 bit alligned collition mask.

The source images `0.tga` and `1.tga` must be of equal size.

The hotspot is set to the middle of the object when doing a scale_maxspect.
Maybe I'll add a hotspot command some day.
