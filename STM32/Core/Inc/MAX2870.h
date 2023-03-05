#ifndef MAX2870_H
#define MAX2870_H

#include "main.h"
#include "config.h"

#define MAX2870_REFIN_MAX   200000000UL   ///< Maximum Reference Frequency
#define MAX2870_REFIN_MIN   10000000UL   ///< Minimum Reference Frequency
#define MAX2870_PFD_MAX   105000000UL      ///< Maximum Frequency for Phase Detector (Integer-N)
#define MAX2870_PFD_MAX_FRAC   50000000UL  ///< Maximum Frequency for Phase Detector (Fractional-N)
#define MAX2870_PFD_MIN   125000UL        ///< Minimum Frequency for Phase Detector
#define MAX2870_REF_FREQ_DEFAULT 10000000UL  ///< Default Reference Frequency

#define MAX2870_AUX_DIVIDED 0
#define MAX2870_AUX_FUNDAMENTAL 1
#define MAX2870_REF_UNDIVIDED 0
#define MAX2870_REF_HALF 1
#define MAX2870_REF_DOUBLE 2

#define MAX2870_ERROR_NONE 0
#define MAX2870_ERROR_STEP_FREQUENCY_EXCEEDS_PFD 1
#define MAX2870_ERROR_RF_FREQUENCY 2
#define MAX2870_ERROR_POWER_LEVEL 3
#define MAX2870_ERROR_AUX_POWER_LEVEL 4
#define MAX2870_ERROR_AUX_FREQ_DIVIDER 5
#define MAX2870_ERROR_ZERO_PFD_FREQUENCY 6
#define MAX2870_ERROR_MOD_RANGE 7
#define MAX2870_ERROR_FRAC_RANGE 8
#define MAX2870_ERROR_N_RANGE 9
#define MAX2870_ERROR_N_RANGE_FRAC 10
#define MAX2870_ERROR_RF_FREQUENCY_AND_STEP_FREQUENCY_HAS_REMAINDER 11
#define MAX2870_ERROR_PFD_EXCEEDED_WITH_FRACTIONAL_MODE 12
#define MAX2870_ERROR_PRECISION_FREQUENCY_CALCULATION_TIMEOUT 13
#define MAX2870_WARNING_FREQUENCY_ERROR 14
#define MAX2870_ERROR_DOUBLER_EXCEEDED 15
#define MAX2870_ERROR_R_RANGE 16
#define MAX2870_ERROR_REF_FREQUENCY 17
#define MAX2870_ERROR_REF_MULTIPLIER_TYPE 18
#define MAX2870_ERROR_PFD_AND_STEP_FREQUENCY_HAS_REMAINDER 19
#define MAX2870_ERROR_PFD_LIMITS 20

extern bool MAX2870_Init(uint64_t Frequency);
extern bool MAX2870_CalcAndSetFrequency(uint64_t Frequency);
extern void MAX2870_RF_OFF(void);
extern void MAX2870_WriteRegs(void);
extern int MAX2870_SetReference(uint32_t f, uint16_t r, uint8_t ReferenceDivisionType);
extern int  MAX2870_SetFrequency(float64_t Frequency, uint8_t PowerLevel, uint8_t AuxPowerLevel, uint8_t AuxFrequencyDivider, bool PrecisionFrequency, uint32_t MaximumFrequencyError, uint32_t CalculationTimeout);

#endif
