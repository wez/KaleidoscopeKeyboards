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
#pragma once
#include "EventDispatcher.h"
#include <bluefruit.h>

class BluefruitEventDispatcher : public EventDispatcher {
public:
  BluefruitEventDispatcher() : EventDispatcher() {}

  void begin() override;
  void queryConnectionTypes(uint8_t &connectionMask) override;
  void consumerPress(uint8_t connectionMask, uint8_t keyCode) override;
  void consumerRelease(uint8_t connectionMask, uint8_t keyCode) override;
  void systemPress(uint8_t connectionMask, uint8_t keyCode) override;
  void systemRelease(uint8_t connectionMask, uint8_t keyCode) override;
  void keyPress(uint8_t connectionMask, uint8_t keyCode) override;
  void keyRelease(uint8_t connectionMask, uint8_t keyCode) override;
  void keyReleaseAll(uint8_t connectionMask) override;
  void keySendReport(uint8_t connectionMask) override;

private:
  hid_keyboard_report_t report_;
};
