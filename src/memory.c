#include "memory.h"
#include <debug.h>
void mem_poke(mem_t mem, uint16_t address, uint8_t value) {
    mem.memory[address] = value;
}
uint8_t io(uint16_t address) {
    return 0xFF;
}
uint8_t mem_peek(mem_t mem, uint16_t address) {
    if (address >= 0xE000) {
        return mem.kernal_rom[address - 0xE000];
    }
    if (address >= 0xDFFF) {
        return mem.memory[address];
    }
    if (address >= 0xD000) {
        return io(address);
    }
    if (address >= 0xBFFF) {
        return mem.memory[address];
    }
    if (address >= 0xA000) {
        return mem.basic_rom[address - 0xA000];
    }
    return mem.memory[address];
}

uint16_t mem_peek2(mem_t mem, uint16_t address) {
    return mem_peek(mem, address) + ((uint16_t) mem_peek(mem, address + 1)) * 256;
}
