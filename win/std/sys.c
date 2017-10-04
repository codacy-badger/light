#include <windows.h>

void system_exit (int exitCode) {
	ExitProcess(exitCode);
}
