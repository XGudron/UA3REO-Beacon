#include "cw.h"
#include "transmitter.h"

bool CW_TransmitStatus = false;
uint32_t CW_current_message_index = 0;
uint32_t CW_current_symbol_index = 0;
uint64_t CW_wait_until = 0;
uint8_t CW_EncodeStatus = 0; // 0 - wait symbol, 1 - transmit symbol, 2 - wait after char, 3 - wait after word

static char* CW_CharToDots(char chr);

void CW_Process(void) {
	uint64_t current_time = HAL_GetTick();
	if (current_time < CW_wait_until) {
		return;
	}
	
	char *chr = CW_CharToDots(*(CW_MESSAGE + CW_current_message_index));
	char symbol = chr[CW_current_symbol_index];
	
	if (CW_EncodeStatus == 1) {
		CW_current_symbol_index++;
		
		if (chr[CW_current_symbol_index] == 0) { // end of char
			CW_current_symbol_index = 0;
			CW_current_message_index++;
			
			CW_wait_until = current_time + CW_CHAR_SPACE_LENGTH_MS;
			
			if(CW_MESSAGE[CW_current_message_index] == 0) {
				CW_wait_until = current_time + CW_WORD_SPACE_LENGTH_MS;
				CW_current_message_index = 0;
				TRANSMITTER_NeedChangeBand = true;
			}
		} else {
			CW_wait_until = current_time + CW_SYMBOL_SPACE_LENGTH_MS;
		}
		
		CW_EncodeStatus = 0;
		CW_TransmitStatus = false;
		return;
	}
	
	if (symbol == '.') {
		CW_wait_until = current_time + CW_DOT_LENGTH_MS;
		CW_TransmitStatus = true;
	}
	
	if (symbol == '-') {
		CW_wait_until = current_time + CW_DASH_LENGTH_MS;
		CW_TransmitStatus = true;
	}
	
	if (symbol == ' ') {
		CW_wait_until = current_time + CW_WORD_SPACE_LENGTH_MS;
		CW_TransmitStatus = false;
	}
	
	if (symbol == 0) {
		CW_TransmitStatus = false;
	}
	
	CW_EncodeStatus = 1;
}

static char* CW_CharToDots(char chr) {
	if (chr == ' ') return " ";
	if (chr == 'A') return ".-";
	if (chr == 'B') return "-...";
	if (chr == 'C') return "-.-.";
	if (chr == 'D') return "-..";
	if (chr == 'E') return ".";
	if (chr == 'F') return "..-.";
	if (chr == 'G') return "--.";
	if (chr == 'H') return "....";
	if (chr == 'I') return "..";
	if (chr == 'J') return ".---";
	if (chr == 'K') return "-.-";
	if (chr == 'L') return ".-..";
	if (chr == 'M') return "--";
	if (chr == 'N') return "-.";
	if (chr == 'O') return "---";
	if (chr == 'P') return ".--.";
	if (chr == 'Q') return "--.-";
	if (chr == 'R') return ".-.";
	if (chr == 'S') return "...";
	if (chr == 'T') return "-";
	if (chr == 'U') return "..-";
	if (chr == 'V') return "...-";
	if (chr == 'W') return ".--";
	if (chr == 'X') return "-..-";
	if (chr == 'Y') return "-.--";
	if (chr == 'Z') return "--..";
	if (chr == '1') return ".----";
	if (chr == '2') return "..---";
	if (chr == '3') return "...--";
	if (chr == '4') return "....-";
	if (chr == '5') return ".....";
	if (chr == '6') return "-....";
	if (chr == '7') return "--...";
	if (chr == '8') return "---..";
	if (chr == '9') return "----.";
	if (chr == '0') return "-----";
	if (chr == '?') return "..--..";
	if (chr == '.') return ".-.-.-";
	if (chr == ',') return "--..--";
	if (chr == '!') return "-.-.--";
	if (chr == '@') return ".--.-.";
	if (chr == ':') return "---...";
	if (chr == '-') return "-....-";
	if (chr == '/') return "-..-.";
	if (chr == '(') return "-.--.";
	if (chr == ')') return "-.--.-";
	if (chr == '_') return ".-...";
	if (chr == '$') return "...-..-";
	if (chr == '>') return "...-.-";
	if (chr == '<') return ".-.-.";
	if (chr == '~') return "...-.";
	
	return " ";
}
