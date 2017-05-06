/*
Copyright 2017 Wez Furlong

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
#include "one-key-teensy-hardware.h"
#include "key_events.h"

static const uint8_t row_pins[ROWS] = {0};
static const uint8_t col_pins[COLS] = {1};

OneKeyTeensy::OneKeyTeensy() : scanner_(row_pins, col_pins) {}

#define LED_PIN 13

void led(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

void blink(uint8_t times) {
  while (true) {
    led(true);
    delay(200);
    led(false);
    if (--times == 0) {
      return;
    }
    delay(200);
  }
}

void OneKeyTeensy::setup() {
  pinMode(LED_PIN, OUTPUT);

  scanner_.begin();

  blink(4);
}

void OneKeyTeensy::scan_matrix() {
  scanner_.scanMatrix();

  uint8_t state = (scanner_.prior()[0] ? WAS_PRESSED : 0) |
                  (scanner_.rows()[0] ? IS_PRESSED : 0);

  led(scanner_.rows()[0]);

  handle_keyswitch_event(Key_NoKey, 0, 0, state);
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
