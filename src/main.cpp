/*
 * Copyright (c) 2007 Claudio Favi (claudio.favi@epfl.ch)
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2001-2002 David Brownell (dbrownell@users.sourceforge.net)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "CLI/CLI.hpp"

#include "libusb.h"
#include "ezusb.h"
#include "fxload-version.h"
#include "ApplicationPaths.h"

struct device_spec { int index; bool searchByVidPid; uint16_t vid, pid; int bus, port; };

/*
 * Finds the correct USB device to open based on the provided device spec.
 * If wanted is nullptr, all USB devices are printed to the console and the user
 * can select which to use.
 * If listOnly is true, we just print the list of USB devices and then return without selecting one.
 */
libusb_device_handle * search_usb_devices(bool listOnly, struct device_spec *wanted) {
    libusb_device **list;
    libusb_device_handle *dev_h = NULL;

    libusb_init(NULL);


    libusb_device *found = NULL;
    int nr_found = 0;

    bool search_all = wanted == nullptr || listOnly;
    
    ssize_t nr = libusb_get_device_list(NULL, &list);
    for (int i = 0; i < nr; i++) {
        libusb_device *dev = list[i];

        struct libusb_device_descriptor desc;
        libusb_get_device_descriptor(dev, &desc);

        uint16_t  vid = desc.idVendor;
        uint16_t  pid = desc.idProduct;
        uint8_t   bus = libusb_get_bus_number(dev);
        uint8_t  port = libusb_get_port_number(dev);

        if(!search_all)
        {
            if(wanted->searchByVidPid)
            {
                if(vid == wanted->vid && ( pid == wanted->pid || wanted->pid == 0))
                {
                    if (nr_found++ == wanted->index) {
                        found = dev;
                        break;
                    }
                }
            }
            else
            {
                if (bus == wanted->bus && (port == wanted->port || wanted->port == 0))
                {
                    if (nr_found++ == wanted->index) {
                        found = dev;
                        break;
                    }
                }
            }
        }


        if (search_all) {
            libusb_device_handle *dh;

            std::string deviceDetailsString = "";

            if (libusb_open(dev, &dh) == LIBUSB_SUCCESS) {

                // Query as many details about the device are available
                unsigned char detailsBuffer[255];
                if(libusb_get_string_descriptor_ascii(dh, desc.iManufacturer, detailsBuffer, sizeof(detailsBuffer)) > 0)
                {
                    deviceDetailsString += " Mfgr: " + std::string(reinterpret_cast<char const *>(detailsBuffer));
                }
                if(libusb_get_string_descriptor_ascii(dh, desc.iProduct, detailsBuffer, sizeof(detailsBuffer)) > 0)
                {
                    deviceDetailsString += " Product: " + std::string(reinterpret_cast<char const *>(detailsBuffer));
                }
                if(libusb_get_string_descriptor_ascii(dh, desc.iSerialNumber, detailsBuffer, sizeof(detailsBuffer)) > 0)
                {
                    deviceDetailsString += " S/N: " + std::string(reinterpret_cast<char const *>(detailsBuffer));
                }
                libusb_close(dh);
            }
            else
            {
#if _WIN32
                deviceDetailsString = "<failed to open device, ensure WinUSB driver is installed>";
#else
                deviceDetailsString = "<failed to open device>";
#endif
            }
            printf("%d: Bus %03d Device %03d: ID %04x:%04x %s\n", i, bus, port, vid, pid, deviceDetailsString.c_str());
        }
    }

    if(listOnly)
    {
        // We just wanted to list them, so we're done now.
        libusb_free_device_list(list, 1);
        return nullptr;
    }

    if (search_all) {
        printf("Please select device to configure [0-%d]: ", static_cast<int>(nr - 1));
        fflush(NULL);

        int sel;
        std::cin >> sel;

        if(!std::cin)
        {
            logerror("Invalid input.\n");
            libusb_free_device_list(list, 1);
            return nullptr;
        }

        if( sel < 0 || sel >= nr) {
            logerror("device selection out of bound: %d\n", sel);
            libusb_free_device_list(list, 1);
            return NULL;
        }

        found = list[sel];
    }

    if (! found) {
        logerror("device not selected\n");
        libusb_free_device_list(list, 1);
        return NULL;
    }

    // If in double-verbose mode or above, set libusb to debug log mode.
    // This will help diagnose errors from opening the device.
    libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL, verbose >= 2 ? LIBUSB_LOG_LEVEL_DEBUG : LIBUSB_LOG_LEVEL_WARNING);
      
    int openRet = libusb_open(found, &dev_h);
    libusb_free_device_list(list, 1);

    if(openRet != 0)
    {
        logerror("libusb_open() failed: %s\n", libusb_error_name(openRet));
    }

    return dev_h;
}

