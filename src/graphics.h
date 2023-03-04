#ifndef GRAPHICS_H
#define GRAPHICS_H
#include <stdint.h>
#include "memory.h"
void vic_text(mem_t *mem, uint16_t pos, uint8_t val);
void graphics_init();
void graphics_close();
#endif