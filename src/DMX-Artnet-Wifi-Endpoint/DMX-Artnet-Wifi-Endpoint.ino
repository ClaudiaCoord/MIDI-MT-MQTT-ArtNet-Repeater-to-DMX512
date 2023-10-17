/*
	MIDI-MT extended device. (https://claudiacoord.github.io/MIDI-MT/)
	+ DMX protocol, support USB Open DMX and USB RS485 dongle.
	+ Art-Net protocol, support UDP local network broadcast.
	+ MQTT subscriber, support ligts control from network.
	(c) CC 2022-2023, MIT

	MIDI-MT LIGHT: Endpoint device -> Art-Net and MQTT to relay or dimmer control.

	See README.md for more details.
	NOT FOR CHINESE USE FOR SALES! FREE SOFTWARE!
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Arduino.h>
#include <ArtnetWifi.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "HashMqttConfig.h"
#include "MQTTPubSubClient.h"
#include "RequestJsonConfig.h"

#if defined(DEBUG_)
  #pragma message "WARNING: build device DEBUG mode.."
#endif

  WiFiClient wificlient{};
  ArtnetWifi artnet{};
  PubSubClient mqttclient(wificlient);

  CONFIG_t config{};
  constexpr uint8_t errorPin = PIN_ERROR;

  void onDmxFrame(uint16_t, uint16_t, uint8_t, uint8_t*);
  void onMqttMessage(char*, byte*, unsigned int);

  bool MqttConnect() {
    if (mqttclient.connected()) return true;
    try{

      String mqttid;
      #if (defined(USING_MQTT_ID) && (USING_MQTT_ID == 1))
        mqttid = config.host + String("-") + String(random(0xffff), HEX);
      #else
        mqttid = config.host;
      #endif
      DEBUG_PRINT_(String("Connect to MQTT: ") + WiFi.gatewayIP().toString() + String(":") + String(config.mqtt_port) + String(", ID: ") + mqttid);
    
      if (!mqttclient.connect(mqttid.c_str(), config.host.c_str(), USING_MQTT_PASSWORD, config.mqtt_will.c_str(), 0, true, "0")) {
        DEBUG_PRINT_(String("MQTT connection failed: ") + String(mqttclient.state()));
        return false;
      }
      if (mqttclient.connected()) {
        mqttclient.subscribe(config.mqtt_subscribe.c_str());
        mqttclient.publish(config.mqtt_will.c_str(), "1");
        return true;
      }
      switch (mqttclient.state()) {
        case MQTT_CONNECT_BAD_PROTOCOL:
        case MQTT_CONNECT_BAD_CLIENT_ID:
        case MQTT_CONNECT_BAD_CREDENTIALS:
        case MQTT_CONNECT_UNAUTHORIZED: {
          config.mqtt_enable = false;
          break;
        }
      }
    } catch(String s) {
      DEBUG_PRINT_(s);
    }
    return false;
  }

  void MqttSetup() {
      mqttclient.setServer(config.ip, config.mqtt_port);
      mqttclient.setCallback(onMqttMessage);
      config.mqtt_will = String(USING_MQTT_SUBSCRIBE) + config.host.c_str() + String(USING_MQTT_STATE);
      config.mqtt_subscribe = String(USING_MQTT_SUBSCRIBE) + String("#");
      (void) MqttConnect();
  }

  bool NetworkUp() {
    int i = 0;
    ::digitalWrite(errorPin, LOW);

    #if !defined (MODE_ONOF_)
    ::analogWriteRange(255U);
    #endif

    DEBUG_PRINT_("");

    WiFi.begin(STASSID, STAPSK);
    while (WiFi.status() != WL_CONNECTED) {
      ::delay(500);
      ::digitalWrite(errorPin, HIGH);
      if (i++ > 50) return false;
    }
    ::digitalWrite(errorPin, LOW);

    config.ip = HTTP_HOST_();
    config.http_port = HTTP_PORT_();
    config.mqtt_port = MQTT_PORT_();

    IPAddress ip = WiFi.localIP();
    config.host = String("art-ep-") + String(ip[2]) + String("-") + String(ip[3]);
    config.domain = config.host + String(".local");
    DEBUG_PRINT_(String("Host: ") + config.domain);

    WiFi.hostname(config.domain);
    MDNS.begin(config.host);
    MDNS.addService("dmx", "udp", 6564);

    /* download MQTT configuration to DMX */
    RequestJsonConfig* jconf { nullptr };
    try {
      jconf = new RequestJsonConfig();
      if (jconf->build(config)) {
        if (!jconf->empty()) {
          DynamicJsonDocument doc = jconf->get();

          JsonArray array = doc["mqtt"].as<JsonArray>();
          if (array) {
            if (array.size()) {
              config.mqttdata.build(array.size());

              uint8_t pins[] PINS_OUT();
              uint16_t i = 0U,
                       n = 0U, x = static_cast<uint8_t>(std::size(pins));

              for (JsonVariant v : array) {
                String s = v["sub"].as<String>();
                uint16_t d = v["dmx"].as<int>();

                /* DEBUG_PRINT_(String("?: ") + s + String(d) + String(", ") + String(n) + String(", ") + String(pins[n])); */
                if (s && d) {
                  uint8_t pin = pins[n++];
                  config.mqttdata.add(i++, s, d, pin);
                  ::pinMode(pin, OUTPUT);
                  ::digitalWrite(pin, LOW);
                  if (n >= x) break;
                }
              }
              if (i > 0U) MqttSetup();
            }
          }
        }
      }

    } catch(String s) {
      DEBUG_PRINT_(s);
      digitalWrite(errorPin, HIGH);
    }
    if (jconf) delete jconf;
    return true;
  }

  void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
    config.mqttdata.set(length, data);
  }

  void onMqttMessage(char* topic, byte* payload, uint32_t length) {

    if ((length == 0U) || (length > 3U) || !topic || !payload) return;

    try {

      uint32_t h = config.mqttdata.hash(topic);
      if (!h || !config.mqttdata.found(h)) {
        DEBUG_PRINT_(String(topic) + String(", HASH:") + String(h) + String(" - NOT FOUND"));
        return;
      }
      config.mqttdata.set(h, length, payload);

    } catch (...) {}
  }

  void setup() {
    if (ISPRINT_()) Serial.begin(115200);
    ::pinMode(errorPin, OUTPUT);

    try {
      while (true) {
        if (!NetworkUp()) {
          DEBUG_PRINT_("Connect to Wifi error..");
        } else {
          DEBUG_PRINT_(String("IP address: ") + WiFi.localIP().toString());
          break;
        }
      }
      digitalWrite(errorPin, LOW);

      ArduinoOTA.setHostname(config.domain.c_str());
      ArduinoOTA.setPassword((const char *)USING_MQTT_PASSWORD);
      ArduinoOTA.onStart([]() {
        mqttclient.publish(config.mqtt_will.c_str(), "0");
      });
      ArduinoOTA.begin();

      artnet.setArtDmxCallback(onDmxFrame);
      artnet.begin();

    } catch(String s) {
        DEBUG_PRINT_(s);
        digitalWrite(errorPin, HIGH);
    }
  }

  void loop() {

    try {

      artnet.read();

      if (config.mqtt_enable) {
        if (!mqttclient.loop()) MqttConnect();
      }
      config.mqttdata.update([=](uint16_t, uint8_t pin, uint8_t value) {
        #if defined (MODE_ONOF_)
        ::digitalWrite(pin, (value > 0U) ? HIGH : LOW);
        #else
        ::analogWrite(pin, value);
        #endif
      });

      ArduinoOTA.handle();

    } catch (...) {
      ::digitalWrite(errorPin, HIGH);
      return;
    }
    ::digitalWrite(errorPin, LOW);
  }
