#include <windows.h>

void print (const char* message) {
	HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
	WriteFile(out, message, _strlen(message), NULL, NULL);
}

int _strlen (const char* message) {
	int len;
	for (len = 0; message[len]; len++);
	return len;
}
