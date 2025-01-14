#ifndef __ezusb_H
#define __ezusb_H
/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2002 David Brownell (dbrownell@users.sourceforge.net)
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

#include <libusb.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

// If supported, define an attribute to mark functions as doing printf formatting
#ifdef __MINGW32__
// On mingw we need to specify that we're using gnu printf provided by ucrt
#define PRINTF_FORMAT_ATTRIBUTE  __attribute__ ((format (gnu_printf, 1, 2)))
#elif defined(_MSC_VER)
// No corresponding attribute for MSVC
#define PRINTF_FORMAT_ATTRIBUTE
#else
#define PRINTF_FORMAT_ATTRIBUTE  __attribute__ ((format (printf, 1, 2)))
#endif

// Utility function to print to stderr
void logerror(const char *format, ...) PRINTF_FORMAT_ATTRIBUTE;


/*
 * Enum to manage various EZ-USB chip types.
 */
typedef enum { NONE, AN21, FX, FX2, FX2LP } ezusb_chip_t;
extern const char *ezusb_name[];

/*
 * This function loads the firmware from the given file into RAM.
 * The file is assumed to be in Intel HEX format.  If fx2 is set, uses
 * appropriate reset commands.  Stage == 0 means this is a single stage
 * load (or the first of two stages).  Otherwise it's the second of
 * two stages; the caller preloaded the second stage loader.
 *
 * The target processor is reset at the end of this download.
 */
extern int ezusb_load_ram (libusb_device_handle *device, const char *path, ezusb_chip_t type, int stage);


/*
 * This function stores the firmware from the given file into EEPROM.
 * The file is assumed to be in Intel HEX format.  This uses the right
 * CPUCS address to terminate the EEPROM load with a reset command,
 * where FX parts behave differently than FX2 ones.  The configuration
 * byte is as provided here (zero for an21xx parts) and the EEPROM
 * type is set so that the microcontroller will boot from it.
 * 
 * The caller must have preloaded a second stage loader that knows
 * how to respond to the EEPROM write request.
 */
extern int ezusb_load_eeprom (
	libusb_device_handle	*dev,		/* usbfs device handle */
	const char *path,	/* path to hexfile */
	ezusb_chip_t type,	/* fx, fx2, an21 */
	int config		/* config byte for fx/fx2; else zero */
	);


/* Verbosity level from 0 (least verbose) to 3 (most verbose) */
extern int verbose;


#define USB_DIR_OUT                     0               /* to device */
#define USB_DIR_IN                      0x80            /* to host */



/*
 * $Log: ezusb.h,v $
 * Revision 1.1  2007/03/19 20:46:30  cfavi
 * fxload ported to use libusb
 *
 * Revision 1.3  2002/04/12 00:28:21  dbrownell
 * support "-t an21" to program EEPROMs for those microcontrollers
 *
 * Revision 1.2  2002/02/26 19:55:05  dbrownell
 * 2nd stage loader support
 *
 * Revision 1.1  2001/06/12 00:00:50  stevewilliams
 *  Added the fxload program.
 *  Rework root makefile and hotplug.spec to install in prefix
 *  location without need of spec file for install.
 *
 */

#ifdef __cplusplus
};
#endif

#endif
