/* Waveforms
 */

#define SYNTH_WF_NONE      0
#define SYNTH_WF_PULSE     1
#define SYNTH_WF_TRIANGLE  2
#define SYNTH_WF_SAW       3

#define SYNTH_NUM_CHANNELS  4   /* Number of channels */


/* All components of a wave.
 */

typedef struct _SynthWave
{
  int wf;         /* Waveform */
  float f;        /* Frequency (Hz) */
  float aa;       /* Attack amplitude (0.0 to 1.0) */
  float sl;       /* Sustain level as parts of aa (0.0 to 1.0) */
  float pw;       /* High pulse width (0.0 to 1.0) */
  int adsr[4];    /* Attack, decay, sustain and release times (ms) */
} SynthWave;


int
synth_open ();

void
synth_close ();

void
synth_update ();

void
synth_wave (SynthWave *sw, int ch);

void
synth_fm (SynthWave *sw, int ch);

void
synth_pwm (SynthWave *sw, int ch);

void
synth_channel_kill (int ch);

int
synth_waves_on_channel (int ch);

void
synth_lock ();

void
synth_unlock ();
