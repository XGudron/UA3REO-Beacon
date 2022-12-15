#include "MAX2870.h"

static uint32_t MAX2870_reffreq = MAX2870_REF_FREQ_DEFAULT;
static uint32_t MAX2870_R[6] = {0x007D0000, 0x2000FFF9, 0x18006E42, 0x0000000B, 0x6180B23C, 0x00400005};
//static uint32_t MAX2870_R[6] = {0x007D0000, 0x2000FFF9, 0x00004042, 0x0000000B, 0x6180B23C, 0x00400005};
static uint32_t MAX2870_ChanStep = 100000UL;
static int32_t MAX2870_FrequencyError = 0;

static void MAX2870_WriteRegs(void);
static uint16_t MAX2870_ReadR(void);
static uint8_t MAX2870_ReadRDIV2(void);
static double MAX2870_ReadPFDfreq(void);
static uint8_t MAX2870_ReadRefDoubler(void);
static uint32_t BitFieldManipulation_ReadBF_dword(uint8_t BitStart, uint8_t BitWidth, uint32_t data_source);
static uint32_t BitFieldManipulation_WriteBF_dword(uint8_t BitStart, uint8_t BitWidth, uint32_t data_source, uint32_t data_field);

/////////////////////////////////////////////

void MAX2870_Init(uint64_t Frequency) {
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_SET);
	MAX2870_RF_OFF();
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_RESET);
	
	MAX2870_SetReference(REFERENCE_CLOCK, 1, MAX2870_REF_UNDIVIDED);
	MAX2870_SetFrequency(Frequency, POWER_LEVEL, 0, MAX2870_AUX_FUNDAMENTAL, true, 0, 10000);
	
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_SET);
	MAX2870_WriteRegs();
	while(HAL_GPIO_ReadPin(MAX_LD_GPIO_Port, MAX_LD_Pin) == GPIO_PIN_RESET) {
		HAL_Delay(1);
	}
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_RESET);
}

void MAX2870_RF_OFF(void) {
	MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(5, 1, MAX2870_R[0x04], 0);
	MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(3, 2, MAX2870_R[0x04], 0);
	MAX2870_WriteRegs();
}

int MAX2870_SetReference(uint32_t f, uint16_t r, uint8_t ReferenceDivisionType)
{
  if (f > 30000000UL && ReferenceDivisionType == MAX2870_REF_DOUBLE) return MAX2870_ERROR_DOUBLER_EXCEEDED;
  if (r > 1023 || r < 1) return MAX2870_ERROR_R_RANGE;
  if (f < MAX2870_REFIN_MIN || f > MAX2870_REFIN_MAX) return MAX2870_ERROR_REF_FREQUENCY;
  if (ReferenceDivisionType != MAX2870_REF_UNDIVIDED && ReferenceDivisionType != MAX2870_REF_HALF && ReferenceDivisionType != MAX2870_REF_DOUBLE) return MAX2870_ERROR_REF_MULTIPLIER_TYPE;

  double ReferenceFactor = 1;
  if (ReferenceDivisionType == MAX2870_REF_HALF) {
    ReferenceFactor /= 2;
  }
  else if (ReferenceDivisionType == MAX2870_REF_DOUBLE) {
    ReferenceFactor *= 2;
  }
  double newfreq  =  (double) f  * ( (double) ReferenceFactor / (double) r);  // check the loop freq

  if ( newfreq > MAX2870_PFD_MAX || newfreq < MAX2870_PFD_MIN ) return MAX2870_ERROR_PFD_LIMITS;

  MAX2870_reffreq = f ;
  MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(14, 10, MAX2870_R[0x02], r);
  if (ReferenceDivisionType == MAX2870_REF_DOUBLE) {
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(24, 2, MAX2870_R[0x02], 2);
  }
  else if (ReferenceDivisionType == MAX2870_REF_HALF) {
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(24, 2, MAX2870_R[0x02], 1);
  }
  else {
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(24, 2, MAX2870_R[0x02], 0);
  }
  return MAX2870_ERROR_NONE;
}

