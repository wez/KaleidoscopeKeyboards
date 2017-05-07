/*
Copyright 2016-2017 Wez Furlong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include "ringbuffer.h"

namespace wezkeeb {

/** SPIFriend is a class targeted at using the Adafruit nRF51
 * based devices that communicate with the MCU via SPI.
 * We're not using the library provided by adafruit because we'd
 * like to perform more non-blocking work, as well as queue up
 * some commands for async delivery.
 * The result is a lower latency HID experience.
 */
class SPIFriend {
  SPISettings spi_;
  int8_t csPin_;
  int8_t irqPin_;
  int8_t rstPin_;
  int8_t powerPin_;
  enum State {
    NotInit,
    Initializing,
    Initialized,
    Configuring,
    Configured,
    Connecting,
    Connected,
  } state_;
  uint16_t lastStateTime_;

  enum QueueType {
    KeyReport,
    Consumer,
    MouseMove,
  };

  struct Item {
    QueueType type;
    uint16_t added;
    union __attribute__((packed)) {
      struct __attribute__((packed)) {
        uint8_t modifier;
        uint8_t keys[6];
      } key;
      uint16_t consumer;
      struct __attribute__((packed)) {
        int8_t x, y, scroll, pan;
        uint8_t buttons;
      } mousemove;
    };

    Item(uint8_t mods, uint8_t (&report)[6]);
    Item(const Item &) = default;
    Item() = default;
  };

  // Items that we wish to send
  RingBuffer<Item, 10> sendBuf_;
  // Pending response; while pending, we can't send any more requests.
  // This records the time at which we sent the command for which we
  // are expecting a response.
  RingBuffer<uint16_t, 2> respBuf_;

  // Commands are encoded using SDEP and sent via SPI
  // https://github.com/adafruit/Adafruit_BluefruitLE_nRF51/blob/master/SDEP.md

#define SdepMaxPayload 16
  struct SdepMsg {
    uint8_t type;
    uint8_t cmd_low;
    uint8_t cmd_high;
    struct __attribute__((packed)) {
      uint8_t len:7;
      uint8_t more:1;
    };
    uint8_t payload[SdepMaxPayload];

    template <typename StringPtr>
    SdepMsg(uint16_t command, const StringPtr *buf, uint8_t buflen, bool moredata);
    SdepMsg(uint16_t command);
    SdepMsg();
  } __attribute__((packed));

  bool sendPacket(const SdepMsg &msg, uint16_t timeout);
  void sendBytes(const uint8_t *buf, uint8_t size);
  bool recvPacket(SdepMsg &msg, uint16_t timeout);

  bool readResponse(char *resp, uint16_t resplen);
  void respBufWait();
  void respBufReadOne(bool greedy);
  void setConnected(bool connected);

  void sendBufSendOne();
  bool process(Item &item, uint16_t timeout);

  template <typename StringPtr>
  bool atCommand(const StringPtr *cmd, char *resp = NULL, uint16_t resplen = 0,
                 uint16_t timeout = 150);

public:
  // The defaults in the constructor are suitable for Feather BLE
  // boards with the nRF51 integrated on board.  If you have the
  // standalone board, you will need to specify the pins to
  // match your wiring.
  // The powerPin is a GPIO pin on the MCU connected to the VIN
  // on the SPIFriend.  If you have VIN connected to VCC instead,
  // leave powerPin set to -1.
  SPIFriend(int8_t csPin = 8, int8_t irqPin = 7, int8_t rstPin = 4,
            int8_t powerPin = -1);

  void begin();
  void end();

  // tick must be called periodically, such as from the loop()
  // function of your sketch, in order for this driver to operate
  // correctly.
  void tick();

  bool isConnected();

  bool keyReport(uint8_t modifiers, uint8_t (&report)[6]);
};

}
