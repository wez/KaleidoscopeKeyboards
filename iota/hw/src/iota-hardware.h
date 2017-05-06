#pragma once
#include <Arduino.h>
#define HARDWARE_IMPLEMENTATION Iota
#include "mcp23017.h"

#define COLS 15
#define ROWS 5

class Iota {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();

  private:
    wezkeeb::MCP23017 expander_;
    uint16_t matrix_[ROWS];
    uint16_t priorMatrix_[ROWS];
};
