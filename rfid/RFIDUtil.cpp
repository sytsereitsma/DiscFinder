#include "RFIDUtil.h"
#include "Stream.h"

namespace RFID {

    void hexdump (arduino::Stream& outStream, const uint8_t* buffer, int count) {
        const uint8_t* pos (buffer);
        const uint8_t* end (buffer + count);
        while (pos != end) {
            if (*pos < 0xA) {
                outStream.print ('0');
            }
       
            outStream.print (*pos, HEX);
            ++pos;
            if (pos != end) {
                outStream.print (' ');
            }
        }

        outStream.print ('\n');
    }
}