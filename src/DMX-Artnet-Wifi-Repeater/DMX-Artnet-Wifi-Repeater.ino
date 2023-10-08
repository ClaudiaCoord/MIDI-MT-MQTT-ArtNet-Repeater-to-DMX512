/*
	MIDI EasyControl9 to MIDI-Mackie translator for Adobe Premiere Pro Control Surfaces.
	+ DMX protocol, support USB Open DMX and USB RS485 dongle.
	+ Art-Net protocol, support UDP local network broadcast.
	+ MQTT subscriber.
	(c) CC 2022-2023, MIT

	MIDI-MT LIGHT: Repeater gateway -> Art-Net and MQTT to DMX512.

	See README.md for more details.
	NOT FOR CHINESE USE FOR SALES! FREE SOFTWARE!
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Arduino.h>
#include <ArtnetWifi.h>
#include "config.h"
#include "DMXSerial.h"
#include "HashMqttConfig.h"
#include "MQTTPubSubClient.h"

#define ARDUINOJSON_ENABLE_ARDUINO_STREAM 1
#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#define ARDUINOJSON_ENABLE_COMMENTS 1
#include <ArduinoJson.h>
#include "RequestJsonConfig.h"

WiFiUDP udpsend{};
WiFiClient wificlient{};
ArtnetWifi artnet{};
DMXSerial dmx{};
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
  digitalWrite(errorPin, LOW);
  DEBUG_PRINT_("");

  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(errorPin, HIGH);
    if (i++ > 50) return false;
  }
  digitalWrite(errorPin, LOW);

  config.ip = HTTP_HOST_();
  config.http_port = HTTP_PORT_();
  config.mqtt_port = MQTT_PORT_();

  IPAddress ip = WiFi.localIP();
  config.host = String("art-dmx-") + String(ip[2]) + String("-") + String(ip[3]);
  config.domain = config.host + String(".local");
  DEBUG_PRINT_(String("Host: ") + config.domain);

  WiFi.hostname(config.domain);
  MDNS.begin(config.host);
  MDNS.addService("dmx", "udp", 6564);

  /* download MQTT to DMX config */
  {
    DynamicJsonDocument doc(4096);

    if (RequestJsonConfig(doc, config)) {
      try {

        JsonArray array = doc["mqtt"].as<JsonArray>();
        if (!array) return true;
        size_t i = 0, sz = array.size();
        if (!sz) return true;
        config.mqttdata.build(sz);

        for (JsonVariant v : array) {
          String s = v["sub"].as<String>();
          uint16_t d = v["dmx"].as<int>();
          if (s && d) config.mqttdata.add(i++, s, d);
        }

      } catch(String s) {
        DEBUG_PRINT_(s);
        digitalWrite(errorPin, HIGH);
        return true;
      }
      MqttSetup();
    }
  }
  return true;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  dmx.write(data, length);
}

void onMqttMessage(char* topic, byte* payload, uint32_t length) {

  if ((length == 0U) || (length > 3U)) return;

  try {

    MQTTTODMX_t* conf = config.mqttdata.get(topic);
    if (!conf) return;

    char num[4]{};
    for (size_t i = 0; i < length; i++)
      num[i] = (char)payload[i];
    conf->value = static_cast<uint8_t>(::atoi(num) * 2);
    conf->value = (conf->value == 254) ? 255 : conf->value;
    dmx.write(conf->dmxid, conf->value);
    DEBUG_PRINT_(String("* Found: ") + String(conf->dmxid) + String(", ") + String(conf->value)); /**/

  } catch (...) {}
}

void setup() {
  if (ISPRINT_()) Serial.begin(115200);
  pinMode(errorPin, OUTPUT);

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

    artnet.setArtDmxCallback(onDmxFrame);
    artnet.begin();
    dmx.init(512);

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
    config.mqttdata.update([=](uint16_t dmxid, uint8_t value) {
      dmx.write(dmxid, value);
    });
    dmx.update();

  } catch (...) {
    digitalWrite(errorPin, HIGH);
    return;
  }
  digitalWrite(errorPin, LOW);
}