int  MAX2870_SetFrequency(float64_t Frequency, uint8_t PowerLevel, uint8_t AuxPowerLevel, uint8_t AuxFrequencyDivider, bool PrecisionFrequency, uint32_t MaximumFrequencyError, uint32_t CalculationTimeout) {
  //  calculate settings from freq
  if (PowerLevel > 4) return MAX2870_ERROR_POWER_LEVEL;
  if (AuxPowerLevel > 4) return MAX2870_ERROR_AUX_POWER_LEVEL;
  if (AuxFrequencyDivider != MAX2870_AUX_DIVIDED && AuxFrequencyDivider != MAX2870_AUX_FUNDAMENTAL) return MAX2870_ERROR_AUX_FREQ_DIVIDER;
  if (MAX2870_ReadPFDfreq() == 0) return MAX2870_ERROR_ZERO_PFD_FREQUENCY;

  uint32_t ReferenceFrequency = MAX2870_reffreq;
  ReferenceFrequency /= MAX2870_ReadR();
  if (PrecisionFrequency == false && MAX2870_ChanStep > 1 && (ReferenceFrequency % MAX2870_ChanStep) != 0) {
    return MAX2870_ERROR_PFD_AND_STEP_FREQUENCY_HAS_REMAINDER;
  }

  if (Frequency > 6000000000 || Frequency < 23437500) {
    return MAX2870_ERROR_RF_FREQUENCY;
  }

  float64_t tmpfloat; // will fit a long including sign and terminator

  if (PrecisionFrequency == false && MAX2870_ChanStep > 1) {
    tmpfloat = MAX2870_ChanStep;
    float64_t BN_freq = Frequency;
    // BigNumber has issues with modulus calculation which always results in 0
    BN_freq /= tmpfloat;
    uint32_t ChanSteps = (uint32_t)((uint32_t) BN_freq); // round off the decimal - overflow is not an issue for the MAX2870 frequency range
    tmpfloat = ChanSteps;
    BN_freq -= tmpfloat;
    if (BN_freq != 0) {
      return MAX2870_ERROR_RF_FREQUENCY_AND_STEP_FREQUENCY_HAS_REMAINDER;
    }
  }

  uint64_t BN_localosc_ratio = 3000000000 / Frequency;
  uint8_t localosc_ratio = (uint32_t)((uint32_t) BN_localosc_ratio);
  uint8_t MAX2870_outdiv = 1 ;
  uint8_t MAX2870_RfDivSel = 0 ;
  uint32_t MAX2870_N_Int;
  uint32_t MAX2870_Mod = 2;
  uint32_t MAX2870_Frac = 0;

  tmpfloat = MAX2870_reffreq;
  uint32_t CurrentR = MAX2870_ReadR();
  uint8_t RDIV2 = MAX2870_ReadRDIV2();
  uint8_t RefDoubler = MAX2870_ReadRefDoubler();
  float64_t BN_MAX2870_PFDFreq = (tmpfloat * (1 * (1 + RefDoubler)) * (1 / (1 + RDIV2))) / CurrentR;
  uint32_t PFDFreq = (uint32_t)((uint32_t) BN_MAX2870_PFDFreq); // used for checking maximum PFD limit under Fractional Mode

  // select the output divider
  if (Frequency > 23437500) {
    while (MAX2870_outdiv <= localosc_ratio && MAX2870_outdiv <= 64) {
      MAX2870_outdiv *= 2;
      MAX2870_RfDivSel++;
    }
  }
  else {
    MAX2870_outdiv = 128;
    MAX2870_RfDivSel = 7;
  }

  bool CalculationTookTooLong = false;
  float64_t BN_MAX2870_N_Int = (Frequency / BN_MAX2870_PFDFreq) * MAX2870_outdiv; // for 4007.5 MHz RF/10 MHz PFD, result is 400.75;
  MAX2870_N_Int = (uint32_t)((uint32_t) BN_MAX2870_N_Int); // round off the decimal
  tmpfloat = MAX2870_N_Int;
  float64_t BN_FrequencyRemainder;
  if (PrecisionFrequency == true) { // frequency is 4007.5 MHz, PFD is 10 MHz and output divider is 2
    uint32_t CalculationTimeStart = HAL_GetTick();
    BN_FrequencyRemainder = ((BN_MAX2870_PFDFreq * tmpfloat) / MAX2870_outdiv) - Frequency; // integer is 4000 MHz, remainder is -7.5 MHz and will be converterd to a positive
    if (BN_FrequencyRemainder < 0) {
      BN_FrequencyRemainder *= -1; // convert to a postivie
    }
    float64_t BN_MAX2870_N_Int_Overflow = (BN_MAX2870_N_Int + 0.00024421); // deal with N having remainder greater than (4094 / 4095) and a frequency within ((PFD - (PFD * (1 / 4095)) / output divider)
    uint32_t MAX2870_N_Int_Overflow = (uint32_t)((uint32_t) BN_MAX2870_N_Int_Overflow);
    if (MAX2870_N_Int_Overflow == MAX2870_N_Int) { // deal with N having remainder greater than (4094 / 4095) and a frequency within ((PFD - (PFD * (1 / 4095)) / output divider)
      MAX2870_FrequencyError = (int32_t)((int32_t) BN_FrequencyRemainder); // initial value should the MOD match loop fail to result in FRAC < MOD
      if (MAX2870_FrequencyError > MaximumFrequencyError) { // use fractional division if out of tolerance
        uint32_t PreviousFrequencyError = MAX2870_FrequencyError;
        for (uint32_t ModToMatch = 2; ModToMatch <= 4095; ModToMatch++) {
          if (CalculationTimeout > 0) {
            uint32_t CalculationTime = HAL_GetTick();
            CalculationTime -= CalculationTimeStart;
            if (CalculationTime > CalculationTimeout) {
              CalculationTookTooLong = true;
              break;
            }
          }
          float64_t BN_ModFrequencyStep = BN_MAX2870_PFDFreq / ModToMatch / MAX2870_outdiv; // For 4007.5 MHz RF/10 MHz PFD, should be 4
          float64_t BN_TempFrac = (BN_FrequencyRemainder / BN_ModFrequencyStep) + 0.5; // result should be 3 to correspond with above line
          uint32_t TempFrac = (uint32_t)((uint32_t) BN_TempFrac);
          if (TempFrac <= ModToMatch) { // FRAC must be < MOD
            if (TempFrac == ModToMatch) { // FRAC must be < MOD
              TempFrac--;
            }
            tmpfloat = TempFrac;
            float64_t BN_FrequencyError = (BN_FrequencyRemainder - (tmpfloat * BN_ModFrequencyStep));
            if (BN_FrequencyError < 0) {
              BN_FrequencyError *= -1; // convert to a postivie
            }
            MAX2870_FrequencyError = (int32_t)((int32_t) BN_FrequencyError);
            if (MAX2870_FrequencyError < PreviousFrequencyError) {
              PreviousFrequencyError = MAX2870_FrequencyError;
              MAX2870_Mod = ModToMatch; // result should be 4 for 4007.5 MHz/10 MHz PFD
              MAX2870_Frac = TempFrac; // result should be 3 to correspond with above line
            }
            if (MAX2870_FrequencyError <= MaximumFrequencyError) { // tolerance has been obtained - for 4007.5 MHz, MOD = 4, FRAC = 3; error = 0
              break;
            }
          }
        }
      }
    }
    else {
      MAX2870_N_Int++;
    }
    tmpfloat = MAX2870_N_Int;
  }
  else {
    BN_MAX2870_N_Int = (((Frequency * MAX2870_outdiv)) / BN_MAX2870_PFDFreq);
    MAX2870_N_Int = (uint32_t)((uint32_t) BN_MAX2870_N_Int);
    tmpfloat = MAX2870_ChanStep;
    float64_t BN_MAX2870_Mod = (BN_MAX2870_PFDFreq / (tmpfloat / MAX2870_outdiv));
    tmpfloat = MAX2870_N_Int;
    float64_t BN_MAX2870_Frac = (((BN_MAX2870_N_Int - tmpfloat) * BN_MAX2870_Mod) + 0.5);
    // for a maximum 105 MHz PFD and a 128 RF divider with frequency steps no smaller than 1 Hz, maximum results for each is 13.44 * (10 ^ 9) but can be divided by the RF division ratio without error (results will be no larger than 105 * (10 ^ 6))
    BN_MAX2870_Frac /= MAX2870_outdiv;
    BN_MAX2870_Mod /= MAX2870_outdiv;

    // calculate the GCD - Mod2/Frac2 values are temporary
    uint32_t GCD_MAX2870_Mod2 = (uint32_t)((uint32_t) BN_MAX2870_Mod);
    uint32_t GCD_MAX2870_Frac2 = (uint32_t)((uint32_t) BN_MAX2870_Frac);
    uint32_t GCD_t;
    while (true) {
      if (GCD_MAX2870_Mod2 == 0) {
        GCD_t = GCD_MAX2870_Frac2;
        break;
      }
      if (GCD_MAX2870_Frac2 == 0) {
        GCD_t = GCD_MAX2870_Mod2;
        break;
      }
      if (GCD_MAX2870_Mod2 == GCD_MAX2870_Frac2) {
        GCD_t = GCD_MAX2870_Mod2;
        break;
      }
      if (GCD_MAX2870_Mod2 > GCD_MAX2870_Frac2) {
        GCD_MAX2870_Mod2 -= GCD_MAX2870_Frac2;
      }
      else {
        GCD_MAX2870_Frac2 -= GCD_MAX2870_Mod2;
      }
    }
    // restore the original Mod2/Frac2 temporary values before dividing by GCD
    GCD_MAX2870_Mod2 = (uint32_t)((uint32_t) BN_MAX2870_Mod);
    GCD_MAX2870_Frac2 = (uint32_t)((uint32_t) BN_MAX2870_Frac);
    GCD_MAX2870_Mod2 /= GCD_t;
    GCD_MAX2870_Frac2 /= GCD_t;
    if (GCD_MAX2870_Mod2 > 4095) { // outside valid range
      while (true) {
        GCD_MAX2870_Mod2 /= 2;
        GCD_MAX2870_Frac2 /= 2;
        if (GCD_MAX2870_Mod2 <= 4095) { // now within valid range
          if (GCD_MAX2870_Frac2 == GCD_MAX2870_Mod2) { // FRAC must be less than MOD
            GCD_MAX2870_Frac2--;
          }
          break;
        }
      }
    }
    // set the final FRAC/MOD values
    MAX2870_Frac = GCD_MAX2870_Frac2;
    MAX2870_Mod = GCD_MAX2870_Mod2;
  }
  if (CalculationTookTooLong == true) {
    return MAX2870_ERROR_PRECISION_FREQUENCY_CALCULATION_TIMEOUT;
  }

  BN_FrequencyRemainder = (((((BN_MAX2870_PFDFreq * tmpfloat) + (MAX2870_Frac * (BN_MAX2870_PFDFreq / MAX2870_Mod))) / MAX2870_outdiv)) - Frequency) + 0.5; // no issue with divide by 0 regarding MOD (set to 2 by default) and FRAC (set to 0 by default) - maximum is PFD maximum frequency of 105 MHz under integer mode - no issues with signed overflow or underflow
  MAX2870_FrequencyError = (int32_t)((int32_t) BN_FrequencyRemainder);

  if (MAX2870_Frac == 0) { // correct the MOD to the minimum required value
    MAX2870_Mod = 2;
  }

  if ( MAX2870_Mod < 2 || MAX2870_Mod > 4095) {
    return MAX2870_ERROR_MOD_RANGE;
  }

  if ( (uint32_t) MAX2870_Frac > (MAX2870_Mod - 1) ) {
    return MAX2870_ERROR_FRAC_RANGE;
  }

  if (MAX2870_Frac == 0 && (MAX2870_N_Int < 16  || MAX2870_N_Int > 65535)) {
    return MAX2870_ERROR_N_RANGE;
  }

  if (MAX2870_Frac != 0 && (MAX2870_N_Int < 19  || MAX2870_N_Int > 4091)) {
    return MAX2870_ERROR_N_RANGE_FRAC;
  }

  if (MAX2870_Frac != 0 && PFDFreq > MAX2870_PFD_MAX_FRAC) {
    return MAX2870_ERROR_PFD_EXCEEDED_WITH_FRACTIONAL_MODE;
  }

  MAX2870_R[0x00] = BitFieldManipulation_WriteBF_dword(3, 12, MAX2870_R[0x00], MAX2870_Frac);
  MAX2870_R[0x00] = BitFieldManipulation_WriteBF_dword(15, 16, MAX2870_R[0x00], MAX2870_N_Int);

  if (MAX2870_Frac == 0) {
    MAX2870_R[0x00] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x00], 1); // integer-n mode
    MAX2870_R[0x01] = BitFieldManipulation_WriteBF_dword(29, 2, MAX2870_R[0x01], 0); // Charge Pump Linearity
    MAX2870_R[0x01] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x01], 1); // Charge Pump Output Clamp
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(8, 1, MAX2870_R[0x02], 1); // Lock Detect Function, int-n mode
    MAX2870_R[0x05] = BitFieldManipulation_WriteBF_dword(24, 1, MAX2870_R[0x05], 1); // integer-n mode
  }
  else {
    MAX2870_R[0x00] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x00], 0); // fractional-n mode
    MAX2870_R[0x01] = BitFieldManipulation_WriteBF_dword(29, 2, MAX2870_R[0x01], 1); // Charge Pump Linearity
    MAX2870_R[0x01] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x01], 0); // Charge Pump Output Clamp
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(8, 1, MAX2870_R[0x02], 0); // Lock Detect Function, frac-n mode
    MAX2870_R[0x05] = BitFieldManipulation_WriteBF_dword(24, 1, MAX2870_R[0x05], 0); // fractional-n mode
  }
  // (0x01, 15, 12, 1) phase
  MAX2870_R[0x01] = BitFieldManipulation_WriteBF_dword(3, 12, MAX2870_R[0x01], MAX2870_Mod);
  // (0x02, 3,1,0) counter reset
  // (0x02, 4,1,0) cp3 state
  // (0x02, 5,1,0) power down
  if (PFDFreq > 32000000UL) { // lock detect speed adjustment
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x02], 1); // Lock Detect Speed
  }
  else  {
    MAX2870_R[0x02] = BitFieldManipulation_WriteBF_dword(31, 1, MAX2870_R[0x02], 0); // Lock Detect Speed
  }
  // (0x02, 13,1,0) dbl buf
  // (0x02, 26,3,0) //  muxout, not used
  // (0x02, 29,2,0) low noise and spurs mode
  // (0x03, 15,2,1) clk div mode
  // (0x03, 17,1,0) reserved
  // (0x03, 18,6,0) reserved
  // (0x03, 24,1,0) VAS response to temperature drift
  // (0x03, 25,1,0) VAS state machine
  // (0x03, 26,6,0) VCO and VCO sub-band manual selection
  if (PowerLevel == 0) {
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(5, 1, MAX2870_R[0x04], 0);
		MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(3, 2, MAX2870_R[0x04], 0);
  }
  else {
    PowerLevel--;
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(5, 1, MAX2870_R[0x04], 1);
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(3, 2, MAX2870_R[0x04], PowerLevel);
  }
  if (AuxPowerLevel == 0) {
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(8, 1, MAX2870_R[0x04], 0);
		MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(6, 2, MAX2870_R[0x04], 0);
  }
  else {
    AuxPowerLevel--;
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(6, 2, MAX2870_R[0x04], AuxPowerLevel);
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(8, 1, MAX2870_R[0x04], 1);
    MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(9, 1, MAX2870_R[0x04], AuxFrequencyDivider);
  }
  // (0x04, 10,1,0) reserved
  // (0x04, 11,1,0) reserved
  // (0x04, 12,8,1) Band Select Clock Divider
  MAX2870_R[0x04] = BitFieldManipulation_WriteBF_dword(20, 3, MAX2870_R[0x04], MAX2870_RfDivSel); // rf divider select
  // (0x04, 23,8,0) reserved
  // (0x04, 24,2,1) Band Select Clock Divider MSBs
  // (0x04, 26,6,1) reserved
  // (0x05, 3,15,0) reserved
  // (0x05, 18,1,0) MUXOUT pin mode
  // (0x05, 22,2,1) lock pin function
  // (0x05, 25,7,0) reserved
  MAX2870_WriteRegs();

  bool NegativeError = false;
  if (MAX2870_FrequencyError < 0) { // convert to a positive for frequency error comparison with a positive value
    MAX2870_FrequencyError ^= 0xFFFFFFFF;
    MAX2870_FrequencyError++;
    NegativeError = true;
  }
  if ((PrecisionFrequency == true && MAX2870_FrequencyError > MaximumFrequencyError) || (PrecisionFrequency == false && MAX2870_FrequencyError != 0)) {
    if (NegativeError == true) { // convert back to negative if changed from negative to positive for frequency error comparison with a positive value
      MAX2870_FrequencyError ^= 0xFFFFFFFF;
      MAX2870_FrequencyError++;
    }
    return MAX2870_WARNING_FREQUENCY_ERROR;
  }

  return MAX2870_ERROR_NONE; // ok
}

