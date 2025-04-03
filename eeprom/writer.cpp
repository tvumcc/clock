#include <fstream>
#include <cstdint>

// Lookup Table that maps digits to their seven segment display counterparts
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

// Returns the necessary byte to be written corresponding to the address of the EEPROM
uint8_t bin_data(int addr) {
    int display_index = addr >> 11;
    int minute = addr & ((1 << 11) - 1);
    int pm = minute >= 720 ? 1 << 7 : 0;
    int display_digit = 0;

    if (minute >= 1440)
        return 0;

    // Find the display digit
    switch (display_index) {
    case 0:
        display_digit = minute % 60 % 10;
        break;
    case 1:
        display_digit = minute % 60 / 10;
        break;
    case 2:
        if (minute % 720 / 60 == 0)
            display_digit = 2;
        else
            display_digit = minute % 720 / 60 % 10;
        break;
    case 3:
        if (minute % 720 / 60 == 0)
            display_digit = 1;
        else
            display_digit = minute % 720 / 60 / 10;
        break;
    }

    return display_index == 3 && display_digit == 0 ? 0 + pm : lookup_7s[display_digit] + pm;
}

int main() {
    std::ofstream out("c.bin");
    uint8_t data[8192] = {0};

    for (int i = 0; i < 8192; i++) {
        data[i] = bin_data(i);
    }

	out.write(reinterpret_cast<char*>(data), sizeof(data));
}