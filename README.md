# Kaleidoscope based firmware for Wez's keyboard builds

Each keyboard has its own directory which in turn comprises of
two separate parts; the hardware library and the driver sketch.

The hardware library is consumed by the various kaleidoscope
components.  The driver sketch pulls in those components and
is the entry point for the overall keyboard firmware.

## Instructions

* Clone this repo, the first time:

```
./make.py iota --sync
```

* Subsequent builds:

```
./make.py iota
```

* To flash it:

```
./make.py iota --flash
```

## Adding a new keyboard

Make a new subdirectory and model it after the `iota` directory.

The `info.py` file defines arduino parameters for the board, as
well as the USB product and vendor ID information.

The `hw` directory should be an arduino library that implements
the keyboard matrix and scanner implementation.

The `keymap` directory holds the main keymap sketch; this is
where you will edit the key layout and macros.

You can then use `./make.py YOURNAME` to build your keyboard,
assuming that you named the directory `YOURNAME`.

## FQBN - Selecting the Arduino Board

The FQBN parameter specifies which MCU you're using.  How do you
find out what this parameter should be?

The easiest way is:

* Open the Arduino IDE
* Open preferences
* Check "Show verbose output during compilation"
* OK
* Create a new default sketch
* Use the board manager to select the board that you want
* Click the verify button
* Inspect the verbose output; you'll likely need to scroll to
  the right to find the command line parameters that are
  being passed.  You're looking for something like this:
  `-fqbn=teensy:avr:teensyLC:usb=serialhid,speed=48,opt=osstd,keys=en-us`
* Strip off the `-fqbn=` portion and you have your fqbn parameter

### Common FQBNs

* `teensy:avr:teensyLC:usb=serialhid,speed=48,opt=osstd,keys=en-us` - Teensy LC
* `adafruit:avr:feather32u4` - Adafruit Feather Atmega 32u4
* `adafruit:nrf52:feather52:debug=l0` - Adafruit Feather nRF52

