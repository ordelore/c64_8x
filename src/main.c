// Based on C64 emulator tutorial from https://github.com/eldendo/ED64
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <keypadc.h>
#include <fileioc.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "cpu.h"


/* Main function, called first */
int main(void)
{
    // clear screen
    os_ClrLCD();
    uint8_t kernal = ti_Open("C64KERN", "r");
    uint8_t basic = ti_Open("C64BASIC", "r");

    cpu_t *cpu = init_cpu(kernal, basic);
    ti_Close(kernal);
    ti_Close(basic);
    dbg_printf("loaded roms\n");
    dbg_printf("New pointer: %X\n", mem_peek2(cpu->memory, 0xFFFC));
    cpu_start(cpu);
    dbg_printf("initialized pc\n");
    dump_cpu(cpu);
    do {} while (!step_cpu(cpu));

    return 0;
}
