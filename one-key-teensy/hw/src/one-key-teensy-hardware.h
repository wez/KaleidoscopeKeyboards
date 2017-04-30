#pragma once
#include <Arduino.h>
#include "TeensyKeyboard.h"

#define HARDWARE_IMPLEMENTATION OneKeyTeensy

#define COLS 1
#define ROWS 1

class OneKeyTeensy {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();

  private:
    bool pressed_;
    bool wasPressed_;
};