static void MAX2870_WriteRegs(void) {
  for (int i = 5 ; i >= 0 ; i--) { // sequence according to the MAX2870 datasheet
		uint8_t txData[4];
		// Move bits around to deal with the little endiness on the STM32
		uint32_t r = MAX2870_R[i];
		txData[0] = r >> 24;
		txData[1] = (r & 0xff0000) >> 16 ;
		txData[2] = (r & 0xff00) >> 8;
		txData[3] = (r & 0xff);

		// Ensure LE high
		HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_SET);
		HAL_Delay(1);
		// Start by bringing LE low
		HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_RESET);

		// Transmit all the bits!
		HAL_SPI_Transmit(&hspi1, (uint8_t *) txData, 4, 50);

		// Once Transfer complete, pull LE high
		while (HAL_SPI_GetState(&hspi1) != HAL_SPI_STATE_READY);
		HAL_GPIO_WritePin(MAX_CS_GPIO_Port, MAX_CS_Pin, GPIO_PIN_SET);
  }
}

static uint16_t MAX2870_ReadR(void) {
  return BitFieldManipulation_ReadBF_dword(14, 10, MAX2870_R[0x02]);
}

static uint8_t MAX2870_ReadRDIV2(void) {
  return BitFieldManipulation_ReadBF_dword(24, 1, MAX2870_R[0x02]);
}

