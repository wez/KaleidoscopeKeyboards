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
#define HARDWARE_IMPLEMENTATION Iota
#include "mcp23017.h"
#include "matrix-scanner.h"

#define COLS 15
#define ROWS 5

class Iota {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();
    Iota();

  private:
    wezkeeb::MatrixScannerWithExpander<COLS, ROWS, wezkeeb::MCP23017> scanner_;
};
