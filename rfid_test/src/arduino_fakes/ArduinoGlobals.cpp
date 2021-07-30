#include "ArduinoGlobals.h"
#include <chrono>

UartClass Serial ("Serial");
UartClass Serial1 ("Serial1");

void pinMode (int pin, int mode) {
    ArduinoTest::sArduinoMock.get().pinMode (pin, mode);
}

void digitalWrite (int pin, int level) {
    ArduinoTest::sArduinoMock.get ().digitalWrite (pin, level);
}

void delay (unsigned long delay) {
    return ArduinoTest::sArduinoMock.get ().delay (delay);
}

unsigned long millis () {
    return ArduinoTest::sArduinoMock.get ().millis ();
}

void tone (int pin, unsigned int frequency, unsigned long duration) {
}

namespace ArduinoTest {
    fakeit::Mock<MockInterface> sArduinoMock;

    void Reset () {
        auto resetSerial = [](UartClass& stream) {
            stream.mInputData = {};
            stream.mOutputData.clear ();
            stream.mBaudrate = 0;
        };

        resetSerial (Serial);
        resetSerial (Serial1);
        sArduinoMock.Reset ();

        //Stub all mocked functions
        fakeit::When (Method (sArduinoMock, pinMode)).AlwaysReturn ();
        fakeit::When (Method (sArduinoMock, digitalWrite)).AlwaysReturn ();
        fakeit::When (Method (sArduinoMock, millis)).AlwaysDo (
            []() {
                auto now (std::chrono::high_resolution_clock::now());
                auto sinceEpoch = now.time_since_epoch ();
                return static_cast <unsigned long> (std::chrono::duration_cast <std::chrono::milliseconds> (sinceEpoch).count ());
            }
        );

        fakeit::When (Method (sArduinoMock, delay)).AlwaysReturn ();
        fakeit::When (Method (sArduinoMock, tone)).AlwaysReturn ();
        fakeit::When (Method (sArduinoMock, noTone)).AlwaysReturn ();
    }

    void SetMillisFunction (std::function <unsigned long ()> fn) {
        if (fn) {
            fakeit::When (Method (sArduinoMock, millis)).AlwaysDo (fn);
        }
        else {
            fakeit::When (Method (sArduinoMock, millis)).AlwaysReturn (0);
        }
    }
}

