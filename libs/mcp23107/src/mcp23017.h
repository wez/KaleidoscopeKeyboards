#pragma once
#include <Wire.h>

namespace wezkeeb {

class MCP23017 {
  public:
    void init();
    uint16_t read();
};

}
