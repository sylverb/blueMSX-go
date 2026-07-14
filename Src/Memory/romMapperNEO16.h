/*****************************************************************************
** NEO-16 mapper (MSXgl / aoineko.org)
******************************************************************************
*/
#ifndef ROMMAPPER_NEO16_H
#define ROMMAPPER_NEO16_H

#include "MsxTypes.h"

int romMapperNEO16Create(const char* filename, UInt8* romData,
                         int size, int slot, int sslot, int startPage);

int romMapperIsNEO16Rom(const void* romData, int size);

#endif
