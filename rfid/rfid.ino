/*
#include <SoftwareSerial.h>
SoftwareSerial swSerial(RX1, TX1); // RX, TX
#define Serial1 swSerial
*/
#include "RFIDParser.h"
#include "RFIDCommands.h"
#include "RFIDUtil.h"

using namespace RFID;

constexpr int kEnablePin = A0;
constexpr int kBuzzerPin = 3; // Features PWM

unsigned long endMillis = 0; // End time for various delay sections
unsigned long prevPoll = 0; // last poll start time
constexpr unsigned long kPollInterval = 1000;

constexpr size_t kBufferSize = 256;
uint8_t serialBuffer[kBufferSize];
uint8_t* bufferPos = serialBuffer + 0;
const uint8_t* kBufferEnd = serialBuffer + kBufferSize;

RFID::Parser sParser (Serial1, Serial);

enum State {
    STATE_CLEAR_BUFFER,
    STATE_POLL,
    STATE_WAIT_DETECTION,

};

State sState = STATE_CLEAR_BUFFER;

void SetClearBufferState () {
    sState = STATE_CLEAR_BUFFER;
    endMillis = millis () + 250;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  while(!Serial);
  while(!Serial1);
  Serial.println("Ready...");

  pinMode(kEnablePin, OUTPUT);
  digitalWrite(kEnablePin, HIGH);

  pinMode(kBuzzerPin, OUTPUT);
  digitalWrite (kBuzzerPin, LOW);
  
  SetClearBufferState ();
}

int GetBuzzerFrequency (uint8_t signalStrength) {
    int freq = 1000;

    //Not all frequencies sound nice with my buzzer, so I picked the nice ones
    if (signalStrength > 200) {
        freq = 2500;
    }
    else if (signalStrength > 190) {
        freq = 1500;
    }

    return freq;

}

void loop() {
    switch (sState) {
        case STATE_CLEAR_BUFFER: {
            while (Serial1.available ()) {
                Serial1.read ();
            }

            if (millis () >= endMillis) {
                bufferPos = serialBuffer + 0;
                sState = STATE_POLL;
                endMillis = prevPoll + kPollInterval;
            }
            break;
        }
        case STATE_POLL: {
            auto now = millis ();
            if (now >= endMillis) {
                prevPoll = now;

                Serial.print (now);
                Serial.println(" Sending poll command");
                RFID::SendSinglePollCommand (Serial1);
            
                sState = STATE_WAIT_DETECTION;
                endMillis = millis () + 250; //1s timeout
            }
            break;
        }
        case STATE_WAIT_DETECTION: {
            while (Serial1.available () && bufferPos != kBufferEnd) {
                *bufferPos = Serial1.read ();
                ++bufferPos;
            }

            RFID::Packet packet;
            const size_t kBufSize = bufferPos - serialBuffer;
            const uint8_t* nextBegin = sParser.ParseBuffer (serialBuffer, kBufSize, packet);
            //if (packet.mType) {
            //    RFID::hexdump (Serial, serialBuffer, bufferPos- serialBuffer);
            //}

            if (packet.mType == PacketType::kNotification && packet.mCommand == CommandID::kSinglePoll &&
                packet.mPayload && packet.mPayloadSize == 17
             ) {
                Serial.print (millis ());
                Serial.print(" RSSI:");
                uint8_t signalStrength = *packet.mPayload;
                Serial.print (signalStrength);
                Serial.print (", PC  :");
                Serial.print ((packet.mPayload[1] << 8) + packet.mPayload[2]);
                Serial.print (", EPC :");
                RFID::hexdump (Serial, packet.mPayload + 3, 14);

                if (signalStrength) {
                    int freq = GetBuzzerFrequency(signalStrength);
                    Serial.print ("Tone: ");
                    Serial.println (freq);

                    tone (kBuzzerPin, freq, 100);
                }

                SetClearBufferState ();
            }

            if (packet.mType == PacketType::kResponse && packet.mCommand == CommandID::kError &&
                packet.mPayload && packet.mPayloadSize == 1
            ) {
                Serial.print (millis());
                Serial.println (" No tag found");
                SetClearBufferState ();
            }

            if (millis () >= endMillis) {
                SetClearBufferState ();
            }

            break;
        }
    }
}
