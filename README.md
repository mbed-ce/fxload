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

The last original version of fxload was published in 2008 by David Brownell at http://linux-hotplug.sourceforge.net.  This version is Linux only as it uses Linux-specific ioctls to talk to the device instead of libusb.

In 2010, Wolfgang Wieser created some patches to make assigning VIDs and PIDs easier [here](https://www.triplespark.net/elec/periph/USB-FX2/eeprom/) (these currently are not merged into this version).

In 2007, Claudio Favi did an [initial port](https://wiki.epfl.ch/cfavi/fxload-libusb) of fxload to libusb 0.1, allowing it to operate on Windows.  However, libusb 0.1 is deprecated, so this port can be difficult to use with modern systems.

In 2015, @tai (Taisuke Yamada) [adapted](https://github.com/tai/fxload-win32) the libusb0.1 version to libusb1.0, allowing it to work using a much better supported library.  However, the build system of this version only supported MinGW on Windows, making it very difficult to use on other platforms.

In 2023, the Mbed CE project is now making an updated version of this library which supports multiple platforms again, plus some quality of life changes.  These include:
- New CMake-based build system
- MS Visual Studio compiler support
- Github Actions CI jobs
- Rewritten, more robust CLI using the CLI11 library
- Improved error messages (esp. for failing to open USB devices)
- Improved device selection menu

## Code Installation

### Installer
On Windows, FXLoad can be installed via the Windows installer downloadable from the Releases page. (TODO)

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
sudo usermod -G plugdev $(whoami)
```
We will use ths group to control access to USB devices.  Next, sign out and back in again to make the change effective.

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

A Win32 + libusb-1.0 API port of fxload, programming tool for Cypress EZ-USB series.
