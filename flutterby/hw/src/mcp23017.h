#pragma once
#include <Wire.h>

class MCP23017 {
  public:
    void init();
    uint16_t read();
};
