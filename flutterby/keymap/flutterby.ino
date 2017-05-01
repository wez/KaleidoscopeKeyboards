#include "Kaleidoscope-MouseKeys.h"
#include "Kaleidoscope-Macros.h"
#include "Kaleidoscope-DualUse.h"
#include "Kaleidoscope.h"
#include <Kaleidoscope-HostOS.h>
#include <Kaleidoscope/HostOS-select.h>

#define KEYMAP(                        \
    /* left hand */                    \
         k01, k02, k03, k04, k05, k06, \
    k10, k11, k12, k13, k14, k15, k16, \
    k20, k21, k22, k23, k24, k25, k26, \
    k30, k31, k32, k33, k34, k35,      \
                                  k07, \
                             k36, k17, \
                             k37, k27, \
    /* right hand */                   \
    k09, k0a, k0b, k0c, k0d, k0e,      \
    k19, k1a, k1b, k1c, k1d, k1e, k1f, \
    k29, k2a, k2b, k2c, k2d, k2e, k2f, \
         k3a, k3b, k3c, k3d, k3e, k3f, \
    k08,                               \
    k18, k39,                          \
    k28, k38) {                        \
  { XXX, k01, k02, k03, k04, k05, k06, k07, k08, k09, k0a, k0b, k0c, k0d, k0e, XXX }, \
  { k10, k11, k12, k13, k14, k15, k16, k17, k18, k19, k1a, k1b, k1c, k1d, k1e, k1f }, \
  { k20, k21, k22, k23, k24, k25, k26, k27, k28, k29, k2a, k2b, k2c, k2d, k2e, k2f }, \
  { k30, k31, k32, k33, k34, k35, k36, k37, k38, k39, k3a, k3b, k3c, k3d, k3e, k3f }, \
}

#define HYPER(k)  ((Key) { k.keyCode,uint8_t( k.flags | GUI_HELD | CTRL_HELD | LALT_HELD | RALT_HELD | SHIFT_HELD) })

// Each layer gets a name for readability, which is then used in the keymap matrix below.
enum layer_id {
  BASE = 0,
  RAISE,
  LOWER,
};

// Macro ids for use with M(n)
enum macro_id {
  MCOPY = 1,
  MCUT,
  MPASTE,
  FNOSTOGGLE,
  FNTHUMBSTICKMODE,
  MRESET,
};

