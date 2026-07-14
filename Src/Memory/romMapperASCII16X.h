/*****************************************************************************
** ASCII16-X mapper (grauw.nl ASCII-X FlashROM cartridge)
******************************************************************************
*/
#ifndef ROMMAPPER_ASCII16X_H
#define ROMMAPPER_ASCII16X_H

#include "MsxTypes.h"

int romMapperASCII16XCreate(const char* filename, UInt8* romData,
                            int size, int slot, int sslot, int startPage);

int romMapperIsASCII16XRom(const void* romData, int size);

#endif
