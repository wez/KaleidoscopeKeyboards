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
#include "spifriend.h"
#include "Kaleidoscope.h"
namespace wezkeeb {

// These string_XXX functions are overloads that allow writing a generic
// implementation of atCommand and SdepMsg::SdepMsg that works with
// both PROGMEM strings and strings in RAM.

inline uint16_t string_len(const char *cmd) { return strlen(cmd); }

inline uint16_t string_len(const __FlashStringHelper *cmd) {
  return strlen_P(reinterpret_cast<PGM_P>(cmd));
}

// Bump the pointer by len
inline const __FlashStringHelper* string_advance(const __FlashStringHelper *s, uint16_t len) {
  return reinterpret_cast<const __FlashStringHelper*>(
      reinterpret_cast<const char *>(s) + len);
}

// Bump the pointer by len
inline const char *string_advance(const char *s, uint16_t len) {
  return s + len;
}

// Copy from src into dest
inline void string_copy(uint8_t *dest, const char *src, uint8_t len) {
  memcpy(dest, src, len);
}

// Copy from src into dest
inline void string_copy(uint8_t *dest, const __FlashStringHelper *src, uint8_t len) {
  memcpy_P(dest, reinterpret_cast<PGM_P>(src), len);
}

inline int16_t string_pos_difference(const char *first, const char *second) {
  return second - first;
}

inline int16_t string_pos_difference(const __FlashStringHelper *first,
                              const __FlashStringHelper *second) {
  return reinterpret_cast<PGM_P>(second) - reinterpret_cast<PGM_P>(first);
}

#define DEBUG_BLE 0

#if DEBUG_BLE
inline void print_helper() {}
template <typename First, typename... Rest>
void print_helper(First &&first, Rest &... rest) {
  Serial.print(first);
  print_helper(rest...);
}

template <typename T> void print_helper(T &&first) { Serial.print(first); }

template <typename... Args> void println(Args &&... args) {
  print_helper(args...);
  Serial.println(F(""));
}
#else
template <typename... Args> void println(Args &&... args) {}
#endif

template <typename Int>
static bool timedout(Int start, uint16_t timeout) {
  return millis() - start > timeout;
}

enum sdep_type {
  SdepCommand = 0x10,
  SdepResponse = 0x20,
  SdepAlert = 0x40,
  SdepError = 0x80,
  SdepSlaveNotReady = 0xfe, // Try again later
  SdepSlaveOverflow = 0xff, // You read more data than is available
};

enum ble_cmd {
  BleInitialize = 0xbeef,
  BleAtWrapper = 0x0a00,
  BleUartTx = 0x0a01,
  BleUartRx = 0x0a02,
};

enum ble_system_event_bits {
  BleSystemConnected = 0,
  BleSystemDisconnected = 1,
  BleSystemUartRx = 8,
  BleSystemMidiRx = 10,
};

#define SdepTimeout 150 /* milliseconds */
#define SdepBackOff 25 /* microseconds */

template <typename StringPtr>
SPIFriend::SdepMsg::SdepMsg(uint16_t command, const StringPtr *buf, uint8_t buflen,
                            bool moredata)
    : type(SdepCommand), cmd_low(command & 0xff), cmd_high(command >> 8),
      len(buflen), more(moredata && len == SdepMaxPayload) {
  static_assert(sizeof(*this) == 20, "SdepMsg is correctly packed");
  string_copy(payload, buf, len);
}

SPIFriend::SdepMsg::SdepMsg(uint16_t command) {
  memset(this, 0, sizeof(*this));
  type = command;
}

SPIFriend::SdepMsg::SdepMsg() {
  memset(this, 0, sizeof(*this));
}

SPIFriend::SPIFriend(int8_t csPin, int8_t irqPin, int8_t rstPin,
                     int8_t powerPin)
    : spi_(4000000, MSBFIRST, SPI_MODE0), csPin_(csPin), irqPin_(irqPin),
      rstPin_(rstPin), powerPin_(powerPin), state_(NotInit) {}

void SPIFriend::begin() {
  SPI.begin();

  pinMode(csPin_, OUTPUT);
  digitalWrite(csPin_, HIGH);

  pinMode(irqPin_, INPUT);

  if (powerPin_ != -1) {
    pinMode(powerPin_, OUTPUT);
    digitalWrite(powerPin_, HIGH);
  }

#if 0
  println("Starting spifriend");
  while (!Serial.available()) {
    delay(1);
  }
#endif

  tick();
}

void SPIFriend::tick() {
  char resbuf[128];

  switch (state_) {
  case NotInit:
    if (rstPin_ != -1) {
      // hardware reset
      pinMode(rstPin_, OUTPUT);
      digitalWrite(rstPin_, HIGH);
      digitalWrite(rstPin_, LOW);
      delay(10);
      digitalWrite(rstPin_, HIGH);
    } else {
      // software reset
      SdepMsg reset(BleInitialize);
      if (!sendPacket(reset, 1000)) {
        // Failed; try again next tick
        return;
      }
    }
    state_ = Initializing;
    lastStateTime_ = millis();
    println(lastStateTime_, F(" -> Initializing"));
    return;
  case Initializing:
    // Wait for the device to become ready
    if (timedout(lastStateTime_, 1000)) {
      state_ = Initialized;
      lastStateTime_ = millis();
      println(lastStateTime_, F(" -> Initialized"));
    }
    return;
  case Initialized:
    // no echo
    static const char kNoEcho[] PROGMEM = "ATE=0";
    // shorter poll intervals
    static const char kIntervals[] PROGMEM = "AT+GAPINTERVALS=10,30,,";
    static const char kProduct[] PROGMEM = "AT+GAPDEVNAME=" KBD_PRODUCT;
    // turn on HID
    static const char kHidEnabled[] PROGMEM = "AT+BLEHIDEN=1";
    // turn down power level
    static const char kPowerLevel[] PROGMEM = "AT+BLEPOWERLEVEL=-40";
    // turn off red mode led
    static const char kModeLED[] PROGMEM = "AT+HWMODELED=0";
    // reset for the above to take effect
    static const char kReset[] PROGMEM = "ATZ";
    static PGM_P const commands[] PROGMEM = {
        kNoEcho,     kIntervals, kProduct, kHidEnabled,
        kPowerLevel, kModeLED,   kReset,
    };
    for (uint8_t i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
      const __FlashStringHelper *cmd;
      memcpy_P(&cmd, commands + i, sizeof(cmd));
      if (!atCommand(cmd, resbuf, sizeof(resbuf))) {
        state_ = NotInit;
        lastStateTime_ = millis();
        return;
      }
    }

    state_ = Configuring;
    lastStateTime_ = millis();
    println(lastStateTime_, F(" -> Configuring"));
    return;
  case Configuring:
    // Wait for the device to become ready
    if (timedout(lastStateTime_, 1000)) {
      state_ = Configured;
      lastStateTime_ = millis();
      println(F("-> Configured"));
    }
    return;
  case Configured:
    if (!atCommand(F("AT+EVENTENABLE=0x1"), resbuf, sizeof(resbuf))) return;
    if (!atCommand(F("AT+EVENTENABLE=0x2"), resbuf, sizeof(resbuf))) return;
    if (atCommand(F("AT+GAPGETCONN"), resbuf, sizeof(resbuf))) {
      state_ = atoi(resbuf) ? Connected : Connecting;
      lastStateTime_ = millis();
      setConnected(state_ == Connected);
    }
    return;
  case Connecting:
  case Connected:
    respBufReadOne(true);
    sendBufSendOne();
    if (respBuf_.empty() && digitalRead(irqPin_)) {
      // Must be an event update
      if (atCommand(F("AT+EVENTSTATUS"), resbuf, sizeof(resbuf))) {
        uint32_t mask = strtoul(resbuf, NULL, 16);

        if (bitRead(mask, BleSystemConnected)) {
          state_ = Connected;
          setConnected(true);
        } else if (bitRead(mask, BleSystemDisconnected)) {
          state_ = Connecting;
          setConnected(false);
        }
      }
    }
  }
}

void SPIFriend::setConnected(bool connected) {
  // TODO: trigger something useful here
  println(connected ? F("-> Connected ") : F("-> Connecting"));

  // Disable the blue LED; it's very bright
  atCommand(F("AT+HWGPIO=19,0"));
}

bool SPIFriend::isConnected() {
  return state_ == Connected;
}

// Send bytes.  We can't use SPI.transfer(buf, size) because our
// input is const.
void SPIFriend::sendBytes(const uint8_t *buf, uint8_t size) {
  const uint8_t *end = buf + size;
  while (buf < end) {
    SPI.transfer(*buf);
    ++buf;
  }
}

bool SPIFriend::sendPacket(const SdepMsg &msg, uint16_t timeout) {
  SPI.beginTransaction(spi_);
  digitalWrite(csPin_, LOW);

  auto start = millis();
  bool ready = false;
  bool success = false;

  do {
    ready = SPI.transfer(msg.type) != SdepSlaveNotReady;
    if (ready) {
      break;
    }

    // Release CS and let it initialize
    digitalWrite(csPin_, HIGH);
    delay(SdepBackOff);
    digitalWrite(csPin_, LOW);
  } while (!timedout(start, timeout));

  if (ready) {
    sendBytes(&msg.cmd_low, sizeof(msg) - (1 + sizeof(msg.payload)) + msg.len);
    success = true;
  } else {
    println(F("timed out during sendPacket"));
  }

  digitalWrite(csPin_, HIGH);
  SPI.endTransaction();
  return success;
}

bool SPIFriend::recvPacket(SdepMsg &msg, uint16_t timeout) {
  bool success = false;
  auto start = millis();
  bool ready = false;

  do {
    ready = digitalRead(irqPin_);
    if (ready) {
      break;
    }
    delayMicroseconds(1);
  } while (!timedout(start, timeout));

  if (ready) {
    SPI.beginTransaction(spi_);

    digitalWrite(csPin_, LOW);

    do {
      // Read the command type, waiting for the data to be ready
      msg.type = SPI.transfer(0);
      if (msg.type == SdepSlaveNotReady || msg.type == SdepSlaveOverflow) {
        // Release it and let it initialize
        digitalWrite(csPin_, HIGH);
        delayMicroseconds(SdepBackOff);
        digitalWrite(csPin_, LOW);
        continue;
      }

      // Read the rest of the header
      SPI.transfer(&msg.cmd_low, sizeof(msg) - (1 + sizeof(msg.payload)));

      // and get the payload if there is any
      if (msg.len <= SdepMaxPayload) {
        SPI.transfer(msg.payload, msg.len);
      }
      success = true;
      break;
    } while (!timedout(start, timeout));

    digitalWrite(csPin_, HIGH);
    SPI.endTransaction();
    if (!success) {
      println(F("timeout waiting for slave to be ready"));
    }
  } else {
    println(F("timeout waiting for irq pin"), digitalRead(irqPin_),
            F(" start="), start, F(" now="), millis());
  }
  return success;
}

bool SPIFriend::readResponse(char *resp, uint16_t resplen) {
  char *dest = resp;
  char *end = dest + resplen;
  while (true) {
    SdepMsg msg;

    if (!recvPacket(msg, 2 * SdepTimeout)) {
      println(F("recvPacket failed"));
      return false;
    }

    if (msg.type != SdepResponse) {
      *resp = 0;
      println(F("recvPacket gave packet with non SdepResponse"));
      return false;
    }

    uint8_t len = min(msg.len, end - dest);
    if (len > 0) {
      memcpy(dest, msg.payload, len);
      dest += len;
    }

    if (!msg.more) {
      // No more data is expected!
      break;
    }
  }

  // Ensure the response is NUL terminated
  *dest = 0;

  // "Parse" the result text; we want to snip off the trailing OK or ERROR line
  // Rewind past the possible trailing CRLF so that we can strip it
  --dest;
  while (dest > resp && (dest[0] == '\n' || dest[0] == '\r')) {
    *dest = 0;
    --dest;
  }

  // Look back for start of preceeding line
  char *last_line = strrchr(resp, '\n');
  if (last_line) {
    ++last_line;
  } else {
    last_line = resp;
  }

  bool success = false;
  static const char kOK[] PROGMEM = "OK";

  success = !strcmp_P(last_line, kOK );
  println(resp);

  return success;
}

template <typename StringPtr>
bool SPIFriend::atCommand(const StringPtr *cmd, char *resp, uint16_t resplen,
                 uint16_t timeout) {
  auto len = string_len(cmd);
  auto end = string_advance(cmd, len);

  println(cmd);

  if (resp) {
    // They want to decode the response, so we need to flush and wait
    // for all pending I/O to finish before we start this one, so
    // that we don't confuse the results
    respBufWait();
    *resp = 0;
  }

  // Fragment the command into a series of SDEP packets
  while (string_pos_difference(cmd, end) > SdepMaxPayload) {
    SdepMsg msg(BleAtWrapper, cmd, SdepMaxPayload, true);
    if (!sendPacket(msg, timeout)) {
      return false;
    }
    cmd = string_advance(cmd, SdepMaxPayload);
  }

  SdepMsg msg(BleAtWrapper, cmd, string_pos_difference(cmd, end), false);
  if (!sendPacket(msg, timeout)) {
    return false;
  }

  if (resp == NULL) {
    auto now = millis();
    while (!respBuf_.enqueue(now)) {
      respBufReadOne(false);
    }
    return true;
  }

  return readResponse(resp, resplen);
}

void SPIFriend::respBufWait() {
  while (!respBuf_.empty()) {
    respBufReadOne(true);
  }
}

void SPIFriend::respBufReadOne(bool greedy) {
  uint16_t last_send;
  if (!respBuf_.peek(last_send)) {
    return;
  }

  if (digitalRead(irqPin_)) {
    SdepMsg msg;

  again:
    if (recvPacket(msg, SdepTimeout)) {
      if (!msg.more) {
        // We got it; consume this entry
        respBuf_.commit();
      }

      if (greedy && respBuf_.peek(last_send) && digitalRead(irqPin_)) {
        println(F("respBufReadOne go again"));
        goto again;
      }
    }
  } else if (timedout(last_send, SdepTimeout * 2)) {
    // Timed out: consume this entry
    respBuf_.commit();
  }
}

bool SPIFriend::process(Item &item, uint16_t timeout) {
  char cmdbuf[48];

  switch (item.type) {
    case KeyReport:
      snprintf(cmdbuf, sizeof(cmdbuf),
               "AT+BLEKEYBOARDCODE=%02x-00-%02x-%02x-%02x-%02x-%02x-%02x",
               item.key.modifier, item.key.keys[0], item.key.keys[1],
               item.key.keys[2], item.key.keys[3], item.key.keys[4],
               item.key.keys[5]);
      return atCommand(cmdbuf, NULL, 0, timeout);
    default:
      println(F("unhandled type in process!"));
      // Return true so that the offending item is de-queued
      return true;
  }
}

void SPIFriend::sendBufSendOne() {
  Item item;

  // Don't send anything more until we get an ACK
  if (!respBuf_.empty()) {
    return;
  }

  if (!sendBuf_.peek(item)) {
    return;
  }

  println(F("dispatch item"));

  if (process(item, SdepTimeout)) {
    // Commit the peek
    sendBuf_.commit();
    println(F("done with item"));
  } else {
    println(F("failed to send it"));
    delay(SdepTimeout);
    respBufReadOne(true);
  }
}

SPIFriend::Item::Item(uint8_t mods, uint8_t (&report)[6])
    : type(KeyReport), added(millis()) {
  key.modifier = mods;
  memcpy(key.keys, report, sizeof(key.keys));
}

bool SPIFriend::keyReport(uint8_t modifiers, uint8_t (&report)[6]) {
  if (!isConnected()) {
    println(F("not doing keyReport because not connected"));
    return false;
  }

  println(F("do keyReport"));
  Item item(modifiers, report);
  while (true) {
    if (sendBuf_.enqueue(item)) {
      break;
    }
    sendBufSendOne();
  }
  println(F("added keyReport"));
  return true;
}

}
