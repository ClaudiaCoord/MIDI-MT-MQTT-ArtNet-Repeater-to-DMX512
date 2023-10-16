/*
	MIDI-MT extended device. (https://claudiacoord.github.io/MIDI-MT/)
	+ DMX protocol, support USB Open DMX and USB RS485 dongle.
	+ Art-Net protocol, support UDP local network broadcast.
	+ MQTT subscriber, support ligts control from network.
	(c) CC 2022-2023, MIT

	MIDI-MT LIGHT: Repeater gateway -> Art-Net and MQTT to DMX512.

	See README.md for more details.
	NOT FOR CHINESE USE FOR SALES! FREE SOFTWARE!
*/

#include <Arduino.h>
#include <uart_register.h>
#include "config.h"
#include "DMXSerial.h"

#define max_Channel  512
#define default_Channel 256

  constexpr uint8_t txPin = PIN_RS485_TX;
  constexpr uint8_t upPin = PIN_RS485_UP;

  static uint16_t build_channel(uint16_t c, uint16_t s) {
    return (c >= s) ? (s - 1U) : ((c > 0U) ? (c - 1U) : c);
  }

  DMXSerial::DMXSerial() {
    pinMode(txPin, OUTPUT);
    pinMode(upPin, OUTPUT);
    digitalWrite(upPin, LOW);
  }
  DMXSerial::~DMXSerial() {
    end();
  }

  void DMXSerial::init(uint16_t csize) {
    if (csize > max_Channel || !csize)
      csize = default_Channel;
    Size = csize;
    if (Data) delete [] Data;
    Data = new byte[Size]{};

    digitalWrite(upPin, HIGH);
    Serial1.begin(250000);
    Started = true;
  }

  uint8_t DMXSerial::read(uint16_t ch) {
    if (!Started) return 0U;
    ch = build_channel(ch, Size);
    return Data[ch];
  }

  void DMXSerial::write(uint16_t ch, const uint8_t value) {
    if (!Started) return;
    ch = build_channel(ch, Size);
    Data[ch] = value;
  }

  void DMXSerial::write(uint8_t* data, const uint16_t length) {
    if (!Started || !data || !length) return;
    (void) std::memcpy((void*)Data, (const void*)data, (length > Size) ? Size : length);
  }

  void DMXSerial::clear() {
    if (!Started) return;
    (void) std::memset((void*)Data, 0, Size);
  }

  void DMXSerial::end() {
    Started = false;
    Serial1.end();
    digitalWrite(upPin, LOW);

    if (Data) delete [] Data;
    Data = nullptr;
    Size = 0;
  }

  #if defined (DEBUG_)
  uint16_t count_ = 0U;
  #endif

  void DMXSerial::update() {
    if (!Started) return;

    try {
      #if defined(SEND_DMX_CODE_T1)
        Serial1.flush();
        Serial1.begin(90000, SERIAL_8N2);
        while (Serial1.available()) Serial1.read();
        Serial1.write(0);
        Serial1.flush();

        Serial1.begin(250000, SERIAL_8N2);
        while (Serial1.available()) Serial1.read();
        Serial1.write(0);

      #elif defined(SEND_DMX_CODE_T2)
        digitalWrite(txPin, HIGH);
        Serial1.begin(83333, SERIAL_8N1);
        Serial1.write(0);
        Serial1.flush();
        delay(1);
        Serial1.end();

        Serial1.begin(250000, SERIAL_8N2);
        digitalWrite(txPin, LOW);

      #endif

      Serial1.write(Data, Size);
      Serial1.flush();
      delay(1);
      Serial1.end();

      #if defined (DEBUG_)
        #define LASTGROUP 16
        if (count_++ > 500U) {
          count_ = 0U;
          for (size_t i = 0; ((i < Size) && (i < LASTGROUP)); i++) {
            Serial.print(String(Data[i]) + String("/") + String((int)(i + 1)) + String(", "));
          }
          Serial.println("* END");
        }
      #endif
    } catch(...) {}
  }
