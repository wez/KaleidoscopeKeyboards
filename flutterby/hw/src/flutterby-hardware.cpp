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
#include "flutterby-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// thumbstick X: A0 - PF7 18
// thumbstick Y: A1 - PF6 19
// row0: A2 - PF5
// row1: A3 - PF4
// row2: A4 - PF1
// row3: A5 - PF0
// col0-15:   sx1509
static const uint8_t row_pins[ROWS] = {20, 21, 22, 23};

Flutterby::Flutterby() : scanner_(row_pins) {}

void Flutterby::setup() {
  scanner_.begin();
  Serial.begin(9600);
}

void Flutterby::scan_matrix() {
  scanner_.scanMatrix();
  act_on_matrix_scan();
}

void Flutterby::act_on_matrix_scan() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    for (uint8_t col = 0; col < COLS; ++col) {
      uint8_t state = (bitRead(scanner_.prior()[row], col) ? WAS_PRESSED : 0) |
                      (bitRead(scanner_.rows()[row], col) ? IS_PRESSED : 0);

      handle_keyswitch_event(Key_NoKey, row, col, state);
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
