// This program creates the binary file which gets loaded onto the EEPROM chip for a seven segment display decoder`

#include <fstream>
#include <iostream>
#include <cstdint>

int main() {
	std::ofstream out("clock.bin");

	uint8_t data[8192] = {0};
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
}
