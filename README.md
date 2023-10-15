# FXLoad

## What is FXLoad?

FXLoad is a tool for flashing firmware to the Anchor Chips/Cypress/Infineon EZ-USB series of USB interface microcontrollers.  It can flash firmware to RAM (not permanent, will be erased when the device is reset or repowered), or to an external EEPROM connected to the microcontroller.  Flashing firmware to the EEPROM causes the microcontroller to load it on bootup, giving the USB device a permanent "identity".

FXLoad supports the following chips (in order of most to least recent):
- [Cypress/Infineon FX2LP](https://www.infineon.com/cms/en/product/universal-serial-bus/usb-2.0-peripheral-controllers/ez-usb-fx2lp-fx2g2-usb-2.0-peripheral-controller/) (CY7C68013A) (`-t FX2LP`)
- [Cypress/Infineon FX2 (CY7C68013) (`-t FX2`)
- Cypress/Infineon FX (CY7C64613) (`-t FX`)
- Anchor Chips AN21 (AN2131SC) (`-t AN21`)

More info about the EZ-USB series of micros can be found [here](http://www.linux-usb.org/ezusb/).

## License & History

## Code Installation

## USB Setup

## Usage

A Win32 + libusb-1.0 API port of fxload, programming tool for Cypress EZ-USB series.
