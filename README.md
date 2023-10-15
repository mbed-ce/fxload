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

In 2007, Claudio Favi did an [initial port](https://wiki.epfl.ch/cfavi/fxload-libusb) of fxload to libusb 0.1, allowing it to operate on Windows.  However, libusb 0.1 is deprecated, so this port is difficult to use with modern systems.

In 2015, @tai (Taisuke Yamada) [adapted](https://github.com/tai/fxload-win32) the libusb0.1 version to libusb1.0, allowing it to work using a much better supported library.  However, the build system of this version only supported MinGW on Windows, making it very difficult to use on other platforms.

In 2023, the Mbed CE project is now making an updated version of this library which supports multiple platforms again, plus some quality of life changes.  These include:
- New CMake-based build system
- Github Actions CI jobs
- Rewritten, more robust CLI using the CLI11 library
- Improved error messages (esp. for failing to open USB devices)
- Bug fixed where 

## Code Installation

## USB Setup

## Usage

A Win32 + libusb-1.0 API port of fxload, programming tool for Cypress EZ-USB series.
