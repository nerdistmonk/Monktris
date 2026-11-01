// src/ppu.c
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ppu.h"
#include "tetris_game.h"
#include "tetris_tables.h"
#include "nes_palette.h"
#include "hud_score.h"
#include "scores.h"

#define SCREEN_W 256
#define SCREEN_H 240
#define PLAYFIELD_PX_X 96
#define PLAYFIELD_PX_Y 40
#define PLAYFIELD_RAM 0x0400

extern uint8_t tetris_strobe_timer;
extern char entered_initials[4];
extern int initial_cursor;
extern uint32_t ending_frame_counter;

// 8x8 font bitmap: 0-9, A-Z, space, hyphen, colon, angle brackets
static const uint8_t NES_FONT[41][8] = {
    { 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C, 0x00 },
    { 0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00 },
    { 0x3C, 0x66, 0x06, 0x0C, 0x18, 0x30, 0x7E, 0x00 },
    { 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00 },
    { 0x0C, 0x1C, 0x3C, 0x6C, 0xFE, 0x0C, 0x0C, 0x00 },
    { 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C, 0x00 },
    { 0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3C, 0x00 },
    { 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00 },
    { 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00 },
    { 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00 },
    { 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00 }, // A
    { 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00 },
    { 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00 },
    { 0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00 },
    { 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00 },
    { 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00 },
    { 0x3C, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3A, 0x00 },
    { 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00 },
    { 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00 },
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x6C, 0x6C, 0x38, 0x00 },
    { 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00 },
    { 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00 },
    { 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00 },
    { 0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00 },
    { 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 },
    { 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00 },
    { 0x3C, 0x66, 0x66, 0x66, 0x6A, 0x6C, 0x36, 0x00 },
    { 0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00 },
    { 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00 },
    { 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 },
    { 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00 },
    { 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00 },
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00 },
    { 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00 },
    { 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00 },
    { 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00 }, // Z
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00 },
    { 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00, 0x00 },
    { 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00 },
    { 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00 }
};

static const uint8_t MINO_TILE_7B[16] = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF,
    0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00
};

static const uint8_t MOSAIC_PALETTES[7][4] = {
    { 0x0F, 0x04, 0x0F, 0x14 },
    { 0x0F, 0x02, 0x0F, 0x12 },
    { 0x0F, 0x06, 0x0F, 0x16 },
    { 0x0F, 0x08, 0x0F, 0x18 },
    { 0x0F, 0x0A, 0x0F, 0x1A },
    { 0x0F, 0x07, 0x0F, 0x17 },
    { 0x0F, 0x0C, 0x0F, 0x1C }
};

static const uint8_t MOSAIC_TILEMAP[8][8] = {
    { 0, 0, 0, 3, 3, 6, 6, 6 },
    { 1, 0, 2, 3, 3, 6, 5, 5 },
    { 1, 2, 2, 4, 4, 1, 1, 5 },
    { 1, 1, 2, 4, 4, 5, 1, 5 },
    { 6, 6, 6, 6, 0, 0, 0, 2 },
    { 3, 3, 4, 4, 1, 0, 2, 2 },
    { 3, 3, 5, 4, 1, 1, 1, 2 },
    { 5, 5, 5, 4, 0, 6, 6, 6 }
};

static inline void set_pixel(uint32_t* fb, int x, int y, uint32_t argb) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        fb[y * SCREEN_W + x] = 0xFF000000 | argb;
    }
}

static inline uint32_t get_pixel(const uint32_t* fb, int x, int y) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        return fb[y * SCREEN_W + x];
    }
    return 0;
}

static void blit_tile_2bpp(uint32_t* fb, int dst_x, int dst_y, const uint8_t* tile_data, const uint8_t palette[4]) {
    for (int y = 0; y < 8; y++) {
        uint8_t p0 = tile_data[y];
        uint8_t p1 = tile_data[y + 8];
        for (int x = 0; x < 8; x++) {
            int bit = 7 - x;
            uint8_t color_idx = ((p0 >> bit) & 1) | (((p1 >> bit) & 1) << 1);
            set_pixel(fb, dst_x + x, dst_y + y, NES_MASTER_PALETTE[palette[color_idx]]);
        }
    }
}

