#define NUM_WAVES       4 /* Normal waves per channel */
#define NUM_WAVES_MOD   2 /* Modifier waves per channel */
#define NUM_WAVES_TOTAL (NUM_WAVES + NUM_WAVES_MOD)

#define WAVE_FREQ_MOD 4 /* Indexes */
#define WAVE_PW_MOD   5

#define ATTACK  0
#define DECAY   1
#define SUSTAIN 2
#define RELEASE 3

/* Internal wave representation.
 */

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

static void mix_samples(uint16_t* buffer, int samples);
