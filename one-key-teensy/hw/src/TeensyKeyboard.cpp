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
#include "TeensyKeyboard.h"
#include <string.h>
#include "usb_keyboard.h"
#include "HIDAliases.h"

void TeensyEventDispatcher::begin() {}

void TeensyEventDispatcher::queryConnectionTypes(
    uint8_t &connectionMask) {
  connectionMask |= Usb;
}

void TeensyEventDispatcher::consumerPress(uint8_t connectionMask,
                                          uint8_t keyCode) {}

void TeensyEventDispatcher::consumerRelease(uint8_t connectionMask,
                                            uint8_t keyCode) {}

void TeensyEventDispatcher::systemPress(uint8_t connectionMask,
                                        uint8_t keyCode) {}

void TeensyEventDispatcher::systemRelease(uint8_t connectionMask,
                                          uint8_t keyCode) {}

void TeensyEventDispatcher::keyPress(uint8_t connectionMask, uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    keyboard_modifier_keys |= 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    return;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (keyboard_keys[i] == k) {
      // Already pressed
      return;
    }
    if (keyboard_keys[i] == 0) {
      keyboard_keys[i] = k;
      return;
    }
  }
}

void TeensyEventDispatcher::keyRelease(uint8_t connectionMask, uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    keyboard_modifier_keys &= ~(1 << (k - HID_KEYBOARD_FIRST_MODIFIER));
    return;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (keyboard_keys[i] == k) {
      keyboard_keys[i] = 0;
    }
  }
}

void TeensyEventDispatcher::keyReleaseAll(uint8_t connectionMask) {
  memset(&keyboard_modifier_keys, 0, sizeof(keyboard_modifier_keys));
  memset(&keyboard_keys, 0, sizeof(keyboard_keys));
}

void TeensyEventDispatcher::keySendReport(uint8_t connectionMask) {
  usb_keyboard_send();
}
