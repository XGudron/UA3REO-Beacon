#include "transmitter.h"

bool TRANSMITTER_NeedChangeBand = false;
uint8_t TRANSMITTER_Band_index = 0;

void TRANSMITTER_Init(void) {
	HAL_Delay(3000);
	MAX2870_Init(FREQUENCIES[TRANSMITTER_Band_index]);
}

void TRANSMITTER_Process(void) {
	CW_Process();
	
	if (TRANSMITTER_NeedChangeBand && !CW_TransmitStatus) {
		TRANSMITTER_NeedChangeBand = false;
		
		TRANSMITTER_Band_index++;
		if(TRANSMITTER_Band_index >= (sizeof(FREQUENCIES) / sizeof(uint64_t))) {
				TRANSMITTER_Band_index = 0;
		}
		
		MAX2870_Init(FREQUENCIES[TRANSMITTER_Band_index]);
	}
	
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, CW_TransmitStatus ? GPIO_PIN_RESET : GPIO_PIN_SET);
	HAL_GPIO_WritePin(MAX_CE_GPIO_Port, MAX_CE_Pin, CW_TransmitStatus ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
