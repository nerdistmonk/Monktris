// src/hud_score.h
#ifndef HUD_SCORE_H
#define HUD_SCORE_H

#include <stdint.h>
#include "tetris_game.h"

// Piece statistics offsets in RAM ($03F0-$03F6)
#define statsCount_T ram[0x03F0]
#define statsCount_J ram[0x03F1]
#define statsCount_Z ram[0x03F2]
#define statsCount_O ram[0x03F3]
#define statsCount_S ram[0x03F4]
#define statsCount_L ram[0x03F5]
#define statsCount_I ram[0x03F6]

// 6-digit BCD Score: score0 (lowest 2 digits), score1 (middle), score2 (highest)
void hud_add_score(uint32_t points);
void hud_add_lines(uint8_t count);
void hud_record_piece_spawn(uint8_t piece_id);
uint32_t hud_get_score_integer(void);

#endif // HUD_SCORE_H