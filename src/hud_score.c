// src/hud_score.c
#include "hud_score.h"
#include "tetris_constants.h"
#include "tetris_tables.h"
#include <stddef.h>
#include <stdbool.h>

static const uint16_t BASE_POINTS[5] = { 0, 40, 100, 300, 1200 };

// Adds a BCD byte with carry (matches 6502 SED + ADC)
static uint8_t bcd_add_byte(uint8_t a, uint8_t b, uint8_t* carry) {
    uint16_t lo = (a & 0x0F) + (b & 0x0F) + (*carry);
    if (lo > 9) lo += 6;

    uint16_t hi = (a & 0xF0) + (b & 0xF0) + (lo & 0xF0);
    if (hi > 0x90) hi += 0x60;

    *carry = (hi > 0x90) ? 1 : 0;
    return (uint8_t)((hi & 0xF0) | (lo & 0x0F));
}

// Converts a standard integer (0-999999) to 3 BCD bytes and accumulates it
void hud_add_score(uint32_t points) {
    uint8_t pts[3];
    pts[0] = ((points % 100) / 10) << 4 | (points % 10);
    pts[1] = (((points / 100) % 100) / 10) << 4 | ((points / 100) % 10);
    pts[2] = (((points / 10000) % 100) / 10) << 4 | ((points / 10000) % 10);

    uint8_t carry = 0;
    score0 = bcd_add_byte(score0, pts[0], &carry);
    score1 = bcd_add_byte(score1, pts[1], &carry);
    score2 = bcd_add_byte(score2, pts[2], &carry);

    // Score cap at 999999 (classic max-out)
    if (carry || score2 > 0x99) {
        score0 = 0x99;
        score1 = 0x99;
        score2 = 0x99;
    }
}

void hud_add_lines(uint8_t count) {
    if (count == 0 || count > 4) return;

    // Calculate points: Base * (Level + 1)
    uint32_t pts = (uint32_t)BASE_POINTS[count] * (levelNumber + 1);
    hud_add_score(pts);

    // Line counter BCD increment
    uint8_t carry = 0;
    uint8_t bcd_count = (count < 10) ? count : 0;
    linesClearedLo = bcd_add_byte(linesClearedLo, bcd_count, &carry);

    // Level progression: level up every 10 lines
    int lines_int = ((linesClearedLo >> 4) * 10) + (linesClearedLo & 0x0F);
    if ((lines_int / 10) > levelNumber) {
        levelNumber++;
    }
}

void hud_record_piece_spawn(uint8_t piece_id) {
    uint8_t* target = NULL;
    switch (piece_id) {
        case PIECE_T: target = &statsCount_T; break;
        case PIECE_J: target = &statsCount_J; break;
        case PIECE_Z: target = &statsCount_Z; break;
        case PIECE_O: target = &statsCount_O; break;
        case PIECE_S: target = &statsCount_S; break;
        case PIECE_L: target = &statsCount_L; break;
        case PIECE_I: target = &statsCount_I; break;
        default: return;
    }

    uint8_t carry = 0;
    *target = bcd_add_byte(*target, 1, &carry);
}

uint32_t hud_get_score_integer(void) {
    uint32_t s = 0;
    s += (score0 & 0x0F) + ((score0 >> 4) * 10);
    s += ((score1 & 0x0F) * 100) + ((score1 >> 4) * 1000);
    s += ((score2 & 0x0F) * 10000) + ((score2 >> 4) * 100000);
    return s;
}