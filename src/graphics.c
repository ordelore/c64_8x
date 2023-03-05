#include "graphics.h"
#include "memory.h"
#include <graphx.h>
#include <debug.h>
void graphics_init() {
    gfx_Begin();
    gfx_SetMonospaceFont(8);
    gfx_SetTextBGColor(0);
    gfx_SetTextFGColor(0xFF);
    gfx_SetTextConfig(gfx_text_noclip);
    gfx_SetTextTransparentColor(0x01);
}
void graphics_close() {
    gfx_End();
}
void vic_text(mem_t *mem, uint16_t pos, uint8_t val) {
    const char charset[] =
                {'@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
                'P','Q','R','S','T','U','V','W','X','Y','Z','[','-',']','^','-',
                ' ','!','"','#','$','%','&','\"','(',')','*','+',',','-','.','/',
                '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
                '-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',
                '-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',
                ' ','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-',
                '-','-','-','-','-','-','-','-','-','-','-','-','-','-','-','-'
                };
    gfx_SetTextXY((pos % 40) * 8, (pos / 40) * 8);
    if (val < 0x80) {
        gfx_PrintChar(charset[val]);
    } else {
        gfx_PrintChar(charset[val-0x80]);
    }
}