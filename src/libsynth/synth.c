/**
 * Simple software synth ala oscillator model.
 * Inspired by the SID chip.
 *
 * History:
 *
 * The first version of this code used floats; it was only converted to
 * fix point arithmetic to run on my old Netwinder with a 275 MHz SA-110
 * CPU.
 *
 * The original code has been lost to time and only the fix point
 * version remains.
 *
 * @author Johan Bolmsjo <johan@nocrew.org>
 */

#include "libsynth.h"

#include <SDL.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum {
    Frequency = 44100,
};

// Channel envelopes
enum {
    NumberOfRegularChannelEnvelopes = 4,
    NumberOfModifierChannelEnvelopes = 2,
    NumberOfChannelEnvelopes = NumberOfRegularChannelEnvelopes + NumberOfModifierChannelEnvelopes,
    FMModifierChannelEnvelopeIndex = NumberOfChannelEnvelopes - 2,
    PWModifierChannelEnvelopeIndex = NumberOfChannelEnvelopes - 1,
};

enum {
    NumberOfPeriodStages = 3,
    PeriodStageInactive = NumberOfPeriodStages,
};

// Stages of an ADSR envelope.
enum EnvelopeStage {
    Attack,
    Decay,
    Sustain,
    Release,
    NumberOfEnvelopeStages,
};

struct Envelope {
    enum SynthWaveform waveform;
    int hz;  // 24:8 format
    int attack_amplitude; // [0, 1]  8:24 format
    int pulse_width;      // [0, 1] — High pulse width  8:24 format */

    // Frequency fraction to keep the right frequency, updated every period.
    // 8:24 format
    int hz_adjust;

    // ADSR parameters
    enum EnvelopeStage stage;
    int amplitude;                       // (8:24 format)
    int delta[NumberOfEnvelopeStages];   // Delta values (8:24 format)
    int samples[NumberOfEnvelopeStages]; // Length in samples

    // Period state
    int period_stage;
    int period_amplitude;                     // (8:24 format)
    int period_delta[NumberOfPeriodStages];   // Delta values (8:24 format)
    int period_samples[NumberOfPeriodStages]; // Length in samples
};

struct Channel {
    struct Envelope envelopes[NumberOfChannelEnvelopes];
};

static struct {
    bool is_open;
    int  samples_ms; // 24:8 format
    int  hz;
    struct Channel channels[NumberOfSynthChannels];
} synth;

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

// Callback routine called by SDL when it wants more audio data.
static void
sdl_audio_callback(void* userdata, Uint8* stream, int len)
{
    (void)userdata;
    mix_samples((int16_t*)stream, len / 2);
}

int
synth_open(void)
{
    static SDL_AudioSpec sdl_audio_spec;

    sdl_audio_spec.freq = Frequency;
    sdl_audio_spec.format = AUDIO_S16SYS;
    sdl_audio_spec.channels = 1;
    sdl_audio_spec.samples = 4096; // TODO(jb): Optimize this for low latency
    sdl_audio_spec.callback = sdl_audio_callback;

    if (SDL_OpenAudio(&sdl_audio_spec, 0) < 0)
        return -1;

    synth.is_open = true;
    synth.samples_ms = (sdl_audio_spec.freq << 8) / 1000;
    synth.hz = sdl_audio_spec.freq;

    memset(&synth.channels, 0, sizeof(synth.channels));

    SDL_PauseAudio(0);

    return 0;
}

