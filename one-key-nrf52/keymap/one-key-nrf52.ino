#define DEBUG_SERIAL true

#include "Kaleidoscope.h"

const Key keymaps[][ROWS][COLS] PROGMEM = { 
  {
    {Key_Space},
  }
};


void setup() {
    Kaleidoscope.setup(KEYMAP_SIZE);
}

void loop() {
    Kaleidoscope.loop();
}
