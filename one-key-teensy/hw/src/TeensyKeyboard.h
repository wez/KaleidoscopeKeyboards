#pragma once
#include "HIDAliases.h"
#include <inttypes.h>
#include <stdint.h>
#include <sys/types.h>

class TeensyConsumerControl_ {
  public:
    void begin(void);
    void end(void);
    void write(uint16_t m);
    void press(uint16_t m);
    void release(uint16_t m);
    void releaseAll(void);
    void SendReport(void* data, int length);
};

extern TeensyConsumerControl_ TeensyConsumerControl;
#define ConsumerControl TeensyConsumerControl

class TeensyKeyboard_ {
  public:
    void begin(void);
    void end(void);

    size_t press(uint8_t k);
    size_t release(uint8_t k);
    void  releaseAll(void);
    int sendReport(void);
};

extern TeensyKeyboard_ TeensyKeyboard;
#define Keyboard TeensyKeyboard

class TeensySystemControl_ {
  public:
    void begin(void);
    void end(void);
    void write(uint8_t s);
    void press(uint8_t s);
    void release(void);
    void releaseAll(void);
    void SendReport(void* data, int length);
};

extern TeensySystemControl_ TeensySystemControl;
#define SystemControl TeensySystemControl