void
synth_close(void)
{
    if (synth.is_open) {
        synth.is_open = false;
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

// Maps an external SynthEnvelope to an internal Envelope.
static void
map_envelope(struct SynthEnvelope* src, struct Envelope* dst)
{
    dst->waveform = src->waveform;
    dst->hz = src->hz * (1 << 8);
    dst->attack_amplitude = src->attack_amplitude * (1 << 24);
    dst->pulse_width = src->pulse_width * (1 << 24);
    dst->hz_adjust = 0;

    // ADSR
    dst->stage = Attack;
    dst->amplitude = 0;
    for (int i = 0; i < 4; i++) {
        dst->samples[i] = src->adsr_ms[i] * synth.samples_ms >> 8;
    }
    dst->delta[Attack] = (1 << 24) * (1.0 / dst->samples[Attack]);
    dst->delta[Decay] = (1 << 24) * (-(1.0 - src->sustain_ratio) / dst->samples[Decay]);
    dst->delta[Sustain] = 0;
    dst->delta[Release] = (1 << 24) * (-src->sustain_ratio / dst->samples[Release]);

    // Period
    dst->period_stage = PeriodStageInactive;
}

// Updates all envelopes on channel.
static inline void
update_channel(int ch)
{
    bool fm_pwm_init = false;
    int fm = 0, pwm = 0;

    for (int w = NumberOfChannelEnvelopes - 1; w >= 0; w--) {
        struct Envelope *e = &synth.channels[ch].envelopes[w];
        if (e->waveform == SynthWaveformNone)
            continue;

        /* TODO(jb): BUG: One sample ahead!!
         *           (Future me has no idea what this means)
         */
        for (; e->stage < NumberOfEnvelopeStages; e->stage++) {
            if (e->samples[e->stage]) {
                e->amplitude += e->delta[e->stage];
                e->samples[e->stage]--;
                break;
            }
        }

        if (e->stage == NumberOfEnvelopeStages) {
            // Envelope has played out; mark it as such!
            e->waveform = SynthWaveformNone;
            e->attack_amplitude = 0;
            continue;
        }

        for (; e->period_stage < NumberOfPeriodStages; e->period_stage++) {
            if (e->period_samples[e->period_stage]) {
                e->period_amplitude += e->period_delta[e->period_stage];
                e->period_samples[e->period_stage]--;
                break;
            }
        }

        if (e->period_stage == NumberOfPeriodStages) {
            // Period has played out. Calculate next period parameters.

            int hz; // 24:8 format
            if (w < NumberOfRegularChannelEnvelopes) {
                if (!fm_pwm_init) {
                    struct Envelope* e = &synth.channels[ch].envelopes[FMModifierChannelEnvelopeIndex];
                    fm = ((int64_t)e->amplitude * e->period_amplitude >> 24) * e->attack_amplitude >> 24;
                    e = &synth.channels[ch].envelopes[PWModifierChannelEnvelopeIndex];
                    pwm = ((int64_t)e->amplitude * e->period_amplitude >> 24) * e->attack_amplitude >> 24;
                    fm_pwm_init = true;
                }
                hz = e->hz + (int)(e->hz * (int64_t)fm >> 24);
            } else {
                hz = e->hz;
            }

            if (!hz) {
                hz = 1;
            }

            int samples = (synth.hz << 8) / hz;
            if (samples < 6) {
                samples = 6;
            }
            e->hz_adjust += (((synth.hz << 8) / hz) - samples);

            if (e->hz_adjust >= (1 << 24)) {
                e->hz_adjust -= (1 << 24);
                samples++;
            }

            switch (e->waveform) {
            case SynthWaveformPulse: {
                int pw = e->pulse_width + pwm; // (8:24 format)
                if (pw < 0) {
                    pw = 0;
                }
                if (pw > (1 << 24)) {
                    pw = 1 << 24;
                }
                const int hi = (int64_t)samples * pw >> 24;
                const int lo = samples - hi;
                e->period_stage = 0;
                e->period_samples[1] = 0;
                e->period_samples[2] = 0;

                if (hi) {
                    e->period_amplitude = 1 << 24;
                } else {
                    e->period_amplitude = -1 << 24;
                }

                if (hi && lo) {
                    e->period_delta[0]   = 0;
                    e->period_samples[0] = hi - 1;
                    e->period_delta[1]   = -2 << 24;
                    e->period_samples[1] = 1;
                    e->period_delta[2]   = 0;
                    e->period_samples[2] = lo - 1;
                } else {
                    e->period_delta[0]   = 0;
                    e->period_samples[0] = samples - 1;
                }
                break;
            }

            case SynthWaveformTriangle: {
                const int hi = samples >> 2;
                const int mi = hi << 1;
                const int lo = samples - hi - mi;
                e->period_stage      = 0;
                e->period_amplitude  = 0;
                e->period_delta[0]   = (1 << 24) / (hi - 1);
                e->period_samples[0] = hi - 1;
                e->period_delta[1]   = (-2 << 24) / (mi - 1);
                e->period_samples[1] = mi - 1;
                e->period_delta[2]   = (1 << 24) / (lo - 1);
                e->period_samples[2] = lo - 1;
                break;
            }

            case SynthWaveformSaw: {
                const int hi = samples >> 1;
                const int lo = samples - hi;
                e->period_stage      = 0;
                e->period_amplitude  = 0;
                e->period_delta[0]   = (1 << 24) / (hi - 1);
                e->period_samples[0] = hi - 1;
                e->period_delta[1]   = -2 << 24;
                e->period_samples[1] = 1;
                e->period_delta[2]   = (1 << 24) / (lo - 1);
                e->period_samples[2] = lo - 1;
                break;
            }

            default:
                break;
            }
        }
    }
}

// Generate samples to write to audio device.
static void
mix_samples(int16_t* buffer, int samples)
{
    int i, j, ch_t[NumberOfSynthChannels] = {0};
    int ch_mix, w_mix;
    struct Envelope* env;

    for (i = NumberOfSynthChannels - 1; i >= 0; i--) {
        for (j = 0; j < NumberOfChannelEnvelopes; j++) {
            if (synth.channels[i].envelopes[j].waveform)
                ch_t[i] = 1;
        }
    }

    for (i = 0; i < samples; i++) {
        ch_mix = 0;
        for (j = 0; j < NumberOfSynthChannels; j++) {
            if (ch_t[j]) {
                update_channel(j);
                env = synth.channels[j].envelopes;
                w_mix = (((int64_t)env[0].amplitude * env[0].period_amplitude >> 24) *
                             env[0].attack_amplitude >> 24);
                w_mix += (((int64_t)env[1].amplitude * env[1].period_amplitude >> 24) *
                              env[1].attack_amplitude >> 24);
                w_mix += (((int64_t)env[2].amplitude * env[2].period_amplitude >> 24) *
                              env[2].attack_amplitude >> 24);
                w_mix += (((int64_t)env[3].amplitude * env[3].period_amplitude >> 24) *
                              env[3].attack_amplitude >> 24);
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

void
synth_envelope(struct SynthEnvelope* sw, int ch)
{
    int i;

    if (ch < NumberOfSynthChannels && synth.is_open) {
        for (i = 0; i < NumberOfRegularChannelEnvelopes; i++) {
            if (synth.channels[ch].envelopes[i].waveform == SynthWaveformNone)
                break;
        }
        if (i < NumberOfRegularChannelEnvelopes)
            map_envelope(sw, &synth.channels[ch].envelopes[i]);
    }
}

void
synth_envelope_fm(struct SynthEnvelope* sw, int ch)
{
    if (ch < NumberOfSynthChannels && synth.is_open)
        map_envelope(sw, &synth.channels[ch].envelopes[FMModifierChannelEnvelopeIndex]);
}

void
synth_envelope_pwm(struct SynthEnvelope* sw, int ch)
{
    if (ch < NumberOfSynthChannels && synth.is_open)
        map_envelope(sw, &synth.channels[ch].envelopes[PWModifierChannelEnvelopeIndex]);
}

void
synth_channel_kill(int ch)
{
    int w;

    if (ch < NumberOfSynthChannels && synth.is_open) {
        for (w = 0; w < NumberOfChannelEnvelopes; w++) {
            synth.channels[ch].envelopes[w].waveform = SynthWaveformNone;
            synth.channels[ch].envelopes[w].attack_amplitude = 0;
        }
    }
}

int
synth_channel_envelopes(int ch)
{
    int waves = 0, w;

    if (ch < NumberOfSynthChannels && synth.is_open) {
        for (w = 0; w < NumberOfRegularChannelEnvelopes; w++) {
            if (synth.channels[ch].envelopes[w].waveform != SynthWaveformNone)
                waves++;
        }
    }

    return waves;
}

void
synth_lock(void)
{
    // TODO(jb): Check what this does or if a mutex can be used instead. SDL
    //           callback called from thread? Also migrating to SDL2.
    SDL_LockAudio();
}

void
synth_unlock(void)
{
    SDL_UnlockAudio();
}
