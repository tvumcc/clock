lookup_7s = {
	# gfedcba
	0b00111111 : "0",
	0b00000110 : "1",
	0b01011011 : "2",
	0b01001111 : "3",
	0b01100110 : "4",
	0b01101101 : "5",
	0b01111101 : "6",
	0b00000111 : "7",
	0b01111111 : "8",
	0b01101111 : "9", 
    0b00000000 : ""
}

with open("c.bin", "br") as f:
    content = f.read()
    for i in range(1440):
        pm = (content[i] & (1 << 7)) != 0

        first_digit = lookup_7s[content[i] & 0x7F]
        second_digit = lookup_7s[content[i | (0b1 << 11)] & 0x7F]
        third_digit = lookup_7s[content[i | (0b10 << 11)] & 0x7F]
        fourth_digit = lookup_7s[content[i | (0b11 << 11)] & 0x7F]

        print(f"Minute {i+1} -> {fourth_digit}{third_digit} : {second_digit}{first_digit} {pm}")