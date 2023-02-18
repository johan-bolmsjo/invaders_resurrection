/**
 * NEO specific parts of the synth.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#ifdef HAVE_LIBNEO

#include <neo/neo.h>

static NeoSound neo_sound = {NEO_SOUND_S16, FREQUENCY, 100};
static int16_t* buffer;

/* Open synth.
 * Returns 0 on success and -1 on failure.
 * Even if this function fails, it's okay to call the other functions.
 * They will just don't do anything.
 */

int
synth_open()
{
    if (neo_sound_open(&neo_sound))
        return -1;

    neo_sound.mode = NEO_SOUND_S16;

    buffer = malloc(neo_sound.frag_size * 2);
    if (!buffer) {
        neo_sound_close();
        return -1;
    }

    open_f = 1;
    samples_ms = (neo_sound.freq << 8) / 1000;
    frequency = neo_sound.freq;

    memset(channels, 0, sizeof(Channel) * SYNTH_NUM_CHANNELS);

    return 0;
}

/* Close synth.
 */

void
synth_close()
{
    if (open_f) {
        neo_sound_close();
        free(buffer);
        open_f = 0;
    }
}

/* Write samples to the audio device.
 */

void
synth_update()
{
    int frags;

    if (open_f) {
        frags = neo_sound_writable() / neo_sound.frag_size;
        for (; frags > 0; frags--) {
            mix_samples(buffer, neo_sound.frag_size);
            neo_sound_write(&neo_sound, buffer, neo_sound.frag_size);
        }
    }
}

#endif /* HAVE_LIBNEO */
