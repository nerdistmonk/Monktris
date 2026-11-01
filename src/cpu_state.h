#ifndef CPU_STATE_H
#define CPU_STATE_H

#include <stdint.h>
#include <stdbool.h>

// 2KB internal RAM + zero-page mirror
extern uint8_t ram[0x800];

// CPU Registers for accurate 6502 arithmetic translation
typedef struct {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t s;       // Stack pointer
    uint8_t p;       // Status flags: N V - B D I Z C
} CPU6502;

extern CPU6502 cpu;

// 6502 Status Register Flags
#define FLAG_C (1 << 0) // Carry
#define FLAG_Z (1 << 1) // Zero
#define FLAG_I (1 << 2) // Interrupt Disable
#define FLAG_D (1 << 3) // Decimal (NES ignores this, but flag exists)
#define FLAG_B (1 << 4) // Break
#define FLAG_V (1 << 6) // Overflow
#define FLAG_N (1 << 7) // Negative

// Precise 6502 flag helpers
static inline void set_nz(uint8_t val) {
    if (val == 0) cpu.p |= FLAG_Z; else cpu.p &= ~FLAG_Z;
    if (val & 0x80) cpu.p |= FLAG_N; else cpu.p &= ~FLAG_N;
}

static inline void adc(uint8_t val) {
    uint16_t carry = (cpu.p & FLAG_C) ? 1 : 0;
    uint16_t sum = cpu.a + val + carry;
    if (~(cpu.a ^ val) & (cpu.a ^ sum) & 0x80) cpu.p |= FLAG_V; else cpu.p &= ~FLAG_V;
    if (sum > 0xFF) cpu.p |= FLAG_C; else cpu.p &= ~FLAG_C;
    cpu.a = (uint8_t)sum;
    set_nz(cpu.a);
}

static inline void sbc(uint8_t val) {
    uint16_t carry = (cpu.p & FLAG_C) ? 0 : 1;
    uint16_t diff = cpu.a - val - carry;
    if ((cpu.a ^ val) & (cpu.a ^ diff) & 0x80) cpu.p |= FLAG_V; else cpu.p &= ~FLAG_V;
    if (diff <= 0xFF) cpu.p |= FLAG_C; else cpu.p &= ~FLAG_C;
    cpu.a = (uint8_t)diff;
    set_nz(cpu.a);
}

#endif // CPU_STATE_H