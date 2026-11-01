// include/tetris_tables.h
#ifndef TETRIS_TABLES_H
#define TETRIS_TABLES_H

#include <stdint.h>

#define TILE_EMPTY 0xEF

// Piece IDs
enum {
    PIECE_T = 0,
    PIECE_J = 1,
    PIECE_Z = 2,
    PIECE_O = 3,
    PIECE_S = 4,
    PIECE_L = 5,
    PIECE_I = 6
};

// 4 minos per piece orientation (relative X, relative Y)
typedef struct {
    int8_t dx;
    int8_t dy;
} MinoOffset;

// 19 orientations matching the original disassembly tables:
// T: 0-3, J: 4-7, Z: 8-9, O: 10, S: 11-12, L: 13-16, I: 17-18
static const MinoOffset TETROMINO_OFFSETS[19][4] = {
    // T-piece (4 orientations)
    { { 0, 0}, {-1, 0}, { 1, 0}, { 0,-1} }, // T-up
    { { 0, 0}, { 0,-1}, { 0, 1}, { 1, 0} }, // T-right
    { { 0, 0}, {-1, 0}, { 1, 0}, { 0, 1} }, // T-down
    { { 0, 0}, { 0,-1}, { 0, 1}, {-1, 0} }, // T-left

    // J-piece (4 orientations)
    { { 0, 0}, {-1, 0}, { 1, 0}, {-1,-1} }, // J-left-hook
    { { 0, 0}, { 0,-1}, { 0, 1}, { 1,-1} }, 
    { { 0, 0}, {-1, 0}, { 1, 0}, { 1, 1} }, 
    { { 0, 0}, { 0,-1}, { 0, 1}, {-1, 1} }, 

    // Z-piece (2 orientations)
    { { 0, 0}, {-1, 0}, { 0, 1}, { 1, 1} }, // Z-horizontal
    { { 0, 0}, { 0,-1}, { 1, 0}, { 1, 1} }, // Z-vertical

    // O-piece (1 orientation)
    { { 0, 0}, { 1, 0}, { 0, 1}, { 1, 1} }, // O-square

    // S-piece (2 orientations)
    { { 0, 0}, { 1, 0}, { 0, 1}, {-1, 1} }, // S-horizontal
    { { 0, 0}, { 0,-1}, {-1, 0}, {-1, 1} }, // S-vertical

    // L-piece (4 orientations)
    { { 0, 0}, {-1, 0}, { 1, 0}, { 1,-1} }, // L-right-hook
    { { 0, 0}, { 0,-1}, { 0, 1}, { 1, 1} },
    { { 0, 0}, {-1, 0}, { 1, 0}, {-1, 1} },
    { { 0, 0}, { 0,-1}, { 0, 1}, {-1,-1} },

    // I-piece (2 orientations)
    { { 0, 0}, {-2, 0}, {-1, 0}, { 1, 0} }, // I-horizontal
    { { 0, 0}, { 0,-1}, { 0, 1}, { 0, 2} }  // I-vertical
};

// Spawn orientations and piece offsets mapping
static const uint8_t PIECE_SPAWN_ORIENTATION[7] = { 0, 4, 8, 10, 11, 13, 17 };

#endif // TETRIS_TABLES_H