#ifndef LIGHT_FILE_BUFFER
#define LIGHT_FILE_BUFFER

#include <fstream>

#include "buffer.cpp"

using namespace std;

class FileBuffer : public PushbackBuffer {
	public:
		FileBuffer (const char* filename) :
			PushbackBuffer(new ifstream(filename, ifstream::in))
		{ /* ignore */ }
};

#endif
