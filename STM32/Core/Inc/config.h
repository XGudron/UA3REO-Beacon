#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"

#define WPM 25
#define POWER_LEVEL 4 // 1 (-4 dBm), 2 (-1 dBm), 3 (+2 dBm), 4(+5 dBm)
#define REFERENCE_CLOCK 10000000 // 25000000 default XO, 10000000 OCXO

#define LONG_DASH_BEFORE_MESSAGE true // true/false long signal (3sec) before start transmit message

//static uint64_t FREQUENCIES[] = {24930, 28200, 144445500, 432450000, 1296900000, 5760895000, 3456290000}; // 12m, 10m, 2m, 70cm, 23cm, 6cm, 3cm (10ghz band from 3x harmonics)
static uint64_t FREQUENCIES[] = {144445500, 432450000, 1296900000}; // 2m, 70cm, 23cm
//static uint64_t FREQUENCIES[] = {144445500, 432450000, 1296900000, 5760895000, 3456290000}; // 2m, 70cm, 23cm, 6cm, 3cm (10ghz band from 3x harmonics)
//static uint64_t FREQUENCIES[] = {144445500}; // single band
static char CW_MESSAGE[] = "CQ CQ BEACON 73";

#endif
