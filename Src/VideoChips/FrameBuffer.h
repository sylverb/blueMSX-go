/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/VideoChips/FrameBuffer.h,v $
**
** $Revision: 1.27 $
**
** $Date: 2009-07-18 14:35:59 $
**
** More info: http://www.bluemsx.com
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
#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "../Common/MsxTypes.h"

#ifdef WII
#define FB_MAX_LINE_WIDTH 544
#define FB_MAX_LINES      480
#elif TARGET_GNW
#define FB_MAX_LINE_WIDTH 320
#define FB_MAX_LINES      240
#else
#define FB_MAX_LINE_WIDTH 640
#define FB_MAX_LINES      480
#endif

typedef enum { INTERLACE_NONE, INTERLACE_ODD, INTERLACE_EVEN } InterlaceMode;

typedef void FrameBuffer;

typedef struct FrameBufferData FrameBufferData;

typedef enum {
    MIXMODE_INTERNAL = 1,
    MIXMODE_BOTH     = 2,
    MIXMODE_EXTERNAL = 4,
    MIXMODE_NONE     = 8
} FrameBufferMixMode;

void frameBufferSetFrameCount(int frameCount);

FrameBuffer* frameBufferGetViewFrame();
FrameBuffer* frameBufferGetDrawFrame();
FrameBuffer* frameBufferFlipViewFrame(int mixFrames);
FrameBuffer* frameBufferFlipDrawFrame();

void frameBufferSetScanline(int scanline);
int frameBufferGetScanline();

FrameBuffer* frameBufferGetWhiteNoiseFrame();
FrameBuffer* frameBufferDeinterlace(FrameBuffer* frameBuffer);
void frameBufferClearDeinterlace();

FrameBufferData* frameBufferDataCreate(int maxWidth, int maxHeight, int defaultHorizZoom);
void frameBufferDataDestroy(FrameBufferData* frameData);

void frameBufferSetActive(FrameBufferData* frameData);
void frameBufferSetMixMode(FrameBufferMixMode mode, FrameBufferMixMode mask);

FrameBufferData* frameBufferGetActive();

void frameBufferSetBlendFrames(int blendFrames);

#ifdef WII
#define BKMODE_TRANSPARENT 0x0020
#define videoGetColor(R, G, B) \
          ((((int)(R) >> 3) << 11) | (((int)(G) >> 3) << 6) | ((int)(B) >> 3))
#elif defined(PS2)
#define BKMODE_TRANSPARENT 0x0000
#define videoGetColor(R, G, B) \
          ((((int)(B) >> 3) << 10) | (((int)(G) >> 3) << 5) | ((int)(R) >> 3))
#else
#if defined(VIDEO_COLOR_TYPE_RGB565)
#define BKMODE_TRANSPARENT 0x0000
#define videoGetColor(R, G, B) \
		((((int)(R) >> 3) << 11) | (((int)(G) >> 2) << 5) | ((int)(B) >> 3))
#elif defined(VIDEO_COLOR_TYPE_RGBA5551)
#define BKMODE_TRANSPARENT 0x0001
#define videoGetColor(R, G, B) \
		((((int)(R) >> 3) << 11) | (((int)(G) >> 3) << 6) | (((int)(B) >> 3) << 1))
#elif defined(TARGET_GNW) // RRR|GGG|BB
#define BKMODE_TRANSPARENT 0x0000
#define videoGetColor(R, G, B) \
		((((int)(R) >> 5) << 5) | (((int)(G) >> 5) << 2) | ((int)(B) >> 6))
#define videoGetColorRGB565(R, G, B) \
		((((int)(R) >> 3) << 11) | (((int)(G) >> 2) << 5) | ((int)(B) >> 3))
#else // default is ARGB1555
#define BKMODE_TRANSPARENT 0x8000
#define videoGetColor(R, G, B) \
		((((int)(R) >> 3) << 10) | (((int)(G) >> 3) << 5) | ((int)(B) >> 3))
#endif // VIDEO_COLOR_TYPE

#endif // WII
#define videoGetTransparentColor() BKMODE_TRANSPARENT


// User implementation
Pixel* frameBufferGetLine(FrameBuffer* frameBuffer, int y);
#ifdef TARGET_GNW
Pixel16* frameBufferGetLine16(FrameBuffer* frameBuffer, int y);
#endif
int    frameBufferGetDoubleWidth(FrameBuffer* frameBuffer, int y);
void   frameBufferSetDoubleWidth(FrameBuffer* frameBuffer, int y, int val);
void   frameBufferSetInterlace(FrameBuffer* frameBuffer, int val);
void   frameBufferSetLineCount(FrameBuffer* frameBuffer, int val);
int    frameBufferGetLineCount(FrameBuffer* frameBuffer);
int    frameBufferGetMaxWidth(FrameBuffer* frameBuffer);


#endif
