# FXLoad

## What is FXLoad?

FXLoad is a tool for flashing firmware to the Anchor Chips/Cypress/Infineon EZ-USB series of USB interface microcontrollers.  It can flash firmware to RAM (not permanent, will be erased when the device is reset or repowered), or to an external EEPROM connected to the microcontroller.  Flashing firmware to the EEPROM causes the microcontroller to load it on bootup, giving the USB device a permanent "identity".

FXLoad supports the following chips (in order of most to least recent):
- [Cypress/Infineon FX2LP](https://www.infineon.com/cms/en/product/universal-serial-bus/usb-2.0-peripheral-controllers/ez-usb-fx2lp-fx2g2-usb-2.0-peripheral-controller/) (CY7C68013A) (`-t FX2LP`)
- Cypress/Infineon FX2 (CY7C68013) (`-t FX2`)
- Cypress/Infineon FX (CY7C64613) (`-t FX`)
- Anchor Chips AN21 (AN2131SC) (`-t AN21`)

More info about the EZ-USB series of micros can be found [here](http://www.linux-usb.org/ezusb/).

## License & History

FXLoad is licensed under the GNU General Public License, Version 2.

The original version of fxload was published from 2001-2008 by David Brownell and others at http://linux-hotplug.sourceforge.net.  This version is Linux only as it uses Linux-specific ioctls to talk to the device instead of libusb.

In 2010, Wolfgang Wieser created some patches to make assigning VIDs and PIDs easier [here](https://www.triplespark.net/elec/periph/USB-FX2/eeprom/) (these currently are not merged into this version).

In 2007, Claudio Favi did an [initial port](https://wiki.epfl.ch/cfavi/fxload-libusb) of fxload to libusb 0.1, allowing it to operate on Windows.  However, libusb 0.1 is deprecated, so this port can be difficult to use with modern systems.

In 2015, @tai (Taisuke Yamada) [adapted](https://github.com/tai/fxload-win32) the libusb0.1 version to libusb1.0, allowing it to work using a much better supported library.  However, the build system of this version only supported MinGW on Windows, making it very difficult to use on other platforms.

In 2023, the Mbed CE project is now making an updated version of this program which supports multiple platforms again, plus some quality of life changes.  These include:
- New CMake-based build system
- MS Visual Studio compiler support
- Github Actions CI jobs
- Rewritten, more robust CLI using the CLI11 library
- Improved error messages (esp. for failing to open USB devices)
- Improved device selection menu

## Installing FXLoad

### Installer
On Windows, FXLoad can be installed via the Windows installer downloadable from the [Releases](https://github.com/mbed-ce/fxload/releases) page.

### Building from Source
FXLoad can be built from source using CMake and a C/C++ compiler.  The first step is to use Git to clone the project.  Make sure to pass "--recursive" to get the submodules:
```
git clone https://github.com/mbed-ce/fxload.git
```

Now, open a terminal in the project directory.  The first step will be to install a development version of libusb1.0.  Then, you can build and install the project like a normal CMake project.

#### On Windows MSVC:
(before doing this step make sure to install MSVC and CMake.  That's outside the scope of this guide...)

To install libusb, the easiest way I've found is to use vcpkg.  First, install vcpkg using its [install instructions](https://vcpkg.io/en/getting-started).  Then, run `C:\vcpkg\vcpkg.exe install libusb:x64-windows` to install libusb.

Now you can get build and install the project:
```
cd /path/to/fxload
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=C:\vcpkg\installed\x64-windows "-DCMAKE_INSTALL_PREFIX=<your install location>"
cmake --build .
cmake --build . --target install
```

Change `CMAKE_INSTALL_PREFIX` to point to where you want to install fxload.  Also note the CMAKE_PREFIX_PATH argument, which makes sure that CMake sees the libraries installed by vcpkg.

#### On Windows MinGW (msys2)
(before doing this step make sure to install [msys2](https://www.msys2.org/).  That's outside the scope of this guide...)

First we need to install the ucrt64 compiler, libusb, and build tools.  From any msys2 terminal:
```
pacman -S mingw-w64-ucrt-x86_64-libusb mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-cmake
```

Now, we can build.  From a ucrt64 terminal:
```
cd /path/to/fxload
mkdir build
cd build
cmake .. -GNinja "-DCMAKE_INSTALL_PREFIX=<your install location>"
ninja
ninja install
```

#### On Mac
(before doing this make sure to install homebrew and a C/C++ compiler, that's outside the scope of this guide...)

From a terminal:
```
brew install libusb ninja
cd /path/to/fxload
mkdir build
cd build
cmake .. -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local
ninja
sudo ninja install
```

#### On Ubuntu Linux
(before doing this make sure to install a C/C++ compiler, that's outside the scope of this guide...)

From a terminal:
```
sudo apt-get install cmake libusb-1.0-0-dev ninja-build
cd /path/to/fxload
mkdir build
cd build
cmake .. -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local
ninja
sudo ninja install
```

## USB Device Access
### On Windows
On Windows, fxload (and other libusb based programs) cannot see USB devices unless they have the "WinUSB" driver attached to them.

To set this up, you will need to use [Zadig](https://zadig.akeo.ie/).  Simply run this program, find whichever USB device represents the EZ-USB microcontroller (you might have to look at the VID & PID values), and install the WinUSB driver for it.

Also note that [NirSoft USBLogView](https://www.nirsoft.net/utils/usb_log_view.html) is extremely useful for answering the question of "what are the VID & PID of the USB device I just plugged in".

### On Linux
On Linux, the kernel does not let ordinary users access USB devices without configuration.  So, you have two options.  For a quick & dirty solution, just run `fxload` as root.  This will allow it to access all USB devices.

For a better solution, we will need to follow a few more steps.

First, add your user to the "plugdev" group:
```
sudo usermod -a -G plugdev $(whoami)
```
We will use ths group to control access to USB devices.  After running this, sign out and back in again to make the change effective.

Second, run `sudo vim /etc/udev/rules.d/99-ezusb.rules` to create a new rules file with the following content:
```
ATTRS{idVendor}=="xxxx", ATTRS{idProduct}=="yyyy", MODE="660", GROUP="plugdev", TAG+="uaccess"
```
(where xxxx and yyyy are the VID and PID of the microcontroller, which you can get from `lsusb`)

For example, for an unconfigured FX2LP device, it would look like: 
```
ATTRS{idVendor}=="04b4", ATTRS{idProduct}=="8613", MODE="660", GROUP="plugdev", TAG+="uaccess"
```

Finally, run the following commands to reload the rules and make them active:
```
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Now, you should be able to work with EZ-USB micros without root access!  Just remember to add another entry to the rules file if you change the VID & PID of the device.

## Using FXLoad

### Specifying the Device To Load
This version of fxload allows you to specify the device to connect to in three different ways.

1. You may not specify the `--device` argument at all, in which case fxload will present a menu of all the available USB devices connected to the machine.  You can select the one to load from the menu.  This mode is ideal for interactive usage.
2. You may specify `--device <vid>:<pid>` to select a device by its vendor ID and hardware ID (in hexadecimal).  For example, to flash an unconfigured FX2LP, you would pass `--device 04b4:8613`.  By default, this will select the first such device found, but you can change that by adding `@N` after the vid and pid to use the Nth device found (where N is the 0-indexed index of the device to use).
3. You may specify `--device <bus>.<port>` to select a device by its bus and device number, specified as decimal numbers.  You can get the bus and device numbers from `lsusb` on Linux, though I'm not aware of a utility to list them on Windows.

To list available devices, run the command
```
fxload list
```
This will list out all available USB devices on your system, so you can pick which device to use.

### Loading a Hex File to RAM
To just load a firmware file into RAM, use a command like:
```sh
$ fxload load_ram --ihex-path <path/to/firmware.hex> -t FX2LP
``` 

(the -t argument may be changed to "FX2", "FX", or "AN21" as appropriate)

Since you are loading to RAM, this method of loading firmware will only last until the device is reset, which is useful for testing firmware builds!

### Loading a Hex File to EEPROM

**Warning: This process can soft-brick your device if you load invalid firmware.  See the "unbricking" section below for more details.**

To load a hex file into EEPROM, use a command line:
```sh
$ fxload load_eeprom --ihex-path <path/to/firmware.hex> -t FX2LP --control-byte 0xC2
```

The `--control-byte` argument gives the value for the command byte (the first byte of the device EEPROM).  The value to use here changes based on the device.  For FX2LP, 0xC2 causes the device to boot from EEPROM, and 0xC0 causes the device to load the VID, PID, and DID from the EEPROM.

Note: You may need to reset the chip before the new firmware will load.

### Loading Only VID, PID, and DID values to EEPROM

Unlike loading an entire firmware file, doing this will cause the EZ-USB chip to enumerate in its default bootup state with no code, but with custom VID, PID, and DID values for your application.  For this mode, use the same command as above but change the command byte for your device to 0xC0, then pass a hex file containing the VID, PID, and DID values in the correct binary format.

TODO need to create an example for how to do this...

### Unbricking
If you load firmware onto the EEPROM which does not properly boot up, your device may be soft-bricked -- you might be unable to flash firmware onto it normally.  In this situation, the easiest way to recover is to use a jumper wire to short the EEPROM's SCL or SDA pin to GND, then turn on the power.  This will force the I2C bus into the low state, preventing the EZ-USB from reading its firmware and making it boot up as an unconfigured device.  Then, remove the jumper and flash the code again.

This shouldn't hurt the device because I2C is an open drain bus, where chips only write 0 to the bus lines or leave them high impedance, so nothing will be writing 1 to the bus and causing a short.

Some dev boards also provide jumpers which can be used to disconnect the EEPROM from the FX2LP.  These will also work for unbricking it -- just remove them and turn on the power, then plug them back in and reflash new firmware.

More info about unbricking can be found [here](https://www.triplespark.net/elec/periph/USB-FX2/eeprom/).
