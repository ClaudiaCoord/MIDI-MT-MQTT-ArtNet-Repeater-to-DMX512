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

#ifndef _REQUESTJSONCONFIG_ALL_H_
#define _REQUESTJSONCONFIG_ALL_H_ 1

#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 1
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>
#include <stdint.h>

class RequestJsonConfig {
  private:
    DynamicJsonDocument doc_;
    uint16_t limit_{ 2048 };
    bool isdata_ = false;

  public:

    RequestJsonConfig();
    RequestJsonConfig(uint16_t size);

    uint16_t limit();
    const bool empty() const;
    DynamicJsonDocument get() const;

    bool build(CONFIG_t& config);
};

#endif
