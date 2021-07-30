#include "RFIDCommands.h"
#include "RFIDUtil.h"
#include "UART.h"
#include "Stream.h"

extern UartClass Serial;

namespace RFID {

void SendSinglePollCommand(arduino::Stream& outStream) {
    outStream.write(kSinglePollCommand + 0, sizeof (kSinglePollCommand));
}

void SendHardwareVersionCommand (arduino::Stream& outStream) {
    outStream.write (kHardwareVersionCommand + 0, sizeof (kHardwareVersionCommand));
}

}
