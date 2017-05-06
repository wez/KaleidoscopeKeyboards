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
#include "sx1509.h"
#include <Wire.h>

namespace wezkeeb {

// Controls the SX1509 16 pin I/O expander

#define i2cAddress 0x3e // Configurable with jumpers
enum sx1509_registers {
  RegReset = 0x7d,
	RegDirA = 0x0f,
  RegDirB = 0x0e,
  RegPullUpA = 0x07,
  RegPullUpB = 0x06,
  DataA = 0x11,
  DataB = 0x10,
};

static void set_reg(enum sx1509_registers reg, uint8_t val) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(uint8_t(reg));
  Wire.write(val);
  Wire.endTransmission();
}

void SX1509::begin() {
  Wire.begin();

  // Software reset
  set_reg(RegReset, 0x12);
  set_reg(RegReset, 0x34);

  // Set all the pins as inputs
  set_reg(RegDirA, 0xff);
  set_reg(RegDirB, 0xff);

  // Turn on internal pull-ups
  set_reg(RegPullUpA, 0xff);
  set_reg(RegPullUpB, 0xff);
}

// Read all 16 inputs and return them
uint16_t SX1509::read() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(uint8_t(DataB));
  Wire.endTransmission();

  Wire.requestFrom(i2cAddress, 2);

  auto a = Wire.read();
  auto b = Wire.read();

  return a << 8 | b;
}

}
