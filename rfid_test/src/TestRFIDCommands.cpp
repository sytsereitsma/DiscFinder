#include <vector>
#include <iomanip>
#include "catch.hpp"
#include "Stream.h"
#include "RFIDCommands.h"
#include "StreamOutputMatcher.h"

TEST_CASE ("Sends the correct commands", "[rfid commands]") {
    SECTION ("SinglePoll") {
        arduino::Stream dataStream;
        RFID::SendSinglePollCommand (dataStream);
        CHECK_THAT (dataStream, StreamOutputMatcher (std::initializer_list <uint8_t> { 0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E }));
    }
}