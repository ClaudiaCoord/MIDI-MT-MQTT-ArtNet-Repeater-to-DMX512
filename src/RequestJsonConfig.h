#include <stdint.h>
constexpr auto READ_CLIENT_SIZE = 2048;

bool RequestJsonConfig(DynamicJsonDocument& doc, CONFIG_t& config) {
  try {

    WiFiClient client{};
    DeserializationError err{};

    client.connect(config.ip, config.http_port);
    if (!client.connected())
      return false;

    client.setTimeout(2000);
    client.println(String("GET /mqttconfigs/") + config.host + String(".json HTTP/1.0"));
    client.println(String("Host: ") + config.ip.toString() + String(":") + String(config.http_port));
    client.println("User-Agent: ArtNet-DMX Reapeter");
    client.println("Connection: close");
    client.println();

    auto tmo = ::millis();
    while (!client.available()) {
      if (::millis() - tmo > 10000) {
        client.stop();
        return false;
      }
      ::delay(5);
    }

    char* ubuf = nullptr;
    
    try {
      uint16_t r = 0;
      ubuf = (char*) new char[READ_CLIENT_SIZE]{};

      tmo = ::millis();
      while (true) {
        int16_t sz_ = READ_CLIENT_SIZE - r - 1;
        if (sz_ <= 0) break;
        int16_t r_ = static_cast<int16_t>(client.read(reinterpret_cast<uint8_t*>(ubuf + r), sz_));
        if (r_ == 0) {
          if (::millis() - tmo > 1000)  break;
          ::delay(3);
          continue;
        }
        if (r_ < 0) break;
        r += r_;
        ::delay(3);
      }

      client.flush();
      client.stop();

      if (r == 0U) {
        delete [] ubuf;
        return false;
      }
      char* cbuf = ubuf;

      DEBUG_PRINT_(String(cbuf));
      for (uint16_t i = 0, n = 0; i < (r - 1); i++) {
        if ((cbuf[i] == '\r') && (cbuf[(i + 1)] == '\n')) {
          i++;
          if (n == 1) {
            uint16_t offset = i + 1;
            if (offset >= r) {
              delete [] ubuf;
              return false;
            }
            cbuf = cbuf + offset;
            break;
          }
          n = 1;
          continue;
        }
        n = 0;
      }

      DEBUG_PRINT_(String(cbuf));
      err = ::deserializeJson(doc, cbuf);
      delete [] ubuf;
      ubuf = nullptr;

    } catch (String s) {
      DEBUG_PRINT_(s);
      if (ubuf) delete [] ubuf;
      return false;
    }

    #if defined(DEBUG_)
      if (err.code() != DeserializationError::Ok)
        DEBUG_PRINT_(String("Deserialize Json failed: ") + String(err.c_str()));
      else
        DEBUG_PRINT_("Deserialize Json OK");
    #endif
    return err.code() == DeserializationError::Ok;

  } catch(String s) {
    DEBUG_PRINT_(s);
  }
  return false;
}

