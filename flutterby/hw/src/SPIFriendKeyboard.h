#pragma once
#include "EventDispatcher.h"
#include "spifriend.h"

class SPIFriendEventDispatcher : public EventDispatcher {
  wezkeeb::SPIFriend &spifriend_;
  uint8_t modifiers_{0};
  uint8_t report_[6];
  bool dirty_;

public:
  SPIFriendEventDispatcher(wezkeeb::SPIFriend &spifriend)
      : EventDispatcher(), spifriend_(spifriend) {}

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
};
