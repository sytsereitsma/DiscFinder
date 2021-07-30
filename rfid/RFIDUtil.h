#pragma once
#include <stdint.h>

namespace arduino {
    class Stream;
}

namespace RFID {

void hexdump(arduino::Stream& outStream, const uint8_t* buffer, int count);

}
