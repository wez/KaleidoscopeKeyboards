#pragma once
#include <Arduino.h>
#include "BluefruitKeyboard.h"
#define HARDWARE_IMPLEMENTATION OneKeyBLE

#define COLS 1
#define ROWS 1

class OneKeyBLE {
  public:
    void setup();
    void scan_matrix();
    void act_on_matrix_scan();

  private:
    bool pressed_;
    bool wasPressed_;
};
