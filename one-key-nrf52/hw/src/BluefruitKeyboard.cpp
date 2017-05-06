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
#include "one-key-nrf52-hardware.h"
#include "HIDAliases.h"
extern HARDWARE_IMPLEMENTATION KeyboardHardware;

void BluefruitEventDispatcher::begin() {}

void BluefruitEventDispatcher::queryConnectionTypes(
    uint8_t &connectionMask) {
  connectionMask |= Ble;
}

void BluefruitEventDispatcher::consumerPress(uint8_t connectionMask,
                                             uint8_t keyCode) {
  KeyboardHardware.blehid.consumerKeyPress(keyCode);
}

void BluefruitEventDispatcher::consumerRelease(uint8_t connectionMask,
                                               uint8_t keyCode) {
  KeyboardHardware.blehid.consumerKeyRelease();
}

void BluefruitEventDispatcher::systemPress(uint8_t connectionMask, uint8_t keyCode) {
}

void BluefruitEventDispatcher::systemRelease(uint8_t connectionMask, uint8_t keyCode) {
}

void BluefruitEventDispatcher::keyPress(uint8_t connectionMask,
                                        uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    report_.modifier |= 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    return;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_.keycode[i] == k) {
      // Already pressed
      return;
    }
    if (report_.keycode[i] == 0) {
      report_.keycode[i] = k;
      return;
    }
  }
}

void BluefruitEventDispatcher::keyRelease(uint8_t connectionMask,
                                          uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    report_.modifier &= ~(1 << (k - HID_KEYBOARD_FIRST_MODIFIER));
    return;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_.keycode[i] == k) {
      report_.keycode[i] = 0;
    }
  }
}

void BluefruitEventDispatcher::keyReleaseAll(uint8_t connectionMask) {
  memset(&report_, 0, sizeof(report_));
}

void BluefruitEventDispatcher::keySendReport(uint8_t connectionMask) {
  KeyboardHardware.blehid.keyboardReport(&report_);
}
