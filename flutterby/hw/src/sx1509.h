#pragma once
#include <inttypes.h>

class SX1509 {
  public:
    void begin();
    uint16_t read();
};
