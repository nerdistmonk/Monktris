// src/ppu.h
#ifndef PPU_H
#define PPU_H

#include <stdint.h>

void ppu_init(void);
void ppu_render_screen(uint32_t* framebuffer);

#endif // PPU_H