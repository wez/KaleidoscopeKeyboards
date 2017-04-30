#include "one-key-nrf52-hardware.h"
extern HARDWARE_IMPLEMENTATION KeyboardHardware;

void BluefruitConsumerControl_::begin() {}
void BluefruitConsumerControl_::end() {}

void BluefruitConsumerControl_::write(uint16_t m){
  KeyboardHardware.blehid.consumerReport(m);
}

void BluefruitConsumerControl_::press(uint16_t m){
  KeyboardHardware.blehid.consumerKeyPress(m);
}

void BluefruitConsumerControl_::release(uint16_t m) {
  KeyboardHardware.blehid.consumerKeyRelease();
}

void BluefruitConsumerControl_::releaseAll(void) {
  KeyboardHardware.blehid.consumerKeyRelease();
}

void BluefruitConsumerControl_::SendReport(void* data, int length) {
  // Does nothing!
}

void BluefruitKeyboard_::begin(void) {
  releaseAll();
  sendReport();
}

void BluefruitKeyboard_::end(void) {
  releaseAll();
  sendReport();
}

size_t BluefruitKeyboard_::press(uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    report_.modifier |= 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    return 1;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_.keycode[i] == k) {
      // Already pressed
      return 1;
    }
    if (report_.keycode[i] == 0) {
      report_.keycode[i] = k;
      return 1;
    }
  }
  return 0;
}

size_t BluefruitKeyboard_::release(uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    report_.modifier &= ~(1 << (k - HID_KEYBOARD_FIRST_MODIFIER));
    return 1;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (report_.keycode[i] == k) {
      report_.keycode[i] = 0;
    }
  }
  return 1;
}

void BluefruitKeyboard_::releaseAll(void) {
  memset(&report_, 0, sizeof(report_));
}

int BluefruitKeyboard_::sendReport(void) {
  KeyboardHardware.blehid.keyboardReport(&report_);
  return 1;
}

void BluefruitSystemControl_::begin(void) {}
void BluefruitSystemControl_::end(void){}
void BluefruitSystemControl_::write(uint8_t s){}
void BluefruitSystemControl_::press(uint8_t s){}
void BluefruitSystemControl_::release(void){}
void BluefruitSystemControl_::releaseAll(void){}
void BluefruitSystemControl_::SendReport(void* data, int length) {}

BluefruitConsumerControl_ BluefruitConsumerControl;
BluefruitKeyboard_ BluefruitKeyboard;
BluefruitSystemControl_ BluefruitSystemControl;
