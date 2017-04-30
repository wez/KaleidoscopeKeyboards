#include "TeensyKeyboard.h"
#include <string.h>

// We don't want teensy to define a Keyboard global, so we're
// re-declaring some of the basic pieces of the teensy core here

extern "C" {
  void usb_keyboard_write(uint8_t c);
  void usb_keyboard_write_unicode(uint16_t cpoint);
  void usb_keyboard_press_keycode(uint16_t n);
  void usb_keyboard_release_keycode(uint16_t n);
  void usb_keyboard_release_all(void);
  int usb_keyboard_press(uint8_t key, uint8_t modifier);
  int usb_keyboard_send(void);
  void usb_keymedia_release_all(void);
  extern uint8_t keyboard_modifier_keys;
  extern uint8_t keyboard_keys[6];
  extern uint8_t keyboard_protocol;
  extern uint8_t keyboard_idle_config;
  extern uint8_t keyboard_idle_count;
  extern volatile uint8_t keyboard_leds;
}

void TeensyConsumerControl_::write(uint16_t m) {}
void TeensyConsumerControl_::press(uint16_t m) {}
void TeensyConsumerControl_::release(uint16_t m) {}
void TeensyConsumerControl_::releaseAll(void) {}
void TeensyConsumerControl_::SendReport(void* data, int length) {}

void TeensyKeyboard_::begin(void) {}
void TeensyKeyboard_::end(void) {}

size_t TeensyKeyboard_::press(uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    keyboard_modifier_keys |= 1 << (k - HID_KEYBOARD_FIRST_MODIFIER);
    return 1;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (keyboard_keys[i] == k) {
      // Already pressed
      return 1;
    }
    if (keyboard_keys[i] == 0) {
      keyboard_keys[i] = k;
      return 1;
    }
  }
  return 0;
}

size_t TeensyKeyboard_::release(uint8_t k) {
  if ((k >= HID_KEYBOARD_FIRST_MODIFIER) && (k <= HID_KEYBOARD_LAST_MODIFIER)) {
    keyboard_modifier_keys &= ~(1 << (k - HID_KEYBOARD_FIRST_MODIFIER));
    return 1;
  }

  for (uint8_t i = 0; i < 6; ++i) {
    if (keyboard_keys[i] == k) {
      keyboard_keys[i] = 0;
    }
  }
  return 1;
}

void TeensyKeyboard_::releaseAll(void) {
  memset(&keyboard_modifier_keys, 0, sizeof(keyboard_modifier_keys));
  memset(&keyboard_keys, 0, sizeof(keyboard_keys));
}

int TeensyKeyboard_::sendReport(void) {
  return usb_keyboard_send();
}

void TeensySystemControl_::begin(void) {};
void TeensySystemControl_::end(void) {};
void TeensySystemControl_::write(uint8_t s) {}
void TeensySystemControl_::press(uint8_t s) {}
void TeensySystemControl_::release(void) {}
void TeensySystemControl_::releaseAll(void) {}
void TeensySystemControl_::SendReport(void* data, int length) {}

TeensyKeyboard_ TeensyKeyboard;
TeensyConsumerControl_ TeensyConsumerControl;
TeensySystemControl_ TeensySystemControl;
