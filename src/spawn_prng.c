// src/spawn_prng.c
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "tetris_game.h"
#include "tetris_tables.h"
#include "hud_score.h"

// Advances the 16-bit LFSR at $0017 / $0018
// Matches the original 6502 routine in tetris.asm
void update_rng(void) {
    uint8_t bit = ((rng_lo >> 1) ^ (rng_lo >> 2) ^ (rng_lo >> 4) ^ (rng_lo >> 5)) & 1;
    rng_lo = (rng_lo >> 1) | ((rng_hi & 1) << 7);
    rng_hi = (rng_hi >> 1) | (bit << 7);
}

// 1:1 NES 6502 piece selector logic
uint8_t roll_next_piece(void) {
    update_rng();
    uint8_t roll = (rng_hi >> 4) & 0x07;

    // Reroll triggered if dummy index 7 is hit or it matches current piece
    if (roll == 7 || roll == currentPiece) {
        update_rng();
        roll = (rng_hi >> 4) & 0x07;
        // NES reroll adds 1 to roll if it still matches or is dummy
        roll = (roll + 1) & 0x07;
        if (roll == 7) {
            roll = 0;
        }
    }

    return roll % 7;
}

// Spawns next piece at top-center of playfield
bool tetris_spawn_piece(void) {
    currentPiece = nextPiece;
    nextPiece = roll_next_piece();
    hud_record_piece_spawn(currentPiece);

    // Default spawn coordinates
    pieceX = 4;
    pieceY = 0;
    pieceRotation = PIECE_SPAWN_ORIENTATION[currentPiece];
    fallTimer = 0;

    extern bool tetris_check_collision(int piece_orient, int test_x, int test_y);
    if (!tetris_check_collision(pieceRotation, pieceX, pieceY)) {
        return false;
    }

    return true;
}