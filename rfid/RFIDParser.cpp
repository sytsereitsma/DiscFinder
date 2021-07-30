#include <stdint.h>
#include "RFIDParser.h"
#include "RFIDUtil.h"


namespace {
    inline uint8_t ComputeChecksum (const uint8_t* inBuffer, const uint8_t* inEnd) {
        uint8_t sum (0);
        const uint8_t* pos (inBuffer);
        while (pos != inEnd) {
            sum += *pos;
            ++pos;
        }

        return sum;
    }

    const uint8_t* find (const uint8_t* begin, const uint8_t* end, uint8_t c) {
        auto p = begin;
        while (p != end && *p != c) {
            ++p;
        }

        return p;
    };
}

namespace RFID {

    Parser::Parser (arduino::Stream& inDataStream, arduino::Stream& outLogStream) :
        mDataStream (inDataStream),
        mLogStream (outLogStream)
    {
    }


    /*
    Command / response structure :
    BB 00 03 00 01 00 04 7E
    -- -- -- -- -- -- -- --
    |  |  |  |  |  |  |  +Trailer
    |  |  |  |  |  |  +--Checksum
    |  |  |  |  |  +--Payload / parameters
    |  |  |  |  +--Payload size LSB
    |  |  |  +--Payload size MSB
    |  |  +--Command
    | +--Type
    + --Header
    */

    const uint8_t* Parser::ParseBuffer (const uint8_t* inBuffer, int inSize, Packet& outPacket) {
        constexpr uint8_t kHeader = 0xBB;
        constexpr uint8_t kTrailer = 0x7E;
        constexpr int kMinSize (7); //Header, Type, cmd, size(2), checksum, trailer
        const uint8_t* kEnd = inBuffer + inSize;
        const uint8_t* nextStart = inBuffer;
        const uint8_t* trailerPos = nullptr;

        outPacket.mType = 0;
        outPacket.mCommand = 0;
        outPacket.mPayloadSize = 0;
        outPacket.mPayload = nullptr;

        while (trailerPos != kEnd && nextStart != kEnd && outPacket.mPayload == nullptr) {
            const uint8_t* startPos = find (nextStart, kEnd, kHeader);
            if (startPos != kEnd) {
                nextStart = startPos;
                trailerPos = kEnd;

                if (startPos + kMinSize <= kEnd) {
                    int expectedPayloadSize = (startPos[3] << 8) + startPos[4];
                    trailerPos = startPos + expectedPayloadSize + kMinSize - 1;
                    nextStart = startPos + 1; // When stuff below fails we have to look for the next packet

                    if (trailerPos < kEnd) {
                        if (*trailerPos == kTrailer) {
                            uint8_t expectedChecksum = ComputeChecksum (startPos + 1, trailerPos - 1);
                            if (expectedChecksum == trailerPos[-1]) {
                                outPacket.mType = startPos[1];
                                outPacket.mCommand = startPos[2];
                                outPacket.mPayloadSize = expectedPayloadSize;
                                outPacket.mPayload = startPos + 5;

                                nextStart = trailerPos + 1;
                            }
                        }
                    }
                }
            }
            else {
                nextStart = kEnd;
            }
        }


        return nextStart;
    }

}
