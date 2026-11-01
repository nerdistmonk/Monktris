#include "tetris_game.h"
#include "tetris_tables.h"
#include <string.h>

// Direct reference to the $0400 playfield buffer in RAM
#define PLAYFIELD_OFFSET 0x0400
#define PLAYFIELD_WIDTH  10
#define PLAYFIELD_HEIGHT 22 // 2 hidden top rows + 20 visible rows

void playfield_clear(void) {
    memset(&ram[PLAYFIELD_OFFSET], TILE_EMPTY, PLAYFIELD_WIDTH * PLAYFIELD_HEIGHT);
}

uint8_t playfield_get_tile(int x, int y) {
    if (x < 0 || x >= PLAYFIELD_WIDTH || y < 0 || y >= PLAYFIELD_HEIGHT) {
        return 0xFF; // Wall/Floor boundary sentinel
    }
    return ram[PLAYFIELD_OFFSET + (y * PLAYFIELD_WIDTH) + x];
}

void playfield_set_tile(int x, int y, uint8_t tile) {
    if (x >= 0 && x < PLAYFIELD_WIDTH && y >= 0 && y < PLAYFIELD_HEIGHT) {
        ram[PLAYFIELD_OFFSET + (y * PLAYFIELD_WIDTH) + x] = tile;
    }
}

// Replicates the collision routine called before shifts and rotations.
// Returns true if position is valid, false if blocked by wall/floor/mino.
bool tetris_check_collision(int piece_orient, int test_x, int test_y) {
    for (int i = 0; i < 4; i++) {
        int x = test_x + TETROMINO_OFFSETS[piece_orient][i].dx;
        int y = test_y + TETROMINO_OFFSETS[piece_orient][i].dy;

        // Playfield horizontal boundaries
        if (x < 0 || x >= PLAYFIELD_WIDTH) {
            return false;
        }

        // Floor boundary
        if (y >= PLAYFIELD_HEIGHT) {
            return false;
        }

        // Allow spawning above visible playfield (y < 0 during top spawn)
        if (y < 0) {
            continue;
        }

        // Check if destination tile is occupied
        if (playfield_get_tile(x, y) != TILE_EMPTY) {
            return false;
        }
    }
    return true;
}

// Bakes the active piece into the playfield once locking occurs
void tetris_lock_piece(int piece_orient, int px, int py, uint8_t tile_id) {
    for (int i = 0; i < 4; i++) {
        int x = px + TETROMINO_OFFSETS[piece_orient][i].dx;
        int y = py + TETROMINO_OFFSETS[piece_orient][i].dy;
        if (y >= 0) {
            playfield_set_tile(x, y, tile_id);
        }
    }
}