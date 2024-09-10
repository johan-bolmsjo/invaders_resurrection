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

#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
    int amplitude;                     // [-1, 1] — Current sample amplitude (8:24 fix-point)
    int slope[NumberOfPeriodStages];   // Period stage slopes (8:24 fix-point)
    int samples[NumberOfPeriodStages]; // Period stage lengths in samples
};

struct Envelope {
    enum SynthWaveform waveform;
    int freq;             // Hz (24:8 fix-point)
    int attack_amplitude; // [0, 1] — (8:24 fix-point)
    int pulse_width;      // [0, 1] — High pulse width (8:24 fix-point)

    // Frequency fraction to keep the right frequency, updated every period (8:24 fix-point).
    int freq_adjust;

    // ADSR parameters
    enum EnvelopeStage stage;
    int amplitude;                       // [0, 1] — Current sample amplitude (8:24 fix-point)
    int slope[NumberOfEnvelopeStages];   // Envelope stage slopes (8:24 fix-point)
    int samples[NumberOfEnvelopeStages]; // Envelope stage lengths in samples

    // Data for the current sound period.
    struct Period period;
};

struct Channel {
    struct Envelope envelopes[NumberOfChannelEnvelopes];
};

static struct {
    int                  bit_noise_count;
    uint64_t             bit_noise_data;
    struct prng64_state* prng_state;
    struct Channel       channels[NumberOfSynthChannels];
} synth_module;
#define M synth_module

void synth_module_init(struct prng64_state* prng_state)
{
    M.prng_state = prng_state;
    M.bit_noise_count = 0;
}

// Get samples per millisecond in 24:8 fix-point.
static inline int
samples_per_millisecond(void)
{
    return (SynthFrequency << 8) / 1000;
}

