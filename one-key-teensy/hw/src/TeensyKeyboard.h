/*
Copyright 2017 Wez Furlong

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
