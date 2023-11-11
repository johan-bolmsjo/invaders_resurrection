#pragma once

enum {
    NumberOfSynthChannels = 4
};

enum SynthWaveform {
    SynthWaveformNone,
    SynthWaveformPulse,
    SynthWaveformTriangle,
    SynthWaveformSaw,
    NumberOfSynthWaveforms,
};

// TODO wave -> envelope

struct SynthEnvelope {
    enum SynthWaveform waveform;
    float hz;
    float attack_amplitude; // [0.0, 1.0]
    float sustain_ratio;    // [0.0, 1.0] — Sustain ratio of attack amplitude
    float pulse_width;      // [0.0, 1.0] — High pulse width
    int   adsr_ms[4];       // Attack, decay, sustain and release times in milliseconds
};

// Open synth.
// Returns 0 on success and -1 on failure. Even if this function fails,
// it's okay to call the other functions. They will just don't do anything.
int synth_open(void);

// Close synth
void synth_close(void);

void synth_update(void);

// Play envelope on channel as a regular sound.
// A channel can play four regular envelopes.
void synth_envelope(struct SynthEnvelope* env, int ch);

// Frequency modulate all regular envelopes on channel with envelope.
void synth_envelope_fm(struct SynthEnvelope* env, int ch);

// Pulse width modulate all regular envelopes on channel with envelope.
void synth_envelope_pwm(struct SynthEnvelope* env, int ch);

// Kill playback on channel.
void synth_channel_kill(int ch);

// Count the number of regular envelopes playing on channel.
int synth_channel_envelopes(int ch);

// Use before creating a sound effect. SDL needs this.
void synth_lock(void);

// Use after having created a sound effect. SDL needs this.
void synth_unlock(void);
