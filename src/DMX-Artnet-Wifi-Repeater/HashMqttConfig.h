#include <algorithm>
#include <functional>

#include <stdint.h>

typedef struct {
  uint32_t topic{ 0U };
  uint16_t dmxid{ 0U };
  uint8_t  value{ 0U };
} MQTTTODMX_t;

class HashMqttConfig {
  private:
    MQTTTODMX_t* data_{ nullptr };
    size_t size_{ 0 };

  public:

    HashMqttConfig() {}
    ~HashMqttConfig() {
      if (data_) delete [] data_;
    }

    void build(uint16_t sz) {
      if (data_) delete [] data_;
      size_ = sz;
      data_ = (MQTTTODMX_t*) new MQTTTODMX_t[size_]{};
      DEBUG_PRINT_(String("Create MQTTTODMX_t objects: ") + String(size_));
    }

    size_t size() {
      return size_;
    }

    MQTTTODMX_t* get(const char* s) {
      uint32_t h = hash(s);
      if (!h) return nullptr;
      return get(h);
    }

    MQTTTODMX_t* get(uint32_t h) {
      if (!data_ || !size_) return nullptr;
      for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
        if (data_[i].topic == h) return &data_[i];
        if (data_[n].topic == h) return &data_[n];
      }
      return nullptr;
    }

    void add(uint16_t idx, String& s, uint16_t dmxid) {
      if (!data_ || (idx >= size_) || !dmxid) return;
      data_[idx].topic = hash(s);
      data_[idx].dmxid = dmxid;
      data_[idx].value = 0U;
      DEBUG_PRINT_(String(idx) + String(": ") + String(data_[idx].topic) + String(", ") + String(data_[idx].dmxid));
    }

    void update(std::function<void(uint16_t, uint8_t)> f) {
      if (!data_ || !size_) return;
      for (size_t i = 0, n = (size_ - 1); i < ceil(size_ / 2); i++, n--) {
        if (data_[i].value > 0) f(data_[i].dmxid, data_[i].value);
        if (data_[n].value > 0) f(data_[n].dmxid, data_[n].value);
      }
    }

    uint32_t hash(String& s) {
      if (!s) return 0;
      return hash(s.c_str());
    }

    uint32_t hash(const char* s) {
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

