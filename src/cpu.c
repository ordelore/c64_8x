#include "cpu.h"
const uint8_t C = 0x01; //0000 0001
const uint8_t Z = 0x02; //0000 0010
const uint8_t I = 0x04; //0000 0100
const uint8_t D = 0x08; //0000 1000
const uint8_t B = 0x10; //0001 0000
const uint8_t V = 0x40; //0100 0000
const uint8_t N = 0x80; //1000 0000

cpu_t *init_cpu(uint8_t kern_fp, uint8_t basic_fp) {
    cpu_t *cpu = (cpu_t *)calloc(1, sizeof(cpu_t));
    static mem_t memory;
    uint8_t rama_fp = ti_Open("C64RAMA", "w+");
    uint8_t ramb_fp = ti_Open("C64RAMB", "w+");
    ti_Resize(0x8000, rama_fp);
    ti_Resize(0x8000, ramb_fp);
    memory.memorya = (uint8_t *)ti_GetDataPtr(rama_fp);
    memory.memoryb = (uint8_t *)ti_GetDataPtr(ramb_fp);
    memory.basic_rom = (uint8_t *)ti_GetDataPtr(basic_fp);
    memory.kernal_rom = (uint8_t *)ti_GetDataPtr(kern_fp);
    cpu->memory = &memory;
    cpu->trace = 0;
    return cpu;
}

void load_sample_program(cpu_t *cpu) {
    mem_poke(cpu->memory, 0xC000, 0xA2);
    mem_poke(cpu->memory, 0xC001, 0x05);
    mem_poke(cpu->memory, 0xC002, 0xCA);
    mem_poke(cpu->memory, 0xC003, 0xD0);
    mem_poke(cpu->memory, 0xC004, 0xFD);
    mem_poke(cpu->memory, 0xC005, 0xFF);
    mem_poke(cpu->memory, 0xFFFC, 0x00);
    mem_poke(cpu->memory, 0xFFFD, 0xC0);
}
void cpu_starttrace(cpu_t *cpu) {
    cpu->trace = 0;
}
void cpu_stoptrace(cpu_t *cpu) {
    cpu->trace = 1;
}
void cpu_dump1(cpu_t *cpu) {
    dbg_printf("PC=%hX IR=%hhX",
                cpu->pc, cpu->ir);
}

void cpu_dump2(cpu_t *cpu) {
    dbg_printf(" A=%hhX X=%hhX Y=%hhX S=%hhX C=%d Z=%d I=%d D=%d B=%d V=%d N=%d\n",
                cpu->a, cpu->x, cpu->y, cpu->s, !!(cpu->p & C), !!(cpu->p & Z), !!(cpu->p & I), !!(cpu->p & D), !!(cpu->p & B), !!(cpu->p & V), !!(cpu->p & N));
}

void dump_cpu(cpu_t *cpu) {
    cpu_dump1(cpu);
    cpu_dump2(cpu);
}

void cpu_start(cpu_t *cpu) {
    uint16_t peek_res = mem_peek2(cpu->memory, 0xFFFC);
    cpu->pc = peek_res;
}

uint16_t cpu_imm(cpu_t *cpu) {
    uint16_t imm = cpu->pc;
    cpu->pc++;
    return imm;
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

uint8_t flagget(cpu_t *cpu, uint8_t flag) {
    return flag & cpu->p;
}

void flagset(cpu_t *cpu, uint8_t flag, uint8_t status) {
    if (status) {
        cpu->p |= flag;
    } else {
        cpu->p &= ~flag;
    }
}

void cpu_lda(cpu_t *cpu, uint16_t addr) {
    cpu->a = mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_ldx(cpu_t *cpu, uint16_t addr) {
    cpu->x = mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->x == 0);
    flagset(cpu, N, cpu->x >= 0x80);
}

void cpu_ldy(cpu_t *cpu, uint16_t addr) {
    cpu->y = mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->y == 0);
    flagset(cpu, N, cpu->y >= 0x80);
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
    uint8_t h = cpu->a + mem_peek(cpu->memory, addr);
    if (flagget(cpu, C)) {
        h++;
    }
    cpu->a = h;
    flagset(cpu, C, h > 0xFF);
    flagset(cpu, Z, h == 0);
    flagset(cpu, N, h >= 0x80);
    h += 0x80;
    flagset(cpu, V, (h > 0xFF) | (h < 0));
}