const Key keymaps[][ROWS][COLS] PROGMEM = {
[BASE] =  KEYMAP(
                  Key_1, Key_2, Key_3, Key_4, Key_5, M(MCOPY),
  Key_Tab,        Key_Q, Key_W, Key_E, Key_R, Key_T, Key_LSquareBracket,
  CTL_T(Esc), Key_A, Key_S, Key_D, Key_F, Key_G, Key_Minus,
  Key_LShift,     Key_Z, Key_X, Key_C, Key_V, Key_B,
                                                    Key_LShift,
                                         Key_Keymap2_Momentary, HYPER(___),
                                         Key_Backspace,   Key_Delete,

  M(MPASTE),            Key_6, Key_7, Key_8,     Key_9,      Key_0,
  Key_RSquareBracket,   Key_Y, Key_U, Key_I,     Key_O,      Key_P,    Key_Backslash,
  Key_Equals,           Key_H, Key_J, Key_K,     Key_L,      Key_Semicolon, Key_Quote,
                        Key_N, Key_M, Key_Comma, Key_Period, Key_Slash, Key_RShift,
  Key_RAlt,
  Key_LGUI,   Key_Keymap1_Momentary,
  Key_Enter,    Key_Space
),

[RAISE] = KEYMAP(
          Key_F1,     Key_F2,     Key_F3,     Key_F4,     Key_F5,     ___,
  ___,   LCTRL(Key_Q), LCTRL(Key_W), LCTRL(Key_E), LCTRL(Key_R), LCTRL(Key_T), ___,
  ___,   LCTRL(Key_A), LCTRL(Key_S), LCTRL(Key_D), LCTRL(Key_F), LCTRL(Key_G), Key_volumeDown,
  ___,   LCTRL(Key_Z), LCTRL(Key_X), LCTRL(Key_C), LCTRL(Key_V), LCTRL(Key_B),
                                                 ___,
                                          ___,  ___,
                                          ___,  ___,

  ___,    Key_F6,   Key_F7,   Key_F8,   Key_F9,    Key_F10,
  ___,    ___,    Key_PageUp, ___,    ___,     ___,  M(FNOSTOGGLE),
  Key_volumeUp, Key_LArrow, Key_DnArrow, Key_UpArrow,   Key_RArrow, ___,  Key_Backtick,
           ___,    ___,    Key_prevTrack, Key_nextTrack,  Key_playPause,  ___,
  ___,
  ___,    ___,
  ___,    ___
  ),

[LOWER] = KEYMAP(
          Key_F1,   Key_F2, Key_F3,   Key_F4,   Key_F5,   ___,
  ___,   M(FNTHUMBSTICKMODE),    ___,  Key_End,  ___,    ___,    ___,
  ___,   Key_Home, ___,  Key_PageDn, Key_mouseBtnL, Key_mouseBtnR, Key_F14,
  Key_Backtick, M(MRESET),   ___,  ___,    ___,    ___,
                                                     ___,
                                            ___,    ___,
                                            ___,    ___,

  ___,    Key_F6,   Key_F7,   Key_F8,   Key_F9,    Key_F10,
  ___,    ___,    Key_PageUp, ___,    ___,     ___,   ___,
  Key_F15,  Key_LArrow, Key_DnArrow, Key_UpArrow,   Key_RArrow, ___,   Key_Backtick,
           ___,    ___,    Key_prevTrack, Key_nextTrack,  Key_playPause,   ___,
  ___,
  Key_PageUp, ___,
  Key_PageDn, ___
),
};

static void reboot_bootloader() {
    // Set the magic bits to get a Caterina-based device
    // to reboot into the bootloader and stay there, rather
    // than run move onward
    //
    // These values are the same as those defined in
    // Caterina.c

    uint16_t bootKey = 0x7777;
    uint16_t *const bootKeyPtr = (uint16_t *)0x0800;

    // Stash the magic key
    *bootKeyPtr = bootKey;

    // Set a watchdog timer
    wdt_enable(WDTO_120MS);

    while (1) {} // This infinite loop ensures nothing else
    // happens before the watchdog reboots us
}

const macro_t *macroAction(uint8_t macroIndex, uint8_t keyState) {
  if (!key_toggled_on(keyState)) {
    return MACRO_NONE;
  }
  switch (macroIndex) {
    case MCOPY:
      if (HostOS.os() == KaleidoscopePlugins::HostOS::OSX) {
        return MACRO(D(LGUI), T(C), U(LGUI), END);
      }
      return MACRO(D(LCtrl), T(Insert), U(LCtrl), END);

    case MPASTE:
      if (HostOS.os() == KaleidoscopePlugins::HostOS::OSX) {
        return MACRO(D(LGUI), T(V), U(LGUI), END);
      }
      return MACRO(D(LShift), T(Insert), U(LShift), END);

    case MRESET:
      reboot_bootloader();
      return MACRO_NONE;

    default:
      return MACRO_NONE;
  }
}

void setup() {
    Kaleidoscope.setup(KEYMAP_SIZE);
    Kaleidoscope.use(&Macros,
                     &MouseKeys,
                     &DualUse,
                     &HostOS,
                     NULL);

   // Calling this can cause things to break :/
   // HostOS.os(KaleidoscopePlugins::HostOS::OSX);

  // Bring down the dual use key timeout to something
  // that matches my typing style.
  KaleidoscopePlugins::DualUse::timeOut = 50;
}

void loop() {
    Kaleidoscope.loop();
}
