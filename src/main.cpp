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

struct device_spec { int index; uint16_t vid, pid; int bus, port; };

//void usage(const char *argv0) {
//    char *p = strrchr(argv0, '/');
//    char *q = strrchr(argv0, '\\');
//
//    // basename of argv0
//    p = (p > q) ? p+1 : q+1;
//
//    fprintf(stderr, "Usage: %s [options] -I file [-c 0xC[02] -s loader]\n", p);
//    fprintf(stderr,
//            "Options:\n"
//            "  -D <dev>   : select device by vid:pid or bus.port\n"
//            "  -t <type>  : select type from (an21|fx|fx2|fx2lp)\n"
//            "  -I <file>  : program hex file\n"
//            "  -s <loader>: program stage1 loader to write a file into EEPROM\n"
//            "  -c <byte>  : program first byte of EEPROM with either 0xC0 or 0xC2\n"
//            "  -V         : show version\n"
//            "  -v         : show verbose message\n");
//    fprintf(stderr,
//            "Examples:\n"
//            "  // program fw.hex to the FIRST device with given vid\n"
//            "  $ %s -d 04b4:@0 -I fw.hex\n"
//            "\n"
//            "  // program fw.hex to the SECOND device at given bus\n"
//            "  $ %s -d 004.@1 -I fw.hex\n"
//            "\n"
//            "  // program vid:pid info to EEPROM\n"
//            "  $ %s -I vidpid.hex -c 0xC0 -s Vend_Ax.hex\n"
//            "\n"
//            "  // program whole firmware to EEPROM\n"
//            "  $ %s -I fwfile.hex -c 0xC2 -s Vend_Ax.hex\n", p, p, p, p);
//    exit(1);
//}

/*
 * Finds the correct USB device to open based on the provided device spec.
 * If wanted is nullptr, all USB devices are printed to the console and the user
 * can select which to use.
 */
libusb_device_handle *get_usb_device(struct device_spec *wanted) {
    libusb_device **list;
    libusb_device_handle *dev_h = NULL;

    libusb_init(NULL);

    // If in double-verbose mode or above, set libusb to debug log mode.
    libusb_set_option(nullptr, LIBUSB_OPTION_LOG_LEVEL, verbose >= 2 ? LIBUSB_LOG_LEVEL_DEBUG : LIBUSB_LOG_LEVEL_WARNING);

    libusb_device *found = NULL;
    int nr_found = 0;

    bool search_all = wanted == nullptr;
    
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
            if ((bus == wanted->bus && (port == wanted->port || wanted->port == 0)) ||
                (vid == wanted->vid && ( pid == wanted->pid  || wanted->pid  == 0))) {
                if (nr_found++ == wanted->index) {
                    found = dev;
                    break;
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
            printf("%d: Bus %03d Device %03d: ID %04X:%04X %s\n", i, bus, port, vid, pid, deviceDetailsString.c_str());
        }
    }

    if (search_all) {
        printf("Please select device to configure [0-%d]: ", static_cast<int>(nr - 1));
        fflush(NULL);

        int sel;
        std::cin >> sel;

        if(!std::cin)
        {
            logerror("Invalid input.\n");
            return nullptr;
        }

        if( sel < 0 || sel >= nr) {
            logerror("device selection out of bound: %d\n", sel);
            return NULL;
        }

        found = list[sel];
    }

    if (! found) {
        logerror("device not selected\n");
        return NULL;
    }
      
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

    // Find resources directory
    std::string app_install_dir = AppPaths::getExecutableDir();
    std::string stage1_loader = app_install_dir + AppPaths::PATH_SEP + ".." + AppPaths::PATH_SEP + "share" + AppPaths::PATH_SEP + "fxload" + AppPaths::PATH_SEP + "Vend_Ax.hex";

    // List of CLI options
    app.add_flag("-v,--verbose", verbose, "Verbose mode.  May be supplied up to 3 times for more verbosity."); // note: CLI11 will count the occurrences of a flag when you pass an integer variable to add_flag()
    app.add_option("-I,--ihex-path", ihex_path, "Hex file to program")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-t,--type", type, "Select device type (from AN21|FX|FX2|FX2LP)")
        ->required()
        ->transform(CLI::CheckedTransformer(DeviceTypeNames, CLI::ignore_case));
    app.add_option("-D,--device", device_spec_string, "Select device by vid:pid(@index) or bus.port(@index).  If not provided, all discovered USB devices will be displayed as options.");
    auto eeprom_first_byte_opt = app.add_option("-c,--eeprom-first-byte", eeprom_first_byte, "Value programmed to first byte of EEPROM to set chip behavior.  e.g. for FX2LP this should be 0xC0 or 0xC2")
        ->check(CLI::Range(static_cast<uint16_t>(std::numeric_limits<uint8_t>::min()), static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    auto eeprom_opt = app.add_flag("-e, --eeprom", load_to_eeprom, "Load the hex file to EEPROM via a 1st stage loader instead of directly to RAM")
        ->needs(eeprom_first_byte_opt);
    app.add_option("-s,--stage1", stage1_loader, "Path to the stage 1 loader file to use when flashing EEPROM.  Default: " + stage1_loader)
        ->needs(eeprom_opt)
        ->check(CLI::ExistingFile);

    CLI11_PARSE(app, argc, argv);

    // Handle CLI options
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
    int status;

    device = get_usb_device(device_spec_string.empty() ? nullptr : &spec);

    if (device == NULL) {
        logerror("No device to configure\n");
        return -1;
    }

    if (type == NONE) {
        type = FX; /* an21-compatible for most purposes */
    }

    if (load_to_eeprom) {
        /* first stage:  put loader into internal memory */
        if (verbose)
            logerror("1st stage:  load 2nd stage loader\n");
        status = ezusb_load_ram (device, stage1_loader.c_str(), type, 0);
        if (status != 0)
            return status;

        /* second stage ... write EEPROM  */
        status = ezusb_load_eeprom (device, ihex_path.c_str(), type, eeprom_first_byte);
        if (status != 0)
            return status;

    } else {
        /* single stage, put into internal memory */
        if (verbose)
            logerror("single stage:  load on-chip memory\n");
        status = ezusb_load_ram (device, ihex_path.c_str(), type, 0);
        if (status != 0)
            return status;
    }

    libusb_close(device);

    printf("Done.\n");

    exit(0);
}