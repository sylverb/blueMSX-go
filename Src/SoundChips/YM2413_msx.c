/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/SoundChips/YM2413.cpp,v $
**
** $Revision: 1.19 $
**
** $Date: 2007-05-23 09:41:56 $
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
#include "YM2413_msx.h"
#include "emu2413_msx.h"
#include "Board.h"
#include "SaveState.h"
#include "IoPort.h"
#include "MediaDb.h"
#include "DeviceManager.h"
#include "Language.h"


struct YM_2413 {
    Mixer* mixer;
    Int32  handle;

    OPLL*  ym2413;
    UInt8  address;
    Int32  buffer[AUDIO_MONO_BUFFER_SIZE];
};

#ifdef MSX_NO_MALLOC
YM_2413 ym2413_global;
#endif

#ifndef MSX_NO_SAVESTATE
void ym2413SaveState(YM_2413* ref)
{
    YM_2413* ym2413 = (YM_2413*)ref;
    SaveState* state = saveStateOpenForWrite("msxmusic");

//    saveStateSetBuffer(state, "regs", ym2413->registers, 256);

    saveStateClose(state);
}

void ym2413LoadState(YM_2413* ref)
{
/*    YM_2413* ym2413 = (YM_2413*)ref;
    SaveState* state = saveStateOpenForRead("msxmusic");

    saveStateGetBuffer(state, "regs", ym2413->registers, 256);

    saveStateClose(state);

    ym2413->ym2413->loadState();*/
}
#endif

void ym2413Reset(YM_2413* ref)
{
    YM_2413* ym2413 = (YM_2413*)ref;

    OPLL_reset(ym2413->ym2413);
}

void ym2413WriteAddress(YM_2413* ym2413, UInt8 address)
{
    ym2413->address = address & 0x3f;
}

void ym2413WriteData(YM_2413* ym2413, UInt8 data)
{
    if (mixerIsChannelTypeEnable(ym2413->mixer,MIXER_CHANNEL_MSXMUSIC)) {
        UInt32 systemTime = boardSystemTime();
        mixerSync(ym2413->mixer);
        OPLL_writeReg(ym2413->ym2413,ym2413->address, data);
    }
}

static Int32* ym2413Sync(void* ref, UInt32 count) 
{
    YM_2413* ym2413 = (YM_2413*)ref;
    if (mixerIsChannelTypeEnable(ym2413->mixer,MIXER_CHANNEL_MSXMUSIC)) {
        UInt32 i;
        for (int i = 0; i < count; i++) {
            ym2413->buffer[i] = OPLL_calc(ym2413->ym2413) << 6;
        }
    }
    return ym2413->buffer;
}

static char* regText(int d)
{
    static char text[5];
    sprintf(text, "R%.2x", d);
    return text;
}

void ym2413SetSampleRate(void* ref, UInt32 rate)
{
    YM_2413* ym2413 = (YM_2413*)ref;
    OPLL_setRate(ym2413->ym2413,rate);
}

YM_2413* ym2413Create(Mixer* mixer)
{
    YM_2413* ym2413;

#ifndef MSX_NO_MALLOC
    ym2413 = (YM_2413*)calloc(1, sizeof(YM_2413));
#else
    ym2413 = &ym2413_global;
    memset (ym2413,0,sizeof(YM_2413));
#endif

    ym2413->ym2413 = OPLL_new(3579545, mixerGetSampleRate(mixer));
    OPLL_reset(ym2413->ym2413);
    ym2413->mixer = mixer;

    ym2413->handle = mixerRegisterChannel(mixer, MIXER_CHANNEL_MSXMUSIC, 0, ym2413Sync, ym2413SetSampleRate, ym2413);

    OPLL_setRate(ym2413->ym2413,mixerGetSampleRate(mixer));

//	ym2413->ym2413->setVolume(32767 * 9 / 10);

    return ym2413;
}

void ym2413Destroy(YM_2413* ym2413) 
{
    mixerUnregisterChannel(ym2413->mixer, ym2413->handle);
    OPLL_delete(ym2413->ym2413);
#ifndef MSX_NO_MALLOC
    free(ym2413);
#endif
}