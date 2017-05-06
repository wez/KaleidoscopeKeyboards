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
#include "one-key-nrf52-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// row0: A0
// col0: A1
static const uint8_t row_pins[ROWS] = {2};
static const uint8_t col_pins[COLS] = {3};

OneKeyBLE::OneKeyBLE() : scanner_(row_pins, col_pins) {}

#define LED_PIN 17

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

void OneKeyBLE::setup() {
  pinMode(LED_PIN, OUTPUT);

  scanner_.begin();

  Bluefruit.begin();
  Bluefruit.setName("woot");

  bledis.setManufacturer(KBD_MANUFACTURER);
  bledis.setModel(KBD_PRODUCT);
  bledis.begin();

  blehid.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();

  blink(4);
}

void OneKeyBLE::scan_matrix() {
  scanner_.scanMatrix();

  uint8_t state = (scanner_.prior()[0] ? WAS_PRESSED : 0) |
                  (scanner_.rows()[0] ? IS_PRESSED : 0);

  led(scanner_.rows()[0]);

  handle_keyswitch_event(Key_NoKey, 0, 0, state);
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
