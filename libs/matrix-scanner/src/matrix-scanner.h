/*
Copyright 2016-2017 Wez Furlong

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
namespace wezkeeb {

// A little bit of compile time magic to determine the smallest
// integer width suitable to hold a given number of bits.
// http://stackoverflow.com/a/9095432/149111
namespace detail {
template <unsigned int N> struct best_integer_type {
  using type = typename best_integer_type<N - 1>::type;
};

template <> struct best_integer_type<0> { using type = uint8_t; };
template <> struct best_integer_type<8> { using type = uint16_t; };
template <> struct best_integer_type<16> { using type = uint32_t; };
template <> struct best_integer_type<32> { using type = uint64_t; };
}

/** Simplistic keyboard matrix scanner that can process
 * a matrix with rows and columns specified by the arrays
 * of pins passed to the constructor */
template <uint8_t NumCols, uint8_t NumRows> class MatrixScanner {
public:
  using col_t = typename detail::best_integer_type<NumCols>::type;
  using matrix_t = col_t[NumRows];

  MatrixScanner(const uint8_t (&rowPins)[NumRows],
                const uint8_t (&colPins)[NumCols])
      : rowPins_(rowPins), colPins_(colPins) {
    static_assert(NumCols > 0, "NumCols must be positive");
    static_assert(NumRows > 0, "NumRows must be positive");
  }

  void begin() {
    memset(rows_, 0, sizeof(rows_));
    memset(prior_, 0, sizeof(prior_));
    unSelectAllRows();

    for (uint8_t col = 0; col < NumCols; ++col) {
      auto pin = colPins_[col];
      digitalWrite(pin, HIGH);
      pinMode(pin, INPUT_PULLUP);
    }
  }

  inline void selectRow(uint8_t row) {
    uint8_t pin = rowPins_[row];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  inline void unSelectRow(uint8_t row) {
    uint8_t pin = rowPins_[row];

    digitalWrite(pin, HIGH);
    pinMode(pin, INPUT_PULLUP);
  }

  void unSelectAllRows() {
    for (uint8_t row = 0; row < NumRows; row++) {
      unSelectRow(row);
    }
  }

  void scanMatrix() {
    memcpy(prior_, rows_, sizeof(prior_));
    memset(rows_, 0, sizeof(rows_));

    for (uint8_t row = 0; row < NumRows; row++) {
      selectRow(row);
      delayMicroseconds(30);
      for (uint8_t col = 0; col < NumCols; col++) {
        if (digitalRead(colPins_[col]) == LOW) {
          bitWrite(rows_[row], col, 1);
        }
      }
      unSelectRow(row);
    }
  }

  const matrix_t &rows() const { return rows_; }
  const matrix_t &prior() const { return prior_; }

protected:
  const uint8_t (&rowPins_)[NumRows];
  const uint8_t (&colPins_)[NumCols];
  matrix_t rows_;
  matrix_t prior_;
};

/** This variant of a matrix scanner uses an IO Expander to
 * handle reading columns.
 * sx1509 and mcp23017 expander implementations can also be
 * found in my keyboard libs dir.
 */
template <uint8_t NumCols, uint8_t NumRows, typename Expander> class MatrixScannerWithExpander {
public:
  using col_t = typename detail::best_integer_type<NumCols>::type;
  using matrix_t = col_t[NumRows];

  MatrixScannerWithExpander(const uint8_t (&rowPins)[NumRows])
      : rowPins_(rowPins) {
    static_assert(NumCols > 0, "NumCols must be positive");
    static_assert(NumRows > 0, "NumRows must be positive");
  }

  void begin() {
    memset(rows_, 0, sizeof(rows_));
    memset(prior_, 0, sizeof(prior_));
    unSelectAllRows();
    expander_.begin();
  }

  inline void selectRow(uint8_t row) {
    uint8_t pin = rowPins_[row];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  inline void unSelectRow(uint8_t row) {
    uint8_t pin = rowPins_[row];

    digitalWrite(pin, HIGH);
    pinMode(pin, INPUT_PULLUP);
  }

  void unSelectAllRows() {
    for (uint8_t row = 0; row < NumRows; row++) {
      unSelectRow(row);
    }
  }

  void scanMatrix() {
    memcpy(prior_, rows_, sizeof(prior_));
    memset(rows_, 0, sizeof(rows_));

    for (uint8_t row = 0; row < NumRows; row++) {
      selectRow(row);
      delayMicroseconds(30);

      // Note: 0 means pressed in the expander bits,
      // so invert that for more rational use.
      rows_[row] = ~expander_.read();

      unSelectRow(row);
    }
  }

  const matrix_t &rows() const { return rows_; }
  const matrix_t &prior() const { return prior_; }

protected:
  const uint8_t (&rowPins_)[NumRows];
  matrix_t rows_;
  matrix_t prior_;
  Expander expander_;
};


}
