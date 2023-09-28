![Keyboardmania Controller](https://github.com/has207/keyboardmania-midi-bridge/blob/ccfd1e9755eed3977eefdcadd358b050256f27e0/keyboardmania.jpg)

# keyboardmania-midi-bridge
Allows Keyboardmania USB controller (Model RU025) for Playstation 2 to be used as a MIDI device on PC.

## About the device
- idVendor: 0x0507 (Hosiden Corp)
- idProduct: 0x0010

Protocol appears to be generic HID protocol with length 8 payload, broken down as follows:
- first byte is always 0x3f
- bytes 2 - 5 are a simple bitmask representing keys/buttons pressed
- last 3 bytes are always 0x00

Bitmask values for each key:

```
Select: 00 40 00 00
Start:  00 00 40 00
Whl-Up: 00 00 00 20
Whl-Dn: 00 00 00 40

C3:     01 00 00 00
C#3:    02 00 00 00
D3:     04 00 00 00
D#3:    08 00 00 00
E3:     10 00 00 00
F3:     20 00 00 00
F#3:    40 00 00 00
G3:     00 01 00 00
G#3:    00 02 00 00
A3:     00 04 00 00
A#3:    00 08 00 00
B3:     00 10 00 00

C4:     00 20 00 00
C#4:    00 00 01 00
D4:     00 00 02 00
D#4:    00 00 04 00
E4:     00 00 08 00
F4:     00 00 10 00
F#4:    00 00 20 00
G4:     00 00 00 01
G#4:    00 00 00 02
A4:     00 00 00 04
A#4:    00 00 00 08
B4:     00 00 00 10
```

## Prerequisites
This program does not provide a MIDI interface directly but rather talks to an existing MIDI device. As a result, you will need to also
install a loopback MIDI driver, such as [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) or [loopBE1](https://www.nerds.de/en/loopbe1.html).
I only tested loopMIDI so will describe what you need in terms of that particular program.

## Running
Install loopMIDI and run it, then add a named loopback MIDI device, which by default will be called "loopMIDI Port" in the loopMIDI UI, but you can
give it whatever name you like, such as "Keyboardmania" so that it shows up that way in your DAW.

Next run keyboardmania-midi-bridge.exe, it will scan for the Keyboard first and wait for you to plug it in if it's not plugged in already. This device
tends to get stuck when PC is put into sleep mode so try to plug it in and out if it's not being detected right away. Once the keyboard is detected,
the program will prompt you to choose an existing MIDI device to use. If you're running loopMIDI and have created your MIDI port you should see it here
so choose the appropriate number and hit Enter. You should see the "Ready..." message at which point key presses on the keyboard will be routed to that
MIDI port. Connect that port in your DAW of choice and you should be good to go.

Select/Start and Wheel motions are programmed to send different MIDI control messages so feel free to map them to actions inside your DAW.

You can also run with -v flag for verbose, and the program will print key presses/releases to the console. This is disabled by default to minimize
latency but it's a cheap enough operation that I doubt it adds much delay.

## Building source
The program relies on [hidapi](https://github.com/signal11/hidapi) and [rtmidi](https://github.com/thestk/rtmidi) libraries. Both of those need to
be built separately and your project will need to ensure it's linking against hidapi.lib and rtmidi.lib as well as can see their respective
includes.
