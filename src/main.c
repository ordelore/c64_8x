// Based on C64 emulator tutorial from https://github.com/eldendo/ED64
#include <ti/screen.h>
#include <ti/getcsc.h>
#include <keypadc.h>
#include <fileioc.h>
#include <debug.h>

#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "graphics.h"

/* Main function, called first */
int main(void)
{
    // clear screen
    os_ClrLCD();
    ti_Close(3);
    uint8_t kernal = ti_Open("C64KERN", "r");
    uint8_t basic = ti_Open("C64BASIC", "r");
    uint8_t charset = ti_Open("C64CHAR", "r");

    cpu_t *cpu = init_cpu(kernal, basic, charset);

    cpu_start(cpu);
    graphics_init();
    do {} while (!step_cpu(cpu));
    dump_cpu(cpu);
    ti_Close(kernal);
    ti_Close(basic);
    graphics_close();

    return 0;
}
