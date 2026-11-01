// src/scores.c
#include "scores.h"
#include <stdio.h>
#include <string.h>

#define SCORES_FILE "monktris_scores.dat"

static HighScoreEntry high_scores[3] = {
    { "MNK", 10000 },
    { "TRS",  5000 },
    { "NES",  2000 }
};

void scores_init(void) {
    FILE* f = fopen(SCORES_FILE, "rb");
    if (f) {
        fread(high_scores, sizeof(HighScoreEntry), 3, f);
        fclose(f);
    }
}

static void scores_save(void) {
    FILE* f = fopen(SCORES_FILE, "wb");
    if (f) {
        fwrite(high_scores, sizeof(HighScoreEntry), 3, f);
        fclose(f);
    }
}

bool scores_qualifies(uint32_t score) {
    return score > high_scores[2].score;
}

void scores_insert(const char* name, uint32_t score) {
    int insert_idx = -1;
    for (int i = 0; i < 3; i++) {
        if (score > high_scores[i].score) {
            insert_idx = i;
            break;
        }
    }

    if (insert_idx >= 0) {
        for (int i = 2; i > insert_idx; i--) {
            high_scores[i] = high_scores[i - 1];
        }
        strncpy(high_scores[insert_idx].name, name, 3);
        high_scores[insert_idx].name[3] = '\0';
        high_scores[insert_idx].score = score;
        scores_save();
    }
}

const HighScoreEntry* scores_get(int index) {
    if (index >= 0 && index < 3) return &high_scores[index];
    return &high_scores[0];
}