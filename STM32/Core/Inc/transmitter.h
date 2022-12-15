#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "main.h"
#include "cw.h"
#include "MAX2870.h"

extern bool TRANSMITTER_NeedChangeBand;

extern void TRANSMITTER_Init(void);
extern void TRANSMITTER_Process(void);

#endif
