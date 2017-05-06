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
#include "iota-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// row0: A0 - PF7
// row1: A1 - PF6
// row2: A2 - PF5
// row3: A3 - PF4
// row4: A4 - PF1
// col0-7: mcp23107 GPIOA0-7
// col8-14: mcp23107 GPIOB1-7 (note that B0 is unused)
static const uint8_t row_pins[ROWS] = {18, 19, 20, 21, 22};

Iota::Iota() : scanner_(row_pins) {}

void Iota::setup() {
  scanner_.begin();
}

void Iota::scan_matrix() {
  scanner_.scanMatrix();
  act_on_matrix_scan();
}

void Iota::act_on_matrix_scan() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    for (uint8_t col = 0; col < COLS; ++col) {
      uint8_t state = (bitRead(scanner_.prior()[row], col) ? WAS_PRESSED : 0) |
                      (bitRead(scanner_.rows()[row], col) ? IS_PRESSED : 0);
      handle_keyswitch_event(Key_NoKey, row, col, state);
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
