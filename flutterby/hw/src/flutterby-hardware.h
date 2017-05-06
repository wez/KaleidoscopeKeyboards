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
#define HARDWARE_IMPLEMENTATION Flutterby
#include "sx1509.h"
#include "matrix-scanner.h"

#define COLS 16
#define ROWS 4

// This is just to satisfy a dependency of the Focus
// extension interface.  It is not actually used.
struct cRGB {
  uint8_t r, g, b;
};

class Flutterby {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();
    Flutterby();

  private:
    wezkeeb::MatrixScannerWithExpander<COLS, ROWS, wezkeeb::SX1509> scanner_;
};
