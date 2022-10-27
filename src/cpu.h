#include <stdint.h>
#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include <fileioc.h>
#include "memory.h"
typedef struct cpu
{
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t s;
    uint8_t i;
    uint8_t ir;
    uint8_t p;
    uint16_t pc;
    mem_t *memory;
    uint8_t trace;
    uint16_t addr;
} cpu_t;
cpu_t *init_cpu(uint8_t kern_fp, uint8_t basic_fp);
uint8_t step_cpu(cpu_t *cpu);
void cpu_start(cpu_t *cpu);
void dump_cpu(cpu_t *cpu);
void load_sample_program(cpu_t *cpu);