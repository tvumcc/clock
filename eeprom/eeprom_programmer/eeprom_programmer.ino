/*
  July 23, 2024
  This sketch is designed to program the AT28C64B EEPROM chip with the Arduino Mega
  I originally had this written for a Raspberry Pi 3B+, but I couldn't get it to work
  likely due to the the RPi's inability to read or supply voltages greater than 3.3V through
  its GPIO pins (The EEPROM required 5V supply and the input signals may have required >3.3V).
*/

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
// GPIO pin numbers for the wires running to the EEPROM address pins (can be changed)
uint8_t addr_pins[13] = {23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 43, 45, 47};
// GPIO pin numbers for the wires running to the EEPROM data pins (can be changed)
uint8_t data_pins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
uint8_t write_enable = 12; // Active low
uint8_t output_enable = 13; // Active low
bool hold_up = false;

// Returns the necessary byte to be written corresponding to the address of the EEPROM
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

void enable_write(bool value) {digitalWrite(write_enable, !value);}
void enable_output(bool value) {digitalWrite(output_enable, !value);}

// Sets the corresponding address lines to high or low based on the passed in address
void set_addr(uint16_t addr) {
  for (int i = 0; i < 13; i++) {
    digitalWrite(addr_pins[i], addr & (1 << i) ? HIGH : LOW);
  }
}

// Sets the pin modes of the data GPIO pins to either OUTPUT (if `output` is true) or INPUT otherwise 
void set_data_output(bool output) {
  for (int i = 0; i < 8; i++) {
    pinMode(data_pins[i], output ? INPUT : OUTPUT);
  }
}

// Sets the corresponding data lines to high or low based on the passed in data byte
void set_data(uint8_t data) {
  enable_output(false);
  set_data_output(false);
  
  for (int i = 0; i < 8; i++) {
    digitalWrite(data_pins[i], data & (1 << i) ? 1 : 0);
  }
}

// Reads the data stored in the EEPROM at that address
uint8_t read_data(uint16_t addr) {
  set_data_output(true);
  enable_output(true);  
  set_addr(addr);

  uint8_t data_byte = 0;
  for (int i = 0; i < 8; i++) {
    data_byte |= digitalRead(data_pins[i]) << i;
  }
  return data_byte;
}

// Writes a byte of data to an address in the EEPROM
bool write_data(uint16_t addr, uint8_t data) {
  if (data == read_data(addr)) { // Return early to prevent wasting write cycles
    return true;
  } 

  set_addr(addr);
  set_data(data);

  enable_write(true);
  delayMicroseconds(10);
  enable_write(false);
  delayMicroseconds(300);

  // Data Polling
  int counter = 0;
  while (read_data(addr) != data) {
    delay(1);
    counter++;
    if (counter == 1000) {
      Serial.print("There is a hold up at address ");
      Serial.print(addr, HEX);
      Serial.print(" with data ");
      Serial.println(data, HEX);

      Serial.print("Reading ");
      Serial.print(read_data(addr));
      Serial.print(" and should be getting: ");
      Serial.println(bin_data(addr));
      hold_up = true;
      return false;
    }
  }

  return false;
}

// Initialize all the GPIO pins
void init_all_pins() {
  // Start off in read mode
  
  // Enabler Pins
  pinMode(write_enable, OUTPUT);
  enable_write(false);
  pinMode(output_enable, OUTPUT);
  enable_output(true);

  // Address Pins
  for (int i = 0; i < 13; i++) {
    pinMode(addr_pins[i], OUTPUT);
  }

  // Data Pins
  set_data_output(true);
}

void setup() {
  init_all_pins();
  Serial.begin(57600);
  
  // Write to the EEPROM
  int limit = 8192;
  Serial.println("\nCommencing the writing process...");

  for (int i = 0; i < limit; i++) {
    bool already_written = write_data(i, bin_data(i));
    if (hold_up) return;
    Serial.print("Wrote value ");
    Serial.print(bin_data(i), HEX);
    Serial.print(" at address ");
    if (already_written) Serial.print(" (X) ");
    Serial.println(i, HEX);
  }

  // Match the contents of the EEPROM to the supplied binary data
  Serial.println("Verifying...");
  int errors = 0;

  for (int i = 0; i < limit; i++) {
    if (read_data(i) != bin_data(i)) {
      Serial.print(read_data(i), HEX);
      Serial.print(" is present at address ");
      Serial.print(i, HEX);
      Serial.print(" instead of ");
      Serial.println(bin_data(i), HEX);
    }
    
  }

  Serial.print("Done. ");
  Serial.print(errors);
  Serial.println(" errors found in total.");
}

void loop() {}