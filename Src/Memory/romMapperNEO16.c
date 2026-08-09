/*****************************************************************************
** NEO-16 mapper (MSXgl / aoineko.org)
**
** Based on the MSXgl NEO mapper spec and openMSX RomNeo16.
******************************************************************************
*/
#include "romMapperNEO16.h"
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

#define NEO16_PAGES 6

typedef struct {
    int deviceHandle;
    UInt8* romData;
    int romSize;
    int slot;
    int sslot;
    int startPage;
    UInt16 bankRegs[3];
} RomMapperNEO16;

int romMapperIsNEO16Rom(const void* romData, int size)
{
    const UInt8* rom = (const UInt8*)romData;

    if (size < 0x18) {
        return 0;
    }

    return memcmp(rom + 0x10, "ROM_NE16", 8) == 0;
}

static UInt16 msxAddress(RomMapperNEO16* rm, UInt16 address)
{
    return address + (UInt16)(rm->startPage << 13);
}

static int isBankRegisterWrite(UInt16 msxAddr)
{
    unsigned bbb = (msxAddr >> 11) & 7;

    return !((bbb < 2) || (bbb & 1));
}

static void mapRegion(RomMapperNEO16* rm, int region)
{
    UInt8* base;
    int page;

    if (region < 0 || region > 2) {
        return;
    }

    base = rm->romData + ((UInt32)rm->bankRegs[region] << 14);
    page = rm->startPage + region * 2;

    slotMapPage(rm->slot, rm->sslot, page,     base,         1, 0);
    slotMapPage(rm->slot, rm->sslot, page + 1, base + 0x2000, 1, 0);
}

static void mapAllRegions(RomMapperNEO16* rm)
{
    mapRegion(rm, 0);
    mapRegion(rm, 1);
    mapRegion(rm, 2);
}

static void updateBankRegister(RomMapperNEO16* rm, UInt16 msxAddr, UInt8 value)
{
    unsigned bbb = (msxAddr >> 11) & 7;
    int region = (int)((bbb >> 1) - 1);
    UInt16 oldReg;

    if (region < 0 || region > 2) {
        return;
    }

    oldReg = rm->bankRegs[region];

    if (msxAddr & 1) {
        rm->bankRegs[region] = (UInt16)((rm->bankRegs[region] & 0x00ff) | ((value & 0x0f) << 8));
    }
    else {
        rm->bankRegs[region] = (UInt16)((rm->bankRegs[region] & 0xff00) | value);
    }

    if (rm->bankRegs[region] != oldReg) {
        mapRegion(rm, region);
    }
}

static void saveState(void* rmv)
{
    RomMapperNEO16* rm = (RomMapperNEO16*)rmv;
    SaveState* state = saveStateOpenForWrite("mapperNEO16");

    saveStateSet(state, "bankReg0", rm->bankRegs[0]);
    saveStateSet(state, "bankReg1", rm->bankRegs[1]);
    saveStateSet(state, "bankReg2", rm->bankRegs[2]);
    saveStateClose(state);
}

static void loadState(void* rmv)
{
    RomMapperNEO16* rm = (RomMapperNEO16*)rmv;
    SaveState* state = saveStateOpenForRead("mapperNEO16");

    rm->bankRegs[0] = (UInt16)saveStateGet(state, "bankReg0", 0);
    rm->bankRegs[1] = (UInt16)saveStateGet(state, "bankReg1", 0);
    rm->bankRegs[2] = (UInt16)saveStateGet(state, "bankReg2", 0);
    saveStateClose(state);

    mapAllRegions(rm);
}

static void destroy(void* rmv)
{
    RomMapperNEO16* rm = (RomMapperNEO16*)rmv;

    slotUnregister(rm->slot, rm->sslot, rm->startPage);
    deviceManagerUnregister(rm->deviceHandle);

#ifndef MSX_NO_MALLOC
    free(rm->romData);
    free(rm);
#endif
}

static void reset(void* rmv)
{
    RomMapperNEO16* rm = (RomMapperNEO16*)rmv;

    rm->bankRegs[0] = 0;
    rm->bankRegs[1] = 0;
    rm->bankRegs[2] = 0;
    mapAllRegions(rm);
}

static void write(void* rmv, UInt16 address, UInt8 value)
{
    RomMapperNEO16* rm = (RomMapperNEO16*)rmv;
    UInt16 msxAddr = msxAddress(rm, address);

    if (isBankRegisterWrite(msxAddr)) {
        updateBankRegister(rm, msxAddr, value);
    }
}

int romMapperNEO16Create(const char* filename, UInt8* romData,
                         int size, int slot, int sslot, int startPage)
{
    DeviceCallbacks callbacks = { destroy, reset, saveState, loadState };
    RomMapperNEO16* rm;

    (void)filename;

#ifndef TARGET_GNW
    rm = malloc(sizeof(RomMapperNEO16));
#else
    rm = ahb_malloc(sizeof(RomMapperNEO16));
#endif

    if (rm == NULL) {
        return 0;
    }

    rm->deviceHandle = deviceManagerRegister(ROM_NEO16, &callbacks, rm);
    slotRegister(slot, sslot, startPage, NEO16_PAGES, NULL, NULL, write, destroy, rm);

#ifndef MSX_NO_MALLOC
    rm->romData = malloc(size);
    if (rm->romData == NULL) {
        destroy(rm);
        return 0;
    }
    memcpy(rm->romData, romData, size);
#else
    rm->romData = romData;
#endif

    rm->romSize = size;
    rm->slot = slot;
    rm->sslot = sslot;
    rm->startPage = startPage;
    rm->bankRegs[0] = 0;
    rm->bankRegs[1] = 0;
    rm->bankRegs[2] = 0;

    mapAllRegions(rm);

    return 1;
}
