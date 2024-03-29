#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
typedef struct mem {
    uint8_t *memorya;
    uint8_t *memoryb;
    uint8_t *basic_rom;
    uint8_t *kernal_rom;
    uint8_t *char_rom;
} mem_t;
void mem_poke(mem_t *mem, uint16_t address, uint8_t value);
uint8_t mem_peek(mem_t *mem, uint16_t address);
uint16_t mem_peek2(mem_t *mem, uint16_t address);
uint8_t vic_peek(mem_t *mem, uint16_t address);
#endif