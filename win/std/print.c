

int _strlen (const char* message) {
	int len;
	for (len = 0; message[len]; len++);
	return len;
}

void _i32_toString (int value, char* buffer) {
	short index = 0;
	if (value == 0) {
		buffer[index++] = '0';
	} else {
		char sign = '\0';
		if (value < 0) {
			value = -value;
			sign = '-';
		}
		while (value != 0) {
			buffer[index++] = (value % 10) + '0';
			value = value / 10;
		}
		if (sign != '\0')
			buffer[index++] = sign;
		for (short t = 0; t < (index / 2); t++) {
			buffer[t] ^= buffer[index - t - 1];
			buffer[index - t - 1] ^= buffer[t];
			buffer[t] ^= buffer[index - t - 1];
		}
	}
	buffer[index] = '\0';
}

void print (const char* message) {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(out, message, _strlen(message), NULL, NULL);
}

void println () {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(out, "\n", 1, NULL, NULL);
}

void print_i32 (int value) {
	char buffer[15];
	char* addr = &buffer[0];
	_i32_toString(value, addr);
	print(buffer);
}

void println_i32 (int value) {
	print_i32(value);
	println();
}
