#pragma once

#ifdef __AVR__
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#endif

#ifdef __AVR_ATmega32U4__
#ifndef PRTIM4
#define PRTIM4 4
#endif
#endif

namespace wezkeeb {
namespace power {

// Subsystems that we wish to disable while LowPowerMode is active
enum Subsystems {
  UsbDevices = 1,
  TimerDevices = 2,
  SerialDevices = 4,
  ParallelDevices = 8,
  SpiDevices = 16,
  I2cDevices = 32,
  AdcDevices = 64
};

// These match the WDTO_XXX defines from wdt.h
enum SleepDuration {
  Sleep15ms,
  Sleep30ms,
  Sleep60ms,
  Sleep120ms,
  Sleep250ms,
  Sleep500ms,
  Sleep1s,
  Sleep2s,
  Sleep4s,
  Sleep8s,
  SleepForever,
};

enum SleepLevel {
  Idle,
  StandBy,
  PowerDown,
};

namespace detail {

#ifdef __AVR_ATmega32U4__
struct PRR0Saver {
  uint8_t adc;
  uint8_t reg;
  PRR0Saver(enum Subsystems off) : adc(ADCSRA), reg(PRR0) {
    if (off & AdcDevices) {
      ADCSRA &= ~(1 << ADEN);
    }

    uint8_t mask = ((off & I2cDevices) ? PRTWI : 0) |
                   ((off & TimerDevices) ? (PRTIM0 | PRTIM1) : 0) |
                   ((off & SpiDevices) ? PRSPI : 0) |
                   ((off & AdcDevices) ? PRADC : 0);
    PRR0 |= mask;
  }

  ~PRR0Saver() {
    PRR0 = reg;
    ADCSRA = adc;
  }
};
#endif

#ifdef __AVR_ATmega32U4__
struct PRR1Saver {
  uint8_t reg;
  PRR1Saver(enum Subsystems off) : reg(PRR1) {
    uint8_t mask = ((off & UsbDevices) ? PRUSB : 0) |
                   ((off & TimerDevices) ? (PRTIM4 | PRTIM3) : 0) |
                   ((off & SerialDevices) ? (PRUSART1) : 0);
    PRR1 |= mask;
  }
  ~PRR1Saver() { PRR1 = reg; }
};
#endif
}

class LowPowerMode {
#ifdef __AVR_ATmega32U4__
  detail::PRR0Saver prr0_;
  detail::PRR1Saver prr1_;
#endif
public:
  LowPowerMode(enum Subsystems off) : prr0_(off), prr1_(off) {}
  LowPowerMode(const LowPowerMode &) = delete;

  void sleep(enum SleepLevel level, enum SleepDuration duration) {
#ifdef __AVR_ATmega32U4__
    if (duration != SleepForever) {
      wdt_enable(duration);
    }
    set_sleep_mode(level == Idle ? SLEEP_MODE_IDLE
                                 : (level == StandBy ? SLEEP_MODE_STANDBY
                                                     : SLEEP_MODE_PWR_DOWN));
    sleep_enable();
    sleep_cpu();
    sleep_disable();

    if (duration != SleepForever) {
      wdt_disable();
    }
#endif
  }
};
}
}
