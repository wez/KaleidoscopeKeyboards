#include "BluefruitKeyboard.h"

void BluefruitConsumerControl_::begin() {}
void BluefruitConsumerControl_::end() {}

void BluefruitConsumerControl_::write(uint16_t m){}
void BluefruitConsumerControl_::press(uint16_t m){}
void BluefruitConsumerControl_::release(uint16_t m) {}
void BluefruitConsumerControl_::releaseAll(void) {}

// Sending is public in the base class for advanced users.
void BluefruitConsumerControl_::SendReport(void* data, int length) {
}

void BluefruitKeyboard_::begin(void){}
void BluefruitKeyboard_::end(void){}

size_t BluefruitKeyboard_::press(uint8_t k){return 0;}
size_t BluefruitKeyboard_::release(uint8_t k){return 0;}
void  BluefruitKeyboard_::releaseAll(void){}
int BluefruitKeyboard_::sendReport(void){return 0;}

boolean BluefruitKeyboard_::isModifierActive(uint8_t k) {return false;}

void BluefruitSystemControl_::begin(void) {}
void BluefruitSystemControl_::end(void){}
void BluefruitSystemControl_::write(uint8_t s){}
void BluefruitSystemControl_::press(uint8_t s){}
void BluefruitSystemControl_::release(void){}
void BluefruitSystemControl_::releaseAll(void){}
void BluefruitSystemControl_::SendReport(void* data, int length) {}

BluefruitConsumerControl_ BluefruitConsumerControl;
BluefruitKeyboard_ BluefruitKeyboard;
BluefruitSystemControl_ BluefruitSystemControl;
