#include "one-key-nrf52-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// row0: A0 - PF7
// col0: A1 - PF6
static const uint8_t row_pins[ROWS] = {18};
static const uint8_t col_pins[COLS] = {19};

static inline void selectRow(uint8_t row) {
  uint8_t pin = row_pins[row];

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

static inline void unSelectRow(uint8_t row) {
  uint8_t pin = row_pins[row];

  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

static void unSelectAllRows(void) {
  for (uint8_t x = 0; x < ROWS; x++) {
    unSelectRow(x);
  }
}

void OneKeyBLE::setup() {
  pressed_ = false;
  wasPressed_ = false;
  unSelectAllRows();
  for (uint8_t col = 0; col < COLS; ++col) {
    auto pin = col_pins[col];
    digitalWrite(pin, HIGH);
    pinMode(pin, INPUT);
  }
}

void OneKeyBLE::scan_matrix() {
  wasPressed_ = pressed_;

  for (uint8_t row = 0; row < ROWS; ++row) {
    selectRow(row);
    delayMicroseconds(30);

    for (uint8_t col = 0; col < COLS; ++col) {
      pressed_ = digitalRead(col_pins[col]);
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
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
