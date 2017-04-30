#include "Kaleidoscope.h"

const Key keymaps[][ROWS][COLS] PROGMEM = { 
  {
    {Key_X},
  }
};


void setup() {
    Kaleidoscope.setup(KEYMAP_SIZE);
}

void loop() {
    Kaleidoscope.loop();
}
