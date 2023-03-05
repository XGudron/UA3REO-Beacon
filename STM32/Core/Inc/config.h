#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"

#define WPM 25
#define POWER_LEVEL 4 // 1 (-4 dBm), 2 (-1 dBm), 3 (+2 dBm), 4(+5 dBm)
#define REFERENCE_CLOCK 10000000 // 100000000 / 25000000 default XO, 10000000 OCXO

#define LONG_DASH_BEFORE_MESSAGE true // true/false long signal (3sec) before start transmit message

#define FREQ_12M 24930000
#define FREQ_10M 28200000
#define FREQ_2M 144445500
#define FREQ_70CM 432450000
#define FREQ_23CM 1296900000
#define FREQ_6CM 5760895000
#define FREQ_3CM 3456290000 // (10ghz (10.368.870.000) band from 3x harmonics

static uint64_t FREQUENCIES[] = {FREQ_2M, FREQ_70CM, FREQ_23CM, FREQ_6CM, FREQ_3CM}; // default bands sequence

static uint64_t FREQUENCY_B11 = FREQ_2M; // single band, if B11 connected to ground
static uint64_t FREQUENCY_B10 = FREQ_70CM; // single band, if B10 connected to ground
static uint64_t FREQUENCY_B1 = FREQ_23CM; // single band, if B1 connected to ground
static uint64_t FREQUENCY_B0 = FREQ_6CM; // single band, if B0 connected to ground
static uint64_t FREQUENCY_A2 = FREQ_3CM; // single band, if A2 connected to ground

// A0 - connect to ground, if no CW message required (only dashes)

static char CW_MESSAGE[] = "CQ CQ BEACON 73";

#endif
