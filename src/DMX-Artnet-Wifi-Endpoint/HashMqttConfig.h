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

#ifndef _HASHMQTTCONFIG_ALL_H_
#define _HASHMQTTCONFIG_ALL_H_ 1

#include <algorithm>
#include <functional>
#include <stdint.h>

typedef struct {
  uint32_t topic{ 0U };
  uint16_t dmxid{ 0U };
  uint8_t  value{ 0U };
  uint8_t  pin{ 0U };
} MQTTTODMX_t;

class HashMqttConfig {
  private:
    MQTTTODMX_t* data_{ nullptr };
    uint16_t size_{ 0 };

  public:

    HashMqttConfig();
    ~HashMqttConfig();

    void build(uint16_t sz);
    uint16_t size();
    const bool found(uint32_t topic_hash);

    void set(uint16_t length, uint8_t* data);
    void set(uint32_t topic_hash, uint16_t length, uint8_t* data);

    MQTTTODMX_t* get(const char* topic_string);
    MQTTTODMX_t* get_by_id(uint16_t dmx_id);
    MQTTTODMX_t* get_by_hash(uint32_t topic_hash);

    void add(uint16_t idx, String& s, uint16_t dmxid);
    void add(uint16_t idx, String& s, uint16_t dmxid, uint8_t pin);

    void update(std::function<void(uint16_t, uint8_t)> f);
    void update(std::function<void(uint16_t, uint8_t, uint8_t)> f);

    uint32_t hash(String& s);
    uint32_t hash(const char* s);
};

typedef struct {
  bool mqtt_enable{ true };
  uint16_t http_port{ 9001 };
  uint16_t mqtt_port{ 1883 };
  IPAddress ip{};
  String host{};
  String domain{};
  String mqtt_will{};
  String mqtt_subscribe{};
  HashMqttConfig mqttdata{};
} CONFIG_t;

#endif
