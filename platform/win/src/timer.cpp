#include "timer.hpp"

#include <windows.h>

double PCFreq = 0;

uint64_t Timer::getTime () {
	LARGE_INTEGER li;

	if (PCFreq == 0) {
	    if(!QueryPerformanceFrequency(&li))
			fprintf(stderr, "[ERROR] QueryPerformanceFrequency failed!\n");
		else PCFreq = double(li.QuadPart);
	}

    QueryPerformanceCounter(&li);
    return static_cast<uint64_t>(li.QuadPart);
}

ATOM atom;

void dummy () {}

double Timer::clockStop (uint64_t start) {
	if (atom == 0) {
		WNDCLASSEX wc = {};
		wc.cbSize		 = sizeof(WNDCLASSEX);
	    wc.lpfnWndProc   = DefWindowProc;
	    wc.hInstance     = GetModuleHandle(NULL);
	    wc.lpszClassName = "DummyClassName";

	    atom = RegisterClassEx(&wc);
		if (atom) {
			/*HWND hwnd = CreateWindowEx(
				0, "DummyClassName", "Learn to Program Windows",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				0, 0, GetModuleHandle(NULL), 0
			);
			if (hwnd == NULL) {
				printf("ERROR CreateWindowExA -> %d\n", GetLastError());
				return 0;
			}

			MSG msg = {};
			while (GetMessage(&msg, hwnd, 0, 0)) {
				if (msg.message == WM_NULL) break;

				TranslateMessage(&msg);

				printf("Message: %u\n", msg.message);
				DispatchMessage(&msg);
			}*/
		} else printf("ERROR RegisterClass -> %d\n", GetLastError());
	}

	LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return double(li.QuadPart - start) / PCFreq;
}
