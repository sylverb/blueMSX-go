/*****************************************************************************
** GNW: YJK -> RGB565 via subsampled AHB lookup table (table-only hot path).
**
** msxYjkColorLo[32][32][32] in AHB (~64 KiB), filled once by msxYjkColorInit().
** Runtime: one indexed load — J and K quantized by >>1 (nearest even sample).
*****************************************************************************/
#include "VDP_MSX.h"
#include "FrameBuffer.h"
#include "gw_malloc.h"
#include <stdint.h>

#define MSX_YJK_JQ 32
#define MSX_YJK_KQ 32
#define MSX_YJK_LO_COUNT (32 * MSX_YJK_JQ * MSX_YJK_KQ)
#define MSX_YJK_LO_SIZE (MSX_YJK_LO_COUNT * (int)sizeof(Pixel16))

static Pixel16 *msxYjkColorLo;

#define MSX_YJK_IDX(y, J, K) \
    (((y) << 10) | (((J) >> 1) << 5) | ((K) >> 1))

static Pixel16 msxYjkColorSample(int y, int J, int K)
{
    int j = (J & 0x1f) - (J & 0x20);
    int k = (K & 0x1f) - (K & 0x20);
    int r = 255 * (y + j) / 31;
    int g = 255 * (y + k) / 31;
    int b = 255 * ((5 * y - 2 * j - k) / 4) / 31;

    if (r < 0) r = 0;
    else if (r > 255) r = 255;
    if (g < 0) g = 0;
    else if (g > 255) g = 255;
    if (b < 0) b = 0;
    else if (b > 255) b = 255;

    return videoGetColorRGB565(r, g, b);
}

#include <stdio.h>
void msxYjkColorInit(void)
{
    int y;
    int jq;
    int kq;

    msxYjkColorLo = ahb_only_malloc(MSX_YJK_LO_SIZE);
    if (msxYjkColorLo == NULL)
        return;

    for (y = 0; y < 32; y++) {
        for (jq = 0; jq < MSX_YJK_JQ; jq++) {
            for (kq = 0; kq < MSX_YJK_KQ; kq++)
                msxYjkColorLo[(y << 10) | (jq << 5) | kq] =
                    msxYjkColorSample(y, jq << 1, kq << 1);
        }
    }
}

Pixel16 msxYjkColorAt(int y, int J, int K)
{
    return msxYjkColorLo[MSX_YJK_IDX(y, J, K)];
}
