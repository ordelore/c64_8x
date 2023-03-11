#include "memory.h"
#include "graphics.h"
#include <debug.h>
void mem_poke(mem_t *mem, uint16_t address, uint8_t value) {
    if (address >= 0x8000) {
        mem->memoryb[address - 0x8000] = value;
    } else {
        mem->memorya[address] = value;
    }
    if ((address <= 0x7e7) && (address >= 0x400)) {
        vic_text(mem, address - 0x400, value);
    }
}
uint8_t io(uint16_t address) {
    if (address == 0xD012)
    {
        return 0x00;
    }
    return 0xFF;
}

uint8_t mem_peek(mem_t *mem, uint16_t address) {
    if (address >= 0xE000) {
        return mem->kernal_rom[address - 0xE000];
    }
    if (address >= 0xD000) {
        return io(address);
    }
    if (address >= 0xBFFF) {
        return mem->memoryb[address - 0x8000];
    }
    if (address >= 0xA000) {
        return mem->basic_rom[address - 0xA000];
    }
    if (address >= 0x8000) {
        return mem->memoryb[address - 0x8000];
    } else {
        return mem->memorya[address];
    }
}

uint16_t mem_peek2(mem_t *mem, uint16_t address) {
    return mem_peek(mem, address) + ((uint16_t) mem_peek(mem, address + 1)) * 256;
}

uint8_t vic_peek(mem_t *mem, uint16_t address) {
    if (address < 0x1000) {
        return mem->memorya[address];
    } else if (address < 0x2000) {
        return mem->char_rom[address-0x1000];
    } else if (address < 0x8000) {
        return mem->memorya[address];
    } else if (address < 0x9000) {
        return mem->memoryb[address-0x8000];
    } else if (address < 0xA000) {
        return mem->char_rom[address-0x9000];
    } else {
        return mem->memoryb[address-0x8000];
    }
}
