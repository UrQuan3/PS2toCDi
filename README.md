# PS/2 Mouse to CDi adapter

Sourcecode for the PS2 mouse to CD-i adapter that I designed.  Code adapted from an earlier PS2toCDi project by danbrakeley, who used anarterb's SNEStoCDi project, which was adapted from Paul Hackmann's gamepad adapter code.  Ah, the power of opensource.

As some PS2 mice pull more power than the CD-i provides, I suggest having an external 5v power source.  Any USB charger will work nicely.  I don't suggest back-feeding the 5v into the CDi (connecting the USB 5v to the CDi 5v).  Seems like it could end poorly to me.

WARNING: USB to PS2 mouse adapters are not true adapters, they simply connect wires.  The USB mouse must know how to 'talk' PS2 or the adapter will not work.  In general, if the mouse did not come with a PS2 adapter, it will not work.

In a serious strangeness, it appears that the CD-i does not provide mouse acceleration.  Since I do not own a CD-i mouse to test with, I do not know if the mouse provided the accelration calculations, or if mouse movements were simply linear.  I couldn't stand it, so I added acceleration in the adapter.  If you do not like the acceleration or wish to adjust it, look for the lines below and change the 'Y' component.  Must be an integer, and 0 = no acceleration.

    ```text
    //X*v linear component, Y*(v^2) acceleration component
    vnew = (1 * v) + (2 * sq(v));
    ```

I believe the original project was licensed "CC-BY" creative commons attrribution, so that is the license for this as well.

Original post: version 1.0
Built and tested around the Arduino Nano Every


***ORIGINAL README BELOW***

## The backstory

I bought an old Philips CDi off ebay, but it has been hard to find affordable input devices for it. I eventually built a SNEStoCDi to allow using SNES controllers, which works great, but I still wanted a mouse, so I hacked this project together to allow a PS2 mouse to be used with with a CDi.

I started from the [SNEStoCDi](https://github.com/anarterb/SNEStoCDi) project. To add PS/2 Mouse support, I grabbed the [PS2-Mouse-Arduino](https://github.com/kristopher/PS2-Mouse-Arduino) library, ~~which needed a quick header fix to build on the lastest Arduino Software (1.6.12)~~ which works great as-is on Arduino 1.8.16 (if you previously used my fork of PS2-Mouse-Arduino, please move back to the main repo).

## Tech docs

To learn about the PS2 data protocol, I used Adam Chapweske's ["The PS/2 Mouse Interface"](http://www.computer-engineering.org/ps2mouse/).

To learn about the various CDi device protocols, I used some scans of Philips technical docs, which are included in [Paul Hackmann's CDi gamepad adapter source](https://web.archive.org/web/20121220052742/http://www.alpinecom.net/phackmann/cdiadapter/CDiGamepadAdapter.zip).

## Install and build

I've tested this with version 1.8.16 of the Arduino IDE on Windows, using an Arduino Micro.

1. Make sure you have a recent version of the [Arduino software](https://www.arduino.cc/en/software).
2. Get the latest version of the [PS2-Mouse-Arduino](https://github.com/kristopher/PS2-Mouse-Arduino) library in your `libraries` folder.
   - One way to do this is to `git clone` from `C:\Users\<username>\Documents\Arduino\libraries\`, for example:
   ```text
   > cd \Users\Dan\Documents\Arduino\libraries
   > git clone https://github.com/kristopher/PS2-Mouse-Arduino.git
   ```
3. Get the latest version of this repo:
   ```text
   > cd \Users\Dan\Documnets\Arduino
   > md Projects
   > git clone https://github.com/danbrakeley/PS2toCDi.git
   ```
4. Open the sketch in the Arduino IDE.
   ```text
   > cd PS2toCDi
   > start sketchPS2toCDi\sketchPS2toCDi.ino
   ```
5. Set the pin constants at the top of the source file to the pins you plan on using

At this point you should be able to click Verify and see that everything compiles, and then Upload to your Arduino.

For testing, you can disable the CDi logic by setting `ENABLE_CDI` to `false`, and then you can print all the PS/2 Mouse events to the serial monitor by setting `DEBUG_OUT_ENABLED` to `true`.

## Troubleshooting

- The mouse cursor on the CDi isn't moving
  - Double check the Arduino pins you are using for the PS/2 mouse and the CDi input match the pins set in the code.
  - You can drop the sketch into PS/2 mouse debugging mode by setting `ENABLE_CDI` to `false` and `DEBUG_OUT_ENABLED` to `true`, and then running the result with the serial monitor open
    - If moving the mouse and clicking the buttons doesn't print anything, then double check all the connections.
    - If it is printing, then double check all the connections between the arduino and the CDi.
  - If it all works when the Arduino is plugged into the PC's USB, but not when you disconnect from the PC and run it just on the CDi, then the CDi may not be producing enough power for the PS/2 Mouse you are using. Just use the USB connector to supply extra +5V, and see if that helps.

Please feel free to drop me a note or open an Issue if you hit something I didn't mention here, especially if you found the answer, and I can add it to this troubleshooting section!