int
parse_device_path(const std::string & device_path, struct device_spec *spec) {
    std::string::size_type colonIdx = device_path.find(':');
    std::string::size_type dotIdx = device_path.find('.');
    std::string::size_type atIndex = device_path.find('@');
    if (colonIdx != std::string::npos) {

        // Always expect 4 hex digits in each part
        if(colonIdx != 4 && device_path.size() < 9)
        {
            logerror("Invalid VID:PID device selector \"%s\"\n", device_path.c_str());
            return 1;
        }

        auto vid_string = device_path.substr(0, colonIdx);
        auto pid_string = device_path.substr(colonIdx + 1, (atIndex == std::string::npos ? std::string::npos : (atIndex - colonIdx - 1)));
        spec->vid = static_cast<uint16_t>(std::stoul(vid_string, nullptr, 16));
        spec->pid = static_cast<uint16_t>(std::stoul(pid_string, nullptr, 16));
        spec->searchByVidPid = true;
    }
    else if (dotIdx != std::string::npos)
    {
        if(dotIdx == 0 || dotIdx == device_path.size() - 1)
        {
            logerror("Invalid bus.port device selector \"%s\"\n", device_path.c_str());
            return 1;
        }
        auto bus_string = device_path.substr(0, dotIdx);
        auto port_string = device_path.substr(dotIdx + 1, (atIndex == std::string::npos ? std::string::npos : (atIndex - dotIdx - 1)));
        spec->bus = std::stoi(bus_string);
        spec->port = std::stoi(port_string);
        spec->searchByVidPid = false;
    }

    // Look for optional "@index" suffix
    if (atIndex != std::string::npos) {
        if(atIndex == device_path.size() - 1)
        {
            logerror("@ sign may not be placed at the end of device selector.\n");
        }
        spec->index = std::stoi(device_path.substr(atIndex));
    }

    return 0;
}

// Map of string names to enum values
const std::map<std::string, ezusb_chip_t> DeviceTypeNames
{
    {"AN21", AN21},
    {"FX", FX},
    {"FX2", FX2},
    {"FX2LP", FX2LP},
};


