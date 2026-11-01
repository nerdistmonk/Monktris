// src/tetris_game.c
#include "tetris_game.h"
#include "apu.h"
#include "hud_score.h"
#include "scores.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

enum GameState {
    STATE_ACTIVE_PIECE,
    STATE_LINE_CLEAR,
    STATE_SPAWN,
    STATE_GAMEOVER
};

static enum GameState current_state = STATE_SPAWN;

bool isGamePaused = false;
bool isGameOver = false;
uint8_t currentMusicTrack = 1;

char entered_initials[4] = "AAA";
int initial_cursor = 0;
uint32_t ending_frame_counter = 0;

extern bool tetris_spawn_piece(void);
extern bool tetris_update_active_piece(void);
extern uint8_t tetris_detect_full_lines(void);
extern bool tetris_animate_line_clear(void);
extern void playfield_clear(void);
extern uint8_t roll_next_piece(void);
extern void update_rng(void);

void tetris_start_new_game(void) {
    playfield_clear();
    linesClearedLo = 0;
    score0 = score1 = score2 = 0;

    statsCount_T = statsCount_J = statsCount_Z = statsCount_O = 0;
    statsCount_S = statsCount_L = statsCount_I = 0;

    isGamePaused = false;
    isGameOver = false;

    currentPiece = roll_next_piece();
    nextPiece = roll_next_piece();
    current_state = STATE_SPAWN;
    gameMode = 1;

    if (currentMusicTrack > 0) {
        apu_set_music(currentMusicTrack);
    }
}

void tetris_init(void) {
    scores_init();
    gameMode = 0;
    levelNumber = 0;
    currentMusicTrack = 1;
    isGamePaused = false;
    isGameOver = false;

    uint32_t seed = (uint32_t)time(NULL) ^ 0x5A5A5A5A;
    rng_lo = (uint8_t)(seed & 0xFF);
    rng_hi = (uint8_t)((seed >> 8) & 0xFF);
    if (rng_lo == 0 && rng_hi == 0) {
        rng_lo = 0x89;
        rng_hi = 0xAB;
    }

    for (int i = 0; i < 16; i++) {
        update_rng();
    }
}

static void handle_menu_input(void) {
    if (buttonsPressed & BTN_RIGHT) {
        if (levelNumber < 19) levelNumber++;
        apu_play_sfx(SFX_MOVE);
    }
    if (buttonsPressed & BTN_LEFT) {
        if (levelNumber > 0) levelNumber--;
        apu_play_sfx(SFX_MOVE);
    }

    if (buttonsPressed & BTN_DOWN) {
        currentMusicTrack = (currentMusicTrack + 1) % 4;
        apu_set_music(currentMusicTrack);
    }
    if (buttonsPressed & BTN_UP) {
        currentMusicTrack = (currentMusicTrack == 0) ? 3 : currentMusicTrack - 1;
        apu_set_music(currentMusicTrack);
    }

    if (buttonsPressed & BTN_START) {
        tetris_start_new_game();
    }
}

static void handle_initials_input(void) {
    if (buttonsPressed & BTN_UP) {
        if (entered_initials[initial_cursor] == 'Z') entered_initials[initial_cursor] = 'A';
        else entered_initials[initial_cursor]++;
        apu_play_sfx(SFX_MOVE);
    }
    if (buttonsPressed & BTN_DOWN) {
        if (entered_initials[initial_cursor] == 'A') entered_initials[initial_cursor] = 'Z';
        else entered_initials[initial_cursor]--;
        apu_play_sfx(SFX_MOVE);
    }

    if (buttonsPressed & (BTN_A | BTN_RIGHT)) {
        initial_cursor++;
        apu_play_sfx(SFX_ROTATE);
        if (initial_cursor > 2) {
            scores_insert(entered_initials, hud_get_score_integer());
            gameMode = 3;
            ending_frame_counter = 0;
            apu_set_music(1);
        }
    }
    if (buttonsPressed & (BTN_B | BTN_LEFT)) {
        if (initial_cursor > 0) {
            initial_cursor--;
            apu_play_sfx(SFX_MOVE);
        }
    }
}

void tetris_step_frame(void) {
    update_rng();

    if (gameMode == 0) {
        handle_menu_input();
        return;
    }

    if (gameMode == 2) {
        handle_initials_input();
        return;
    }

    if (gameMode == 3) {
        ending_frame_counter++;
        if (buttonsPressed & (BTN_START | BTN_A)) {
            gameMode = 0;
            current_state = STATE_SPAWN;
            isGameOver = false;
        }
        return;
    }

    if ((buttonsPressed & BTN_START) && !isGameOver) {
        isGamePaused = !isGamePaused;
        return;
    }

    if (isGamePaused) return;

    switch (current_state) {
        case STATE_SPAWN:
            if (!tetris_spawn_piece()) {
                current_state = STATE_GAMEOVER;
                isGameOver = true;
                if (scores_qualifies(hud_get_score_integer())) {
                    gameMode = 2;
                    initial_cursor = 0;
                    entered_initials[0] = 'A';
                    entered_initials[1] = 'A';
                    entered_initials[2] = 'A';
                    entered_initials[3] = '\0';
                }
            } else {
                current_state = STATE_ACTIVE_PIECE;
            }
            break;

        case STATE_ACTIVE_PIECE:
            if (tetris_update_active_piece()) {
                if (tetris_detect_full_lines() > 0) {
                    current_state = STATE_LINE_CLEAR;
                } else {
                    current_state = STATE_SPAWN;
                }
            }
            break;

        case STATE_LINE_CLEAR:
            if (tetris_animate_line_clear()) {
                current_state = STATE_SPAWN;
            }
            break;

        case STATE_GAMEOVER:
            if (buttonsPressed & (BTN_START | BTN_A)) {
                gameMode = 0;
                current_state = STATE_SPAWN;
                isGameOver = false;
            }
            break;
    }
}