static uint8_t MAX2870_ReadRefDoubler(void) {
  return BitFieldManipulation_ReadBF_dword(25, 1, MAX2870_R[0x02]);
}

static double MAX2870_ReadPFDfreq(void) {
  double value = MAX2870_reffreq;
  uint16_t temp = MAX2870_ReadR();
  if (temp == 0) { // avoid division by zero
    return 0;
  }
  value /= temp;
  if (MAX2870_ReadRDIV2() != 0) {
    value /= 2;
  }
  if (MAX2870_ReadRefDoubler() != 0) {
    value *= 2;
  }
  return value;
}

static uint32_t BitFieldManipulation_ReadBF_dword(uint8_t BitStart, uint8_t BitWidth, uint32_t data_source) {
  data_source >>= BitStart; // if data = 0x103E00 and BitStart = 8, data is now 0x103E
  uint32_t mask = 1;
  mask <<= BitWidth; // 0x40 if BitStart = 6;
  mask--; // now 0x3F - 6 consecutive bits from 0 set
  data_source &= mask; // data is now 0x3E
  return data_source;
}

static uint32_t BitFieldManipulation_WriteBF_dword(uint8_t BitStart, uint8_t BitWidth, uint32_t data_source, uint32_t data_field) {
  uint32_t mask = 1;
  mask <<= BitWidth; // 0x40 if BitStart = 6;
  mask--; // now 0x3F; // now 0x3F - 6 consecutive bits from 0 set
  data_field &= mask; // do not exceed BitWidth
  mask <<= BitStart; // shift to the destination field
  data_source &= (~mask); // clear the destination field for a write
  data_field <<= BitStart; // 0x3F00 if BitStart = 8;
  data_source |= data_field; // write to the destination field
  return data_source;
}