int main(int argc, char*argv[])
{
    CLI::App app{std::string(FXLOAD_VERSION_STR) + "\nA utility to load the EZ-USB family of microcontrollers over USB."};

    // Variables written to by CLI options
    std::string ihex_path;
    std::string device_spec_string;
    ezusb_chip_t type = NONE;
    int eeprom_first_byte = -1;
    bool load_to_eeprom = false;
    bool printVersion = false;

    // Find resources directory
    std::string app_install_dir = AppPaths::getExecutableDir();
    std::string stage1_loader = app_install_dir + AppPaths::PATH_SEP + ".." + AppPaths::PATH_SEP + "share" + AppPaths::PATH_SEP + "fxload" + AppPaths::PATH_SEP + "Vend_Ax.hex";

    // CLI options for fxload
    app.add_flag("-v,--verbose", verbose, "Verbose mode.  May be supplied up to 3 times for more verbosity."); // note: CLI11 will count the occurrences of a flag when you pass an integer variable to add_flag()
    app.add_flag("-V,--version", printVersion, "Print version and exit.");

    // Subcommands
    CLI::App * load_ram_subcommand = app.add_subcommand("load_ram", "Load a binary into file into the EZ-USB chip's RAM.");
    CLI::App * load_eeprom_subcommand = app.add_subcommand("load_eeprom", "Load a binary into file into the EZ-USB chip's EEPROM.");
    CLI::App * list_usb_subcommand = app.add_subcommand("list", "List all available USB devices and exit");

    // load_ram options
    load_ram_subcommand->add_option("-I,--ihex-path", ihex_path, "Hex file to program")
        ->required()
        ->check(CLI::ExistingFile);
    load_ram_subcommand->add_option("-t,--type", type, "Select device type (from AN21|FX|FX2|FX2LP)")
        ->required()
        ->transform(CLI::CheckedTransformer(DeviceTypeNames, CLI::ignore_case).description(""));
    load_ram_subcommand->add_option("-D,--device", device_spec_string,
                                    "Select device by vid:pid(@index) or bus.port(@index).  If not provided, all discovered USB devices will be displayed as options.");

    // load_eeprom options
    load_eeprom_subcommand->add_option("-I,--ihex-path", ihex_path, "Hex file to program")
        ->required()
        ->check(CLI::ExistingFile);
    load_eeprom_subcommand->add_option("-t,--type", type, "Select device type (from AN21|FX|FX2|FX2LP)")
        ->required()
        ->transform(CLI::CheckedTransformer(DeviceTypeNames, CLI::ignore_case).description(""));
    load_eeprom_subcommand->add_option("-D,--device", device_spec_string,
                                    "Select device by vid:pid(@index) or bus.port(@index).  If not provided, all discovered USB devices will be displayed as options.");
    load_eeprom_subcommand->add_option("-c,--control-byte", eeprom_first_byte, "Value programmed to first byte of EEPROM to set chip behavior.  e.g. for FX2LP this should be 0xC0 or 0xC2")
        ->check(CLI::Range(std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()).description(""));
    load_eeprom_subcommand->add_option("-s,--stage1", stage1_loader, "Path to the stage 1 loader file to use when flashing EEPROM.  Default: " + stage1_loader)
        ->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    // handle -V
    if(printVersion)
    {
        printf("%s\n", FXLOAD_VERSION_STR);
        return 0;
    }
    else
    {
        if(app.get_subcommands().size() == 0)
        {
            printf("Must specify a subcommand!  Run with --help for more information.\n");
            return 1;
        }
    }

    // Handle subcommands
    if(list_usb_subcommand->parsed())
    {
        search_usb_devices(true, nullptr);
        return 0;
    }
    else // load_ram or load_eeprom (all commands which open a USB device)
    {
        // Find USB device to operate on
        struct device_spec spec = {0};
        if(!device_spec_string.empty())
        {
            int parseResult = parse_device_path(device_spec_string, &spec);
            if(parseResult != 0)
            {
                return parseResult;
            }
        }

        libusb_device_handle *device;

        device = search_usb_devices(false, device_spec_string.empty() ? nullptr : &spec);

        if (device == NULL) {
            logerror("Failed to select device\n");
            return -1;
        }

        if(load_ram_subcommand->parsed())
        {
             /* single stage, put into internal memory */
            if (verbose)
                logerror("single stage:  load on-chip memory\n");
            int status = ezusb_load_ram (device, ihex_path.c_str(), type, 0);
            if(status != 0)
            {
                libusb_close(device);
                return status;
            }

        }
        else if(load_eeprom_subcommand->parsed())
        {
            /* first stage:  put loader into internal memory */
            if (verbose)
                logerror("1st stage:  load 2nd stage loader\n");
            int status = ezusb_load_ram (device, stage1_loader.c_str(), type, 0);
            if (status != 0)
            {
                libusb_close(device);
                return status;
            }

            /* second stage ... write EEPROM  */
            status = ezusb_load_eeprom (device, ihex_path.c_str(), type, eeprom_first_byte);
            if (status != 0)
            {
                libusb_close(device);
                return status;
            }
        }

        libusb_close(device);
        printf("Done.\n");
    }

    return 0;
}