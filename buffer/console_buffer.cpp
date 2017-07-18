#ifndef LIGHT_CONSOLE_BUFFER
#define LIGHT_CONSOLE_BUFFER

#include "buffer.cpp"

using namespace std;

class ConsoleBuffer : public PushbackBuffer {
	public:
		ConsoleBuffer () : PushbackBuffer(&cin)
		{ /* ignore */ }
};

#endif
