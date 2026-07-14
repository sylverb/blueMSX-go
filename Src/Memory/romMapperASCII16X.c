/*****************************************************************************
** ASCII16-X mapper (grauw.nl ASCII-X FlashROM cartridge)
**
** Based on the openMSX RomAscii16X implementation and the ASCII16-X spec.
******************************************************************************
*/
#include "romMapperASCII16X.h"
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

#define ASCII16X_PAGES 8

typedef struct {
    int deviceHandle;
    UInt8* romData;
    int romSize;
    int slot;
    int sslot;
    int startPage;
    UInt16 bankRegs[2];
    int flashCmdLen;
} RomMapperASCII16X;

int romMapperIsASCII16XRom(const void* romData, int size)
{
    const UInt8* rom = (const UInt8*)romData;

    if (size < 0x18) {
        return 0;
    }

    return memcmp(rom + 0x10, "ASCII16X", 8) == 0;
}

static UInt16 msxAddress(RomMapperASCII16X* rm, UInt16 address)
{
    return address + (UInt16)(rm->startPage << 13);
}

static UInt32 romOffset(RomMapperASCII16X* rm, UInt16 msxAddr)
{
    UInt16 bank = rm->bankRegs[((msxAddr >> 14) & 1) ^ 1];

    return ((UInt32)bank << 14) | (msxAddr & 0x3fff);
}

static int isBankRegisterWrite(UInt16 msxAddr)
{
    return (msxAddr & 0x3fff) >= 0x2000;
}

static void mapBankReg(RomMapperASCII16X* rm, int index)
{
    static const int bank0Pages[4] = { 2, 3, 6, 7 };
    static const int bank1Pages[4] = { 0, 1, 4, 5 };
    const int* pages = index ? bank1Pages : bank0Pages;
    UInt8* base = rm->romData + ((UInt32)rm->bankRegs[index] << 14);
    int i;

    for (i = 0; i < 4; i += 2) {
        slotMapPage(rm->slot, rm->sslot, rm->startPage + pages[i],     base,         1, 0);
        slotMapPage(rm->slot, rm->sslot, rm->startPage + pages[i + 1], base + 0x2000, 1, 0);
    }
}

static void mapAllBanks(RomMapperASCII16X* rm)
{
    mapBankReg(rm, 0);
    mapBankReg(rm, 1);
}

static void updateBankRegister(RomMapperASCII16X* rm, UInt16 msxAddr, UInt8 value)
{
    int index = (msxAddr >> 12) & 1;
    UInt16 oldReg = rm->bankRegs[index];
    UInt16 newReg = (UInt16)((msxAddr & 0x0f00) | value);

    if (newReg == oldReg) {
        return;
    }

    rm->bankRegs[index] = newReg;
    mapBankReg(rm, index);
}

static void flashHandleWrite(RomMapperASCII16X* rm, UInt32 offset, UInt8 value)
{
    if (offset == 0x0aaa && value == 0xaa) {
        rm->flashCmdLen = 1;
        return;
    }

    if (rm->flashCmdLen == 1 && offset == 0x0555 && value == 0x55) {
        rm->flashCmdLen = 2;
        return;
    }

    if (rm->flashCmdLen == 2 && offset == 0x0aaa && value == 0xa0) {
        rm->flashCmdLen = 3;
        return;
    }

    if (rm->flashCmdLen == 3) {
        rm->flashCmdLen = 0;
        if (offset < (UInt32)rm->romSize) {
            rm->romData[offset] &= value;
        }
        return;
    }

    if (value == 0xf0) {
        rm->flashCmdLen = 0;
        return;
    }

    rm->flashCmdLen = 0;
}

static void saveState(void* rmv)
{
    RomMapperASCII16X* rm = (RomMapperASCII16X*)rmv;
    SaveState* state = saveStateOpenForWrite("mapperASCII16X");

    saveStateSet(state, "bankReg0", rm->bankRegs[0]);
    saveStateSet(state, "bankReg1", rm->bankRegs[1]);
    saveStateClose(state);
}

static void loadState(void* rmv)
{
    RomMapperASCII16X* rm = (RomMapperASCII16X*)rmv;
    SaveState* state = saveStateOpenForRead("mapperASCII16X");

    rm->bankRegs[0] = (UInt16)saveStateGet(state, "bankReg0", 0);
    rm->bankRegs[1] = (UInt16)saveStateGet(state, "bankReg1", 0);
    saveStateClose(state);

    mapAllBanks(rm);
}

static void destroy(void* rmv)
{
    RomMapperASCII16X* rm = (RomMapperASCII16X*)rmv;

    slotUnregister(rm->slot, rm->sslot, rm->startPage);
    deviceManagerUnregister(rm->deviceHandle);

#ifndef MSX_NO_MALLOC
    free(rm->romData);
    free(rm);
#endif
}

static void reset(void* rmv)
{
    RomMapperASCII16X* rm = (RomMapperASCII16X*)rmv;

    rm->bankRegs[0] = 0;
    rm->bankRegs[1] = 0;
    rm->flashCmdLen = 0;
    mapAllBanks(rm);
}

static void write(void* rmv, UInt16 address, UInt8 value)
{
    RomMapperASCII16X* rm = (RomMapperASCII16X*)rmv;
    UInt16 msxAddr = msxAddress(rm, address);
    UInt32 offset = romOffset(rm, msxAddr);

    flashHandleWrite(rm, offset, value);

    if (isBankRegisterWrite(msxAddr)) {
        updateBankRegister(rm, msxAddr, value);
    }
}

int romMapperASCII16XCreate(const char* filename, UInt8* romData,
                            int size, int slot, int sslot, int startPage)
{
    DeviceCallbacks callbacks = { destroy, reset, saveState, loadState };
    RomMapperASCII16X* rm;

    (void)filename;

#ifndef TARGET_GNW
    rm = malloc(sizeof(RomMapperASCII16X));
#else
    rm = itc_malloc(sizeof(RomMapperASCII16X));
#endif

    if (rm == NULL) {
        return 0;
    }

    rm->deviceHandle = deviceManagerRegister(ROM_ASCII16X, &callbacks, rm);
    slotRegister(slot, sslot, startPage, ASCII16X_PAGES, NULL, NULL, write, destroy, rm);

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
    rm->flashCmdLen = 0;

    mapAllBanks(rm);

    return 1;
}
