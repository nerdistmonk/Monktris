// src/tetris_game.h
#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include "cpu_state.h"

#define gameMode ram[0x00C0] // 0: Menu, 1: Play, 2: Initials Entry, 3: Ending Screen
#define gameState ram[0x0048]
#define currentPiece ram[0x0042]
#define nextPiece ram[0x00BF]
#define pieceX ram[0x0040]
#define pieceY ram[0x0041]
#define pieceRotation ram[0x0043]
#define fallTimer ram[0x0045]
#define levelNumber ram[0x0044]
#define linesClearedLo ram[0x0050]
#define score0 ram[0x0053]
#define score1 ram[0x0054]
#define score2 ram[0x0055]
#define rng_lo ram[0x0017]
#define rng_hi ram[0x0018]

#define buttonsDown ram[0x00F5]
#define buttonsPressed ram[0x00F7]

#define BTN_RIGHT  0x01
#define BTN_LEFT   0x02
#define BTN_DOWN   0x04
#define BTN_UP     0x08
#define BTN_START  0x10
#define BTN_SELECT 0x20
#define BTN_B      0x40
#define BTN_A      0x80

void tetris_init(void);
void tetris_step_frame(void);

#endif // TETRIS_GAME_H