/*****************************************************************************
** $Source: /cygdrive/d/Private/_SVNROOT/bluemsx/blueMSX/Src/Memory/romMapperASCII16nf.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2008-03-30 18:38:42 $
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
#include "romMapperASCII16nf.h"
#include "MediaDb.h"
#include "SlotManager.h"
#include "DeviceManager.h"
#include "SaveState.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef TARGET_GNW
#include "gw_malloc.h"
#endif

//  Uses ASCII16 but in different adresses :
//  Know Game : Super Pierrot


typedef struct {
    int deviceHandle;
    UInt8* romData;
    int slot;
    int sslot;
    int startPage;
    UInt32 romMask;
    int romMapper[4];
} RomMapperASCII16nf;

static void saveState(void* rmv)
{
    RomMapperASCII16nf *rm = (RomMapperASCII16nf *)rmv;
    SaveState* state = saveStateOpenForWrite("mapperASCII16nf");
    char tag[16];
    int i;

    for (i = 0; i < 4; i++) {
        sprintf(tag, "romMapper%d", i);
        saveStateSet(state, tag, rm->romMapper[i]);
    }

    saveStateClose(state);
}

static void loadState(void* rmv)
{
    RomMapperASCII16nf *rm = (RomMapperASCII16nf *)rmv;
    SaveState* state = saveStateOpenForRead("mapperASCII16nf");
    char tag[16];
    int i;

    for (i = 0; i < 4; i++) {
        sprintf(tag, "romMapper%d", i);
        rm->romMapper[i] = saveStateGet(state, tag, 0);
    }

    saveStateClose(state);

    for (i = 0; i < 4; i += 2) {
        UInt8* bankData = rm->romData + (rm->romMapper[i] << 14);
        slotMapPage(rm->slot, rm->sslot, rm->startPage + i,     bankData, 1, 0);
        slotMapPage(rm->slot, rm->sslot, rm->startPage + i + 1, bankData + 0x2000, 1, 0);
    }
}

static void destroy(void* rmv)
{
    RomMapperASCII16nf *rm = (RomMapperASCII16nf *)rmv;
    slotUnregister(rm->slot, rm->sslot, rm->startPage);
    deviceManagerUnregister(rm->deviceHandle);

#ifdef MSX_NO_MALLOC
    free(rm->romData);
    free(rm);
#endif
}

static void write(void* rmv, UInt16 address, UInt8 value) 
{
    RomMapperASCII16nf *rm = (RomMapperASCII16nf *)rmv;
    int bank;

    address += 0x4000;

    if (address & 0x0800) {
        return;
    }

    bank = (address & 0x1000) >> 11;

    value &= rm->romMask;

    if (rm->romMapper[bank] != value) {
        UInt8* bankData = rm->romData + ((int)value << 14);
        
        rm->romMapper[bank] = value;

        slotMapPage(rm->slot, rm->sslot, rm->startPage + bank,     bankData, 1, 0);
        slotMapPage(rm->slot, rm->sslot, rm->startPage + bank + 1, bankData + 0x2000, 1, 0);
    }
}

int romMapperASCII16nfCreate(const char* filename, UInt8* romData, 
                             int size, int slot, int sslot, int startPage) 
{
    DeviceCallbacks callbacks = { destroy, NULL, saveState, loadState };
    RomMapperASCII16nf* rm;
    int i;

#ifdef TARGET_GNW
    rm = malloc(sizeof(RomMapperASCII16nf));
#else
    rm = itc_malloc(sizeof(RomMapperASCII16nf));
#endif

    rm->deviceHandle = deviceManagerRegister(ROM_ASCII16NF, &callbacks, rm);
    slotRegister(slot, sslot, startPage, 4, NULL, NULL, write, destroy, rm);

    size = (size + 0x3fff) & ~0x3fff;

#ifndef MSX_NO_MALLOC
    rm->romData = malloc(size);
    memcpy(rm->romData, romData, size);
#else
    rm->romData = romData;
#endif
    rm->romMask = size / 0x4000 - 1;
    rm->slot  = slot;
    rm->sslot = sslot;
    rm->startPage  = startPage;

    rm->romMapper[0] = 0;
    rm->romMapper[2] = 0;

    for (i = 0; i < 4; i += 2) {   
        slotMapPage(rm->slot, rm->sslot, rm->startPage + i,     rm->romData + rm->romMapper[i] * 0x2000, 1, 0);
        slotMapPage(rm->slot, rm->sslot, rm->startPage + i + 1, rm->romData + rm->romMapper[i] * 0x2000 + 0x2000, 1, 0);
    }

    return 1;
}
