#pragma once
#include <bluefruit.h>
#include "HIDTables.h"

typedef union {
    // Every usable Consumer key possible, up to 4 keys presses possible
    uint16_t keys[4];
    struct {
        uint16_t key1;
        uint16_t key2;
        uint16_t key3;
        uint16_t key4;
    };
} HID_ConsumerControlReport_Data_t;


class BluefruitConsumerControl_ {
  public:
    void begin(void);
    void end(void);
    void write(uint16_t m);
    void press(uint16_t m);
    void release(uint16_t m);
    void releaseAll(void);

    // Sending is public in the base class for advanced users.
    void SendReport(void* data, int length);

  protected:
    HID_ConsumerControlReport_Data_t _report;
};

extern BluefruitConsumerControl_ BluefruitConsumerControl;
#ifndef USBCON
#define ConsumerControl BluefruitConsumerControl
#endif

#define KEY_BYTES 28
typedef union {
    // Modifiers + keymap
    struct {
        uint8_t modifiers;
        uint8_t keys[KEY_BYTES ];
    };
    uint8_t allkeys[1 + KEY_BYTES];
} HID_KeyboardReport_Data_t;

class BluefruitKeyboard_ {
  public:
    void begin(void);
    void end(void);

    size_t press(uint8_t k);
    size_t release(uint8_t k);
    void  releaseAll(void);
    int sendReport(void);

    boolean isModifierActive(uint8_t k);

  protected:
    HID_KeyboardReport_Data_t _keyReport;
};

extern BluefruitKeyboard_ BluefruitKeyboard;
#ifndef USBCON
#define Keyboard BluefruitKeyboard
#endif

typedef union {
    // Every usable system control key possible
    uint8_t key;
} HID_SystemControlReport_Data_t;


class BluefruitSystemControl_ {
  public:
    void begin(void);
    void end(void);
    void write(uint8_t s);
    void press(uint8_t s);
    void release(void);
    void releaseAll(void);
    void SendReport(void* data, int length);

};

extern BluefruitSystemControl_ BluefruitSystemControl;
#ifndef USBCON
#define SystemControl BluefruitSystemControl
#endif
