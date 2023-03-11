#include "cpu.h"
#include "input.h"
#include <graphx.h>
#include <time.h>

#define HI_8(X) ((((X) & 0xF0) >> 4) & 0xF)
#define LO_8(X) ((X) & 0xF)
#define HI_16(X) ((((X) & 0xFF00) >> 8) & 0xFF)
#define LO_16(X) ((X) & 0xFF)

const uint8_t C = 0x01; //0000 0001
const uint8_t Z = 0x02; //0000 0010
const uint8_t I = 0x04; //0000 0100
const uint8_t D = 0x08; //0000 1000
const uint8_t B = 0x10; //0001 0000
const uint8_t V = 0x40; //0100 0000
const uint8_t N = 0x80; //1000 0000

const clock_t TIMER_STEP = 16;

cpu_t *init_cpu(uint8_t kern_fp, uint8_t basic_fp) {
    static cpu_t cpu;
    static mem_t memory;
    uint8_t rama_fp = ti_Open("C64RAMA", "w+");
    uint8_t ramb_fp = ti_Open("C64RAMB", "w+");
    ti_Resize(0x8000, rama_fp);
    ti_Resize(0x8000, ramb_fp);
    memory.memorya = (uint8_t *)ti_GetDataPtr(rama_fp);
    memory.memoryb = (uint8_t *)ti_GetDataPtr(ramb_fp);
    memory.basic_rom = (uint8_t *)ti_GetDataPtr(basic_fp);
    memory.kernal_rom = (uint8_t *)ti_GetDataPtr(kern_fp);
    cpu.memory = &memory;
    // if you want to enable tracing from the start of execution, set this to 1
    cpu.trace = 0;
    cpu.starttime = clock();
    cpu.timer = 0;
    return &cpu;
}

void cpu_starttrace(cpu_t *cpu) {
    cpu->trace = 1;
}
void cpu_stoptrace(cpu_t *cpu) {
    cpu->trace = 0;
}
void cpu_dump1(cpu_t *cpu) {
    dbg_printf("PC=%04hX IR=%02hhX",
                cpu->pc, cpu->ir);
}

void cpu_dump2(cpu_t *cpu) {
    dbg_printf(" A=%02hhX X=%02hhX Y=%02hhX S=%02hhX P=%d%d0%d%d%d%d%d\n",
                cpu->a, cpu->x, cpu->y, cpu->s, !!(cpu->p & N), !!(cpu->p & V), !!(cpu->p & B), !!(cpu->p & D), !!(cpu->p & I), !!(cpu->p & Z), !!(cpu->p & C));
}

void dump_cpu(cpu_t *cpu) {
    cpu_dump1(cpu);
    cpu_dump2(cpu);
}

void cpu_start(cpu_t *cpu) {
    cpu->pc = mem_peek2(cpu->memory, 0xFFFC);
}

// memory addressing functions
uint16_t cpu_imm(cpu_t *cpu) {
    uint16_t imm = cpu->pc;
    cpu->pc++;
    return imm;
}

uint8_t cpu_zp(cpu_t *cpu) {
    uint8_t zp = mem_peek(cpu->memory, cpu->pc);
    cpu->pc++;
    return zp;
}

uint8_t cpu_zpx(cpu_t *cpu) {
    uint8_t zpx = mem_peek(cpu->memory, cpu->pc) + cpu->x;
    cpu->pc++;
    return zpx;
}

uint8_t cpu_zpy(cpu_t *cpu) {
    uint8_t zpy = mem_peek(cpu->memory, cpu->pc) + cpu->y;
    cpu->pc++;
    return zpy;
}

uint16_t cpu_abs(cpu_t *cpu) {
    uint16_t abs = mem_peek2(cpu->memory, cpu->pc);
    cpu->pc += 2;
    return abs;
}

uint16_t cpu_absx(cpu_t *cpu) {
    uint16_t absx = mem_peek2(cpu->memory, cpu->pc) + cpu->x;
    cpu->pc += 2;
    return absx;
}

