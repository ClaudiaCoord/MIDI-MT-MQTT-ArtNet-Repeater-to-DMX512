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

#ifndef _DMXSERIAL_H_
#define _DMXSERIAL_H_ 1

#include <stdint.h>
#include <inttypes.h>

class DMXSerial {
  private:
    bool Started{ false };
    uint16_t Size{ 512 };
    uint8_t* Data{ nullptr };
  public:
    DMXSerial();
    ~DMXSerial();

    void init(uint16_t = 512U);
    uint8_t read(uint16_t);
    void write(uint16_t, const uint8_t);
    void write(uint8_t*, const uint16_t);
    void update();
    void clear();
    void end();
};

#endif
