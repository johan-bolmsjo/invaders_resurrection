#include "sfx.h"

#include "libsynth/libsynth.h"

void
sfx_extra_life(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformTriangle,
        1000,
        1,
        1,
        0,
        {25, 50, 0, 25}
    };

    synth_channel_kill(CH_EXTRA_LIFE);
    synth_envelope(&e0, CH_EXTRA_LIFE);
}

void
sfx_player_shot(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        400,
        .5,
        .5,
        .3,
        {75, 100, 0, 25}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformPulse,
        0,
        .5,
        .5,
        1,
        {50, 150, 0, 0}
    };
    static const struct SynthEnvelope pwm = {
        SynthWaveformPulse,
        0,
        .25,
        .5,
        1,
        {50, 150, 0, 0}
    };

    synth_channel_kill(CH_PLAYER_SHOT);
    synth_envelope(&e0, CH_PLAYER_SHOT);
    synth_envelope_fm(&fm, CH_PLAYER_SHOT);
    synth_envelope_pwm(&pwm, CH_PLAYER_SHOT);
}

void
sfx_player_explode(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        50,
        .5,
        .5,
        .3,
        {50, 200, 0, 50}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformTriangle,
        5,
        .5,
        1,
        0,
        {1, 299, 0, 0}
    };

    synth_channel_kill(CH_PLAYER_DIE);
    synth_envelope(&e0, CH_PLAYER_DIE);
    synth_envelope_fm(&fm, CH_PLAYER_DIE);
}

static void
bomber_move(int channel, int sfx_index)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        100,
        .3,
        1,
        .5,
        {5, 10, 0, 5}
    };
    static const struct SynthEnvelope e1 = {
        SynthWaveformPulse,
        97,
        .3,
        1,
        .5,
        {5, 10, 0, 5}
    };
    static const struct SynthEnvelope e2 = {
        SynthWaveformPulse,
        96,
        .3,
        1,
        .5,
        {5, 10, 0, 5}
    };
    static const struct SynthEnvelope e3 = {
        SynthWaveformPulse,
        95,
        .3,
        1,
        .5,
        {5, 10, 0, 5}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformPulse,
        1,
        .05,
        1,
        1,
        {1, 30, 0, 0}
    };
    static const struct SynthEnvelope* w[4] = {&e0, &e1, &e2, &e3};

    synth_channel_kill(channel);
    synth_envelope(w[sfx_index], channel);
    synth_envelope_fm(&fm, channel);
}

/* Used when the bombers move.
 */

void
sfx_bomber_move(void)
{
    static int sfx_index = 0;

    // Play sound on available channel to avoid cut of sound effects.
    if (!synth_channel_envelopes(CH_BOMBER_MOVE)) {
        bomber_move(CH_BOMBER_MOVE, sfx_index);
    } else if (!synth_channel_envelopes(CH_BOMBER_DIE)) {
        bomber_move(CH_BOMBER_DIE, sfx_index);
    }

    if (++sfx_index > 3) {
        sfx_index = 0;
    }
}

void
sfx_bomber_explode(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        50,
        .5,
        .5,
        .3,
        {50, 200, 0, 50}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformTriangle,
        5,
        .5,
        1,
        0,
        {1, 299, 0, 0}
    };

    synth_channel_kill(CH_BOMBER_DIE);
    synth_envelope(&e0, CH_BOMBER_DIE);
    synth_envelope_fm(&fm, CH_BOMBER_DIE);
}

void
sfx_mystery_move(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        200,
        .2,
        .5,
        .3,
        {100, 10000, 0, 100}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformTriangle,
        5,
        .5,
        1,
        0,
        {1, 10199, 0, 0}
    };

    synth_channel_kill(CH_UFO_MOVE);
    synth_envelope(&e0, CH_UFO_MOVE);
    synth_envelope_fm(&fm, CH_UFO_MOVE);
}

void
sfx_mystery_explode(void)
{
    static const struct SynthEnvelope e0 = {
        SynthWaveformPulse,
        300,
        .5,
        .5,
        .3,
        {50, 350, 0, 0}
    };
    static const struct SynthEnvelope fm = {
        SynthWaveformTriangle,
        10,
        1,
        1,
        0,
        {400, 0, 0, 0}
    };

    synth_channel_kill(CH_UFO_DIE);
    synth_envelope(&e0, CH_UFO_DIE);
    synth_envelope_fm(&fm, CH_UFO_DIE);
}
