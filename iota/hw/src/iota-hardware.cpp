#include "iota-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// row0: A0 - PF7
// row1: A1 - PF6
// row2: A2 - PF5
// row3: A3 - PF4
// row4: A4 - PF1
// col0-7: mcp23107 GPIOA0-7
// col8-14: mcp23107 GPIOB1-7 (note that B0 is unused)
static const uint8_t row_pins[ROWS] = {18, 19, 20, 21, 22};

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

void Iota::setup() {
  memset(matrix_, 0, sizeof(matrix_));
  memset(priorMatrix_, 0, sizeof(priorMatrix_));
  unSelectAllRows();
  expander_.init();
}

void Iota::scan_matrix() {
  memcpy(priorMatrix_, matrix_, sizeof(matrix_));

  for (uint8_t row = 0; row < ROWS; ++row) {
    selectRow(row);
    delayMicroseconds(30);

    // Note: 0 means pressed in the expander bits,
    // so invert that for more rational use.
    matrix_[row] = ~expander_.read();
    unSelectRow(row);
  }

  act_on_matrix_scan();
}

void Iota::act_on_matrix_scan() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    for (uint8_t col = 0; col < COLS; ++col) {
      uint8_t state = (bitRead(priorMatrix_[row], col) ? WAS_PRESSED : 0) |
                      (bitRead(matrix_[row], col) ? IS_PRESSED : 0);
      handle_keyswitch_event(Key_NoKey, row, col, state);
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
