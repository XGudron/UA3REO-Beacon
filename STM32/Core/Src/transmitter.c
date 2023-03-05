#include "transmitter.h"

bool TRANSMITTER_NeedChangeBand = false;
uint8_t TRANSMITTER_Band_index = 0;
static uint64_t TRANSMITTER_getCurrentFrequency();

void TRANSMITTER_Init(void) {
	HAL_Delay(1000);
	MAX2870_Init(TRANSMITTER_getCurrentFrequency());
}

void TRANSMITTER_Process(void) {
	if (HAL_GPIO_ReadPin(ONLY_DASHES_GPIO_Port, ONLY_DASHES_Pin) == GPIO_PIN_RESET) { // only dashes mode
		TRANSMITTER_NeedChangeBand = true;
		CW_TransmitStatus = false;
	} else { // normal mode
		CW_Process();
	}
	
	if (TRANSMITTER_NeedChangeBand && !CW_TransmitStatus) {
		TRANSMITTER_NeedChangeBand = false;
		
		TRANSMITTER_Band_index++;
		if(TRANSMITTER_Band_index >= (sizeof(FREQUENCIES) / sizeof(uint64_t))) {
				TRANSMITTER_Band_index = 0;
		}
		
		while(!MAX2870_Init(TRANSMITTER_getCurrentFrequency())) {}
		
		// LONG_DASH
		if (LONG_DASH_BEFORE_MESSAGE) {
			uint32_t startTime = HAL_GetTick();
			while((HAL_GetTick() - startTime) < 300) {
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_RESET);
				__WFI();
			}
			
			startTime = HAL_GetTick();
			while((HAL_GetTick() - startTime) < 3000) {
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_SET);
				__WFI();
			}
			
			startTime = HAL_GetTick();
			while((HAL_GetTick() - startTime) < 300) {
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, GPIO_PIN_RESET);
				__WFI();
			}
		}
	}
	
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, CW_TransmitStatus ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, CW_TransmitStatus ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint64_t TRANSMITTER_getCurrentFrequency() {
	if (HAL_GPIO_ReadPin(BAND_B11_GPIO_Port, BAND_B11_Pin) == GPIO_PIN_RESET) {
		return FREQUENCY_B11;
	}
	if (HAL_GPIO_ReadPin(BAND_B10_GPIO_Port, BAND_B10_Pin) == GPIO_PIN_RESET) {
		return FREQUENCY_B10;
	}
	if (HAL_GPIO_ReadPin(BAND_B1_GPIO_Port, BAND_B1_Pin) == GPIO_PIN_RESET) {
		return FREQUENCY_B1;
	}
	if (HAL_GPIO_ReadPin(BAND_B0_GPIO_Port, BAND_B0_Pin) == GPIO_PIN_RESET) {
		return FREQUENCY_B0;
	}
	if (HAL_GPIO_ReadPin(BAND_A2_GPIO_Port, BAND_A2_Pin) == GPIO_PIN_RESET) {
		return FREQUENCY_A2;
	}
	
	return FREQUENCIES[TRANSMITTER_Band_index];
}
