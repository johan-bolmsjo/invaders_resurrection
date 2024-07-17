#pragma once

#include <stdint.h>
#include "libutil/prng.h"

enum {
    SynthFrequency = 44100,
    NumberOfSynthChannels = 4
};

enum SynthWaveform {
    SynthWaveformNone,
    SynthWaveformPulse,
    SynthWaveformTriangle,
    SynthWaveformSaw,
    NumberOfSynthWaveforms,
};

struct SynthEnvelope {
    enum SynthWaveform waveform;
    float freq;             // Frequency (Hz)
    float attack_amplitude; // [0.0, 1.0]
    float sustain_ratio;    // [0.0, 1.0] — Sustain ratio of attack amplitude
    float pulse_width;      // [0.0, 1.0] — High pulse width
    int   adsr_ms[4];       // Attack, decay, sustain and release times in milliseconds
};

/// Initialize synth.
void synth_module_init(struct prng64_state* prng_state);

/// Generate mono channel 16 bit samples to an audio buffer. Envelopes that runs to
/// completion during mixing stops producing sound. The game loop typically start new
/// envelopes on channels to generate sounds corresponding to game events. Use a small
/// mixing buffer to minimize sound artifacts.
void synth_mix(int16_t* buffer, int n_samples);

/// Play envelope on channel as a regular sound.
/// A channel can play four regular envelopes.
void synth_envelope(const struct SynthEnvelope* env, int ch);

/// Frequency modulate all regular envelopes on channel with envelope.
void synth_envelope_fm(const struct SynthEnvelope* env, int ch);

/// Pulse width modulate all regular envelopes on channel with envelope.
void synth_envelope_pwm(const struct SynthEnvelope* env, int ch);

/// Kill playback on channel.
void synth_channel_kill(int ch);

/// Count the number of regular envelopes playing on channel.
int synth_channel_envelopes(int ch);