void cpu_push(cpu_t *cpu, uint8_t b) {
    mem_poke(cpu->memory, 0x100+cpu->s, b);
    cpu->s--;
}

uint8_t cpu_pull(cpu_t *cpu) {
    cpu->s++;
    return mem_peek(cpu->memory, 0x100+cpu->s);
}

void cpu_jmp(cpu_t *cpu, uint16_t addr) {
    cpu->pc = addr;
}

void cpu_bfs(cpu_t *cpu, uint8_t flag) {
    // this can be made branchfree by multiplying mem_peek by flagget()
    if (flagget(cpu, flag)) {
        cpu->pc = cpu->pc + ((int8_t)mem_peek(cpu->memory, cpu->pc)) + 1;
    } else {
        cpu->pc++;
    }
}

void cpu_bfc(cpu_t *cpu, uint8_t flag) {
    if (flagget(cpu, flag)) {
        cpu->pc++;
    } else {
        cpu->pc = cpu->pc + ((int8_t) mem_peek(cpu->memory, cpu->pc)) + 1;
    }
}

void cpu_jsr(cpu_t *cpu, uint16_t addr) {
    cpu->pc--;
    cpu_push(cpu, cpu->pc >> 8);
    cpu_push(cpu, cpu->pc & 0xFF);
    cpu->pc = addr;
}

