#include <keypadc.h>
#include <ti/getcsc.h>
#include "cpu.h"
#include "memory.h"


/*
sk_Del = 0x00
sk_Enter = 0x01
sk_Left = 0x02
sk_F4 = 0x03
sk_F1 = 0x04
sk_F2 = 0x05
sk_F3 = 0x06
sk_Up = 0x07
sk_3 = 0x08
sk_W = 0x09
sk_A = 0x0A
sk_4 = 0x0B

Z = 0x0C
S = 0x0D
E = 0x0E
(Unused) = 0x0F
5 = 0x10
R = 0x11
D = 0x12
6 = 0x13
C = 0x14
F = 0x15
T = 0x16
X = 0x17
7 = 0x18
Y = 0x19
G = 0x1A
8 = 0x1B
B = 0x1C
H = 0x1D
U = 0x1E
V = 0x1F
9 = 0x20
I = 0x21
J = 0x22
0 = 0x23
M = 0x24
K = 0x25
O = 0x26
N = 0x27
+ = 0x28
P = 0x29
L = 0x2A
− = 0x2B
> = 0x2C
[ = 0x2D
@ = 0x2E
< = 0x2F
£ = 0x30
* = 0x31
] = 0x32
Clr/Home = 0x33
(Unused) = 0x34
= = 0x35
↑ = 0x36
? = 0x37
1 = 0x38
← = 0x39
(Unused) = 0x3A
2 = 0x3B
Space = 0x3C
(Unused) = 0x3D
Q = 0x3E
Run/Stop = 0x3F
*/

// const char *c64_chars = {};
uint8_t ti_key_to_64_key(uint8_t key, uint8_t k_2nd, uint8_t k_alpha) {
    if (!(k_2nd || k_alpha)) {
        switch (key)
        {
            case(sk_Left): {return 157;}
            case(sk_Down): {return 17;}
            case(sk_Right): {return 29;}
            case(sk_Up): {return 145;}
            case(sk_Del): {return 0x20;}
            case(sk_Enter): {return 13;}
            case(sk_Sub): {return 87;}
            case(sk_Math): {return 65;}
            case(sk_2): {return 90;}
            case(sk_Ln): {return 83;}
            case(sk_Sin): {return 69;}
            case(sk_Mul): {return 82;}
            case(sk_Recip): {return 68;}
            case(sk_Prgm): {return 67;}
            case(sk_Cos): {return 70;}
            case(sk_4): {return 84;}
            case(sk_GraphVar): {return 88;}
            case(sk_1): {return 89;}
            case(sk_Tan): {return 71;}
            case(sk_Apps): {return 66;}
            case(sk_Power): {return 72;}
            case(sk_5): {return 85;}
            case(sk_6): {return 86;}
            case(sk_Square): {return 73;}
            case(sk_Comma): {return 74;}
            case(sk_Div): {return 77;}
            case(sk_LParen): {return 40;}
            case(sk_7): {return 79;}
            case(sk_Log): {return 78;}
            case(sk_8): {return 80;}
            case(sk_RParen): {return 76;}
            case(sk_Chs): {return 63;}
            case(sk_0): {return 160;}
            case(sk_9): {return 81;}
            case(sk_Vars): {return 88;}
            case(sk_Add): {return 34;}
            case(sk_3): {return 64;}
            case(sk_DecPnt): {return 58;}
            default: return 0;
        }
    }
    if (k_alpha && !k_2nd) {
        dbg_printf("Bananas!\n");
        switch (key)
        {
            case(sk_Left): {return 157;}
            case(sk_Down): {return 17;}
            case(sk_Right): {return 29;}
            case(sk_Up): {return 145;}
            case(sk_0): {return 48;}
            case(sk_1): {return 49;}
            case(sk_2): {return 50;}
            case(sk_3): {return 51;}
            case(sk_4): {return 52;}
            case(sk_5): {return 53;}
            case(sk_6): {return 54;}
            case(sk_7): {return 55;}
            case(sk_8): {return 56;}
            case(sk_9): {return 57;}
            case(sk_Chs): {return 63;}
            case(sk_Div): {return 47;}
            case(sk_Mul): {return 42;}
            case(sk_Sub): {return 45;}
            case(sk_Add): {return 43;}
            case(sk_Comma): {return 44;}
            case(sk_DecPnt): {return 46;}
            case(sk_LParen): {return 40;}
            case(sk_RParen): {return 41;}
            default: return 0;
        }
    }
    return 0;
}

uint8_t scankey(cpu_t *cpu) {
    kb_Scan();
    uint8_t buff = mem_peek(cpu->memory, 0xC6);
    uint8_t pressed_key = 0;
    uint8_t k_2nd = kb_Data[1] & kb_2nd;
    uint8_t k_alpha = kb_Data[2] & kb_Alpha;
    static uint8_t prev_key;
    for (uint8_t key = 1, group = 7; group; --group) {
        for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
            if ((kb_Data[group] & mask) && !((group == 1) && (mask == kb_2nd)) && !((group == 2) && (mask == kb_Alpha))) {
                pressed_key = key;
            }
        }
    }
    if (pressed_key && (prev_key != pressed_key) && (buff < 10)) {
        mem_poke(cpu->memory, 0x0277+buff, ti_key_to_64_key(pressed_key, k_2nd, k_alpha));
        mem_poke(cpu->memory, 0xC6, buff+1);
    }
    prev_key = pressed_key;
    if (kb_On) {
        return 1;
    } else {
        return 0;
    }
}

// $48 : ch := 145; //up
// $4b : ch := 157; //left
// $50 : ch :=  17; //down
// $4D : ch :=  29; //right
// 82 : ch := 148; //inst-del
