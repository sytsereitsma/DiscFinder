#include "catch.hpp"
#include "Stream.h"
#include "RFIDParser.h"
#include <initializer_list>


TEST_CASE ("Parser parses data", "[rfid parser]") {
    arduino::Stream mLogStream;
    arduino::Stream mDataStream;
    RFID::Parser mParser (mDataStream, mLogStream);

    SECTION ("Ideal packet buffer") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x01, 0x56, 0x9D, 0x7E };        
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x12u);
        REQUIRE (packet.mCommand == 0x34u);
        REQUIRE (packet.mPayload == buffer + 5);
        REQUIRE (packet.mPayloadSize == 1u);
        REQUIRE (*packet.mPayload == 0x56);
    }

    SECTION ("Ideal packet without payload") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x00, 0x46, 0x7E };
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x12u);
        REQUIRE (packet.mCommand == 0x34u);
        REQUIRE (packet.mPayload == buffer + 5);
        REQUIRE (packet.mPayloadSize == 0u);
    }

    SECTION ("Packet with offset start") {
        const uint8_t buffer[] = {0xAA, 0xBB, 0xBB, 0x12, 0x34, 0x00, 0x00, 0x46, 0x7E };
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x12u);
        REQUIRE (packet.mCommand == 0x34u);
        REQUIRE (packet.mPayload == buffer + 7);
        REQUIRE (packet.mPayloadSize == 0u);
    }

    SECTION ("Incomplete buffer") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x00, 0x46 };
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + 0);
        REQUIRE (packet.mPayload == nullptr);
    }

    SECTION ("Incomplete buffer with offset start") {
        const uint8_t buffer[] = { 0xAA, 0xBB, 0xBB};
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + 1);
        REQUIRE (packet.mPayload == nullptr);
    }

    SECTION ("Packet with trailer in payload") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x01, 0x7E, 0xC5, 0x7E };
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x12u);
        REQUIRE (packet.mCommand == 0x34u);
        REQUIRE (packet.mPayload == buffer + 5);
        REQUIRE (*packet.mPayload == 0x7E);
        REQUIRE (packet.mPayloadSize == 1u);
    }
    
    SECTION ("Packet with checksum error") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x00, 0xFF, 0x7E };
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mPayload == nullptr);
    }

    SECTION ("Packet with false start ") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x00, 0xBB, 0x7E, 0x12, 0x00, 0x01, 0xAA, 0x3B, 0x7E };
                                                               //< ----------- real packet ------------------->
        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x7E);
        REQUIRE (packet.mCommand == 0x12);
        REQUIRE (packet.mPayload == buffer + 10);
        REQUIRE (packet.mPayloadSize == 1u);
        REQUIRE (*packet.mPayload == 0xAA);
    }

    SECTION ("Packet with multi byte payload") {
        const uint8_t buffer[] = { 0xBB, 0x12, 0x34, 0x00, 0x03, 0xAA, 0xBB, 0xCC, 0x7A, 0x7E };

        constexpr size_t kBufferSize (sizeof (buffer) / sizeof (uint8_t));

        RFID::Packet packet;
        auto nextStart = mParser.ParseBuffer (buffer, kBufferSize, packet);
        REQUIRE (nextStart == buffer + kBufferSize);
        REQUIRE (packet.mType == 0x12);
        REQUIRE (packet.mCommand == 0x34);
        REQUIRE (packet.mPayload == buffer + 5);
        REQUIRE (packet.mPayloadSize == 3u);
        REQUIRE (packet.mPayload [0] == 0xAA);
        REQUIRE (packet.mPayload [1] == 0xBB);
        REQUIRE (packet.mPayload [2] == 0xCC);
    }
}