void cpu_and_(cpu_t *cpu, uint16_t addr) {
    cpu->a &= mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_asl_A(cpu_t *cpu) {
    flagset(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a << 1;
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_asl(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    flagset(cpu, C, b & 0x80);
    b = b << 1;
    flagset(cpu, Z, b == 0);
    flagset(cpu, N, b >= 0x80);
}

void cpu_bit(cpu_t *cpu, uint16_t addr) {
    uint8_t h = mem_peek(cpu->memory, addr);
    flagset(cpu, N, h & 0x80);
    flagset(cpu, V, h & 0x40);
    flagset(cpu, Z, (h & cpu->a) == 0);
}

void cpu_brk(cpu_t *cpu) {
    cpu->pc++;
    cpu_push(cpu, (uint8_t) (cpu->pc >> 8));
    cpu_push(cpu, (uint8_t) (cpu->pc & 0xFF));
    cpu->pc--;
    cpu_push(cpu, cpu->p);
    flagset(cpu, I, 0x1);
}

void cpu_cmp(cpu_t *cpu, uint16_t addr) {
    uint8_t h = cpu->a - mem_peek(cpu->memory, addr);
    flagset(cpu, C, h <= 0xFF);
    flagset(cpu, Z, (h & 0xFF) == 0);
    flagset(cpu, N, (h & 0xFF) >= 0x80);
}

void cpu_cpx(cpu_t *cpu, uint16_t addr) {
    uint8_t h = cpu->x - mem_peek(cpu->memory, addr);
    flagset(cpu, C, h <= 0xFF);
    flagset(cpu, Z, (h & 0xFF) == 0);
    flagset(cpu, N, (h & 0xFF) >= 0x80);
}

void cpu_cpy(cpu_t *cpu, uint16_t addr) {
    uint8_t h = cpu->y - mem_peek(cpu->memory, addr);
    flagset(cpu, C, h <= 0xFF);
    flagset(cpu, Z, (h & 0xFF) == 0x0);
    flagset(cpu, N, (h & 0xFF) >= 0x80);
}

void cpu_dec_(cpu_t *cpu, uint16_t addr) {
    uint8_t h = mem_peek(cpu->memory, addr) - 1;
    cpu->x = h;
    flagset(cpu, Z, h == 0x0);
    flagset(cpu, N, h >= 0x80);
}

void cpu_dex(cpu_t *cpu) {
    uint8_t h = cpu->x - 1;
    cpu->x = h;
    flagset(cpu, Z, h == 0x00);
    flagset(cpu, N, h >= 0x80);
}

void cpu_dey(cpu_t *cpu) {
    uint8_t h = cpu->y - 1;
    cpu->y = h;
    flagset(cpu, Z, h == 0);
    flagset(cpu, N, h >= 0x80);
}

void cpu_inc_(cpu_t *cpu, uint16_t addr) {
    uint8_t h = mem_peek(cpu->memory, addr) + 1;
    cpu->x = h;
    flagset(cpu, Z, h == 0);
    flagset(cpu, N, h >= 0x80);
}

void cpu_inx(cpu_t *cpu) {
    uint8_t h = cpu->x + 1;
    cpu->x = h;
    flagset(cpu, Z, h == 0);
    flagset(cpu, N, h >= 0x80);
}

void cpu_iny(cpu_t *cpu) {
    uint8_t h = cpu->y + 1;
    cpu->y = h;
    flagset(cpu, Z, h == 0);
    flagset(cpu, N, h >= 0x80);
}

void cpu_eor(cpu_t *cpu, uint16_t addr) {
    cpu->a ^= mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_lsr_A(cpu_t *cpu) {
    flagset(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a >> 1;
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_lsr(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    flagset(cpu, C, b & 0x80);
    b = b >> 1;
    flagset(cpu, Z, b == 0);
    flagset(cpu, N, b >= 0x80);
}

void cpu_ora(cpu_t *cpu, uint16_t addr) {
    cpu->a |= mem_peek(cpu->memory, addr);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_pla(cpu_t *cpu) {
    cpu->a = cpu_pull(cpu);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_rol_A(cpu_t *cpu) {
    uint8_t bit = flagget(cpu, C);
    flagset(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a << 1;
    if (!bit) {
        cpu->a = 0x1;
    }
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_rol(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    uint8_t bit = flagget(cpu, C);
    flagset(cpu, C, b & 0x80);
    b = b << 1;
    if (bit) {
        b = b | 0x01;
    }
    mem_poke(cpu->memory, addr, b);
    flagset(cpu, Z, b == 0);
    flagset(cpu, N, b >= 0x80);
}

void cpu_ror_A(cpu_t *cpu) {
    uint8_t bit = flagget(cpu, C);
    flagset(cpu, C, cpu->a & 0x80);
    cpu->a = cpu->a >> 1;
    if (!bit) {
        cpu->a = 0x1;
    }
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_ror(cpu_t *cpu, uint16_t addr) {
    uint8_t b = mem_peek(cpu->memory, addr);
    uint8_t bit = flagget(cpu, C);
    flagset(cpu, C, b & 0x80);
    b = b >> 1;
    if (bit) {
        b = b | 0x01;
    }
    mem_poke(cpu->memory, addr, b);
    flagset(cpu, Z, b == 0);
    flagset(cpu, N, b >= 0x80);
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
    uint8_t h = cpu->a - mem_peek(cpu->memory, addr);
    if (!flagget(cpu, C)) {
        h--;
    }
    cpu->a = h;
    flagset(cpu, C, h <= 0xFF);
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
    h += 0x80;
    flagset(cpu, V, (h > 0xFF) | (h < 0));
}

void cpu_tax(cpu_t *cpu) {
    cpu->x = cpu->a;
    flagset(cpu, Z, cpu->x == 0);
    flagset(cpu, N, cpu->x >= 0x80);
}

void cpu_tay(cpu_t *cpu) {
    cpu->y = cpu->a;
    flagset(cpu, Z, cpu->y == 0);
    flagset(cpu, N, cpu->y >= 0x80);
}

void cpu_tsx(cpu_t *cpu) {
    cpu->x = cpu->s;
    flagset(cpu, Z, cpu->x == 0);
    flagset(cpu, N, cpu->x >= 0x80);
}

void cpu_txa(cpu_t *cpu) {
    cpu->a = cpu->x;
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_tya(cpu_t *cpu) {
    cpu->a = cpu->y;
    flagset(cpu, Z, cpu->a == 0);
    flagset(cpu, N, cpu->a >= 0x80);
}

void cpu_txs(cpu_t *cpu) {
    cpu->s = cpu->x;
}

uint8_t step_cpu(cpu_t *cpu) {
    cpu->ir = mem_peek(cpu->memory, cpu->pc);
    if (cpu->trace) {
        cpu_dump1(cpu);
    }
    cpu->pc++;
    if (cpu->pc == 0xFD9A) {
        cpu_starttrace(cpu);
    }
    if (cpu->ir == 0x00) {cpu_brk(cpu);}
    else if (cpu->ir == 0x01) {cpu_ora(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0x05) {cpu_ora(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x06) {cpu_asl(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x08) {cpu_push(cpu, cpu->p);}
    else if (cpu->ir == 0x09) {cpu_ora(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0x0A) {cpu_asl_A(cpu);}
    else if (cpu->ir == 0x0D) {cpu_ora(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x0E) {cpu_asl(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x10) {cpu_bfc(cpu, N);} //bpl
    else if (cpu->ir == 0x11) {cpu_ora(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0x15) {cpu_ora(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x16) {cpu_asl(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x18) {flagset(cpu, C, 0);} //clc
    else if (cpu->ir == 0x19) {cpu_ora(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0x1D) {cpu_ora(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x1E) {cpu_asl(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x20) {cpu_jsr(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x29) {cpu_and_(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0x21) {cpu_and_(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0x24) {cpu_bit(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x25) {cpu_and_(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x26) {cpu_rol(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x28) {cpu_pull(cpu);} //plp
    else if (cpu->ir == 0x2A) {cpu_rol_A(cpu);}
    else if (cpu->ir == 0x2C) {cpu_bit(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x2D) {cpu_and_(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x2E) {cpu_rol(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x30) {cpu_bfs(cpu, N);} //bmi
    else if (cpu->ir == 0x31) {cpu_and_(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0x35) {cpu_and_(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x36) {cpu_rol(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x38) {flagset(cpu, C, 1);}     //sec
    else if (cpu->ir == 0x39) {cpu_and_(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0x3D) {cpu_and_(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x3E) {cpu_rol(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x40) {cpu_rti(cpu);}
    else if (cpu->ir == 0x41) {cpu_eor(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0x45) {cpu_eor(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x46) {cpu_lsr(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x48) {cpu_push(cpu, cpu->a);} //pha
    else if (cpu->ir == 0x49) {cpu_eor(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0x4A) {cpu_lsr_A(cpu);}
    else if (cpu->ir == 0x4C) {cpu_jmp(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x4D) {cpu_eor(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x4E) {cpu_lsr(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x50) {cpu_bfc(cpu, V);} //bvc
    else if (cpu->ir == 0x51) {cpu_eor(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0x55) {cpu_eor(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x56) {cpu_lsr(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x58) {flagset(cpu, I, 0);} //cli
    else if (cpu->ir == 0x59) {cpu_eor(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0x5D) {cpu_eor(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x5E) {cpu_lsr(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x60) {cpu_rts(cpu);}
    else if (cpu->ir == 0x61) {cpu_adc(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0x65) {cpu_adc(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x66) {cpu_ror(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x68) {cpu_pla(cpu);}
    else if (cpu->ir == 0x69) {cpu_adc(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0x6A) {cpu_ror_A(cpu);}
    else if (cpu->ir == 0x6C) {cpu_jmp(cpu, cpu_ind(cpu));}
    else if (cpu->ir == 0x6D) {cpu_adc(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x6E) {cpu_ror(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x70) {cpu_bfs(cpu, V);} //bvs
    else if (cpu->ir == 0x71) {cpu_adc(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0x75) {cpu_adc(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x76) {cpu_ror(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x78) {flagset(cpu, I, 1);}     //sei
    else if (cpu->ir == 0x79) {cpu_adc(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0x7D) {cpu_adc(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x7E) {cpu_ror(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0x81) {cpu_sta(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0x84) {cpu_sty(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x85) {cpu_sta(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x86) {cpu_stx(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0x88) {cpu_dey(cpu);}
    else if (cpu->ir == 0x8A) {cpu_txa(cpu);}
    else if (cpu->ir == 0x8C) {cpu_sty(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x8D) {cpu_sta(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x8E) {cpu_stx(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0x90) {cpu_bfc(cpu, C);} //bcc
    else if (cpu->ir == 0x91) {cpu_sta(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0x94) {cpu_sty(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x95) {cpu_sta(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0x96) {cpu_stx(cpu, cpu_zpy(cpu));}
    else if (cpu->ir == 0x98) {cpu_tya(cpu);}
    else if (cpu->ir == 0x99) {cpu_sta(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0x9A) {cpu_txs(cpu);}
    else if (cpu->ir == 0x9D) {cpu_sta(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xA0) {cpu_ldy(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xA1) {cpu_lda(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0xA2) {cpu_ldx(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xA4) {cpu_ldy(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xA5) {cpu_lda(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xA6) {cpu_ldx(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xA8) {cpu_tay(cpu);}
    else if (cpu->ir == 0xA9) {cpu_lda(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xAA) {cpu_tax(cpu);}
    else if (cpu->ir == 0xAC) {cpu_ldy(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xAD) {cpu_lda(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xAE) {cpu_ldx(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xB0) {cpu_bfs(cpu, C);} //bcs
    else if (cpu->ir == 0xB1) {cpu_lda(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0xB4) {cpu_ldy(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xB5) {cpu_lda(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xB6) {cpu_ldx(cpu, cpu_zpy(cpu));}
    else if (cpu->ir == 0xB8) {flagset(cpu, V, 0);} //clv
    else if (cpu->ir == 0xB9) {cpu_lda(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0xBA) {cpu_tsx(cpu);}
    else if (cpu->ir == 0xBC) {cpu_ldy(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xBD) {cpu_lda(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xBE) {cpu_ldx(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0xC0) {cpu_cpy(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xC1) {cpu_cmp(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0xC4) {cpu_cpy(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xC5) {cpu_cmp(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xC6) {cpu_dec_(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xC8) {cpu_iny(cpu);}
    else if (cpu->ir == 0xC9) {cpu_cmp(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xCA) {cpu_dex(cpu);}
    else if (cpu->ir == 0xCC) {cpu_cpy(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xCD) {cpu_cmp(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xCE) {cpu_dec_(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xD0) {cpu_bfc(cpu, Z);} //bne
    else if (cpu->ir == 0xD1) {cpu_cmp(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0xD5) {cpu_cmp(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xD6) {cpu_dec_(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xD8) {flagset(cpu, D, 0);} //cld
    else if (cpu->ir == 0xD9) {cpu_cmp(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0xDD) {cpu_cmp(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xDE) {cpu_dec_(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xE0) {cpu_cpx(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xE1) {cpu_sbc(cpu, cpu_indx(cpu));}
    else if (cpu->ir == 0xE4) {cpu_cpx(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xE5) {cpu_sbc(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xE6) {cpu_inc_(cpu, cpu_zp(cpu));}
    else if (cpu->ir == 0xE8) {cpu_inx(cpu);}
    else if (cpu->ir == 0xE9) {cpu_sbc(cpu, cpu_imm(cpu));}
    else if (cpu->ir == 0xEA) {return 1;}  // nop
    else if (cpu->ir == 0xEC) {cpu_cpx(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xED) {cpu_sbc(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xEE) {cpu_inc_(cpu, cpu_abs(cpu));}
    else if (cpu->ir == 0xF0) {cpu_bfs(cpu, Z);} //beq
    else if (cpu->ir == 0xF1) {cpu_sbc(cpu, cpu_indy(cpu));}
    else if (cpu->ir == 0xF5) {cpu_sbc(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xF6) {cpu_inc_(cpu, cpu_zpx(cpu));}
    else if (cpu->ir == 0xF8) {flagset(cpu, D, 1);}     //sed
    else if (cpu->ir == 0xF9) {cpu_sbc(cpu, cpu_absy(cpu));}
    else if (cpu->ir == 0xFD) {cpu_sbc(cpu, cpu_absx(cpu));}
    else if (cpu->ir == 0xFE) {cpu_inc_(cpu, cpu_absx(cpu));}
    else {return 1;}
    if (cpu->trace) {
        cpu_dump2(cpu);
    }
    return 0;
}