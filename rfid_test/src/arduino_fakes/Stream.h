#pragma once
#include <sstream>
#include <cstdint>
#include <queue>
#include <type_traits>
#include <string>

enum PrintBase {
    DEC = 10,
    HEX = 16    
};

namespace arduino {
    class Stream {
    public:
        Stream () = default;
        Stream (std::string const& name) :
            mName (name)
        {
        }

        virtual ~Stream () = default;
        int readBytesUntil (uint8_t inChar, void* outBuffer, size_t inBufferSize);
        uint8_t read ();

        template <typename T>
        void println (const T& value) {
            mOutputData << value << '\n';
        }

        template <typename T>
        void print (const T& value) {
            mOutputData << value;
        }

        template <typename T>
        void print (const T& value, PrintBase inBase /*= DEC*/) {
            if (inBase == DEC) {
                mOutputData << value;
            }
            else {
                mOutputData << std::hex << value;
            }
        }

        void write (const void* buffer, size_t inBufferSize);

        bool available () const {
            return !mInputData.empty ();
        }
    public: //Testing
        void SetInputBuffer (std::initializer_list <uint8_t>&& inList) {
            for (auto c : inList) {
                mInputData.push (c);
            }
        };
    public:
        std::queue <uint8_t> mInputData;
        std::ostringstream mOutputData;
    private:
        std::string mName;
    };
}

class UartClass : public arduino::Stream {
public:
    using arduino::Stream::Stream;

    void begin (int baudrate) {
        mBaudrate = baudrate;
    };
    bool operator!() const {return false;}

public:
    int mBaudrate = 0;
};
