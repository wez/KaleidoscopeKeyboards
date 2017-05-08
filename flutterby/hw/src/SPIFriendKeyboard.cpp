#include "SPIFriendKeyboard.h"
#include "HIDAliases.h"

void SPIFriendEventDispatcher::begin() {
  modifiers_ = 0;
  memset(report_, 0, sizeof(report_));
  dirty_ = false;
}

void SPIFriendEventDispatcher::queryConnectionTypes(
    uint8_t &connectionMask) {
  if (spifriend_.isConnected()) {
    connectionMask |= Ble;
  }
}

void SPIFriendEventDispatcher::consumerPress(uint8_t connectionMask,
                                             uint8_t keyCode) {
}

void SPIFriendEventDispatcher::consumerRelease(uint8_t connectionMask,
                                               uint8_t keyCode) {
}

void SPIFriendEventDispatcher::systemPress(uint8_t connectionMask, uint8_t keyCode) {
}

void SPIFriendEventDispatcher::systemRelease(uint8_t connectionMask, uint8_t keyCode) {
}

void SPIFriendEventDispatcher::keyPress(uint8_t connectionMask,
                                        uint8_t k) {
  if (!(connectionMask & Ble)) {
    return;
  }

  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    auto mod = 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    if ((modifiers_ & mod) == 0) {
      modifiers_ |= mod;
      dirty_ = true;
    }
    return;
  }

  int8_t availablePos = -1;

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_[i] == k) {
      // Already pressed
      return;
    }
    if (availablePos == -1 && report_[i] == 0) {
      // We found a slot to record this key press,
      // but we'll keep looking in case this is already
      // recorded later in the report array.
      availablePos = i;
    }
  }

  if (availablePos != -1) {
    report_[availablePos] = k;
    dirty_ = true;
  }
}

void SPIFriendEventDispatcher::keyRelease(uint8_t connectionMask,
                                          uint8_t k) {
  if (!(connectionMask & Ble)) {
    return;
  }

  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    auto mod = 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    if ((modifiers_ & mod) != 0) {
      modifiers_ &= ~mod;
      dirty_ = true;
    }
    return;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_[i] == k) {
      report_[i] = 0;
      dirty_ = true;
    }
  }
}

void SPIFriendEventDispatcher::keyReleaseAll(uint8_t connectionMask) {
  if (!(connectionMask & Ble)) {
    return;
  }

  if (modifiers_ != 0) {
    modifiers_ = 0;
    dirty_ = true;
  }

  if (!dirty_) {
    for (uint8_t i = 0; i < 6; ++i) {
      if (report_[i]) {
        dirty_ = true;
        break;
      }
    }
  }

  if (dirty_) {
    memset(&report_, 0, sizeof(report_));
  }
}

void SPIFriendEventDispatcher::keySendReport(uint8_t connectionMask) {
  if (!(connectionMask & Ble)) {
    return;
  }

  if (!dirty_) {
    return;
  }
  dirty_ = false;
  spifriend_.keyReport(modifiers_, report_);
}
