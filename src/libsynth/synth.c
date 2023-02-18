/**
 * Simple software synth ala oscillator model.
 * Inspired by the SID chip.
 *
 * BUGS:
 * Very CPU hungry.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#include "libsynth.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <SDL.h>

#define FREQUENCY 44100

#define NUM_WAVES       4 /* Normal waves per channel */
#define NUM_WAVES_MOD   2 /* Modifier waves per channel */
#define NUM_WAVES_TOTAL (NUM_WAVES + NUM_WAVES_MOD)

#define WAVE_FREQ_MOD 4 /* Indexes */
#define WAVE_PW_MOD   5

#define ATTACK  0
#define DECAY   1
#define SUSTAIN 2
#define RELEASE 3

typedef struct _Wave {
    int wf; /* Waveform */
    int f;  /* Frequency (Hz) 24:8 format */
    int aa; /* Attack amplitude (0.0 to 1.0) 8:24 format */
    int pw; /* High pulse width (0.0 to 1.0) 8:24 format */

    int ff; /* Frequency fraction to keep the right frequency,
               updated every period 8:24 format */

    /* ADSR.
     */
    int i;    /* Index to "d" and "s" */
    int a;    /* Amplitude 8:24 format */
    int d[4]; /* Delta values 8:24 format */
    int s[4]; /* Length in samples */

    /* Period.
     */
    int ip;    /* Index to "dp" and "sp" */
    int ap;    /* Amplitude 8:24 format */
    int dp[3]; /* Delta values 8:24 format */
    int sp[3]; /* Length in samples */
} Wave;

typedef struct _Channel {
    Wave w[NUM_WAVES_TOTAL]; /* Waves */
} Channel;

static int open_f = 0;
static int samples_ms; /* 24:8 format */
static int frequency;
static Channel channels[SYNTH_NUM_CHANNELS];

static void mix_samples(int16_t* buffer, int samples);

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

/* Callback routine called by SDL when it wants more audio data.
 */

static void
sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;
    mix_samples((int16_t*)stream, len / 2);
}

/* Open synth.
 * Returns 0 on success and -1 on failure.
 * Even if this function fails, it's okay to call the other functions.
 * They will just don't do anything.
 */

int
synth_open(void)
{
    static SDL_AudioSpec sdl_audio_spec;

    sdl_audio_spec.freq = FREQUENCY;
    sdl_audio_spec.format = AUDIO_S16SYS;
    sdl_audio_spec.channels = 1;
    sdl_audio_spec.samples = 4096; // TODO: Optimize this for low latency
    sdl_audio_spec.callback = sdl_audio_callback;

    if (SDL_OpenAudio(&sdl_audio_spec, 0) < 0)
        return -1;

    open_f = 1;
    samples_ms = (sdl_audio_spec.freq << 8) / 1000;
    frequency = sdl_audio_spec.freq;

    memset(channels, 0, sizeof(Channel) * SYNTH_NUM_CHANNELS);

    SDL_PauseAudio(0);

    return 0;
}

/* Close synth.
 */

void
synth_close(void)
{
    if (open_f) {
        open_f = 0;
        SDL_CloseAudio();
    }
}

void
synth_update(void)
{
    /* This doesn't do anything with SDL since we have the callback
     * routine instead.
     */
}

/* Maps a SynthWave to a Wave.
 * SynthWaves are used externaly while Waves are used internaly.
 */

static void
map_wave(SynthWave* sw, Wave* w)
{
    int i;

    w->wf = sw->wf;
    w->f = sw->f * (1 << 8);
    w->aa = sw->aa * (1 << 24);
    w->pw = sw->pw * (1 << 24);
    w->ff = 0;

    /* ADSR
     */
    w->i = 0;
    w->a = 0;
    for (i = 0; i < 4; i++)
        w->s[i] = sw->adsr[i] * samples_ms >> 8;
    w->d[ATTACK] = (1 << 24) * (1.0 / w->s[ATTACK]);
    w->d[DECAY] = (1 << 24) * (-(1.0 - sw->sl) / w->s[DECAY]);
    w->d[SUSTAIN] = 0;
    w->d[RELEASE] = (1 << 24) * (-sw->sl / w->s[RELEASE]);

    /* Period
     */
    w->ip = 3;
}

