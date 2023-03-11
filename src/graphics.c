#include "graphics.h"
#include "memory.h"
#include <graphx.h>
#include <debug.h>

const uint16_t Y_OFFSET = 20;

void graphics_init() {
    gfx_Begin();
    gfx_SetDrawBuffer();
    gfx_ZeroScreen();
    gfx_palette[0] = gfx_RGBTo1555(0,0,0);
    gfx_palette[1] = gfx_RGBTo1555(255,255,255);
    gfx_palette[2] = gfx_RGBTo1555(255,0,0);
    gfx_palette[3] = gfx_RGBTo1555(0,255,255);
    gfx_palette[4] = gfx_RGBTo1555(255,0,255);
    gfx_palette[5] = gfx_RGBTo1555(0,255,0);
    gfx_palette[6] = gfx_RGBTo1555(0,0,255);
    gfx_palette[7] = gfx_RGBTo1555(255,255,0);
    gfx_palette[8] = gfx_RGBTo1555(0xFF,0xA5,0x00);
    gfx_palette[9] = gfx_RGBTo1555(0xA5,0x2A,0x2A);
    gfx_palette[10] = gfx_RGBTo1555(0xff,0xcc,0xcb);
    gfx_palette[11] = gfx_RGBTo1555(64,64,64);
    gfx_palette[12] = gfx_RGBTo1555(128,128,128);
    gfx_palette[13] = gfx_RGBTo1555(0x90, 0xEE, 0x90);
    gfx_palette[14] = gfx_RGBTo1555(0xad,0xd8,0xe6);
    gfx_palette[15] = gfx_RGBTo1555(192,192,192);
}

void graphics_close() {
    gfx_End();
}

void vic_text(mem_t *mem, uint16_t pos, uint8_t val) {
    uint16_t x0 = (pos % 40) * 8;
    uint16_t y0 = (pos / 40) * 8 + Y_OFFSET;
    for (uint8_t x = 0; x < 8; x++) {
        for (uint8_t y = 0; y < 8; y++) {
            if (vic_peek(mem, 0x1000+val*8+y) & (0x80 >> x)) {
                gfx_SetColor(14);
            } else {
                gfx_SetColor(6);
            }
            gfx_SetPixel(x0+x, y0+y);
        }
    }
    gfx_BlitBuffer();
}