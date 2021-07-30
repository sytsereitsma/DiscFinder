#pragma once
#include "catch.hpp"
#include "Stream.h"


struct StreamOutputMatcher : Catch::MatcherBase <arduino::Stream> {
    StreamOutputMatcher (const std::initializer_list <uint8_t>& inExpected) :
        mExpected (inExpected)
    {}

    template <size_t taArraySize>
    StreamOutputMatcher (const uint8_t (&cArray) [taArraySize]) :
        mExpected (cArray + 0, cArray + taArraySize)
    {}

    bool match (const arduino::Stream& actual) const {
        {
            auto str = actual.mOutputData.str ();
            mActual.clear ();
            mActual.reserve (str.size ());
            std::transform (str.begin (), str.end (), std::back_inserter (mActual),
                [](char c) -> uint8_t { return static_cast <uint8_t> (c); });
        }

        return std::equal (mExpected.begin (), mExpected.end (), mActual.begin (), mActual.end ());
    }

    std::string describe () const override {
        std::ostringstream msg;
        auto hexDump = [](std::ostream& stream, const std::vector <uint8_t>& data) {
            auto pos (data.begin ());
            const auto end (data.end ());
            while (pos != end) {
                stream << std::setfill ('0') << std::setw (2) << std::hex << std::setw (2) << unsigned (*pos);
                ++pos;
                if (pos != end) {
                    stream << ' ';
                }
            }

            stream << '\n';
        };

        msg << "Binary data mismatch:\n";
        msg << "  Expected: ";
        hexDump (msg, mExpected);
        msg << "  Actual  : ";
        hexDump (msg, mActual);

        return msg.str ();
    }

private:
    const std::vector <uint8_t> mExpected;
    mutable std::vector <uint8_t> mActual;
};