/* Updates all waves on a channel.
 */

static inline void
update_channel(int ch)
{
    int w, hi, mi, lo, samples, fm_pwm_init = 0;
    int f;                   /* 24:8 format */
    int fm = 0, pwm = 0, pw; /* 8:24 format */
    Wave *wp, *wp2;

    for (w = NUM_WAVES_TOTAL - 1; w >= 0; w--) {
        wp = &channels[ch].w[w];
        if (wp->wf == SYNTH_WF_NONE)
            continue;

        /* BUG: One sample ahead!!
         */
        for (; wp->i < 4; wp->i++) {
            if (wp->s[wp->i]) {
                wp->a += wp->d[wp->i];
                wp->s[wp->i]--;
                break;
            }
        }

        if (wp->i == 4) {
            wp->wf = SYNTH_WF_NONE;
            wp->aa = 0;
            continue;
        }

        for (; wp->ip < 3; wp->ip++) {
            if (wp->sp[wp->ip]) {
                wp->ap += wp->dp[wp->ip];
                wp->sp[wp->ip]--;
                break;
            }
        }

        if (wp->ip == 3) {
            if (w < NUM_WAVES) {
                if (!fm_pwm_init) {
                    wp2 = &channels[ch].w[WAVE_FREQ_MOD];
                    fm = ((int64_t)wp2->a * wp2->ap >> 24) * wp2->aa >> 24;
                    wp2 = &channels[ch].w[WAVE_PW_MOD];
                    pwm = ((int64_t)wp2->a * wp2->ap >> 24) * wp2->aa >> 24;
                    fm_pwm_init = 1;
                }
                f = wp->f + (int)(wp->f * (int64_t)fm >> 24);
            } else {
                f = wp->f;
            }

            if (!f)
                f = 1;

            samples = (frequency << 8) / f;
            if (samples < 6)
                samples = 6;
            wp->ff += (((frequency << 8) / f) - samples);

            if (wp->ff >= (1 << 24)) {
                wp->ff -= (1 << 24);
                samples++;
            }

            switch (wp->wf) {
            case SYNTH_WF_PULSE:
                pw = wp->pw + pwm;
                if (pw < 0)
                    pw = 0;
                if (pw > (1 << 24))
                    pw = 1 << 24;
                hi = (int64_t)samples * pw >> 24;
                lo = samples - hi;
                wp->ip = 0;
                wp->sp[1] = 0;
                wp->sp[2] = 0;

                if (hi)
                    wp->ap = 1 << 24;
                else
                    wp->ap = -1 << 24;

                if (hi && lo) {
                    wp->dp[0] = 0;
                    wp->sp[0] = hi - 1;
                    wp->dp[1] = -2 << 24;
                    wp->sp[1] = 1;
                    wp->dp[2] = 0;
                    wp->sp[2] = lo - 1;
                } else {
                    wp->dp[0] = 0;
                    wp->sp[0] = samples - 1;
                }
                break;

            case SYNTH_WF_TRIANGLE:
                hi = samples >> 2;
                mi = hi << 1;
                lo = samples - hi - mi;
                wp->ip = 0;
                wp->ap = 0;
                wp->dp[0] = (1 << 24) / (hi - 1);
                wp->sp[0] = hi - 1;
                wp->dp[1] = (-2 << 24) / (mi - 1);
                wp->sp[1] = mi - 1;
                wp->dp[2] = (1 << 24) / (lo - 1);
                wp->sp[2] = lo - 1;
                break;

            case SYNTH_WF_SAW:
                hi = samples >> 1;
                lo = samples - hi;
                wp->ip = 0;
                wp->ap = 0;
                wp->dp[0] = (1 << 24) / (hi - 1);
                wp->sp[0] = hi - 1;
                wp->dp[1] = -2 << 24;
                wp->sp[1] = 1;
                wp->dp[2] = (1 << 24) / (lo - 1);
                wp->sp[2] = lo - 1;
            }
        }
    }
}

