#pragma once
#include <Arduino.h>
#define HARDWARE_IMPLEMENTATION Flutterby
#include "sx1509.h"

#define COLS 16
#define ROWS 4

class Flutterby {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();

  private:
    SX1509 expander_;
    uint16_t matrix_[ROWS];
    uint16_t priorMatrix_[ROWS];
};
