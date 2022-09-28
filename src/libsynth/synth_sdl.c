/**
 * SDL specific parts of the synth.
 *
 * BUGS:
 * The SDL way of handling sound update is different from what Invaders
 * was written for in the beginning. Therefore the sound effects will
 * not be played back properly. The problem is that SDL uses a callback
 * routine that can be called anytime, while when Invaders was NEO only,
 * it called synth_update when all sound effects was in a consistent
 * state.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#ifdef HAVE_LIBSDL

#include <SDL.h>

static SDL_AudioSpec sdl_audio_spec;


/* Callback routine called by SDL when it wants more audio data.
 */

static void
sdl_audio_callback (void *userdata, Uint8 *stream, int len)
{
  mix_samples ((int16_t *)stream, len / 2);
}


/* Open synth.
 * Returns 0 on success and -1 on failure.
 * Even if this function fails, it's okay to call the other functions.
 * They will just don't do anything.
 */

int
synth_open ()
{
  sdl_audio_spec.freq = FREQUENCY;
  sdl_audio_spec.format = AUDIO_S16SYS;
  sdl_audio_spec.channels = 1;
#ifdef AUDIO_44KHZ
  sdl_audio_spec.samples = 4096;
#else
  sdl_audio_spec.samples = 2048;
#endif
  sdl_audio_spec.callback = sdl_audio_callback;
  
  if (SDL_OpenAudio (&sdl_audio_spec, 0) < 0)
    return -1;
  
  open_f = 1;
  samples_ms = (sdl_audio_spec.freq << 8) / 1000;
  frequency = sdl_audio_spec.freq;
  
  memset (channels, 0, sizeof(Channel) * SYNTH_NUM_CHANNELS);
  
  SDL_PauseAudio (0);
  
  return 0;
}


/* Close synth.
 */

void
synth_close ()
{
  if (open_f)
    {
      open_f = 0;
      SDL_CloseAudio ();
    }
}


/* This doesn't do anything with SDL since we have the callback
 * routine instead.
 */

void
synth_update ()
{
}

#endif /* HAVE_LIBSDL */
