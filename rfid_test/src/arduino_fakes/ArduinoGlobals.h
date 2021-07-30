#pragma once
#include <functional>
#include "Stream.h"
#include "fakeit.hpp"

enum Pins {
    A0,
};

enum PinMode {
    INPUT,
    OUTPUT,
};

enum PinLevel {
    LOW,
    HIGH
};

void pinMode (int pin, int mode);
void digitalWrite (int pin, int level);

void delay(unsigned long);
unsigned long millis ();

void tone (int pin, unsigned int frequency, unsigned long duration);
void noTone (int pin);

extern UartClass Serial;
extern UartClass Serial1;

namespace ArduinoTest {
    struct MockInterface {
        virtual ~MockInterface () = default;

        virtual void pinMode (int pin, int mode) = 0;
        virtual void digitalWrite (int pin, int level) = 0;

        virtual unsigned long millis () = 0;
        virtual void delay (unsigned long) = 0;

        virtual void tone (int pin, unsigned int frequency, unsigned long duration) = 0;
        virtual void noTone (int pin) = 0;
    };

    extern fakeit::Mock<MockInterface> sArduinoMock;

    void Reset ();

    void SetMillisFunction (std::function <unsigned long ()> fn);
}
