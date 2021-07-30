#pragma once
#include <stdint.h>

namespace arduino {
    class Stream;
}

namespace RFID {
    enum CommandID {
        kSinglePoll = 0x22,
        kError = 0xFF, //only in response frames
    };

    static const uint8_t kSinglePollCommand[] = { 0xBB, 0x00, 0x22, 0x00, 0x00, 0x22, 0x7E };
    static const uint8_t kHardwareVersionCommand [] = { 0xBB, 0x00, 0x03, 0x00, 0x01, 0x00, 0x04, 0x7E };
    

    void SendSinglePollCommand (arduino::Stream& outStream);
    void SendHardwareVersionCommand(arduino::Stream& outStream);
}
