// src/piece_control.c
#include <stdbool.h>
#include <stdint.h>
#include "tetris_game.h"
#include "tetris_tables.h"
#include "tetris_constants.h"
#include "apu.h"
#include "hud_score.h"

// Forward declarations from playfield.c
bool tetris_check_collision(int piece_orient, int test_x, int test_y);
void tetris_lock_piece(int piece_orient, int px, int py, uint8_t tile_id);

static void handle_horizontal_input(void) {
    int dir = 0;
    if (buttonsDown & BTN_LEFT)  dir = -1;
    if (buttonsDown & BTN_RIGHT) dir = 1;

    // Both held or neither held -> reset DAS
    if ((buttonsDown & (BTN_LEFT | BTN_RIGHT)) == (BTN_LEFT | BTN_RIGHT) || dir == 0) {
        dasCounter = 0;
        return;
    }

    bool do_move = false;

    // Fresh button press
    if (buttonsPressed & (BTN_LEFT | BTN_RIGHT)) {
        do_move = true;
        dasCounter = 0;
    } else {
        // Button held: advance DAS counter
        dasCounter++;
        if (dasCounter >= DAS_INITIAL_DELAY) {
            do_move = true;
            dasCounter = DAS_INITIAL_DELAY - DAS_REPEAT_RATE; // reset to repeat interval
        }
    }

    if (do_move) {
        int test_x = pieceX + dir;
        if (tetris_check_collision(pieceRotation, test_x, pieceY)) {
            pieceX = test_x;
            apu_play_sfx(SFX_MOVE);
        }
    }
}

static void handle_rotation_input(void) {
    int rot_dir = 0;
    // B button = CCW (-1), A button = CW (+1)
    if (buttonsPressed & BTN_B) rot_dir = -1;
    if (buttonsPressed & BTN_A) rot_dir = 1;

    if (rot_dir == 0) return;

    uint8_t count = PIECE_ORIENTATION_COUNT[currentPiece];
    uint8_t base  = PIECE_BASE_ORIENTATION[currentPiece];

    // Current relative orientation (0 to count-1)
    int current_rel = pieceRotation - base;
    int next_rel = (current_rel + rot_dir) % (int)count;
    if (next_rel < 0) next_rel += count;

    uint8_t test_orient = base + next_rel;

    // Strictly test candidate position without shifting (authentic lack of wall kicks)
    if (tetris_check_collision(test_orient, pieceX, pieceY)) {
        pieceRotation = test_orient;
        apu_play_sfx(SFX_ROTATE);
    }
}

bool tetris_update_active_piece(void) {
    handle_horizontal_input();
    handle_rotation_input();

    // Determine drop rate
    uint8_t current_level = (levelNumber < 29) ? levelNumber : 29;
    uint8_t target_delay = (buttonsDown & BTN_DOWN) ? 2 : GRAVITY_TABLE[current_level];

    fallTimer++;
    if (fallTimer < target_delay) {
        return false;
    }

    fallTimer = 0;

    // Attempt downward step
    if (tetris_check_collision(pieceRotation, pieceX, pieceY + 1)) {
        pieceY++;
        // NES awards 1 point per row during soft drop
        if (buttonsDown & BTN_DOWN) {
            hud_add_score(1);
        }
        return false;
    }

    // Blocked below: Lock piece into the playfield buffer ($0400) using piece index
    uint8_t tile_id = currentPiece;
    tetris_lock_piece(pieceRotation, pieceX, pieceY, tile_id);
    apu_play_sfx(SFX_LOCK);

    return true; // Signals transition to row clear scan
}