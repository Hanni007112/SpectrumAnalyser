# SpectrumAnalyser
A Arduino based spectrum analyser. outputs to a LED matrix

## Demo

todo: demo video

## Controlls
- Switch: On/Off
- Button: cycles the modes (spectrum analyser --> white --> color --> spectrum analyser)
- Rotary encoder:
  - push & rotate: changes brigthness
  - rotate:
    - in Mode spectrum analyser: changes the sensitivity
    - in Mode white: changes how cold/warm the white is
    - in mode color: changes the hue

## Circuit
I've used the MSGEQ7, chip because i ran into performance issue whilst using the FFT method.

todo: embed diagram

## Code
