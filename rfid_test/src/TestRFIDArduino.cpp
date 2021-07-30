#define CATCH_CONFIG_MAIN 
#include "catch.hpp"
#include "fakeit.hpp"
#include "ArduinoGlobals.h"
#include "Stream.h"
#include "StreamOutputMatcher.h"
#include "RFIDCommands.h"

extern void setup ();
extern void loop ();

namespace {
    void LoopUntilSinglePollState () {
        loop (); //Clear buffer
        REQUIRE (Serial1.mOutputData.str ().empty ());

        loop ();
        CHECK_THAT (Serial1, StreamOutputMatcher (RFID::kSinglePollCommand));
    }

    unsigned long sMillis = 0;
    unsigned long FakeMillis () {
        return sMillis += 250;
    }
}

TEST_CASE ("RFID arduino main file tests parses data", "[rfid arduino]") {

    ArduinoTest::Reset ();
    sMillis = 0;
    ArduinoTest::SetMillisFunction (FakeMillis);

    setup ();

    SECTION ("Setup configures IO pins and serial ports") {
        fakeit::Verify (Method (ArduinoTest::sArduinoMock, pinMode).Using (A0, OUTPUT)).Once ();
        fakeit::Verify (Method (ArduinoTest::sArduinoMock, digitalWrite).Using (A0, HIGH)).Once ();
        fakeit::Verify (Method (ArduinoTest::sArduinoMock, pinMode).Using (3, OUTPUT)).Once ();
        fakeit::Verify (Method (ArduinoTest::sArduinoMock, digitalWrite).Using (3, LOW)).Once ();

        REQUIRE (115200 == Serial.mBaudrate);
        REQUIRE (115200 == Serial1.mBaudrate);
    }

    SECTION ("State machine clears buffer first") {
        Serial1.SetInputBuffer ({ 1, 2, 3 });
        loop ();
        REQUIRE (Serial1.mInputData.empty ());
        REQUIRE (Serial1.mOutputData.str().empty ());
    }

    SECTION ("After clearing buffer the poll command should be sent only once") {
        LoopUntilSinglePollState ();
        Serial1.mOutputData.str ("");
        loop (); //Loop again, we should not send the poll command again
        REQUIRE (Serial1.mOutputData.str ().empty ());
    }

    SECTION ("Wait for a response after sending the poll command") {
        LoopUntilSinglePollState ();
        
        Serial1.SetInputBuffer ({ 1, 2, 3 });
        loop ();
    }
};
