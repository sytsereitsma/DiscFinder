#include "Stream.h"

namespace arduino {

int Stream::readBytesUntil (uint8_t inChar, void* outBuffer, size_t inBufferSize) {
    auto buffer (static_cast <uint8_t*> (outBuffer));
    
    int count = 0;
    while(!mInputData.empty ()) {
        uint8_t c = mInputData.front ();
        mInputData.pop ();

        if (c == inChar) {
            break;
        }

        *buffer = c;
        ++buffer;
        ++count;

        if (count == inBufferSize) {
            break;
        }
    }
    

    return count;
}

uint8_t Stream::read () {
    uint8_t c = mInputData.front ();
    mInputData.pop ();

    return c;
}

void Stream::write (const void* buffer, size_t inBufferSize) {
    auto p = static_cast <const char*> (buffer);
    mOutputData.write (p, inBufferSize);
}
}