// Maps an external SynthEnvelope to an internal Envelope.
static void
map_envelope(const struct SynthEnvelope* src, struct Envelope* dst)
{
    dst->waveform = src->waveform;
    dst->freq = src->freq * (1 << 8);
    dst->attack_amplitude = src->attack_amplitude * (1 << 24);
    dst->pulse_width = src->pulse_width * (1 << 24);
    dst->freq_adjust = 0;

    // ADSR
    dst->stage = Attack;
    dst->amplitude = 0;
    for (int i = 0; i < 4; i++) {
        dst->samples[i] = (src->adsr_ms[i] * samples_per_millisecond()) >> 8;
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
        struct Envelope* e = &M.channels[ch].envelopes[ei];
        if (e->waveform == SynthWaveformNone) {
            continue;
        }

        // TODO(jb): BUG: One sample ahead!!
        //           (Future me has no idea what this means)
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
            if (p->samples[p->stage] > 0) {
                p->amplitude += p->slope[p->stage];
                p->samples[p->stage]--;
                break;
            }
        }

        if (p->stage == NumberOfPeriodStages) {
            // Period has played out. Calculate next period parameters.

            int freq; // Hz (24:8 fix-point)
            if (ei < NumberOfRegularChannelEnvelopes) {
                if (!fm_pwm_init) {
                    struct Envelope* e2 = &M.channels[ch].envelopes[FMModifierChannelEnvelopeIndex];
                    fm = ((int64_t)e2->amplitude * e2->period.amplitude >> 24) * e2->attack_amplitude >> 24;
                    e2 = &M.channels[ch].envelopes[PWModifierChannelEnvelopeIndex];
                    pwm = ((int64_t)e2->amplitude * e2->period.amplitude >> 24) * e2->attack_amplitude >> 24;
                    fm_pwm_init = true;
                }
                freq = e->freq + (int)(e->freq * (int64_t)fm >> 24);
            } else {
                freq = e->freq;
            }

            if (!freq) {
                freq = 1;
            }

            int samples = (SynthFrequency << 8) / freq;
            if (samples < 6) {
                samples = 6;
            }
            e->freq_adjust += (((SynthFrequency << 8) / freq) - samples);

            if (e->freq_adjust >= (1 << 24)) {
                e->freq_adjust -= (1 << 24);
                samples++;
            }

            switch (e->waveform) {
            case SynthWaveformPulse: {
                int pw = e->pulse_width + pwm; // (8:24 fix-point)
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

static inline uint16_t
bit_noise(void)
{
    if (M.bit_noise_count == 0) {
        M.bit_noise_count = 64;
        M.bit_noise_data = prng64_next(M.prng_state);
    }
    M.bit_noise_count--;
    int16_t noise = M.bit_noise_data & 1;
    M.bit_noise_data >>= 1;
    return noise;
}

void
synth_mix(int16_t* buffer, int n_samples)
{
    bool ch_on[NumberOfSynthChannels] = {false};

    for (int ch = 0; ch < NumberOfSynthChannels; ch++) {
        for (int ei = 0; ei < NumberOfChannelEnvelopes; ei++) {
            if (M.channels[ch].envelopes[ei].waveform != SynthWaveformNone) {
                ch_on[ch] = true;
                break;
            }
        }
    }

    for (int si = 0; si < n_samples; si++) {
        int ch_mix = 0;
        for (int ch = 0; ch < NumberOfSynthChannels; ch++) {
            if (ch_on[ch]) {
                update_channel(ch);
                struct Envelope* e = M.channels[ch].envelopes;
                int e_mix = (((int64_t)e[0].amplitude * e[0].period.amplitude >> 24) *
                             e[0].attack_amplitude >> 24);
                e_mix += (((int64_t)e[1].amplitude * e[1].period.amplitude >> 24) *
                              e[1].attack_amplitude >> 24);
                e_mix += (((int64_t)e[2].amplitude * e[2].period.amplitude >> 24) *
                              e[2].attack_amplitude >> 24);
                e_mix += (((int64_t)e[3].amplitude * e[3].period.amplitude >> 24) *
                              e[3].attack_amplitude >> 24);
                if (e_mix < (-1 * (1<<24))) {
                    e_mix = -1 * (1<<24);
                }
                if (e_mix > (1 << 24) - 1) {
                    e_mix = (1 << 24) - 1;
                }
                ch_mix += e_mix;
            }
        }

        const int16_t sample = ch_mix >> 10;

        // Mix in some inaudible noise to prevent pulse audio from stop playing
        // the game audio stream. It seems that pulse audio stops playing audio
        // if the stream is silent (all zeroes) for a short while. In our case,
        // the condition is triggered with a close to full invaders armada that
        // moves slowly, and thus generates tick sounds below the threshold
        // frequency.
        //
        // It took some hours to figure this one out :-/
        //
        buffer[si] = sample | bit_noise();
    }
}

void
synth_envelope(const struct SynthEnvelope* sw, int ch)
{
    if (ch < NumberOfSynthChannels) {
        int ei;
        for (ei = 0; ei < NumberOfRegularChannelEnvelopes; ei++) {
            if (M.channels[ch].envelopes[ei].waveform == SynthWaveformNone) {
                break;
            }
        }
        if (ei < NumberOfRegularChannelEnvelopes) {
            map_envelope(sw, &M.channels[ch].envelopes[ei]);
        }
    }
}

void
synth_envelope_fm(const struct SynthEnvelope* sw, int ch)
{
    if (ch < NumberOfSynthChannels) {
        map_envelope(sw, &M.channels[ch].envelopes[FMModifierChannelEnvelopeIndex]);
    }
}

void
synth_envelope_pwm(const struct SynthEnvelope* sw, int ch)
{
    if (ch < NumberOfSynthChannels) {
        map_envelope(sw, &M.channels[ch].envelopes[PWModifierChannelEnvelopeIndex]);
    }
}

void
synth_channel_kill(int ch)
{
    if (ch < NumberOfSynthChannels) {
        for (int ei = 0; ei < NumberOfChannelEnvelopes; ei++) {
            M.channels[ch].envelopes[ei].waveform = SynthWaveformNone;
            M.channels[ch].envelopes[ei].attack_amplitude = 0;
        }
    }
}

int
synth_channel_envelopes(int ch)
{
    int count = 0;
    if (ch < NumberOfSynthChannels) {
        for (int ei = 0; ei < NumberOfRegularChannelEnvelopes; ei++) {
            if (M.channels[ch].envelopes[ei].waveform != SynthWaveformNone) {
                count++;
            }
        }
    }
    return count;
}
