#pragma once
#include <inttypes.h>

class SX1509 {
  public:
    /** Configure the expander as all inputs */
    void begin();

    /** Read all the input pins */
    uint16_t read();
};