static void draw_char(uint32_t* fb, int px, int py, char c, uint32_t color) {
    int idx = 36;
    if (c >= '0' && c <= '9') idx = c - '0';
    else if (c >= 'A' && c <= 'Z') idx = 10 + (c - 'A');
    else if (c >= 'a' && c <= 'z') idx = 10 + (c - 'a');
    else if (c == '-') idx = 37;
    else if (c == ':') idx = 38;
    else if (c == '<') idx = 39;
    else if (c == '>') idx = 40;

    for (int y = 0; y < 8; y++) {
        uint8_t row = NES_FONT[idx][y];
        for (int x = 0; x < 8; x++) {
            if ((row >> (7 - x)) & 1) {
                set_pixel(fb, px + x, py + y, color);
            }
        }
    }
}

static void draw_string(uint32_t* fb, int px, int py, const char* str, uint32_t color) {
    while (*str) {
        draw_char(fb, px, py, *str, color);
        px += 8;
        str++;
    }
}

static void draw_bcd_byte(uint32_t* fb, int px, int py, uint8_t bcd, uint32_t color) {
    draw_char(fb, px,     py, '0' + ((bcd >> 4) & 0x0F), color);
    draw_char(fb, px + 8, py, '0' + (bcd & 0x0F), color);
}

static void render_mosaic_background(uint32_t* fb, int color_shift) {
    for (int ty = 0; ty < 30; ty++) {
        for (int tx = 0; tx < 32; tx++) {
            uint8_t base_piece = MOSAIC_TILEMAP[ty % 8][tx % 8];
            uint8_t piece = (base_piece + (uint8_t)(color_shift % 7)) % 7;
            blit_tile_2bpp(fb, tx * 8, ty * 8, MINO_TILE_7B, MOSAIC_PALETTES[piece]);
        }
    }
}

static void cast_shadow(uint32_t* fb, int x, int y, int w, int h) {
    for (int sy = y; sy < y + h; sy++) {
        for (int sx = x; sx < x + w; sx++) {
            if (sx >= 0 && sx < SCREEN_W && sy >= 0 && sy < SCREEN_H) {
                uint32_t color = get_pixel(fb, sx, sy);
                uint32_t r = ((color >> 16) & 0xFF) * 4 / 10;
                uint32_t g = ((color >> 8) & 0xFF) * 4 / 10;
                uint32_t b = (color & 0xFF) * 4 / 10;
                set_pixel(fb, sx, sy, (r << 16) | (g << 8) | b);
            }
        }
    }
}

static void draw_window(uint32_t* fb, int x, int y, int w, int h, uint32_t border_color) {
    cast_shadow(fb, x + 5, y + 5, w, h);

    uint32_t bg_color = NES_MASTER_PALETTE[0x0F];
    for (int iy = y + 2; iy < y + h - 2; iy++) {
        for (int ix = x + 2; ix < x + w - 2; ix++) {
            set_pixel(fb, ix, iy, bg_color);
        }
    }

    uint32_t highlight = NES_MASTER_PALETTE[0x30];
    for (int i = 0; i < w; i++) {
        set_pixel(fb, x + i, y, highlight);
        set_pixel(fb, x + i, y + 1, border_color);
        set_pixel(fb, x + i, y + h - 2, border_color);
        set_pixel(fb, x + i, y + h - 1, border_color);
    }
    for (int j = 0; j < h; j++) {
        set_pixel(fb, x, y + j, highlight);
        set_pixel(fb, x + 1, y + j, border_color);
        set_pixel(fb, x + w - 2, y + j, border_color);
        set_pixel(fb, x + w - 1, y + j, border_color);
    }
}

void ppu_init(void) {}

static void render_menu_screen(uint32_t* fb) {
    render_mosaic_background(fb, 0);

    // Title Window
    draw_window(fb, 32, 12, 192, 54, NES_MASTER_PALETTE[0x22]);
    draw_string(fb, 68, 24, "M O N K T R I S", NES_MASTER_PALETTE[0x30]);
    draw_string(fb, 52, 42, "Design by RShaklfrd", NES_MASTER_PALETTE[0x27]);

    // High Scores Window
    draw_window(fb, 32, 72, 192, 54, NES_MASTER_PALETTE[0x28]);
    draw_string(fb, 84, 78, "TOP SCORES", NES_MASTER_PALETTE[0x28]);
    for (int i = 0; i < 3; i++) {
        const HighScoreEntry* e = scores_get(i);
        char entry_str[24];
        snprintf(entry_str, sizeof(entry_str), "%d. %s  %06u", i + 1, e->name, e->score);
        draw_string(fb, 56, 92 + (i * 10), entry_str, NES_MASTER_PALETTE[0x30]);
    }

    // Options Window
    draw_window(fb, 32, 132, 192, 94, NES_MASTER_PALETTE[0x12]);
    draw_string(fb, 56, 142, "SELECT START LEVEL", NES_MASTER_PALETTE[0x30]);

    char lvl_str[16];
    snprintf(lvl_str, sizeof(lvl_str), "<  %02d  >", levelNumber);
    draw_string(fb, 96, 156, lvl_str, NES_MASTER_PALETTE[0x28]);

    draw_string(fb, 84, 172, "MUSIC TRACK", NES_MASTER_PALETTE[0x30]);
    extern uint8_t currentMusicTrack;
    if (currentMusicTrack == 0) {
        draw_string(fb, 100, 186, "< OFF >", NES_MASTER_PALETTE[0x16]);
    } else {
        char mus_str[16];
        snprintf(mus_str, sizeof(mus_str), "< THEME %d >", currentMusicTrack);
        draw_string(fb, 84, 186, mus_str, NES_MASTER_PALETTE[0x2A]);
    }

    draw_string(fb, 52, 208, "PRESS START TO PLAY", NES_MASTER_PALETTE[0x20]);
}

