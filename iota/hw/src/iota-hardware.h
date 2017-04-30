#pragma once
#include <Arduino.h>
#define HARDWARE_IMPLEMENTATION Iota
#include "KeyboardioScanner.h"

#define COLS 15
#define ROWS 5

class Iota {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();
};
