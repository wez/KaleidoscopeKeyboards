/*
Copyright 2017 Wez Furlong

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define DEBUG_SERIAL true

#include "Kaleidoscope-MouseKeys.h"
#include "Kaleidoscope-Macros.h"
#include "Kaleidoscope.h"

const Key keymaps[][ROWS][COLS] PROGMEM = { 
  {
    {Key_Backtick, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9, Key_0, Key_Minus, Key_Equals, Key_Backspace, Key_Backspace},
    {Key_Tab, Key_Q, Key_W, Key_E, Key_R, Key_T, Key_Y, Key_U, Key_I, Key_O, Key_P, Key_LSquareBracket, Key_RSquareBracket, Key_Backslash, Key_Delete},
    {Key_LCtrl, Key_A, Key_S, Key_D, Key_F, Key_G, Key_H, Key_J, Key_K, Key_L, Key_Semicolon, Key_Quote, Key_Enter, Key_Enter, Key_PageUp},
    {Key_LShift, Key_Z, Key_X, Key_C, Key_V, Key_B, Key_N, Key_M, Key_Comma, Key_Period, Key_Slash, Key_RShift, Key_RShift, Key_UpArrow, Key_PageDn},
    {Key_LShift, Key_LCtrl, Key_LGUI, ___, Key_LGUI, Key_Space, Key_Space, Key_LGUI, Key_RAlt, ___, Key_RCtrl, ___, Key_LArrow, Key_DnArrow, Key_RArrow}
  }
};


const macro_t *macroAction(uint8_t macroIndex, uint8_t keyState) {
    return MACRO_NONE;
}

void setup() {
    Kaleidoscope.setup(KEYMAP_SIZE);
    Kaleidoscope.use(&Macros,
                     &MouseKeys,
                     NULL);
}

void loop() {
    Kaleidoscope.loop();
}
