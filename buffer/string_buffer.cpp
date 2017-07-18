#ifndef LIGHT_STRING_BUFFER
#define LIGHT_STRING_BUFFER

#include <sstream>

#include "buffer.cpp"

using namespace std;

class StringBuffer : public PushbackBuffer {
	public:
		StringBuffer (const char* source) :
			PushbackBuffer(new istringstream(source))
		{ /* ignore */ }
};

#endif
