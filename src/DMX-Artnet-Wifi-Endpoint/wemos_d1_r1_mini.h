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

#ifndef _WEMOS_D1_MINI_H_
#define _WEMOS_D1_MINI_H_ 1

/* Wemos D1 Mini Pinout
       name | gpio | desc
*/
#  define WD0 16  // Digital free
#  define WD1 5   // SCL (I2C)
#  define WD2 4   // SDA (I2C)
#  define WD3 0   // -- (FLASH)
#  define WD4 2   // CE (SPI)
#  define WD5 14  // SCK (SPI)
#  define WD6 12  // MISO (SPI)
#  define WD7 13  // MOSI (SPI)
#  define WD8 15  // CSN (SPI)
#  define WTX 1   // -- (TX Serial)
#  define WRX 3   // -- (RX Serial)
#  define WA0 A0  // Analog input

#endif
