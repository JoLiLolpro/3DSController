3DS~~Controller~~Mouse ![](/3DS/cxi/icon48x48.png?raw=true)
===
A 3DS homebrew application which allows you to use your 3DS touchscreen as a wireless mouse for Windows and Linux(touchscreen).

## About this fork
This fork is from [CTurt/3DSController](https://github.com/CTurt/3DSController) and delete all the features except the touchscreen one and make it more usable for playing osu! with a 3DS/3DS XL.

This fork implement smoothing for the cursor,
the active zone of the touchscreen is fully customizable because the 3DSXL's screen can be too big for playing osu, and the blue rectangle on screen show the limits of the active zone.
You can also manually input the PC IP with the 3DS's built-in keyboard.

## Download
The latest release will always be downloadable from [here](https://github.com/JoLiLolpro/3DSController/releases).

You don't need vjoy for this fork.

## Setup and Usage
Extract the archive and copy the file in the `3DS` directory with the extension that applies to your loader: `3DSController.3dsx` for the homebrew app, or `3DSController.cia` for FBI or other titles managers, into your 3DS's SD card or flashcard's micro SD card.

Copy the file `3DS/3DSController.ini` to the root of your 3DS's SD card, and change the line that says `IP: 192.168.0.1` to match your computer's local IP.

If you are unsure of your local IP address, run the PC app and it will tell you.

### for WINDOWS:

Run `3DSController.exe` on your computer. If you are prompted, make sure to allow it through your firewall.

Start the application on your 3DS, press A the use the IP you put in the settings file or press B the enter manually an ip.

now it should say `connecting to...`, it will automatically try to connect to the IP address you selected.

Otherwise, it should say `connected`, this is a good sign. To see if it works, touch the 3ds's bottom screen and see if your mouse move.

**Press Start to return to the Homebrew Loader/Home menu.**

### for LINUX:

**IMPORTANT INFORMATION:** While the Windows version emulates a mouse cursor, the Linux version emulates a touchscreen, which means that you will not see the cursor moving. I made this choice because it was the only way to play osu lazer in Linux (as far as I know). In osu you need to enable `input>touch` and `gameplay>show gameplay cursor during touch input`. **osu-wine will not work.**

You need to open your port with `sudo ufw allow <your-port>/udp`, by default the port is `8889` so type `sudo ufw allow 8889/udp`.

After that, open a terminal in the `PC` folder and type `./3DSController-linux`.

Start the application on your 3DS, press A the use the IP you put in the settings file or press B the enter manually an ip.

now it should say `connecting to...`, it will automatically try to connect to the IP address you selected.

Otherwise, it should say `connected`, this is a good sign. To see if it works, go [here](https://www.touchscreentest.com/) (or any touchscreen tester) on your pc, touch the 3ds's bottom screen and see if it detect something.

**Press Start to return to the Homebrew Loader/Home menu.**

## Configuration/Settings
If you want to channge the port, find the line `Port: 8889` and change it to your desired port, do this for both the 3DS's `3DSController.ini` and the PC's `settings.json`.

There is a `debug` option in `settings.json` and if you set it to `true` it will display the latency of the 3ds.

In the 3DS's `3DSController.ini` the screen is turned on by default, if you still want to completely turn off the screen like the original app does, then set `BackLight` to `0`.

In the PC's `settings.json` there is a setting called `Smooth`, it determines how smooth the mouse movements will be (more smooth = slower movement) I recommend setting it to `1` if you want to play osu!

By default the active zone is the whole 3ds bottom screen but you can change it by setting `Custom_Active_Zone` to `true`, the active zone is determined as a rectangle with a `Start_Coordinate` that corresponds to the top-left corner and a `End_Coordinate` that corresponds to the bottom-right corner.

The 3ds origin point (0;0) is in the top-left corner of the screen, and the screen resolution is 240*320.

## List of coordinates that matches the 16:9 aspect ratio

| **Location**| **Start point** | **End point** |
| ----------  | --------------- | ------------- |
|Total screen |      (0;60)      |     (320;240)     |
|Top Left     |      (0;0)      |     (208;117)     |
|Top Right    |      (112;0)      |     (320;117)     |
|Bottom Left  |      (0;123)      |     (208;240)     |
|Bottom Right |      (112;123)      |     (320;240)     |

## Building
For the 3ds app, install devkitpro, open a cmd/terminal in the 3DS folder and type `make clean` then `make`.

For the PC app you need msys2 and mingw32, open a cmd in the PC folder and type `mingw32 make clean` then `mingw32 make`.

For linux you need gcc.

## General Troubleshooting
- Make sure that you are using the 3DS and PC application from the same release,
- Make sure your 3DS has internet access (turn on the switch on the side of the 3DS so that an orange light shows) and is on the same network as your PC,
- Make sure that the `3DSController.ini` is in the root of your 3DS's SD card (not flashcard micro SD),
- Make sure that the `3DSController.ini` has the local IP of your computer, not your public IP,
- Make sure your firewall isn't blocking the application,
- Try using a different port (change the port for both the 3DS and PC's .ini file).
