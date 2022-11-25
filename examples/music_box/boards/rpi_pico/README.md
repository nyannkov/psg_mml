[English](README.md) | [日本語](README_ja.md)

YMZ294 and Raspberrypi Pico (rpi_pico).
The pins of YMZ294 and rpi_pico should correspond as follows.

|YMZ294|rpi_pico|remarks|
|--|--|--|
|/WR|GP15||
|/CS|GP15||
|A0|GP14||
|phiM|GP13|GP13 is used as a 4MHz PWM output.|
|D0|GP28||
|D1|GP27||
|D2|GP26||
|D3|GP22||
|D4|GP21||
|D5|GP18||
|D6|GP19||
|D7|GP20||

The PWM output of rpi_pico is input to the phiM pin (master clock input) of the YMZ294.
Since the PWM frequency is set to 4 MHz, the pins 4/6 (master clock frequency selection) are set High.

**Development environment**

Use [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk).

**Build**

The build command is as follows:

```bash
$ make -f build.mk
```
.

