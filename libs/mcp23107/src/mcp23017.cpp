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
#include "mcp23017.h"

namespace wezkeeb {

#define i2cAddress 0x27 // Configurable with jumpers

enum mcp23017_registers {
	IODirectionA = 0x00,
  IODirectionB = 0x01,
  InputPolarityA = 0x02,
  InputPolarityB = 0x03,
  InterruptOnChangeA = 0x04,
  InterruptOnChangeB = 0x05,
  DefaultValueA = 0x06,
  DefaultValueB = 0x07,
  InterruptControlA = 0x08,
  InterruptControlB = 0x09,
  IOConfigurationA = 0x0a,
  IOConfigurationB = 0x0b,
  PullUpA = 0x0c,
  PullUpB = 0x0d,
  InterruptFlagA = 0x0e,
  InterruptFlagB = 0x0f,
  InterruptCaptureA = 0x10,
  InterruptCaptureB = 0x11,
  IOPortA = 0x12,
  IOPortB = 0x13,
  OutputLatchA = 0x14,
  OutputLatchB = 0x15,
};

static void set_reg(enum mcp23017_registers reg, uint8_t val) {
  Wire.beginTransmission(i2cAddress);
  Wire.write(uint8_t(reg));
  Wire.write(val);
  Wire.endTransmission();
}

void MCP23017::begin() {
  Wire.begin();

  // Set all the pins as inputs
  set_reg(IODirectionA, 0xff);
  set_reg(IODirectionB, 0xff);

  // Read key presses (logic low) as 0s
  set_reg(InputPolarityB, 0x00);
  set_reg(InputPolarityA, 0x00);

  // Turn on internal pull-ups; we're adding our own
  set_reg(PullUpA, 0xff);
  set_reg(PullUpB, 0xff);

  // Disable interrupts
  set_reg(InterruptOnChangeA, 0x0);
  set_reg(InterruptOnChangeB, 0x0);
}

uint16_t MCP23017::read() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(uint8_t(IOPortA));
  Wire.endTransmission();

  Wire.requestFrom(i2cAddress, 2);

  auto a = Wire.read();
  auto b = Wire.read();

  return a << 8 | b;
}

}
