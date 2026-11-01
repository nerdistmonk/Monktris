// src/line_clear.c
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tetris_game.h"
#include "tetris_tables.h"
#include "hud_score.h"
#include "apu.h"

#define PLAYFIELD_RAM 0x0400

static uint8_t cleared_rows[4];
static uint8_t completed_count = 0;
static uint8_t anim_frame = 0;
uint8_t tetris_strobe_timer = 0;

uint8_t tetris_detect_full_lines(void) {
    completed_count = 0;
    anim_frame = 0;

    for (int y = 2; y < 22; y++) {
        bool full = true;
        for (int x = 0; x < 10; x++) {
            if (ram[PLAYFIELD_RAM + (y * 10) + x] == TILE_EMPTY) {
                full = false;
                break;
            }
        }
        if (full) {
            cleared_rows[completed_count++] = y;
        }
    }

    if (completed_count > 0) {
        apu_play_sfx(SFX_LINE_CLEAR);
        if (completed_count == 4) {
            tetris_strobe_timer = 20; // 20 frames of palette flashing
        }
    }

    return completed_count;
}

// Center-out curtain animation:
// Frames 0-3: cols 4 & 5 vanish
// Frames 4-7: cols 3 & 6 vanish
// Frames 8-11: cols 2 & 7 vanish
// Frames 12-15: cols 1 & 8 vanish
// Frames 16-19: cols 0 & 9 vanish
// Frame 20: Collapse stack downward
bool tetris_animate_line_clear(void) {
    if (completed_count == 0) return true;

    if (tetris_strobe_timer > 0) {
        tetris_strobe_timer--;
    }

    int step = anim_frame / 4;
    if (step <= 4) {
        int left_col = 4 - step;
        int right_col = 5 + step;

        for (int i = 0; i < completed_count; i++) {
            int y = cleared_rows[i];
            ram[PLAYFIELD_RAM + (y * 10) + left_col] = TILE_EMPTY;
            ram[PLAYFIELD_RAM + (y * 10) + right_col] = TILE_EMPTY;
        }
    }

    anim_frame++;
    if (anim_frame < 20) {
        return false;
    }

    // Collapse playfield rows downward
    for (int i = 0; i < completed_count; i++) {
        int target_y = cleared_rows[i];
        for (int y = target_y; y > 0; y--) {
            for (int x = 0; x < 10; x++) {
                ram[PLAYFIELD_RAM + (y * 10) + x] = ram[PLAYFIELD_RAM + ((y - 1) * 10) + x];
            }
        }
        for (int x = 0; x < 10; x++) {
            ram[PLAYFIELD_RAM + x] = TILE_EMPTY;
        }
    }

    hud_add_lines(completed_count);
    completed_count = 0;
    anim_frame = 0;
    return true;
}