uint16_t cpu_absy(cpu_t *cpu) {
    uint16_t absy = mem_peek2(cpu->memory, cpu->pc) + cpu->y;
    cpu->pc += 2;
    return absy;
}

uint16_t cpu_ind(cpu_t *cpu) {
    uint16_t ind = mem_peek2(cpu->memory, mem_peek2(cpu->memory, cpu->pc));
    cpu->pc += 2;
    return ind;
}


uint16_t cpu_indx(cpu_t *cpu) {
    uint16_t indx = mem_peek2(cpu->memory, mem_peek(cpu->memory, cpu->pc) + cpu->x);
    cpu->pc++;
    return indx;
}

uint16_t cpu_indy(cpu_t *cpu) {
    uint16_t indy = mem_peek2(cpu->memory, mem_peek(cpu->memory, cpu->pc)) + cpu->y;
    cpu->pc++;
    return indy;
}

// flag functions
uint8_t flagset(cpu_t *cpu, uint8_t flag) {
    return flag & cpu->p;
}

void setflag(cpu_t *cpu, uint8_t flag, uint8_t status) {
    if (status) {
        cpu->p = cpu->p | flag;
    } else {
        cpu->p = cpu->p & ~flag;
    }
}
// stack operations
void cpu_push(cpu_t *cpu, uint8_t b) {
    mem_poke(cpu->memory, 0x100+cpu->s, b);
    cpu->s--;
}

uint8_t cpu_pull(cpu_t *cpu) {
    cpu->s++;
    return mem_peek(cpu->memory, 0x100+cpu->s);
}

// branching instructions
void cpu_bfs(cpu_t *cpu, uint8_t flag) {
    // this can be made branchfree by multiplying mem_peek by flagget()
    if (flagset(cpu, flag)) {
        cpu->pc = cpu->pc + ((int8_t) mem_peek(cpu->memory, cpu->pc)) + 1;
    } else {
        cpu->pc++;
    }
}

void cpu_bfc(cpu_t *cpu, uint8_t flag) {
    if (flagset(cpu, flag)) {
        cpu->pc++;
    } else {
        cpu->pc = cpu->pc + ((int8_t) mem_peek(cpu->memory, cpu->pc)) + 1;
    }
}

// cpu instructions
void cpu_lda(cpu_t *cpu, uint16_t addr) {
    cpu->a = mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_ldx(cpu_t *cpu, uint16_t addr) {
    cpu->x = mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->x == 0);
    setflag(cpu, N, cpu->x >= 0x80);
}

void cpu_ldy(cpu_t *cpu, uint16_t addr) {
    cpu->y = mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->y == 0);
    setflag(cpu, N, cpu->y >= 0x80);
}

void cpu_sta(cpu_t *cpu, uint16_t addr) {
    mem_poke(cpu->memory, addr, cpu->a);
}

void cpu_stx(cpu_t *cpu, uint16_t addr) {
    mem_poke(cpu->memory, addr, cpu->x);
}

void cpu_sty(cpu_t *cpu, uint16_t addr) {
    mem_poke(cpu->memory, addr, cpu->y);
}

void cpu_adc(cpu_t *cpu, uint16_t addr) {
    uint16_t h = cpu->a + mem_peek(cpu->memory, addr);
    if (flagset(cpu, C)) {
        h++;
    }
    cpu->a = h;
    setflag(cpu, C, h > 0xFF);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
    h += 0x80;
    setflag(cpu, V, (h > 0xFF) | (h < 0));
}

void cpu_jmp(cpu_t *cpu, uint16_t addr) {
    cpu->pc = addr;
}

void cpu_jsr(cpu_t *cpu, uint16_t addr) {
    cpu->pc--;
    cpu_push(cpu, HI_16(cpu->pc));
    cpu_push(cpu, LO_16(cpu->pc));
    cpu->pc = addr;
}

