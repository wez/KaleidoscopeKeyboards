#include "one-key-teensy-hardware.h"
#include "key_events.h"

static const uint8_t row_pins[ROWS] = {0};
static const uint8_t col_pins[COLS] = {1};

#define LED_PIN 13

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

void OneKeyTeensy::setup() {
  pinMode(LED_PIN, OUTPUT);

  pressed_ = false;
  wasPressed_ = false;
  unSelectAllRows();
  for (uint8_t col = 0; col < COLS; ++col) {
    auto pin = col_pins[col];
    digitalWrite(pin, HIGH);
    pinMode(pin, INPUT_PULLUP);
  }

  blink(4);
}

void OneKeyTeensy::scan_matrix() {
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

void OneKeyTeensy::act_on_matrix_scan() {
  uint8_t state = (wasPressed_ ? WAS_PRESSED : 0) |
    (pressed_ ? IS_PRESSED : 0);

  handle_keyswitch_event(Key_NoKey, 0, 0, state);
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