/* Generate samples to write to audio device.
 */

static void
mix_samples(int16_t* buffer, int samples)
{
    int i, j, ch_t[SYNTH_NUM_CHANNELS] = {0};
    int ch_mix, w_mix;
    Wave* wp;

    for (i = SYNTH_NUM_CHANNELS - 1; i >= 0; i--) {
        for (j = 0; j < NUM_WAVES_TOTAL; j++) {
            if (channels[i].w[j].wf)
                ch_t[i] = 1;
        }
    }

    for (i = 0; i < samples; i++) {
        ch_mix = 0;
        for (j = 0; j < SYNTH_NUM_CHANNELS; j++) {
            if (ch_t[j]) {
                update_channel(j);
                wp = channels[j].w;
                w_mix = (((int64_t)wp[0].a * wp[0].ap >> 24) *
                             wp[0].aa >>
                         24);
                w_mix += (((int64_t)wp[1].a * wp[1].ap >> 24) *
                              wp[1].aa >>
                          24);
                w_mix += (((int64_t)wp[2].a * wp[2].ap >> 24) *
                              wp[2].aa >>
                          24);
                w_mix += (((int64_t)wp[3].a * wp[3].ap >> 24) *
                              wp[3].aa >>
                          24);
                if (w_mix < (-1 << 24))
                    w_mix = -1 << 24;
                if (w_mix > (1 << 24) - 1)
                    w_mix = (1 << 24) - 1;
                ch_mix += w_mix;
            }
        }

        buffer[i] = ch_mix >> 10;
    }
}

/* Plays a wave on a channel.
 */

void
synth_wave(SynthWave* sw, int ch)
{
    int i;

    if (ch < SYNTH_NUM_CHANNELS && open_f) {
        for (i = 0; i < NUM_WAVES; i++) {
            if (channels[ch].w[i].wf == SYNTH_WF_NONE)
                break;
        }
        if (i < NUM_WAVES)
            map_wave(sw, &channels[ch].w[i]);
    }
}

/* Modifies all frequencies on a channel.
 */

void
synth_fm(SynthWave* sw, int ch)
{
    if (ch < SYNTH_NUM_CHANNELS && open_f)
        map_wave(sw, &channels[ch].w[WAVE_FREQ_MOD]);
}

/* Modifies all pulse widths on a channel.
 */

void
synth_pwm(SynthWave* sw, int ch)
{
    if (ch < SYNTH_NUM_CHANNELS && open_f)
        map_wave(sw, &channels[ch].w[WAVE_PW_MOD]);
}

/* Kills all sounds on a channel.
 */

void
synth_channel_kill(int ch)
{
    int w;

    if (ch < SYNTH_NUM_CHANNELS && open_f) {
        for (w = 0; w < NUM_WAVES_TOTAL; w++) {
            channels[ch].w[w].wf = SYNTH_WF_NONE;
            channels[ch].w[w].aa = 0;
        }
    }
}

/* Returns the number of waves currently playing on a channel not
 * counting modifiers.
 */

int
synth_waves_on_channel(int ch)
{
    int waves = 0, w;

    if (ch < SYNTH_NUM_CHANNELS && open_f) {
        for (w = 0; w < NUM_WAVES; w++) {
            if (channels[ch].w[w].wf != SYNTH_WF_NONE)
                waves++;
        }
    }

    return waves;
}

/* Use before creating a sound effect. SDL needs this.
 */

void
synth_lock(void)
{
    // TODO: Check what this does or if a mutex can be used instead. SDL
    //       callback called from thread? Also migrating to SDL2.
    SDL_LockAudio();
}

/* Use after having created a sound effect. SDL needs this.
 */

void
synth_unlock(void)
{
    SDL_UnlockAudio();
}
