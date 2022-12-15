#ifndef CW_H
#define CW_H

#include "main.h"
#include "config.h"

#define CW_DOT_TO_DASH_RATE 3.0f
#define CW_DOT_LENGTH_MS (1200 / WPM)
#define CW_DASH_LENGTH_MS ((float)CW_DOT_LENGTH_MS * CW_DOT_TO_DASH_RATE)
#define CW_SYMBOL_SPACE_LENGTH_MS CW_DOT_LENGTH_MS
#define CW_CHAR_SPACE_LENGTH_MS (3 * CW_DOT_LENGTH_MS)
#define CW_WORD_SPACE_LENGTH_MS ((7 - 3) * CW_DOT_LENGTH_MS)

extern bool CW_TransmitStatus;

extern void CW_Process(void);

#endif