static void render_initials_screen(uint32_t* fb) {
    render_mosaic_background(fb, 0);

    draw_window(fb, 48, 60, 160, 120, NES_MASTER_PALETTE[0x28]);
    draw_string(fb, 76, 76, "NEW HIGH SCORE!", NES_MASTER_PALETTE[0x28]);

    char sc_str[16];
    snprintf(sc_str, sizeof(sc_str), "%06u", hud_get_score_integer());
    draw_string(fb, 104, 96, sc_str, NES_MASTER_PALETTE[0x30]);

    draw_string(fb, 80, 116, "ENTER INITIALS", NES_MASTER_PALETTE[0x20]);

    for (int i = 0; i < 3; i++) {
        uint32_t col = (i == initial_cursor) ? NES_MASTER_PALETTE[0x28] : NES_MASTER_PALETTE[0x30];
        draw_char(fb, 112 + (i * 12), 136, entered_initials[i], col);
        if (i == initial_cursor) {
            draw_char(fb, 112 + (i * 12), 146, '-', NES_MASTER_PALETTE[0x28]);
        }
    }

    draw_string(fb, 64, 162, "USE PAD - A TO SET", NES_MASTER_PALETTE[0x30]);
}

static void render_ending_screen(uint32_t* fb) {
    int shift = (int)((ending_frame_counter / 15) % 7);
    render_mosaic_background(fb, shift);

    uint32_t score = hud_get_score_integer();
    const char* tier_title = "BRONZE MASTER";
    uint32_t tier_col = NES_MASTER_PALETTE[0x27];

    if (score >= 100000) {
        tier_title = "GRAND MASTER";
        tier_col = NES_MASTER_PALETTE[0x28];
    } else if (score >= 50000) {
        tier_title = "SILVER MASTER";
        tier_col = NES_MASTER_PALETTE[0x30];
    }

    draw_window(fb, 36, 40, 184, 150, tier_col);
    draw_string(fb, 68, 56, "CONGRATULATIONS", NES_MASTER_PALETTE[0x30]);
    draw_string(fb, 76, 76, tier_title, tier_col);

    uint32_t total_lines = (uint32_t)(((linesClearedLo >> 4) & 0x0F) * 10) + (linesClearedLo & 0x0F);
    char lines_str[24];
    snprintf(lines_str, sizeof(lines_str), "LINES : %03u", total_lines);
    draw_string(fb, 84, 104, lines_str, NES_MASTER_PALETTE[0x30]);

    char score_str[24];
    snprintf(score_str, sizeof(score_str), "SCORE : %06u", score);
    draw_string(fb, 76, 120, score_str, NES_MASTER_PALETTE[0x30]);

    draw_string(fb, 52, 154, "PRESS START FOR MENU", NES_MASTER_PALETTE[0x20]);
}

