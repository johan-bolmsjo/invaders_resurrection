/* All sound effects.
 */

#include "sfx.h"

#include "libsynth/libsynth.h"


/* Extra life sound.
 */

void
sfx_extra_life(void)
{
    static SynthWave w0 = {SYNTH_WF_TRIANGLE,
                           1000,
                           1,
                           1,
                           0,
                           {25, 50, 0, 25}};

    synth_lock();
    synth_channel_kill(CH_EXTRA_LIFE);
    synth_wave(&w0, CH_EXTRA_LIFE);
    synth_unlock();
}

/* Used when the player shoots.
 */

void
sfx_player_shot(void)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           400,
                           .5,
                           .5,
                           .3,
                           {75, 100, 0, 25}};
    static SynthWave fm = {SYNTH_WF_PULSE,
                           0,
                           .5,
                           .5,
                           1,
                           {50, 150, 0, 0}};
    static SynthWave pwm = {SYNTH_WF_PULSE,
                            0,
                            .25,
                            .5,
                            1,
                            {50, 150, 0, 0}};

    synth_lock();
    synth_channel_kill(CH_PLAYER_SHOT);
    synth_wave(&w0, CH_PLAYER_SHOT);
    synth_fm(&fm, CH_PLAYER_SHOT);
    synth_pwm(&pwm, CH_PLAYER_SHOT);
    synth_unlock();
}

/* Used when the player explodes.
 */

void
sfx_player_explode(void)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           50,
                           .5,
                           .5,
                           .3,
                           {50, 200, 0, 50}};
    static SynthWave fm = {SYNTH_WF_TRIANGLE,
                           5,
                           .5,
                           1,
                           0,
                           {1, 299, 0, 0}};

    synth_lock();
    synth_channel_kill(CH_PLAYER_DIE);
    synth_wave(&w0, CH_PLAYER_DIE);
    synth_fm(&fm, CH_PLAYER_DIE);
    synth_unlock();
}

static void
bomber_move(int channel, int count)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           100,
                           .3,
                           1,
                           .5,
                           {5, 10, 0, 5}};
    static SynthWave w1 = {SYNTH_WF_PULSE,
                           97,
                           .3,
                           1,
                           .5,
                           {5, 10, 0, 5}};
    static SynthWave w2 = {SYNTH_WF_PULSE,
                           96,
                           .3,
                           1,
                           .5,
                           {5, 10, 0, 5}};
    static SynthWave w3 = {SYNTH_WF_PULSE,
                           95,
                           .3,
                           1,
                           .5,
                           {5, 10, 0, 5}};
    static SynthWave fm = {SYNTH_WF_PULSE,
                           1,
                           .05,
                           1,
                           1,
                           {1, 30, 0, 0}};
    static SynthWave* w[4] = {&w0, &w1, &w2, &w3};

    synth_lock();
    synth_channel_kill(channel);
    synth_wave(w[count], channel);
    synth_fm(&fm, channel);
    synth_unlock();
}

/* Used when the bombers move.
 */

void
sfx_bomber_move(void)
{
    static int count = 0;

    if (!synth_waves_on_channel(CH_BOMBER_MOVE))
        bomber_move(CH_BOMBER_MOVE, count);
    else if (!synth_waves_on_channel(CH_BOMBER_DIE))
        bomber_move(CH_BOMBER_DIE, count);

    count++;
    if (count > 3)
        count = 0;
}

/* Used when the bomber explodes.
 */

void
sfx_bomber_explode(void)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           50,
                           .5,
                           .5,
                           .3,
                           {50, 200, 0, 50}};
    static SynthWave fm = {SYNTH_WF_TRIANGLE,
                           5,
                           .5,
                           1,
                           0,
                           {1, 299, 0, 0}};

    synth_lock();
    synth_channel_kill(CH_BOMBER_DIE);
    synth_wave(&w0, CH_BOMBER_DIE);
    synth_fm(&fm, CH_BOMBER_DIE);
    synth_unlock();
}

/* Used while the mystery moves.
 */

void
sfx_mystery_move(void)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           200,
                           .5,
                           .5,
                           .3,
                           {100, 10000, 0, 100}};
    static SynthWave fm = {SYNTH_WF_TRIANGLE,
                           5,
                           .5,
                           1,
                           0,
                           {1, 10199, 0, 0}};

    synth_lock();
    synth_channel_kill(CH_UFO_MOVE);
    synth_wave(&w0, CH_UFO_MOVE);
    synth_fm(&fm, CH_UFO_MOVE);
    synth_unlock();
}

/* Used when the mystery explodes.
 */

void
sfx_mystery_explode(void)
{
    static SynthWave w0 = {SYNTH_WF_PULSE,
                           300,
                           .5,
                           .5,
                           .3,
                           {50, 350, 0, 0}};
    static SynthWave fm = {SYNTH_WF_TRIANGLE,
                           10,
                           1,
                           1,
                           0,
                           {400, 0, 0, 0}};

    synth_lock();
    synth_channel_kill(CH_UFO_DIE);
    synth_wave(&w0, CH_UFO_DIE);
    synth_fm(&fm, CH_UFO_DIE);
    synth_unlock();
}
