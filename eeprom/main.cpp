// This program creates the binary file which gets loaded onto the EEPROM chip for a seven segment display decoder`

#include <fstream>
#include <iostream>
#include <cstdint>

uint8_t lookup_7s[10] = {
	// gfedcba
	0b00111111, // 0x0
	0b00000110, // 0x1
	0b01011011, // 0x2
	0b01001111, // 0x3
	0b01100110, // 0x4
	0b01101101, // 0x5
	0b01111101, // 0x6
	0b00000111, // 0x7
	0b01111111, // 0x8
	0b01101111, // 0x9
};

#define TEST_MAPPING

#ifdef TEST_MAPPING
uint8_t bin_data(int addr) {
  	int display_index = addr >> 11;
	int minute = addr & ((1 << 11) - 1);
	int pm = minute >= 720 ? 1 << 7 : 0;
	int display_digit = 0;

	if (minute >= 1440) return 0;

	// Find the display digit
	switch(display_index) {
		case 0: display_digit = minute % 60 % 10; break;
		case 1: display_digit = minute % 60 / 10; break;
		case 2: 
		if (minute % 720 / 60 == 0) display_digit = 2;
		else display_digit = minute % 720 / 60 % 10;
		break;
		case 3: 
		if (minute % 720 / 60 == 0) display_digit = 1;
		else display_digit = minute % 720 / 60 / 10;
		break;
	}

	return display_index == 3 && display_digit == 0 ? 0 + pm : lookup_7s[display_digit] + pm;
}
#endif

int main() {
	std::ofstream out("clock.bin");

	uint8_t data[8192] = {0};

	// Loop for each of the 4 seven segment digit displays
	for (int i = 0; i < 4; i++) {
		int display_index = i << 11;

		// Loop through all minutes of the day
		for (int minute = 0; minute < 1440; minute++) {
			int display_digit = 0;
			int pm = minute >= 720 ? 1 << 7 : 0;

			// Find the display digit
			switch(i) {
				case 0: display_digit = minute % 60 % 10; break;
				case 1: display_digit = minute % 60 / 10; break;
				case 2: 
					if (minute % 720 / 60 == 0) display_digit = 2;
					else display_digit = minute % 720 / 60 % 10;
					break;
				case 3: 
					if (minute % 720 / 60 == 0) display_digit = 1;
					else display_digit = minute % 720 / 60 / 10;
					break;
			}
			
			// Write the correct seven segment encoding for the respective display digit
			data[display_index + minute] = i == 3 && display_digit == 0 ? 0 + pm : lookup_7s[display_digit] + pm;
		}
	}

	out.write(reinterpret_cast<char*>(data), sizeof(data));

#ifdef TEST_MAPPING
	int incorrect_count = 0;
	for (int i = 0; i < 8192; i++) {
		if (data[i] != bin_data(i)) {
			incorrect_count++;
			std::cout << "INCORRECT at " << i << ", " << (int)bin_data(i) << " should be " << (int)data[i] << "\n";
		}
	}	
	std::cout << "INCORRECT COUNT: " << incorrect_count << "\n";
#endif
}
