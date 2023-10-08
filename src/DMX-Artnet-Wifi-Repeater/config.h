
#include "wemos_d1_r1_mini.h"

#define STASSID "TUVIT01F"
#define STAPSK "52223549"

#define USING_HTTP_AS_GW 1
#define USING_HTTP_AS_HOST 192,168,22,1
#define USING_HTTP_PORT 9001

#define USING_MQTT_SUBSCRIBE "sensor/"
#define USING_MQTT_PASSWORD "12345"
#define USING_MQTT_STATE "/state"
#define USING_MQTT_PORT 1883
#define USING_MQTT_ID 0

#define DEBUG 0
#define SEND_DMX_CODE_T1 1

#define PIN_TX WD4
#define PIN_ERROR WD8
#define PIN_RS485_UP WD2

/* ------------------------------------- */

#if (defined(DEBUG) && (DEBUG == 1))
  #define DEBUG_ 1
  #define ISPRINT_() true
  #define DEBUG_PRINT_(...) Serial.println(__VA_ARGS__)
#else
  #if defined(HTTP_TEST_INPUT)
    #undef HTTP_TEST_INPUT
  #endif
  #define ISPRINT_() false
  #define DEBUG_PRINT_(...)
#endif

#if (defined(USING_HTTP_AS_GW) && (USING_HTTP_AS_GW == 1))
  #define HTTP_HOST_() IPAddress(WiFi.gatewayIP())
#else
  #define HTTP_HOST_() IPAddress(USING_HTTP_AS_HOST)
#endif

#define HTTP_PORT_() static_cast<uint16_t>(USING_HTTP_PORT)
#define MQTT_PORT_() static_cast<uint16_t>(USING_MQTT_PORT)

