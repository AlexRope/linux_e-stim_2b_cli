/*
 * E-Stim Command Line Tool for 2B
 * Author: AlexRope <https://fetlife.com/users/4887759>
 * */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
        int battery;
        int channel_a;
        int channel_b;
        int duty_cycle;
        int feeling;
        int mode;
        char power;
        int joined;
        char version[5];
} Status;

typedef enum emode { MODE_PULSE, MODE_BOUNCE, MODE_CONTINUOUS, MODE_ASPLIT, MODE_BSPLIT, MODE_WAVE, MODE_WATERFALL, MODE_SQUEEZE, MODE_MILK, MODE_THROB, MODE_THRUST, MODE_RANDOM, MODE_STEP, MODE_TRAINING, MODE_MICROPHONE, MODE_STEREO, MODE_TICKLE } emode_t;
typedef enum channel { CHANNEL_A, CHANNEL_B } channel_t;
typedef enum power { POWER_LOW, POWER_HIGH } power_t;
typedef enum joined { CHANNELS_UNLINKED, CHANNELS_LINKED } joined_t;
