#include "one-key-nrf52-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// row0: A0
// col0: A1
static const uint8_t row_pins[ROWS] = {2};
static const uint8_t col_pins[COLS] = {3};

BLEDis bledis;
BLEHidAdafruit blehid;

#define LED_PIN 17

static inline void selectRow(uint8_t row) {
  uint8_t pin = row_pins[row];

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

static inline void unSelectRow(uint8_t row) {
  uint8_t pin = row_pins[row];

  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT_PULLUP);
}

static void unSelectAllRows(void) {
  for (uint8_t x = 0; x < ROWS; x++) {
    unSelectRow(x);
  }
}

void led(bool on) {
  digitalWrite(LED_PIN, on ? HIGH : LOW);
}

void blink(uint8_t times) {
  while (true) {
    led(true);
    delay(200);
    led(false);
    if (--times == 0) {
      return;
    }
    delay(200);
  }
}

void OneKeyBLE::setup() {
  pinMode(LED_PIN, OUTPUT);

  pressed_ = false;
  wasPressed_ = false;
  unSelectAllRows();
  for (uint8_t col = 0; col < COLS; ++col) {
    auto pin = col_pins[col];
    digitalWrite(pin, HIGH);
    pinMode(pin, INPUT_PULLUP);
  }

  Bluefruit.begin();
  Bluefruit.setName("woot");

  bledis.setManufacturer(KBD_MANUFACTURER);
  bledis.setModel(KBD_PRODUCT);
  bledis.begin();

  blehid.begin();

  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  Bluefruit.Advertising.start();

  blink(4);
}

void OneKeyBLE::scan_matrix() {
  wasPressed_ = pressed_;

  for (uint8_t row = 0; row < ROWS; ++row) {
    selectRow(row);
    delayMicroseconds(30);

    for (uint8_t col = 0; col < COLS; ++col) {
      pressed_ = digitalRead(col_pins[col]) == LOW;
      led(pressed_);
    }

    unSelectRow(row);
  }

  act_on_matrix_scan();
}

void OneKeyBLE::act_on_matrix_scan() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    for (uint8_t col = 0; col < COLS; ++col) {
      uint8_t state = (wasPressed_ ? WAS_PRESSED : 0) |
                      (pressed_ ? IS_PRESSED : 0);
      handle_keyswitch_event(Key_NoKey, row, col, state);

      if (!pressed_ && wasPressed_) {
        if (Bluefruit.connected()) {
          blehid.keyRelease();
        }
      } else if (!wasPressed_ && pressed_) {
        if (Bluefruit.connected()) {
          blehid.keyPress(' ');
        }
      }
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
