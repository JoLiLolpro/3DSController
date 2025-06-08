3DS~~Controller~~Mouse ![](/3DS/cxi/icon48x48.png?raw=true)
===
A 3DS homebrew application which allows you to use your 3DS touchscreen as a wireless mouse for Windows.
### About this fork
This fork is from [CTurt/3DSController](https://github.com/CTurt/3DSController) and delete all the features except the touchscreen one and make it more usable for playing osu! with a 3DS XL.

This fork implement smoothing for the cursor,
the active zone of the touchscreen is 1/4 of the screen (the bottom right corner) because the 3DSXL's screen is too big for me, and the blue rectangle on screen show the limits of the active zone.

TODO: Make the active zone completely customizable.

### Download
The latest release will always be downloadable from [here](https://github.com/JoLiLolpro/3DSController/releases).

You don't need vjoy for this fork.

### Setup and Usage
Extract the archive and copy the file in the `3DS` directory with the extension that applies to your loader: `3DSController.3dsx` for the homebrew app, or `3DSController.cia` for FBI or other titles managers, into your 3DS's SD card or flashcard's micro SD card.

Copy the file `3DS/3DSController.ini` to the root of your 3DS's SD card, and change the line that says `IP: 192.168.0.4` to match your computer's local IP.

If you are unsure of your local IP address, run `3DSController.exe` and it will tell you.

Run `3DSController.exe` on your computer. If you are prompted, make sure to allow it through your firewall.

Start the application on your 3DS, there is no GUI, it will automatically try to connect to the IP address you put in `3DSController.ini`.

If it wasn't able to read the IP from `3DSController.ini`, it will notify you and quit.

Otherwise, you should just see a black screen with blue lines, this is a good sign. To see if it works, touch the 3ds's bottom screen and see if your mouse move.

Press Start and Select to return to the Homebrew Loader/Home menu.

### Configuration
Find the line `Port: 8889` and change it to your desired port, do this for both the 3DS's `3DSController.ini` and the PC's `3DSController.ini`.

In the 3DS's `3DSController.ini` the screen is turned on by default, if you still want to completely turn off the screen like the original app does, then set `BackLight` to `0`.

In the PC's `3DSController.ini` there is a setting called `Smooth`, it determines how smooth the mouse movements will be (more smooth = slower movement) if recommend setting it to `1` if you want to play osu!

### Building

for the 3ds app, install devkitpro, open a cmd in the 3DS folder and type `make clean` then `make`

for the PC app you need msys2 and mingw32, open a cmd in the PC folder and type `mingw32 make clean` then `mingw32 make`

### Setup and Usage (Linux) Note that I don't know if this still works with my fork.
-For keyboard emulation
Follow the Windows instructions, but use `3DSController.py` instead of the EXE.

-For Joystick emulation, first, install [python-uinput](https://github.com/tuomasjjrasanen/python-uinput). BEWARE: The latest release of this library as of the writing of this tutorial is 0.10.2 which is broken for most updated systems. Download the master branch directly.

Make sure that uinput module is running. You can do it from cosole like so: `#!sudo modprobe uinput`

Then, follow the Windows instructions, but use `3DSController_gamepad.py` instead of the EXE.

May work on OS X too, but this is not tested.

### Configuration (Linux) Note that I don't know if this still works with my fork.
The configuration for the keyboard emulation is in `3DSController.py`, not the INI.

The configuration for the joystick emulation is in `3DSController_gamepad.py`, not the INI.

### General Troubleshooting
- Make sure that you are using the 3DS and PC application from the same release,
- Make sure your 3DS has internet access (turn on the switch on the side of the 3DS so that an orange light shows) and is on the same network as your PC,
- Make sure that the `3DSController.ini` is in the root of your 3DS's SD card (not flashcard micro SD),
- Make sure that the `3DSController.ini` has the local IP of your computer, not your public IP,
- Make sure your firewall isn't blocking the application,
- Try using a different port (change the port for both the 3DS and PC's .ini file),
DS
