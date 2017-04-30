# Kaleidoscope based firmware for Wez's keyboard builds

Each keyboard has its own directory which in turn comprises of
two separate parts; the hardware library and the driver sketch.

The hardware library is consumed by the various kaleidoscope
components.  The driver sketch pulls in those components and
is the entry point for the overall keyboard firmware.

## Instructions

* Clone this repo, then:

```
git submodule sync --recursive
git submodule update --init --recursive
```