static void render_ingame_screen(uint32_t* fb) {
    render_mosaic_background(fb, 0);

    uint8_t pal_idx = levelNumber % 10;
    uint32_t border_color = NES_MASTER_PALETTE[LEVEL_PALETTES[pal_idx][1]];
    if (tetris_strobe_timer > 0 && (tetris_strobe_timer & 2)) {
        border_color = NES_MASTER_PALETTE[0x30];
    }
    uint32_t text_color = NES_MASTER_PALETTE[0x30];

    // Playfield Window
    draw_window(fb, 94, 38, 84, 164, border_color);

    // Playfield Cells
    for (int y = 2; y < 22; y++) {
        for (int x = 0; x < 10; x++) {
            uint8_t tile = ram[PLAYFIELD_RAM + (y * 10) + x];
            if (tile != TILE_EMPTY) {
                uint8_t piece_id = (tile <= PIECE_I) ? tile : PIECE_T;
                int px = PLAYFIELD_PX_X + (x * 8);
                int py = PLAYFIELD_PX_Y + ((y - 2) * 8);
                blit_tile_2bpp(fb, px, py, MINO_TILE_7B, PIECE_PALETTES[piece_id]);
            }
        }
    }

    // Active Piece
    for (int i = 0; i < 4; i++) {
        int gx = pieceX + TETROMINO_OFFSETS[pieceRotation][i].dx;
        int gy = pieceY + TETROMINO_OFFSETS[pieceRotation][i].dy;
        if (gy >= 2 && gy < 22 && gx >= 0 && gx < 10) {
            int px = PLAYFIELD_PX_X + (gx * 8);
            int py = PLAYFIELD_PX_Y + ((gy - 2) * 8);
            blit_tile_2bpp(fb, px, py, MINO_TILE_7B, PIECE_PALETTES[currentPiece % 7]);
        }
    }

    // Lines Window
    draw_window(fb, 94, 12, 84, 20, border_color);
    draw_string(fb, 100, 18, "LINES", NES_MASTER_PALETTE[0x28]);
    draw_bcd_byte(fb, 152, 18, linesClearedLo, text_color);

    // Score & Level Window
    draw_window(fb, 184, 38, 56, 68, border_color);
    draw_string(fb, 192, 44, "SCORE", NES_MASTER_PALETTE[0x28]);
    draw_bcd_byte(fb, 188, 56, score2, text_color);
    draw_bcd_byte(fb, 204, 56, score1, text_color);
    draw_bcd_byte(fb, 220, 56, score0, text_color);

    draw_string(fb, 192, 74, "LEVEL", NES_MASTER_PALETTE[0x28]);
    draw_char(fb, 204, 86, '0' + (levelNumber / 10), text_color);
    draw_char(fb, 212, 86, '0' + (levelNumber % 10), text_color);

    // Next Preview Window
    draw_window(fb, 184, 116, 56, 56, border_color);
    draw_string(fb, 196, 122, "NEXT", NES_MASTER_PALETTE[0x28]);
    uint8_t next_orient = PIECE_SPAWN_ORIENTATION[nextPiece];
    for (int i = 0; i < 4; i++) {
        int nx = 212 + (TETROMINO_OFFSETS[next_orient][i].dx * 8);
        int ny = 148 + (TETROMINO_OFFSETS[next_orient][i].dy * 8);
        blit_tile_2bpp(fb, nx, ny, MINO_TILE_7B, PIECE_PALETTES[nextPiece % 7]);
    }

    // Piece Statistics Window
    draw_window(fb, 16, 38, 70, 164, border_color);
    draw_string(fb, 22, 44, "STATS", NES_MASTER_PALETTE[0x28]);

    const char* piece_labels[7] = { "T", "J", "Z", "O", "S", "L", "I" };
    uint8_t stats[7] = { statsCount_T, statsCount_J, statsCount_Z, statsCount_O, statsCount_S, statsCount_L, statsCount_I };

    for (int i = 0; i < 7; i++) {
        int sy = 62 + (i * 19);
        draw_string(fb, 24, sy, piece_labels[i], NES_MASTER_PALETTE[PIECE_PALETTES[i][1]]);
        draw_bcd_byte(fb, 52, sy, stats[i], text_color);
    }
}

void ppu_render_screen(uint32_t* framebuffer) {
    if (gameMode == 0) {
        render_menu_screen(framebuffer);
    } else if (gameMode == 2) {
        render_initials_screen(framebuffer);
    } else if (gameMode == 3) {
        render_ending_screen(framebuffer);
    } else {
        render_ingame_screen(framebuffer);

        extern bool isGamePaused;
        if (isGamePaused) {
            draw_window(framebuffer, 72, 98, 112, 38, NES_MASTER_PALETTE[0x30]);
            draw_string(framebuffer, 104, 113, "PAUSED", NES_MASTER_PALETTE[0x30]);
        }

        extern bool isGameOver;
        if (isGameOver) {
            draw_window(framebuffer, 64, 90, 128, 52, NES_MASTER_PALETTE[0x16]);
            draw_string(framebuffer, 92, 102, "GAME OVER", NES_MASTER_PALETTE[0x16]);
            draw_string(framebuffer, 84, 118, "PRESS START", NES_MASTER_PALETTE[0x30]);
        }
    }
}