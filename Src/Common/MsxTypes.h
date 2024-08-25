/*****************************************************************************
** File:        msxTypes.h
**
** Author:      Daniel Vik
**
** Description: Type definitions
**
** More info:   www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#ifndef BLUEMSX_TYPES
#define BLUEMSX_TYPES

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifdef __GNUC__
#define __int64 long long
#endif

#ifdef _WIN32
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR "/"
#endif

#ifndef TARGET_GNW
#define PROP_MAXPATH 512
#else
#define PROP_MAXPATH 128
#endif

/* Define double type for different targets
 */
#if defined(__x86_64__) || defined(__i386__) || defined _WIN32
typedef double DoubleT;
#else
typedef float DoubleT;
#endif


/* So far, only support for MSVC types
 */
typedef uint8_t          UInt8;
#ifndef __CARBON__
typedef uint16_t         UInt16;
typedef uint32_t         UInt32;
typedef unsigned __int64 UInt64;
#endif
typedef int8_t           Int8;
typedef int16_t          Int16;
typedef int32_t          Int32;

// Define color stuff

#if PIXEL_WIDTH==32

#define COLSHIFT_R  16
#define COLMASK_R   0xff
#define COLSHIFT_G  8
#define COLMASK_G   0xff
#define COLSHIFT_B  0
#define COLMASK_B   0xff

typedef UInt32 Pixel;

#elif PIXEL_WIDTH==8

// RRRGGGBB
#define COLSHIFT_R  5
#define COLMASK_R   0x07
#define COLSHIFT_G  2
#define COLMASK_G   0x07
#define COLSHIFT_B  0
#define COLMASK_B   0x03

typedef UInt8 Pixel;
#ifdef TARGET_GNW
// On the G&W, the YJK modes (MSX2+ Screen 10/11/12)
// can't be written in a temporary 8 bits framebuffer due
// to ram limitation. Because of that, the YJK modes are
// directly written as RGB565 in real framebuffer
// To be able to perform that, we need to manipulate a 16 bits
// pixel object that we are calling Pixel16
typedef UInt16 Pixel16;
#endif
#else

#ifdef VIDEO_COLOR_TYPE_RGB565
#define COLSHIFT_R  11
#define COLMASK_R   0x1F
#define COLSHIFT_G  5
#define COLMASK_G   0x3F
#define COLSHIFT_B  0
#define COLMASK_B   0x1F
#else
#define COLSHIFT_R  5
#define COLMASK_R   0x07
#define COLSHIFT_G  2
#define COLMASK_G   0x03
#define COLSHIFT_B  0
#define COLMASK_B   0x07
#endif

typedef UInt16 Pixel;

#endif

#ifdef __cplusplus
}
#endif


///
/// The following section contain target dependent configuration
/// It should probably be moved to its own file but for convenience
/// its kept here....
///


//
// Target dependent video configuration options
//

#if 0
// Should be enabled for IPHONE
#include <Config/VideoChips.h>

// Enable for displays that are only 320 pixels wide
#define MAX_VIDEO_WIDTH_320

// Skip overscan for CRT (e.g. Iphone)
#define CRT_SKIP_OVERSCAN

// Enable for 565 RGB displays
#define VIDEO_COLOR_TYPE_RGB565

// Enable for 565 RGB displays
#define VIDEO_COLOR_TYPE_RGB565

// Enable for 5551 RGBA displays
#define VIDEO_COLOR_TYPE_RGBA5551

// Provide custom z80 configuration
#define Z80_CUSTOM_CONFIGURATION

// For Iphone custom z80 configuration is in separate file:
#include <Config/Z80.h>


// Exclude embedded samples from build
#define NO_EMBEDDED_SAMPLES

#endif


// Placeholder for configuration definitions
// Targets that wish to disable features should create an ifdef block
// for that particular target with the appropriate definitions or
// add them as a pre-processor directive in the build system
#if 0


#define EXCLUDE_JOYSTICK_PORT_GUNSTICK
#define EXCLUDE_JOYSTICK_PORT_ASCIILASER
#define EXCLUDE_JOYSTICK_PORT_JOYSTICK
#define EXCLUDE_JOYSTICK_PORT_MOUSE
#define EXCLUDE_JOYSTICK_PORT_TETRIS2DONGLE
#define EXCLUDE_JOYSTICK_PORT_MAGICKEYDONGLE
#define EXCLUDE_JOYSTICK_PORT_ARKANOID_PAD



#define EXCLUDE_SPECIAL_GAME_CARTS
#define EXCLUDE_MSXMIDI
#define EXCLUDE_NMS8280DIGI
#define EXCLUDE_JOYREXPSG
#define EXCLUDE_OPCODE_DEVICES
#define EXCLUDE_SVI328_DEVICES
#define EXCLUDE_SVIMSX_DEVICES
#define EXCLUDE_FORTEII
#define EXCLUDE_OBSONET
#define EXCLUDE_NOWIND
#define EXCLUDE_DUMAS
#define EXCLUDE_MOONSOUND
#define EXCLUDE_PANASONIC_DEVICES
#define EXCLUDE_YAMAHA_SFG
#define EXCLUDE_ROM_YAMAHANET
#define EXCLUDE_SEGA_DEVICES
#define EXCLUDE_DISK_DEVICES
#define EXCLUDE_IDE_DEVICES
#define EXCLUDE_MICROSOL80
#define EXCLUDE_SRAM_MATSUCHITA
#define EXCLUDE_SRAM_S1985
#define EXCLUDE_ROM_S1990
#define EXCLUDE_ROM_TURBOR
#define EXCLUDE_ROM_F4DEVICE

#endif



#endif

