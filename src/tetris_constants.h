// include/tetris_constants.h
#ifndef TETRIS_CONSTANTS_H
#define TETRIS_CONSTANTS_H

#include <stdint.h>

// Drop delay in frames per level (Levels 0 through 29+)
static const uint8_t GRAVITY_TABLE[30] = {
    48, 43, 38, 33, 28, 23, 18, 13, 8, 6, // 0-9
    5,  5,  5,  4,  4,  4,  3,  3,  3, 2, // 10-19
    2,  2,  2,  2,  2,  2,  2,  2,  2, 1  // 20-29+
};

// Orientation counts and base index for each piece type
// T: 4, J: 4, Z: 2, O: 1, S: 2, L: 4, I: 2
static const uint8_t PIECE_ORIENTATION_COUNT[7] = { 4, 4, 2, 1, 2, 4, 2 };
static const uint8_t PIECE_BASE_ORIENTATION[7]  = { 0, 4, 8, 10, 11, 13, 17 };

// Zero-page RAM addresses for DAS and drop timers
#define dasCounter      ram[0x0046] // DAS charge counter
#define dasDelay        ram[0x0047] // Tracks active DAS direction
#define dropSpeed       ram[0x0065] // Current frames-per-drop setting
#define autorepeatTimer ram[0x0066]

// DAS Constants (NTSC 60Hz timings)
#define DAS_INITIAL_DELAY 16
#define DAS_REPEAT_RATE   6

#endif // TETRIS_CONSTANTS_H