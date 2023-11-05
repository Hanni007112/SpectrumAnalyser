# SpectrumAnalyser
An Arduino based spectrum analyser. Outputs to a 7x8 LED Grid

## Demo

[Demo Video](https://youtu.be/UyyZbMvOpg8)

## Controlls
- Switch: On/Off
- Button: cycles the modes (spectrum analyser --> white --> color --> spectrum analyser)
- Rotary encoder:
  - push & rotate: changes brigthness
  - rotate:
    - in mode spectrum analyser: changes the sensitivity
    - in mode white: changes how cold/warm the white is
    - in mode color: changes the hue

## Circuit
I've used the MSGEQ7, chip because i ran into performance issues whilst using the FFT method.

![circuit diagram](/doc/Circuit%20Diagram.png)

![led diagram](/doc/LED%20Diagram.png)

## Code

[Source Code](/doc/spectrumAnalyser_V3.ino)
