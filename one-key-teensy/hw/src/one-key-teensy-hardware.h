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
#pragma once
#include <Arduino.h>
#include "TeensyKeyboard.h"
#define HARDWARE_IMPLEMENTATION OneKeyTeensy
#include "matrix-scanner.h"

#define COLS 1
#define ROWS 1

class OneKeyTeensy {
  public:
    OneKeyTeensy();
    void setup();
    void scan_matrix();

  private:
    wezkeeb::MatrixScanner<COLS, ROWS> scanner_;
};
