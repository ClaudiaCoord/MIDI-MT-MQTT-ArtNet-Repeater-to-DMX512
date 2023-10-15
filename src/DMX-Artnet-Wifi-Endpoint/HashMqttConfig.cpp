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
#include <IPAddress.h>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include "config.h"
#include "HashMqttConfig.h"

  HashMqttConfig::HashMqttConfig() {}
  HashMqttConfig::~HashMqttConfig() {
    if (data_) delete [] data_;
  }

  void HashMqttConfig::build(uint16_t sz) {
      if (data_) delete [] data_;
      size_ = sz;
      data_ = (MQTTTODMX_t*) new MQTTTODMX_t[size_]{};
      DEBUG_PRINT_(String("Create MQTTTODMX_t objects: ") + String(size_));
    }

  size_t HashMqttConfig::size() {
      return size_;
  }

  void HashMqttConfig::set(uint16_t length, uint8_t* data) {
    if (!data_ || !size_ || !length || !data) return;
    for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
      if (int16_t id = (data_[i].dmxid - 1); ((id >= 0) && (id < length))) data_[i].value = data[id];
      if (int16_t id = (data_[n].dmxid - 1); ((id >= 0) && (id < length))) data_[n].value = data[id];
    }
    #if defined (DEBUG_)
      #define LASTGROUP 16
      for (size_t i = 0; ((i < length) && (i < LASTGROUP)); i++) {
        Serial.print(String(data[i]) + String("/") + String((int)(i + 1)) + String(", "));
      }
      Serial.println("* END");
    #endif
  }

  MQTTTODMX_t* HashMqttConfig::get(const char* s) {
    uint32_t h = hash(s);
    if (!h) return nullptr;
    return get(h);
  }

  MQTTTODMX_t* HashMqttConfig::get(uint32_t h) {
    if (!data_ || !size_) return nullptr;
    for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
      if (data_[i].topic == h) return &data_[i];
      if (data_[n].topic == h) return &data_[n];
    }
    return nullptr;
  }

  MQTTTODMX_t* HashMqttConfig::get(uint16_t id) {
      if (!data_ || !size_) return nullptr;
      for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
        if (data_[i].dmxid == id) return &data_[i];
        if (data_[n].dmxid == id) return &data_[n];
      }
      return nullptr;
    }    

  void HashMqttConfig::add(uint16_t idx, String& s, uint16_t dmxid) {
    add(idx, s, dmxid, 0U);
  }

  void HashMqttConfig::add(uint16_t idx, String& s, uint16_t dmxid, uint8_t pin) {
    if (!data_ || (idx >= size_) || !dmxid) return;
    data_[idx].topic = hash(s);
    data_[idx].dmxid = dmxid;
    data_[idx].pin = pin;
    data_[idx].value = 0U;
    DEBUG_PRINT_(String(idx) + String(": ") + String(data_[idx].topic) + String(", DMX:") + String(data_[idx].dmxid) + String(", PIN:") + String((int)data_[idx].pin));
  }

  void HashMqttConfig::update(std::function<void(uint16_t, uint8_t)> f) {
    if (!data_ || !size_) return;
    for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
      #if defined (ARTNET_FILTER_)
        f(data_[i].dmxid, data_[i].value);
        f(data_[n].dmxid, data_[n].value);
      #else
        if (data_[i].value > 0) f(data_[i].dmxid, data_[i].value);
        if (data_[n].value > 0) f(data_[n].dmxid, data_[n].value);
      #endif
    }
  }

  void HashMqttConfig::update(std::function<void(uint16_t, uint8_t, uint8_t)> f) {
    if (!data_ || !size_) return;
    for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
      if (data_[i].pin > 0) f(data_[i].dmxid, data_[i].pin, data_[i].value);
      if (data_[n].pin > 0) f(data_[n].dmxid, data_[n].pin, data_[n].value);
    }
  }

  uint32_t HashMqttConfig::hash(String& s) {
    if (!s) return 0;
    return hash(s.c_str());
  }

  uint32_t HashMqttConfig::hash(const char* s) {
    if (!s) return 0;
    uint32_t chksum_ = 0;
    uint8_t tmp_chksum_;
    const char* buf_ = s;

    if (!buf_) return 0U;
    while (*buf_) {
      tmp_chksum_ = *buf_++;
      chksum_ += (tmp_chksum_ + 1);
    }
    return chksum_;
  }
