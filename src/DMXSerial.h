
#ifndef DMXSerial_h
#define DMXSerial_h

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
  void end();
};

#endif
