// src/apu.h
#ifndef APU_H
#define APU_H

#include <stdint.h>

#define SFX_MOVE       1
#define SFX_ROTATE     2
#define SFX_LOCK       3
#define SFX_LINE_CLEAR 4

void apu_init(void);
void apu_shutdown(void);
void apu_set_music(uint8_t song_id);
void apu_play_sfx(uint8_t sfx_id);
void apu_tick_frame(void);

#endif // APU_H