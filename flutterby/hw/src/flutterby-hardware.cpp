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
#include "flutterby-hardware.h"
#include "key_events.h"

// The keyboard matrix is attached to the following pins:
// thumbstick X: A0 - PF7 18
// thumbstick Y: A1 - PF6 19
// row0: A2 - PF5
// row1: A3 - PF4
// row2: A4 - PF1
// row3: A5 - PF0
// col0-15:   sx1509
static const uint8_t row_pins[ROWS] = {20, 21, 22, 23};

using wezkeeb::power::Subsystems;

//#define PowerOffInterval 600000UL // 10 minutes
#define PowerOffInterval 60000UL // 1 minute
#define LowPowerInterval 10000 // 10 seconds
#define SPIFRIEND_CS_PIN 1    // D3
#define SPIFRIEND_IRQ_PIN 5   // C6
#define SPIFRIEND_POWER_PIN 0 // D2
Flutterby::Flutterby()
    : spifriend_(SPIFRIEND_CS_PIN, SPIFRIEND_IRQ_PIN, -1, SPIFRIEND_POWER_PIN),
      basePowerMode_(Subsystems(Subsystems::SerialDevices |
                                Subsystems::ParallelDevices |
                                Subsystems::AdcDevices)),
      scanner_(row_pins), BLEdispatcher_(spifriend_) {}

static inline void led(bool on) {
  pinMode(13, OUTPUT);
  digitalWrite(13, on ? HIGH : LOW);
}

void Flutterby::setup() {
  scanner_.begin();
  spifriend_.begin();
  Serial.begin(9600);
  led(false);
}

void Flutterby::scan_matrix() {
  spifriend_.tick();
  scanner_.scanMatrix();
  act_on_matrix_scan();

  // After 10 seconds of inactivity, go into lower power mode
  if (scanner_.timeSinceLastChange() > LowPowerInterval) {
    lowPowerMode();
  }
}

void Flutterby::lowPowerMode() {
  using namespace wezkeeb::power;

  // The first stage of low power mode is to start putting the MCU to sleep
  // for progressively longer increments.  The sleep durations increase
  // by approximate powers of 2.  We'll keep doing this until we reach
  // a maximum sleep interval of 1 second.  We'll stay in this sleep
  // mode until we detect any key press or until we cross the PowerOffInterval
  {
    auto start = millis();
    uint16_t interval = 30;
    SleepDuration duration = Sleep15ms;

    while (millis() - start < PowerOffInterval) {
      auto intervalStart = millis();
      while (millis() - intervalStart < 10 * interval) {
        if (scanner_.isAnyKeyPressed()) {
          goto idle_wake;
        }
        basePowerMode_.sleep(Idle, duration);
      }

      if (duration != Sleep1s) {
        duration = SleepDuration(int(duration) + 1);
        // This isn't 100% precise, but it is close enough to
        // give us the approximate time we want to spend in
        // each level of sleep.
        interval *= 2;
      }
    }
  }

  // If we get here, we have been waiting for input for
  // long enough that we'd like to power down BLE for a while.
  // Blink the LED to make this more obvious
  led(true);
  delay(100);
  led(false);
  delay(100);
  led(true);
  delay(100);
  led(false);

  // In this stage of low power mode, we're turning off most devices.
  // The sleep interval is 2 seconds.
  {
    LowPowerMode pwr(Subsystems(SerialDevices | ParallelDevices | SpiDevices |
                                AdcDevices | UsbDevices));


    spifriend_.end();

    while (true) {
      // Turn off I2C while we sleep in this mode.
      // We'll need it back on for the matrix scan though,
      // so scope it to avoid disabling it during the
      // key press check
      {
        LowPowerMode lowerPwr(Subsystems(SerialDevices | ParallelDevices |
                                         SpiDevices | AdcDevices | UsbDevices |
                                         I2cDevices));
        lowerPwr.sleep(Idle, Sleep2s);
      }
      if (scanner_.isAnyKeyPressed()) {
        break;
      }
    }

    spifriend_.begin();
  }

idle_wake:
  scanner_.begin();
}

void Flutterby::act_on_matrix_scan() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    for (uint8_t col = 0; col < COLS; ++col) {
      uint8_t state = (bitRead(scanner_.prior()[row], col) ? WAS_PRESSED : 0) |
                      (bitRead(scanner_.rows()[row], col) ? IS_PRESSED : 0);

      handle_keyswitch_event(Key_NoKey, row, col, state);
    }
  }
}

HARDWARE_IMPLEMENTATION KeyboardHardware;
