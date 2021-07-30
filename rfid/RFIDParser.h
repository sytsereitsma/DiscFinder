#pragma once

namespace arduino {
    class Stream;
}

namespace RFID {

enum PacketType {
    kCommand = 0,
    kResponse = 1,
    kNotification = 2,
};

struct Packet {
    uint8_t mType = 0;
    uint8_t mCommand = 0;
    const uint8_t* mPayload = nullptr;
    int mPayloadSize = 0;
};

class Parser {
public:
    Parser (arduino::Stream& inDataStream, arduino::Stream& outLogStream);

    const uint8_t* ParseBuffer (const uint8_t* inBuffer, int inSize, Packet& outPacket);
private:
    arduino::Stream& mDataStream;
    arduino::Stream& mLogStream;
};

}
