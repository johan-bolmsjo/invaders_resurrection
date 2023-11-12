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
};

// Stages of an ADSR envelope.
enum EnvelopeStage {
    Attack,
    Decay,
    Sustain,
    Release,
    NumberOfEnvelopeStages,
};

// Data for one period of a sound.
struct Period {
    int stage;                         // [0, NumberOfPeriodStages]
    int amplitude;                     // [-1, 1] — Current sample amplitude (8:24 format)
    int slope[NumberOfPeriodStages];   // Period stage slopes (8:24 format)
    int samples[NumberOfPeriodStages]; // Period stage lengths in samples
};

struct Envelope {
    enum SynthWaveform waveform;
    int hz;               // (24:8 format)
    int attack_amplitude; // [0, 1] — (8:24 format)
    int pulse_width;      // [0, 1] — High pulse width (8:24 format) */

    // Frequency fraction to keep the right frequency, updated every period (8:24 format).
    int hz_adjust;

    // ADSR parameters
    enum EnvelopeStage stage;
    int amplitude;                       // [0, 1] — Current sample amplitude (8:24 format)
    int slope[NumberOfEnvelopeStages];   // Envelope stage slopes (8:24 format)
    int samples[NumberOfEnvelopeStages]; // Envelope stage lengths in samples

    // Data for the current sound period.
    struct Period period;
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
    dst->slope[Attack] = (1 << 24) * (1.0 / dst->samples[Attack]);
    dst->slope[Decay] = (1 << 24) * (-(1.0 - src->sustain_ratio) / dst->samples[Decay]);
    dst->slope[Sustain] = 0;
    dst->slope[Release] = (1 << 24) * (-src->sustain_ratio / dst->samples[Release]);

    // Period
    dst->period.stage = NumberOfPeriodStages; // mark as inactive
}

// Updates all envelopes on channel.
static inline void
update_channel(int ch)
{
    bool fm_pwm_init = false;
    int fm = 0, pwm = 0;

    for (int ei = NumberOfChannelEnvelopes - 1; ei >= 0; ei--) {
        struct Envelope* e = &synth.channels[ch].envelopes[ei];
        if (e->waveform == SynthWaveformNone)
            continue;

        /* TODO(jb): BUG: One sample ahead!!
         *           (Future me has no idea what this means)
         */
        for (; e->stage < NumberOfEnvelopeStages; e->stage++) {
            if (e->samples[e->stage] > 0) {
                e->amplitude += e->slope[e->stage];
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

        struct Period* p = &e->period;

        for (; p->stage < NumberOfPeriodStages; p->stage++) {
            if (p->samples[p->stage]) {
                p->amplitude += p->slope[p->stage];
                p->samples[p->stage]--;
                break;
            }
        }

        if (p->stage == NumberOfPeriodStages) {
            // Period has played out. Calculate next period parameters.

            int hz; // 24:8 format
            if (ei < NumberOfRegularChannelEnvelopes) {
                if (!fm_pwm_init) {
                    struct Envelope* e2 = &synth.channels[ch].envelopes[FMModifierChannelEnvelopeIndex];
                    fm = ((int64_t)e2->amplitude * e2->period.amplitude >> 24) * e2->attack_amplitude >> 24;
                    e2 = &synth.channels[ch].envelopes[PWModifierChannelEnvelopeIndex];
                    pwm = ((int64_t)e2->amplitude * e2->period.amplitude >> 24) * e2->attack_amplitude >> 24;
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
                p->stage = 0;
                p->samples[1] = 0;
                p->samples[2] = 0;

                if (hi) {
                    p->amplitude = 1 * (1<<24);
                } else {
                    p->amplitude = -1 * (1<<24);
                }

                if (hi && lo) {
                    p->slope[0]   = 0;
                    p->samples[0] = hi - 1;
                    p->slope[1]   = -2 * (1<<24);
                    p->samples[1] = 1;
                    p->slope[2]   = 0;
                    p->samples[2] = lo - 1;
                } else {
                    p->slope[0]   = 0;
                    p->samples[0] = samples - 1;
                }
                break;
            }

            case SynthWaveformTriangle: {
                const int hi = samples >> 2;
                const int mi = hi << 1;
                const int lo = samples - hi - mi;
                p->stage      = 0;
                p->amplitude  = 0;
                p->slope[0]   = (1 * (1<<24)) / (hi - 1);
                p->samples[0] = hi - 1;
                p->slope[1]   = (-2 * (1<<24)) / (mi - 1);
                p->samples[1] = mi - 1;
                p->slope[2]   = (1 * (1<<24)) / (lo - 1);
                p->samples[2] = lo - 1;
                break;
            }

            case SynthWaveformSaw: {
                const int hi = samples >> 1;
                const int lo = samples - hi;
                p->stage      = 0;
                p->amplitude  = 0;
                p->slope[0]   = (1 * (1<<24)) / (hi - 1);
                p->samples[0] = hi - 1;
                p->slope[1]   = -2 * (1<<24);
                p->samples[1] = 1;
                p->slope[2]   = (1 * (1<<24)) / (lo - 1);
                p->samples[2] = lo - 1;
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
    bool ch_on[NumberOfSynthChannels] = {false};

    for (int i = NumberOfSynthChannels - 1; i >= 0; i--) {
        for (int j = 0; j < NumberOfChannelEnvelopes; j++) {
            if (synth.channels[i].envelopes[j].waveform != SynthWaveformNone) {
                ch_on[i] = true;
                break;
            }
        }
    }

    for (int i = 0; i < samples; i++) {
        int ch_mix = 0;
        for (int j = 0; j < NumberOfSynthChannels; j++) {
            if (ch_on[j]) {
                update_channel(j);
                struct Envelope* e = synth.channels[j].envelopes;
                int w_mix = (((int64_t)e[0].amplitude * e[0].period.amplitude >> 24) *
                             e[0].attack_amplitude >> 24);
                w_mix += (((int64_t)e[1].amplitude * e[1].period.amplitude >> 24) *
                              e[1].attack_amplitude >> 24);
                w_mix += (((int64_t)e[2].amplitude * e[2].period.amplitude >> 24) *
                              e[2].attack_amplitude >> 24);
                w_mix += (((int64_t)e[3].amplitude * e[3].period.amplitude >> 24) *
                              e[3].attack_amplitude >> 24);
                if (w_mix < (-1 * (1<<24))) {
                    w_mix = -1 * (1<<24);
                }
                if (w_mix > (1 << 24) - 1) {
                    w_mix = (1 << 24) - 1;
                }
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