void cpu_and_(cpu_t *cpu, uint16_t addr) {
    cpu->a &= mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_asl_A(cpu_t *cpu) {
    setflag(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a << 1;
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_asl(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    setflag(cpu, C, b & 0x80);
    b = b << 1;
    mem_poke(cpu->memory, addr, b);
    setflag(cpu, Z, b == 0);
    setflag(cpu, N, b >= 0x80);
}

void cpu_bit(cpu_t *cpu, uint16_t addr) {
    uint8_t h = mem_peek(cpu->memory, addr);
    setflag(cpu, N, h & 0x80);
    setflag(cpu, V, h & 0x40);
    setflag(cpu, Z, (h & cpu->a) == 0);
}

void cpu_brk(cpu_t *cpu) {
    setflag(cpu, B,true);
    cpu->pc++;
    cpu_push(cpu, (uint8_t) HI_16(cpu->pc));
    cpu_push(cpu, (uint8_t) LO_16(cpu->pc));
    cpu->pc--;
    cpu_push(cpu, cpu->p);
    setflag(cpu, I, 0x1);
    cpu->pc = mem_peek2(cpu->memory, 0xFFFE);
}

void cpu_cmp(cpu_t *cpu, uint16_t addr) {
    uint16_t h = cpu->a - mem_peek(cpu->memory, addr);
    setflag(cpu, C, h <= 0xFF);
    setflag(cpu, Z, LO_16(h) == 0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_cpx(cpu_t *cpu, uint16_t addr) {
    uint16_t h = cpu->x - mem_peek(cpu->memory, addr);
    setflag(cpu, C, h <= 0xFF);
    setflag(cpu, Z, LO_16(h) == 0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_cpy(cpu_t *cpu, uint16_t addr) {
    uint16_t h = cpu->y - mem_peek(cpu->memory, addr);
    setflag(cpu, C, h <= 0xFF);
    setflag(cpu, Z, LO_16(h) == 0x0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_dec_(cpu_t *cpu, uint16_t addr) {
    uint16_t h = mem_peek(cpu->memory, addr) - 1;
    mem_poke(cpu->memory, addr, h);
    setflag(cpu, Z, LO_16(h) == 0x0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_dex(cpu_t *cpu) {
    cpu->x -= 1;
    setflag(cpu, Z, cpu->x == 0x00);
    setflag(cpu, N, cpu->x >= 0x80);
}

void cpu_dey(cpu_t *cpu) {
    uint16_t h = cpu->y - 1;
    cpu->y = h;
    setflag(cpu, Z, LO_16(h)== 0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_inc_(cpu_t *cpu, uint16_t addr) {
    uint16_t h = mem_peek(cpu->memory, addr) + 1;
    mem_poke(cpu->memory, addr, h);
    setflag(cpu, Z, LO_16(h) == 0);
    setflag(cpu, N, LO_16(h) >= 0x80);
}

void cpu_inx(cpu_t *cpu) {
    cpu->x += 1;
    setflag(cpu, Z, cpu->x == 0);
    setflag(cpu, N, cpu->x >= 0x80);
}

void cpu_iny(cpu_t *cpu) {
    cpu->y += 1;
    setflag(cpu, Z, cpu->y == 0);
    setflag(cpu, N, cpu->y >= 0x80);
}

void cpu_eor(cpu_t *cpu, uint16_t addr) {
    cpu->a ^= mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_lsr_A(cpu_t *cpu) {
    setflag(cpu, C, cpu->a & 0x01);
    cpu->a = cpu->a >> 1;
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, 0);
}

void cpu_lsr(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    setflag(cpu, C, b & 0x01);
    b = b >> 1;
    mem_poke(cpu->memory, addr, b);
    setflag(cpu, Z, b == 0);
    setflag(cpu, N, 0);
}

void cpu_ora(cpu_t *cpu, uint16_t addr) {
    cpu->a |= mem_peek(cpu->memory, addr);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_pla(cpu_t *cpu) {
    cpu->a = cpu_pull(cpu);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_rol_A(cpu_t *cpu) {
    uint8_t bit = flagset(cpu, C);
    setflag(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a << 1;
    if (bit) {
        cpu->a |= 0x1;
    }
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_rol(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    uint8_t bit = flagset(cpu, C);
    setflag(cpu, C, b & 0x80);
    b = b << 1;
    if (bit) {
        b |= 0x01;
    }
    mem_poke(cpu->memory, addr, b);
    setflag(cpu, Z, b == 0);
    setflag(cpu, N, b >= 0x80);
}

void cpu_ror_A(cpu_t *cpu) {
    uint8_t bit = flagset(cpu, C);
    setflag(cpu, C, cpu->a & 0x01);
    cpu->a = cpu->a >> 1;
    if (bit) {
        cpu->a |= 0x80;
    }
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_ror(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    uint8_t bit = flagset(cpu, C);
    setflag(cpu, C, b & 0x01);
    b = b >> 1;
    if (bit) {
        b |= 0x80;
    }
    mem_poke(cpu->memory, addr, b);
    setflag(cpu, Z, b == 0);
    setflag(cpu, N, b >= 0x80);
}

void cpu_rti(cpu_t *cpu) {
    cpu->p = cpu_pull(cpu);
    cpu->pc = cpu_pull(cpu);
    cpu->pc += cpu_pull(cpu) * 0x100;
}

void cpu_rts(cpu_t *cpu) {
    cpu->pc = cpu_pull(cpu);
    cpu->pc += cpu_pull(cpu) * 0x100 + 1;
}

void cpu_sbc(cpu_t *cpu, uint16_t addr) {
    uint16_t h = cpu->a - mem_peek(cpu->memory, addr);
    if (!flagset(cpu, C)) {
        h--;
    }
    cpu->a = h;
    setflag(cpu, C, h <= 0xFF);
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
    h += 0x80;
    setflag(cpu, V, (h > 0xFF) | (h < 0));
}

void cpu_tax(cpu_t *cpu) {
    cpu->x = cpu->a;
    setflag(cpu, Z, cpu->x == 0);
    setflag(cpu, N, cpu->x >= 0x80);
}

void cpu_tay(cpu_t *cpu) {
    cpu->y = cpu->a;
    setflag(cpu, Z, cpu->y == 0);
    setflag(cpu, N, cpu->y >= 0x80);
}

void cpu_tsx(cpu_t *cpu) {
    cpu->x = cpu->s;
    setflag(cpu, Z, cpu->x == 0);
    setflag(cpu, N, cpu->x >= 0x80);
}

void cpu_txa(cpu_t *cpu) {
    cpu->a = cpu->x;
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_tya(cpu_t *cpu) {
    cpu->a = cpu->y;
    setflag(cpu, Z, cpu->a == 0);
    setflag(cpu, N, cpu->a >= 0x80);
}

void cpu_txs(cpu_t *cpu) {
    cpu->s = cpu->x;
}

uint8_t cpu_irq(cpu_t *cpu) {
    if (!flagset(cpu, I)) {
        setflag(cpu, B,false);
        cpu_push(cpu, (uint8_t) HI_16(cpu->pc));
        cpu_push(cpu, (uint8_t) LO_16(cpu->pc));
        cpu_push(cpu, cpu->p);
        setflag(cpu, I, true);
        cpu->pc = mem_peek2(cpu->memory, 0xFFFE);
        if (scankey(cpu)) {
            return 1;
        }
    }
    cpu->timer += TIMER_STEP;
    return 0;
}

void cpu_nmi(cpu_t *cpu) {
    setflag(cpu, B,false);
    cpu_push(cpu, HI_16(cpu->pc));
    cpu_push(cpu, LO_16(cpu->pc));
    cpu_push(cpu, cpu->p);
    setflag(cpu, I, true);
    cpu->pc = mem_peek2(cpu->memory, 0xFFFA);
}

void cpu_reset(cpu_t *cpu) {
    cpu->pc = mem_peek2(cpu->memory, 0xFFFC);
    setflag(cpu, I, true);
}

uint8_t step_cpu(cpu_t *cpu) {
    // wait for the C64 to start scanning the keyboard before printing the trace
    // if (cpu->pc == 0xE5CD) {
    //     cpu_starttrace(cpu);
    // }
    cpu->ir = mem_peek(cpu->memory, cpu->pc);
    if (cpu->trace) {
        cpu_dump1(cpu);
    }
    cpu->pc++;
    switch (cpu->ir) {
    case(0x00): {cpu_brk(cpu); break;} //0x00
    case(0x01): {cpu_ora(cpu, cpu_indx(cpu)); break;} //0x01
    case(0x05): {cpu_ora(cpu, cpu_zp(cpu)); break;} //0x05
    case(0x06): {cpu_asl(cpu, cpu_zp(cpu)); break;} //0x06
    case(0x08): {cpu_push(cpu, cpu->p); break;} //0x08
    case(0x09): {cpu_ora(cpu, cpu_imm(cpu)); break;} //0x09
    case(0x0A): {cpu_asl_A(cpu); break;} //0x0A
    case(0x0D): {cpu_ora(cpu, cpu_abs(cpu)); break;} //0x0D
    case(0x0E): {cpu_asl(cpu, cpu_abs(cpu)); break;} //0x0E
    case(0x10): {cpu_bfc(cpu, N); break;} //bpl //0x10
    case(0x11): {cpu_ora(cpu, cpu_indy(cpu)); break;} //0x11
    case(0x15): {cpu_ora(cpu, cpu_zpx(cpu)); break;} //0x15
    case(0x16): {cpu_asl(cpu, cpu_zpx(cpu)); break;} //0x16
    case(0x18): {setflag(cpu, C, 0); break;} //clc //0x18
    case(0x19): {cpu_ora(cpu, cpu_absy(cpu)); break;} //0x19
    case(0x1D): {cpu_ora(cpu, cpu_absx(cpu)); break;} //0x1D
    case(0x1E): {cpu_asl(cpu, cpu_absx(cpu)); break;} //0x1E
    case(0x20): {cpu_jsr(cpu, cpu_abs(cpu)); break;} //0x20
    case(0x29): {cpu_and_(cpu, cpu_imm(cpu)); break;} //0x29
    case(0x21): {cpu_and_(cpu, cpu_indx(cpu)); break;} //0x21
    case(0x24): {cpu_bit(cpu, cpu_zp(cpu)); break;} //0x24
    case(0x25): {cpu_and_(cpu, cpu_zp(cpu)); break;} //0x25
    case(0x26): {cpu_rol(cpu, cpu_zp(cpu)); break;} //0x26
    case(0x28): {cpu_pull(cpu); break;} //plp //0x28
    case(0x2A): {cpu_rol_A(cpu); break;} //0x2A
    case(0x2C): {cpu_bit(cpu, cpu_abs(cpu)); break;} //0x2C
    case(0x2D): {cpu_and_(cpu, cpu_abs(cpu)); break;} //0x2D
    case(0x2E): {cpu_rol(cpu, cpu_abs(cpu)); break;} //0x2E
    case(0x30): {cpu_bfs(cpu, N); break;} //bmi //0x30
    case(0x31): {cpu_and_(cpu, cpu_indy(cpu)); break;} //0x31
    case(0x35): {cpu_and_(cpu, cpu_zpx(cpu)); break;} //0x35
    case(0x36): {cpu_rol(cpu, cpu_zpx(cpu)); break;} //0x36
    case(0x38): {setflag(cpu, C, 1); break;}     //sec //0x38
    case(0x39): {cpu_and_(cpu, cpu_absy(cpu)); break;} //0x39
    case(0x3D): {cpu_and_(cpu, cpu_absx(cpu)); break;} //0x3D
    case(0x3E): {cpu_rol(cpu, cpu_absx(cpu)); break;} //0x3E
    case(0x40): {cpu_rti(cpu); break;} //0x40
    case(0x41): {cpu_eor(cpu, cpu_indx(cpu)); break;} //0x41
    case(0x45): {cpu_eor(cpu, cpu_zp(cpu)); break;} //0x45
    case(0x46): {cpu_lsr(cpu, cpu_zp(cpu)); break;} //0x46
    case(0x48): {cpu_push(cpu, cpu->a); break;} //pha //0x48
    case(0x49): {cpu_eor(cpu, cpu_imm(cpu)); break;} //0x49
    case(0x4A): {cpu_lsr_A(cpu); break;} //0x4A
    case(0x4C): {cpu_jmp(cpu, cpu_abs(cpu)); break;} //0x4C
    case(0x4D): {cpu_eor(cpu, cpu_abs(cpu)); break;} //0x4D
    case(0x4E): {cpu_lsr(cpu, cpu_abs(cpu)); break;} //0x4E
    case(0x50): {cpu_bfc(cpu, V); break;} //bvc //0x50
    case(0x51): {cpu_eor(cpu, cpu_indy(cpu)); break;} //0x51
    case(0x55): {cpu_eor(cpu, cpu_zpx(cpu)); break;} //0x55
    case(0x56): {cpu_lsr(cpu, cpu_zpx(cpu)); break;} //0x56
    case(0x58): {setflag(cpu, I, 0); break;} //cli //0x58
    case(0x59): {cpu_eor(cpu, cpu_absy(cpu)); break;} //0x59
    case(0x5D): {cpu_eor(cpu, cpu_absx(cpu)); break;} //0x5D
    case(0x5E): {cpu_lsr(cpu, cpu_absx(cpu)); break;} //0x5E
    case(0x60): {cpu_rts(cpu); break;} //0x60
    case(0x61): {cpu_adc(cpu, cpu_indx(cpu)); break;} //0x61
    case(0x65): {cpu_adc(cpu, cpu_zp(cpu)); break;} //0x65
    case(0x66): {cpu_ror(cpu, cpu_zp(cpu)); break;} //0x66
    case(0x68): {cpu_pla(cpu); break;} //0x68
    case(0x69): {cpu_adc(cpu, cpu_imm(cpu)); break;} //0x69
    case(0x6A): {cpu_ror_A(cpu); break;} //0x6A
    case(0x6C): {cpu_jmp(cpu, cpu_ind(cpu)); break;} //0x6C
    case(0x6D): {cpu_adc(cpu, cpu_abs(cpu)); break;} //0x6D
    case(0x6E): {cpu_ror(cpu, cpu_abs(cpu)); break;} //0x6E
    case(0x70): {cpu_bfs(cpu, V); break;} //bvs //0x70
    case(0x71): {cpu_adc(cpu, cpu_indy(cpu)); break;} //0x71
    case(0x75): {cpu_adc(cpu, cpu_zpx(cpu)); break;} //0x75
    case(0x76): {cpu_ror(cpu, cpu_zpx(cpu)); break;} //0x76
    case(0x78): {setflag(cpu, I, 1); break;}     //sei //0x78
    case(0x79): {cpu_adc(cpu, cpu_absy(cpu)); break;} //0x79
    case(0x7D): {cpu_adc(cpu, cpu_absx(cpu)); break;} //0x7D
    case(0x7E): {cpu_ror(cpu, cpu_absx(cpu)); break;} //0x7E
    case(0x81): {cpu_sta(cpu, cpu_indx(cpu)); break;} //0x81
    case(0x84): {cpu_sty(cpu, cpu_zp(cpu)); break;} //0x84
    case(0x85): {cpu_sta(cpu, cpu_zp(cpu)); break;} //0x85
    case(0x86): {cpu_stx(cpu, cpu_zp(cpu)); break;} //0x86
    case(0x88): {cpu_dey(cpu); break;} //0x88
    case(0x8A): {cpu_txa(cpu); break;} //0x8A
    case(0x8C): {cpu_sty(cpu, cpu_abs(cpu)); break;} //0x8C
    case(0x8D): {cpu_sta(cpu, cpu_abs(cpu)); break;} //0x8D
    case(0x8E): {cpu_stx(cpu, cpu_abs(cpu)); break;} //0x8E
    case(0x90): {cpu_bfc(cpu, C); break;} //bcc //0x90
    case(0x91): {cpu_sta(cpu, cpu_indy(cpu)); break;} //0x91
    case(0x94): {cpu_sty(cpu, cpu_zpx(cpu)); break;} //0x94
    case(0x95): {cpu_sta(cpu, cpu_zpx(cpu)); break;} //0x95
    case(0x96): {cpu_stx(cpu, cpu_zpy(cpu)); break;} //0x96
    case(0x98): {cpu_tya(cpu); break;} //0x98
    case(0x99): {cpu_sta(cpu, cpu_absy(cpu)); break;} //0x99
    case(0x9A): {cpu_txs(cpu); break;} //0x9A
    case(0x9D): {cpu_sta(cpu, cpu_absx(cpu)); break;} //0x9D
    case(0xA0): {cpu_ldy(cpu, cpu_imm(cpu)); break;} //0xA0
    case(0xA1): {cpu_lda(cpu, cpu_indx(cpu)); break;} //0xA1
    case(0xA2): {cpu_ldx(cpu, cpu_imm(cpu)); break;} //0xA2
    case(0xA4): {cpu_ldy(cpu, cpu_zp(cpu)); break;} //0xA4
    case(0xA5): {cpu_lda(cpu, cpu_zp(cpu)); break;} //0xA5
    case(0xA6): {cpu_ldx(cpu, cpu_zp(cpu)); break;} //0xA6
    case(0xA8): {cpu_tay(cpu); break;} //0xA8
    case(0xA9): {cpu_lda(cpu, cpu_imm(cpu)); break;} //0xA9
    case(0xAA): {cpu_tax(cpu); break;} //0xAA
    case(0xAC): {cpu_ldy(cpu, cpu_abs(cpu)); break;} //0xAC
    case(0xAD): {cpu_lda(cpu, cpu_abs(cpu)); break;} //0xAD
    case(0xAE): {cpu_ldx(cpu, cpu_abs(cpu)); break;} //0xAE
    case(0xB0): {cpu_bfs(cpu, C); break;} //bcs //0xB0
    case(0xB1): {cpu_lda(cpu, cpu_indy(cpu)); break;} //0xB1
    case(0xB4): {cpu_ldy(cpu, cpu_zpx(cpu)); break;} //0xB4
    case(0xB5): {cpu_lda(cpu, cpu_zpx(cpu)); break;} //0xB5
    case(0xB6): {cpu_ldx(cpu, cpu_zpy(cpu)); break;} //0xB6
    case(0xB8): {setflag(cpu, V, 0); break;} //clv //0xB8
    case(0xB9): {cpu_lda(cpu, cpu_absy(cpu)); break;} //0xB9
    case(0xBA): {cpu_tsx(cpu); break;} //0xBA
    case(0xBC): {cpu_ldy(cpu, cpu_absx(cpu)); break;} //0xBC
    case(0xBD): {cpu_lda(cpu, cpu_absx(cpu)); break;} //0xBD
    case(0xBE): {cpu_ldx(cpu, cpu_absy(cpu)); break;} //0xBE
    case(0xC0): {cpu_cpy(cpu, cpu_imm(cpu)); break;} //0xC0
    case(0xC1): {cpu_cmp(cpu, cpu_indx(cpu)); break;} //0xC1
    case(0xC4): {cpu_cpy(cpu, cpu_zp(cpu)); break;} //0xC4
    case(0xC5): {cpu_cmp(cpu, cpu_zp(cpu)); break;} //0xC5
    case(0xC6): {cpu_dec_(cpu, cpu_zp(cpu)); break;} //0xC6
    case(0xC8): {cpu_iny(cpu); break;} //0xC8
    case(0xC9): {cpu_cmp(cpu, cpu_imm(cpu)); break;} //0xC9
    case(0xCA): {cpu_dex(cpu); break;} //0xCA
    case(0xCC): {cpu_cpy(cpu, cpu_abs(cpu)); break;} //0xCC
    case(0xCD): {cpu_cmp(cpu, cpu_abs(cpu)); break;} //0xCD
    case(0xCE): {cpu_dec_(cpu, cpu_abs(cpu)); break;} //0xCE
    case(0xD0): {cpu_bfc(cpu, Z); break;} //bne //0xD0
    case(0xD1): {cpu_cmp(cpu, cpu_indy(cpu)); break;} //0xD1
    case(0xD5): {cpu_cmp(cpu, cpu_zpx(cpu)); break;} //0xD5
    case(0xD6): {cpu_dec_(cpu, cpu_zpx(cpu)); break;} //0xD6
    case(0xD8): {setflag(cpu, D, 0); break;} //cld //0xD8
    case(0xD9): {cpu_cmp(cpu, cpu_absy(cpu)); break;} //0xD9
    case(0xDD): {cpu_cmp(cpu, cpu_absx(cpu)); break;} //0xDD
    case(0xDE): {cpu_dec_(cpu, cpu_absx(cpu)); break;} //0xDE
    case(0xE0): {cpu_cpx(cpu, cpu_imm(cpu)); break;} //0xE0
    case(0xE1): {cpu_sbc(cpu, cpu_indx(cpu)); break;} //0xE1
    case(0xE4): {cpu_cpx(cpu, cpu_zp(cpu)); break;} //0xE4
    case(0xE5): {cpu_sbc(cpu, cpu_zp(cpu)); break;} //0xE5
    case(0xE6): {cpu_inc_(cpu, cpu_zp(cpu)); break;} //0xE6
    case(0xE8): {cpu_inx(cpu); break;} //0xE8
    case(0xE9): {cpu_sbc(cpu, cpu_imm(cpu)); break;} //0xE9
    case(0xEC): {cpu_cpx(cpu, cpu_abs(cpu)); break;} //0xEC
    case(0xEA): {break;} //0xEA
    case(0xED): {cpu_sbc(cpu, cpu_abs(cpu)); break;} //0xED
    case(0xEE): {cpu_inc_(cpu, cpu_abs(cpu)); break;} //0xEE
    case(0xF0): {cpu_bfs(cpu, Z); break;} //beq //0xF0
    case(0xF1): {cpu_sbc(cpu, cpu_indy(cpu)); break;} //0xF1
    case(0xF5): {cpu_sbc(cpu, cpu_zpx(cpu)); break;} //0xF5
    case(0xF6): {cpu_inc_(cpu, cpu_zpx(cpu)); break;} //0xF6
    case(0xF8): {setflag(cpu, D, 1); break;}     //sed //0xF8
    case(0xF9): {cpu_sbc(cpu, cpu_absy(cpu)); break;} //0xF9
    case(0xFD): {cpu_sbc(cpu, cpu_absx(cpu)); break;} //0xFD
    case(0xFE): {cpu_inc_(cpu, cpu_absx(cpu)); break;} //0xFE
    default: return 1;
    }
    if (cpu->trace) {
        cpu_dump2(cpu);
    }

    if ((clock() - cpu->starttime) / CLOCKS_PER_SEC * 1000 > cpu->timer) {
        if (cpu_irq(cpu)) {
            return 1;
        }
    }

    return 0;
}